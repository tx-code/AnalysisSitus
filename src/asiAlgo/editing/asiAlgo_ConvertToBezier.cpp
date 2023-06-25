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

#include <CTiglBSplineAlgorithms.h>

// Asi includes
#include <asiAlgo_Utils.h>

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

bool asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Curve)& curve)
{
  Handle(ShapeUpgrade_ConvertCurve3dToBezier) curve3dConverter = new ShapeUpgrade_ConvertCurve3dToBezier;
  auto f = curve->FirstParameter();
  auto l = curve->LastParameter();
  curve3dConverter->Init(curve,f,l);
  curve3dConverter->Perform();
  Handle(TColGeom_HArray1OfCurve) curves = curve3dConverter->GetCurves();
  ShapeConstruct_Curve scc;
  std::vector<Handle(Geom_BSplineCurve)> bcurves;
  Handle(Geom_BSplineCurve) bsplineRes;
  auto res = curves->First();
  bool isFirst = true;
  for (auto it = curves->begin(); it != curves->end(); ++it)
  {
    if (isFirst)
    {
      isFirst = false;
      continue;
    }
    auto bspline = GeomConvert::CurveToBSplineCurve(*it);
    if (!JoinCurves(res, bspline, res->FirstParameter(), res->LastParameter(),
                         bspline->FirstParameter(), bspline->LastParameter(), res))
    {
      return false;
    }
  }
  m_plotter.DRAW_CURVE(res, Color_Blue, true, "curve_BEZIER");
  return true;
}

void asiAlgo_ConvertToBezier::BuildCurveV(const Handle(Geom_Surface)& patch, std::vector<Handle(Geom_Curve)>& curves, bool last)
{
  double u1, u2, v1, v2;
  patch->Bounds(u1, u2, v1, v2);
  u1 = last ? u2 : u1;
  double vv = v1;
  double step = 0.001;
  auto max = (v2 - v1) / step;
  Handle(TColgp_HArray1OfPnt) pnts = new TColgp_HArray1OfPnt(1, max);
  TColgp_Array1OfVec tangs(1, max);
  Handle(TColStd_HArray1OfBoolean) flags = new TColStd_HArray1OfBoolean(1, max);
  int i = 1;
  while (vv <= v2)
  {
    gp_Pnt pnt;
    gp_Vec d1u, d1v;
    patch->D1(u1, vv, pnt, d1u, d1v);
    pnts->SetValue(i, pnt);
    tangs.SetValue(i, d1v);
    flags->SetValue(i++, true);
    vv += step;
  }
  Handle(Geom_BezierCurve) bcurve;
  GeomAPI_Interpolate Interpol(pnts, Standard_False, Precision::Confusion());
  Interpol.Load(tangs, flags, true);
  Interpol.Perform();
  if (!Interpol.IsDone())
  {
    std::cout<<std::endl<<"Interpolation failed"<<std::endl;
  }
  auto curve = Interpol.Curve();
  curves.push_back(curve);
  m_plotter.DRAW_SURFACE(patch, Color_Purple, "bspl_SURFACE");
}

void asiAlgo_ConvertToBezier::BuildCurveU(const Handle(Geom_Surface)& patch, std::vector<Handle(Geom_Curve)>& curves, bool last)
{
  double u1, u2, v1, v2;
  patch->Bounds(u1, u2, v1, v2);
  v1 = last ? v2 : v1;
  double uu = u1;
  double step = 0.001;
  auto max = (u2 - u1) / step;
  Handle(TColgp_HArray1OfPnt) pnts = new TColgp_HArray1OfPnt(1, max);
  TColgp_Array1OfVec tangs(1, max);
  Handle(TColStd_HArray1OfBoolean) flags = new TColStd_HArray1OfBoolean(1, max);
  int i = 1;
  while (uu <= u2)
  {
    gp_Pnt pnt;
    gp_Vec d1u, d1v;
    patch->D1(uu, v1, pnt, d1u, d1v);
    pnts->SetValue(i, pnt);
    tangs.SetValue(i, d1v);
    flags->SetValue(i++, true);
    uu += step;
  }
  Handle(Geom_BezierCurve) bcurve;
  GeomAPI_Interpolate Interpol(pnts, Standard_False, Precision::Confusion());
  Interpol.Load(tangs, flags, true);
  Interpol.Perform();
  if (!Interpol.IsDone())
  {
    std::cout<<std::endl<<"Interpolation failed"<<std::endl;
  }
  auto curve = Interpol.Curve();
  curves.push_back(curve);

  auto bezier = Handle(Geom_BezierSurface)::DownCast(patch);
  //bezier->Increase(3,3);

}

