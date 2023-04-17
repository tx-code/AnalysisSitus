//-----------------------------------------------------------------------------
// Created on: 04 March 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
#include "asiAlgo_AppSurf2.h"

// asiAlgo includes
#include <asiAlgo_AppSurfUtils.h>
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_MobiusProgressNotifier.h>
#include <asiAlgo_PlaneOnPoints.h>
#include <asiAlgo_Timer.h>

#if defined USE_MOBIUS
  // Mobius includes
  #include <mobius/cascade.h>
  #include <mobius/geom_ApproxBSurf.h>
#endif

// OCCT includes
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <GeomConvert.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

using namespace mobius;

//-----------------------------------------------------------------------------

asiAlgo_AppSurf2::asiAlgo_AppSurf2(core_ProgressEntry progress,
                                   core_PlotterEntry  plotter)
: core_IAlgorithm   (progress, plotter),
  m_fEdgeDiscrPrec  ( 1.0 ),
  m_fFairCoeff      ( 0.01 ),
  m_iNumUKnots      ( 2 ),
  m_iNumVKnots      ( 2 ),
  m_iDegU           ( 3 ),
  m_iDegV           ( 3 )
{}

//-----------------------------------------------------------------------------

void asiAlgo_AppSurf2::SetExtraPoints(const Handle(asiAlgo_BaseCloud<double>)& points)
{
  m_extraPts = points;
}

//-----------------------------------------------------------------------------

bool asiAlgo_AppSurf2::Build(const Handle(TopTools_HSequenceOfShape)& edges,
                             Handle(Geom_BSplineSurface)&             support,
                             TopoDS_Face&                             face)
{
  /* ==============================
   *  STAGE 1: prepare constraints.
   * ============================== */

  t_ptr<t_pcloud> pts = new t_pcloud;

  asiAlgo_AppSurfUtils::PrepareConstraints(m_fEdgeDiscrPrec, edges, m_extraPts, pts);

  // Fill constraints for giving them back.
  m_pinPts = new asiAlgo_BaseCloud<double>;
  //
  for ( int k = 0; k < pts->GetNumberOfPoints(); ++k )
    m_pinPts->AddElement( cascade::GetOpenCascadePnt( pts->GetPoint(k) ) );

  /* ======================
   *  STAGE 2: approximate.
   * ====================== */

  TIMER_NEW
  TIMER_GO

  // Prepare approximation tool.
  geom_ApproxBSurf approx(pts, m_iDegU, m_iDegV, m_progress);

  // Optional initial surface.
  Handle(Geom_BSplineSurface) initSurf;
  //
  if ( !m_initSurf.IsNull() )
  {
    initSurf = GeomConvert::SurfaceToBSplineSurface(m_initSurf);
  }
  else
  {
    initSurf = asiAlgo_AppSurfUtils::PrepareInitialPlane(m_pinPts,
                                                         m_iNumUKnots,
                                                         m_iNumVKnots,
                                                         m_iDegU,
                                                         m_iDegV);
  }
  //
  if ( !initSurf.IsNull() )
    approx.SetInitSurface( cascade::GetMobiusBSurface(initSurf) );

  // Approximate.
  try
  {
    if ( !approx.Perform(m_fFairCoeff) )
    {
      m_progress.SendLogMessage(MobiusErr(Normal) << "Approximation with APPSURF2 failed.");
      return false;
    }
  }
  catch ( ... )
  {
    m_progress.SendLogMessage(MobiusErr(Normal) << "Approximation with APPSURF2 failed.");
    return false;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER_MOBIUS(m_progress, "APPSURF2")

  // Get result.
  t_ptr<t_bsurf> mobResSurf = approx.GetResult();

  // Convert to OpenCascade B-surface.
  support = cascade::GetOpenCascadeBSurface(mobResSurf);

  /* ====================
   *  STAGE 3: make face.
   * ==================== */

  if ( !edges->IsEmpty() )
  {
    // To get rid of `const`.
    Handle(TopTools_HSequenceOfShape)
      unsortedEdges = new TopTools_HSequenceOfShape;
    //
    for ( TopTools_SequenceOfShape::Iterator eit(*edges); eit.More(); eit.Next() )
      unsortedEdges->Append( eit.Value() );

    // Compose a new wire.
    Handle(TopTools_HSequenceOfShape) freeWires;
    ShapeAnalysis_FreeBounds::ConnectEdgesToWires(unsortedEdges, 1e-3, 0, freeWires);
    //
    const TopoDS_Wire& repatchW = TopoDS::Wire( freeWires->First() );

    // Build new face.
    TopoDS_Face newF = BRepBuilderAPI_MakeFace(support, repatchW, false);

    // Heal defects.
    ShapeFix_Shape shapeHealer(newF);
    shapeHealer.Perform();
    newF = TopoDS::Face( shapeHealer.Shape() );

    // Check if the face is valid.
    asiAlgo_CheckValidity checker;
    const double tol               = checker.GetMaxTolerance(face)*5.0;
    const bool   hasAllClosedWires = checker.HasAllClosedWires(face, tol);
    //
    if ( hasAllClosedWires )
      face = newF;
  }

  return true;
}
