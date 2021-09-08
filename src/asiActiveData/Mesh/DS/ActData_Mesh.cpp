//-----------------------------------------------------------------------------
// Created on: June 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_Mesh.h>

// Mesh includes
#include <ActData_Mesh_Edge.h>
#include <ActData_Mesh_EdgesIterator.h>
#include <ActData_Mesh_Element.h>
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Face.h>
#include <ActData_Mesh_FacesIterator.h>
#include <ActData_Mesh_IDFactory.h>
#include <ActData_Mesh_Node.h>
#include <ActData_Mesh_NodesIterator.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

// OCCT includes
#include <gp_XYZ.hxx>

ActData_Mesh::ActData_Mesh(const Handle(Poly_Triangulation)& tri)
  : myNodeIDFactory     (new ActData_Mesh_IDFactory),
    myElementIDFactory  (new ActData_Mesh_IDFactory)
{
  for ( int node_idx = 1; node_idx <= tri->Nodes().Length(); ++node_idx )
  {
    this->AddNode( tri->Nodes()(node_idx).X(), tri->Nodes()(node_idx).Y(), tri->Nodes()(node_idx).Z() );
  }

  for ( int tri_idx = 1; tri_idx <= tri->Triangles().Length(); ++tri_idx )
  {
    int nodes[3] = {0, 0, 0};
    tri->Triangles()(tri_idx).Get(nodes[0], nodes[1], nodes[2]);

    this->AddFace(nodes, 3);
  }
}

//=======================================================================
//function : Mesh
//purpose  : creation of a new mesh object
//=======================================================================
ActData_Mesh::ActData_Mesh(const int nbnodes,
                           const int nbedges,
                           const int nbfaces)
  : myNodeIDFactory     (new ActData_Mesh_IDFactory), 
    myElementIDFactory  (new ActData_Mesh_IDFactory),
    myNodes             (nbnodes /*, new NCollection_IncAllocator*/),
    myEdges             (nbedges),
    myFaces             (nbfaces),
    myHasInverse        (Standard_False)
{
}

//=======================================================================
//function : Clear
//purpose  : Destructor
//           This code is to release the elements of removed mesh to clean
//           memory space and avoid future collisions
//=======================================================================

void ActData_Mesh::Clear (const Standard_Boolean isClearNodes)
{
  if (isClearNodes)
    RebuildAllInverseConnections();
  ActData_Mesh_VectorOfElements::Iterator anIter = myElementIDFactory -> Iterator();
  for (; anIter.More(); anIter.Next()) {
    Handle(ActData_Mesh_Element)& anElem = anIter.ChangeValue();
    if (!anElem.IsNull())
      if (anElem->GetMesh() == this) {
        if (myHasInverse) {
          Standard_Integer i, nbcnx = anElem->NbNodes();
          for (i=1; i <= nbcnx; ++i) {
            const Handle(ActData_Mesh_Node) aNode = GetNode(i, anElem);
            RemoveInverseElement(aNode, anElem);
            if (isClearNodes && aNode -> InverseElements().IsEmpty())
              FreeNode (aNode);
          }
        }
        myElementIDFactory -> ReleaseID(anElem -> GetID());
      }
  }
}

//=======================================================================
//function : AddNode
//purpose  : create a MeshNode and returns an ID
//=======================================================================

Standard_Integer ActData_Mesh::AddNode(const Standard_Real x, 
                                       const Standard_Real y, 
                                       const Standard_Real z)
{
  Standard_Integer ID;
  for (;;) {
    ID = myNodeIDFactory->GetFreeID();
    if (AddNodeWithID(x,y,z,ID)) break;
  }
  return ID;
}

//=======================================================================
//function : AddNode
//purpose  : create a MeshNode. Returns False if the ID already exists
//=======================================================================

