//-----------------------------------------------------------------------------
// Created on: 17 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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
#include <asiAlgo_DiscrBuilder.h>

// asiAlgo includes
#include <asiAlgo_DiscrCurveAdaptor.h>
#include <asiAlgo_DiscrTessellateCurve.h>

// OpenCascade includes
#include <Adaptor3d_CurveOnSurface.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <BRepAdaptor_HCurve2d.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ReShape.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#include <GeomAdaptor_HSurface.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeAnalysis_WireOrder.hxx>
#include <ShapeExtend_WireData.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_SequenceOfShape.hxx>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

static TopoDS_Wire makeOrientedWire (const TopoDS_Wire& theWire)
{
  TopoDS_Wire aWire;
  BRep_Builder().MakeWire (aWire);
  for (TopoDS_Iterator it (theWire); it.More(); it.Next())
  {
    TopoDS_Edge aEdge = TopoDS::Edge (it.Value());
    aEdge.Orientation (TopAbs_FORWARD);
    BRep_Builder().Add (aWire, aEdge);
  }
  return aWire;
}

//-----------------------------------------------------------------------------

void Builder::Tessellate()
{
  if ( m_bDone )
    return;

  if ( m_shape.IsNull() )
  {
    AddCompStatus(FailureNullShape);
    return;
  }

  // Clean up auxiliary maps.
  m_mapF.Clear();
  m_mapEF.Clear();
  m_mapV.Clear();
  m_badShapes.Clear();
  m_mapELen.Clear();
  m_mapEDefl.Clear();
  m_mapFMinSize.Clear();
  m_mapESameParam.Clear();
  m_mapEIgnored.Clear();

  /* =================
   *  Index subshapes.
   * ================= */

  // Faces
  TopExp::MapShapes(m_shape, TopAbs_FACE, m_mapF);
  int nbFaces = m_mapF.Extent();
  if ( nbFaces == 0 )
  {
    AddCompStatus(FailureShapeWithoutFaces);
    return;
  }

  // Edges and faces.
  TopExp::MapShapesAndAncestors(m_shape, TopAbs_EDGE, TopAbs_FACE, m_mapEF);
  int nbEdges = m_mapEF.Extent();
  if ( nbEdges == 0 )
  {
    AddCompStatus(FailureShapeWithoutEdges);
    return;
  }

  // Initialize the outcome discrete model.
  m_model = new Model;
  m_model->InitFaces(nbFaces);
  m_model->InitEdges(nbEdges);

  // Discretise 3D curves of all edges.
  int i;
  for ( i = 1; i <= nbEdges; ++i )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( m_mapEF.FindKey(i) );
    m_mapESameParam.Bind( i, BRep_Tool::SameParameter(edge) );

    // Map discrete edge to original analytic one.
    Edge& dEdge = m_model->ChangeEdge(i);
    m_mapDEdges.Bind(&dEdge, edge);

    // Exclude edges that have 2d and 3d curves of null-length
    // from consideration.
    if ( BRep_Tool::Degenerated(edge) )
    {
      if ( CheckNullLength2d(edge) )
        m_mapEIgnored.Add(i);

      continue;
    }

    if ( GetEdgeLength(edge) < Precision::Confusion() )
    {
      m_mapEIgnored.Add(i);
      continue;
    }

    // Tolerance to check Same Parameterization property.
    double aSameParamToler = m_meshParams.IsDeflection() ?
      m_meshParams.Deflection() : m_meshParams.MinElemSize();

    // save the original meshing parameters just in case
    const Params aMeshParams = m_meshParams;

    // Discretise twice if Same Parameter property is false
    bool isDiscretised = false;
    for(int nPass = 2; nPass > 0; nPass--) {
      isDiscretised = DiscretiseEdge(edge, dEdge);
      if ( !isDiscretised )
        break;
      if (!m_meshParams.IsDeflection())
        // same-param is checked only with deflection set
        break;

      // Check Same Parameter property of the edge and deflection of 2D
      // polygons from 3D one.
      bool isEdgeSameParam = m_mapESameParam(i) > 0;
      if (!isEdgeSameParam)
        // calculation has already been done in non-sameparam mode
        break;
      const TColStd_SequenceOfReal& aDiscrParams = dEdge.GetParams();
      bool isTolUpdated;
      isEdgeSameParam = IsSameParameterEdge(edge, aDiscrParams, aSameParamToler, isTolUpdated);
      if (isEdgeSameParam && !isTolUpdated)
        // check is OK, no need to repeat discretisation
        break;
      if (!isEdgeSameParam)
        // switch off same-parameter flag
        m_mapESameParam(i) = 0;
      if (isTolUpdated)
      {
        // tolerance is increased, so repeat with updated deflection
        AddCompStatus(WarningLargeTolerance);
        m_meshParams.SetDeflection(aSameParamToler);
      }
    }

    // restore the original meshing parameters
    m_meshParams = aMeshParams;

    if ( !isDiscretised )
    {
      AddCompStatus(FailureDiscretizeCurve);
      m_badShapes.Add(edge);
      const TopTools_ListOfShape& lf = m_mapEF.FindFromKey(edge);
      TopTools_ListIteratorOfListOfShape itl(lf);
      for (; itl.More(); itl.Next()) {
        const TopoDS_Shape& face = itl.Value();
        m_badShapes.Add(face);
      }
      continue;
    }

    // ** adding vertices
    TopoDS_Vertex aFV, aLV;
    TopExp::Vertices(edge, aFV, aLV);
    if (aFV.IsSame(aLV) && !IsSignificant(edge, dEdge, aFV))
    {
      m_mapEIgnored.Add(i);
      continue;
    }

    Vertex* aFVertex;
    Vertex* aLVertex;
    if(m_mapV.Contains(aFV)) {
      aFVertex = &m_model->ChangeVertex(m_mapV.FindIndex(aFV));
    }
    else {
      aFVertex = &m_model->ChangeVertex(m_model->AddVertex());
      m_mapV.Add(aFV);
      aFVertex->ChangePnt() = BRep_Tool::Pnt(aFV);
      aFVertex->SetTolerance(BRep_Tool::Tolerance(aFV));
    }
    dEdge.SetFirstVertex(*aFVertex);
    aFVertex->AddEdge(dEdge);

    if(m_mapV.Contains(aLV)) {
      aLVertex = &m_model->ChangeVertex(m_mapV.FindIndex(aLV));
    }
    else {
      aLVertex = &m_model->ChangeVertex(m_model->AddVertex());
      m_mapV.Add(aLV);
      aLVertex->ChangePnt() = BRep_Tool::Pnt(aLV);
      aLVertex->SetTolerance(BRep_Tool::Tolerance(aLV));
    }
    dEdge.SetLastVertex(*aLVertex);
    aLVertex->AddEdge(dEdge);
    // **
  }

  // Cycle by faces:
  // create discrete faces and wires, discretise 2D curves
  for (i=1; i <= nbFaces; i++)
  {
    TopoDS_Face face = TopoDS::Face(m_mapF(i));

    if (m_badShapes.Contains(face)) continue;
    TopoDS_Face faceFixed = FixFaceWires(face);
    if (faceFixed.IsNull())
    {
      AddCompStatus(FailureNonMonifoldWires);
      m_badShapes.Add(face);
      continue;
    }
    bool isForward = face.Orientation() != TopAbs_REVERSED;
    faceFixed.Orientation(TopAbs_FORWARD);

    // count wires of face
    TopTools_SequenceOfShape wires;
    TopoDS_Iterator its (faceFixed, false);
    for (; its.More(); its.Next()) if (its.Value().ShapeType() == TopAbs_WIRE)
    {
      const TopoDS_Shape& wire = its.Value();

      // check that wire contains all edges either oriented (forward or reversed)
      // or non-oriented (internal or external)
      bool hasOriented = false;
      bool hasNonOriented = false;
      int nbe = 0;
      for (TopoDS_Iterator ite(wire); ite.More(); ite.Next(), nbe++)
      {
        TopAbs_Orientation eOri = ite.Value().Orientation();
        if (eOri == TopAbs_FORWARD || eOri == TopAbs_REVERSED)
          hasOriented = true;
        else
          hasNonOriented = true;
      }
      if (hasOriented && hasNonOriented)
        AddCompStatus(WarningInternalEdges);
      else if (nbe == 0)
        AddCompStatus(WarningWireWithoutEdges);
      else
        wires.Append(wire);
    }
    int nbw = wires.Length();
    if (nbw == 0)
    {
      AddCompStatus(FailureFaceWithoutWires);
      m_badShapes.Add(face);
      continue;
    }

    Face& dFace = m_model->ChangeFace(i);
    m_mapDFacesId.Bind(&dFace, i);
    dFace.SetForward(isForward);
    dFace.InitWires(nbw);

    // Cycle by wires
    int j;
    for (j=1; j <= nbw; j++) {
      TopoDS_Wire wire = TopoDS::Wire(wires(j));
      Wire& dWire = dFace.ChangeWire(j);
      dWire.SetClosed (wire.Closed());
      TopAbs_Orientation eOri = TopoDS_Iterator (wire).Value().Orientation();
      bool isOriented = eOri == TopAbs_FORWARD || eOri == TopAbs_REVERSED;
      dWire.SetInternal (!isOriented);
      if (!DiscretiseWire (wire, dWire, dFace, faceFixed))
      {
        // an error occured while discretising this face, throw out it
        dFace.Nullify();
        m_badShapes.Add(face);
        break;
      }
    }
  }

  m_bDone = true;
}

