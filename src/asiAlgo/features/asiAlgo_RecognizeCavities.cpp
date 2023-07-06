//-----------------------------------------------------------------------------
// Created on: 14 May 2020
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
#include <asiAlgo_RecognizeCavities.h>

// asiAlgo includes
#include <asiAlgo_RecognizeCavitiesRule.h>

// Analysis Situs includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_AAGIterator.h>
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_FindBridge.h>

// OpenCascade includes
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopExp_Explorer.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAlgo_RecognizeCavities::asiAlgo_RecognizeCavities(const TopoDS_Shape&  shape,
                                                     ActAPI_ProgressEntry progress,
                                                     ActAPI_PlotterEntry  plotter)
//
: asiAlgo_Recognizer ( shape, nullptr, progress, plotter ),
  m_fMaxSize         ( Precision::Infinite() )
{}

//-----------------------------------------------------------------------------

asiAlgo_RecognizeCavities::asiAlgo_RecognizeCavities(const Handle(asiAlgo_AAG)& aag,
                                                     ActAPI_ProgressEntry       progress,
                                                     ActAPI_PlotterEntry        plotter)
//
: asiAlgo_Recognizer ( aag, progress, plotter ),
  m_fMaxSize         ( Precision::Infinite() )
{}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeCavities::SetMaxSize(const double maxSize)
{
  m_fMaxSize = maxSize;
}

//-----------------------------------------------------------------------------

double asiAlgo_RecognizeCavities::GetMaxSize() const
{
  return m_fMaxSize;
}

//-----------------------------------------------------------------------------

