//-----------------------------------------------------------------------------
// Created on: 09 March 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiAlgo_PlateOnEdges.h>

// asiAlgo includes
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_Timer.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgo.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepLib.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomPlate_BuildPlateSurface.hxx>
#include <GeomPlate_HArray1OfHCurve.hxx>
#include <GeomPlate_MakeApprox.hxx>
#include <GeomPlate_PlateG0Criterion.hxx>
#include <NCollection_CellFilter.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Shape.hxx>
#include <TColGeom2d_HArray1OfCurve.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/geom_FairBSurf.h>

  using namespace mobius;
#endif

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

#undef DRAW_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

namespace
{

  class PlateInspector : public NCollection_CellFilter_InspectorXYZ
  {
  public:
    typedef gp_XYZ Target;

    //! Constructor; remembers the tolerance.
    PlateInspector(const double tol)
    { m_tol = tol * tol; m_isFind = false; }

    void ClearFind()
    { m_isFind = false; }

    const bool IsFind() const
    { return m_isFind; }

    //! Set current point to search for coincidence.
    void SetCurrent(const gp_XYZ& pnt)
    { m_current = pnt; }

    //! Implementation of inspection method.
    NCollection_CellFilter_Action Inspect(const Target& obj)
    {
      const gp_XYZ pt = m_current.Subtracted(obj);
      const double sqDist = pt.SquareModulus();
      if( sqDist < m_tol )
        m_isFind = true;

      return CellFilter_Keep;
    }

  private:
    Standard_Real    m_tol;     //!< Squared comparison tolerance.
    gp_XYZ           m_current; //!< Current point.
    Standard_Boolean m_isFind;  //!< Detection state.
  };
}

//-----------------------------------------------------------------------------

