//-----------------------------------------------------------------------------
// Created on: 26 May 2023
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
#include "asiAlgo_JoinSurf.h"

// asiAlgo includes
#include "asiAlgo_BaseCloud.h"
#include "asiAlgo_BuildGordonSurf.h"
#include "asiAlgo_PatchJointAdaptor.h"

// OpenCascade includes
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <Geom_BezierCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomConvert.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeConstruct_Curve.hxx>
#include <TopoDS.hxx>

// TiGL includes
#include <CTiglBSplineAlgorithms.h>

#ifdef USE_MOBIUS
  // Mobius includes.
  #include <mobius/cascade.h>

  using namespace mobius;
#endif

#define ConcatCurvesTol 1.0
#define Prec2d          0.0001

namespace
{
  //! Attempts to improve the parametric quality of the passed
  //! B-spline curve by removing redundant knots.
  void ImproveCurve(Handle(Geom_BSplineCurve)& BS,
                    const double               Tol,
                    const int                  MultMin)

  {
    double    tol = Tol;
    int       Mult, ii;
    const int NbK = BS->NbKnots();

    for ( Mult = BS->Degree(); Mult > MultMin; Mult-- )
    {
      for ( ii = NbK; ii > 1; ii-- )
      {
        if ( BS->Multiplicity(ii) == Mult )
          BS->RemoveKnot(ii, Mult - 1, tol);
      }
    }
  }

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

  template<class Pt> 
  static inline void GetReversedParameters(const Pt& p11, 
                                           const Pt& p12,
                                           const Pt& p21,
                                           const Pt& p22,
                                           bool& isRev1, 
                                           bool& isRev2)
  {
    isRev1 = Standard_False;
    isRev2 = Standard_False;
    //gka protection against crossing seem on second face 

    double d11   = p11.Distance(p21);
    double d21   = p12.Distance(p21);
    double d12   = p11.Distance(p22);
    double d22   = p22.Distance(p12);
    double Dmin1 = Min(d11,d21);
    double Dmin2 = Min(d12,d22);
    if(fabs(Dmin1 - Dmin2) <= Precision::Confusion() || Dmin2 > Dmin1) {
      isRev1 = (d11 < d21 ? Standard_True : Standard_False);
    }
    else if(Dmin2 < Dmin1) {
      isRev1 = (d12 < d22 ? Standard_True  : Standard_False);
      isRev2 = Standard_True;
    }
  }

  template<class Curve>
  static inline Curve GetCurveCopy(const Curve&              curve, 
                                   double&                   first,
                                   double&                   last, 
                                   const TopAbs_Orientation& orient)
  {
    if ( orient == TopAbs_REVERSED ) {
      double cf = first;
      first = curve->ReversedParameter ( last );
      last = curve->ReversedParameter ( cf );
      return curve->Reversed();
    }
    return Curve::DownCast(curve->Copy());
  }

  // Joins the passed 3D curves.
  bool ConcatCurves(const Handle(Geom_Curve)& ac3d1,
                    const Handle(Geom_Curve)& ac3d2,
                    const TopAbs_Orientation& Orient1,
                    const TopAbs_Orientation& Orient2,
                    double&                   first1,
                    double&                   last1,
                    double&                   first2,
                    double&                   last2,
                    Handle(Geom_Curve)&       c3dOut,
                    bool&                     isRev1,
                    bool&                     isRev2)

  {
    Handle(Geom_Curve) c3d1,c3d2;

    c3d1 = GetCurveCopy ( ac3d1, first1, last1, Orient1 );
    c3d2 = GetCurveCopy ( ac3d2, first2, last2, Orient2 );

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

    GetReversedParameters(pp11,pp12,pp21,pp22,isRev1,isRev2);

    if(isRev1) {
      bsplc1->Reverse();
    }
    if(isRev2)
      bsplc2->Reverse();

    gp_Pnt pmid = 0.5 * ( bsplc1->Pole(bsplc1->NbPoles()).XYZ() + bsplc2->Pole(1).XYZ() );
    bsplc1->SetPole(bsplc1->NbPoles(), pmid);
    bsplc2->SetPole(1, pmid);
    GeomConvert_CompCurveToBSplineCurve connect3d(bsplc1);
    if(!connect3d.Add(bsplc2,Precision::Confusion(), after, false))
    {
      return false;
    }
    c3dOut = connect3d.BSplineCurve();
    return true;
  }