const asiAlgo_RecognizeCavities::t_cavities&
  asiAlgo_RecognizeCavities::GetCavities() const
{
  return m_cavities;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeCavities::Perform()
{
  // Clean up the result.
  m_result.faces.Clear();
  m_result.ids.Clear();

  /* ====================
   *  Stage 1: build AAG
   * ==================== */

  // Build AAG if not available.
  if ( m_aag.IsNull() )
  {
#if defined COUT_DEBUG
    TIMER_NEW
    TIMER_GO
#endif

    m_aag = new asiAlgo_AAG(m_master, false);

#if defined COUT_DEBUG
    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Construct AAG")
#endif
  }

  /* ===========================
   *  Stage 2: recognition loop
   * =========================== */

#if defined COUT_DEBUG
  TIMER_NEW
  TIMER_GO
#endif

  // Find promising seed faces to start recognition from.
  asiAlgo_Feature seeds, traversed;
  this->findSeeds(seeds);
  //
  Handle(asiAlgo_AAGSetIterator)
    sit = new asiAlgo_AAGSetIterator(m_aag, seeds);

  // Recognize in a loop having the rule as a cursor.
  Handle(asiAlgo_RecognizeCavitiesRule)
    rule = new asiAlgo_RecognizeCavitiesRule(sit,
                                             m_fMaxSize,
                                             m_progress,
                                             m_plotter);
  //
  for ( ; sit->More(); sit->Next() )
  {
    // Recognizer iterates some faces internally. We don't want to
    // use such faces as seeds, so we skip them here.
    if ( traversed.Contains( sit->GetFaceId() ) )
      continue;

    // Attempt to recognize locally.
    if ( rule->Recognize(m_result.faces, m_result.ids) )
    {
      // Pick up those faces iterated by the recognizer and exclude them
      // from the list to iterate.
      traversed.Unite( rule->JustTraversed() );
    }

    // The recognition process might have been cancelled, so let's check
    // it here in return just in case.
    if ( m_progress.IsCancelling() )
    {
      return false;
    }
  }

  // Initialize cavities.
  this->collectCavities();

#if defined COUT_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("Cavity recognition")
#endif

  return true; // Success.
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeCavities::findSeeds(asiAlgo_Feature& seeds)
{
  seeds.Clear();

  // Iterate in random order.
  Handle(asiAlgo_AAGRandomIterator)
    it = new asiAlgo_AAGRandomIterator(m_aag);
  //
  asiAlgo_Feature traversed;
  //
  for ( ; it->More(); it->Next() )
  {
    const int fid = it->GetFaceId();
    //
    if ( traversed.Contains(fid) )
      continue;

    const TopoDS_Face& face = m_aag->GetFace(fid);

    //
    std::vector<TopoDS_Wire> wires;
    const TopoDS_Wire& outerWire = asiAlgo_Utils::OuterWire(face);

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

        if (bridges.size())
        {
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

          if (!isOpened)
          {
            wires.push_back(newWire);
            m_plotter.DRAW_SHAPE(newWire, "Outer");
          }
        }
      }

    }

    // Loop over the inner wires.
    for (TopExp_Explorer wexp(face, TopAbs_WIRE); wexp.More(); wexp.Next())
    {
      const TopoDS_Wire& wire = TopoDS::Wire(wexp.Current());
      //
      if (wire.IsPartner(outerWire))
        continue;

      wires.push_back(wire);
    }

    std::vector<TopoDS_Wire>::const_iterator itW = wires.cbegin();
    for (; itW != wires.cend(); ++itW)
    {
      const TopoDS_Wire& wire = *itW;
      //
      if ( wire.IsPartner(outerWire) )
        continue;

      /* The following code is dealing with inner contours only */

      bool isConvexOnly = true;
      asiAlgo_Feature nids;

      // Loop over the inner edges to check vexity.
      for ( TopExp_Explorer eexp(wire, TopAbs_EDGE); eexp.More(); eexp.Next() )
      {
        const TopoDS_Edge& edgeOnWire = TopoDS::Edge( eexp.Current() );

        // Get neighbors across the current edge.
        const asiAlgo_Feature&
          edgeNids = m_aag->GetNeighborsThru(fid, edgeOnWire);
        //
        if ( !edgeNids.Extent() ) // Protection, just in case.
        {
          isConvexOnly = false;
          break;
        }

        // All neighbors have to be convex-adjacent to the base face.
        for ( asiAlgo_Feature::Iterator nit(edgeNids); nit.More(); nit.Next() )
        {
          const int nid = nit.Key();

          asiAlgo_AAG::t_arc arc(fid, nid);

          // Get angle to check for vexity.
          Handle(asiAlgo_FeatureAttrAngle)
            attrAngle = m_aag->ATTR_ARC<asiAlgo_FeatureAttrAngle>(arc);
          //
          if ( attrAngle.IsNull() )
            continue;

          // Check vexity.
          if ( !asiAlgo_FeatureAngle::IsConvex( attrAngle->GetAngleType() ) )
          {
            isConvexOnly = false;
            break;
          }
        }

        // Add to the collection of traversed faces.
        nids.Unite(edgeNids);
      }

      // If all inner wires are convex, take current face as another seed.
      if ( isConvexOnly )
      {
        traversed.Unite(nids);
        seeds.Add(fid);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeCavities::collectCavities()
{
  std::vector<asiAlgo_Feature> ccomps;

  m_aag->PushSubgraph(m_result.ids);
  {
    m_aag->GetConnectedComponents(ccomps);
  }
  m_aag->PopSubgraph();

  for ( const auto& ccomp : ccomps )
  {
    asiAlgo_Feature bases;

    // Collect neighbor faces that are not cavities. These would be the bases.
    for ( asiAlgo_Feature::Iterator fit(ccomp); fit.More(); fit.Next() )
    {
      const int fid = fit.Key();

      // Collect base faces by traversing all the neighbors.
      const asiAlgo_Feature& nids = m_aag->GetNeighbors(fid);
      //
      for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
      {
        const int nid = nit.Key();

        // Skip feature faces.
        if ( m_result.ids.Contains(nid) || bases.Contains(nid) )
          continue;

        bases.Add(nid);
      }
    } // by connected components

    // Add to the result.
    m_cavities.push_back({ccomp, bases});
  }
}
