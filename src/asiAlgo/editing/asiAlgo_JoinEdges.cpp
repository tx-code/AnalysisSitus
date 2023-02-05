//-----------------------------------------------------------------------------
// Created on: 30 March 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiAlgo_JoinEdges.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepTools_ReShape.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GCE2d_MakeLine.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAPI_Interpolate.hxx>
#include <GeomAPI.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <gp_Pln.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeConstruct_Curve.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Wire.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#define MaxAllowedGap 1.e-4

//-----------------------------------------------------------------------------

namespace
{
  template<class Pt>
  void GetReversedParameters(const Pt& p11,
                             const Pt& p12,
                             const Pt& p21,
                             const Pt& p22,
                             bool&     isRev1,
                             bool&     isRev2)
  {
    isRev1 = false;
    isRev2 = false;

    double d11   = p11.Distance(p21);
    double d21   = p12.Distance(p21);
    double d12   = p11.Distance(p22);
    double d22   = p22.Distance(p12);
    double Dmin1 = Min(d11, d21);
    double Dmin2 = Min(d12, d22);
    //
    if ( fabs(Dmin1 - Dmin2) <= Precision::Confusion() || Dmin2 > Dmin1 )
    {
      isRev1 = (d11 < d21 ? true : false);
    }
    else if ( Dmin2 < Dmin1 )
    {
      isRev1 = (d12 < d22 ? true : false);
      isRev2 = true;
    }
  }

  TopoDS_Wire FixWire(const TopoDS_Wire& theWire)
  {
    ShapeFix_Wire wireFixer;
    wireFixer.Load(theWire);
    wireFixer.FixClosed();
    wireFixer.FixGaps2d();
    wireFixer.FixConnected();
    return wireFixer.Wire();
  }

  template<class HCurve>
  static inline void SegmentCurve(HCurve&      curve,
                                  const double first,
                                  const double last)
  {
    const double pconfusion = Precision::PConfusion();

    if ( curve->FirstParameter() < first - pconfusion ||
         curve->LastParameter() > last + pconfusion)
    {
      if ( curve->IsPeriodic() )
      {
        curve->Segment(first, last);
      }
      else
      {
        curve->Segment( Max(curve->FirstParameter(), first),
                        Min(curve->LastParameter(), last) );
      }
  } 
}

  template<class HCurve>
  HCurve GetCurveCopy(const HCurve&            curve,
                      double&                  first,
                      double&                  last,
                      const TopAbs_Orientation orient)
  {
    if ( orient == TopAbs_REVERSED )
    {
      double cf = first;
      first = curve->ReversedParameter(last);
      last  = curve->ReversedParameter(cf);
      return curve->Reversed();
    }
    return HCurve::DownCast( curve->Copy() );
  }

