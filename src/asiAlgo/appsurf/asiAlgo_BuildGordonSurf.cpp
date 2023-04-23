//-----------------------------------------------------------------------------
// Created on: 29 March 2023
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
#include "asiAlgo_BuildGordonSurf.h"

// asiAlgo includes
#include <asiAlgo_AppSurfUtils.h>
#include <asiAlgo_Utils.h>

// Mobius includes
#include <mobius/bspl_UnifyKnots.h>
#include <mobius/cascade.h>
#include <mobius/geom_InterpolateCurve.h>
#include <mobius/geom_InterpolateMultiCurve.h>
#include <mobius/geom_SkinSurface.h>
#include <mobius/geom_UnifyBCurves.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <GeomConvert.hxx>
#include <Geom_BSplineCurve.hxx>

#include <CTiglInterpolateCurveNetwork.h>
#include <CTiglBSplineAlgorithms.h>
#include <CTiglCurveNetworkSorter.h>

using namespace mobius;

#undef USE_TIGL

#define CurveOriginToler 1e-2

namespace
{
  template <class T>
  T Clamp(T val, T min, T max)
  {
    if (min > max) {
      throw Standard_ProgramError("Minimum may not be larger than maximum in clamp!");
    }
    return std::max(min, std::min(val, max));
  }

  size_t Clamp(size_t val, size_t min, size_t max)
  {
    return Clamp<>(val, min, max);
  }

  //! Computes the default edge discretization precision for the passed shape.
  double ComputeDefaultPrec(const TopoDS_Shape& shape)
  {
    double xMin, yMin, zMin, xMax, yMax, zMax;
    if ( asiAlgo_Utils::Bounds(shape, xMin, yMin, zMin, xMax, yMax, zMax, false) )
    {
      double d = gp_Pnt(xMin, yMin, zMin).Distance( gp_Pnt(xMax, yMax, zMax) );
      d *= 0.05; // some percentage of AABB diagonal
      //
      return d;
    }

    return 0;
  }

  //! Polygonizes the passed edges with the passed tolerance taken as
  //! a uniform step over the corresponding curves.
  void DiscretizeEdges(const std::vector<TopoDS_Edge>&    edges,
                       const double                       tol,
                       Handle(asiAlgo_BaseCloud<double>)& pts)
  {
    pts = new asiAlgo_BaseCloud<double>;

    // Iterate over edges to add point constraints.
    for ( const auto& edge : edges )
    {
      BRepAdaptor_Curve curve(edge);
      //
      const double f = curve.FirstParameter();
      const double l = curve.LastParameter();

      GCPnts_UniformAbscissa splitter( curve, tol, Precision::Confusion() );
      int pointCount = splitter.IsDone() ? splitter.NbPoints() : 2;
      std::vector<double> params;

      params.push_back(f);
      for ( int j = 2; j < pointCount; ++j )
      {
        double param = splitter.Parameter(j);
        params.push_back(param);
      }
      params.push_back(l);

      for ( auto p : params )
      {
        pts->AddElement( curve.Value(p) );
      }
    }
  }