  //! Concatenates the passed two curves by constructing a smooth transitioning
  //! spline between them.
  Handle(Geom_Curve) JoinCurves(const Handle(Geom_Curve)& C1,
                                const Handle(Geom_Curve)& C2,
                                ActAPI_PlotterEntry       plotter)
  {
    struct t_endPoint
    {
      double param;
      gp_XYZ P;
      gp_XYZ T;
      bool   reverse;

      t_endPoint() : param(0.), reverse(false) {}
    };

    const double f1 = C1->FirstParameter();
    const double l1 = C1->LastParameter();
    const double f2 = C2->FirstParameter();
    const double l2 = C2->LastParameter();

    gp_Pnt Pf1, Pl1, Pf2, Pl2;
    gp_Vec Tf1, Tl1, Tf2, Tl2;
    //
    C1->D1(f1, Pf1, Tf1);
    C1->D1(l1, Pl1, Tl1);
    C2->D1(f2, Pf2, Tf2);
    C2->D1(l2, Pl2, Tl2);

    const double d_ff = Pf1.Distance(Pf2);
    const double d_fl = Pf1.Distance(Pl2);
    const double d_lf = Pl1.Distance(Pf2);
    const double d_ll = Pl1.Distance(Pl2);

    // We normalize tangent vectors here although it is necessary
    // to use the actual distance between the endpoints to choose
    // the proper scale factor for those vectors. This is done later.
    Tf1.Normalize();
    Tl1.Normalize();
    Tf2.Normalize();
    Tl2.Normalize();

    // Find extremities to connect.
    t_endPoint onC1, onC2;
    //
    if ( (d_ff < d_fl) && (d_ff < d_lf) && (d_ff < d_ll) ) // Case 1: C1(f) --- C2(f)
    {
      onC1.param = f1;
      onC1.P     = Pf1.XYZ();
      onC1.T     = Tf1.XYZ();

      onC2.param = f2;
      onC2.P     = Pf2.XYZ();
      onC2.T     = Tf2.XYZ();
    }
    else if ( (d_fl < d_ff) && (d_fl < d_lf) && (d_fl < d_ll) ) // Case 2: C1(f) --- C2(l)
    {
      onC1.param   = f1;
      onC1.P       = Pf1.XYZ();
      onC1.T       = Tf1.XYZ();
      onC1.reverse = true;

      onC2.param   = l2;
      onC2.P       = Pl2.XYZ();
      onC2.T       = Tl2.XYZ();
      onC2.reverse = true;
    }
    else if ( (d_lf < d_ff) && (d_lf < d_fl) && (d_lf < d_ll) ) // Case 3: C1(l) --- C2(f)
    {
      onC1.param = l1;
      onC1.P     = Pl1.XYZ();
      onC1.T     = Tl1.XYZ();

      onC2.param = f2;
      onC2.P     = Pf2.XYZ();
      onC2.T     = Tf2.XYZ();
    }
    else if ( (d_ll < d_ff) && (d_ll < d_fl) && (d_ll < d_lf) ) // Case 4: C1(l) --- C2(l)
    {
      onC1.param = l1;
      onC1.P     = Pl1.XYZ();
      onC1.T     = Tl1.XYZ();

      onC2.param   = l2;
      onC2.P       = Pl2.XYZ();
      onC2.T       = Tl2.XYZ();
      onC2.reverse = true;
    }

    // Use the distance between the endpoints to calibrate tengency constraints.
    const double bzGap = (onC2.P - onC1.P).Modulus();
    //
    onC1.T *= bzGap*0.25;
    onC2.T *= bzGap*0.25;

    // Construct 3-degree Bezier.
    TColgp_Array1OfPnt bzPoles(1, 4);
    //
    bzPoles(1) = onC1.P;
    bzPoles(2) = onC1.P + (onC1.reverse ? -1 :  1) * onC1.T;
    bzPoles(3) = onC2.P + (onC2.reverse ?  1 : -1) * onC2.T;
    bzPoles(4) = onC2.P;
    //
    Handle(Geom_Curve) bzSegment = new Geom_BezierCurve(bzPoles);

    // Prepare all segments to concatenate.
    Handle(Geom_BSplineCurve) seg1 = Handle(Geom_BSplineCurve)::DownCast( C1->Copy() );
    Handle(Geom_BSplineCurve) seg2 = GeomConvert::CurveToBSplineCurve(bzSegment);
    Handle(Geom_BSplineCurve) seg3 = Handle(Geom_BSplineCurve)::DownCast( C2->Copy() );
    //
    if ( onC1.reverse )
      seg1->Reverse();
    //
    if ( onC2.reverse )
      seg3->Reverse();

    plotter.DRAW_CURVE( C1,   Color_Khaki,  true, "C1" );
    plotter.DRAW_CURVE( C2,   Color_Khaki,  true, "C2" );
    plotter.DRAW_CURVE( seg2, Color_Yellow, true, "C12_bzSegment" );

    plotter.DRAW_CURVE( seg1, Color_Khaki,  true, "seg1" );
    plotter.DRAW_CURVE( seg3, Color_Khaki,  true, "seg3" );

    Handle(Geom_Curve) C12;
    {
      double ff1 = seg1->FirstParameter(), ll1 = seg1->LastParameter();
      double ff2 = seg2->FirstParameter(), ll2 = seg2->LastParameter();
      bool rev1, rev2;
      ConcatCurves(seg1, seg2, TopAbs_FORWARD, TopAbs_FORWARD, ff1, ll1, ff2, ll2, C12, rev1, rev2);
      //
      plotter.DRAW_CURVE( C12, Color_Khaki, true, "C12" );
    }

    Handle(Geom_Curve) C123;
    {
      double ff1 = C12->FirstParameter(), ll1 = C12->LastParameter();
      double ff2 = seg3->FirstParameter(), ll2 = seg3->LastParameter();
      bool rev1, rev2;
      ConcatCurves(C12, seg3, TopAbs_FORWARD, TopAbs_FORWARD, ff1, ll1, ff2, ll2, C123, rev1, rev2);
      //
      plotter.DRAW_CURVE( C123, Color_Khaki, true, "C123" );
    }

    // Concatenate.
    //t_ptr<t_bcurve> mbSeg1 = cascade::GetMobiusBCurve(seg1);
    //t_ptr<t_bcurve> mbSeg2 = cascade::GetMobiusBCurve(seg2);
    //t_ptr<t_bcurve> mbSeg3 = cascade::GetMobiusBCurve(seg3);
    ////
    //mbSeg1->ConcatenateCompatible(mbSeg2);
    //mbSeg1->ConcatenateCompatible(mbSeg3);
    //
    //Handle(Geom_BSplineCurve) concatC1C2 = cascade::GetOpenCascadeBCurve(mbSeg1);

    /*std::vector<Handle(Geom_BSplineCurve)> segments = {seg1, seg2, seg3};

    Handle(Geom_BSplineCurve) concatC1C2 = tigl::CTiglBSplineAlgorithms::concatCurves(segments, false);*/

    Handle(Geom_BSplineCurve)
      concatC1C2 = Handle(Geom_BSplineCurve)::DownCast(C123);

    // Get rid of C0 defects at joints.
    ImproveCurve(concatC1C2, ConcatCurvesTol, 1);

    //plotter.DRAW_CURVE( concatC1C2, Color_Khaki, true, "concatC1C2" );

    return concatC1C2;
  }

