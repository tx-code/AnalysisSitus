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

namespace
{
  template<class HPoint>
  void GetReversedParameters(const HPoint& p11,
                             const HPoint& p12,
                             const HPoint& p21,
                             const HPoint& p22,
                             Standard_Boolean& isRev1,
                             Standard_Boolean& isRev2)
  {
    isRev1 = Standard_False;
    isRev2 = Standard_False;
     //gka protection against crossing seem on second face 
  
    Standard_Real d11 = p11.Distance(p21);
    Standard_Real d21 =p12.Distance(p21);
  
    Standard_Real d12 = p11.Distance(p22);
    Standard_Real d22 = p22.Distance(p12);
    Standard_Real Dmin1 = Min(d11,d21);
    Standard_Real Dmin2 = Min(d12,d22);
    if(fabs(Dmin1 - Dmin2) <= Precision::Confusion() || Dmin2 > Dmin1) {
      isRev1 = (d11 < d21 ? Standard_True : Standard_False);
    }
    else if(Dmin2 < Dmin1) {
      isRev1 = (d12 < d22 ? Standard_True  : Standard_False);
      isRev2 = Standard_True;
    }
  }

  TopoDS_Wire FixWire(const TopoDS_Wire& theWire)
  {
    ShapeFix_Wire aWireFixer;
    aWireFixer.Load(theWire);
    aWireFixer.FixClosed();
    aWireFixer.FixGaps2d();
    aWireFixer.FixConnected();
    return aWireFixer.Wire();
  }

  template<class HCurve> 
  static inline void SegmentCurve (HCurve& curve,
                                   const Standard_Real first,
                                   const Standard_Real last)
  {
    if(curve->FirstParameter() < first - Precision::PConfusion() || 
       curve->LastParameter() > last + Precision::PConfusion()) {
      if(curve->IsPeriodic())
        curve->Segment(first,last);
      else curve->Segment(Max(curve->FirstParameter(),first),
                          Min(curve->LastParameter(),last));
  } 
}

  template<class HCurve>
  HCurve GetCurveCopy(const HCurve&             curve,
                      double&                   first,
                      double&                   last,
                      const TopAbs_Orientation& orient)
  {
    if ( orient == TopAbs_REVERSED )
    {
      double cf = first;
      first = curve->ReversedParameter ( last );
      last  = curve->ReversedParameter ( cf );
      return curve->Reversed();
    }
    return HCurve::DownCast(curve->Copy());
  }

  // Joins the passed 2D curves.
  bool JoinCurves(const Handle(Geom2d_Curve)& aC2d1,
                  const Handle(Geom2d_Curve)& aC2d2,
                  const TopAbs_Orientation Orient1,
                  const TopAbs_Orientation Orient2,
                  Standard_Real& first1,
                  Standard_Real& last1,
                  Standard_Real& first2,
                  Standard_Real& last2,
                  Handle(Geom2d_Curve)& C2dOut,
                  Standard_Boolean& isRev1,
                  Standard_Boolean& isRev2,
                  const Standard_Boolean isError)
  {
    Handle(Geom2d_Curve) c2d1_copy, c2d2_copy;
    c2d1_copy = GetCurveCopy(aC2d1, first1, last1, Orient1);
    c2d2_copy = GetCurveCopy(aC2d2, first2, last2, Orient2);
    ShapeConstruct_Curve scc;
    Standard_Boolean After =  Standard_True;

    // Convert to splines.
    Handle(Geom2d_BSplineCurve)
      bsplc12d = scc.ConvertToBSpline(c2d1_copy,first1,last1,Precision::Confusion());
    //
    Handle(Geom2d_BSplineCurve)
      bsplc22d = scc.ConvertToBSpline(c2d2_copy,first2,last2,Precision::Confusion());

    if(bsplc12d.IsNull() || bsplc22d.IsNull()) return Standard_False;
  
    SegmentCurve(bsplc12d,first1,last1);
    SegmentCurve(bsplc22d,first2,last2);
    //gka protection against crossing seem on second face 
    gp_Pnt2d pp112d =  bsplc12d->Pole(1).XY();
    gp_Pnt2d pp122d =  bsplc12d->Pole(bsplc12d->NbPoles()).XY();
  
    gp_Pnt2d pp212d =  bsplc22d->Pole(1).XY();
    gp_Pnt2d pp222d =  bsplc22d->Pole(bsplc22d->NbPoles()).XY();
  
    GetReversedParameters(pp112d,pp122d,pp212d,pp222d,isRev1,isRev2);
  
    //regression on file 866026_M-f276-f311.brep bug OCC482
    //if(isRev1 || isRev2)
    //  return newedge1;
    if(isRev1) {
      bsplc12d->Reverse();
    }
    if(isRev2)
      bsplc22d->Reverse(); 
  
  
    //---------------------------------------------------------
    //protection against invalid topology Housing(sam1296.brep(face 707) - bugmergeedges4.brep)
    if(isError) {
      gp_Pnt2d pp1 = bsplc12d->Value(bsplc12d->FirstParameter());
      gp_Pnt2d pp2 = bsplc12d->Value(bsplc12d->LastParameter());
      gp_Pnt2d pp3 = bsplc12d->Value((bsplc12d->FirstParameter() + bsplc12d->LastParameter())*0.5);
    
      Standard_Real leng = pp1.Distance(pp2);
      Standard_Boolean isCircle = (leng < pp1.Distance(pp3) + Precision::PConfusion());
      if((pp1.Distance(bsplc22d->Pole(1)) < leng) && !isCircle) return Standard_False;
    }
    //-------------------------------------------------------
    gp_Pnt2d pmid1 = 0.5 * ( bsplc12d->Pole(bsplc12d->NbPoles()).XY() + bsplc22d->Pole(1).XY() );
        bsplc12d->SetPole(bsplc12d->NbPoles(), pmid1);
    bsplc22d->SetPole(1, pmid1);
  
    // abv 01 Sep 99: Geom2dConvert ALWAYS performs reparametrisation of the
    // second curve before merging; this is quite not suitable
    // Use 3d tool instead
  //      Geom2dConvert_CompCurveToBSplineCurve connect2d(bsplc12d);
    gp_Pnt vPnt(0,0,0);
    gp_Vec vDir(0,0,1);
    gp_Pln vPln ( vPnt, vDir );
    Handle(Geom_BSplineCurve) bspl1 = 
      Handle(Geom_BSplineCurve)::DownCast ( GeomAPI::To3d ( bsplc12d, vPln ) );
    Handle(Geom_BSplineCurve) bspl2 = 
      Handle(Geom_BSplineCurve)::DownCast ( GeomAPI::To3d ( bsplc22d, vPln ) );
    GeomConvert_CompCurveToBSplineCurve connect2d(bspl1);
    if(!connect2d.Add(bspl2,Precision::PConfusion(), After, Standard_False)) return Standard_False;
     C2dOut = GeomAPI::To2d ( connect2d.BSplineCurve(), vPln );
  
    return Standard_True;
  }
}

