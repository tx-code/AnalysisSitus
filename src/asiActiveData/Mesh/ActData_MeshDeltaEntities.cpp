//-----------------------------------------------------------------------------
// Created on: April 2012
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
#include <ActData_MeshDeltaEntities.h>

// Active Data includes
#include <ActData_Common.h>

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMNode
//-----------------------------------------------------------------------------

//! Adds the entity represented by Modification Delta Object to the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMNode::AddTo(Handle(ActData_Mesh)& theMesh)
{
  theMesh->AddNode(X, Y, Z);
}

//! Removes the entity represented by Modification Delta Object from the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMNode::RemoveFrom(Handle(ActData_Mesh)& theMesh)
{
  theMesh->RemoveNode(ID);
}

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMTriangle
//-----------------------------------------------------------------------------

//! Adds the entity represented by Modification Delta Object to the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMTriangle::AddTo(Handle(ActData_Mesh)& theMesh)
{
  theMesh->AddFace(Nodes, 3);
}

//! Removes the entity represented by Modification Delta Object from the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMTriangle::RemoveFrom(Handle(ActData_Mesh)& theMesh)
{
  Handle(ActData_Mesh_Element) anElem = theMesh->FindElement(ID);
  theMesh->RemoveElement(ID);
}

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMQuadrangle
//-----------------------------------------------------------------------------

//! Adds the entity represented by Modification Delta Object to the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMQuadrangle::AddTo(Handle(ActData_Mesh)& theMesh)
{
  theMesh->AddFace(Nodes, 4);
}

//! Removes the entity represented by Modification Delta Object from the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMQuadrangle::RemoveFrom(Handle(ActData_Mesh)& theMesh)
{
  Handle(ActData_Mesh_Element) anElem = theMesh->FindElement(ID);
  theMesh->RemoveElement(ID);
}

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMMesh
//-----------------------------------------------------------------------------

//! Adds the entity represented by Modification Delta Object to the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMMesh::AddTo(Handle(ActData_Mesh)& theMesh)
{
  theMesh = this->NewMesh;
}

//! Removes the entity represented by Modification Delta Object from the
//! passed Mesh DS.
//! \param theMesh [in] mesh to apply modifications on.
void ActData_DeltaMMesh::RemoveFrom(Handle(ActData_Mesh)& theMesh)
{
  theMesh = this->OldMesh;
}
