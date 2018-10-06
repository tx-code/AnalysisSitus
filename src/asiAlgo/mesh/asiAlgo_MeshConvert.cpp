//-----------------------------------------------------------------------------
// Created on: 14 December 2015
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
//    * Neither the name of Sergey Slyadnev nor the
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
#include <asiAlgo_MeshConvert.h>

// asiAlgo includes
#include <asiAlgo_MeshMerge.h>

// Active Data includes
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

// VTK includes
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------

void ElemNodes(const Handle(ActData_Mesh)&             mesh,
               const Handle(ActData_Mesh_Element)&     elem,
               std::vector<Handle(ActData_Mesh_Node)>& nodes)
{
  nodes.clear();
  int  numNodes = elem->NbNodes();
  int* nodeIDs  = NULL;

  if ( numNodes == 3 )
  {
    const Handle(ActData_Mesh_Triangle)&
      tri = Handle(ActData_Mesh_Triangle)::DownCast(elem);
    //
    nodeIDs = (int*) tri->GetConnections();
  }
  else if ( numNodes == 4 )
  {
    const Handle(ActData_Mesh_Quadrangle)&
      quad = Handle(ActData_Mesh_Quadrangle)::DownCast(elem);
    //
    nodeIDs = (int*) quad->GetConnections();
  }

  // Populate output collection.
  for ( int n = 0; n < numNodes; ++n )
    nodes.push_back( mesh->FindNode(nodeIDs[n]) );
}

//-----------------------------------------------------------------------------

//! Converts internal mesh of the given shape to a persistent form.
//! \param source [in]  source shape (mesh keeper).
//! \param result [out] mesh in a persistent form.
//! \return true in case of success, false -- otherwise.
bool asiAlgo_MeshConvert::ToPersistent(const TopoDS_Shape&   source,
                                       Handle(ActData_Mesh)& result)
{
  asiAlgo_MeshMerge conglomerate(source, asiAlgo_MeshMerge::Mode_Mesh);
  result = conglomerate.GetResultMesh();
  return true;
}

//-----------------------------------------------------------------------------

//! Converts from Poly mesh to AD mesh.
//! \param source [in]  Poly mesh.
//! \param result [out] mesh in a persistent form.
//! \return true in case of success, false -- otherwise.
bool asiAlgo_MeshConvert::ToPersistent(const Handle(Poly_Triangulation)& source,
                                       Handle(ActData_Mesh)&             result)
{
  result = new ActData_Mesh(source);
  return true;
}

//-----------------------------------------------------------------------------

//! Converts AD mesh to Poly mesh. All quads as split into triangle pairs.
//! \param[in]  source AD mesh to convert.
//! \param[out] result result mesh.
bool asiAlgo_MeshConvert::FromPersistent(const Handle(ActData_Mesh)& source,
                                         Handle(Poly_Triangulation)& result)
{
  const int numNodes = source->NbNodes();

  // Loop over the nodes of AD mesh to populate those of Poly triangulation.
  TColgp_Array1OfPnt polyNodes(1, numNodes);
  int arrIdx = 1;
  NCollection_DataMap<int, int> nodeIdsMap;
  //
  for ( ActData_Mesh_ElementsIterator nit(source, ActData_Mesh_ET_Node); nit.More(); nit.Next(), ++arrIdx )
  {
    // Access next node with its owner mesh elements.
    const Handle(ActData_Mesh_Node)&
      node = Handle(ActData_Mesh_Node)::DownCast( nit.GetValue() );

    polyNodes(arrIdx) = node->Pnt();
    nodeIdsMap.Bind(node->GetID(), arrIdx);
  }

  // Prepare triangles.
  std::vector<Poly_Triangle> triangles;
  //
  for ( ActData_Mesh_ElementsIterator it(source, ActData_Mesh_ET_Face); it.More(); it.Next() )
  {
    const Handle(ActData_Mesh_Element)& elem = it.GetValue();

    // Get element's nodes.
    std::vector<Handle(ActData_Mesh_Node)> elemNodes;
    ElemNodes(source, elem, elemNodes);

    // Compute norm vector.
    if ( elem->IsKind( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
    {
      const int n0 = elemNodes[0]->GetID();
      const int n1 = elemNodes[1]->GetID();
      const int n2 = elemNodes[2]->GetID();

      triangles.push_back( Poly_Triangle( nodeIdsMap(n0), nodeIdsMap(n1), nodeIdsMap(n2) ) );
    }
    else if ( elem->IsKind( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
    {
      const int n0 = elemNodes[0]->GetID();
      const int n1 = elemNodes[1]->GetID();
      const int n2 = elemNodes[2]->GetID();
      const int n3 = elemNodes[3]->GetID();

      triangles.push_back( Poly_Triangle( nodeIdsMap(n0), nodeIdsMap(n1), nodeIdsMap(n2) ) );
      triangles.push_back( Poly_Triangle( nodeIdsMap(n0), nodeIdsMap(n2), nodeIdsMap(n3) ) );
    }
  }

  // Construct array of triangles.
  Poly_Array1OfTriangle polyTriangles( 1, int( triangles.size() ) );
  //
  for ( size_t t = 0; t < triangles.size(); ++t )
    polyTriangles( int(t + 1) ) = triangles[t];

  // Construct the result.
  result = new Poly_Triangulation(polyNodes, polyTriangles);
  return true;
}

//-----------------------------------------------------------------------------

//! Converts VTK polygonal data set to the persistent one.
//! \param source [in]  VTK polygonal data set.
//! \param result [out] persistent mesh.
//! \return true in case of success, false -- otherwise.
bool asiAlgo_MeshConvert::ToPersistent(vtkPolyData*          source,
                                       Handle(ActData_Mesh)& result)
{
  // Elements
  const vtkIdType numCells = source->GetNumberOfCells();
  if ( numCells )
    result = new ActData_Mesh;

  // Mapping between VTK nodes against stored nodes
  NCollection_DataMap<vtkIdType, int> NodeMap;

  // Points
  const vtkIdType numPts = source->GetNumberOfPoints();
  for ( vtkIdType ptId = 0; ptId < numPts; ++ptId )
  {
    double coord[3] = {0.0, 0.0, 0.0};
    source->GetPoint(ptId, coord);
    const int storedNodeId = result->AddNode(coord[0], coord[1], coord[2]);
    //
    NodeMap.Bind(ptId, storedNodeId);
  }

  // Loop over the VTK polygonal cells
  vtkSmartPointer<vtkIdList> ptIds = vtkSmartPointer<vtkIdList>::New();
  ptIds->Allocate(VTK_CELL_SIZE);
  //
  for ( vtkIdType cellId = 0; cellId < numCells; ++cellId )
  {
    const int cellType = source->GetCellType(cellId);
    if ( cellType != VTK_TRIANGLE )
    {
      std::cout << "Warning: non-triangular elements are not supported" << std::endl;
      continue;
    }

    // Get the list of points for this cell
    source->GetCellPoints(cellId, ptIds);

    // Add face
    const int id0 = NodeMap.Find( ptIds->GetId(0) );
    const int id1 = NodeMap.Find( ptIds->GetId(1) );
    const int id2 = NodeMap.Find( ptIds->GetId(2) );
    //
    result->AddFace(id0, id1, id2);
  }
  return true;
}
