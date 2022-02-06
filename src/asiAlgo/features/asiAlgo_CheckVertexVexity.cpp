//-----------------------------------------------------------------------------
// Created on: 04 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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
#include <asiAlgo_CheckVertexVexity.h>

// asiAlgo includes
#include <asiAlgo_FeatureAttrAngle.h>

// OpenCascade includes
#include <BRepAdaptor_Curve.hxx>
#include <BRepClass_FClassifier.hxx>
#include <ElSLib.hxx>
#include <ShapeExtend_WireData.hxx>

//-----------------------------------------------------------------------------

#define AxisAngTolerRad   1*M_PI/180.
#define InPlaneShiftCoeff 0.5

//-----------------------------------------------------------------------------

asiAlgo_CheckVertexVexity::asiAlgo_CheckVertexVexity(const Handle(asiAlgo_AAG)& aag,
                                                     ActAPI_ProgressEntry       progress,
                                                     ActAPI_PlotterEntry        plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_aag             (aag)
{}

//-----------------------------------------------------------------------------

void asiAlgo_CheckVertexVexity::CollectEdges(const int      fid,
                                             t_edgeInfoMap& edgesInfo) const
{
  const TopoDS_Face& face = m_aag->GetFace(fid);

  if ( asiAlgo_Utils::IsPlanar(face) ||
       asiAlgo_Utils::IsCylindrical(face) )
  {
    // Collect all the edges.
    const asiAlgo_Feature& nids = m_aag->GetNeighbors(fid);
    //
    for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
    {
      const int nid = nit.Key();

      asiAlgo_AAG::t_arc arc(fid, nid);

      Handle(asiAlgo_FeatureAttrAngle)
        DA = m_aag->ATTR_ARC<asiAlgo_FeatureAttrAngle>(arc);
      //
      if ( DA.IsNull() )
        continue;

      const bool isConcave =  asiAlgo_FeatureAngle::IsConcave ( DA->GetAngleType() );
      const bool isSharp   = !asiAlgo_FeatureAngle::IsSmooth  ( DA->GetAngleType() );

      // Get edges realizing the adjacency.
      Handle(asiAlgo_FeatureAttrAdjacency)
        attrAdj = m_aag->ATTR_ARC<asiAlgo_FeatureAttrAdjacency>( arc);
      //
      if ( attrAdj.IsNull() )
        continue;

      // Get the edges realizing adjacency.
      TopTools_IndexedMapOfShape edges;
      attrAdj->GetEdges(edges);
      //
      for ( int kk = 1; kk <= edges.Extent(); ++kk )
      {
        const TopoDS_Edge& edge = TopoDS::Edge( edges(kk) );

        // Prepare edge info descriptor.
        t_edgeInfo edgeInfo;
        edgeInfo.edge       = edge;
        edgeInfo.isConcave  = isConcave;
        edgeInfo.isSharp    = isSharp;
        edgeInfo.isStraight = asiAlgo_Utils::IsStraight(edge);
        //
        if ( !edgeInfo.isStraight )
          edgeInfo.isCircular = asiAlgo_Utils::IsCircular(edge);
        //
        asiAlgo_Utils::ComputeBorderTrihedron(face, edge, edgeInfo.axes);
        //
        edgesInfo.Add(edge, edgeInfo);
      }
    }
  }
}

//-----------------------------------------------------------------------------

//! Checks convexity of a vertex common for the passed two edges.
asiAlgo_FeatureAngleType
  asiAlgo_CheckVertexVexity::CheckConvexity(const TopoDS_Edge& E1,
                                            const TopoDS_Edge& E2,
                                            const int          fid,
                                            TopoDS_Vertex&     V) const
{
  const TopoDS_Face& F = m_aag->GetFace(fid);

  const double pprec = Precision::PConfusion();

  V = asiAlgo_Utils::GetCommonVertex(E1, E2);
  TopoDS_Edge ePrev, eNext;

  const double p1 = BRep_Tool::Parameter(V, E1);
  const double p2 = BRep_Tool::Parameter(V, E2);

  // Decide which edge is previous and which one is the next one.
  double f1, l1, f2, l2;
  double p_prev, p_next;
  double ePrev_l, eNext_l;
  //
  BRep_Tool::Range(E1, f1, l1);
  BRep_Tool::Range(E2, f2, l2);
  //
  if ( Abs(p1 - l1) < pprec )
  {
    p_prev  = p1;
    p_next  = p2;
    ePrev   = E1;
    eNext   = E2;
    ePrev_l = l1;
    eNext_l = l2;
  }
  else
  {
    p_prev  = p2;
    p_next  = p1;
    ePrev   = E2;
    eNext   = E1;
    ePrev_l = l2;
    eNext_l = l1;
  }

  BRepAdaptor_Curve cPrev(ePrev);
  BRepAdaptor_Curve cNext(eNext);

  // Evaluate directions.
  gp_Pnt P_prev, P_next;
  gp_Vec V_prev, V_next;
  //
  cPrev.D1(p_prev, P_prev, V_prev);
  cNext.D1(p_next, P_next, V_next);

  // To compute the angle, the directions should be properly reversed.
  if ( Abs(p_prev - ePrev_l) < pprec )
    V_prev.Reverse();
  //
  if ( Abs(p_next - eNext_l) < pprec )
    V_next.Reverse();

  double ang = Abs( V_prev.Angle(V_next) );

  // Check for smooth transition.
  if ( (ang < AxisAngTolerRad) || (Abs(ang - M_PI) < AxisAngTolerRad) )
    return FeatureAngleType_Smooth;

  if ( !m_plotter.Access().IsNull() )
  {
    m_plotter.DRAW_SHAPE     (ePrev,          Color_Red,   1., true, "ePrev");
    m_plotter.DRAW_SHAPE     (eNext,          Color_Green, 1., true, "eNext");
    m_plotter.DRAW_VECTOR_AT (P_prev, V_prev, Color_Red,             "V_prev");
    m_plotter.DRAW_VECTOR_AT (P_next, V_next, Color_Green,           "V_next");
  }

  // Prepare a probe point.
  Handle(Geom_Plane) plane;
  gp_XYZ probe = P_prev.XYZ() + InPlaneShiftCoeff*V_prev.Normalized().XYZ()
                              + InPlaneShiftCoeff*V_next.Normalized().XYZ();
  //
  if ( asiAlgo_Utils::IsPlanar(F, plane) )
  {
    double S, T;
    ElSLib::Parameters(plane->Pln(), probe, S, T);

    // Prepare classification utility.
    BRepClass_FClassifier faceClass2d;

    // Prepare face adaptor to initialize the classifier.
    BRepClass_FaceExplorer fe(F);

    faceClass2d.Perform(fe, gp_Pnt2d(S, T), 1.0e-4);

    // Check if a point is inside the face.
    const bool isIn = (faceClass2d.State() == TopAbs_IN);

    if ( !m_plotter.Access().IsNull() )
    {
      m_plotter.DRAW_POINT(probe, isIn ? Color_Green : Color_Red, "probe");
    }

    if ( !isIn )
      ang = 2*M_PI - ang;

    if ( ang > M_PI )
      return FeatureAngleType_Convex;

    return FeatureAngleType_Concave;
  }

  return FeatureAngleType_Undefined;
}

//-----------------------------------------------------------------------------

void asiAlgo_CheckVertexVexity::CheckContours(const int    fid,
                                              t_vexityMap& vexity) const
{
  t_edgeInfoMap edgesInfo;
  this->CollectEdges(fid, edgesInfo);

  if ( edgesInfo.IsEmpty() )
    return;

  const TopoDS_Face& face = m_aag->GetFace(fid);

  // Wire by wire.
  for ( TopExp_Explorer wexp(face, TopAbs_WIRE); wexp.More(); wexp.Next() )
  {
    const TopoDS_Wire&                 W  = TopoDS::Wire( wexp.Current() );
    const Handle(ShapeExtend_WireData) WD = new ShapeExtend_WireData(W);
    const int                          ne = WD->NbEdges();

    // Edge by edge.
    for ( int eid = 1; eid <= ne; ++eid )
    {
      // Get consecutive edges.
      const TopoDS_Edge& E1 = WD->Edge(eid);
      const TopoDS_Edge& E2 = WD->Edge(eid == ne ? 1 : eid + 1);

      if ( !edgesInfo.Contains(E1) || !edgesInfo.Contains(E2) )
        continue;

      const t_edgeInfo& einfo1 = edgesInfo.FindFromKey(E1);
      const t_edgeInfo& einfo2 = edgesInfo.FindFromKey(E2);

      TopoDS_Vertex V;
      asiAlgo_FeatureAngleType
        vVexity = this->CheckConvexity(einfo1.edge, einfo2.edge, fid, V);

      vexity.Bind(V, vVexity);
    } // By edges.
  } // By wires.
}
