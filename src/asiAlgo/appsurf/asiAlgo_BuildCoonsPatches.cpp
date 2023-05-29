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
#include <asiAlgo_AAG.h>
#include <asiAlgo_UntrimSurf.h>
#include <asiAlgo_BuildCoonsSurf.h>

// Tigl include
#include "CTiglInterpolateCurveNetwork.h"
#include "asiAlgo_IntersectCurves.h"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_QuasiUniformAbscissa.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRep_Builder.hxx"
#include "CTiglBSplineAlgorithms.h"

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

bool asiAlgo_BuildCoonsPatches::Build(const std::vector<TopoDS_Edge>& profiles,
                                      const std::vector<TopoDS_Edge>& guides,
                                      std::vector<TopoDS_Edge>&       profileEdges,
                                      std::vector<TopoDS_Edge>&       guidesEdges)
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
        __pts->AddElement( pt.X(), pt.Y(), pt.Z() );
      }

    m_plotter.DRAW_POINTS( __pts->GetCoordsArray(), Color_Yellow, "points" );
  }

  /* =============
  *  Trim Curves.
  * ============= */

  std::vector<Handle(Geom_Curve)> uTrimmedCurves;
  std::vector<Handle(Geom_Curve)> vTrimmedCurves;

  if (!computeCurvesFromIntersections(profileCurves,
                                      guideCurves,
                                      intersection_params_u,
                                      intersection_params_v,
                                      uTrimmedCurves,
                                      vTrimmedCurves))
  {
    return false;
  }

  TopTools_IndexedMapOfShape edges;
  for (const auto& i : uTrimmedCurves)
  {
    auto edge = BRepBuilderAPI_MakeEdge(i);
    profileEdges.push_back(edge);
   // m_plotter.DRAW_SHAPE(edge, Color_Red, "u");
  }
  for (const auto& i : vTrimmedCurves)
  {
    auto edge = BRepBuilderAPI_MakeEdge(i);
    guidesEdges.push_back(edge);
   // m_plotter.DRAW_SHAPE(edge, Color_Red, "v" );
  }

  if (!concatEdgesIntoPatches(uTrimmedCurves, vTrimmedCurves))
  {
    return false;
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

//-----------------------------------------------------------------------------

bool asiAlgo_BuildCoonsPatches::computeCurvesFromIntersections(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
                                                               const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
                                                               const math_Matrix&                            uParams,
                                                               const math_Matrix&                            vParams,
                                                               std::vector<Handle(Geom_Curve)>&              uTrimmedCurves,
                                                               std::vector<Handle(Geom_Curve)>&              vTrimmedCurves) const
{
  for (int row = 0; row < uParams.RowNumber(); ++row)
  {
    double prev = 0;
    bool isFirst = true;
    for (int col = 0; col < (int) uParams.ColNumber(); ++col)
    {
      auto param = uParams(row, col);
      if (isFirst)
      {
        prev = param;
        isFirst = false;
        continue;
      }
      if (abs(param - prev) < 0.01)
      {
        continue;
      }
      try
      {
        auto uTrimmedCurve = GeomConvert::SplitBSplineCurve(uCurves[row], prev, param, 0.001, true);
        prev = param;
        uTrimmedCurves.emplace_back(uTrimmedCurve);
      } catch (...)
      {
        continue;
      }
    }
  }

  for (int col = 0; col < (int) vParams.ColNumber(); ++col)
  {
    double prev = 0;
    bool isFirst = true;
    for (int row = 0; row < vParams.RowNumber(); ++row)
    {
      auto param = vParams(row, col);
      if (isFirst)
      {
        prev = param;
        isFirst = false;
        continue;
      }
      if (abs(param - prev) < 0.01)
      {
        continue;
      }
      try
      {
        auto vTrimmedCurve = GeomConvert::SplitBSplineCurve(vCurves[col], prev, param, 0.001, true);
        prev = param;
        vTrimmedCurves.emplace_back(vTrimmedCurve);
      } catch (...)
      {
        continue;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildCoonsPatches::concatEdgesIntoPatches(std::vector<Handle(Geom_Curve)>& uEdge,
                                                       std::vector<Handle(Geom_Curve)>& vEdge) const
{
  std::vector<std::vector<Handle(Geom_Curve)>> patches;
  for (auto& ue : uEdge)
  {
    auto fVertU = ue->Value(ue->FirstParameter());
    auto lVertU = ue->Value(ue->LastParameter());
    std::vector<Handle(Geom_Curve)> commonCurve;
    for (auto& ve : vEdge)
    {
      auto fVertV = ve->Value(ve->FirstParameter());
      auto lVertV = ve->Value(ve->FirstParameter());
      if (fVertU.IsEqual(fVertV, 0.001) || fVertU.IsEqual(lVertV, 0.001) ||
          lVertU.IsEqual(fVertV, 0.001) || lVertU.IsEqual(lVertV, 0.001))
      {
        commonCurve.emplace_back(ve);
      }
    }
    if (commonCurve.empty())
    {
      continue;
    }
    for (size_t i = 0; i < commonCurve.size() - 1; ++i)
    {
      for (size_t j = i + 1; j < commonCurve.size(); ++j)
      {
        Handle(Geom_Curve) resEdge;
        if (findCommonEdge(commonCurve[i], commonCurve[j], ue, uEdge, resEdge))
        {
          std::vector<Handle(Geom_Curve)> patch;
          //
          Handle(TopTools_HSequenceOfShape) patchS = new TopTools_HSequenceOfShape;
          patchS->Append(BRepBuilderAPI_MakeEdge(commonCurve[i]).Edge());
          patchS->Append(BRepBuilderAPI_MakeEdge(commonCurve[j]).Edge());
          patchS->Append(BRepBuilderAPI_MakeEdge(ue).Edge());
          patchS->Append(BRepBuilderAPI_MakeEdge(resEdge).Edge());

          Handle(Geom_BSplineCurve) c0, c1, b0, b1;
          //
          asiAlgo_UntrimSurf surf;
          if ( !surf.sortEdges(patchS, c0, c1, b0, b1) )
          {
            continue;
          }
          tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*c0, 0., 1., 0.0001);
          tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*c1, 0., 1., 0.0001);
          tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*b0, 0., 1., 0.0001);
          tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*b1, 0., 1., 0.0001);
          c0->IncreaseDegree(3);
          c1->IncreaseDegree(3);
          b0->IncreaseDegree(3);
          b1->IncreaseDegree(3);
          //
          patch.emplace_back(c0);
          patch.emplace_back(c1);
          patch.emplace_back(b0);
          patch.emplace_back(b1);
          //
          return true;
          //
          patches.emplace_back(patch);
          //
          asiAlgo_BuildCoonsSurf coons(c0, c1, b0, b1, m_progress, m_plotter);
          try {
            if (!coons.Perform())
            {
              continue;
            }
            auto surface = coons.GetResult();
            m_plotter.DRAW_SURFACE(surface, Color_Green, "vEdge0" );

            m_plotter.DRAW_CURVE(c0, Color_Red, "vEdge0" );
            m_plotter.DRAW_CURVE(c1, Color_Red, "vEdge1" );
            m_plotter.DRAW_CURVE(b0, Color_Red, "vEdge0" );
            m_plotter.DRAW_CURVE(b1, Color_Red, "vEdge1" );
          } catch (...) {
            continue;
          }
        }
      }
    }
  }
  return true;
}

bool asiAlgo_BuildCoonsPatches::findCommonEdge(const Handle(Geom_Curve)&        v1,
                                               const Handle(Geom_Curve)&        v2,
                                               const Handle(Geom_Curve)&        uEdge,
                                               std::vector<Handle(Geom_Curve)>& uEdges,
                                               Handle(Geom_Curve)&              resEdge) const
{
  auto fVertV1 = v1->Value(v1->FirstParameter());
  auto lVertV1 = v1->Value(v1->LastParameter());
  //
  auto fVertV2 = v2->Value(v2->FirstParameter());
  auto lVertV2 = v2->Value(v2->LastParameter());
  for (auto& ue : uEdges)
  {
    if (uEdge == ue)
    {
      continue;
    }
    auto fVertU = ue->Value(ue->FirstParameter());
    auto lVertU = ue->Value(ue->LastParameter());
    if ((fVertU.IsEqual(fVertV1, 0.001) || fVertU.IsEqual(lVertV1, 0.001) ||
      lVertU.IsEqual(fVertV1, 0.001) || lVertU.IsEqual(lVertV1, 0.001)) &&
      (fVertU.IsEqual(fVertV2, 0.001) || fVertU.IsEqual(lVertV2, 0.001) ||
        lVertU.IsEqual(fVertV2, 0.001) || lVertU.IsEqual(lVertV2, 0.001)))
    {
      resEdge = ue;
      return true;
    }
  }
  return false;
}
