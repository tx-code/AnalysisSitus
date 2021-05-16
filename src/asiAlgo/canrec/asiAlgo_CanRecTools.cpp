//-----------------------------------------------------------------------------
// Created on: 30 April 2021
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
#include <asiAlgo_CanRecTools.h>

// OpenCascade includes
#include <ElSLib.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomLib.hxx>
#include <GeomLProp_CLProps.hxx>
#include <GeomLProp_SLProps.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pln.hxx>
#include <Precision.hxx>
#include <TColgp_HArray1OfPnt.hxx>

//------------------------------------------------------------------------------

#define INF_LIMIT 100

//------------------------------------------------------------------------------

// Auxiliary functions.
namespace
{
  bool EvaluateCurvature(const Handle(Geom_Surface)& surf,
                         const double                angleRad,
                         const gp_Pnt2d&             UV,
                         double&                     k)
  {
    // Calculate differential properties.
    GeomLProp_SLProps Props(surf, UV.X(), UV.Y(), 2, 1e-7);
    //
    if ( !Props.IsCurvatureDefined() )
    {
#if defined COUT_DEBUG
      std::cout << "Error: curvature is not defined" << std::endl;
#endif
      return false;
    }

    // Get differential properties.
    const gp_Vec Xu  = Props.D1U();
    const gp_Vec Xv  = Props.D1V();
    const gp_Vec Xuu = Props.D2U();
    const gp_Vec Xuv = Props.DUV();
    const gp_Vec Xvv = Props.D2V();
    const gp_Vec n   = Props.Normal();

    // Coefficients of the FFF.
    const double E = Xu.Dot(Xu);
    const double F = Xu.Dot(Xv);
    const double G = Xv.Dot(Xv);

    // Coefficients of the SFF.
    const double L = n.Dot(Xuu);
    const double M = n.Dot(Xuv);
    const double N = n.Dot(Xvv);

    // Calculate curvature using the coefficients of both fundamental forms.
    if ( Abs(angleRad - M_PI*0.5) < 1.0e-5 )
    {
      k = N / G;
    }
    else
    {
      const double lambda = Tan(angleRad);
      k = (L + 2*M*lambda + N*lambda*lambda) / (E + 2*F*lambda + G*lambda*lambda);
    }

    return true;
  }