//=======================================================================
//function : CheckNullLength2d
//purpose  : Returns TRUE in case if the given edge has null-length
//           2d curve, returns FALSE elsewhere.
//=======================================================================

bool Builder::CheckNullLength2d(const TopoDS_Edge& theEdge)
{
  const TopTools_ListOfShape& aFaces = m_mapEF.FindFromKey(theEdge);
  TopTools_ListIteratorOfListOfShape aFaceIt(aFaces);
  for (; aFaceIt.More(); aFaceIt.Next())
  {
    TopoDS_Face& aFace = TopoDS::Face(aFaceIt.Value());

    gp_Pnt2d aFirstUV, aLastUV;
    BRep_Tool::UVPoints(theEdge, aFace, aFirstUV, aLastUV);

    if (aFirstUV.SquareDistance(aLastUV) < Precision::SquareConfusion())
      return true;
  }

  return false;
}

//=======================================================================
//function : GetEdgeLength
//purpose  : 
//=======================================================================

double Builder::GetEdgeLength(const TopoDS_Edge& aEdge)
{
  int iEdge = m_mapEF.FindIndex(aEdge);
  double aLen;
  if (!m_mapELen.IsBound(iEdge)) {
    BRepAdaptor_Curve aCurve (aEdge);
    aLen = GCPnts_AbscissaPoint::Length(aCurve);
    m_mapELen.Bind(iEdge, aLen);
  }
  else
    aLen = m_mapELen(iEdge);
  return aLen;
}


