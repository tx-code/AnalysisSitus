//-----------------------------------------------------------------------------
// Created on: 04 December 2021
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
#include <asiAlgo_MeshCheckTopology.h>

// OpenCascade includes
#include <BRep_Tool.hxx>
#include <TColStd_PackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Connect.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

namespace
{
  //! Computes the area of a triangle given by its three points.
  double ComputeArea(const gp_XYZ& P1,
                     const gp_XYZ& P2,
                     const gp_XYZ& P3)
  {
    return 0.5*(P3 - P1).Crossed(P2 - P1).Modulus();
  }

  double ComputeArea(const gp_XY& P1,
                     const gp_XY& P2,
                     const gp_XY& P3)
  {
    return 0.5*Abs((P3 - P1).Crossed(P2 - P1));
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshCheckTopology::Perform()
{
  TopTools_IndexedMapOfShape aMapF;
  TopTools_IndexedDataMapOfShapeListOfShape aMapEF;
  TopExp::MapShapes (m_shape, TopAbs_FACE, aMapF);
  TopExp::MapShapesAndAncestors (m_shape, TopAbs_EDGE, TopAbs_FACE, aMapEF);

  // check polygons
  int ie;
  for (ie=1; ie <= aMapEF.Extent(); ie++) {
    const TopoDS_Edge& aEdge = TopoDS::Edge(aMapEF.FindKey(ie));
    const TopTools_ListOfShape& aFaces = aMapEF(ie);
    if (aFaces.Extent() < 2) continue;

    // get polygon on first face
    const TopoDS_Face& aFace1 = TopoDS::Face(aFaces.First());
    TopLoc_Location aLoc1;
    Handle(Poly_Triangulation) aT1 = BRep_Tool::Triangulation(aFace1, aLoc1);
    Handle(Poly_PolygonOnTriangulation) aPoly1 =
      BRep_Tool::PolygonOnTriangulation(aEdge, aT1, aLoc1);
    if (aPoly1.IsNull() || aT1.IsNull()) {
#ifdef OCCT_DEBUG
      std::cout<<"problem getting PolygonOnTriangulation of edge "<<ie<<std::endl;
#endif
      continue;
    }
    const TColStd_Array1OfInteger& aNodes1 = aPoly1->Nodes();

    // cycle on other polygons
    TopTools_ListIteratorOfListOfShape it(aFaces);
    it.Next();
    for (; it.More(); it.Next()) {
      const TopoDS_Face& aFace2 = TopoDS::Face(it.Value());
      TopLoc_Location aLoc2;
      Handle(Poly_Triangulation) aT2 = BRep_Tool::Triangulation(aFace2, aLoc2);
      Handle(Poly_PolygonOnTriangulation) aPoly2 =
	BRep_Tool::PolygonOnTriangulation(aEdge, aT2, aLoc2);
      if (aPoly2.IsNull() || aT2.IsNull()) {
#ifdef OCCT_DEBUG
	std::cout<<"problem getting PolygonOnTriangulation of edge "<<ie<<std::endl;
#endif
	continue;
      }
      const TColStd_Array1OfInteger& aNodes2 = aPoly2->Nodes();

      // check equality of polygons lengths
      if (aNodes2.Length() != aNodes1.Length()) {
	m_asyncEdges.Append(ie);
	break;
      }

      // check distances between corresponding points
      double aSqDefle = BRep_Tool::Tolerance(aEdge);
      aSqDefle *= aSqDefle;
      const TColgp_Array1OfPnt& aPoints1 = aT1->Nodes();
      const TColgp_Array1OfPnt& aPoints2 = aT2->Nodes();
      int iF1 = aMapF.FindIndex(aFace1);
      int iF2 = aMapF.FindIndex(aFace2);
      int i1 = aNodes1.Lower();
      int i2 = aNodes2.Lower();
      const gp_Trsf &aTrsf1 = aFace1.Location().Transformation();
      const gp_Trsf &aTrsf2 = aFace2.Location().Transformation();
      for (; i1 <= aNodes1.Upper(); i1++, i2++) {
	const gp_Pnt aP1 = aPoints1(aNodes1(i1)).Transformed(aTrsf1);
	const gp_Pnt aP2 = aPoints2(aNodes2(i2)).Transformed(aTrsf2);
	const double aSqDist = aP1.SquareDistance(aP2);
        if (aSqDist > aSqDefle)
        {
	  m_errors.Append(iF1);
	  m_errors.Append(i1);
	  m_errors.Append(iF2);
	  m_errors.Append(i2);
          m_errorsVal.Append(Sqrt(aSqDist));
	}
      }
    }
  }

  // check triangulations
  int iF;
  for (iF=1; iF <= aMapF.Extent(); iF++) {
    const TopoDS_Face& face = TopoDS::Face(aMapF.FindKey(iF));
    TopLoc_Location loc;

    Handle(Poly_Triangulation) tris = BRep_Tool::Triangulation(face, loc);
    //
    if ( tris.IsNull() )
    {
      m_progress.SendLogMessage(LogInfo(Normal) << "The face %1 does not have triangulation."
                                                << iF);
      continue;
    }

    const gp_Trsf &aTrsf = loc.Transformation();

    // remember boundary nodes
    TColStd_PackedMapOfInteger aMapBndNodes;
    TopExp_Explorer ex(face, TopAbs_EDGE);
    for (; ex.More(); ex.Next()) {
      const TopoDS_Edge& aEdge = TopoDS::Edge(ex.Current());
      Handle(Poly_PolygonOnTriangulation) aPoly =
        BRep_Tool::PolygonOnTriangulation(aEdge, tris, loc);
      if (aPoly.IsNull()) continue;
      const TColStd_Array1OfInteger& aNodes = aPoly->Nodes();
      int i;
      for (i=aNodes.Lower(); i <= aNodes.Upper(); i++)
        aMapBndNodes.Add(aNodes(i));
    }

    TColStd_PackedMapOfInteger aUsedNodes;

    // check of free links and nodes
    Poly_Connect aConn(tris);
    const Poly_Array1OfTriangle& aTriangles = tris->Triangles();
    int nbTri = tris->NbTriangles(), i, j, n[3], t[3];
    for (i = 1; i <= nbTri; i++) {
      aTriangles(i).Get(n[0], n[1], n[2]);
      
      aUsedNodes.Add (n[0]);
      aUsedNodes.Add (n[1]);
      aUsedNodes.Add (n[2]);

      const gp_Pnt aPts[3] = {tris->Node(n[0]).Transformed(aTrsf),
                              tris->Node(n[1]).Transformed(aTrsf),
                              tris->Node(n[2]).Transformed(aTrsf)};

      double anArea = ComputeArea(aPts[0].XYZ(), aPts[1].XYZ(), aPts[2].XYZ());
      if (anArea < Precision::SquareConfusion())
      {
        m_smallTrianglesFaces.Append(iF);
        m_smallTrianglesTriangles.Append(i);
      }
      else if (tris->HasUVNodes())
      {
        const gp_XY aPUV[3] = {tris->UVNode(n[0]).XY(),
                               tris->UVNode(n[1]).XY(),
                               tris->UVNode(n[2]).XY()};
        anArea = ComputeArea(aPUV[0], aPUV[1], aPUV[2]);
        if (anArea < Precision::SquarePConfusion())
        {
          m_smallTrianglesFaces.Append(iF);
          m_smallTrianglesTriangles.Append(i);
        }
      }

      aConn.Triangles(i, t[0], t[1], t[2]);
      for (j = 0; j < 3; j++) {
	if (t[j] == 0) {
	  // free link found
	  int k = (j+1) % 3;  // the following node of the edge
	  int n1 = n[j];
	  int n2 = n[k];
	  // skip if it is on boundary
	  if (aMapBndNodes.Contains(n1) && aMapBndNodes.Contains(n2))
	    continue;
	  if (!m_mapFaceLinks.Contains(iF)) {
            Handle(TColStd_HSequenceOfInteger) tmpSeq = new TColStd_HSequenceOfInteger;
	    m_mapFaceLinks.Add(iF, tmpSeq);
          }
	  Handle(TColStd_HSequenceOfInteger)& aSeq = m_mapFaceLinks.ChangeFromKey(iF);
	  aSeq->Append(n1);
	  aSeq->Append(n2);
	}
      }
    }
    
    // check of free nodes
    int aNbNodes = tris->NbNodes();
    for ( int k = 1; k <= aNbNodes; k++ )
    {
      if ( ! aUsedNodes.Contains(k) )
      {
        m_freeNodeFaces.Append (iF);
        m_freeNodeNums.Append (k);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshCheckTopology::GetFreeLink(const int theFaceIndex,
                                            const int theLinkIndex,
                                            int&      theNode1,
                                            int&      theNode2) const
{
  const Handle(TColStd_HSequenceOfInteger)& aSeq = m_mapFaceLinks(theFaceIndex);
  int aInd = (theLinkIndex-1)*2 + 1;
  theNode1 = aSeq->Value(aInd);
  theNode2 = aSeq->Value(aInd+1);
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshCheckTopology::GetCrossFaceError(const int theIndex,
                                                  int&      theFace1,
                                                  int&      theNode1,
                                                  int&      theFace2,
                                                  int&      theNode2,
                                                  double&   theValue) const
{
  int aInd = (theIndex-1)*4 + 1;
  theFace1 = m_errors(aInd);
  theNode1 = m_errors(aInd+1);
  theFace2 = m_errors(aInd+2);
  theNode2 = m_errors(aInd+3);
  theValue = m_errorsVal(theIndex);
}