asiAlgo_PlateOnEdges::asiAlgo_PlateOnEdges(ActAPI_ProgressEntry progress,
                                           ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_iNumDiscrPts    ( 10 ),
  m_fFairCoeff      ( 0.01 )
{}

//-----------------------------------------------------------------------------

asiAlgo_PlateOnEdges::asiAlgo_PlateOnEdges(const Handle(asiAlgo_AAG)& aag,
                                           ActAPI_ProgressEntry       progress,
                                           ActAPI_PlotterEntry        plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_shape           ( aag->GetMasterShape() ),
  m_aag             ( aag),
  m_iNumDiscrPts    ( 10 ),
  m_fFairCoeff      ( 0.01 )
{}

//-----------------------------------------------------------------------------

asiAlgo_PlateOnEdges::asiAlgo_PlateOnEdges(const TopoDS_Shape&  shape,
                                           ActAPI_ProgressEntry progress,
                                           ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_shape           ( shape ),
  m_iNumDiscrPts    ( 10 ),
  m_fFairCoeff      ( 0.01 )
{}

//-----------------------------------------------------------------------------

void asiAlgo_PlateOnEdges::SetExtraPoints(const Handle(asiAlgo_BaseCloud<double>)& points)
{
  m_extraPts = points;
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlateOnEdges::Build(Handle(TopTools_HSequenceOfShape)& edges,
                                 const unsigned int                 continuity,
                                 Handle(Geom_BSplineSurface)&       support,
                                 TopoDS_Face&                       result)
{
  // Build surface.
  if ( !this->BuildSurf(edges, continuity, support) )
    return false;

  // Build face.
  this->BuildFace(edges, support, result);

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlateOnEdges::Build(const TColStd_PackedMapOfInteger&  edgeIndices,
                                 const unsigned int                 continuity,
                                 Handle(TopTools_HSequenceOfShape)& edges,
                                 Handle(Geom_BSplineSurface)&       support,
                                 TopoDS_Face&                       result)
{
  // Build surface.
  if ( !this->BuildSurf(edgeIndices, continuity, edges, support) )
    return false;

  // Build face.
  if ( !this->BuildFace(edges, support, result) )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlateOnEdges::Build(const TColStd_PackedMapOfInteger& edgeIndices,
                                 const unsigned int                continuity,
                                 Handle(Geom_BSplineSurface)&      support,
                                 TopoDS_Face&                      result)
{
  Handle(TopTools_HSequenceOfShape) edges;
  //
  return this->Build(edgeIndices, continuity, edges, support, result);
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlateOnEdges::BuildSurf(const TColStd_PackedMapOfInteger&  edgeIndices,
                                     const unsigned int                 continuity,
                                     Handle(TopTools_HSequenceOfShape)& edges,
                                     Handle(Geom_BSplineSurface)&       support)
{
  if ( edges.IsNull() )
    edges = new TopTools_HSequenceOfShape;

  for ( TColStd_MapIteratorOfPackedMapOfInteger eit(edgeIndices); eit.More(); eit.Next() )
  {
    const int          eidx = eit.Key();
    const TopoDS_Edge& E    = TopoDS::Edge( m_aag->RequestMapOfEdges().FindKey(eidx) );
    //
    edges->Append(E);
  }

  return this->BuildSurf(edges, continuity, support);
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlateOnEdges::BuildSurf(const Handle(TopTools_HSequenceOfShape)& edges,
                                     const unsigned int                       continuity,
                                     Handle(Geom_BSplineSurface)&             support)
{
  /* ==============================
   *  STAGE 1: prepare constraints
   * ============================== */

  // Fill constraints for giving them back.
  m_pinPts = new asiAlgo_BaseCloud<double>;

  // Create builder instance.
  GeomPlate_BuildPlateSurface builder;
  this->fillConstraints(edges, continuity, builder);

  // Set optional initial surface.
  if ( !m_initSurf.IsNull() )
    builder.LoadInitSurface(m_initSurf);

  /* ======================
   *  STAGE 2: build plate
   * ====================== */

  try
  {
    builder.Perform();
  }
  catch ( ... )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Exception in OCCT plate surface builder.");
    return false;
  }
  //
  if ( !builder.IsDone() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Plating failed.");
    return false;
  }

  Handle(GeomPlate_Surface) plate = builder.Surface();

  if ( plate.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Plating resulted in null surface.");
    return false;
  }

  /* =======================================
   *  STAGE 3: approximate plate with NURBS
   * ======================================= */

  GeomPlate_MakeApprox MKS(plate, 1.0e-7, 16, 7, 1.0e-7, 0);
  support = MKS.Surface();

  /* ================================
   *  STAGE 4: fair surface if asked
   * ================================ */

   if ( m_fFairCoeff )
   {
#if defined USE_MOBIUS
     TIMER_NEW
     TIMER_GO

     t_ptr<t_bsurf> mobSurf = cascade::GetMobiusBSurface(support);

     // Perform fairing.
     geom_FairBSurf fairing(mobSurf, m_fFairCoeff, nullptr, nullptr);
     //
     const int nPolesU = int( mobSurf->GetPoles().size() );
     const int nPolesV = int( mobSurf->GetPoles()[0].size() );
     //
     for ( int i = 0; i < nPolesU; ++i )
     {
       fairing.AddPinnedPole( i, 0 );
       fairing.AddPinnedPole( i, nPolesV - 1 );
     }
     //
     for ( int j = 0; j < nPolesV; ++j )
     {
       fairing.AddPinnedPole( 0, j );
       fairing.AddPinnedPole( nPolesU - 1, j );
     }
     //
     if ( !fairing.Perform() )
     {
       m_progress.SendLogMessage(LogErr(Normal) << "Fairing failed.");
       return false;
     }

     // Get the faired surface.
     const t_ptr<t_bsurf>& mobResult = fairing.GetResult();

     // Convert to OCCT B-surface.
     support = cascade::GetOpenCascadeBSurface(mobResult);

     TIMER_FINISH
     TIMER_COUT_RESULT_MSG("Fair B-surface")
#else
     m_progress.SendLogMessage(LogWarn(Normal) << "Fairing feature is not available.");
#endif
   }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_PlateOnEdges::BuildFace(Handle(TopTools_HSequenceOfShape)& edges,
                                     const Handle(Geom_BSplineSurface)& support,
                                     TopoDS_Face&                       result)
{
  if ( edges->IsEmpty() )
    return false;

  // Compose a new wire.
  Handle(TopTools_HSequenceOfShape) freeWires;
  ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, 1e-3, 0, freeWires);
  //
  const TopoDS_Wire& repatchW = TopoDS::Wire( freeWires->First() );

  // Build new face.
  TopoDS_Face newF = BRepBuilderAPI_MakeFace(support, repatchW, false);

  // Heal defects.
  ShapeFix_Shape shapeHealer(newF);
  shapeHealer.Perform();
  newF = TopoDS::Face( shapeHealer.Shape() );

  // Classify point to invert wire if necessary.
  BRepTopAdaptor_FClass2d FClass(newF, 0.0);
  if ( FClass.PerformInfinitePoint() == TopAbs_IN )
  {
    BRep_Builder B;
    TopoDS_Shape S = newF.EmptyCopied();
    TopoDS_Iterator it(newF);
    while ( it.More() )
    {
      B.Add( S, it.Value().Reversed() );
      it.Next();
    }
    newF = TopoDS::Face(S);
  }

  // Check if the face is valid.
  asiAlgo_CheckValidity checker;
  const double tol               = checker.GetMaxTolerance(newF)*5.0;
  const bool   hasAllClosedWires = checker.HasAllClosedWires(newF, tol);
  //
  if ( hasAllClosedWires )
    result = newF;
  else
    return false;

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_PlateOnEdges::fillConstraints(const Handle(TopTools_HSequenceOfShape)& edges,
                                           const unsigned int                       continuity,
                                           GeomPlate_BuildPlateSurface&             builder) const
{
  const double tol =  0.5;
  NCollection_CellFilter<PlateInspector> cellFilter(tol);

  /* ================================
   *  Add constraints over the edges.
   * ================================ */

  // Iterate over edges to add point constraints.
  for ( int i = 1; i <= edges->Size(); ++i )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( edges->Value(i) );
    BRepAdaptor_Curve curve(edge);
    const double f = curve.FirstParameter();
    const double l = curve.LastParameter();

    // Get points from the current edge.
    const int nbPnt = 23;
    for ( int j = 0; j < nbPnt; ++j )
    {
      const double param = f + j * (l - f) / (nbPnt - 1);
      const gp_Pnt pnt = curve.Value(param);

      PlateInspector inspector(tol);
      const gp_XYZ min = inspector.Shift(pnt.XYZ(), -tol);
      const gp_XYZ max = inspector.Shift(pnt.XYZ(),  tol);

      inspector.ClearFind();
      inspector.SetCurrent( pnt.XYZ() );
      cellFilter.Inspect(min, max, inspector);

      if ( !inspector.IsFind() )
      {
        cellFilter.Add( pnt.XYZ(), pnt.XYZ() );

        Handle(GeomPlate_PointConstraint)
          pntCon = new GeomPlate_PointConstraint(pnt, continuity, 1.0e-4);

        builder.Add(pntCon);

        // Store constraint for reference.
        m_pinPts->AddElement(pnt);
      }
    }
  }

  /* ================================
   *  Add optional inner constraints.
   * ================================ */

  if ( !m_extraPts.IsNull() )
  {
    for ( int i = 0; i < m_extraPts->GetNumberOfElements(); ++i )
    {
      gp_Pnt Pi = m_extraPts->GetElement(i);

      Handle(GeomPlate_PointConstraint)
        pntCon = new GeomPlate_PointConstraint(Pi, continuity, 1.0e-4);

      builder.Add(pntCon);

      // Store constraint for reference.
      m_pinPts->AddElement(Pi);
    }
  }
}
