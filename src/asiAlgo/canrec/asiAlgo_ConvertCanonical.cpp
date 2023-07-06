//-----------------------------------------------------------------------------
// Created on: 29 October 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Own include
#include <asiAlgo_ConvertCanonical.h>

// asiAlgo includes
#include <asiAlgo_BRepNormalizer.h>
#include <asiAlgo_ConvertCanonicalMod.h>
#include <asiAlgo_GeomSummary.h>
#include <asiAlgo_Utils.h>

// asiAlgo includes
#include <asiAlgo_Timer.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepTools_Modifier.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ElementarySurface.hxx>
#include <Geom_Plane.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeFix.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Wire.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

namespace
{
  //! This is a copy of `ShapeCustom::ApplyModifier` giving us more control on
  //! what's going on.
  TopoDS_Shape ApplyModifier(const TopoDS_Shape&                      S,
                             const Handle(asiAlgo_BRepNormalization)& M,
                             TopTools_DataMapOfShapeShape&            context,
                             asiAlgo_BRepNormalizer&                  MD,
                             ActAPI_ProgressEntry                     progress,
                             ActAPI_PlotterEntry                      plotter,
                             const Handle(ShapeBuild_ReShape)&        reShape = nullptr)
  {
    // protect against INTERNAL/EXTERNAL shapes
    TopoDS_Shape SF = S.Oriented(TopAbs_FORWARD);
  
    // Process COMPOUNDs separately in order to handle sharing in assemblies
    if ( SF.ShapeType() == TopAbs_COMPOUND )
    {
      bool locModified = false;
      TopoDS_Compound C;
      BRep_Builder B;
      B.MakeCompound(C);

      for ( TopoDS_Iterator it(SF); it.More(); it.Next() )
      {
        TopoDS_Shape    shape = it.Value();
        TopLoc_Location L     = shape.Location(), nullLoc;
        shape.Location(nullLoc);
        TopoDS_Shape res;

        if ( context.IsBound(shape) )
          res = context.Find(shape).Oriented ( shape.Orientation() );
        else
          res = ApplyModifier(shape, M, context, MD, progress, plotter);

        if ( !res.IsSame(shape) )
        {
          context.Bind(shape, res);
          locModified = true;
        }

        res.Location(L, false);
        B.Add(C, res);
      }

      if ( !locModified )
        return S;

      context.Bind(SF, C);
      return C.Oriented( S.Orientation() );
    }

    // Modify the shape
    MD.Init(SF);
    MD.Perform(M);

    if ( !reShape.IsNull() )
    {
      for ( TopoDS_Iterator it(SF, false); it.More(); it.Next() )
      {
        const TopoDS_Shape& current = it.Value();
        TopoDS_Shape result;

        if ( !MD.ModifiedShape(current, result) )
        {
          progress.SendLogMessage(LogErr(Normal) << "Failed to modify shape.");
          continue;
        }

        if ( !result.IsNull() && !current.IsSame(result) )
        {
          reShape->Replace(current, result);
        }
      }
    }

    TopoDS_Shape RS;
    if ( MD.ModifiedShape(SF, RS) )
      return RS.Oriented( S.Orientation() );

    return TopoDS_Shape();
  }
}

//-----------------------------------------------------------------------------