//=======================================================================
//function : DiscretiseWire
//purpose  : 
//=======================================================================

bool Builder::DiscretiseWire(const TopoDS_Wire& theWire,
                             Wire&              theDWire,
                             Face&              theDFace,
                             const TopoDS_Face& theFace)
{
  // reorder edges of wire.
  // Enable check of non-manifold topology in case of wires of internal edges.
  bool isManifold = !theDWire.IsInternal();
  TopoDS_Wire aWire = isManifold ? theWire : makeOrientedWire (theWire);
  Handle(ShapeExtend_WireData) aWireData = 
    new ShapeExtend_WireData(aWire, true, isManifold);

  ShapeAnalysis_Wire saw(aWireData, theFace, Precision::Confusion());
   
  ShapeAnalysis_WireOrder sawo;
  // MSV 1.10.2004: the following line is commented due to the bug
  //                in the case emh/002/E1
  //sawo.KeepLoopsMode() = true;
  const bool isClosed = isManifold || theDWire.IsClosed();
  const bool is3d = false;
  saw.CheckOrder (sawo, isClosed, is3d);
  if (saw.LastCheckStatus(ShapeExtend_FAIL))
  {
    AddCompStatus(FailureOrderOfEdges);
    return false;
  }
  if (saw.LastCheckStatus(ShapeExtend_DONE3))
  {
    AddCompStatus(WarningNeedToReorderEdges);
  }
  Handle(ShapeExtend_WireData) swd = saw.WireData();
  int nbe = sawo.NbEdges();
  if (nbe != swd->NbEdges() || nbe == 0)
  {
    AddCompStatus(FailureDiscretizeWire);
    return false;
  }

  // Cycle by edges
  TopTools_MapOfShape aCorrectedEdges;
  int k;
  for (k=1; k <= nbe; k++) {
    int ind = sawo.Ordered(k);

    const TopoDS_Edge& edge = swd->Edge(ind);

    TopAbs_Orientation eOri = edge.Orientation();
    bool isEForward = (eOri == TopAbs_FORWARD);

    int iEdge = m_mapEF.FindIndex(edge);
    if (iEdge==0)
      return false;

    if (m_mapEIgnored.Contains(iEdge))
      continue;

    Edge& dEdge = m_model->ChangeEdge(iEdge);

    theDWire.AppendEdge (dEdge, isEForward);
    // Compute a polyline on theFace
    if (!DiscretiseEdge (edge,theFace,dEdge,theDFace,isEForward))
    {
      AddCompStatus(FailureDiscretizePCurve);
      return false;
    }
  }

  // enrich a wire if it consists of two points only
  if (nbe == 2) {
    TopoDS_Edge edges[2];
    edges[0] = swd->Edge(sawo.Ordered(1));
    edges[1] = swd->Edge(sawo.Ordered(2));
    Edge* dEdges[2];
    int nbPnt = 0;
    int anEdgeNum = 1;
    dEdges[0] = &m_model->ChangeEdge(m_mapEF.FindIndex(edges[0]));
    if (!dEdges[0]->IsDegenerated()) {
      nbPnt = dEdges[0]->GetCurve().NbPoints() - 1;
      if (edges[0].IsSame(edges[1]))
        nbPnt *= 2;
      else {
        anEdgeNum = 2;
        dEdges[1] = &m_model->ChangeEdge(m_mapEF.FindIndex(edges[1]));
        if (!dEdges[1]->IsDegenerated())
          nbPnt += dEdges[1]->GetCurve().NbPoints() - 1;
        else
          nbPnt = 0;
      }
    }
    if (nbPnt == 2) {
      TColStd_MapOfInteger aMap;
      // add a middle point in each edge
      for (k=0; k < anEdgeNum; k++) {
        aMap.Clear();
        BRepAdaptor_Curve aCurve (edges[k]);
        // insert a middle parameter
        TColStd_SequenceOfReal& params = dEdges[k]->ChangeParams();
        double midpar = (params(1) + params(2)) * 0.5;
        params.InsertAfter(1, midpar);
        // insert a middle point in 3d curve
        gp_Pnt midPnt = aCurve.Value(midpar);
        dEdges[k]->ChangeCurve().InsertAfter(1, midPnt);
        // insert a middle point in pcurves
        const TopTools_ListOfShape& lf = m_mapEF.FindFromKey(edges[k]);
        TopTools_ListIteratorOfListOfShape itl(lf);
        for (; itl.More(); itl.Next()) {
          const TopoDS_Face& face = TopoDS::Face(itl.Value());
          int iFace = m_mapF.FindIndex(face);
          const Face& dFace = m_model->GetFace(iFace);
          int iPcurve = dEdges[k]->FindPCurve(dFace, true);
          if (iPcurve == 0 || aMap.Contains(iFace)) continue;
          aMap.Add(iFace);
          Curve2d& dPcurve = dEdges[k]->ChangePCurve(iPcurve);
          double fpar,lpar;
          Handle(Geom2d_Curve) pcurve =
            BRep_Tool::CurveOnSurface (edges[k],face,fpar,lpar);
          gp_Pnt2d pt;
          pcurve->D0 (midpar, pt);
          dPcurve.InsertAfter(1, pt);
        }
      }
    }
  }
  return true;
}


