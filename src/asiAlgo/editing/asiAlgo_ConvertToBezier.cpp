//-----------------------------------------------------------------------------
// Created on: 16 June 2023
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
#include "asiAlgo_ConvertToBezier.h"

// Asi include
#include <asiAlgo_Utils.h>
#include <asiAlgo_AppSurfUtils.h>

// OpenCascade include
#include <ShapeUpgrade_ConvertCurve3dToBezier.hxx>
#include <ShapeUpgrade_ConvertSurfaceToBezierBasis.hxx>
#include <ShapeExtend_CompositeSurface.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomConvert_BSplineCurveToBezierCurve.hxx>
#include <ShapeConstruct_Curve.hxx>
#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomConvert.hxx>
#include <Adaptor3d_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <GeomConvert_ApproxSurface.hxx>
#include <GeomConvert_ApproxCurve.hxx>

// tigl includes
#include <CTiglBSplineAlgorithms.h>

//-----------------------------------------------------------------------------

template<class Curve>
  static inline void SegmentCurve(Curve&       curve,
                                  const double first,
                                  const double last)
{
  const double pconfusion = Precision::PConfusion();

  if (curve->FirstParameter() < first - pconfusion ||
      curve->LastParameter() > last + pconfusion)
  {
    if (curve->IsPeriodic())
    {
      curve->Segment(first, last);
    }
    else
    {
      curve->Segment(Max(curve->FirstParameter(), first),
                     Min(curve->LastParameter(), last));
    }
  }
}

//-----------------------------------------------------------------------------

bool JoinCurves(const Handle(Geom_Curve)& c3d1,
                const Handle(Geom_Curve)& c3d2,
                const double&             first1,
                const double&             last1,
                const double&             first2,
                const double&             last2,
                Handle(Geom_Curve)&       c3dOut)

{
  ShapeConstruct_Curve scc;
  Standard_Boolean after =  true;
  Handle(Geom_BSplineCurve) bsplc1 = scc.ConvertToBSpline(c3d1, first1, last1,Precision::Confusion());
  Handle(Geom_BSplineCurve) bsplc2 = scc.ConvertToBSpline(c3d2, first2, last2,Precision::Confusion());

  if(bsplc1.IsNull() || bsplc2.IsNull())
  {
    return false;
  }

  SegmentCurve(bsplc1,first1, last1);
  SegmentCurve(bsplc2,first2, last2);

  //regression on file 866026_M-f276-f311.brep bug OCC482
  gp_Pnt pp11 =  bsplc1->Pole(1);
  gp_Pnt pp12 =  bsplc1->Pole(bsplc1->NbPoles());

  gp_Pnt pp21 =  bsplc2->Pole(1);
  gp_Pnt pp22 =  bsplc2->Pole(bsplc2->NbPoles());

  gp_Pnt pmid = 0.5 * ( bsplc1->Pole(bsplc1->NbPoles()).XYZ() + bsplc2->Pole(1).XYZ() );
  bsplc1->SetPole(bsplc1->NbPoles(), pmid);
  bsplc2->SetPole(1, pmid);
  GeomConvert_CompCurveToBSplineCurve connect3d(bsplc1, Convert_RationalC1);
  if(!connect3d.Add(bsplc2,Precision::Confusion(), after, false))
  {
    return false;
  }
  c3dOut = connect3d.BSplineCurve();
  return true;
}

//-----------------------------------------------------------------------------

static void SimplifyCurve(Handle(Geom_BSplineCurve)& BC)

{
  int  mult, ii;

  const TColStd_Array1OfReal&    U = BC->Knots();
  for ( ii = U.Length() - 1; ii > 1; ii-- )
  {
    BC->InsertKnot(ii);
  }
}

//-----------------------------------------------------------------------------

static void SimplifySurface(Handle(Geom_BSplineSurface)& BS,
                            const double                 Tol,
                            const int                    MultMin)