//! Ctor.
asiAlgo_JoinEdges::asiAlgo_JoinEdges(const TopoDS_Shape&  masterCAD,
                                     ActAPI_ProgressEntry progress,
                                     ActAPI_PlotterEntry  plotter)
{
  m_master   = masterCAD;
  m_progress = progress;
  m_plotter  = plotter;
}

//! Joins the given edges in the master model.
//! \param edges [in] edges to join.
//! \param face  [in] base face.
//! \return true in case of success, false -- otherwise.
bool asiAlgo_JoinEdges::Perform(const TopTools_IndexedMapOfShape& edges,
                                const TopoDS_Face&                face)
{
  if ( edges.Extent() != 2 )
  {
    std::cout << "Error: only pair of edges is accepted" << std::endl;
    return false;
  }

  // Join edges
  TopoDS_Edge E1, E2, newE;
  this->chooseOrder(edges, E1, E2);
  //
  if ( !this->joinEdges(E1, E2, face, newE) )
  {
    std::cout << "Error: cannot join edges" << std::endl;
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

  return true; // Success
}

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
  // work in the parametric domain
  TopAbs_Orientation eFirstOri = TopAbs_EXTERNAL, eSecondOri = TopAbs_EXTERNAL;
  //
  for ( TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    if ( exp.Current().IsPartner(eFirst) )
      eFirstOri = exp.Current().Orientation();
    else if ( exp.Current().IsPartner(eSecond) )
      eSecondOri = exp.Current().Orientation();
  }

  // Choose orientation of the bridge edge being built
  TopoDS_Edge eForward;
  if ( eFirstOri == TopAbs_FORWARD )
    eForward = eFirst;
  else if ( eSecondOri == TopAbs_FORWARD )
    eForward = eSecond;
  //
  TopAbs_Orientation bridgeOri = eFirst.Orientation();

  if ( !sae.PCurve(eFirst, face, c2d1, first1, last1, false) )
    return false;

  if ( !sae.PCurve(eSecond, face, c2d2, first2, last2, false) )
    return false;

  Handle(Geom2d_Curve) c2dRes;
  bool isRev12, isRev22;
  if( !::JoinCurves(c2d1, c2d2, eFirstOri, eSecondOri, first1, last1, first2, last2, c2dRes, isRev12, isRev22, false) )
    return false;

  //m_plotter.REDRAW_CURVE2D("c2d1", c2d1, Color_White);
  //m_plotter.REDRAW_CURVE2D("c2d2", c2d2, Color_White);
  //m_plotter.REDRAW_CURVE2D("c2dRes", c2dRes, Color_Green);

  // Get host surface.
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

  // Build edge.
  BRepBuilderAPI_MakeEdge mkEdge(c2dRes, surf);
  eResult = mkEdge.Edge();

  // Recover missing geometry
  ShapeFix_Edge SFE;
  SFE.FixAddCurve3d(eResult);

  // Set orientation.
  eResult.Orientation(bridgeOri);

  //m_plotter.REDRAW_SHAPE("eResult", eResult, Color_Blue);

  return true;
}