bool asiAlgo_ConvertToBezier::SplitBezier(const Handle(Geom_BezierSurface)& surface)
{
  auto bspline = GeomConvert::SurfaceToBSplineSurface(surface);
  auto uKnots = bspline->NbUPoles();
  int nbUPoles = (bspline->NbUPoles() + 2) / 3;
  if (bspline->NbUPoles() % 4)
  {
    int num = bspline->NbUPoles() % 4;
    for (int i = 0; i < num; ++i)
    {
      for (int v = 1; v < bspline->NbVPoles() + 1; ++v)
      {
        double value;
        auto uNext = (bspline->Pole(4, v).XYZ() - bspline->Pole(3, v).XYZ()) / 2. + bspline->Pole(3, v).XYZ();
        GeomAPI_ProjectPointOnSurf proj(uNext, bspline);
        if (proj.IsDone())
        {
          bspline->SetPole(4, v, proj.Point(1));
        }
      }
      //bspline->Po(4);
    }
  }
  ShapeAnalysis_Surface sas(bspline);
  for (int u = 0; u < nbUPoles; ++u)
  {
    auto uvMin = sas.ValueOfUV(bspline->Pole(u * 3 + 1, 1), 1e-15);
    auto uvMax = sas.ValueOfUV(bspline->Pole(u * 3 + 3, bspline->NbVPoles()), 1e-15);
    auto res = tigl::CTiglBSplineAlgorithms::trimSurface(bspline, uvMin.X(), uvMax.X(), uvMin.Y(), uvMax.Y());
    //
    //auto res2 = GeomConvert::SplitBSplineSurface(bspline, uvMin.X(), uvMax.X(), true);
    //m_plotter.DRAW_SURFACE(res2, Color_Purple, "bspl_RES");
    //
    Handle(ShapeUpgrade_ConvertSurfaceToBezierBasis) surf3dConverter = new ShapeUpgrade_ConvertSurfaceToBezierBasis;
    surf3dConverter->Init(res);
    surf3dConverter->Perform(true);
    Handle(ShapeExtend_CompositeSurface) surfaces = surf3dConverter->ResSurfaces();
    int k = 0;
    for (auto u = surfaces->UJointValues()->begin(); u != surfaces->UJointValues()->end(); ++u)
    {
      //std::vector<Handle(Geom_Curve)> curves;
      bool isFirst = true;
      double prevU = -1;
      double prevV = -1;
      for (auto v = surfaces->VJointValues()->begin(); v != surfaces->VJointValues()->end(); ++v)
      {
        auto patch = surfaces->Patch({*u,*v});
        //BuildCurveV(patch, curves);

        auto bspline = GeomConvert::SurfaceToBSplineSurface(patch);
        Handle(Geom_BezierSurface) bezier = Handle(Geom_BezierSurface)::DownCast(patch);

        m_plotter.DRAW_SURFACE(GeomConvert::SurfaceToBSplineSurface(bezier), Color_Blue, "bspl_RES");
      }
      k++;
    }

  }
  return true;
}

bool asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Surface)& surface)
{
  Handle(ShapeUpgrade_ConvertSurfaceToBezierBasis) surf3dConverter = new ShapeUpgrade_ConvertSurfaceToBezierBasis;
  surf3dConverter->Init(surface);
  surf3dConverter->Perform(false);
  Handle(ShapeExtend_CompositeSurface) surfaces = surf3dConverter->ResSurfaces();

  /*---------------------------
   * V curves 
   ---------------------------*/
  m_mobBSurfaces.resize(surfaces->UJointValues()->Length());
  int k = 0;
  for (auto u = surfaces->UJointValues()->begin(); u != surfaces->UJointValues()->end(); ++u)
  {
    //std::vector<Handle(Geom_Curve)> curves;
    bool isFirst = true;
    double prevU = -1;
    double prevV = -1;
    for (auto v = surfaces->VJointValues()->begin(); v != surfaces->VJointValues()->end(); ++v)
    {
      auto patch = surfaces->Patch({*u,*v});
      //BuildCurveV(patch, curves);

      auto bspline = GeomConvert::SurfaceToBSplineSurface(patch);
      Handle(Geom_BezierSurface) bezier = Handle(Geom_BezierSurface)::DownCast(patch);
      if (bezier->NbUPoles() > 3)
      {
        SplitBezier(bezier);
      }
      Handle(Geom_BezierSurface) surf = new Geom_BezierSurface(bspline->Poles());
      //bspline->IncreaseDegree(3, 3);
      if (!isFirst)
      {
        tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*bspline.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
      }
      auto mobBSpline = mobius::cascade::GetMobiusBSurface(bspline);
      m_mobBSurfaces[k].push_back(mobBSpline);
      isFirst = false;
      prevU = bspline->UKnot(bspline->UKnots().Length());
      prevV = bspline->VKnot(bspline->VKnots().Length());
    }
    k++;
  }
  std::vector<mobius::t_ptr<mobius::geom_BSplineSurface>> spln;
  double prevU = -1;
  double prevV = -1;
  bool isFirst = true;
  for (size_t i = 0; i < m_mobBSurfaces.size(); ++i )
  {
    auto res = m_mobBSurfaces[i][0];
    //
    for ( size_t j = 1; j < m_mobBSurfaces[i].size(); ++j )
    {
      if (!res->ConcatenateCompatible(m_mobBSurfaces[i][j], true))
        res->ConcatenateCompatible(m_mobBSurfaces[i][j], false);
    }
    auto cascSurf = mobius::cascade::GetOpenCascadeBSurface(res);
    if (!isFirst)
    {
      tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*cascSurf.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
    }
    //cascSurf->IncreaseDegree(3,3);
    auto mobBSpline = mobius::cascade::GetMobiusBSurface(cascSurf);
    isFirst = false;
    prevU = cascSurf->UKnot(cascSurf->UKnots().Length());
    prevV = cascSurf->VKnot(cascSurf->VKnots().Length());
    spln.push_back(mobBSpline);
  }
  auto res = spln[0];
  for ( size_t j = 1; j < spln.size(); ++j )
  {
    if (!res->ConcatenateCompatible(spln[j], true))
      res->ConcatenateCompatible(spln[j], false);
  }
  m_plotter.DRAW_SURFACE(mobius::cascade::GetOpenCascadeBSurface(res), Color_Purple, "bspl_SURFACE");
  return true;
}

std::vector<Handle(Geom_Curve)> asiAlgo_ConvertToBezier::GetCurves() const
{
  return m_bCurves;
}

std::vector<Handle(Geom_BezierSurface)> asiAlgo_ConvertToBezier::GetSurfaces() const
{
  return m_bSurfaces;
}
