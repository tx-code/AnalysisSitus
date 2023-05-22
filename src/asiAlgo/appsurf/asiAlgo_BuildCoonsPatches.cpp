//-----------------------------------------------------------------------------
// Created on: 15 May 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Elizaveta Krylova
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
#include <asiAlgo_BuildCoonsPatches.h>

// OCC include
#include <Geom_TrimmedCurve.hxx>

#include <asiAlgo_Utils.h>
#include <GeomConvert.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <ShapeAnalysis_Curve.hxx>

// Tigl include
#include "CTiglInterpolateCurveNetwork.h"
#include "asiAlgo_IntersectCurves.h"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_QuasiUniformAbscissa.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"

#ifdef USE_MOBIUS
#include <mobius/bspl_UnifyKnots.h>
#include <mobius/cascade.h>
#include <mobius/geom_BSplineCurve.h>
#include <mobius/geom_CoonsSurfaceCubic.h>
#include <mobius/geom_CoonsSurfaceLinear.h>
#include <mobius/geom_MakeBicubicBSurf.h>
#include <mobius/geom_SkinSurface.h>
#include <mobius/geom_UnifyBCurves.h>
#include <mobius/geom_InterpolateMultiCurve.h>

  using namespace mobius;
#endif

void GetPointGrid(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
  const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
  const math_Matrix&                            intersection_params_u,
  const math_Matrix&                            intersection_params_v,
  std::vector< std::vector<t_xyz> >&            points)
{
  (void) intersection_params_v;

  /* We can evaluate only U */

  for ( int c = 0; c < intersection_params_u.ColNumber(); ++c )
  {
    std::vector<t_xyz> row;

    for ( int r = 0; r < intersection_params_u.RowNumber(); ++r )
    {
      const double u = intersection_params_u(r, c);
      const gp_Pnt P = uCurves[r]->Value(u);

      row.push_back( cascade::GetMobiusPnt(P) );
    }
    //
    points.push_back(row);
  }
}

