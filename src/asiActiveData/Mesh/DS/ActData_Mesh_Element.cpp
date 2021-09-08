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
#include <ActData_Mesh_Element.h>

// Mesh includes
#include <ActData_Mesh_Direction.h>

// OCCT includes
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_VectorWithNullMagnitude.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_Type.hxx>

//! Constructor for a mesh element with the given ID and number of nodes.
//! \param ID [in] ID of the mesh element.
//! \param nb [in] number of nodes to set.
ActData_Mesh_Element::ActData_Mesh_Element(const int ID, const int nb)
: myID(ID), myNbNodes(nb), myNormals(0L)
{}

//=======================================================================
//function : Destruct
//purpose  : desctructor
//=======================================================================

void ActData_Mesh_Element::Destruct ()
{
  if (myNormals) {
    Standard::Free(myNormals);
    myNormals = 0L;
  }
}

//=======================================================================
//function : HashCode
//purpose  : 
//=======================================================================

int ActData_Mesh_Element::HashCode(const int Upper) const
{
  const int aKey = GetKey();
  return ( (aKey >= 0 ? aKey : -aKey) % Upper );
}

//=======================================================================
//function : GetEdgeDefinedByNodes
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::GetEdgeDefinedByNodes(const Standard_Integer /*rank*/,
                                                     Standard_Integer& idnode1,
                                                     Standard_Integer& idnode2) const
{
  idnode1 = 0;
  idnode2 = 0;
}

//=======================================================================
//function : GetFaceDefinedByNodes
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::GetFaceDefinedByNodes(const Standard_Integer /*rank*/,
                                                       const Standard_Address /*idnode*/,
                                                       Standard_Integer& idface) const
{
  idface = 0;
}

//=======================================================================
//function : GetConnections
//purpose  : 
//=======================================================================

Standard_Address ActData_Mesh_Element::GetConnections() const
{
  return (Standard_Address) &myID;
}

//=======================================================================
//function : GetConnection
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh_Element::GetConnection(const Standard_Integer /*rank*/) const
{
  return myID;
}

//=======================================================================
//function : AddInverseElement
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::AddInverseElement(const Handle(ActData_Mesh_Element)& /*elem*/)
{
}

//=======================================================================
//function : ClearInverseElements
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::ClearInverseElements()
{
}

//=======================================================================
//function : NbNodes
//purpose  : 
//           
//=======================================================================

Standard_Integer ActData_Mesh_Element::NbNodes() const
{
  return myNbNodes;
}

//=======================================================================
//function : NbEdges
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh_Element::NbEdges() const
{
  return 0;
}

//=======================================================================
//function : NbFaces
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh_Element::NbFaces() const
{
  return 0;
}

//=======================================================================
//function : GetID
//purpose  : 
//=======================================================================

Standard_Integer ActData_Mesh_Element::GetID() const
{
  return myID;
}

//=======================================================================
//function : GetMesh
//purpose  : 
//=======================================================================

ActData_Mesh * ActData_Mesh_Element::GetMesh() const
{
  return 0L;
}

//=======================================================================
//function : SetNormal
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::SetNormal(const Standard_Integer rank, const gp_Vec& V)
{
  SetNormal (rank, V.X(), V.Y(), V.Z());
}

//=======================================================================
//function : IsSame
//purpose  : 
//=======================================================================
Standard_Boolean ActData_Mesh_Element::IsSame(const Handle(ActData_Mesh_Element)& other) const
{
  if (this->NbNodes()!=other->NbNodes())
    return Standard_False;
  Standard_Integer n = this->NbNodes();
  const Standard_Integer *c1 = (Standard_Integer *)this->GetConnections();
  const Standard_Integer *c2 = (Standard_Integer *)other->GetConnections();
  if (*c1 != *c2)
    return Standard_False;

  n--;
  c1++;
  c2++;

  Standard_Integer off = n-1;

  for (;n--; c1++,c2++) {
    if (*c1 != *c2 && *c1 != *(c2+off))
      return Standard_False;
    off -= 2;
  }

  return Standard_True;
}