  //! Interpolates surfaces with the given props.
  bool InterpolateSurf(const std::vector< std::vector<t_xyz> >& points,
                       const int                                degU,
                       const int                                degV,
                       const std::vector<double>&               uParams,
                       const std::vector<double>&               U,
                       const std::vector<double>&               vParams,
                       const std::vector<double>&               V,
                       t_ptr<t_bsurf>&                          result)
  {
    // Heap allocator
    core_HeapAlloc<double> Alloc;

    double* uParamsRaw = Alloc.Allocate( uParams.size() );
    //
    for ( size_t k = 0; k < uParams.size(); ++k )
      uParamsRaw[k] = uParams[k];

    double* URaw = Alloc.Allocate( U.size() );
    //
    for ( size_t k = 0; k < U.size(); ++k )
      URaw[k] = U[k];

    double* vParamsRaw = Alloc.Allocate( vParams.size() );
    //
    for ( size_t k = 0; k < vParams.size(); ++k )
      vParamsRaw[k] = vParams[k];

    double* VRaw = Alloc.Allocate( V.size() );
    //
    for ( size_t k = 0; k < V.size(); ++k )
      VRaw[k] = V[k];

    /* ---------------------------------------
     *  Choose reper (interpolant) parameters
     * --------------------------------------- */

    if ( points.size() < 2 )
    {
      return false;
    }

    // Check if the passed grid is rectangular
    size_t record_size = points[0].size();
    if ( record_size < 2 )
    {
      return false;
    }
    for ( size_t record_idx = 1; record_idx < points.size(); ++record_idx )
    {
      if ( points[record_idx].size() != record_size )
      {
        return false;
      }
    }

    // Dimensions of reper grid
    const int n = (int) (points.size() - 1);
    const int m = (int) (points.at(0).size() - 1);

    const int r = bspl::M(n, degU);
    const int s = bspl::M(m, degV);

    // Check if there are enough reper points
    if ( !bspl::Check(n, degU) || !bspl::Check(m, degV) )
      return false;

    std::vector< t_ptr<t_bcurve> > IsoV_Curves;
    std::vector< t_ptr<t_bcurve> > ReperU_Curves;

    /* ---------------------------------------------
     *  Find R_{i,j} by interpolation of V-isolines
     * --------------------------------------------- */

    for ( int l = 0; l <= m; ++l )
    {
      // Populate reper points for fixed V values
      std::vector<t_xyz> iso_V_poles;
      for ( int k = 0; k <= n; ++k )
        iso_V_poles.push_back(points[k][l]);

      t_xyz D1_start;
      t_xyz D1_end;
      t_xyz D2_start;
      t_xyz D2_end;

      // Interpolate over these cross-sections only
      t_ptr<t_bcurve> iso_V;
      if ( !geom_InterpolateCurve::Interp(iso_V_poles, n, degU, uParamsRaw, URaw, r,
                                          false,
                                          false,
                                          false,
                                          false,
                                          D1_start,
                                          D1_end,
                                          D2_start,
                                          D2_end,
                                          iso_V) )
      {
        return false;
      }
      IsoV_Curves.push_back(iso_V);
    }

    /* ------------------------------------------
     *  Find P_{i,j} by interpolation of R_{i,j}
     * ------------------------------------------ */

    // Poles of interpolant
    std::vector< std::vector<t_xyz> > final_poles;

    // Interpolate by new repers
    ReperU_Curves.clear();
    const int corrected_n = n;

    for ( int k = 0; k <= corrected_n; ++k )
    {
      // Populate reper points: we use the control points of V-isocurves
      // as reper points now
      std::vector<t_xyz> R_poles;
      for ( int l = 0; l <= m; ++l )
        R_poles.push_back(IsoV_Curves[l]->GetPoles()[k]);

      // Interpolate again
      t_ptr<t_bcurve> R_interp;
      if ( !geom_InterpolateCurve::Interp(R_poles, m, degV, vParamsRaw, VRaw, s,
                                          false, false, false, false,
                                          t_xyz(), t_xyz(), t_xyz(), t_xyz(),
                                          R_interp) )
      {
        return false;
      }
      ReperU_Curves.push_back(R_interp);

      // Poles in V column of the resulting grid
      std::vector<t_xyz> V_column_poles;
      for ( int p = 0; p <= m; ++p )
        V_column_poles.push_back(R_interp->GetPoles()[p]);

      // Save to resulting grid
      final_poles.push_back(V_column_poles);
    }

    /* -----------------------
     *  Construct interpolant
     * ----------------------- */

    result = new t_bsurf(final_poles,
                         URaw, VRaw,
                         r + 1, s + 1,
                         degU, degV);

    return true;
  }

  //! Extracts points from the passed curve network.
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
}

//-----------------------------------------------------------------------------