//=======================================================================
//function : DiscretiseEdge
//purpose  : for 3D curve
//=======================================================================

bool Builder::DiscretiseEdge(const TopoDS_Edge& aEdge, Edge& aDEdge)
{
  BRepAdaptor_Curve aCurve (aEdge);
  if (aCurve.LastParameter() - aCurve.FirstParameter() < Precision::PConfusion())
    return false;
  const TopTools_ListOfShape& lf = m_mapEF.FindFromKey(aEdge);

  Params aMeshParams = m_meshParams;
  if (myIsAutoSize) {
    // Implement AutoSize checking.
    // Compute local min size from edge length
    double aLen = GetEdgeLength(aEdge);

    if (aLen >= Precision::Infinite())
      return false;

    double aSize = Max(aLen*myAutoSize, Precision::Confusion()*2);
    // Get minimal local min size among adjacent faces
    TopTools_ListIteratorOfListOfShape itl(lf);
    for (; itl.More(); itl.Next()) {
      const TopoDS_Face& aFace = TopoDS::Face (itl.Value());
      double aSz = GetMinSizeForFace(aFace);
      if (aSz < aSize)
        aSize = aSz;
    }
    if (aSize < aMeshParams.MinElemSize()) {
      // correct MinSize
      aMeshParams.SetMinElemSize(aSize);
      //if (aMeshParams.IsDeflection()) {
      //  double aDefl = aSize / 2;
      //  if (aDefl < aMeshParams.Deflection())
      //    // correct Deflection
      //    aMeshParams.SetDeflection(aDefl);
      //}
    }
#if 0
    // Implement inter-wires collisions checking.
    // Sometimes insufficient deflection parameter leads
    // to intersection of discretized curves while the original
    // curves have no intersection. To avoid this we introduce
    // this check to reduce deflection in critical places.
    double aDist = -1;
    CheckCurveCollisions(aEdge, lf, aDist);
    if (aDist > 0)
    {
      double aDefl = aDist * 0.8;
      aSize = aDefl * 2;
      if (aSize < aMeshParams.MinElemSize())
        // correct MinSize
        aMeshParams.SetMinElemSize(aSize);
      if (!aMeshParams.IsDeflection() || aDefl < aMeshParams.Deflection())
        // correct Deflection
        aMeshParams.SetDeflection(aDefl);
    }
#endif
  }

  // Construct curve adaptor
  CurveAdaptor aCurveAdaptor(aCurve, aMeshParams);

  //bool useLocalSizeOfSurface = false;
  bool useLocalSizeOfSurface = true;
  if (useLocalSizeOfSurface) {
    TopTools_ListIteratorOfListOfShape itl(lf);
    for (; itl.More(); itl.Next()) {
      const TopoDS_Face& aFace = TopoDS::Face (itl.Value());
      double fpar, lpar;
      if (!BRep_Tool::CurveOnSurface(aEdge, aFace, fpar, lpar).IsNull())
      {
        BRepAdaptor_Curve2d aCurve2d(aEdge, aFace);
        Handle(BRepAdaptor_HCurve2d) hCurve2d = new BRepAdaptor_HCurve2d(aCurve2d);
        BRepAdaptor_Surface aSurf(aFace, false);
        Handle(BRepAdaptor_HSurface) hSurf = new BRepAdaptor_HSurface(aSurf);
        aCurveAdaptor.AddCurveOnSurface(hCurve2d, hSurf);
      }
    }
  }

  // Invoke discretizer
  const bool isEdgeSameParam =
    (m_mapESameParam( m_mapEF.FindIndex(aEdge) ) > 0);

  TessellateCurve dc (aCurveAdaptor, aMeshParams, isEdgeSameParam,
                      BRep_Tool::Tolerance(aEdge));

  if (!dc.IsDone())
    return false;

  int nbpt = dc.NbPoints();
  aDEdge.Init();
  Curve& dCurve = aDEdge.ChangeCurve();
  dCurve.SetSegmentMinSize(aMeshParams.MinElemSize());
  TColStd_SequenceOfReal& aParams = aDEdge.ChangeParams();

  int i;
  for (i=1; i <= nbpt; i++) {
    const gp_Pnt& pt = dc.Point(i);
    double par = dc.Parameter(i);
    dCurve.Append(pt);
    aParams.Append(par);
  }

  return true;
}

