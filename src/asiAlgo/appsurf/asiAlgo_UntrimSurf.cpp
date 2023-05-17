//-----------------------------------------------------------------------------
// Created on: 17 May 2023
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
#include "asiAlgo_UntrimSurf.h"

// asiAlgo includes
#include "asiAlgo_BuildCoonsSurf.h"

// OpenCascade includes
#include <BRep_Tool.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

// Standard includes
#include <set>

//-----------------------------------------------------------------------------

void asiAlgo_UntrimSurf::CheckDeviation(const Handle(Geom_BSplineSurface)& resSurf,
                                        const Handle(Geom_BSplineSurface)& initSurf,
                                        double&                            maxDev,
                                        ActAPI_PlotterEntry                plotter)
{
  const int numSteps = 10;

  /* ===============================================
   *  Prepare probe points on the untrimmed surface.
   * =============================================== */

  double uMin, uMax, vMin, vMax;
  resSurf->Bounds(uMin, uMax, vMin, vMax);

  const double uStep = (uMax - uMin) / numSteps;
  const double vStep = (vMax - vMin) / numSteps;

  // Choose u values
  std::vector<double> U;
  {
    double u     = uMin;
    bool   uStop = false;
    //
    while ( !uStop )
    {
      if ( u > uMax )
      {
        u     = uMax;
        uStop = true;
      }

      U.push_back(u);
      u += uStep;
    }
  }

  // Choose v values
  std::vector<double> V;
  {
    double v     = vMin;
    bool   vStop = false;
    //
    while ( !vStop )
    {
      if ( v > vMax )
      {
        v     = vMax;
        vStop = true;
      }

      V.push_back(v);
      v += vStep;
    }
  }

  /* =======================================
   *  Check distance to the initial surface.
   * ======================================= */

  ShapeAnalysis_Surface sas(initSurf);

  double maxDist = 0;

  for ( const auto u : U )
  {
    for ( const auto v : V )
    {
      gp_Pnt   probe  = resSurf->Value(u, v);
      gp_Pnt2d projUV = sas.ValueOfUV(probe, 1e-3);
      gp_Pnt   proj   = initSurf->Value( projUV.X(), projUV.Y() );

      const double d = proj.Distance(probe);
      //
      if ( d > maxDist )
        maxDist = d;
    }
  }

  maxDev = maxDist;
}

//-----------------------------------------------------------------------------