void
  asiAlgo_BuildGordonSurf::CheckDeviation(const Handle(Geom_BSplineSurface)& surf,
                                          const std::vector<TopoDS_Edge>&    uEdges,
                                          const std::vector<TopoDS_Edge>&    vEdges,
                                          double&                            bndDev,
                                          double&                            innerDev,
                                          double&                            maxDev,
                                          ActAPI_PlotterEntry                plotter)
{
  // Put all edges into a compound to compute the default deviation.
  TopoDS_Compound comp;
  BRep_Builder bbuilder;
  bbuilder.MakeCompound(comp);
  //
  for ( const auto& edge : uEdges )
    bbuilder.Add(comp, edge);
  //
  for ( const auto& edge : vEdges )
    bbuilder.Add(comp, edge);

  const double tol = ::ComputeDefaultPrec(comp);

  // Collect boundary edges.
  std::vector<TopoDS_Edge> bndEdges;
  //
  bndEdges.push_back( uEdges.front() );
  bndEdges.push_back( uEdges.back() );
  bndEdges.push_back( vEdges.front() );
  bndEdges.push_back( vEdges.back() );

  // Collect inner edges.
  std::vector<TopoDS_Edge> innerEdges;
  //
  for ( size_t i = 1; i < uEdges.size() - 1; ++i )
    innerEdges.push_back( uEdges[i] );
  //
  for ( size_t i = 1; i < vEdges.size() - 1; ++i )
    innerEdges.push_back( vEdges[i] );

  Handle(asiAlgo_BaseCloud<double>) bndPts, innerPts;
  //
  ::DiscretizeEdges(bndEdges,   tol, bndPts);
  ::DiscretizeEdges(innerEdges, tol, innerPts);

  gp_Pnt bndMaxDevPt, innerMaxDevPt;
  double bndDevs[3], innerDevs[3];
  asiAlgo_AppSurfUtils::MeasureDeviation(surf, bndPts,   bndDevs[0],   bndDevs[1],   bndDevs[2],   bndMaxDevPt);
  asiAlgo_AppSurfUtils::MeasureDeviation(surf, innerPts, innerDevs[0], innerDevs[1], innerDevs[2], innerMaxDevPt);

  bndDev   = bndDevs[1];
  innerDev = innerDevs[1];
  maxDev   = Max(bndDev, innerDev);

  plotter.REDRAW_POINT("bndMaxDevPt",   bndMaxDevPt,   Color_Red);
  plotter.REDRAW_POINT("innerMaxDevPt", innerMaxDevPt, Color_Violet);
}

//-----------------------------------------------------------------------------