//=======================================================================
//function : IsEqual
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh_Element::IsEqual(const Handle(ActData_Mesh_Element)& other) const
{
  if (this->NbNodes()!=other->NbNodes())
    return Standard_False;
  Standard_Integer n = this->NbNodes();
  const Standard_Integer *c1 = (Standard_Integer *)this->GetConnections();
  const Standard_Integer *c2 = (Standard_Integer *)other->GetConnections();
  if (*c1 != *c2)
    return Standard_False;

  n--;
  c1++;
  c2++;

  for (;n--; c1++,c2++) {
    if (*c1 != *c2)
      return Standard_False;
  }

  return Standard_True;
}

//=======================================================================
//function : IsNodeInElement
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh_Element::IsNodeInElement(const Standard_Integer idnode) const
{
  if (idnode < GetConnection(1))
    return Standard_False;

  const Standard_Integer *c = (Standard_Integer *)this->GetConnections();
  Standard_Integer n = myNbNodes;

  for (;n--; c++) {
    if (*c == idnode)
      return Standard_True;
  }

  return Standard_False;
}

//=======================================================================
//function : InverseElements
//purpose  : 
//=======================================================================

const NCollection_List<Handle(ActData_Mesh_Element)>& ActData_Mesh_Element::InverseElements() const
{
  static const NCollection_List<Handle(ActData_Mesh_Element)> empty;
  return empty;
}

//=======================================================================
//function : SetNormal
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::SetNormal(const Standard_Integer rank,
                             const Standard_Real vx,
                             const Standard_Real vy,
                             const Standard_Real vz)
{
  if (rank > 0 && rank <= NbNodes()) {
    ActData_Mesh_Direction * aNormals = reinterpret_cast<ActData_Mesh_Direction *>(myNormals);
    if (!aNormals) {
      myNormals = Standard::Allocate (NbNodes() * sizeof(ActData_Mesh_Direction));
      aNormals = reinterpret_cast<ActData_Mesh_Direction *> (myNormals);
      for (Standard_Integer i = 0; i < NbNodes(); i++)
        aNormals[i].Clear();
    }
    aNormals[rank-1].Set (vx, vy, vz);
  }
}

//=======================================================================
//function : ClearNormals
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::ClearNormal(const Standard_Integer rank)
{
  if (myNormals) {
    if (rank <= 0)
      Standard::Free (myNormals);
    else if (rank <= NbNodes()) {
      ActData_Mesh_Direction * aNormal =
        (reinterpret_cast<ActData_Mesh_Direction * const>(myNormals)) + rank-1;
      aNormal->Clear();
    }
  }
}

//=======================================================================
//function : IsNormalDefined
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh_Element::IsNormalDefined(const Standard_Integer rank) const
{
  if (myNormals) {
    const ActData_Mesh_Direction * const aNormals =
      reinterpret_cast<const ActData_Mesh_Direction * const> (myNormals);
    // test for 0 vector
    if (rank > 0)
      return (rank <= NbNodes() && !aNormals[rank-1].IsNull());

    for (Standard_Integer i = 0; i < NbNodes(); i++)
      if (!aNormals[i].IsNull())
        return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
//function : GetNormal
//purpose  : 
//=======================================================================

gp_Dir ActData_Mesh_Element::GetNormal(const Standard_Integer rank) const
{
  gp_Dir aResultDir;
  if (myNormals && rank > 0 && rank <= NbNodes()) {
    const ActData_Mesh_Direction * aNormal =
      (reinterpret_cast<const ActData_Mesh_Direction * const>(myNormals)) + rank-1;
    Standard_Boolean ok = aNormal->Get(aResultDir);
    // test for 0 vector
    gp_VectorWithNullMagnitude_Raise_if(!ok,"ActData_Mesh_Element::GetNormal : normal is not defined");
    (void)ok; // avoid compiler warning in Release mode
  }
  return aResultDir;
}

//=======================================================================
//function : GetNDirection
//purpose  : 
//=======================================================================

ActData_Mesh_Direction ActData_Mesh_Element::GetNDirection(const Standard_Integer rank) const
{
  return (!myNormals || rank <= 0 || rank > NbNodes()) ?
    ActData_Mesh_Direction() :
    reinterpret_cast<ActData_Mesh_Direction * const>(myNormals) [rank-1];
}

//=======================================================================
//function : Print
//purpose  : 
//=======================================================================

void ActData_Mesh_Element::Print(Standard_OStream& OS) const
{
  OS << "dump of mesh element" << std::endl;
}

Standard_OStream& operator << (Standard_OStream& OS, const Handle(ActData_Mesh_Element)& ME)
{
  ME->Print(OS);
  return OS;
}