asiAlgo_ConvertCanonical::asiAlgo_ConvertCanonical(ActAPI_ProgressEntry progress,
                                                   ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm(progress, plotter)
{
  m_history = new BRepTools_History();
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_ConvertCanonical::Perform(const TopoDS_Shape& shape,
                                               const double        tol,
                                               const bool          convertSurfaces,
                                               const bool          convertCurves,
                                               const bool          buildHistory)
{
#if defined COUT_DEBUG
  TIMER_NEW
  TIMER_GO
#endif

  /* ======================
   *  Prepare modification.
   * ====================== */

  Handle(asiAlgo_ConvertCanonicalMod)
    M = new asiAlgo_ConvertCanonicalMod;
  //
  M->SetTolerance   (tol);
  M->SetSurfaceMode (convertSurfaces);
  M->SetCurveMode   (convertCurves);

  /* ====================
   *  Apply modification.
   * ==================== */

  TopTools_DataMapOfShapeShape context;
  asiAlgo_BRepNormalizer       MD(m_progress, m_plotter);
  TopoDS_Shape                 result;

  try // You never know...
  {
    result = ::ApplyModifier(shape, M, context, MD, m_progress, m_plotter);
  }
  catch ( ... )
  {
    return shape;
  }

  /* ================
   *  Fix the result.
   * ================ */

  // Fix faces.
  Handle(ShapeBuild_ReShape) cxt = new ShapeBuild_ReShape;
  this->fixFaces(result, cxt, tol);
  //
  result = cxt->Apply(result);

  // Fix edges.
  this->fixEdges(result);

  /* ==================
   *  Populate summary.
   * ================== */

  asiAlgo_GeomSummary summ[2];
  asiAlgo_Utils::GeomSummary(shape,  summ[0]);
  asiAlgo_Utils::GeomSummary(result, summ[1]);
  //
  m_summary = asiAlgo_ConvertCanonicalSummary(summ[0], summ[1]);

#if defined COUT_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_progress, "asiAlgo_ConvertCanonical::Perform()")
#endif

  if ( buildHistory )
    this->fillHistory(shape, result);

  return result;
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonical::fixFaces(const TopoDS_Shape&         result,
                                        Handle(ShapeBuild_ReShape)& context,
                                        const double                tol)
{
  BRep_Builder bbuilder;

  // Process (fix) each face individually.
  for ( TopExp_Explorer ex_f(result, TopAbs_FACE); ex_f.More(); ex_f.Next() )
  {
    TopoDS_Shape
      shape = context->Apply( ex_f.Current().Oriented(TopAbs_FORWARD) );

    TopLoc_Location L;
    const TopoDS_Face&   face = TopoDS::Face(shape);
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face, L);

    // Perform fixes for the elementary surfaces only as these are presumably
    // the outcomes of canonical conversion.
    if ( surf->IsKind( STANDARD_TYPE(Geom_ElementarySurface) ) ||
         surf->IsKind( STANDARD_TYPE(Geom_SweptSurface) ) )
    {
      int nbWires = 0;

      // Fix wires.
      for ( TopExp_Explorer wexp(face, TopAbs_WIRE); wexp.More(); wexp.Next() )
      {
        const TopoDS_Wire& wire = TopoDS::Wire( wexp.Current() );

        nbWires++;

        Handle(ShapeFix_Wire)
          sfw = new ShapeFix_Wire( wire,
                                   face,
                                   Precision::Confusion() );

        // Order of edges.
        sfw->FixReorder();
        //
        if ( sfw->StatusReorder(ShapeExtend_FAIL) )
          continue;

        // Fix shifted pcurves.
        sfw->SetPrecision(2.*tol);
        sfw->FixShifted();

        bool isDone = sfw->LastFixStatus(ShapeExtend_DONE);

        // Fix degenerated.
        isDone |= sfw->FixDegenerated();

        // Remove degenerated edges from not degenerated corners.
        ShapeAnalysis_Edge sae;
        Handle(ShapeExtend_WireData) sewd = sfw->WireData();
        //
        for ( int i = 1; i <= sewd->NbEdges(); ++i )
        {
          TopoDS_Edge E = sewd->Edge(i);

          if ( BRep_Tool::Degenerated(E) && !sae.HasPCurve(E, face) )
          {
            sewd->Remove(i);
            isDone = true;
            i--;
          }
        }

        // Fix lacking.
        isDone |= sfw->FixLacking();

        if ( isDone )
        {
          TopoDS_Wire resWire = sfw->Wire();
          context->Replace(wire, resWire);
        }
      }

      // Fix orientation in case of several wires.
      if ( nbWires > 1 )
      {
        TopoDS_Face           fixedFace = TopoDS::Face( context->Apply(face) );
        Handle(ShapeFix_Face) sff       = new ShapeFix_Face(fixedFace);

        if ( sff->FixOrientation() )
          context->Replace( fixedFace, sff->Face() );
      }

      // Reset the `NaturalRestriction` flag for the infinite surfaces
      // after converting them from splines.
      if ( !BRep_Tool::NaturalRestriction(face) || nbWires == 0 )
        continue;
      //
      if ( surf->IsKind( STANDARD_TYPE(Geom_Plane) )              ||
           surf->IsKind( STANDARD_TYPE(Geom_CylindricalSurface) ) ||
           surf->IsKind( STANDARD_TYPE(Geom_ConicalSurface) )     ||
           surf->IsKind( STANDARD_TYPE(Geom_SweptSurface) ) )
      {
        bbuilder.NaturalRestriction(face, false);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonical::fixEdges(const TopoDS_Shape& result)
{
  // Adjust the tolerances of edges to accommodate the tolerances of vertices.
  ShapeFix_Edge sfe;
  for ( TopExp_Explorer exp(result, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    TopoDS_Edge E = TopoDS::Edge( exp.Current() );
    sfe.FixVertexTolerance(E);
  }

  // Fix same parameterization for edges.
  ShapeFix::SameParameter(result, false, 0.0);
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonical::fillHistory(const TopoDS_Shape& input,
                                           const TopoDS_Shape& output)
{
  m_history->Clear();

  // Here we take advantage of the fact that canonical recognition is
  // realized as "homeomorphism" based on BRepTools_Modification. It
  // means that topology of the model is not affected, and we can use
  // the same face and solid indices in the result shape as the input shape has.

  // build a history for solids
  TopTools_IndexedMapOfShape inputSolids;
  TopExp::MapShapes(input, TopAbs_SOLID, inputSolids);

  TopTools_IndexedMapOfShape outputSolids;
  TopExp::MapShapes(output, TopAbs_SOLID, outputSolids);

  for ( int si = 1; si <= inputSolids.Extent(); ++si )
  {
    const TopoDS_Shape& solid_in  = inputSolids.FindKey(si);
    const TopoDS_Shape& solid_out = outputSolids.FindKey(si);
    //
    if ( !solid_in.IsSame(solid_out) )
      m_history->AddModified(solid_in, solid_out);
  }

  // build a history for faces
  TopTools_IndexedMapOfShape inputFaces;
  TopExp::MapShapes(input, TopAbs_FACE, inputFaces);

  TopTools_IndexedMapOfShape outputFaces;
  TopExp::MapShapes(output, TopAbs_FACE, outputFaces);

  for ( int fi = 1; fi <= inputFaces.Extent(); ++fi )
  {
    const TopoDS_Face& face_in  = TopoDS::Face( inputFaces.FindKey(fi) );
    const TopoDS_Face& face_out = TopoDS::Face( outputFaces.FindKey(fi) );

    if ( !face_in.IsPartner(face_out) )
    {
      m_history->AddModified(face_in, face_out);
    }
  }
}