  //! Prepares a uniform distribution of parameters in [`tMin`,`tMax`] range
  //! with the `tStep` increment between the successive values.
  void SampleParameters(const double         tMin,
                        const double         tMax,
                        const double         tStep,
                        std::vector<double>& T)
  {
    double t     = tMin;
    bool   tStop = false;
    //
    while ( !tStop )
    {
      if ( (t > tMax) || Abs(t - tMax) < 1e-6 )
      {
        t     = tMax;
        tStop = true;
      }

      T.push_back(t);
      t += tStep;
    }
  }
}

//-----------------------------------------------------------------------------

asiAlgo_JoinSurf::asiAlgo_JoinSurf(ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_iNumProfilesS1  ( 2 ),
  m_iNumProfilesS2  ( 2 ),
  m_iNumGuides      ( 2 ),
  m_fBndOffset      ( 1.0 ),
  m_fMaxError       ( 0. )
{}

//-----------------------------------------------------------------------------

bool asiAlgo_JoinSurf::Build(const Handle(TopTools_HSequenceOfShape)& inFaces,
                             Handle(Geom_BSplineSurface)&             outSupport,
                             TopoDS_Face&                             outFace)
{
  const double projPrec = 1e-4; // Precision of point-surface inversion (projection).

  std::vector<Handle(Geom_Curve)> profileCurves;
  std::vector<Handle(Geom_Curve)> guideCurves;

  /* =======================
   *  Apply contract checks.
   * ======================= */

  if ( inFaces->Size() != 2 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Only two faces can be passed to JOIN.");
    return false;
  }

  const TopoDS_Face& F1 = TopoDS::Face( inFaces->First() );
  const TopoDS_Face& F2 = TopoDS::Face( inFaces->Last() );

  // Check surface type: only splines are currently allowed.
  Handle(Geom_BSplineSurface)
    S1 = Handle(Geom_BSplineSurface)::DownCast( BRep_Tool::Surface(F1)->Copy() ); // Make a copy to avoid affecting the input model.
  //
  if ( S1.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "The input face F1 is not a B-spline surface.");
    return false;
  }
  //
  Handle(Geom_BSplineSurface)
    S2 = Handle(Geom_BSplineSurface)::DownCast( BRep_Tool::Surface(F2)->Copy() ); // Make a copy to avoid affecting the input model.
  //
  if ( S2.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "The input face F2 is not a B-spline surface.");
    return false;
  }

  m_plotter.REDRAW_SURFACE("S1", S1, Color_Green);
  m_plotter.REDRAW_SURFACE("S2", S2, Color_Blue);

  // Get parametric bounds of S1.
  double S1_uMin, S1_uMax, S1_vMin, S1_vMax;
  S1->Bounds(S1_uMin, S1_uMax, S1_vMin, S1_vMax);

  // Parametric bounds of S2.
  double S2_uMin, S2_uMax, S2_vMin, S2_vMax;

  /* =====================
   *  Analyze joint curve.
   * ===================== */

  struct t_iso
  {
    bool   isIsoU;
    bool   isMin;
    double param;

    t_iso() : isIsoU(false), isMin(false), param(0.) {}
  };

  t_iso jointOnS1, jointOnS2;

  std::vector<double>
    bndParams = { S1_uMin, S1_uMax, S1_vMin, S1_vMax };

  // Candidate joint curves.
  std::vector<Handle(Geom_Curve)>
    jCurves = { S1->UIso(bndParams[0]),
                S1->UIso(bndParams[1]),
                S1->VIso(bndParams[2]),
                S1->VIso(bndParams[3]) };

  int j = -1;

  for ( int k = 0; k < 4; ++k )
  {
    m_plotter.DRAW_CURVE(jCurves[k], Color_Red, true, "jCurve");

    // Analyze basic local properties at coedge.
    bool isSurfGoesU, isLeftBound;
    //
    if ( !asiAlgo_PatchJointAdaptor::AnalyzeJoint(jCurves[k], S2,
                                                  isSurfGoesU, isLeftBound,
                                                  S2_uMin, S2_uMax, S2_vMin, S2_vMax) )
      continue; // Failure.

    j = k;

    // Joint curve props on S1.
    jointOnS1.isMin  = ((k == 0) || (k == 2));
    jointOnS1.isIsoU = ((k == 0) || (k == 1));
    jointOnS1.param  = bndParams[k];

    // Joint curve props on S2.
    jointOnS2.isMin  =  isLeftBound;
    jointOnS2.isIsoU = !isSurfGoesU;
    //
    if ( jointOnS2.isIsoU && isLeftBound )
      jointOnS2.param = S2_uMin;
    //
    else if ( jointOnS2.isIsoU && !isLeftBound )
      jointOnS2.param = S2_uMax;
    //
    else if ( !jointOnS2.isIsoU && isLeftBound )
      jointOnS2.param = S2_vMin;
    //
    else
      jointOnS2.param = S2_vMax;

    break;
  }

  if ( j == -1 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot find a common curve between surfaces.");
    return false;
  }

  m_plotter.DRAW_CURVE(jointOnS1.isIsoU ? S1->UIso(jointOnS1.param) : S1->VIso(jointOnS1.param), Color_Red, true, "jCurve_S1");
  m_plotter.DRAW_CURVE(jointOnS2.isIsoU ? S2->UIso(jointOnS2.param) : S2->VIso(jointOnS2.param), Color_Red, true, "jCurve_S2");

  /* ====================================================
   *  Build guide curves by discretizing the joint curve.
   * ==================================================== */

  // Discretize the joint curve with a uniform curvilinear step.
  GeomAdaptor_Curve gac(jCurves[j]);
  GCPnts_QuasiUniformAbscissa Defl(gac, m_iNumGuides);
  //
  if ( !Defl.IsDone() )
    return false;

  // Fill a set of discretization points along the joint curve.
  Handle(asiAlgo_BaseCloud<double>) guideOrigins = new asiAlgo_BaseCloud<double>;
  //
  for ( int i = 1; i <= m_iNumGuides; ++i )
  {
    const double param = Defl.Parameter(i);
    gp_XYZ       P     = gac.Value(param).XYZ();
    //
    guideOrigins->AddElement(P);
  }

  m_plotter.REDRAW_POINTS("guideOrigins", guideOrigins->GetCoordsArray(), Color_Yellow);

  // Get parameters of the origin points on the surface S1 and S2 by inverting them.
  std::vector<t_iso> guideParams_S1, guideParams_S2;
  //
  ShapeAnalysis_Surface sas_S1(S1);
  ShapeAnalysis_Surface sas_S2(S2);
  //
  for ( int i = 1; i <= m_iNumGuides; ++i )
  {
    gp_XYZ P = guideOrigins->GetElement(i - 1);

    gp_Pnt2d uv_S1 = sas_S1.ValueOfUV(P, projPrec);
    gp_Pnt2d uv_S2 = sas_S2.ValueOfUV(P, projPrec);

    double p_S1, p_S2;
    //
    if ( jointOnS1.isIsoU )
    {
      p_S1 = uv_S1.Y(); // Take V.

      t_iso guideParam;
      guideParam.isIsoU = false;
      guideParam.param  = p_S1;
      //
      guideParams_S1.push_back(guideParam);
    }
    else
    {
      p_S1 = uv_S1.X(); // Take U.

      t_iso guideParam;
      guideParam.isIsoU = true;
      guideParam.param  = p_S1;
      //
      guideParams_S1.push_back(guideParam);
    }
    //
    if ( jointOnS2.isIsoU )
    {
      p_S2 = uv_S2.Y(); // Take V.

      t_iso guideParam;
      guideParam.isIsoU = false;
      guideParam.param  = p_S2;
      //
      guideParams_S2.push_back(guideParam);
    }
    else
    {
      p_S2 = uv_S2.X(); // Take U.

      t_iso guideParam;
      guideParam.isIsoU = true;
      guideParam.param  = p_S2;
      //
      guideParams_S2.push_back(guideParam);
    }
  }

  /* ==============================
   *  Trim surfaces near the joint.
   * ============================== */

  if ( m_fBndOffset > Precision::Confusion() )
  {
    // Trim S1.
    if ( jointOnS1.isIsoU && jointOnS1.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S1_uMin + m_fBndOffset > S1_uMax) || Abs(S1_uMin + m_fBndOffset - S1_uMax) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S1."
                                                 << m_fBndOffset);
        return false;
      }

      S1->Segment(S1_uMin + m_fBndOffset, S1_uMax, S1_vMin, S1_vMax);
    }
    //
    else if ( jointOnS1.isIsoU && !jointOnS1.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S1_uMax - m_fBndOffset < S1_uMin) || Abs(S1_uMax - m_fBndOffset - S1_uMin) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S1."
                                                 << m_fBndOffset);
        return false;
      }

      S1->Segment(S1_uMin, S1_uMax - m_fBndOffset, S1_vMin, S1_vMax);
    }
    //
    else if ( !jointOnS1.isIsoU && jointOnS1.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S1_vMin + m_fBndOffset > S1_vMax) || Abs(S1_vMin + m_fBndOffset - S1_vMax) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S1."
                                                 << m_fBndOffset);
        return false;
      }

      S1->Segment(S1_uMin, S1_uMax, S1_vMin + m_fBndOffset, S1_vMax);
    }
    //
    else if ( !jointOnS1.isIsoU && !jointOnS1.isMin )
    {
      // Check if the requested offset value is legal.
      if ( S1_vMax - m_fBndOffset < S1_vMin )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S1."
                                                 << m_fBndOffset);
        return false;
      }

      S1->Segment(S1_uMin, S1_uMax, S1_vMin, S1_vMax - m_fBndOffset);
    }

    // Trim S2.
    if ( jointOnS2.isIsoU && jointOnS2.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S2_uMin + m_fBndOffset > S2_uMax) || Abs(S2_uMin + m_fBndOffset - S2_uMax) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S2."
                                                 << m_fBndOffset);
        return false;
      }

      S2->Segment(S2_uMin + m_fBndOffset, S2_uMax, S2_vMin, S2_vMax);
    }
    //
    else if ( jointOnS2.isIsoU && !jointOnS2.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S2_uMax - m_fBndOffset < S2_uMin) || Abs(S2_uMax - m_fBndOffset - S2_uMin) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S2."
                                                 << m_fBndOffset);
        return false;
      }

      S2->Segment(S2_uMin, S2_uMax - m_fBndOffset, S1_vMin, S2_vMax);
    }
    //
    else if ( !jointOnS2.isIsoU && jointOnS2.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S2_vMin + m_fBndOffset > S2_vMax) || Abs(S2_vMin + m_fBndOffset - S2_vMax) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S2."
                                                 << m_fBndOffset);
        return false;
      }

      S2->Segment(S2_uMin, S2_uMax, S1_vMin + m_fBndOffset, S2_vMax);
    }
    //
    else if ( !jointOnS2.isIsoU && !jointOnS2.isMin )
    {
      // Check if the requested offset value is legal.
      if ( (S2_vMax - m_fBndOffset < S2_vMin) || Abs(S2_vMax - m_fBndOffset - S2_vMin) < Prec2d )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Offset value %1 is too large for the parametric bounds of the surface S2."
                                                 << m_fBndOffset);
        return false;
      }

      S2->Segment(S2_uMin, S2_uMax, S2_vMin, S2_vMax - m_fBndOffset);
    }

    // Update surface bounds after trimming.
    S1->Bounds(S1_uMin, S1_uMax, S1_vMin, S1_vMax);
    S2->Bounds(S2_uMin, S2_uMax, S2_vMin, S2_vMax);
  }

  m_plotter.REDRAW_SURFACE("TS1", S1, Color_Green);
  m_plotter.REDRAW_SURFACE("TS2", S2, Color_Blue);

  /* =======================================
   *  Generate profiles "parallel" to joint.
   * ======================================= */

  // Parametric steps to generate isolines.
  const double S1_uStep = (S1_uMax - S1_uMin) / (m_iNumProfilesS1 + 1);
  const double S1_vStep = (S1_vMax - S1_vMin) / (m_iNumProfilesS1 + 1);
  const double S2_uStep = (S2_uMax - S2_uMin) / (m_iNumProfilesS2 + 1);
  const double S2_vStep = (S2_vMax - S2_vMin) / (m_iNumProfilesS2 + 1);

  // Choose parameter values.
  std::vector<double> S1_U, S1_V, S2_U, S2_V;
  //
  ::SampleParameters(S1_uMin, S1_uMax, S1_uStep, S1_U);
  ::SampleParameters(S1_vMin, S1_vMax, S1_vStep, S1_V);
  ::SampleParameters(S2_uMin, S2_uMax, S2_uStep, S2_U);
  ::SampleParameters(S2_vMin, S2_vMax, S2_vStep, S2_V);

  if ( jointOnS1.isIsoU )
  {
    for ( const auto u : S1_U )
    {
      profileCurves.push_back( S1->UIso(u) );
    }
  }
  else
  {
    for ( const auto v : S1_V )
    {
      profileCurves.push_back( S1->VIso(v) );
    }
  }

  if ( jointOnS2.isIsoU )
  {
    for ( const auto u : S2_U )
    {
      profileCurves.push_back( S2->UIso(u) );
    }
  }
  else
  {
    for ( const auto v : S2_V )
    {
      profileCurves.push_back( S2->VIso(v) );
    }
  }

  // Visually dump profile curves.
  for ( const auto& profileCurve : profileCurves )
  {
    m_plotter.DRAW_CURVE( profileCurve, Color_Violet, true, "profileCurve" );
  }

  /* ============================================
   *  Build guide curves on the trimmed surfaces.
   * ============================================ */

  std::vector<Handle(Geom_Curve)> guideCurves_S1, guideCurves_S2;

  // For S1.
  for ( const auto& pOnS : guideParams_S1 )
  {
    Handle(Geom_Curve) iso = ( pOnS.isIsoU ? S1->UIso(pOnS.param) : S1->VIso(pOnS.param) );
    //
    guideCurves_S1.push_back(iso);
  }

  // For S2.
  for ( const auto& pOnS : guideParams_S2 )
  {
    Handle(Geom_Curve) iso = ( pOnS.isIsoU ? S2->UIso(pOnS.param) : S2->VIso(pOnS.param) );
    //
    guideCurves_S2.push_back(iso);
  }

  // Visually dump the constructed partial guide curves.
  for ( const auto& guideCurve_S : guideCurves_S1 )
  {
    m_plotter.DRAW_CURVE( guideCurve_S, Color_Green, true, "guideCurve_S1" );
  }
  //
  for ( const auto& guideCurve_S : guideCurves_S2 )
  {
    m_plotter.DRAW_CURVE( guideCurve_S, Color_Snow, true, "guideCurve_S2" );
  }

  /* ===========================================
   *  Connect S1 and S2 guides into solo curves.
   * =========================================== */

  for ( size_t k = 0; k < guideCurves_S1.size(); ++k )
  {
    const Handle(Geom_Curve)& C1 = guideCurves_S1[k];
    const Handle(Geom_Curve)& C2 = guideCurves_S2[k];

    Handle(Geom_Curve) C12 = ::JoinCurves(C1, C2, m_plotter);

    guideCurves.push_back(C12);
  }

  /* ==========================
   *  Make edges out of curves.
   * ========================== */

  std::vector<TopoDS_Edge> guides, profiles;

  for ( const auto& C : profileCurves )
  {
    profiles.push_back( BRepBuilderAPI_MakeEdge(C) );
  }

  for ( const auto& C : guideCurves )
  {
    guides.push_back( BRepBuilderAPI_MakeEdge(C) );
  }

  /* ========================
   *  Build a Gordon surface.
   * ======================== */

  asiAlgo_BuildGordonSurf GORDON(m_progress, nullptr);

  if ( !GORDON.Build(profiles, guides, outSupport, outFace) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot build a Gordon surface.");
    return false;
  }

  m_profiles = profiles;
  m_guides   = guides;

  return true;
}
