//-----------------------------------------------------------------------------
// Created on: 11 July 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017-present, Sergey Slyadnev
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
#include <asiVisu_TriangulationSource.h>

// asiAlgo includes
#include <asiAlgo_MeshLink.h>

// asiVisu includes
#include <asiVisu_MeshUtils.h>
#include <asiVisu_Utils.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>

// VTK includes
#include <vtkCellData.h>
#include <vtkDataObject.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#if defined USE_MOBIUS
  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_TriangulationSource)

//-----------------------------------------------------------------------------

asiVisu_TriangulationSource::asiVisu_TriangulationSource()
{
  // Enable collecting edges and vertices.
  this->CollectVerticesModeOn();
  this->CollectEdgesModeOn();

  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_TriangulationSource::~asiVisu_TriangulationSource()
{}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS

void asiVisu_TriangulationSource::SetInputTriangulation(const t_ptr<t_mesh>& triangulation)
{
  m_mesh = triangulation;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

const t_ptr<t_mesh>&
  asiVisu_TriangulationSource::GetInputTriangulation() const
{
  return m_mesh;
}

#endif

//-----------------------------------------------------------------------------

int asiVisu_TriangulationSource::RequestData(vtkInformation*        request,
                                             vtkInformationVector** inputVector,
                                             vtkInformationVector*  outputVector)
{
#if defined USE_MOBIUS
  if ( m_mesh.IsNull() )
  {
    vtkErrorMacro( << "Invalid input: null triangulation" );
    return 0;
  }

  m_regPoints.Clear();

  /* ================
   *  Prepare output.
   * ================ */

  // Get output polygonal data from the information vector.
  vtkPolyData* polyOutput = vtkPolyData::GetData(outputVector);
  polyOutput->Allocate();
  polyOutput->SetPoints( vtkSmartPointer<vtkPoints>::New() );

  // Add array for mesh item types.
  vtkSmartPointer<vtkIdTypeArray> typeArr = vtkSmartPointer<vtkIdTypeArray>::New();
  typeArr->SetName(ARRNAME_MESH_ITEM_TYPE);
  typeArr->SetNumberOfComponents(1);
  polyOutput->GetCellData()->AddArray(typeArr);

  // Array for mesh node IDs.
  vtkSmartPointer<vtkIntArray> nodeIDsArr = asiVisu_Utils::InitIntArray(ARRNAME_MESH_NODE_IDS);
  nodeIDsArr->SetNumberOfComponents(1);
  polyOutput->GetPointData()->AddArray(nodeIDsArr);

  // Add array for mesh element IDs.
  vtkSmartPointer<vtkIdTypeArray> faceIDsArr = vtkSmartPointer<vtkIdTypeArray>::New();
  faceIDsArr->SetName(ARRNAME_MESH_ELEM_IDS);
  faceIDsArr->SetNumberOfComponents(1);
  polyOutput->GetCellData()->SetPedigreeIds(faceIDsArr);

  /* ==============================================================
   *  Take care of free nodes by collecting them into a dedicated
   *  VTK_POLY_VERTEX cell. Notice that such cell (as well as
   *  simple VTK_VERTEX) must be added FIRST when working with
   *  vtkPolyData objects in order to have the data set consistent.
   * ============================================================== */

  // Collect all used nodes.
  NCollection_Map<int> usedNodeIDs;
  //
  for ( t_mesh::TriangleIterator tit(m_mesh); tit.More(); tit.Next() )
  {
    const poly_TriangleHandle th = tit.Current();
    poly_Triangle<>           tri;

    if ( !m_mesh->GetTriangle(th, tri) || tri.IsDeleted() )
      continue;

    poly_VertexHandle vh[3];
    tri.GetVertices(vh[0], vh[1], vh[2]);

    usedNodeIDs.Add(vh[0].iIdx);
    usedNodeIDs.Add(vh[1].iIdx);
    usedNodeIDs.Add(vh[2].iIdx);
  }

  // Collect free nodes (ones which are not used).
  TColStd_PackedMapOfInteger freeNodeIDs;
  //
  for ( t_mesh::VertexIterator vit(m_mesh); vit.More(); vit.Next() )
  {
    const poly_VertexHandle vh = vit.Current();

    poly_Vertex v;
    if ( !m_mesh->GetVertex(vh, v) || v.IsDeleted() )
      continue;

    // Add as a free node.
    if ( !usedNodeIDs.Contains(vh.iIdx) )
    {
      freeNodeIDs.Add(vh.iIdx);
    }
    else if ( m_bVerticesOn )
    {
      this->registerNodeCell(vh.iIdx, MeshPrimitive_SharedNode, polyOutput);
    }
  }
  //
  if ( !freeNodeIDs.IsEmpty() )
  {
    m_progress.SendLogMessage( LogWarn(Normal) << "Num. free nodes: %1." << freeNodeIDs.Extent() );
    //
    this->registerFreeNodesCell(freeNodeIDs, polyOutput);
  }

  /* ===========
   *  Add edges.
   * =========== */

  if ( m_bEdgesOn )
  {
    //   0 -- never happens by construction.
    //   1 -- free link.
    //   2 -- shared link.
    // > 2 -- non-manifold link.
    NCollection_DataMap<asiAlgo_MeshLink, int, asiAlgo_MeshLink> linkOccurenceMap;

    for ( t_mesh::TriangleIterator tit(m_mesh); tit.More(); tit.Next() )
    {
      const poly_TriangleHandle th = tit.Current();
      poly_Triangle<>           tri;

      if ( !m_mesh->GetTriangle(th, tri) || tri.IsDeleted() )
        continue;

      // Get nodes.
      poly_VertexHandle vh[3];
      tri.GetVertices(vh[0], vh[1], vh[2]);

      // Add unoriented links to the set of all links
      asiAlgo_MeshLink l1(vh[0].iIdx, vh[1].iIdx);
      asiAlgo_MeshLink l2(vh[1].iIdx, vh[2].iIdx);
      asiAlgo_MeshLink l3(vh[2].iIdx, vh[0].iIdx);
      //
      if ( linkOccurenceMap.IsBound(l1) )
        linkOccurenceMap.ChangeFind(l1)++;
      else
        linkOccurenceMap.Bind(l1, 1);
      //
      if ( linkOccurenceMap.IsBound(l2) )
        linkOccurenceMap.ChangeFind(l2)++;
      else
        linkOccurenceMap.Bind(l2, 1);
      //
      if ( linkOccurenceMap.IsBound(l3) )
        linkOccurenceMap.ChangeFind(l3)++;
      else
        linkOccurenceMap.Bind(l3, 1);
    }

    // Add links to VTK data set.
    //
    int numFreeLinks = 0;
    int numNonManifoldLinks = 0;
    //
    for ( NCollection_DataMap<asiAlgo_MeshLink, int, asiAlgo_MeshLink>::Iterator lit(linkOccurenceMap);
          lit.More(); lit.Next() )
    {
      asiVisu_MeshPrimitive linkType;

      if ( lit.Value() == 1 )
      {
        numFreeLinks++;
        linkType = MeshPrimitive_BorderLink;
      }
      else if ( lit.Value() == 2 )
      {
        linkType = MeshPrimitive_ManifoldLink;
      }
      else if ( lit.Value() > 2 )
      {
        numNonManifoldLinks++;
        linkType = MeshPrimitive_NonManifoldLink;
      }
      else
      {
        linkType = MeshPrimitive_Undefined;
      }

      if ( lit.Key().N1 == lit.Key().N2 )
        m_progress.SendLogMessage(LogWarn(Normal) << "Topologically degenerated link skipped...");
      else
        this->registerLinkCell(lit.Key().N1, lit.Key().N2, linkType, polyOutput);
    }

    if ( numFreeLinks )
      m_progress.SendLogMessage( LogWarn(Normal) << "Num. free links: %1" << numFreeLinks );
    if ( numNonManifoldLinks )
      m_progress.SendLogMessage( LogWarn(Normal) << "Num. non-manifold links: %1" << numNonManifoldLinks );
  }

  /* ============
   *  Add facets.
   * ============ */

  for ( t_mesh::TriangleIterator tit(m_mesh); tit.More(); tit.Next() )
  {
    const poly_TriangleHandle th = tit.Current();
    poly_Triangle<>           tri;

    if ( !m_mesh->GetTriangle(th, tri) || tri.IsDeleted() )
      continue;

    // Get nodes.
    poly_VertexHandle vh[3];
    tri.GetVertices(vh[0], vh[1], vh[2]);

    // Register VTK cell.
    this->registerFacet(th.iIdx, vh[0].iIdx, vh[1].iIdx, vh[2].iIdx, polyOutput);
  }

  return Superclass::RequestData(request, inputVector, outputVector);
#else
  (void) request;
  (void) inputVector;
  (void) outputVector;

  vtkErrorMacro( << "Mobius is not available." );
  return 0;
#endif
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_TriangulationSource::findMeshNode(const int    nodeID,
                                            vtkPolyData* polyData)
{
#if defined USE_MOBIUS
  // Access necessary arrays
  vtkPoints* points = polyData->GetPoints();

  // Access mesh node
  t_xyz node;
  m_mesh->GetVertex( poly_VertexHandle(nodeID), node );

  vtkIdType resPid;
  if ( !m_regPoints.IsBound(nodeID) )
  {
    // Push the point into VTK data set
    resPid = points->InsertNextPoint( node.X(), node.Y(), node.Z() );
    m_regPoints.Bind(nodeID, resPid);

    // Access array of IDs
    vtkIntArray*
      nodeIDsArr = vtkIntArray::SafeDownCast( polyData->GetPointData()->GetArray(ARRNAME_MESH_NODE_IDS) );
    //
    nodeIDsArr->InsertNextValue(nodeID);
  }
  else
    resPid = m_regPoints.Find(nodeID);

  return resPid;
#else
  (void) nodeID;
  (void) polyData;
  return -1;
#endif
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_TriangulationSource::registerFacet(const int    elemId,
                                             const int    nodeID1,
                                             const int    nodeID2,
                                             const int    nodeID3,
                                             vtkPolyData* polyData)
{
  std::vector<vtkIdType> pids =
  {
    this->findMeshNode(nodeID1, polyData),
    this->findMeshNode(nodeID2, polyData),
    this->findMeshNode(nodeID3, polyData)
  };

  // Register cell
  vtkIdType cellID = polyData->InsertNextCell(VTK_TRIANGLE, 3, &pids[0]);

  // Store element type
  vtkIdTypeArray*
    typeArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ITEM_TYPE) );
  //
  typeArr->InsertNextValue(MeshPrimitive_CellTriangle);

  // Store element ID
  vtkIdTypeArray*
    elemIDsArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ELEM_IDS) );
  //
  elemIDsArr->InsertNextValue(elemId);

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_TriangulationSource::registerFreeNodesCell(const TColStd_PackedMapOfInteger& nodeIDs,
                                                     vtkPolyData*                      polyData)
{
  if ( nodeIDs.IsEmpty() )
    return VTK_BAD_ID;

  std::vector<vtkIdType> pids;
  for ( TColStd_MapIteratorOfPackedMapOfInteger it(nodeIDs); it.More(); it.Next() )
  {
    const int nodeID = it.Key();
    pids.push_back( this->findMeshNode(nodeID, polyData) );
  }

  // Create cell
  vtkIdType
    cellID = polyData->InsertNextCell( VTK_POLY_VERTEX, (int) pids.size(), &pids[0] );

  // Store primitive type
  vtkIdTypeArray*
    typeArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ITEM_TYPE) );
  //
  typeArr->InsertNextValue(MeshPrimitive_FreeNode);

  // Store element ID
  vtkIdTypeArray*
    elemIDsArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ELEM_IDS) );
  //
  elemIDsArr->InsertNextValue(VTK_BAD_ID);

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType
    asiVisu_TriangulationSource::registerLinkCell(const int                   nodeID1,
                                                  const int                   nodeID2,
                                                  const asiVisu_MeshPrimitive type,
                                                  vtkPolyData*                polyData)
{
  if ( nodeID1 == VTK_BAD_ID || nodeID2 == VTK_BAD_ID )
    return VTK_BAD_ID;

  std::vector<vtkIdType> pids;
  pids.push_back( this->findMeshNode(nodeID1, polyData) );
  pids.push_back( this->findMeshNode(nodeID2, polyData) );

  // Register VTK cell
  vtkIdType cellID =
    polyData->InsertNextCell( VTK_LINE, (int) pids.size(), &pids[0] );

  // Set type of the mesh element
  vtkIdTypeArray*
    typeArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ITEM_TYPE) );
  //
  typeArr->InsertNextValue(type);

  // Store element ID
  vtkIdTypeArray*
    elemIDsArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ELEM_IDS) );
  //
  elemIDsArr->InsertNextValue(VTK_BAD_ID);

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType
    asiVisu_TriangulationSource::registerNodeCell(const int                   nodeID,
                                                  const asiVisu_MeshPrimitive type,
                                                  vtkPolyData*                polyData)
{
  if ( nodeID == VTK_BAD_ID )
    return VTK_BAD_ID;

  std::vector<vtkIdType> pids;
  pids.push_back( this->findMeshNode(nodeID, polyData) );

  // Register VTK cell
  vtkIdType cellID =
    polyData->InsertNextCell( VTK_VERTEX, (int) pids.size(), &pids[0] );

  // Set type of the mesh element
  vtkIdTypeArray*
    typeArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ITEM_TYPE) );
  //
  typeArr->InsertNextValue(type);

  // Store element ID
  vtkIdTypeArray*
    elemIDsArr = vtkIdTypeArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_MESH_ELEM_IDS) );
  //
  elemIDsArr->InsertNextValue(nodeID);

  return cellID;
}