//=======================================================================
//function : DiscretiseEdge
//purpose  : for pcurve
//=======================================================================

bool Builder::DiscretiseEdge
  (const TopoDS_Edge& aEdge, const TopoDS_Face& aFace,
   Edge& aDEdge, const Face& aDFace,
   const bool isEForward)
{
  if (aDEdge.IsDegenerated())
  {
    AddCompStatus(FailureDegeneratedEdge);
    return false;
  }

  double fpar, lpar;
  double fpar3d, lpar3d;
  Handle(Geom2d_Curve) curve = BRep_Tool::CurveOnSurface (aEdge, aFace, fpar, lpar);
  Handle(Geom_Curve) curve3d = BRep_Tool::Curve(aEdge, fpar3d, lpar3d);
  Handle(Geom_Surface) surf = BRep_Tool::Surface(aFace);

  if (curve.IsNull())
  {
    AddCompStatus(FailurePCurveOnSurface);
    return false;
  }

  if (aDEdge.IsNull()) {
    // edge seams to be degenerated
    if (!BRep_Tool::Degenerated(aEdge))
    {
      AddCompStatus(FailureEdgeWithout3DCurve);
      return false;
    }
    // get its two ends
    gp_Pnt2d fpt2d, lpt2d;
    curve->D0(fpar, fpt2d);
    curve->D0(lpar, lpt2d);
    aDEdge.InitDegenerated (aDFace);
    Curve2d& dCurve = aDEdge.ChangePCurve(1);
    dCurve.Append(fpt2d);
    dCurve.Append(lpt2d);
    TColStd_SequenceOfReal& aParams = aDEdge.ChangeParams();
    aParams.Append(fpar);
    aParams.Append(lpar);
    // init 3D curve by one middle point
    gp_Pnt2d mpt2d ((fpt2d.XY() + lpt2d.XY()) * 0.5);
    gp_Pnt mpt;
    surf->D0 (mpt2d.X(), mpt2d.Y(), mpt);
    aDEdge.ChangeCurve().Append(mpt);
    aDEdge.ChangeCurve().Append(mpt);
    // set vertex
    TopoDS_Vertex aFV, aLV;
    TopExp::Vertices(aEdge, aFV, aLV);
    Vertex* aVertex;
    if(m_mapV.Contains(aFV)) {
      aVertex = &m_model->ChangeVertex(m_mapV.FindIndex(aFV));
    }
    else {
      aVertex = &m_model->ChangeVertex(m_model->AddVertex());
      m_mapV.Add(aFV);
      aVertex->ChangePnt() = BRep_Tool::Pnt(aFV);
      aVertex->SetTolerance(BRep_Tool::Tolerance(aFV));
    }
    aDEdge.SetFirstVertex(*aVertex);
    aDEdge.SetLastVertex(*aVertex);
    aVertex->AddEdge(aDEdge);
  }

  else {
    // edge's 3D curve has been already discretised => use its parameters
    int ic = aDEdge.AddPCurve(aDFace,isEForward);
    if (ic == 0)
    {
      AddCompStatus(FailurePCurveRedundancy);
      return false;
    }
    Curve2d& dCurve = aDEdge.ChangePCurve(ic);
    const TColStd_SequenceOfReal& aParams = aDEdge.GetParams();
    bool isSameParam = (m_mapESameParam( m_mapEF.FindIndex(aEdge) ) > 0);
    int nbpt = aDEdge.GetCurve().NbPoints();

    CurveAdaptor::CurveOnSurface aCOS;
    aCOS.Init(new BRepAdaptor_HCurve2d(BRepAdaptor_Curve2d(aEdge, aFace)),
              new BRepAdaptor_HSurface(BRepAdaptor_Surface(aFace, false)));

    // Projection stuff
    const double aProjToler = Max(m_meshParams.IsDeflection() ?
                                         m_meshParams.Deflection() :
                                         Precision::Confusion(), BRep_Tool::Tolerance(aEdge));

    TessellateCurve::ProjectionCache aProjCache;
    aProjCache.IsPrevDefined = true;
    aProjCache.PrevParameter = aParams(1);
    for (int i = 1; i <= nbpt; i++)
    {
      double aNextParam = aParams(i);
      gp_Pnt2d pt;

      if ( isSameParam )
        curve->D0(aNextParam, pt); // Apply just the same discretisation
      else { // Project each discretisation point onto the surface
        gp_Pnt aPnt3d = curve3d->Value(aNextParam);

        // Check projection result
        double aParamOnPCurve;
        if (!TessellateCurve::PntToCurve2d(aPnt3d, aCOS, aProjToler, aProjCache, pt, aParamOnPCurve))
        {
          AddCompStatus(FailureProcessEdge);
          m_badShapes.Add(aEdge);
          return false;
        }
      }

      dCurve.Append(pt);
    }
  }

  return true;
}

