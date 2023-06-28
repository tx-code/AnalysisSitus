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
#include <asiAlgo_BSplineSurface.h>

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

bool asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Curve)& curve, double f, double l)
{
  Handle(ShapeUpgrade_ConvertCurve3dToBezier) curve3dConverter = new ShapeUpgrade_ConvertCurve3dToBezier;
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

void asiAlgo_ConvertToBezier::CreateSurfaceU0(Handle(Geom_BSplineSurface)& bspline)
{
  GeomAPI_ProjectPointOnSurf proj;
  ShapeAnalysis_Surface sas(bspline);
  //
  std::vector<Handle(Geom_BSplineSurface)> segments;

  /* ----------------------------
  * NbUPoles > NbVPoles
  ------------------------------ */
  if (bspline->NbUPoles() > bspline->NbVPoles())
  {
    int diff = bspline->NbUPoles() - bspline->NbVPoles();
    for (int i = 0; i < diff; ++i)
    {
      auto newV = (bspline->Pole(1, 2).XYZ() - bspline->Pole(1, 1).XYZ()) / 2. + bspline->Pole(1, 1).XYZ();
      bspline->SetPole(1, 2, newV);
    }
  }
  
  /* ----------------------------
  * NbUPoles == NbVPoles
  ------------------------------ */
  if (bspline->NbUPoles() == bspline->NbVPoles())
  {
    int uNum = (bspline->NbUPoles() + 1) % 4;
    for (int i = 0; i < uNum; ++i)
    {
      gp_Pnt lPnt = bspline->Pole(i * 3 + 1, i * 3 + 1);
      proj.Init(lPnt, bspline);
      auto lUV = sas.ValueOfUV(proj.Point(1), 1e-15);
      //
      gp_Pnt rPnt = bspline->Pole(i * 3 + 4, i * 4 + 1);
      proj.Init(rPnt, bspline);
      auto rUV = sas.ValueOfUV(proj.Point(1), 1e-15);
      //
      Handle(asiAlgo_BSplineSurface) res = new asiAlgo_BSplineSurface(bspline->Poles(), bspline->UKnots(), bspline->VKnots(), bspline->UMultiplicities(),
        bspline->VMultiplicities(), bspline->UDegree(), bspline->VDegree());
      res->Segment(lUV.X(), rUV.X(), lUV.Y(), rUV.Y(), 3, 3);

      Handle(Geom_BSplineSurface) segment = new Geom_BSplineSurface(res->Poles(),
                                             res->UKnots(),
                                             res->VKnots(),
                                             res->UMultiplicities(),
                                             res->VMultiplicities(),
                                             res->UDegree(),
                                             res->VDegree());

      //m_bSplineSurfaces.push_back(segment);
      m_plotter.DRAW_SURFACE(segment, Color_Yellow, "bspl_Seg");
    }
  }
  /* ----------------------------
  * NbUPoles < NbVPoles
  ------------------------------ */
  if (bspline->NbUPoles() < bspline->NbVPoles())
  {
    int uNum = (bspline->NbUPoles() + 1) % 4;
    for (int i = 0; i < uNum; ++i)
    {
      gp_Pnt lPnt = bspline->Pole(i * 3 + 1, i * 3 + 1);
      proj.Init(lPnt, bspline);
      auto lUV = sas.ValueOfUV(proj.Point(1), 1e-15);
      //
      gp_Pnt rPnt = bspline->Pole(i * 3 + 4, i * 4 + 1);
      proj.Init(rPnt, bspline);
      auto rUV = sas.ValueOfUV(proj.Point(1), 1e-15);
      //
      Handle(asiAlgo_BSplineSurface) res = new asiAlgo_BSplineSurface(bspline->Poles(), bspline->UKnots(), bspline->VKnots(), bspline->UMultiplicities(),
        bspline->VMultiplicities(), bspline->UDegree(), bspline->VDegree());
      res->Segment(lUV.X(), rUV.X(), lUV.Y(), rUV.Y(), 3, 3);

      segments.push_back(new Geom_BSplineSurface(res->Poles(),
                                                 res->UKnots(),
                                                 res->VKnots(),
                                                 res->UMultiplicities(),
                                                 res->VMultiplicities(),
                                                 res->UDegree(),
                                                 res->VDegree()));
      m_plotter.DRAW_SURFACE(segments.back(), Color_Yellow, "bspl_Seg");
    }
    //
    //int vNum = (bspline->NbUPoles() + 1) % 4;
    //for (int i = 0; i < uNum; ++i)
    //{
    //  gp_Pnt lPnt = bspline->Pole(i * 3 + 1, i * 3 + 1);
    //  proj.Init(lPnt, bspline);
    //  auto lUV = sas.ValueOfUV(proj.Point(1), 1e-15);
    //  //
    //  gp_Pnt rPnt = bspline->Pole(i * 3 + 4, i * 4 + 1);
    //  proj.Init(rPnt, bspline);
    //  auto rUV = sas.ValueOfUV(proj.Point(1), 1e-15);
    //  //
    //  Handle(asiAlgo_BSplineSurface) res = new asiAlgo_BSplineSurface(bspline->Poles(), bspline->UKnots(), bspline->VKnots(), bspline->UMultiplicities(),
    //    bspline->VMultiplicities(), bspline->UDegree(), bspline->VDegree());
    //  res->Segment(lUV.X(), rUV.X(), lUV.Y(), rUV.Y(), 3, 3);

    //  segments.push_back(new Geom_BSplineSurface(res->Poles(),
    //    res->UKnots(),
    //    res->VKnots(),
    //    res->UMultiplicities(),
    //    res->VMultiplicities(),
    //    res->UDegree(),
    //    res->VDegree()));
    //  m_plotter.DRAW_SURFACE(segments.back(), Color_Yellow, "bspl_Seg");
    //}
  }
}

void asiAlgo_ConvertToBezier::CreateSurfaceU1(Handle(Geom_BSplineSurface)& bspline)
{
  GeomAPI_ProjectPointOnSurf proj;
  ShapeAnalysis_Surface sas(bspline);

  /* ----------------------------
  * NbUPoles == NbVPoles
  ------------------------------ */

  if (bspline->Pole(1, 1).Distance(bspline->Pole(1, bspline->NbVPoles())) < 1. &&
      bspline->Pole(1, 1).Distance(bspline->Pole(bspline->NbUPoles(), 1)) < 1.)
  {
    TColgp_Array2OfPnt poles(1, 4, 1, 4);
    poles.SetValue(1, 1, bspline->Pole(1, 1));
    //
    poles.SetValue(1, 2, bspline->Pole(1, 2));
    poles.SetValue(1, 3, bspline->Pole(1, bspline->NbVPoles() - 1));
    poles.SetValue(1, 4, bspline->Pole(1, bspline->NbVPoles()));
    //
    poles.SetValue(2, 1, bspline->Pole(2, 1));
    poles.SetValue(2, 2, bspline->Pole(2, 2));
    poles.SetValue(2, 3, bspline->Pole(2, bspline->NbVPoles() - 1));
    poles.SetValue(2, 4, bspline->Pole(2, bspline->NbVPoles()));
    //
    poles.SetValue(3, 1, bspline->Pole(bspline->NbUPoles() - 1, 1));
    //
    poles.SetValue(3, 2, bspline->Pole(bspline->NbUPoles() - 1, 2));
    poles.SetValue(3, 3, bspline->Pole(bspline->NbUPoles() - 1, bspline->NbVPoles() - 1));
    poles.SetValue(3, 4, bspline->Pole(bspline->NbUPoles() - 1, bspline->NbVPoles()));
    //
    poles.SetValue(4, 1, bspline->Pole(bspline->NbUPoles(), 1));
    //
    poles.SetValue(4, 2, bspline->Pole(bspline->NbUPoles(), 2));
    poles.SetValue(4, 3, bspline->Pole(bspline->NbUPoles(), bspline->NbVPoles() - 1));
    //
    poles.SetValue(4, 4, bspline->Pole(bspline->NbUPoles(), bspline->NbVPoles()));

    Handle(Geom_BezierSurface) bezier = new Geom_BezierSurface(poles);
    bspline = GeomConvert::SurfaceToBSplineSurface(bezier);
    m_plotter.DRAW_SURFACE(bezier, Color_Red, "bezier_Res");
    return;
  }
  int uNum = bspline->NbUPoles() % 4;
  if (bspline->Pole(1, 1).Distance(bspline->Pole(1, bspline->NbVPoles())) < 1.)
  {
    m_bSplineSurfaces.resize(1);
    TColgp_Array2OfPnt poles(1, 4, 1, 4);
    TColStd_Array1OfReal uKnots(1, 2);
    TColStd_Array1OfReal vKnots(1, 2);
    TColStd_Array1OfInteger uMults(1, 2);
    TColStd_Array1OfInteger vMults(1, 2);
    uKnots.SetValue(1, 0.);
    uKnots.SetValue(2, 1.);
    vKnots.SetValue(1, 0.);
    vKnots.SetValue(2, 1.);
    bool isFirst = true;
    gp_Pnt prevV1;
    gp_Pnt prevV2;
    gp_Pnt prevV3;
    gp_Pnt prevV4;
    for (int j = 0; j < uNum; ++j)
    {
      if (isFirst)
      {
        isFirst = false;
        //
        poles.SetValue(1, 1, bspline->Pole(1, j * 3 + 1));
        poles.SetValue(1, 2, bspline->Pole(1, j * 3 + 2));
        poles.SetValue(1, 3, bspline->Pole(1, j * 3 + 3));
        //
        gp_Pnt pnt = (bspline->Pole(1, 4).XYZ() + bspline->Pole(1, 3).XYZ()) / 2.;
        proj.Init(pnt, bspline);
        auto project = proj.Point(1);\
          poles.SetValue(1, 4, project);
        //
        poles.SetValue(2, 1, bspline->Pole(2, j * 3 + 1));
        poles.SetValue(2, 2, bspline->Pole(2, j * 3 + 2));
        poles.SetValue(2, 3, bspline->Pole(2, j * 3 + 3));
        //
        pnt = (bspline->Pole(2, 4).XYZ() + bspline->Pole(2, 3).XYZ()) / 2.;
        proj.Init(pnt, bspline);
        project = proj.Point(1);
        poles.SetValue(2, 4, project);
        //
        poles.SetValue(3, 1, bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 1));
        poles.SetValue(3, 2, bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 2));
        poles.SetValue(3, 3, bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 3));
        //
        pnt = (bspline->Pole(bspline->NbUPoles() - 1, 4).XYZ() + bspline->Pole(bspline->NbUPoles() - 1, 3).XYZ()) / 2.;
        proj.Init(pnt, bspline);
        project = proj.Point(1);
        poles.SetValue(3, 4, project);
        //
        poles.SetValue(4, 1, bspline->Pole(bspline->NbUPoles(), j * 3 + 1));
        poles.SetValue(4, 2, bspline->Pole(bspline->NbUPoles(), j * 3 + 2));
        poles.SetValue(4, 3, bspline->Pole(bspline->NbUPoles(), j * 3 + 3));
        //
        pnt = (bspline->Pole(bspline->NbUPoles(), 4).XYZ() + bspline->Pole(bspline->NbUPoles(), 3).XYZ()) / 2.;
        proj.Init(pnt, bspline);
        project = proj.Point(1);
        poles.SetValue(4, 4, project);
        //
        prevV1 = (bspline->Pole(1, j * 3 + 4).XYZ() + bspline->Pole(1, j * 3 + 3).XYZ()) / 2.;
        prevV2 = (bspline->Pole(2, j * 3 + 4).XYZ() + bspline->Pole(2, j * 3 + 3).XYZ()) / 2.;
        prevV3 = (bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 4).XYZ() + bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 3).XYZ()) / 2.;
        prevV4 = (bspline->Pole(bspline->NbUPoles(), j * 3 + 4).XYZ() + bspline->Pole(bspline->NbUPoles(), j * 3 + 3).XYZ()) / 2.;

        /*for (int p = 0; p < poles.NbRows(); ++p)
        {
        for (int g = 0; g < poles.NbColumns(); ++g)
        {
        m_plotter.DRAW_POINT(poles.Value(p + 1, g + 1), Color_Green, "point");
        }
        }*/
      }
      else
      {
        proj.Init(prevV1, bspline);
        auto project = proj.Point(1);
        poles.SetValue(1, 1, project);
        //
        poles.SetValue(1, 2, bspline->Pole(1, j * 3 + 1));
        poles.SetValue(1, 3, bspline->Pole(1, j * 3 + 2));
        poles.SetValue(1, 4, bspline->Pole(1, j * 3 + 3));
        //
        proj.Init(prevV2, bspline);
        project = proj.Point(1);
        poles.SetValue(2, 1, project);
        //
        poles.SetValue(2, 2, bspline->Pole(4, j * 3 + 1));
        poles.SetValue(2, 3, bspline->Pole(4, j * 3 + 2));
        poles.SetValue(2, 4, bspline->Pole(4, j * 3 + 3));
        //
        proj.Init(prevV3, bspline);
        project = proj.Point(1);
        poles.SetValue(3, 1, project);
        //
        poles.SetValue(3, 2, bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 1));
        poles.SetValue(3, 3, bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 2));
        poles.SetValue(3, 4, bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 3));
        //
        proj.Init(prevV4, bspline);
        project = proj.Point(1);
        poles.SetValue(4, 1, project);
        //
        poles.SetValue(4, 2, bspline->Pole(bspline->NbUPoles(), j * 3 + 1));
        poles.SetValue(4, 3, bspline->Pole(bspline->NbUPoles(), j * 3 + 2));
        poles.SetValue(4, 4, bspline->Pole(bspline->NbUPoles(), j * 3 + 3));
        //
        prevV1 = bspline->Pole(1, j * 3 + 3).XYZ();
        prevV2 = bspline->Pole(2, j * 3 + 3).XYZ();
        prevV3 = bspline->Pole(bspline->NbUPoles() - 1, j * 3 + 3).XYZ();
        prevV4 = bspline->Pole(bspline->NbUPoles(), j * 3 + 3).XYZ();
      }
    }
    Handle(asiAlgo_BSplineSurface) algoBspline = new asiAlgo_BSplineSurface(bspline->Poles(), bspline->UKnots(),
                                                     bspline->VKnots(), bspline->UMultiplicities(),
                                                     bspline->VMultiplicities(), bspline->UDegree(), bspline->VDegree());
    //
    gp_Pnt2d lUV = sas.ValueOfUV(poles.Value(1, 1), 1e-15);
    gp_Pnt2d rUV = sas.ValueOfUV(poles.Value(4, 4), 1e-15);
    //
    algoBspline->Segment(lUV.X(), rUV.X(), lUV.Y(), rUV.Y(), 3, 3);
    //
    uMults.SetValue(1, algoBspline->UMultiplicity(1));
    uMults.SetValue(2, algoBspline->UMultiplicity(2));
    vMults.SetValue(1, algoBspline->VMultiplicity(1));
    vMults.SetValue(2, algoBspline->VMultiplicity(2));
    //
    Handle(Geom_BSplineSurface) segment = new Geom_BSplineSurface(poles, uKnots, vKnots, uMults, vMults, 3, 3);
    //
    Handle(Geom_BezierSurface) bezier = new Geom_BezierSurface(poles);
    m_bSplineSurfaces[0].push_back(segment);
    //m_plotter.DRAW_SURFACE(segment, Color_Red, "bezierSegment_Res");
  }
  else
  {
    m_bSplineSurfaces.resize(uNum);
    bool isFirst = true;
    //
    gp_Pnt prevU1;
    gp_Pnt prevU2;
    gp_Pnt prevU3;
    gp_Pnt prevU4;
    //
    for (int i = 0; i < uNum; ++i)
    {
      // col iteration
      // || ||     ||
      // \/ \/ ... \/
      bool isFirstJ = true;
      gp_Pnt prevV1;
      gp_Pnt prevV2;
      gp_Pnt prevV3;
      gp_Pnt prevV4;
      //
      gp_Pnt prevV1_;
      gp_Pnt prevV2_;
      gp_Pnt prevV3_;
      gp_Pnt prevV4_;
      for (int j = 0; j < uNum; ++j)
      {
        TColgp_Array2OfPnt poles(1, 4, 1, 4);
        TColStd_Array2OfReal weights(1, 4, 1, 4);
        TColStd_Array1OfReal uKnots(1, 2);
        TColStd_Array1OfReal vKnots(1, 2);
        TColStd_Array1OfInteger uMults(1, 2);
        TColStd_Array1OfInteger vMults(1, 2);
        uKnots.SetValue(1, 0.);
        uKnots.SetValue(2, 1.);
        vKnots.SetValue(1, 0.);
        vKnots.SetValue(2, 1.);
        if (isFirst)
        {
          if (isFirstJ)
          {
            isFirstJ = false;
            //
            poles.SetValue(1, 1, bspline->Pole(1, j * 3 + 1));
            poles.SetValue(1, 2, bspline->Pole(1, j * 3 + 2));
            poles.SetValue(1, 3, bspline->Pole(1, j * 3 + 3));
            //
            gp_Pnt pnt = (bspline->Pole(1, 4).XYZ() + bspline->Pole(1, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            auto project = proj.Point(1);\
            poles.SetValue(1, 4, project);
            //
            poles.SetValue(2, 1, bspline->Pole(2, j * 3 + 1));
            poles.SetValue(2, 2, bspline->Pole(2, j * 3 + 2));
            poles.SetValue(2, 3, bspline->Pole(2, j * 3 + 3));
            //
            pnt = (bspline->Pole(2, 4).XYZ() + bspline->Pole(2, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(2, 4, project);
            //
            poles.SetValue(3, 1, bspline->Pole(3, j * 3 + 1));
            poles.SetValue(3, 2, bspline->Pole(3, j * 3 + 2));
            poles.SetValue(3, 3, bspline->Pole(3, j * 3 + 3));
            //
            pnt = (bspline->Pole(3, 4).XYZ() + bspline->Pole(3, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(3, 4, project);
            //
            pnt = (bspline->Pole(4, 1).XYZ() + bspline->Pole(3, 1).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 1, project);
            //
            pnt = (bspline->Pole(4, 2).XYZ() + bspline->Pole(3, 2).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 2, project);
            //
            pnt = (bspline->Pole(4, 3).XYZ() + bspline->Pole(3, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 3, project);
            //
            pnt = (bspline->Pole(4, 4).XYZ() + bspline->Pole(3, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 4, project);
            //
            prevV1 = (bspline->Pole(1, j * 3 + 4).XYZ() + bspline->Pole(1, j * 3 + 3).XYZ()) / 2.;
            prevV2 = (bspline->Pole(2, j * 3 + 4).XYZ() + bspline->Pole(2, j * 3 + 3).XYZ()) / 2.;
            prevV3 = (bspline->Pole(3, j * 3 + 4).XYZ() + bspline->Pole(3, j * 3 + 3).XYZ()) / 2.;
            prevV4 = (bspline->Pole(4, j * 3 + 4).XYZ() + bspline->Pole(3, j * 3 + 3).XYZ()) / 2.;
            //
            prevU1 = (bspline->Pole(4, 1).XYZ() + bspline->Pole(3, 1).XYZ()) / 2.;
            prevU2 = (bspline->Pole(4, 2).XYZ() + bspline->Pole(3, 2).XYZ()) / 2.;
            prevU3 = (bspline->Pole(4, 3).XYZ() + bspline->Pole(3, 3).XYZ()) / 2.;
            prevU4 = (bspline->Pole(4, 4).XYZ() + bspline->Pole(3, 3).XYZ()) / 2.;

            /*for (int p = 0; p < poles.NbRows(); ++p)
            {
              for (int g = 0; g < poles.NbColumns(); ++g)
              {
                m_plotter.DRAW_POINT(poles.Value(p + 1, g + 1), Color_Green, "point");
              }
            }*/
          }
          else
          {
            proj.Init(prevV1, bspline);
            auto project = proj.Point(1);
            poles.SetValue(1, 1, project);
            //
            poles.SetValue(1, 2, bspline->Pole(1, j * 3 + 1));
            poles.SetValue(1, 3, bspline->Pole(1, j * 3 + 2));
            poles.SetValue(1, 4, bspline->Pole(1, j * 3 + 3));
            //
            proj.Init(prevV2, bspline);
            project = proj.Point(1);
            poles.SetValue(2, 1, project);
            //
            poles.SetValue(2, 2, bspline->Pole(4, j * 3 + 1));
            poles.SetValue(2, 3, bspline->Pole(4, j * 3 + 2));
            poles.SetValue(2, 4, bspline->Pole(4, j * 3 + 3));
            //
            proj.Init(prevV3, bspline);
            project = proj.Point(1);
            poles.SetValue(3, 1, project);
            //
            poles.SetValue(3, 2, bspline->Pole(3, j * 3 + 1));
            poles.SetValue(3, 3, bspline->Pole(3, j * 3 + 2));
            poles.SetValue(3, 4, bspline->Pole(3, j * 3 + 3));
            //
            proj.Init(prevV4, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 1, project);
            //
            auto pnt = (bspline->Pole(4, j * 3 + 1).XYZ() + bspline->Pole(3, j * 3 + 1).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 2, project);
            //
            pnt = (bspline->Pole(4, j * 3 + 2).XYZ() + bspline->Pole(3, j * 3 + 2).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 3, project);
            //
            pnt = (bspline->Pole(4, j * 3 + 3).XYZ() + bspline->Pole(3, j * 3 + 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 4, project);
            //
            prevV1 = bspline->Pole(1, j * 3 + 3).XYZ();
            prevV2 = bspline->Pole(2, j * 3 + 3).XYZ();
            prevV3 = bspline->Pole(3, j * 3 + 3).XYZ();
            prevV4 = bspline->Pole(4, j * 3 + 3).XYZ();
          }
        }
        else
        {
          if (isFirstJ)
          {
            isFirstJ = false;
            //
            proj.Init(prevU1, bspline);
            auto project = proj.Point(1);
            poles.SetValue(1, 1, project);
            //
            proj.Init(prevU2, bspline);
            project = proj.Point(1);
            poles.SetValue(1, 2, project);
            //
            proj.Init(prevU3, bspline);
            project = proj.Point(1);
            poles.SetValue(1, 3, project);
            //
            proj.Init(prevU4, bspline);
            project = proj.Point(1);
            poles.SetValue(1, 4, project);
            //
            poles.SetValue(2, 1, bspline->Pole(i * 3 + 1, j * 3 + 1));
            poles.SetValue(2, 2, bspline->Pole(i * 3 + 1, j * 3 + 2));
            poles.SetValue(2, 3, bspline->Pole(i * 3 + 1, j * 3 + 3));
            //
            auto pnt = (bspline->Pole(i * 3 + 1, 4).XYZ() + bspline->Pole(i * 3 + 1, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(2, 4, project);
            //
            poles.SetValue(3, 1, bspline->Pole(i * 3 + 2, j * 3 + 1));
            poles.SetValue(3, 2, bspline->Pole(i * 3 + 2, j * 3 + 2));
            poles.SetValue(3, 3, bspline->Pole(i * 3 + 2, j * 3 + 3));
            //
            pnt = (bspline->Pole(i * 3 + 2, 4).XYZ() + bspline->Pole(i * 3 + 2, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(3, 4, project);
            //
            poles.SetValue(4, 1, bspline->Pole(i * 3 + 3, 1));
            poles.SetValue(4, 2, bspline->Pole(i * 3 + 3, 2));
            poles.SetValue(4, 3, bspline->Pole(i * 3 + 3, 3));
            //
            pnt = (bspline->Pole(i * 3 + 3, 4).XYZ() + bspline->Pole(i * 3 + 3, 3).XYZ()) / 2.;
            proj.Init(pnt, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 4, project);
            //
            prevV1 = (bspline->Pole(i * 3 + 1, j * 3 + 4).XYZ() + bspline->Pole(i * 3, j * 3 + 4).XYZ()) / 2.;
            prevV2 = (bspline->Pole(i * 3 + 1, j * 3 + 5).XYZ() + bspline->Pole(i * 3, j * 3 + 5).XYZ()) / 2.;
            prevV3 = (bspline->Pole(i * 3 + 1, j * 3 + 6).XYZ() + bspline->Pole(i * 3, j * 3 + 6).XYZ()) / 2.;
            //
            prevV1_ = (bspline->Pole(i * 3 + 1, j * 3 + 4).XYZ() + bspline->Pole(i * 3, j * 3 + 3).XYZ()) / 2.;
            prevV2_ = (bspline->Pole(i * 3 + 1, j * 3 + 4).XYZ() + bspline->Pole(i * 3 + 1, j * 3 + 3).XYZ()) / 2.;
            prevV3_ = (bspline->Pole(i * 3 + 2, j * 3 + 4).XYZ() + bspline->Pole(i * 3 + 2, j * 3 + 3).XYZ()) / 2.;
            prevV4_ = (bspline->Pole(i * 3 + 3, j * 3 + 4).XYZ() + bspline->Pole(i * 3 + 3, j * 3 + 3).XYZ()) / 2.;
          }
          else
          {
            proj.Init(prevV1_, bspline);
            auto project = proj.Point(1);
            poles.SetValue(1, 1, project);
            //
            proj.Init(prevV1, bspline);
            project = proj.Point(1);
            poles.SetValue(1, 2, project);
            //
            proj.Init(prevV2, bspline);
            project = proj.Point(1);
            poles.SetValue(1, 3, project);
            //
            proj.Init(prevV3, bspline);
            project = proj.Point(1);
            poles.SetValue(1, 4, project);
            //
            proj.Init(prevV2_, bspline);
            project = proj.Point(1);
            poles.SetValue(2, 1, project);
            //
            poles.SetValue(2, 2, bspline->Pole(i * 3 + 1, j * 3 + 1));
            poles.SetValue(2, 3, bspline->Pole(i * 3 + 1, j * 3 + 2));
            poles.SetValue(2, 4, bspline->Pole(i * 3 + 1, j * 3 + 3));
            //
            proj.Init(prevV3_, bspline);
            project = proj.Point(1);
            poles.SetValue(3, 1, project);
            //
            poles.SetValue(3, 2, bspline->Pole(i * 3 + 2, 4));
            poles.SetValue(3, 3, bspline->Pole(i * 3 + 2, 5));
            poles.SetValue(3, 4, bspline->Pole(i * 3 + 2, 6));
            //
            proj.Init(prevV4_, bspline);
            project = proj.Point(1);
            poles.SetValue(4, 1, project);
            //
            poles.SetValue(4, 2, bspline->Pole(i * 3 + 3, j * 3 + 1));
            poles.SetValue(4, 3, bspline->Pole(i * 3 + 3, j * 3 + 2));
            poles.SetValue(4, 4, bspline->Pole(i * 3 + 3, j * 3 + 3));

            //
            prevV1 = bspline->Pole(1, j * 3 + 3).XYZ();
            prevV2 = bspline->Pole(2, j * 3 + 3).XYZ();
            prevV3 = bspline->Pole(3, j * 3 + 3).XYZ();
            prevV4 = bspline->Pole(4, j * 3 + 3).XYZ();

          }
        }
        //
        Handle(asiAlgo_BSplineSurface) algoBspline = new asiAlgo_BSplineSurface(bspline->Poles(), bspline->UKnots(),
          bspline->VKnots(), bspline->UMultiplicities(),
          bspline->VMultiplicities(), bspline->UDegree(), bspline->VDegree());
        //
        gp_Pnt2d lUV = sas.ValueOfUV(poles.Value(1, 1), 1e-15);
        gp_Pnt2d rUV = sas.ValueOfUV(poles.Value(4, 4), 1e-15);
        //
        algoBspline->Segment(lUV.X(), rUV.X(), lUV.Y(), rUV.Y(), 3, 3);
        //
        uMults.SetValue(1, algoBspline->UMultiplicity(1));
        uMults.SetValue(2, algoBspline->UMultiplicity(2));
        vMults.SetValue(1, algoBspline->VMultiplicity(1));
        vMults.SetValue(2, algoBspline->VMultiplicity(2));
        //
        Handle(Geom_BSplineSurface) segment = new Geom_BSplineSurface(poles, uKnots, vKnots, uMults, vMults, 3, 3);
        m_bSplineSurfaces[i].push_back(segment);
        //m_plotter.DRAW_SURFACE(segment, Color_Red, "bspl_Seg");
      }
      isFirst = false;
    }
  }
  /*---------------------------
  * Concat segments 
  ---------------------------*/
  std::vector<std::vector<mobius::t_ptr<mobius::geom_BSplineSurface>>> mobBSurfaces;
  mobBSurfaces.resize(m_bSplineSurfaces.size());
  int k = 0;
  for (auto surfs : m_bSplineSurfaces)
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
  auto res = spln[0];
  for ( size_t j = 1; j < spln.size(); ++j )
  {
    if (!res->ConcatenateCompatible(spln[j], true))
      res->ConcatenateCompatible(spln[j], false);
  }
  Handle(Geom_BezierSurface) bezier = new Geom_BezierSurface(mobius::cascade::GetOpenCascadeBSurface(res)->Poles());
  bspline = mobius::cascade::GetOpenCascadeBSurface(res);
  //m_plotter.DRAW_SURFACE(mobius::cascade::GetOpenCascadeBSurface(res), Color_Red, "bspl_Res");
  m_bSplineSurfaces.clear();
}

void asiAlgo_ConvertToBezier::CreateSurfaceU2(Handle(Geom_BSplineSurface)& bspline)
{
  CreateSurfaceU0(bspline);
}

void asiAlgo_ConvertToBezier::CreateSurfaceU3(Handle(Geom_BSplineSurface)& bspline)
{
  CreateSurfaceU0(bspline);
}

//----------------------------------------------------------------------------------------------

Handle(Geom_BSplineSurface) asiAlgo_ConvertToBezier::SplitBezier(const Handle(Geom_BezierSurface)& surface)
{
  auto bspline = GeomConvert::SurfaceToBSplineSurface(surface);
  int uNum = 4 - (bspline->NbUPoles() + 1) % 4;
  switch (uNum)
  {
    case 0 : CreateSurfaceU0(bspline); break;
    case 1 : CreateSurfaceU1(bspline); break;
    case 2 : CreateSurfaceU2(bspline); break;
    case 3 : CreateSurfaceU3(bspline); break;
  }
  return bspline;
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
    for (auto v = surfaces->VJointValues()->begin(); v != surfaces->VJointValues()->end(); ++v)
    {
      Handle(Geom_BSplineSurface) bspline;
      Handle(Geom_BezierSurface) bezier = Handle(Geom_BezierSurface)::DownCast(surfaces->Patch({*u,*v}));
      if (bezier->NbUPoles() > 3 || bezier->NbUPoles() > 3)
      {
        bspline = SplitBezier(bezier);
      }
      else
      {
        bspline->IncreaseDegree(3, 3);
      }
      auto mobBSpline = mobius::cascade::GetMobiusBSurface(bspline);
      m_plotter.DRAW_SURFACE(bspline, Color_Purple, "bspl_SURFACE");
      m_mobBSurfaces[k].push_back(mobBSpline);
    }
    k++;
  }
  /*---------------------------
  * Concat segments 
  ---------------------------*/
  k = 0;
  for (auto surfs : m_mobBSurfaces)
  {
    bool isFirst = true;
    double prevU = -1;
    double prevV = -1;
    int iter = 0;
    for (auto surf : surfs)
    {
      auto mobBSpline = mobius::cascade::GetOpenCascadeBSurface(surf);
      if (!isFirst)
      {
        tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*mobBSpline.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
      }
      surf = mobius::cascade::GetMobiusBSurface(mobBSpline);
      m_mobBSurfaces[k].push_back(surf);
      isFirst = false;
      prevU = surf->GetKnots_U().back();
      prevV = surf->GetKnots_V().back();
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
  m_bSurface = mobius::cascade::GetOpenCascadeBSurface(res);
  m_bSurface = mobius::cascade::GetOpenCascadeBSurface(res);
  //m_plotter.DRAW_SURFACE(, Color_Purple, "bspl_SURFACE");
  return true;
}

std::vector<Handle(Geom_Curve)> asiAlgo_ConvertToBezier::GetCurves() const
{
  return m_bCurves;
}

Handle(Geom_BSplineSurface) asiAlgo_ConvertToBezier::GetSurface() const
{
  return m_bSurface;
}