  // Joins the passed 2D curves.
  bool JoinCurves(const Handle(Geom2d_Curve)& c2d1,
                  const Handle(Geom2d_Curve)& c2d2,
                  const TopAbs_Orientation    Orient1,
                  const TopAbs_Orientation    Orient2,
                  double&                     first1,
                  double&                     last1,
                  double&                     first2,
                  double&                     last2,
                  Handle(Geom2d_Curve)&       C2dOut,
                  ActAPI_ProgressEntry        progress,
                  ActAPI_PlotterEntry         plotter)
  {
    //plotter.REDRAW_CURVE2D("c2d1", c2d1, Color_White);
    //plotter.REDRAW_CURVE2D("c2d2", c2d2, Color_White);

    // Make working copies of the curves.
    Handle(Geom2d_Curve) c2d1_copy, c2d2_copy;
    c2d1_copy = GetCurveCopy(c2d1, first1, last1, Orient1);
    c2d2_copy = GetCurveCopy(c2d2, first2, last2, Orient2);

    //plotter.REDRAW_CURVE2D("c2d1_copy", c2d1_copy, Color_White);
    //plotter.REDRAW_CURVE2D("c2d2_copy", c2d2_copy, Color_White);

    ShapeConstruct_Curve scc;

    // Convert to splines.
    Handle(Geom2d_BSplineCurve)
      bsplc12d = scc.ConvertToBSpline( c2d1_copy, first1, last1, Precision::Confusion() );
    //
    Handle(Geom2d_BSplineCurve)
      bsplc22d = scc.ConvertToBSpline( c2d2_copy, first2, last2, Precision::Confusion() );
    //
    if ( bsplc12d.IsNull() || bsplc22d.IsNull() )
    {
      progress.SendLogMessage(LogErr(Normal) << "Failed to convert p-curves to splines.");
      return false;
    }

    //plotter.REDRAW_CURVE2D("c2d1_copy_conv", c2d1_copy, Color_White);
    //plotter.REDRAW_CURVE2D("c2d2_copy_conv", c2d2_copy, Color_White);

    // Make sure that curves are properly trimmed.
    SegmentCurve(bsplc12d, first1, last1);
    SegmentCurve(bsplc22d, first2, last2);

    gp_Pnt2d pp112d = bsplc12d->Pole(1).XY();
    gp_Pnt2d pp122d = bsplc12d->Pole(bsplc12d->NbPoles()).XY();
    //
    gp_Pnt2d pp212d = bsplc22d->Pole(1).XY();
    gp_Pnt2d pp222d = bsplc22d->Pole(bsplc22d->NbPoles()).XY();

    // Check the geometric order of curves after they have been reapproximated
    // taking into account the orientations of their edges.
    bool isRev1, isRev2;
    GetReversedParameters(pp112d, pp122d, pp212d, pp222d, isRev1, isRev2);

    int iLastOnC1  = isRev1 ? 1                   : bsplc12d->NbPoles();
    int iFirstOnC2 = isRev2 ? bsplc22d->NbPoles() : 1;

    //plotter.REDRAW_CURVE2D("c2d1_copy_segmented", c2d1_copy, Color_White);
    //plotter.REDRAW_CURVE2D("c2d2_copy_segmented", c2d2_copy, Color_White);

    // Check for a gap.
    gp_XY ptFirst = bsplc12d->Pole(iLastOnC1).XY();
    gp_XY ptNext  = bsplc22d->Pole(iFirstOnC2).XY();
    //
    if ( (ptFirst - ptNext).Modulus() > MaxAllowedGap )
    {
      plotter.REDRAW_POINT("ptFirst", ptFirst, Color_Red);
      plotter.REDRAW_POINT("ptNext",  ptNext,  Color_Red);

      progress.SendLogMessage( LogErr(Normal) << "Edges do not touch each other. "
                                                 "The detected gap is %1 mm while "
                                                 "the max allowed gap is %2 mm."
                                              << (ptFirst - ptNext).Modulus()
                                              << MaxAllowedGap );
      return false;
    }

    // Average the connection point to have mean coordinates on stitching.
    gp_Pnt2d pmid1 = 0.5 * (ptFirst + ptNext);
    //
    bsplc12d->SetPole(iLastOnC1,  pmid1);
    bsplc22d->SetPole(iFirstOnC2, pmid1);

    // abv 01 Sep 99: Geom2dConvert ALWAYS performs reparametrisation of the
    // second curve before merging; this is quite not suitable
    // Use 3d tool instead

    // Convert curves to 3D in order to call the "right" tool.
    gp_Pln uvPln( gp::Origin(), gp::DZ() );
    //
    Handle(Geom_BSplineCurve)
      bspl1 = Handle(Geom_BSplineCurve)::DownCast( GeomAPI::To3d(bsplc12d, uvPln) );
    //
    Handle(Geom_BSplineCurve)
      bspl2 = Handle(Geom_BSplineCurve)::DownCast( GeomAPI::To3d(bsplc22d, uvPln) );

    // Concatenate curves.
    GeomConvert_CompCurveToBSplineCurve connect2d(bspl1);
    //
    const bool after = true;
    //
    if ( !connect2d.Add(bspl2, Precision::PConfusion(), after, false) )
      return false;

    // Get back to UV.
    C2dOut = GeomAPI::To2d(connect2d.BSplineCurve(), uvPln);
  
    return true;
  }
}

//-----------------------------------------------------------------------------

//! Ctor.
asiAlgo_JoinEdges::asiAlgo_JoinEdges(const TopoDS_Shape&  masterCAD,
                                     ActAPI_ProgressEntry progress,
                                     ActAPI_PlotterEntry  plotter)
{
  m_master   = masterCAD;
  m_progress = progress;
  m_plotter  = plotter;
}

//-----------------------------------------------------------------------------

//! Joins the given edges in the master model.
//! \param edges [in] edges to join.
//! \param face  [in] base face.
//! \return true in case of success, false -- otherwise.
bool asiAlgo_JoinEdges::Perform(const TopTools_IndexedMapOfShape& edges,
                                const TopoDS_Face&                face)
{
  if ( edges.Extent() != 2 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Only a pair of edges is accepted.");
    return false;
  }

  // Join edges
  TopoDS_Edge E1, E2, newE;
  this->chooseOrder(edges, E1, E2);
  //
  if ( !this->joinEdges(E1, E2, face, newE) )
  {
    return false;
  }

  // Build a new wire. It will be re-ordered properly by healing at the end
  Handle(ShapeExtend_WireData) WD = new ShapeExtend_WireData;
  for ( TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    const TopoDS_Shape& E = exp.Current();

    if ( E.IsPartner(E1) || E.IsPartner(E2) )
      continue;

    WD->Add( exp.Current() );
  }
  WD->Add( newE );
  //
  TopoDS_Wire W = FixWire( WD->Wire() );

  // Build another face
  BRepBuilderAPI_MakeFace mkFace( BRep_Tool::Surface(face), W, Precision::Confusion() );
  TopoDS_Face newFace = mkFace.Face();
  //
  if ( face.Orientation() == TopAbs_REVERSED )
    newFace.Reverse();

  // Change old face with the reconstructed one
  Handle(BRepTools_ReShape) ReShape = new BRepTools_ReShape;
  ReShape->Replace(face, newFace);
  m_result = ReShape->Apply(m_master);

  m_progress.SendLogMessage(LogInfo(Normal) << "Edges have been concatenated successfully.");

  return true; // Success
}