//=======================================================================
//function : IsSameParameterEdge
//purpose  : 
//=======================================================================

bool
  Builder::IsSameParameterEdge(const TopoDS_Edge& theEdge,
                               const TColStd_SequenceOfReal& theParams,
                               double& theTolerance,
                               bool& isTolUpdated) const
{
  isTolUpdated = false;

  // Get curve 3D
  double aFirstPar3d, aLastPar3d;
  Handle(Geom_Curve) aCurve3d = BRep_Tool::Curve(theEdge, aFirstPar3d, aLastPar3d);

  if (aCurve3d.IsNull())
    throw Standard_ProgramError("Edge does not contain 3D curve");

  if (Precision::IsInfinite(theTolerance))
    return true;

  GeomAPI_ProjectPointOnCurve aPrj;
  bool isPrjInit = false;
  bool isSamePar = true;
  double aMaxSameParError = 0.;

  // In order to be able to ignore insignificant deviations increase the tolerance;
  // the idea of increase constant value is to allow same-parameter point to deviate
  // from projection point so that to obtain the angle not more than 45 degrees.
  const double anIncreaseCoeff = 1.41421356; // (1./cos(PI/4))
  const double anIncreaseCoeff2 = anIncreaseCoeff * anIncreaseCoeff;
  double anInitTol2 = theTolerance * anIncreaseCoeff;
  anInitTol2 *= anInitTol2;
  double aMaxDist = anInitTol2;

  const TopTools_ListOfShape& aListOfFaces = m_mapEF.FindFromKey(theEdge);
  TopTools_ListIteratorOfListOfShape anIt(aListOfFaces);
  for (; anIt.More(); anIt.Next())
  {
    const TopoDS_Face& aCurrentFace = TopoDS::Face(anIt.Value());

    Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aCurrentFace);
    Handle(GeomAdaptor_HSurface) aSurfAdt = new GeomAdaptor_HSurface(aSurf);

    // Get curve 2D
    double aFirstPar2d, aLastPar2d;
    Handle(Geom2d_Curve) aCurve2d =
      BRep_Tool::CurveOnSurface(theEdge, aCurrentFace, aFirstPar2d, aLastPar2d);

    if (aCurve2d.IsNull())
      // Edge does not contain 2D curve on this face; just skip this face,
      // as later it will be marked as a bad one
      continue;

    // Same range check
    if (Abs(aFirstPar3d - aFirstPar2d) > Precision::Confusion() ||
        Abs(aLastPar3d - aLastPar2d) > Precision::Confusion())
      return false;

    Handle(Geom2dAdaptor_HCurve) aCurve2dAdt =
      new Geom2dAdaptor_HCurve(aCurve2d, aFirstPar2d, aLastPar2d);
    Adaptor3d_CurveOnSurface aCurveOnSurfAdt(aCurve2dAdt, aSurfAdt);

    // Check parameterization against the passed working set of parameters
    for (int i = 2; i < theParams.Length(); i++)
    {
      double aParam = theParams(i);
      gp_Pnt aPntVia3dCrv = aCurve3d->Value(aParam);
      gp_Pnt aPntVia2dCrv = aCurveOnSurfAdt.Value(aParam);

      double aPointDist = aPntVia3dCrv.SquareDistance(aPntVia2dCrv);

      if (aPointDist > aMaxDist)
      {
        // compute orthogonal projection of the point from pcurve on 3D curve
        if (!isPrjInit)
        {
          aPrj.Init(aCurve3d, aFirstPar3d, aLastPar3d);
          isPrjInit = true;
        }
        aPrj.Perform(aPntVia2dCrv);
        if (!aPrj.NbPoints())
          return false;
        double anOrtDist = Square(aPrj.LowerDistance());
        if (anOrtDist < aMaxDist)
        {
          // the orthogonal distance is in tolerance, 
          // it means the property same-parameter is not fulfilled
          isSamePar = false;
          aMaxSameParError = Max(aMaxSameParError, aPointDist);
        }
        else
        {
          // update the tolerance
          aMaxDist = anOrtDist * anIncreaseCoeff2;
          if (!isSamePar && aMaxSameParError < aMaxDist)
            isSamePar = true;
        }
      }
    }
  }

  if (anInitTol2 < aMaxDist)
  {
    theTolerance = Sqrt(aMaxDist / anIncreaseCoeff2);
    isTolUpdated = true;
  }
  return isSamePar;
}