Standard_Boolean ActData_Mesh::AddNodeWithID(const Standard_Real x, 
                                             const Standard_Real y, 
                                             const Standard_Real z,
                                             const Standard_Integer ID)
{
  // find the MeshNode corresponding to ID
  Handle(ActData_Mesh_Element) node;
  node = GetNode(ID);

  if (node.IsNull()) {
    node = new ActData_Mesh_Node(ID,x,y,z);
    AddNode(node);
    return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::AddNode(const Handle(ActData_Mesh_Element)& node)
{
  if (node.IsNull())
    return Standard_False;

  const Standard_Integer anID = node -> GetID();
  if (myNodes.Add (anID)) {
    myNodeIDFactory -> BindID (anID, node);
  }
  return Standard_True;
}

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh::AddEdge(const Standard_Integer idnode1,
                                       const Standard_Integer idnode2)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddEdgeWithID(idnode1,idnode2,ID))
    return ID;
  return 0;
}

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::AddEdgeWithID(const Standard_Integer idnode1,
                                             const Standard_Integer idnode2,
                                             const Standard_Integer ID)
{
  Handle(ActData_Mesh_Element) edge,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (myNodes.Contains(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (myNodes.Contains(idnode2)) {
      elem = CreateEdge(ID,idnode1,idnode2);
      if (FindEdge(elem).IsNull())
        if (myElementIDFactory->BindID(ID,elem)) {
          successAdd = Standard_True;
          myEdges.Add(elem);
          if (myHasInverse) 
            AddInverseElement(elem);
        }
    }
  }
  return successAdd;
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh::AddFace(const Standard_Integer idnode1, 
                                       const Standard_Integer idnode2,
                                       const Standard_Integer idnode3)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddFaceWithID(idnode1,idnode2,idnode3,ID))
    return ID;
  else 
    return 0;

}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::AddFaceWithID(const Standard_Integer idnode1, 
                                             const Standard_Integer idnode2,
                                             const Standard_Integer idnode3,
                                             const Standard_Integer ID)
{
  Handle(ActData_Mesh_Element) elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (myNodes.Contains(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (myNodes.Contains(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (myNodes.Contains(idnode3)) {
        elem = CreateFace(ID,idnode1,idnode2,idnode3);
        if (FindFace(elem).IsNull())
          if (myElementIDFactory->BindID(ID,elem)) {
            successAdd = Standard_True;
            myFaces.Add(elem);
            if (myHasInverse) 
              AddInverseElement(elem);
          }
      }
    }
  }
  return successAdd;
}


//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh::AddFace(const Standard_Integer idnode1, 
                                       const Standard_Integer idnode2,
                                       const Standard_Integer idnode3,
                                       const Standard_Integer idnode4)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddFaceWithID(idnode1,idnode2,idnode3,idnode4,ID))
    return ID;
  else
    return 0;

}


//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::AddFaceWithID(const Standard_Integer idnode1, 
                                             const Standard_Integer idnode2,
                                             const Standard_Integer idnode3,
                                             const Standard_Integer idnode4,
                                             const Standard_Integer ID)
{
  Handle(ActData_Mesh_Element) face,elem;
  Standard_Boolean successAdd = Standard_False;

  // find the MeshNode corresponding to idnode1
  if (myNodes.Contains(idnode1)) {
    // find the MeshNode corresponding to idnode2
    if (myNodes.Contains(idnode2)) {
      // find the MeshNode corresponding to idnode3
      if (myNodes.Contains(idnode3)) {
        // find the MeshNode corresponding to idnode4
        if (myNodes.Contains(idnode4)) {
          elem = CreateFace(ID,idnode1,idnode2,idnode3,idnode4);
          if (FindFace(elem).IsNull())
            if (myElementIDFactory->BindID(ID,elem)) {
              successAdd = Standard_True;
              myFaces.Add(elem);
              if (myHasInverse) 
                AddInverseElement(elem);
            }
        }
      }
    }
  }
  return successAdd;
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh::AddFace(const Standard_Address theIDnodes,
                                       const Standard_Integer theNbNodes)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();
  
  if (AddFaceWithID(theIDnodes, theNbNodes, ID))
    return ID;
  else 
    return 0;
}