asiAlgo_UntrimSurf::asiAlgo_UntrimSurf(ActAPI_ProgressEntry progress,
                                       ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_iNumUKnots      ( 2 ),
  m_iNumVKnots      ( 2 ),
  m_iDegU           ( 3 ),
  m_iDegV           ( 3 ),
  m_fMaxError       ( 0. )
{}

//-----------------------------------------------------------------------------

bool asiAlgo_UntrimSurf::Build(const Handle(TopTools_HSequenceOfShape)& inFaces,
                               const Handle(TopTools_HSequenceOfShape)& inEdges,
                               Handle(Geom_BSplineSurface)&             outSupport,
                               TopoDS_Face&                             outFace)
{
  /* =======================
   *  Apply contract checks.
   * ======================= */

  if ( inFaces->Size() != 1 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Only one face can be passed to UNTRIM.");
    return false;
  }

  const TopoDS_Face& face = TopoDS::Face( inFaces->First() );

  // Check surface type: only splines are currently allowed.
  Handle(Geom_BSplineSurface)
    initSurf = Handle(Geom_BSplineSurface)::DownCast( BRep_Tool::Surface(face) );
  //
  if ( initSurf.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "The input face is not a B-spline surface.");
    return false;
  }

  /* ===================================
   *  Construct the initial Coons patch.
   * =================================== */

  // Find rail curves.
  Handle(Geom_BSplineCurve) c0, c1, b0, b1;
  //
  if ( !this->sortEdges(inEdges, c0, c1, b0, b1) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot sort edges to build a Coons patch.");
    return false;
  }

  // Build a Coons patch.
  asiAlgo_BuildCoonsSurf coonsAlgo(c0, c1, b0, b1, m_progress, m_plotter);
  //
  if ( !coonsAlgo.Perform() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot build a bilinear Coons patch.");
    return false;
  }
  //
  const Handle(Geom_BSplineSurface)& coonsSurf = coonsAlgo.GetResult();
  //
  m_plotter.REDRAW_SURFACE("coonsSurf", coonsSurf, Color_Default);

  // TODO: NYI

  /* ==================
   *  Check deviations.
   * ================== */

  // Check deviation from the initial surface.
  CheckDeviation(coonsSurf, initSurf, m_fMaxError, m_plotter);

  m_progress.SendLogMessage(LogNotice(Normal) << "Max deviation: %1."
                                              << m_fMaxError);

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_UntrimSurf::sortEdges(const Handle(TopTools_HSequenceOfShape)& edges,
                                   Handle(Geom_BSplineCurve)&               c0,
                                   Handle(Geom_BSplineCurve)&               c1,
                                   Handle(Geom_BSplineCurve)&               b0,
                                   Handle(Geom_BSplineCurve)&               b1) const
{
  if ( edges->Size() != 4 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Only 4 edges are supported.");
    return false; // contract check
  }

  const double tol3d = 1e-2;

  // Edge analysis tool.
  ShapeAnalysis_Edge sae;

  // Take the first edge as a reference.
  const TopoDS_Edge& e0 = TopoDS::Edge( edges->First() );
  //
  gp_Pnt e0_Pf = BRep_Tool::Pnt( sae.FirstVertex(e0) );
  gp_Pnt e0_Pl = BRep_Tool::Pnt( sae.LastVertex(e0) );

  // Find another edge that has no extremities close to the first one's.
  TopoDS_Edge e1;
  int         e1_idx = 0;
  //
  for ( int eidx = 2; eidx <= edges->Length(); ++eidx )
  {
    // Take the next edge.
    const TopoDS_Edge& ei = TopoDS::Edge( (*edges)(eidx) );
    //
    gp_Pnt ei_Pf = BRep_Tool::Pnt( sae.FirstVertex(ei) );
    gp_Pnt ei_Pl = BRep_Tool::Pnt( sae.LastVertex(ei) );

    const double d_ff = e0_Pf.Distance(ei_Pf);
    const double d_fl = e0_Pf.Distance(ei_Pl);
    const double d_lf = e0_Pl.Distance(ei_Pf);
    const double d_ll = e0_Pl.Distance(ei_Pl);

    if ( (d_ff > tol3d) && (d_fl > tol3d) &&
         (d_lf > tol3d) && (d_ll > tol3d) )
    {
      e1     = ei;
      e1_idx = eidx;
      break;
    }
  }

  if ( !e1_idx )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot find a pair for e0.");
    return false;
  }

  std::vector<TopoDS_Edge> railEdges; // <c0, c1, b0, b1>
  std::set<int>            consumedIds = {1, e1_idx};
  //
  railEdges.push_back(e0);
  railEdges.push_back(e1);

  // Find other edges.
  for ( int eidx = 2; eidx <= edges->Length(); ++eidx )
  {
    if ( consumedIds.find(eidx) == consumedIds.end() )
      railEdges.push_back( TopoDS::Edge( (*edges)(eidx) ) );
  }

  if ( railEdges.size() != 4 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot group edges into pairs of rails.");
    return false;
  }

  // Initialize rail curves.
  double f, l;
  c0 = Handle(Geom_BSplineCurve)::DownCast( BRep_Tool::Curve(railEdges[0], f, l) );
  c1 = Handle(Geom_BSplineCurve)::DownCast( BRep_Tool::Curve(railEdges[1], f, l) );
  b0 = Handle(Geom_BSplineCurve)::DownCast( BRep_Tool::Curve(railEdges[2], f, l) );
  b1 = Handle(Geom_BSplineCurve)::DownCast( BRep_Tool::Curve(railEdges[3], f, l) );

  m_plotter.REDRAW_CURVE("c0_original", c0, Color_Red,   true);
  m_plotter.REDRAW_CURVE("c1_original", c1, Color_Red,   true);
  m_plotter.REDRAW_CURVE("b0_original", b0, Color_Green, true);
  m_plotter.REDRAW_CURVE("b1_original", b1, Color_Green, true);

  // Orient `c` curves.
  {
    gp_Pnt P0, P1;
    gp_Vec V0, V1;

    c0->D1( 0.5*( c0->FirstParameter() + c0->LastParameter() ), P0, V0 );
    c1->D1( 0.5*( c1->FirstParameter() + c1->LastParameter() ), P1, V1 );

    const double dot = V0.Dot(V1);

    if ( dot < 0 )
      c1->Reverse();
  }

  // Orient `b` curves.
  {
    gp_Pnt P0, P1;
    gp_Vec V0, V1;

    b0->D1( 0.5*( b0->FirstParameter() + b0->LastParameter() ), P0, V0 );
    b1->D1( 0.5*( b1->FirstParameter() + b1->LastParameter() ), P1, V1 );

    const double dot = V0.Dot(V1);

    if ( dot < 0 )
      b1->Reverse();
  }

  /* b0 and c0 should have a common point. */

  // Case 1: c0(l) == b0(f) -> reverse c0
  {
    gp_Pnt c0_Pl = c0->Value( c0->LastParameter() );
    gp_Pnt b0_Pf = b0->Value( b0->FirstParameter() );
    //
    const double controlDist = c0_Pl.Distance(b0_Pf);
    //
    if ( controlDist < tol3d )
    {
      c0->Reverse();
      c1->Reverse(); // Coupled one.
    }
  }

  // Case 2: c0(f) == b0(l) -> reverse b0
  {
    gp_Pnt c0_Pf = c0->Value( c0->FirstParameter() );
    gp_Pnt b0_Pl = b0->Value( b0->LastParameter() );
    //
    const double controlDist = c0_Pf.Distance(b0_Pl);
    //
    if ( controlDist < tol3d )
    {
      b0->Reverse();
      b1->Reverse(); // Coupled one.
    }
  }

  // Case 3: c0(l) == b0(l) -> reverse c0, reverse b0
  {
    gp_Pnt c0_Pl = c0->Value( c0->LastParameter() );
    gp_Pnt b0_Pl = b0->Value( b0->LastParameter() );
    //
    const double controlDist = c0_Pl.Distance(b0_Pl);
    //
    if ( controlDist < tol3d )
    {
      c0->Reverse();
      c1->Reverse(); // Coupled one.
      b0->Reverse();
      b1->Reverse(); // Coupled one.
    }
  }

  m_plotter.REDRAW_CURVE("c0", c0, Color_Red,   true);
  m_plotter.REDRAW_CURVE("c1", c1, Color_Red,   true);
  m_plotter.REDRAW_CURVE("b0", b0, Color_Green, true);
  m_plotter.REDRAW_CURVE("b1", b1, Color_Green, true);

  return true;
}