//=======================================================================
//function : IsSignificant
//purpose  : Returns TRUE in case if the given edge is not covered by
//           vertex tolerance, FALSE elsewhere.
//=======================================================================

bool Builder::IsSignificant(const TopoDS_Edge&   theEdge,
                            const Edge&   theDEdge,
                            const TopoDS_Vertex& theVertex)
{
  // Check that whole edge is not covered by vertex tolerance
  const double aTol     = Precision::Confusion();
  const double aEdgeLen = GetEdgeLength(theEdge);
  const double aVertTol = BRep_Tool::Tolerance(theVertex);

  // Isn't it length less than vertex tolerance?
  if ((aEdgeLen - aVertTol) < aTol)
    return false;

  gp_Pnt aRefPnt = BRep_Tool::Pnt(theVertex);
  const Curve& aCurve = theDEdge.GetCurve();
  const double aVertTol2 = aVertTol * aVertTol;
  int aPntIt = 1, aNbPnts = aCurve.NbPoints();
  for (; aPntIt <= aNbPnts; ++aPntIt)
  {
    const gp_Pnt& aPnt = aCurve.Point(aPntIt);
    if (aPnt.SquareDistance(aRefPnt) > aVertTol2)
      return true;
  }

  return false;
}

//=======================================================================
//function : FixFaceWires
//purpose  : 
//=======================================================================