asiAlgo_BuildGordonSurf::asiAlgo_BuildGordonSurf(ActAPI_ProgressEntry progress,
                                                 ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_fMaxError       (0.)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildGordonSurf::Build(const std::vector<TopoDS_Edge>& profiles,
                                    const std::vector<TopoDS_Edge>& guides,
                                    Handle(Geom_BSplineSurface)&    support,
                                    TopoDS_Face&                    face)
{
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

    if ( OP.Distance(OG) < CurveOriginToler )
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

  this->computeCurveIntersections(profileCurves, guideCurves,
                                  intersection_params_u,
                                  intersection_params_v);

  intersection_params_u.Dump(std::cout);
  intersection_params_v.Dump(std::cout);

  /* ===========================================
   *  Collect curve network intersection points.
   * =========================================== */

  std::vector< std::vector<t_xyz> > pointGrid;

  ::GetPointGrid(profileCurves, guideCurves, intersection_params_u, intersection_params_v, pointGrid);

  // Draw.
  {
    Handle(asiAlgo_BaseCloud<double>) __pts = new asiAlgo_BaseCloud<double>;
    //
    for ( const auto& row : pointGrid )
      for ( const auto& pt : row )
      {
        __pts->AddElement( pt.X(), pt.Y(), pt.Z() );/*
        m_plotter.DRAW_POINT( gp_Pnt( pt.X(), pt.Y(), pt.Z() ), Color_Yellow, "pt" );*/
      }

    m_plotter.DRAW_POINTS( __pts->GetCoordsArray(), Color_Yellow, "points" );
  }

  /* =======================
   *  Sort curves spatially.
   * ======================= */

  std::vector<double> newParametersProfiles;
  std::vector<double> newParametersGuides;

  {
    const int nGuides   = numIsoV;
    const int nProfiles = numIsoU;

    for ( int spline_v_idx = 1; spline_v_idx <= nGuides; ++spline_v_idx )
    {
      double sum = 0;
      for ( int spline_u_idx = 1; spline_u_idx <= nProfiles; ++spline_u_idx )
      {
        sum += intersection_params_u(spline_u_idx - 1, spline_v_idx - 1);
      }
      newParametersProfiles.push_back(sum / nProfiles);
    }

    for ( int spline_u_idx = 1; spline_u_idx <= nProfiles; ++spline_u_idx )
    {
      double sum = 0;
      for ( int spline_v_idx = 1; spline_v_idx <= nGuides; ++spline_v_idx )
      {
        sum += intersection_params_v(spline_u_idx - 1, spline_v_idx - 1);
      }
      newParametersGuides.push_back(sum / nGuides);
    }

    if ( newParametersProfiles.front() > 1e-4 || newParametersGuides.front() > 1e-4 )
    {
      this->AddStatusFlag(Status_InconsistentOrientationOfCurves);
      return false;
    }

    // Get maximum number of control points to figure out detail of spline
    size_t max_cp_u = 0, max_cp_v = 0;
    for ( const auto& C : profileCurves )
    {
      max_cp_u = std::max( max_cp_u, static_cast<size_t>( C->NbPoles() ) );
    }
    for ( const auto& C : guideCurves )
    {
      max_cp_v = std::max( max_cp_v, static_cast<size_t>( C->NbPoles() ) );
    }

    // we want to use at least 10 and max 80 control points to be able to reparametrize the geometry properly
    size_t mincp = 10;
    size_t maxcp = 80;

    // since we interpolate the intersections, we cannot use fewer control points than curves
    // We need to add two since we want c2 continuity, which adds two equations
    size_t min_u = std::max(guideCurves.size() + 2, mincp);
    size_t min_v = std::max(profileCurves.size() + 2, mincp);

    size_t max_u = std::max(min_u, maxcp);
    size_t max_v = std::max(min_v, maxcp);

    max_cp_u = Clamp(max_cp_u + 10, min_u, max_u);
    max_cp_v = Clamp(max_cp_v + 10, min_v, max_v);

    // reparametrize u-directional B-splines
    for (int spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx) {

        std::vector<double> oldParametersProfile;
        for (int spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx) {
            oldParametersProfile.push_back(intersection_params_u(spline_u_idx, spline_v_idx));
        }

        // eliminate small inaccuracies at the first knot
        if (std::abs(oldParametersProfile.front()) < 1e-4) {
            oldParametersProfile.front() = 0;
        }

        if (std::abs(newParametersProfiles.front()) < 1e-4) {
            newParametersProfiles.front() = 0;
        }

        // eliminate small inaccuracies at the last knot
        if (std::abs(oldParametersProfile.back() - 1) < 1e-4) {
            oldParametersProfile.back() = 1;
        }

        if (std::abs(newParametersProfiles.back() - 1) < 1e-4) {
            newParametersProfiles.back() = 1;
        }

        Handle(Geom_BSplineCurve)& profile = profileCurves[static_cast<size_t>(spline_u_idx)];
        profile = tigl::CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(profile, oldParametersProfile, newParametersProfiles, max_cp_u).curve;
    }

    // reparametrize v-directional B-splines
    for (int spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx) {

        std::vector<double> oldParameterGuide;
        for (int spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx) {
            oldParameterGuide.push_back(intersection_params_v(spline_u_idx, spline_v_idx));
        }

        // eliminate small inaccuracies at the first knot
        if (std::abs(oldParameterGuide.front()) < 1e-5) {
            oldParameterGuide.front() = 0;
        }

        if (std::abs(newParametersGuides.front()) < 1e-5) {
            newParametersGuides.front() = 0;
        }

        // eliminate small inaccuracies at the last knot
        if (std::abs(oldParameterGuide.back() - 1) < 1e-5) {
            oldParameterGuide.back() = 1;
        }

        if (std::abs(newParametersGuides.back() - 1) < 1e-5) {
            newParametersGuides.back() = 1;
        }

        Handle(Geom_BSplineCurve)& guide = guideCurves[static_cast<size_t>(spline_v_idx)];
        guide = tigl::CTiglBSplineAlgorithms::reparametrizeBSplineContinuouslyApprox(guide, oldParameterGuide, newParametersGuides, max_cp_v).curve;
    }
  }

  for ( const auto& C : profileCurves )
    m_plotter.DRAW_CURVE(C, Color_Pink, true, "profile");

  for ( const auto& C : guideCurves )
    m_plotter.DRAW_CURVE(C, Color_Violet, true, "guide");

  /* =================
   *  Give TiGL a try.
   * ================= */

#if defined USE_TIGL
  {
    std::vector<Handle(Geom_Curve)> uCurvesTigl, vCurvesTigl;
    //
    for ( const auto c : uCurves )
      uCurvesTigl.push_back(c);
    //
    for ( const auto c : vCurves )
      vCurvesTigl.push_back(c);

    tigl::CTiglInterpolateCurveNetwork tiglGordon(uCurvesTigl, vCurvesTigl, 1e-3, m_progress, m_plotter);

    Handle(Geom_BSplineSurface) res = tiglGordon.Surface();

    m_plotter.REDRAW_SURFACE("tiglGordon", res, Color_White);

    // Check surface deviation from the curve network.
    double bndDev, innerDev, maxDev;
    //
    CheckDeviation( res,
                    uEdges, vEdges, bndDev, innerDev, maxDev,
                    m_plotter );

    m_progress.SendLogMessage(LogNotice(Normal) << "\n\tBoundary deviation: %1."
                                                   "\n\tInner deviation: %2."
                                                   "\n\tMax deviation: %3."
                                                << bndDev << innerDev << maxDev);

    return true;
  }
#endif

  /* ===================
   *  Convert to Mobius.
   * =================== */

  // Convert to Mobius form.
  std::vector< t_ptr<t_bcurve> > uCurvesMb, vCurvesMb;
  //
  for ( const auto& C : profileCurves )
  {
    t_ptr<t_bcurve> crvMb = cascade::GetMobiusBCurve(C);

    uCurvesMb.push_back(crvMb);
  }
  //
  for ( const auto& C : guideCurves )
  {
    t_ptr<t_bcurve> crvMb = cascade::GetMobiusBCurve(C);

    vCurvesMb.push_back(crvMb);
  }

  /* =======================================
   *  Make curves compatible by their knots.
   * ======================================= */

  // Unify U curves.
  //{
  //  geom_UnifyBCurves unifyCurves(nullptr, nullptr);
  //  //
  //  for ( const auto& C : uCurvesMb )
  //    unifyCurves.AddCurve(C);

  //  if ( !unifyCurves.Perform() )
  //  {
  //    m_progress.SendLogMessage(LogErr(Normal) << "Failed to unify 'U' curves.");
  //    return false;
  //  }
  //}

  //// Unify V curves.
  //{
  //  geom_UnifyBCurves unifyCurves(nullptr, nullptr);
  //  //
  //  for ( const auto& C : vCurvesMb )
  //    unifyCurves.AddCurve(C);

  //  if ( !unifyCurves.Perform() )
  //  {
  //    m_progress.SendLogMessage(LogErr(Normal) << "Failed to unify 'V' curves.");
  //    return false;
  //  }
  //}

  //// Draw unified curves.
  //for ( const auto& C : uCurvesMb )
  //{
  //  m_plotter.DRAW_CURVE( cascade::GetOpenCascadeBCurve(C), Color_Red, true, "uCurve" );
  //}
  ////
  //for ( const auto& C : vCurvesMb )
  //{
  //  m_plotter.DRAW_CURVE( cascade::GetOpenCascadeBCurve(C), Color_Red, true, "vCurve" );
  //}

  /* ===============
   *  Skin U curves.
   * =============== */

  const int degP1S = Min(3, int(uCurvesMb.size()) - 1);
  const int degP2S = Min(3, int(vCurvesMb.size()) - 1);

  // Build P1S.
  t_ptr<t_bsurf> P1S;
  Handle(Geom_BSplineSurface) P1Socc;
  {
    // Prepare rail curves.
    std::vector< t_ptr<t_bcurve> > rails = uCurvesMb;

    geom_SkinSurface skinner(rails, degP1S, false);
    //
    skinner.SetUseChordLength(false);
    skinner.ForceParameters(newParametersGuides);
   /* skinner.ForceKnots(uKnots);*/
    //
    if ( !skinner.Perform() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Cannot build P1S.");
      return false;
    }
    //
    P1S    = skinner.GetResult();
    P1Socc = cascade::GetOpenCascadeBSurface(P1S);
    //
    //P1Socc->IncreaseDegree(3, 3);
    //
    P1S = cascade::GetMobiusBSurface(P1Socc);

    uKnots  = skinner.GetResultKnots();
    uParams = skinner.GetResultParams();

    m_plotter.DRAW_SURFACE( P1Socc, Color_Default, "P1S" );
  }

  /* ===============
   *  Skin V curves.
   * =============== */

  // Build P1S.
  t_ptr<t_bsurf> P2S;
  Handle(Geom_BSplineSurface) P2Socc;
  {
    // Prepare rail curves.
    std::vector< t_ptr<t_bcurve> > rails = vCurvesMb;

    geom_SkinSurface skinner(rails, degP2S, false);
    //
    skinner.SetUseChordLength(false);
    skinner.ForceParameters(newParametersProfiles);
    //
    if ( !skinner.Perform() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Cannot build P2S.");
      return false;
    }
    //
    P2S = skinner.GetResult();
    P2S->ExchangeUV();
    P2Socc = cascade::GetOpenCascadeBSurface(P2S);
    //
    //P2Socc->IncreaseDegree(3, 3);
    //
    P2S = cascade::GetMobiusBSurface(P2Socc);

    vKnots  = skinner.GetResultKnots();
    vParams = skinner.GetResultParams();

    m_plotter.DRAW_SURFACE( P2Socc, Color_Default, "P2S" );
  }

  /* =========================================
   *  Interpolate surface over the point grid.
   * ========================================= */

  t_ptr<t_bsurf> P12S;
  //
  if ( !::InterpolateSurf(pointGrid, degP2S, degP1S, vParams, vKnots, uParams, uKnots, P12S) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot build P12S.");
    return false;
  }

  /*geom_InterpolateSurface interp(pointGrid, 1, 3, ParamsSelection_ChordLength, KnotsSelection_Average);

  if ( !interp.Perform() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot build P12S.");
    return false;
  }

  t_ptr<t_bsurf>              P12S    = interp.GetResult();*/
  //P12S->ExchangeUV();
  Handle(Geom_BSplineSurface) P12Socc = cascade::GetOpenCascadeBSurface(P12S);
  //
  //P12Socc->IncreaseDegree(3, 3);
  //
  P12S = cascade::GetMobiusBSurface(P12Socc);

  m_plotter.DRAW_SURFACE( P12Socc, Color_Default, "P12S" );

  /* =================================
   *  Unify components of Boolean sum
   * ================================= */

  P1Socc ->IncreaseDegree(3, 3);
  P2Socc ->IncreaseDegree(3, 3);
  P12Socc->IncreaseDegree(3, 3);
  //
  P1S  = cascade::GetMobiusBSurface(P1Socc);
  P2S  = cascade::GetMobiusBSurface(P2Socc);
  P12S = cascade::GetMobiusBSurface(P12Socc);

  //return true;

  enum PS
  {
    PS_P1S = 0,
    PS_P2S,
    PS_P12S
  };

  // All U knots.
  std::vector< std::vector<double> >
    knotVectors_U = { P1S->GetKnots_U(),
                      P2S->GetKnots_U(),
                      P12S->GetKnots_U() };

  // All V knots.
  std::vector< std::vector<double> >
    knotVectors_V = { P1S->GetKnots_V(),
                      P2S->GetKnots_V(),
                      P12S->GetKnots_V() };

  // Compute addendum knots.
  bspl_UnifyKnots unifyKnots;
  //
  std::vector< std::vector<double> > U_addendums = unifyKnots(knotVectors_U);
  std::vector< std::vector<double> > V_addendums = unifyKnots(knotVectors_V);

  // Insert U knots to P1S.
  for ( size_t ii = 0; ii < U_addendums[PS_P1S].size(); ++ii )
    P1S->InsertKnot_U(U_addendums[PS_P1S][ii]);

  // Insert V knots to P1S.
  for ( size_t ii = 0; ii < V_addendums[PS_P1S].size(); ++ii )
    P1S->InsertKnot_V(V_addendums[PS_P1S][ii]);

  // Insert U knots to P2S.
  for ( size_t ii = 0; ii < U_addendums[PS_P2S].size(); ++ii )
    P2S->InsertKnot_U(U_addendums[PS_P2S][ii]);

  // Insert V knots to P2S.
  for ( size_t ii = 0; ii < V_addendums[PS_P2S].size(); ++ii )
    P2S->InsertKnot_V(V_addendums[PS_P2S][ii]);

  // Insert U knots to P12S.
  for ( size_t ii = 0; ii < U_addendums[PS_P12S].size(); ++ii )
    P12S->InsertKnot_U(U_addendums[PS_P12S][ii]);

  // Insert V knots to P12S.
  for ( size_t ii = 0; ii < V_addendums[PS_P12S].size(); ++ii )
    P12S->InsertKnot_V(V_addendums[PS_P12S][ii]);

  // Draw.
  m_plotter.REDRAW_SURFACE("P1S",  cascade::GetOpenCascadeBSurface(P1S),  Color_Default);
  m_plotter.REDRAW_SURFACE("P2S",  cascade::GetOpenCascadeBSurface(P2S),  Color_Default);
  m_plotter.REDRAW_SURFACE("P12S", cascade::GetOpenCascadeBSurface(P12S), Color_Default);

  //return true;

  // Common knots.
  const std::vector<double>& Ucommon = P1S->GetKnots_U();
  const std::vector<double>& Vcommon = P1S->GetKnots_V();

  std::cout << "\nU[S1] = ";
  //
  for ( const auto& u : P1S->GetKnots_U() )
  {
    std::cout << u << " ";
  }
  std::cout << "\nV[S1] = ";
  //
  for ( const auto& v : P1S->GetKnots_V() )
  {
    std::cout << v << " ";
  }

  std::cout << "\nU[S2] = ";
  //
  for ( const auto& u : P2S->GetKnots_U() )
  {
    std::cout << u << " ";
  }
  std::cout << "\nV[S2] = ";
  //
  for ( const auto& v : P2S->GetKnots_V() )
  {
    std::cout << v << " ";
  }

  std::cout << "\nU[S12] = ";
  //
  for ( const auto& u : P12S->GetKnots_U() )
  {
    std::cout << u << " ";
  }
  std::cout << "\nV[S12] = ";
  //
  for ( const auto& v : P12S->GetKnots_V() )
  {
    std::cout << v << " ";
  }

  // Common degrees.
  const int pcommon = P1S->GetDegree_U();
  const int qcommon = P1S->GetDegree_V();

  /* =============================
   *  Compute Boolean sum surface
   * ============================= */

  // Now all patches are of the same degrees and defined on identical knot
  // vectors. It means that all patches are defined on the same basis. Therefore,
  // we can now produce a Boolean sum.

  const std::vector< std::vector<t_xyz> >& polesP1S  = P1S->GetPoles();
  const std::vector< std::vector<t_xyz> >& polesP2S  = P2S->GetPoles();
  const std::vector< std::vector<t_xyz> >& polesP12S = P12S->GetPoles();

  const int numPolesU = P1S->GetNumOfPoles_U();
  const int numPolesV = P1S->GetNumOfPoles_V();

  // Compute the resulting poles.
  std::vector< std::vector<t_xyz> > resPoles;
  //
  for ( int i = 0; i < numPolesU; ++i )
  {
    std::vector<t_xyz> col;
    for ( int j = 0; j < numPolesV; ++j )
    {
      t_xyz resPole = polesP1S[i][j] + polesP2S[i][j] - polesP12S[i][j];
      //
      col.push_back(resPole);
    }
    resPoles.push_back(col);
  }

  // Construct the resulting surface.
  t_ptr<t_bsurf>
    gordonSurf = new t_bsurf(resPoles, Ucommon, Vcommon, pcommon, qcommon);

  // Check surface deviation from the curve network.
  double bndDev, innerDev, maxDev;
  //
  CheckDeviation( cascade::GetOpenCascadeBSurface(gordonSurf),
                  profiles, guides, bndDev, innerDev, maxDev,
                  m_plotter );

  m_progress.SendLogMessage(LogNotice(Normal) << "\n\tBoundary deviation: %1."
                                                 "\n\tInner deviation: %2."
                                                 "\n\tMax deviation: %3."
                                              << bndDev << innerDev << maxDev);

  m_fMaxError = maxDev;

  support = cascade::GetOpenCascadeBSurface(gordonSurf);
  face    = BRepBuilderAPI_MakeFace(support, Precision::Confusion());

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildGordonSurf::reapproxCurves(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                                             std::vector<Handle(Geom_BSplineCurve)>&       result,
                                             std::vector<double>&                          params,
                                             std::vector<double>&                          knots) const
{
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

    m_plotter.DRAW_CURVE(resCurve, Color_Khaki, true, "reapproxedCurve");
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildGordonSurf::computeCurveIntersections(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
                                                        const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
                                                        math_Matrix&                                  uParams,
                                                        math_Matrix&                                  vParams) const
{
  /* =================================
   *  Compute intersection parameters.
   * ================================= */

  const int    numCurvesU = (int)( uCurves.size() );
  const int    numCurvesV = (int)( vCurves.size() );
  const double spatialTol = 0.01;
  bool         fail       = false;
  //
  for ( int spline_u_idx = 0; spline_u_idx < numCurvesU; ++spline_u_idx )
  {
    for ( int spline_v_idx = 0; spline_v_idx < numCurvesV; ++spline_v_idx )
    {
      std::vector<std::pair<double, double> >
        currentIntersections = tigl::CTiglBSplineAlgorithms::intersections(uCurves[static_cast<size_t>(spline_u_idx)],
                                                                           vCurves[static_cast<size_t>(spline_v_idx)],
                                                                           spatialTol);

      if ( currentIntersections.size() < 1 )
      {
        fail = true;
      }
      else if ( currentIntersections.size() == 1 )
      {
        uParams(spline_u_idx, spline_v_idx) = currentIntersections[0].first;
        vParams(spline_u_idx, spline_v_idx) = currentIntersections[0].second;
      }
      else if ( currentIntersections.size() == 2 ) // for closed curves
      {
        // only the u-directional B-spline curves are closed
        if ( uCurves[0]->IsClosed() )
        {
          if ( spline_v_idx == 0 )
          {
            uParams(spline_u_idx, spline_v_idx) = std::min(currentIntersections[0].first, currentIntersections[1].first);
          }
          else if ( spline_v_idx == static_cast<int>(vCurves.size() - 1) )
          {
            uParams(spline_u_idx, spline_v_idx) = std::max(currentIntersections[0].first, currentIntersections[1].first);
          }

          // intersection_params_vector[0].second == intersection_params_vector[1].second
          vParams(spline_u_idx, spline_v_idx) = currentIntersections[0].second;
        }

        // only the v-directional B-spline curves are closed
        if ( vCurves[0]->IsClosed() )
        {
          if ( spline_u_idx == 0 )
          {
            vParams(spline_u_idx, spline_v_idx) = std::min(currentIntersections[0].second, currentIntersections[1].second);
          }
          else if ( spline_u_idx == static_cast<int>(uCurves.size() - 1) )
          {
            vParams(spline_u_idx, spline_v_idx) = std::max(currentIntersections[0].second, currentIntersections[1].second);
          }

          // intersection_params_vector[0].first == intersection_params_vector[1].first
          uParams(spline_u_idx, spline_v_idx) = currentIntersections[0].first;
        }
      }

      else if ( currentIntersections.size() > 2 )
      {
        return false;
      }
    }
  }

  /* ===========================
   *  Beautify parameter values.
   * =========================== */

  int nProfiles = numCurvesU;
  int nGuides   = numCurvesV;

  /* eliminate small inaccuracies of the intersection parameters */

  // first intersection
  for ( int spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx )
  {
    if ( std::abs(uParams(spline_u_idx, 0) - uCurves[0]->Knot(1) ) < 0.001)
    {
      if ( std::abs(uCurves[0]->Knot(1)) < 1e-10 )
      {
        uParams(spline_u_idx, 0) = 0;
      }
      else
      {
        uParams(spline_u_idx, 0) = uCurves[0]->Knot(1);
      }
    }
  }

  for ( int spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx )
  {
    if ( std::abs(vParams(0, spline_v_idx) - vCurves[0]->Knot(1)) < 0.001 )
    {
      if ( std::abs( vCurves[0]->Knot(1) ) < 1e-10 )
      {
        vParams(0, spline_v_idx) = 0;
      }
      else
      {
        vParams(0, spline_v_idx) = vCurves[0]->Knot(1);
      }
    }
  }

  // last intersection
  for ( int spline_u_idx = 0; spline_u_idx < nProfiles; ++spline_u_idx )
  {
    if ( std::abs( uParams(spline_u_idx, nGuides - 1) - uCurves[0]->Knot( uCurves[0]->NbKnots() ) ) < 0.001 )
    {
      uParams(spline_u_idx, nGuides - 1) = uCurves[0]->Knot(uCurves[0]->NbKnots());
    }
  }

  for ( int spline_v_idx = 0; spline_v_idx < nGuides; ++spline_v_idx )
  {
    if ( std::abs( vParams(nProfiles - 1, spline_v_idx) - vCurves[0]->Knot( vCurves[0]->NbKnots() ) ) < 0.001 )
    {
      vParams(nProfiles - 1, spline_v_idx) = vCurves[0]->Knot( vCurves[0]->NbKnots() );
    }
  }

  return !fail;
}