//-----------------------------------------------------------------------------

  asiAlgo_BuildCoonsPatches::asiAlgo_BuildCoonsPatches(ActAPI_ProgressEntry progress,
                                                       ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm(progress, plotter)
{
}

bool asiAlgo_BuildCoonsPatches::Build(const std::vector<TopoDS_Edge>& profiles,
                                      const std::vector<TopoDS_Edge>& guides,
                                      Handle(Geom_BSplineSurface)&    support,
                                      TopoDS_Face&                    face)
{
#ifdef USE_MOBIUS
  if ( profiles.empty() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "No profile curves.");
    return false;
  }

  if ( guides.empty() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "No guide curves.");
    return false;
  }

  /* =============
  *  Preparation.
  * ============= */

  // Get curves out of edges.
  std::vector<Handle(Geom_BSplineCurve)> profileCurves, guideCurves;
  //
  for ( const auto& edge : profiles )
  {
    double f, l;
    Handle(Geom_Curve) C = BRep_Tool::Curve(edge, f, l);

    if ( !asiAlgo_Utils::IsTypeOf<Geom_BSplineCurve>(edge) )
    {
      if ( !C->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
        C = new Geom_TrimmedCurve(C, f, l);
    }

    profileCurves.push_back( GeomConvert::CurveToBSplineCurve(C) );
  }
  //
  for ( const auto& edge : guides )
  {
    double f, l;
    Handle(Geom_Curve) C = BRep_Tool::Curve(edge, f, l);

    if ( !asiAlgo_Utils::IsTypeOf<Geom_BSplineCurve>(edge) )
    {
      if ( !C->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
        C = new Geom_TrimmedCurve(C, f, l);
    }

    guideCurves.push_back( GeomConvert::CurveToBSplineCurve(C) );
  }
  /* =========================================
  *  Make curves compatible by their degrees.
  * ========================================= */

  // Find max used degree in U, V.
  int p = 3, q = 3; // Min desired degrees.
  for ( const auto& C : profileCurves )
  {
    p = Max( p, C->Degree() );
  }
  for ( const auto& C : guideCurves )
  {
    q = Max( q, C->Degree() );
  }

  // Elevate degrees with OpenCascade as Mobius cannot do this (March 2023).
  for ( auto& C : profileCurves )
  {
    C->IncreaseDegree(p);
  }
  for ( auto& C : guideCurves )
  {
    C->IncreaseDegree(q);
  }

  /* ============================================
  *  Make curves compatible by their directions.
  * ============================================ */

  int  numIsoU = (int) ( profileCurves.size() );
  int  numIsoV = (int) ( guideCurves.size() );
  int  s       = 0;
  int  j       = 0;
  bool stop    = false;

  // Orient profiles.
  do
  {
    ++j;
    s = j - 1;

    if ( j < numIsoU )
    {
      gp_Pnt Ps, Pj;
      gp_Vec Vs, Vj;

      profileCurves[s]->D1( 0.5*( profileCurves[s]->FirstParameter() + profileCurves[s]->LastParameter() ), Ps, Vs );
      profileCurves[j]->D1( 0.5*( profileCurves[j]->FirstParameter() + profileCurves[j]->LastParameter() ), Pj, Vj );

      const double dot = Vj.Dot(Vs);

      if ( dot < 0 )
        profileCurves[j]->Reverse();
    }
    else
    {
      stop = true;
    }
  }
  while ( !stop );

  // Orient guides.
  s    = 0;
  j    = 0;
  stop = false;
  do
  {
    ++j;
    s = j - 1;

    if ( j < numIsoV )
    {
      gp_Pnt Ps, Pj;
      gp_Vec Vs, Vj;

      guideCurves[s]->D1( 0.5*( guideCurves[s]->FirstParameter() + guideCurves[s]->LastParameter() ), Ps, Vs );
      guideCurves[j]->D1( 0.5*( guideCurves[j]->FirstParameter() + guideCurves[j]->LastParameter() ), Pj, Vj );

      const double dot = Vj.Dot(Vs);

      if ( dot < 0 )
        guideCurves[j]->Reverse();
    }
    else
    {
      stop = true;
    }
  }
  while ( !stop );

  /* ====================================
  *  Check origin of profiles vs guides.
  * ==================================== */

  std::vector< std::pair<unsigned, unsigned> >
    oris = { {0u, 0u},
      {0u, 1u},
      {1u, 0u},
      {1u, 1u} };

  bool   syncStop    = false;
  size_t syncAttempt = 0;
  do
  {
    std::vector<Handle(Geom_BSplineCurve)> _profileCurves;
    std::vector<Handle(Geom_BSplineCurve)> _guideCurves;

    // Reverse profiles if the flag is true.
    if ( oris[syncAttempt].first )
    {
      for ( const auto& C : profileCurves )
      {
        _profileCurves.push_back( Handle(Geom_BSplineCurve)::DownCast( C->Reversed() ) );
      }
    }
    else
    {
      _profileCurves = profileCurves;
    }

    // Reverse guides if the flag is true.
    if ( oris[syncAttempt].second )
    {
      for ( const auto& C : guideCurves )
      {
        _guideCurves.push_back( Handle(Geom_BSplineCurve)::DownCast( C->Reversed() ) );
      }
    }
    else
    {
      _guideCurves = guideCurves;
    }

    const gp_Pnt OP = _profileCurves[0]->Value( _profileCurves[0] ->FirstParameter() );
    const gp_Pnt OG = _guideCurves  [0]->Value( _guideCurves[0]   ->FirstParameter() );
    //
    m_plotter.REDRAW_POINT("OP", OP, Color_Red);
    m_plotter.REDRAW_POINT("GP", OG, Color_Green);

    if ( OP.Distance(OG) < 1e-2 )
    {
      syncStop      = true;
      profileCurves = _profileCurves;
      guideCurves   = _guideCurves;
    }

    if ( ++syncAttempt > 3 )
    {
      syncStop = true;
    }
  }
  while ( !syncStop );

  /* ======================
  *  Reapproximate curves.
  * ====================== */

  std::vector<double> uParams, uKnots, vParams, vKnots;
  //
  this->reapproxCurves(profileCurves, profileCurves, uParams, uKnots);
  this->reapproxCurves(guideCurves,   guideCurves,   vParams, vKnots);

  /* ====================================
  *  Find curve intersection parameters.
  * ==================================== */

  math_Matrix intersection_params_u(0, numIsoU - 1,
    0, numIsoV - 1);
  math_Matrix intersection_params_v(0, numIsoU - 1,
    0, numIsoV - 1);

  /// Experimental
  {
    math_Matrix intersection_params_u2(0, numIsoU - 1,
      0, numIsoV - 1);
    math_Matrix intersection_params_v2(0, numIsoU - 1,
      0, numIsoV - 1);

    asiAlgo_IntersectCurves ccInt(m_progress, m_plotter);
    ccInt.Perform(profileCurves, guideCurves,
      intersection_params_u2,
      intersection_params_v2);

    intersection_params_u2.Dump(std::cout);
    intersection_params_v2.Dump(std::cout);


    intersection_params_u = intersection_params_u2;
    intersection_params_v = intersection_params_v2;
  }

  //return false;

  /* ===========================================
  *  Collect curve network intersection points.
  * =========================================== */

  std::vector< std::vector<t_xyz> > pointGrid;

  ::GetPointGrid(profileCurves, guideCurves, intersection_params_u, intersection_params_v, pointGrid);

  // Draw.
  std::vector<gp_Pnt> pnts;
  {
    Handle(asiAlgo_BaseCloud<double>) __pts = new asiAlgo_BaseCloud<double>;
    //
    for ( const auto& row : pointGrid )
      for ( const auto& pt : row )
      {
        pnts.push_back({pt.X(), pt.Y(), pt.Z()});
        __pts->AddElement( pt.X(), pt.Y(), pt.Z() );/*
        m_plotter.DRAW_POINT( gp_Pnt( pt.X(), pt.Y(), pt.Z() ), Color_Yellow, "pt" );*/
      }

    m_plotter.DRAW_POINTS( __pts->GetCoordsArray(), Color_Yellow, "points" );
  }
  //
  /* =============
  *  Trim Curves.
  * ============= */
  std::vector<Handle(Geom_Curve)> uTrimmedCurves;
  std::vector<Handle(Geom_Curve)> vTrimmedCurves;

  if (!computeCurvesFromIntersections(profileCurves,
                                      guideCurves,
                                      uTrimmedCurves,
                                      vTrimmedCurves,
                                      pnts))
  {
    return false;
  }

  std::vector<TopoDS_Edge> uEdges;
  std::vector<TopoDS_Edge> vEdges;
  for (auto& i : uTrimmedCurves)
  {
    auto edge = BRepBuilderAPI_MakeEdge(i);
    uEdges.push_back(edge);
    m_plotter.DRAW_SHAPE(edge, Color_Red, "uEdge" );
  }
  for (auto& i : vTrimmedCurves)
  {
    auto edge = BRepBuilderAPI_MakeEdge(i);
    vEdges.push_back(edge);
    m_plotter.DRAW_SHAPE(edge, Color_Red, "vEdge" );
  }
#endif
}

bool asiAlgo_BuildCoonsPatches::reapproxCurves(const std::vector<Handle(Geom_BSplineCurve)>& curves,
  std::vector<Handle(Geom_BSplineCurve)>&       result,
  std::vector<double>&                          params,
  std::vector<double>&                          knots) const
{
#ifdef USE_MOBIUS
  /* =====================================================
  *  Discretize curves to have the same number of points.
  * ===================================================== */

  const int numPts = 100;

  // Get max degree.
  int degree = 3;
  for ( const auto& C : curves )
  {
    degree = Max( degree, C->Degree() );
  }

  // Prepare interpolation tool.
  geom_InterpolateMultiCurve interpTool(degree,
    ParamsSelection_ChordLength,
    KnotsSelection_Average);

  for ( const auto& C : curves )
  {
    // Discretize with a uniform curvilinear step.
    GeomAdaptor_Curve gac(C);
    GCPnts_QuasiUniformAbscissa Defl(gac, numPts);
    //
    if ( !Defl.IsDone() )
      return false;

    // Fill row of points.
    std::vector<t_xyz> ptsRow;
    //
    for ( int i = 1; i <= numPts; ++i )
    {
      const double param = Defl.Parameter(i);
      t_xyz P = cascade::GetMobiusPnt( C->Value(param) );
      //
      ptsRow.push_back(P);
    }

    // Add points to the interpolation tool.
    interpTool.AddRow(ptsRow);
  }

  /* ========================
  *  Interpolate multicurve.
  * ======================== */

  if ( !interpTool.Perform() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Multicurve interpolation failed.");
    return false;
  }

  result.clear(); // in case it's the same as the input

  params = interpTool.GetResultParams();
  knots  = interpTool.GetResultKnots();

  for ( int k = 0; k < interpTool.GetNumRows(); ++k )
  {
    Handle(Geom_BSplineCurve)
      resCurve = cascade::GetOpenCascadeBCurve( interpTool.GetResult(k) );

    result.push_back(resCurve);
  }

  return true;
#else
  (void)curves;
  (void)result;
  (void)params;
  (void)knots;
  return false;
#endif
}

bool asiAlgo_BuildCoonsPatches::computeCurvesFromIntersections(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
                                                               const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
                                                               std::vector<Handle(Geom_Curve)>&              uTrimmedCurves,
                                                               std::vector<Handle(Geom_Curve)>&              vTrimmedCurves,
                                                               const std::vector<gp_Pnt>&                    points) const
{
  ShapeAnalysis_Curve sac;
  for (auto& curve : uCurves)
  {
    double uPrev = 0;
    bool isFirst = true;
    for (auto& pnt : points)
    {
      gp_Pnt proj;
      double param;
      sac.Project(curve, pnt, Precision::Confusion(), proj, param);
      if (pnt.IsEqual(proj, 0.001))
      {
        if (isFirst || uPrev == param)
        {
          isFirst = false;
          uPrev = param;
          continue;
        }
        try
        {
          Handle(Geom_Curve) uTrimmedCurve = new Geom_TrimmedCurve(curve, uPrev, param);
          uPrev = param;
          uTrimmedCurves.emplace_back(uTrimmedCurve);
        } catch (...)
        {
          return false;
        }
      }
    }
  }
  for (auto& curve : vCurves)
  {
    double uPrev = 0;
    bool isFirst = true;
    for (auto& pnt : points)
    {
      gp_Pnt proj;
      double param;
      sac.Project(curve, pnt, Precision::Confusion(), proj, param);
      if (pnt.IsEqual(proj, 0.001))
      {
        if (isFirst || uPrev == param)
        {
          isFirst = false;
          uPrev = param;
          continue;
        }
        try
        {
          Handle(Geom_Curve) TrimmedCurve = new Geom_TrimmedCurve(curve, uPrev, param);
          uPrev = param;
          vTrimmedCurves.emplace_back(TrimmedCurve);
        } catch (...)
        {
          return false;
        }
      }
    }
  }
  return true;
}