TopoDS_Face Builder::FixFaceWires(const TopoDS_Face& theFace)
{
  TopoDS_Shape aFwdFace = theFace.Oriented(TopAbs_FORWARD);

  // compute the data map vertex->wires
  //TopExp::MapShapesAndAncestors (aFwdFace, TopAbs_VERTEX, TopAbs_WIRE, aMapVW);
  // unfortunately, MapShapesAndAncestors allows duplicates in the list,
  // so we re-implement its behaviour here but with unique items in lists
  TopTools_IndexedDataMapOfShapeListOfShape aMapVW;
  TopExp_Explorer ex1(aFwdFace, TopAbs_WIRE);
  for (; ex1.More(); ex1.Next()) {
    const TopoDS_Shape& aWire = ex1.Current();
    TopTools_MapOfShape aProcessed;
    TopExp_Explorer ex2(aWire, TopAbs_VERTEX);
    for (; ex2.More(); ex2.Next()) {
      const TopoDS_Shape& aVert = ex2.Current();
      if (aProcessed.Contains(aVert)) continue;
      aProcessed.Add(aVert);
      TopTools_ListOfShape empty;
      int ind = aMapVW.Add(aVert, empty);
      aMapVW(ind).Append(aWire);
    }
  }

  // determine groups of wires connected with each other by common vertices;
  // check for condition that a pair of connected wires share
  // one and only one vertex;
  // generate an error in the cases:
  //  - a vertex belongs to more than two wires;
  //  - a pair of wires share more than one vertex
  NCollection_Vector<TopTools_ListOfShape> aGroups;
  TopTools_DataMapOfShapeInteger aShapeGroup;  
  for (int i=1; i <= aMapVW.Extent(); ++i)
  {
    const TopTools_ListOfShape& aWires = aMapVW(i);
    int nb = aWires.Extent();
    if (nb < 2) continue;
    if (nb > 2)
      return TopoDS_Face();

    // perform check for the same pair
    const TopoDS_Shape& aWire1 = aWires.First();
    const TopoDS_Shape& aWire2 = aWires.Last();
    int i1 = -1, i2 = -1;
    if (aShapeGroup.IsBound(aWire1))
      i1 = aShapeGroup(aWire1);
    if (aShapeGroup.IsBound(aWire2))
      i2 = aShapeGroup(aWire2);
    if (i1 != -1 && i1 == i2)
      // these wires are already in the same group, so it is error
      return TopoDS_Face();

    // place wires in a group
    if (i1 != -1 && i2 == -1)
    {
      // join the 2nd wire to the group of the 1st one
      aGroups(i1).Append(aWire2);
      aShapeGroup.Bind(aWire2, i1);
    }
    else if (i1 == -1 && i2 != -1)
    {
      // join the 1st wire to the group of the 2nd one
      aGroups(i2).Append(aWire1);
      aShapeGroup.Bind(aWire1, i2);
    }
    else if (i1 == -1 && i2 == -1)
    {
      // create a new group
      i1 = aGroups.Size();
      aGroups.Append(aWires);
      aShapeGroup.Bind(aWire1, i1);
      aShapeGroup.Bind(aWire2, i1);
    }
    else
    {
      // wires are in different groups, so join the groups,
      // making the 2nd group empty
      aGroups(i1).Append(aGroups(i2));
    }
  }

  if (aGroups.Size() > 0)
  {
    // for each group of wires, replace them with one wire composed from
    // all the edges of the group
    BRepTools_ReShape aReshape;
    int i;
    for (i = 0; i < aGroups.Size(); i++)
    {
      const TopTools_ListOfShape& aGroup = aGroups(i);
      TopoDS_Wire aNewWire;
      BRep_Builder aBld;
      aBld.MakeWire(aNewWire);
      bool isFirst = true;
      TopTools_ListIteratorOfListOfShape itw(aGroup);
      for (; itw.More(); itw.Next())
      {
        TopoDS_Shape aWire = itw.Value();
        TopoDS_Iterator ite(aWire);
        for (; ite.More(); ite.Next())
        {
          TopoDS_Edge aEdge = TopoDS::Edge(ite.Value());
          aBld.Add(aNewWire, aEdge);
        }
        if (isFirst)
        {
          aReshape.Replace(aWire, aNewWire);
          isFirst = false;
        }
        else
          aReshape.Remove(aWire);
      }
    }
    aReshape.Apply(theFace, TopAbs_WIRE);
    TopoDS_Shape aNewFace = aReshape.Value(theFace);
#ifdef _DEBUG_FixFaceWires
    BRepTools::Write(aNewFace, "f.brep");
#endif
    return TopoDS::Face(aNewFace);
  }
  return theFace;
}

//=======================================================================
//function : GetMinSizeForFace
//purpose  : 
//=======================================================================

double Builder::GetMinSizeForFace(const TopoDS_Face& aFace) const
{
  int iFace = m_mapF.FindIndex(aFace);
  double aMin;
  if (!m_mapFMinSize.IsBound(iFace)) {
    // Get average ISO lines restricted by UV bounds
    // and compute their lengths
    double aUMin, aUMax, aVMin, aVMax;
    BRepTools::UVBounds(aFace, aUMin, aUMax, aVMin, aVMax);

    TopLoc_Location aLoc;
    const Handle(Geom_Surface)& aS = BRep_Tool::Surface(aFace,aLoc);
    Handle(Geom_Surface) aTS = aS;

    if (!Handle(Geom_OffsetSurface)::DownCast(aS).IsNull())
    {
      double aSUMin, aSUMax, aSVMin, aSVMax;
      aS->Bounds(aSUMin, aSUMax, aSVMin, aSVMax);

      aUMin = Max(aUMin, aSUMin);
      aUMax = Min(aUMax, aSUMax);
      aVMin = Max(aVMin, aSVMin);
      aVMax = Min(aVMax, aSVMax);

      aTS = new Geom_RectangularTrimmedSurface(aS, aUMin, aUMax, aVMin, aVMax);
    }

    const double aU = (aUMin + aUMax) / 2;
    Handle(Geom_Curve) aC = aTS->UIso(aU);
    GeomAdaptor_Curve aCU(aC, aVMin, aVMax);
    double aLenU = GCPnts_AbscissaPoint::Length(aCU);

    const double aV = (aVMin + aVMax) / 2;
    aC = aS->VIso(aV);
    GeomAdaptor_Curve aCV(aC, aUMin, aUMax);
    double aLenV = GCPnts_AbscissaPoint::Length(aCV);

    aMin = Min (Max(aLenU*myAutoSize, Precision::Confusion()*2),
                Max(aLenV*myAutoSize, Precision::Confusion()*2));

    Standard_Mutex::Sentry aSentry(m_mutex);
    m_mapFMinSize.Bind(iFace, aMin);
  }
  else
    aMin = m_mapFMinSize(iFace);
  return aMin;
}