{
  int  multU, multV, ii;
  bool Ok;

  const TColStd_Array1OfReal&    U  = BS->UKnots();
  const TColStd_Array1OfReal&    V  = BS->VKnots();
  const TColStd_Array1OfInteger& UM = BS->UMultiplicities();
  const TColStd_Array1OfInteger& VM = BS->VMultiplicities();

  for ( ii = U.Length() - 1; ii > 1; ii-- )
  {
    Ok    = true;
    multU = UM.Value(ii);
    for  ( ; Ok && multU > MultMin; multU-- )
    {
      Ok = BS->RemoveUKnot(ii, 1, Tol);
    }
  }

  for ( ii = V.Length() - 1; ii > 1; ii-- )
  {
    Ok    = true;
    multV = VM.Value(ii);
    for  ( ; Ok && multV > MultMin; multV-- )
    {
      Ok = BS->RemoveVKnot(ii, 1, Tol);
    }
  }
  if (BS->Continuity() == GeomAbs_C2)
  {
    for ( ii = U.Lower(); ii <= U.Upper(); ii++ )
    {
      BS->InsertUKnot(U(ii), 1, Precision::Confusion());
    }

    for ( ii = V.Lower(); ii <= V.Upper(); ii++ )
    {
       BS->InsertVKnot(V(ii), 1, Precision::Confusion());
    }
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Curve)& curve, double f, double l)
{
  Handle(ShapeUpgrade_ConvertCurve3dToBezier) curve3dConverter = new ShapeUpgrade_ConvertCurve3dToBezier;
  curve3dConverter->Init(curve,f,l);
  curve3dConverter->Perform();
  Handle(TColGeom_HArray1OfCurve) curves = curve3dConverter->GetCurves();
  auto res = GeomConvert::CurveToBSplineCurve(curves->First());
  bool isFirst = true;
  for (auto it = curves->begin(); it != curves->end(); ++it)
  {
    if (isFirst)
    {
      isFirst = false;

      GeomConvert_ApproxCurve approx (res, 1e-15, GeomAbs_C1, 1, 3 );
      if ( approx.HasResult() )
        res = approx.Curve();
      continue;
    }
    auto bspline = GeomConvert::CurveToBSplineCurve(*it);
    GeomConvert_ApproxCurve approx (bspline, 1e-15, GeomAbs_C1, 1, 3 );
    if ( approx.HasResult() )
      bspline = approx.Curve();
    if (!JoinCurves(res, bspline, res->FirstParameter(), res->LastParameter(),
      bspline->FirstParameter(), bspline->LastParameter(), res))
    {
      return false;
    }
  }
  if (res->Continuity() == GeomAbs_C0)
  {
    auto bspline = GeomConvert::CurveToBSplineCurve(res);
    GeomConvert::C0BSplineToC1BSplineCurve(bspline, 1e-7);
    res = bspline;
  }
  if (res->Continuity() == GeomAbs_C2)
  {
    auto bspline = GeomConvert::CurveToBSplineCurve(res);
    SimplifyCurve(bspline);
    res = bspline;
  }
  m_plotter.DRAW_CURVE(res, Color_Blue, true, "curve_BEZIER");
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Surface)& surface, bool toApprox)
{
  auto bspline_surface = GeomConvert::SurfaceToBSplineSurface(surface);
  if (bspline_surface->UDegree() < 3 && bspline_surface->VDegree() < 3)
  {
    bspline_surface->IncreaseDegree(3, 3);
  }
  else if (bspline_surface->UDegree() < 3)
  {
    bspline_surface->IncreaseDegree(3, bspline_surface->VDegree());
  }
  else if (bspline_surface->VDegree() < 3)
  {
    bspline_surface->IncreaseDegree(bspline_surface->UDegree(), 3);
  }
  else
  {
    Handle(ShapeUpgrade_ConvertSurfaceToBezierBasis) surf3dConverter = new ShapeUpgrade_ConvertSurfaceToBezierBasis;
    surf3dConverter->Init(surface);
    surf3dConverter->Perform();
    Handle(ShapeExtend_CompositeSurface) surfaces = surf3dConverter->ResSurfaces();
    //
    std::vector<std::vector<mobius::t_ptr<mobius::geom_BSplineSurface>>> mobBSurfaces;
    std::vector<std::vector<Handle(Geom_BSplineSurface)>> ocBSurfaces;
    mobBSurfaces.resize(surfaces->UJointValues()->Length());
    ocBSurfaces.resize(surfaces->UJointValues()->Length());
    //
    if (!toApprox)
    {
      /*---------------------------
      * Add knots
      ---------------------------*/
      int k = 0;
      for (auto u = surfaces->UJointValues()->begin(); u != surfaces->UJointValues()->end(); ++u)
      {
        for (auto v = surfaces->VJointValues()->begin(); v != surfaces->VJointValues()->end(); ++v)
        {
          Handle(Geom_BezierSurface) bezier = Handle(Geom_BezierSurface)::DownCast(surfaces->Patch({*u,*v}));
          if (bezier.IsNull())
            continue;
          Handle(Geom_BSplineSurface) bspline = GeomConvert::SurfaceToBSplineSurface(bezier);
          bspline->InsertUKnot(bspline->UKnot(1), 1, Precision::Confusion());
          bspline->InsertUKnot(bspline->UKnots().Last(), 1, Precision::Confusion());
          //
          bspline->InsertVKnot(bspline->VKnot(1), 1, Precision::Confusion());
          bspline->InsertVKnot(bspline->VKnots().Last(), 1, Precision::Confusion());
          auto mobBSpline = mobius::cascade::GetMobiusBSurface(bspline);
          ocBSurfaces[k].push_back(bspline);
        }
        k++;
      }
    }
    else
    {
      /* ---------------------------------------
      * Create Segments and approximate them
      --------------------------------------- */
      int k = 0;
      for (auto u = surfaces->UJointValues()->begin(); u != surfaces->UJointValues()->end(); ++u)
      {
        for (auto v = surfaces->VJointValues()->begin(); v != surfaces->VJointValues()->end(); ++v)
        {
          Handle(Geom_BSplineSurface) bspline;
          Handle(Geom_BezierSurface) bezier = Handle(Geom_BezierSurface)::DownCast(surfaces->Patch({*u,*v}));
          if (bezier.IsNull())
            continue;
          bspline = GeomConvert::SurfaceToBSplineSurface(bezier);
          if (bezier->NbUPoles() > 3)
          {
            GeomConvert_ApproxSurface anApprox(bezier, 1e-15, GeomAbs_C1, GeomAbs_C1, 3, 3, 0, 0);
            //
            Handle(Geom_Surface) result = anApprox.Surface();
            bspline = GeomConvert::SurfaceToBSplineSurface(result);
          }
          else
          {
            bspline->IncreaseDegree(3, 3);
          }
          auto mobBSpline = mobius::cascade::GetMobiusBSurface(bspline);
          ocBSurfaces[k].push_back(bspline);
        }
        k++;
      }
    }
    
    /*---------------------------
    * Concat segments 
    ---------------------------*/
    int k = 0;
    for (auto surfs : ocBSurfaces)
    {
      bool isFirst = true;
      double prevU = -1;
      double prevV = -1;
      int iter = 0;
      for (auto surf : surfs)
      {
        if (!isFirst)
        {
          tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*surf.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
        }
        auto mobBSpline = mobius::cascade::GetMobiusBSurface(surf);
        mobBSurfaces[k].push_back(mobBSpline);
        isFirst = false;
        prevU = surf->UKnot(surf->UKnots().Length());
        prevV = surf->VKnot(surf->VKnots().Length());
      }
      k++;
    }
    std::vector<mobius::t_ptr<mobius::geom_BSplineSurface>> spln;
    double prevU = -1;
    double prevV = -1;
    bool isFirst = true;
    if (mobBSurfaces.empty() || mobBSurfaces[0].empty())
    {
      return false;
    }
    for (size_t i = 0; i < mobBSurfaces.size(); ++i )
    {
      auto res = mobBSurfaces[i][0];
      for ( size_t j = 1; j < mobBSurfaces[i].size(); ++j )
      {
        if (!res->ConcatenateCompatible(mobBSurfaces[i][j], true))
          res->ConcatenateCompatible(mobBSurfaces[i][j], false);
      }
      auto cascSurf = mobius::cascade::GetOpenCascadeBSurface(res);
      if (!isFirst)
      {
        tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*cascSurf.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
      }
      auto mobBSpline = mobius::cascade::GetMobiusBSurface(cascSurf);
      isFirst = false;
      prevU = cascSurf->UKnot(cascSurf->UKnots().Length());
      prevV = cascSurf->VKnot(cascSurf->VKnots().Length());
      spln.push_back(mobBSpline);
    }
    if (spln.empty())
    {
      return false;
    }
    auto res = spln[0];
    for ( size_t j = 1; j < spln.size(); ++j )
    {
      if (!res->ConcatenateCompatible(spln[j], true))
        res->ConcatenateCompatible(spln[j], false);
    }
    m_bSurface = mobius::cascade::GetOpenCascadeBSurface(res);
    if (toApprox && m_bSurface->Continuity() == GeomAbs_C0)
    {
      SimplifySurface(m_bSurface, 1e10, 1);
    }
  }
  //
  double maxDev = 0.;
  asiAlgo_AppSurfUtils::MeasureDeviation(m_bSurface, GeomConvert::SurfaceToBSplineSurface(surface), maxDev, m_plotter);
  m_plotter.DRAW_TEXT(maxDev, "dev");
  return true;
}

//-----------------------------------------------------------------------------

Handle(Geom_Curve) asiAlgo_ConvertToBezier::GetCurve() const
{
  return m_bCurve;
}

//-----------------------------------------------------------------------------

Handle(Geom_Surface) asiAlgo_ConvertToBezier::GetSurface() const
{
  return m_bSurface;
}