  double TrimInf(const double val,
                 const double limit = INF_LIMIT)
  {
    double ret_val = val;
    if ( Precision::IsPositiveInfinite(val) )
      ret_val = limit;
    else if ( Precision::IsNegativeInfinite(val) )
      ret_val = -limit;

    return ret_val;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_CanRecTools::IsLinear(const TColgp_Array1OfPnt2d& pts,
                                   const double                toler,
                                   double&                     dev,
                                   gp_Lin2d&                   lin)
{
  const int nbPoles = pts.Length();
  //
  if ( nbPoles < 2 )
    return false;

  /* ========================
   *  Construct fitting line.
   * ======================== */

  double dMax = 0;
  int    iMax1 = 0, iMax2 = 0;
  int    i;

  for ( i = 1; i < nbPoles; ++i )
  {
    for ( int j = i + 1; j <= nbPoles; ++j )
    {
      const double dist = pts(i).SquareDistance( pts(j) );
      //
      if ( dist > dMax )
      {
        dMax  = dist;
        iMax1 = i;
        iMax2 = j;
      }
    }
  }

  double dPreci = Precision::Confusion()*Precision::Confusion();
  //
  if ( dMax < dPreci )
    return false;

  /* =========================================
   *  Test fitting line w.r.t. the input data.
   * ========================================= */

  // Prepare test line.
  gp_Vec2d vec( pts(iMax1), pts(iMax2) );
  gp_Dir2d dir( vec );
  //
  lin = gp_Lin2d( pts(iMax1), dir );

  const double tol2   = toler*toler;
  double       devMax = 0.;
  //
  for ( i = 1; i <= nbPoles; ++i )
  {
    const double
      dist = lin.SquareDistance( pts(i) );

    if ( dist > tol2 )
      return false; // Bad accuracy of approximation.

    if ( dist > devMax )
      devMax = dist;
  }

  // Return the reached deviation.
  dev = sqrt(devMax);
  return true;
}
//-----------------------------------------------------------------------------

bool
  asiAlgo_CanRecTools::IsCylindrical(const Handle(Geom_Surface)& surface,
                                     const double                uMinSurf,
                                     const double                uMaxSurf,
                                     const double                vMinSurf,
                                     const double                vMaxSurf,
                                     const double                toler,
                                     gp_Cylinder&                cyl,
                                     double&                     uMinCyl,
                                     double&                     uMaxCyl,
                                     double&                     vMinCyl,
                                     double&                     vMaxCyl,
                                     ActAPI_ProgressEntry        progress,
                                     ActAPI_PlotterEntry         plotter)
{
  // Prepare adaptor to avoid tons of downcasting.
  GeomAdaptor_Surface surfaceAdt(surface);
  GeomAbs_SurfaceType surfaceType = surfaceAdt.GetType();

  if ( surfaceType == GeomAbs_Cylinder )
  {
    cyl = surfaceAdt.Cylinder();
    return true;
  }

  // Do not consider irrelevant surface types.
  switch ( surfaceType )
  {
    case GeomAbs_Plane:
    case GeomAbs_Cone:
    case GeomAbs_Sphere:
    case GeomAbs_Torus:
    {
      return false;
    }
    default: break;
  }

  /* ============================
   *  Analyze shape by curvature.
   * ============================ */

  const int stepsNum = 10;

  double uMin, uMax, vMin, vMax;
  //
  uMin = ::TrimInf(uMinSurf);
  uMax = ::TrimInf(uMaxSurf);
  vMin = ::TrimInf(vMinSurf);
  vMax = ::TrimInf(vMaxSurf);

  const double uStep = (uMax - uMin) / stepsNum;
  const double vStep = (vMax - vMin) / stepsNum;
  std::vector<double> uValues, vValues;

  // Generate u-isos.
  double u     = uMin;
  bool   uStop = false;
  while ( !uStop )
  {
    if ( u > uMax )
    {
      u     = uMax;
      uStop = true;
    }
    //
    uValues.push_back(u);

    u += uStep;
  }

  // Generate v-isos.
  double v     = vMin;
  bool   vStop = false;
  while ( !vStop )
  {
    if ( v > vMax )
    {
      v     = vMax;
      vStop = true;
    }
    //
    vValues.push_back(v);

    v += vStep;
  }

  // Collect curvatures.
  std::vector<double> k1, k2;
  for ( auto uu : uValues )
  {
    for ( auto vv : vValues )
    {
      gp_Pnt2d uv(uu, vv);
      double kk[2] = {0., 0.};

      ::EvaluateCurvature(surface, 0.,      uv, kk[0]);
      ::EvaluateCurvature(surface, M_PI/2., uv, kk[1]);

      k1.push_back(kk[0]);
      k2.push_back(kk[1]);
    }
  }

  /* Now that we have two arrays of curvatures, we can check if
     all values are the same. One array should contain all zeroes.
     The second array should contain reciprocals of radius. */

  if ( k1.empty() || k2.empty() )
    return false;

  /* The curvature values should not be compared with the tolerance
     directly as the tolerance comes from the modeling space and
     has Euclidian distance nature. */

  const double numericZero = 1.e-6;
  const double numericInf  = 1.e5;

  // Check k1.
  const double k1_ref = k1[0];
  const double r1_ref = (Abs(k1_ref) < numericZero) ? numericInf : Abs(1./k1_ref);
  //
  for ( size_t ii = 1; ii < k1.size(); ++ii )
  {
    const double r_ii = (Abs(k1[ii]) < numericZero) ? numericInf : Abs(1./k1[ii]);

    if ( Abs(r_ii - r1_ref) > toler )
      return false;
  }

  // Check k2.
  const double k2_ref = k2[0];
  const double r2_ref = (Abs(k2_ref) < numericZero) ? numericInf : Abs(1./k2_ref);
  //
  for ( size_t ii = 1; ii < k2.size(); ++ii )
  {
    const double r_ii = (Abs(k2[ii]) < numericZero) ? numericInf : Abs(1./k2[ii]);

    if ( Abs(r_ii - r2_ref) > toler )
      return false;
  }

  /* ===============
   *  Extract props.
   * =============== */

  bool   isCurvedU  = false;
  double convRadius = 0.;

  if ( (Abs(k1_ref) < numericZero) && (Abs(k2_ref) > numericZero) )
  {
    isCurvedU  = false;
    convRadius = 1./Abs(k2_ref);
    cyl.SetRadius(convRadius);
  }
  else if ( (Abs(k1_ref) > numericZero) && (Abs(k2_ref) < numericZero) )
  {
    isCurvedU  = true;
    convRadius = 1./Abs(k1_ref);
    cyl.SetRadius(convRadius);
  }
  else
  {
    return false;
  }

  // Get the midpoint.
  gp_Pnt2d P2d( (uMin + uMax)*0.5, (vMin + vMax)*0.5 );

  // Evaluate the differential props.
  gp_Pnt P3d;
  gp_Vec D1u, D1v;
  surface->D1(P2d.X(), P2d.Y(), P3d, D1u, D1v);

  // Get isoline in the probe point.
  Handle(Geom_Curve) iso = isCurvedU ? surface->VIso( P2d.Y() )
                                     : surface->UIso( P2d.X() );

  // Get the curve's differential props.
  const double t_onCurve = ( iso->FirstParameter() + iso->LastParameter() )*0.5;
  //
  GeomLProp_CLProps CLProp( iso, 2, Precision::Confusion() );
  CLProp.SetParameter(t_onCurve);
  //
  if ( !CLProp.IsTangentDefined() )
    return false; // Degeneracy.

  const double K_onCurve = CLProp.Curvature();
  //
  if ( Abs(K_onCurve) < Precision::Confusion() )
    return false; // No real curvature.

  // Calculate the center of curvature.
  gp_Pnt center_onCurve;
  CLProp.CentreOfCurvature(center_onCurve);

#if defined DRAW_DEBUG
  plotter.REDRAW_CURVE("iso", iso,            Color_Blue);
  plotter.REDRAW_POINT("C",   center_onCurve, Color_Red);
#endif

  // Axis.
  cyl.SetPosition( gp_Ax3(center_onCurve, isCurvedU ? D1v : D1u) );

  gp_Pnt Pmin = surface->Value(uMin, vMin);
  gp_Pnt Pmax = surface->Value(uMax, vMax);

  ElSLib::Parameters(cyl, Pmin, uMinCyl, vMinCyl);
  ElSLib::Parameters(cyl, Pmax, uMaxCyl, vMaxCyl);

  // Resolve possible ambiguity of projection on periodic surface
  // by adjusting the min/max U values according the curve length
  // they induce.
  if ( uMinCyl > uMaxCyl )
  {
    // Compute the spline curve's length by Gauss integration.
    const double originalLen = GCPnts_AbscissaPoint::Length( GeomAdaptor_Curve(iso) );

    // Compute the converted arc's length by angle and radius.
    const double convLen = (uMinCyl - uMaxCyl)*convRadius;

    // Check if the lengths are matching.
    if ( Abs(convLen - originalLen) > Precision::Confusion() )
    {
      uMinCyl -= 2*M_PI;
    }
  }

#if defined DRAW_DEBUG
  plotter.REDRAW_POINT     ("PminCyl", ElSLib::Value(uMinCyl, vMinCyl, cyl),  Color_Red);
  plotter.REDRAW_POINT     ("PmaxCyl", ElSLib::Value(uMaxCyl, vMaxCyl, cyl),  Color_Red);
  plotter.REDRAW_VECTOR_AT ("Ax",      center_onCurve, isCurvedU ? D1v : D1u, Color_Red);
#endif

  return true;
}