//=======================================================================
//function : AddFaceWithID
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::AddFaceWithID(const Standard_Address theIDnodes,
                                             const Standard_Integer theNbNodes,
                                             const Standard_Integer theID)
{
  Standard_Boolean ok = Standard_False;
  Standard_Integer* anIDs = (Standard_Integer*) theIDnodes;
  if (theNbNodes == 3)
    ok = AddFaceWithID(anIDs[0], anIDs[1], anIDs[2], theID);
  else if (theNbNodes == 4)
    ok = AddFaceWithID(anIDs[0], anIDs[1], anIDs[2], anIDs[3], theID);
  return ok;
}

//=======================================================================
//function : AddElement
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh::AddElement(const Handle(ActData_Mesh_Element)& theElem)
{
  Standard_Integer ID = myElementIDFactory->GetFreeID();

  if (AddElementWithID(theElem, ID))
    return ID;
  else
    return 0;
}

//=======================================================================
//function : AddElementWithID
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::AddElementWithID
                                    (const Handle(ActData_Mesh_Element)& theElem,
                                     const Standard_Integer          theID)
{
  Standard_Boolean aResult (Standard_False);
  // check existence of nodes
  Standard_Integer i, nbConn = theElem->NbNodes();
  for (i=1; i <= nbConn; i++) {
    const Standard_Integer aNodeID = theElem->GetConnection(i);
    if (!myNodes.Contains(aNodeID))
      break;
  }

  if (i > nbConn &&         // i.e., all nodes are safely located in the mesh
      myElementIDFactory->MeshElement(theID).IsNull()) // ID has no duplicate
  {
    Handle(ActData_Mesh_Element) anElemInMesh;

    if (theElem->IsKind(STANDARD_TYPE(ActData_Mesh_Edge))) {
      anElemInMesh = FindEdge(theElem);
      if (anElemInMesh.IsNull()) {
        anElemInMesh = theElem->Copy(theID);
        static_cast <ActData_Mesh_Edge *> (anElemInMesh.operator->()) -> myMesh =
          const_cast<ActData_Mesh *> (this);
        myEdges.Add(anElemInMesh);
      }
    }

    else if (theElem->IsKind(STANDARD_TYPE(ActData_Mesh_Face))) {
      anElemInMesh = FindFace(theElem);
      if (anElemInMesh.IsNull()) {
        anElemInMesh = theElem->Copy(theID);
        static_cast <ActData_Mesh_Face *> (anElemInMesh.operator->()) -> myMesh =
          const_cast<ActData_Mesh *> (this);
        myFaces.Add(anElemInMesh);
      }
    }

    if (!anElemInMesh.IsNull()) {
      if (myHasInverse)
        AddInverseElement(anElemInMesh);
      aResult = myElementIDFactory->BindID(theID,anElemInMesh);
    }
  }
  return aResult;
}


//=======================================================================
//function : GetNode
//purpose  : returns the MeshNode corresponding to the ID
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::GetNode(const Standard_Integer idnode) const
{

  Handle(ActData_Mesh_Element) node;

  Handle(ActData_Mesh_Element) elem = FindNode(idnode);
  if (!elem.IsNull()) { // found one correspondence
    node =  elem;
  }

  return node;
}

//=======================================================================
//function : FindNode
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Node) ActData_Mesh::FindNode(const Standard_Integer ID) const
{
  ActData_Mesh_Node * aNode = 0L;
  if (myNodes.Contains(ID))
    aNode = (ActData_Mesh_Node *) myNodeIDFactory -> MeshElement(ID).operator->();
  return aNode;
}

//=======================================================================
//function : FindNode
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Node) ActData_Mesh::FindNode
                                (const Handle(ActData_Mesh_Element)& node) const
{
  ActData_Mesh_Node * aNode = 0L;
  const Standard_Integer anID = node -> GetID();
  if (myNodes.Contains(anID))
    aNode = (ActData_Mesh_Node *) myNodeIDFactory -> MeshElement(anID).operator->();
  return aNode;
}

//=======================================================================
//function : CreateEdge
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::CreateEdge(const Standard_Integer ID,
                                              const Standard_Integer idnode1,
                                              const Standard_Integer idnode2) const
{
  Handle(ActData_Mesh_Edge) edge = new ActData_Mesh_Edge(ID,idnode1,idnode2);
  edge -> myMesh = const_cast<ActData_Mesh *> (this);
  return edge;
}