//-----------------------------------------------------------------------------

//! Finds a correct geometric order in which the edges should follow one
//! after another in order to be joined properly.
//! \param edges   [in] initial collection of edges.
//! \param eFirst  [in] first edge.
//! \param eSecond [in] second edge.
void asiAlgo_JoinEdges::chooseOrder(const TopTools_IndexedMapOfShape& edges,
                                    TopoDS_Edge&                      eFirst,
                                    TopoDS_Edge&                      eSecond) const
{
  const TopoDS_Edge& eCandidate1 = TopoDS::Edge( edges(1) );
  const TopoDS_Edge& eCandidate2 = TopoDS::Edge( edges(2) );

  double f1, l1, f2, l2;
  Handle(Geom_Curve) cCandidate1 = BRep_Tool::Curve(eCandidate1, f1, l1);
  Handle(Geom_Curve) cCandidate2 = BRep_Tool::Curve(eCandidate2, f2, l2);

  const double dist1 = cCandidate1->Value(l1).Distance( cCandidate2->Value(f2) );
  const double dist2 = cCandidate2->Value(l2).Distance( cCandidate1->Value(f1) );

  if ( dist1 < dist2 )
  {
    eFirst  = eCandidate1;
    eSecond = eCandidate2;
  }
  else
  {
    eFirst  = eCandidate2;
    eSecond = eCandidate1;
  }
}

//-----------------------------------------------------------------------------

//! Joins a couple of edges into a single edge.
//! \param edge_A  [in]  first edge to join.
//! \param edge_B  [in]  second edge to join.
//! \param face    [in]  base face.
//! \param eResult [out] result edge to fill gap.
//! \return true in case of success, false -- otherwise.
bool asiAlgo_JoinEdges::joinEdges(const TopoDS_Edge& eFirst,
                                  const TopoDS_Edge& eSecond,
                                  const TopoDS_Face& face,
                                  TopoDS_Edge&       eResult) const
{
  ShapeAnalysis_Edge sae;

  Handle(Geom2d_Curve) c2d1, c2d2;
  double first1, last1, first2, last2;

  // Get orientation of the edges on their host faces. We want to have
  // orientation irrelevant of face orientation, as we are going to
  // work in the parametric domain.
  TopAbs_Orientation eFirstOri = TopAbs_EXTERNAL, eSecondOri = TopAbs_EXTERNAL;
  //
  for ( TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    if ( exp.Current().IsPartner(eFirst) )
      eFirstOri = exp.Current().Orientation();
    else if ( exp.Current().IsPartner(eSecond) )
      eSecondOri = exp.Current().Orientation();
  }

  // Choose orientation of the bridge edge being built.
  TopoDS_Edge eForward;
  if ( eFirstOri == TopAbs_FORWARD )
    eForward = eFirst;
  else if ( eSecondOri == TopAbs_FORWARD )
    eForward = eSecond;

  if ( !sae.PCurve(eFirst, face, c2d1, first1, last1, false) )
    return false;

  if ( !sae.PCurve(eSecond, face, c2d2, first2, last2, false) )
    return false;

  Handle(Geom2d_Curve) c2dRes;
  //
  if( !::JoinCurves(c2d1,
                    c2d2,
                    eFirstOri,
                    eSecondOri,
                    first1,
                    last1,
                    first2,
                    last2,
                    c2dRes,
                    m_progress,
                    m_plotter) )
  {
    return false;
  }

  /*m_plotter.REDRAW_CURVE2D("c2d1", c2d1, Color_White);
  m_plotter.REDRAW_CURVE2D("c2d2", c2d2, Color_White);
  m_plotter.REDRAW_CURVE2D("c2dRes", c2dRes, Color_Green);*/

  // Get host surface.
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

  // Build edge.
  BRepBuilderAPI_MakeEdge mkEdge(c2dRes, surf);
  eResult = mkEdge.Edge();

  // Recover missing geometry
  ShapeFix_Edge SFE;
  SFE.FixAddCurve3d(eResult);

  return true;
}
