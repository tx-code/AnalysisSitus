//-----------------------------------------------------------------------------
// Created on: 14 May 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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
#include <asiAlgo_RecognizeCavitiesRule.h>

// Analysis Situs includes
#include <asiAlgo_FeatureAttrAdjacency.h>
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_FindBridge.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopExp_Explorer.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

// Defines maximum number of elements in the feature.
const static int MaxNumOfFeatureFaces = 100;

//-----------------------------------------------------------------------------

asiAlgo_RecognizeCavitiesRule::asiAlgo_RecognizeCavitiesRule(const Handle(asiAlgo_AAGIterator)& it,
                                                             const double                       maxSize,
                                                             ActAPI_ProgressEntry               progress,
                                                             ActAPI_PlotterEntry                plotter)
//
: asiAlgo_RecognitionRule (it, progress, plotter),
  m_fMaxSize              (maxSize)
{
  this->init();
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeCavitiesRule::recognize(TopTools_IndexedMapOfShape& featureFaces,
                                              asiAlgo_Feature&            featureIndices)
{
  // Get seed face and iterate over its internal wires.
  const int          sid      = m_it->GetFaceId();
  const TopoDS_Face& seedFace = m_it->GetGraph()->GetFace(sid);

  // Skip already recognized faces.
  if ( featureIndices.Contains(sid) )
    return true;

  const TopoDS_Wire* wirePtr   = m_mapFaceOuterWire.Seek(seedFace);

  std::vector<TopoDS_Wire> wires;
  const TopoDS_Wire  outerWire = ( wirePtr ) ? (*wirePtr)
                                             : asiAlgo_Utils::OuterWire(seedFace);

  // Check the outer contour for subcontours.
  {
    TopTools_IndexedDataMapOfShapeListOfShape verticesEdgesMap;
    TopTools_IndexedDataMapOfShapeListOfShape edgesVerticesMap;
    NCollection_IndexedDataMap<TopoDS_Shape, int, TopTools_ShapeMapHasher> vertexIDMap;
    int id = 0;

    {
      TopExp::MapShapesAndAncestors(outerWire, TopAbs_VERTEX, TopAbs_EDGE, verticesEdgesMap);
      TopTools_IndexedDataMapOfShapeListOfShape::Iterator itVEM(verticesEdgesMap);
      for (; itVEM.More(); itVEM.Next())
      {
        vertexIDMap.Add(itVEM.Key(), id++);

        const TopTools_ListOfShape& listOfEdges = itVEM.Value();
        TopTools_ListOfShape::Iterator itLE(listOfEdges);
        for (; itLE.More(); itLE.Next())
        {
          if (!edgesVerticesMap.Contains(itLE.Value()))
          {
            TopTools_ListOfShape listOfVertices;
            listOfVertices.Append(itVEM.Key());
            edgesVerticesMap.Add(itLE.Value(), listOfVertices);
          }
          else
          {
            edgesVerticesMap.ChangeFromKey(itLE.Value()).Append(itVEM.Key());
          }
        }
      }
    }

    NCollection_IndexedDataMap<TopoDS_Shape, int, TopTools_ShapeMapHasher> edgeIDMap;
    id = 0;
    {
      TopTools_IndexedDataMapOfShapeListOfShape::Iterator itEVM(edgesVerticesMap);
      for (; itEVM.More(); itEVM.Next())
      {
        // Loop in graph.
        if (itEVM.Value().Size() == 1)
        {
          TopoDS_Wire wire = BRepBuilderAPI_MakeWire(TopoDS::Edge(itEVM.Key()));
          wires.push_back(wire);
          continue;
        }

        edgeIDMap.Add(itEVM.Key(), id++);
      }
    }

    std::vector<std::vector<std::pair<int, int>>> graph(vertexIDMap.Size());

    TopTools_IndexedDataMapOfShapeListOfShape::Iterator itVEM(verticesEdgesMap);
    for (; itVEM.More(); itVEM.Next())
    {
      const TopoDS_Vertex& vertex = TopoDS::Vertex(itVEM.Key());
      id = vertexIDMap.FindFromKey(vertex);

      NCollection_IndexedDataMap<TopoDS_Shape, int, TopTools_ShapeMapHasher> vertexMultiplicityMap;

      const TopTools_ListOfShape& listOfEdges = itVEM.Value();
      TopTools_ListOfShape::Iterator itLE(listOfEdges);
      for (; itLE.More(); itLE.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(itLE.Value());
        const TopTools_ListOfShape& listOfVertices = edgesVerticesMap.FindFromKey(edge);
        TopTools_ListOfShape::Iterator itLV(listOfVertices);
        for (; itLV.More(); itLV.Next())
        {
          const TopoDS_Vertex& toVertex = TopoDS::Vertex(itLV.Value());
          if (toVertex.IsEqual(vertex))
          {
            continue;
          }

          if (vertexMultiplicityMap.Contains(toVertex))
          {
            ++(vertexMultiplicityMap.ChangeFromKey(toVertex));
          }
          else
          {
            vertexMultiplicityMap.Add(toVertex, 1);
          }
        }
      }

      std::vector<std::pair<int, int>> connectivityList;
      NCollection_IndexedDataMap<TopoDS_Shape, int, TopTools_ShapeMapHasher>::Iterator itVM(vertexMultiplicityMap);
      for (; itVM.More(); itVM.Next())
      {
        int subId = vertexIDMap.FindFromKey(itVM.Key());
        connectivityList.push_back(std::pair<int, int>(subId, itVM.Value()));
      }
      graph[id] = connectivityList;

    }

    Handle(TopTools_HSequenceOfShape) edgesForContours = new TopTools_HSequenceOfShape();

    asiAlgo_FindBridge bridgeFinder;
    bridgeFinder.Init(graph);
    if (bridgeFinder.Perform())
    {
      const std::vector<std::pair<int, int>>& bridges = bridgeFinder.GetBridges();

      TopTools_IndexedDataMapOfShapeListOfShape::Iterator itEVM(edgesVerticesMap);
      for (; itEVM.More(); itEVM.Next())
      {
        TColStd_PackedMapOfInteger ids;
        const TopoDS_Edge& edge = TopoDS::Edge(itEVM.Key());
        const TopTools_ListOfShape& listOfVertices = edgesVerticesMap.FindFromKey(edge);
        TopTools_ListOfShape::Iterator itLV(listOfVertices);
        for (; itLV.More(); itLV.Next())
        {
          id = vertexIDMap.FindFromKey(itLV.Value());
          ids.Add(id);
        }

        if (ids.Extent() < 2)
        {
          continue;
        }

        bool isFound = false;
        std::vector<std::pair<int, int>>::const_iterator itB = bridges.cbegin();
        for (; itB != bridges.cend(); ++itB)
        {
          if (std::min((*itB).first, (*itB).second) == ids.GetMinimalMapped() &&
            std::max((*itB).first, (*itB).second) == ids.GetMaximalMapped())
          {
            isFound = true;
            break;
          }
        }

        if (isFound)
        {
          continue;
        }

        edgesForContours->Append(edge);
      }
    }

    Handle(TopTools_HSequenceOfShape) auxWires;
    ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edgesForContours, 1e-3, false, auxWires);
    if (!auxWires.IsNull())
    {
      TopTools_HSequenceOfShape::Iterator itAW(*auxWires);
      for (; itAW.More(); itAW.Next())
      {
        const TopoDS_Wire& wire = TopoDS::Wire(itAW.Value());

        BRepBuilderAPI_MakeWire wireMaker;

        for (TopExp_Explorer exp(wire, TopAbs_EDGE); exp.More(); exp.Next())
        {
          const TopoDS_Edge& auxEdge = TopoDS::Edge(exp.Value());
          ShapeAnalysis_Edge edgeAnalysis;
          const TopoDS_Vertex& firstVertex = edgeAnalysis.FirstVertex(auxEdge);
          const TopoDS_Vertex& lastVertex = edgeAnalysis.LastVertex(auxEdge);
          gp_Pnt fPnt = BRep_Tool::Pnt(firstVertex);
          gp_Pnt lPnt = BRep_Tool::Pnt(lastVertex);

          TopTools_HSequenceOfShape::Iterator itEC(*edgesForContours);
          for (; itEC.More(); itEC.Next())
          {
            const TopoDS_Edge& checkedEdge = TopoDS::Edge(itEC.Value());
            const TopoDS_Vertex& firstVertexC = edgeAnalysis.FirstVertex(checkedEdge);
            const TopoDS_Vertex& lastVertexC = edgeAnalysis.LastVertex(checkedEdge);
            gp_Pnt fPntC = BRep_Tool::Pnt(firstVertexC);
            gp_Pnt lPntC = BRep_Tool::Pnt(lastVertexC);
            if (fPnt.IsEqual(fPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(firstVertexC), BRep_Tool::Tolerance(firstVertex)))) &&
              lPnt.IsEqual(lPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(lastVertexC), BRep_Tool::Tolerance(lastVertex)))) ||
              fPnt.IsEqual(lPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(firstVertex), BRep_Tool::Tolerance(lastVertexC)))) &&
              lPnt.IsEqual(fPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(lastVertex), BRep_Tool::Tolerance(firstVertexC)))))
            {
              wireMaker.Add(checkedEdge);
              break;
            }
          }
        }

        const TopoDS_Wire& newWire = wireMaker.Wire();
        if (newWire.IsNull())
        {
          continue;
        }

        bool isOpened = false;
        TopTools_IndexedDataMapOfShapeListOfShape veMap;
        TopExp::MapShapesAndAncestors(newWire, TopAbs_VERTEX, TopAbs_EDGE, veMap);
        for (int index = 1; index <= veMap.Extent(); ++index)
        {
          const TopTools_ListOfShape& edges = verticesEdgesMap(index);

          if (edges.Extent() == 1)
          {
            isOpened = true;
            break;
          }
        }

        if (!isOpened && !newWire.IsPartner(outerWire))
        {
          wires.push_back(newWire);
        }
      }
    }

  }

  // Explore inner wires.
  for ( TopExp_Explorer wit(seedFace, TopAbs_WIRE); wit.More(); wit.Next() )
  {
    const TopoDS_Wire& seedWire = TopoDS::Wire( wit.Current() );

    // Skip outer wire.
    if ( seedWire.IsPartner(outerWire) )
      continue;

    wires.push_back(seedWire);
  }

  std::vector<TopoDS_Wire>::const_iterator itW = wires.cbegin();
  for (; itW != wires.cend(); ++itW)
  {
    const TopoDS_Wire& seedWire = *itW;
    //
    if (seedWire.IsPartner(outerWire))
      continue;

    if ( !this->isConvex(sid, seedWire) )
      continue;

    TopTools_IndexedMapOfShape foundFaces;
    asiAlgo_Feature            foundIds;

    /* Try to complete the candidate cavity feature by recursive propagation */
    {
      TopExp_Explorer eexp(seedWire, TopAbs_EDGE);
      const TopoDS_Edge& anyEdge = TopoDS::Edge( eexp.Current() );

      const asiAlgo_Feature&
        nids = m_it->GetGraph()->GetNeighborsThru(sid, anyEdge);

      bool isOk = true;
      this->propagate(sid, nids, foundFaces, foundIds, isOk);

      if ( !isOk )
        continue; // Finishing face does not have convex adjacency on the inner wire.
    }

    // Check that feature is found.
    if ( (foundIds.Extent() > MaxNumOfFeatureFaces) || !foundIds.Extent() )
    {
      continue; // Protection from incomplete and incorrect features.
    }

    // Check that we do not detect the full solid as a "feature".
    if ( !this->isNotEntireShape(foundFaces, foundIds) )
      continue;

    // Check feature size.
    if ( !this->isSizeOk(foundFaces) )
      continue;

    // Compose the result.
    if ( foundFaces.Size() )
    {
      for ( TopTools_IndexedMapOfShape::Iterator fit(foundFaces); fit.More(); fit.Next() )
        featureFaces.Add( fit.Value() );

      for ( asiAlgo_Feature::Iterator fit(foundIds); fit.More(); fit.Next())
        featureIndices.Add( fit.Key() );
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeCavitiesRule::init()
{
  // Fill map of faces vs their outer wires.
  Handle(asiAlgo_AAGRandomIterator)
    it = new asiAlgo_AAGRandomIterator( m_it->GetGraph() );
  //
  for ( ; it->More(); it->Next() )
  {
    const int          faceId    = it->GetFaceId();
    const TopoDS_Face& face      = m_it->GetGraph()->GetFace(faceId);
    TopoDS_Wire        outerWire = asiAlgo_Utils::OuterWire(face);

    m_mapFaceOuterWire.Bind(face, outerWire);
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeCavitiesRule::propagate(const int                   startId,
                                              const asiAlgo_Feature&      seedIds,
                                              TopTools_IndexedMapOfShape& featureFaces,
                                              asiAlgo_Feature&            featureIndices,
                                              bool&                       isOk)
{
  asiAlgo_Feature nextIterIds;

  for ( asiAlgo_Feature::Iterator sit(seedIds); sit.More(); sit.Next() )
  {
    const int fid = sit.Key();

    featureFaces  .Add( m_it->GetGraph()->GetFace(fid) );
    featureIndices.Add(fid);

    // Iterate over neighbors and find new candidates for the recursive call.
    const asiAlgo_Feature&
      nids = m_it->GetGraph()->GetNeighbors(fid);
    //
    for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
    {
      const int          nid   = nit.Key();
      const TopoDS_Face& nFace = m_it->GetGraph()->GetFace(nid);

      if ( (startId == nid) || seedIds.Contains(nid) )
        continue; // Do not look back.

      if ( featureIndices.Contains(nid) )
        continue; // Skip checked.

      if ( nextIterIds.Contains(nid) )
        continue; // Skip faces that will be checked later on recursively.

      asiAlgo_AAG::t_arc arc(fid, nid);

      // Connecting edges.
      Handle(asiAlgo_FeatureAttrAdjacency)
        feature = m_it->GetGraph()->ATTR_ARC<asiAlgo_FeatureAttrAdjacency>(arc);

      TopTools_IndexedMapOfShape edges;
      feature->GetEdges(edges);

      const TopoDS_Wire* wirePtr   = m_mapFaceOuterWire.Seek(nFace);
      const TopoDS_Wire  outerWire = ( wirePtr ) ? (*wirePtr)
                                                 : asiAlgo_Utils::OuterWire(nFace);

      TopTools_IndexedMapOfShape edgesOW; // Edges on outer wire.
      TopExp::MapShapes(outerWire, TopAbs_EDGE, edgesOW);

      // Check that all edges are on outer wire of neighbor.
      bool isOnOuter = true;
      //
      for ( TopTools_IndexedMapOfShape::Iterator eit(edges); eit.More(); eit.Next() )
      {
        if ( !edgesOW.Contains( eit.Value() ) )
        {
          isOnOuter = false;
          break;
        }
      }

      if ( isOnOuter )
      {
        nextIterIds.Add(nid);
      }
      else
      {
        // We in internal wire. Arc in AAG should have Convex type.
        Handle(asiAlgo_FeatureAttrAngle)
          angAttr = m_it->GetGraph()->ATTR_ARC<asiAlgo_FeatureAttrAngle>( asiAlgo_AAG::t_arc(fid, nid) );

        if ( !asiAlgo_FeatureAngle::IsConvex( angAttr->GetAngleType() ) )
          isOk = false;
      }
    }
  }

  if ( (nextIterIds.Extent() != 0) &&
       (featureIndices.Extent() < MaxNumOfFeatureFaces) ) // Protection from infinite recursion.
  {
    // Continue recursively.
    this->propagate(startId, nextIterIds, featureFaces, featureIndices, isOk);
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeCavitiesRule::isConvex(const int          fid,
                                             const TopoDS_Wire& wire)
{
  bool isConvex = true;

  for ( TopExp_Explorer eexp(wire, TopAbs_EDGE); eexp.More() && isConvex; eexp.Next() )
  {
    const TopoDS_Edge& edgeOnWire = TopoDS::Edge( eexp.Current() );

    // Check that starting inner wire have convex adjacency.
    // Iterate over adjacent faces and check their convexity.
    const asiAlgo_Feature&
      nids = m_it->GetGraph()->GetNeighborsThru(fid, edgeOnWire);
    //
    for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
    {
      const int nid = nit.Key();

      asiAlgo_AAG::t_arc arc(fid, nid);

      // Check the dihedral angle.
      Handle(asiAlgo_FeatureAttrAngle)
        angAttr = m_it->GetGraph()->ATTR_ARC<asiAlgo_FeatureAttrAngle>(arc);
      //
      if ( !asiAlgo_FeatureAngle::IsConvex( angAttr->GetAngleType() ) )
      {
        isConvex = false;
        break;
      }
    }
  }

  return isConvex;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeCavitiesRule::isNotEntireShape(const TopTools_IndexedMapOfShape& newFeatureFaces,
                                                     const asiAlgo_Feature&            newFeatureIndices)
{
  const TopTools_IndexedMapOfShape&
    allFaces = m_it->GetGraph()->GetMapOfFaces();

  // Check that not full solid is found.
  if ( allFaces.Size() != (newFeatureIndices.Extent() + 1) )
    return true; // Full part except base face is found.

  bool allFacesFound = true;

  // Check feature faces.
  for ( TopTools_IndexedMapOfShape::Iterator fit(newFeatureFaces); fit.More(); fit.Next() )
  {
    if ( !allFaces.Contains( fit.Value() ) )
    {
      allFacesFound = false;
      break;
    }
  }

  return !allFacesFound;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeCavitiesRule::isSizeOk(const TopTools_IndexedMapOfShape& newFeatureFaces)
{
  if ( Precision::IsInfinite(m_fMaxSize) || ( Abs(m_fMaxSize) < gp::Resolution() ) )
    return true; // Early exit when maximal size is not set.

  // Iterate over faces and check the size criteria.
  TopoDS_Compound comp;
  BRep_Builder builder; builder.MakeCompound(comp);
  TopTools_IndexedMapOfShape::Iterator facesIter(newFeatureFaces);
  //
  for ( ; facesIter.More(); facesIter.Next() )
    builder.Add( comp, facesIter.Value() );

  double xMin, yMin, zMin, xMax, yMax, zMax;
  asiAlgo_Utils::Bounds(comp, xMin, yMin, zMin, xMax, yMax, zMax, 0., false);

  const double dim[3] = { Abs(xMax - xMin), Abs(yMax - yMin), Abs(zMax - zMin) };

  gp_Pnt pntMin(xMin, yMin, zMin);
  gp_Pnt pntMax(xMax, yMax, zMax);
  //
  const double size = Max( dim[0], Max(dim[1], dim[2]) );

  if ( size - m_fMaxSize > Precision::Confusion() )
    return false;

  return true;
}