//=======================================================================
//function : CreateFace
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::CreateFace(const Standard_Integer ID,
                                              const Standard_Integer idnode1,
                                              const Standard_Integer idnode2,
                                              const Standard_Integer idnode3) const
{
  Handle(ActData_Mesh_Face) face =
    new ActData_Mesh_Triangle(ID,idnode1,idnode2,idnode3);
  face -> myMesh = const_cast<ActData_Mesh *> (this);
  return face;
}


//=======================================================================
//function : CreateFace
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::CreateFace(const Standard_Integer ID,
                                              const Standard_Integer idnode1,
                                              const Standard_Integer idnode2,
                                              const Standard_Integer idnode3,
                                              const Standard_Integer idnode4) const
{
  Handle(ActData_Mesh_Face) face = new ActData_Mesh_Quadrangle(ID,idnode1,idnode2,
                                                         idnode3,idnode4);
  face -> myMesh = const_cast<ActData_Mesh *> (this);
  return face;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::Contains(const Handle(ActData_Mesh_Element)& elem) const
{
  Standard_Boolean isInMesh = Standard_False;
  switch (elem -> GetType()) {
  case ActData_Mesh_ET_Node:
    isInMesh = myNodes.Contains (elem -> GetID());
    break;
  case ActData_Mesh_ET_Edge:
    isInMesh = ((this == elem->GetMesh()) && myEdges.Contains(elem));
    break;
  case ActData_Mesh_ET_Face:
    isInMesh = ((this == elem->GetMesh()) && myFaces.Contains(elem));
    break;
  default: ;
  }
  return isInMesh;
}

//=======================================================================
//function : FindEdge
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::FindEdge(const Handle(ActData_Mesh_Element)& edge) const
{
  Handle(ActData_Mesh_Element) elem;
  if (myEdges.Contains(edge))
    elem = myEdges.Find(edge);

  return elem;
}

//=======================================================================
//function : FindFace
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::FindFace
                                (const Handle(ActData_Mesh_Element)& face) const
{
  Handle(ActData_Mesh_Element) elem;
  if (myFaces.Contains(face))
    elem = myFaces.Find(face);

  return elem;
}


//=======================================================================
//function : FreeNode
//purpose  : 
//=======================================================================

void ActData_Mesh::FreeNode(const Handle(ActData_Mesh_Element)& node)
{
  if (!node.IsNull()) {
    const Standard_Integer anID = node -> GetID();
    if (myNodes.Remove (anID)) {
        myNodeIDFactory->ReleaseID(anID);
    }
  }
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::RemoveNode(const Standard_Integer ID,
                                          const Standard_Boolean OnlyFree,
                                          const Standard_Boolean DeleteFreeNodes)
{
  Standard_Boolean aResult (Standard_False);
  // find the MeshNode corresponding to ID
  if (myNodes.Contains(ID)) {
    const Handle(ActData_Mesh_Node) aNode =
      (ActData_Mesh_Node *) myNodeIDFactory->MeshElement(ID).operator->();
    if (!aNode.IsNull()) {
      // If the node is not empty, do the actual removal
      if (!myHasInverse)
        RebuildAllInverseConnections();
      // to avoid the references on the non-existent nodes
      if (!aNode->InverseElements().IsEmpty() && OnlyFree)
        // remove only the free node but the node has inverse elements
        return aResult;
      aResult = Standard_True;
      if (!OnlyFree)
        RemoveAncestors (aNode, DeleteFreeNodes);
      FreeNode (aNode); 
    }
  }
  return aResult;
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::RemoveNode(const Handle(ActData_Mesh_Element)& aNode)
{
  Standard_Boolean aResult (Standard_False);
  if (!aNode.IsNull()) {
    const Standard_Integer anID = aNode -> GetID();
    if (aNode->GetType() == ActData_Mesh_ET_Node && myNodes.Contains(anID)) {
      // do the actual removal
      aResult = Standard_True;
      RemoveAncestors (aNode);
      FreeNode (aNode);
    }
  }
  return aResult;
}

//=======================================================================
//function : RemoveEdge
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveEdge (const Standard_Integer idnode1,
                               const Standard_Integer idnode2)
{
  Handle(ActData_Mesh_Element) edge = FindEdge(idnode1,idnode2);
  RemoveEdge(edge);
}

//=======================================================================
//function : RemoveEdge
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveEdge(const Handle(ActData_Mesh_Element)& edge)
{
  if (!edge.IsNull())
    if (myEdges.Remove(edge))
      myElementIDFactory->ReleaseID(edge->GetID());
}

//=======================================================================
//function : RemoveFace
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveFace(const Standard_Integer idnode1, 
                              const Standard_Integer idnode2,
                              const Standard_Integer idnode3)
{
  Handle(ActData_Mesh_Element) face = FindFace(idnode1,idnode2,idnode3);
  RemoveFace(face);
}

//=======================================================================
//function : RemoveFace
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveFace(const Standard_Integer idnode1, 
                              const Standard_Integer idnode2,
                              const Standard_Integer idnode3,
                              const Standard_Integer idnode4)
{
  Handle(ActData_Mesh_Element) face = FindFace(idnode1,idnode2,idnode3,idnode4);
  RemoveFace(face);
}


//=======================================================================
//function : RemoveFace
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveFace(const Handle(ActData_Mesh_Element)& face)
{
  if (!face.IsNull())
    if (myFaces.Remove(face))
      myElementIDFactory->ReleaseID(face->GetID());
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveElement (const Standard_Integer IDelem,
                                  const Standard_Boolean removenodes)
{
  Handle(ActData_Mesh_Element) elem =  myElementIDFactory->MeshElement(IDelem);
  RemoveElement(elem,removenodes);
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveElement (const Handle(ActData_Mesh_Element)& theElem,
                                  const Standard_Boolean          removenodes)
{
  // create copy of handle to avoid entity nullifying till the end of scope
  Handle(ActData_Mesh_Element) elem = theElem;
  switch (elem -> GetType()) {
  case ActData_Mesh_ET_Node:
    RemoveNode(elem);
    return;
  case ActData_Mesh_ET_Edge:
    RemoveEdge(elem);
    break;
  case ActData_Mesh_ET_Face:
    RemoveFace(elem);
    break;
  default:
    std::cout << "remove function : unknown type" << std::endl;
    return;
  }

  Standard_Integer i, nbcnx = elem->NbNodes();
  for (i=1; i <= nbcnx; ++i)
    RemoveInverseElement(GetNode(i,elem),elem);

  if (removenodes) {
    if (!myHasInverse)
      RebuildAllInverseConnections();
    for (i = 1; i <= nbcnx; ++i)
      if (GetNode(i,elem)->InverseElements().IsEmpty())
        FreeNode(GetNode(i,elem));
  }
}

//=======================================================================
//function : RemoveInverseElement
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveInverseElement(const Handle(ActData_Mesh_Element)& anElem, 
                                        const Handle(ActData_Mesh_Element)& parent) const
{
  if (myHasInverse && anElem->GetType() == ActData_Mesh_ET_Node) {
    Handle(ActData_Mesh_Node) aNode = Handle(ActData_Mesh_Node)::DownCast (anElem);
    aNode->RemoveInverseElement(parent);
  }
}

//=======================================================================
//function : AddInverseElement
//purpose  : adds the inverse element
//=======================================================================

void ActData_Mesh::AddInverseElement(const Handle(ActData_Mesh_Element)& ME) const
{
  if (ME.IsNull())
    return;
  const Standard_Integer nbcnx = ME->NbNodes();
  for (Standard_Integer inode=1; inode <= nbcnx; ++inode) {
    Standard_Integer idnode = ME->GetConnection(inode);
    Handle(ActData_Mesh_Element) aNode = FindNode(idnode);
    if (!aNode.IsNull())
      aNode->AddInverseElement(ME);
  }
}

//=======================================================================
//function : RemoveAncestors
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveAncestors(const Handle(ActData_Mesh_Element)& anElem,
                                   const Standard_Boolean DeleteFreeNodes)
{
  if (myHasInverse && anElem->GetType() == ActData_Mesh_ET_Node) {
    Handle(ActData_Mesh_Node) aNode = Handle(ActData_Mesh_Node)::DownCast(anElem);
    ActData_Mesh_ListOfElements lstInvElements;
    lstInvElements = aNode -> InverseElements();
    //tma: create a copy of the list to avoid the invalid iterator;
    //list nodes are removed during the iteration process
    ActData_Mesh_ListOfElements::Iterator it(lstInvElements);
    for (;it.More();it.Next()) {
      Handle(ActData_Mesh_Element) anInvElem = it.Value();
      // tma: if the mesh is root then remove all elements
      // built on anElem
      if (Contains(anInvElem))
          RemoveElement(anInvElem, DeleteFreeNodes);
    }
  }
}


//=======================================================================
//function : FindEdge
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::FindEdge(const Standard_Integer idnode1,
                                            const Standard_Integer idnode2 ) const
{
  Handle(ActData_Mesh_Edge) edge = new ActData_Mesh_Edge(0,idnode1,idnode2);
  return FindEdge(edge);
}

//=======================================================================
//function : FindFace
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::FindFace(const Standard_Integer idnode1,
                                            const Standard_Integer idnode2,
                                            const Standard_Integer idnode3 ) const
{
  Handle(ActData_Mesh_Face) face = new ActData_Mesh_Triangle(0,idnode1,idnode2,idnode3);
  return FindFace(face);
}

//=======================================================================
//function : FindFace
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::FindFace(const Standard_Integer idnode1,
                                            const Standard_Integer idnode2,
                                            const Standard_Integer idnode3,
                                            const Standard_Integer idnode4 ) const
{
  Handle(ActData_Mesh_Face) face =
    new ActData_Mesh_Quadrangle(0,idnode1,idnode2,idnode3,idnode4);
  return FindFace(face);
}

//=======================================================================
//function : FindElement
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::FindElement
                                        (const Standard_Integer IDelem) const
{
  return myElementIDFactory->MeshElement(IDelem);
}

//=======================================================================
//function : GetNode
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Node) ActData_Mesh::GetNode
                                (const Standard_Integer          rank, 
                                 const Handle(ActData_Mesh_Element)& ME) const

{
  // take care, no control of bounds
  const Standard_Integer idnode = ME->GetConnection(rank);
  return static_cast <Handle(ActData_Mesh_Node)> (FindNode(idnode));
}


//=======================================================================
//function : DumpNodes
//purpose  : 
//=======================================================================

void ActData_Mesh::DumpNodes (Standard_OStream& aStream) const
{
  aStream << "  DUMP NODES OF MESH : " << std::endl;

  ActData_MeshNodesIterator itnode (this);
  for (;itnode.More();itnode.Next()) {
    Handle(ActData_Mesh_Element) node = itnode.Value();
    aStream << node;
  }
}

//=======================================================================
//function : DumpEdges
//purpose  : 
//=======================================================================

void ActData_Mesh::DumpEdges (Standard_OStream& aStream) const
{
  aStream << "  DUMP EDGES OF MESH : " << std::endl;

  ActData_MeshEdgesIterator itedge (this);
  for (;itedge.More();itedge.Next()) {
    Handle(ActData_Mesh_Element) edge = itedge.Value();
    aStream << edge;
  }
}

//=======================================================================
//function : DumpFaces
//purpose  : 
//=======================================================================

void ActData_Mesh::DumpFaces(Standard_OStream& aStream) const
{
  aStream << "  DUMP FACES OF MESH : " << std::endl;

  ActData_MeshFacesIterator itface (this);
  for (;itface.More();itface.Next()) {
    Handle(ActData_Mesh_Element) face = itface.Value();
    aStream << face;
  }
}

//=======================================================================
//function : DebugStats
//purpose  : 
//=======================================================================

void ActData_Mesh::DebugStats(Standard_OStream& aStream) const
{
  aStream << "Debug stats of mesh : " << std::endl;

  aStream << "===== NODES =====" << std::endl;
  myNodes.Statistics(aStream);

  aStream << "===== EDGES =====" << std::endl;
  myEdges.Statistics(aStream);

  aStream << "===== FACES =====" << std::endl;
  myFaces.Statistics(aStream);

  aStream << "End Debug stats of mesh " << std::endl;

  //#ifdef DEB
  ActData_MeshNodesIterator itnode (this);
  Standard_Integer sizeofnodes =
    myNodes.Extent() * (sizeof(ActData_Mesh_Node) + sizeof(Handle(ActData_Mesh_Node)));
  Standard_Integer sizeoffaces = 0;

  for (;itnode.More();itnode.Next()) {
    Handle(ActData_Mesh_Node) aNode = itnode.Value();
    sizeofnodes += aNode->InverseElements().Extent() * 
                   sizeof (ActData_Mesh_MapOfElements::MapNode);
  }

  ActData_MeshFacesIterator itface (this);
  for (;itface.More();itface.Next()) {
    Handle(ActData_Mesh_Element) aFace = itface.Value();
    if (aFace->NbNodes() == 3)
      sizeoffaces += sizeof (ActData_Mesh_Triangle) +
                     sizeof(Handle(ActData_Mesh_Triangle));
    else
      sizeoffaces += sizeof (ActData_Mesh_Quadrangle) +
                     sizeof(Handle(ActData_Mesh_Quadrangle));
  }

  // These totals do not take into account the sizes of normals in
  // every node/face
  aStream << "total size of node elements = " << sizeofnodes << std::endl;;
  aStream << "total size of face elements = " << sizeoffaces << std::endl;;

  //#endif
}

//=======================================================================
//function : RebuildAllInverseConnections
//purpose  : 
//=======================================================================

void ActData_Mesh::RebuildAllInverseConnections()
{
  if (myHasInverse)
    return;

  ActData_Mesh_VectorOfElements::Iterator itelem = myElementIDFactory->Iterator();
  for (; itelem.More(); itelem.Next())
    AddInverseElement(itelem.Value());
  myHasInverse = Standard_True;
}

//=======================================================================
//function : RemoveAllInverseConnections
//purpose  : 
//=======================================================================

void ActData_Mesh::RemoveAllInverseConnections()
{
  // Clear all inverse connections from nodes
  ActData_Mesh_VectorOfElements::Iterator itnode = myNodeIDFactory->Iterator();
  for (;itnode.More();itnode.Next()) {
    Handle(ActData_Mesh_Element) aNode = itnode.Value();
    if (!aNode.IsNull())
      aNode -> ClearInverseElements();
  }

  myHasInverse = Standard_False;
}

//=======================================================================
//function : GetCenterOfGravity
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::GetCenterOfGravity
                                (const Handle(ActData_Mesh_Element)& ME,
                                 gp_XYZ& CenterOfGravity) const
{
  Standard_Integer nbnodes = ME->NbNodes();
  if ( nbnodes == 0 ) return Standard_False;
  CenterOfGravity.SetCoord( 0.0, 0.0, 0.0 );
  if (!ME.IsNull()) { 
     for (Standard_Integer rank=1; rank<=nbnodes; rank++) {
         Handle(ActData_Mesh_Node) aNode = GetNode(rank, ME);
         if(!aNode.IsNull())
             CenterOfGravity += gp_XYZ(aNode->X(), aNode->Y(), aNode->Z());  
         else return Standard_False;
     }
  } else return Standard_False;
  CenterOfGravity = CenterOfGravity/nbnodes;
  return Standard_True;
}  

//=======================================================================
//function : IsSameMeshTree
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh::IsSameMeshTree
                                (const Handle(ActData_Mesh)& theOtherMesh) const
{
  Standard_Boolean aResult = Standard_False;
  if (!theOtherMesh.IsNull())
    aResult = (myNodeIDFactory == theOtherMesh -> myNodeIDFactory);
  return aResult;
}

//=======================================================================
//function : GetElementFromTree
//purpose  : 
//=======================================================================

Handle(ActData_Mesh_Element) ActData_Mesh::GetElementFromTree
                                        (const Standard_Integer anID,
                                         const Standard_Boolean isElement) const
{
  return isElement ?
    myElementIDFactory->MeshElement(anID) :
    myNodeIDFactory->MeshElement(anID);
}
