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

#ifndef ActData_MeshDeltaEntities_HeaderFile
#define ActData_MeshDeltaEntities_HeaderFile

// OCCT includes
#include <NCollection_Handle.hxx>
#include <NCollection_Sequence.hxx>
#include <Standard_Type.hxx>

// Mesh includes
#include <ActData_Mesh.h>

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMObject
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_DeltaMObject, Standard_Transient)

//! \ingroup AD_DF
//!
//! Modification entity.
class ActData_DeltaMObject : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_DeltaMObject, Standard_Transient)

public:

  Standard_Integer ID; //!< ID of the object in the mesh structure.

  //! Default constructor.
  ActData_DeltaMObject() : ID(-1) {}

  //! Adds Delta entity the passed Mesh DS.
  //! \param theMesh [in] Mesh DS to add the entity into.
  virtual void AddTo(Handle(ActData_Mesh)& theMesh) = 0;

  //! Removes Delta entity from the passed Mesh DS.
  //! \param theMesh [in] Mesh DS to remove the entity from.
  virtual void RemoveFrom(Handle(ActData_Mesh)& theMesh) = 0;
};

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMNode
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_DeltaMNode, ActData_DeltaMObject)

//! \ingroup AD_DF
//!
//! Node playing as a modification entity.
class ActData_DeltaMNode : public ActData_DeltaMObject
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_DeltaMNode, ActData_DeltaMObject)

public:

  Standard_Real X; //!< X coordinate.
  Standard_Real Y; //!< Y coordinate.
  Standard_Real Z; //!< Z coordinate.
  
  //! Default constructor.
  ActData_DeltaMNode() : ActData_DeltaMObject(), X(0.0), Y(0.0), Z(0.0) {}

  virtual void AddTo(Handle(ActData_Mesh)& theMesh);
  virtual void RemoveFrom(Handle(ActData_Mesh)& theMesh);
};

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMTriangle
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_DeltaMTriangle, ActData_DeltaMObject)

//! \ingroup AD_DF
//!
//! Triangle face playing as a modification entity.
class ActData_DeltaMTriangle : public ActData_DeltaMObject
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_DeltaMTriangle, ActData_DeltaMObject)

public:

  Standard_Integer Nodes[3]; //!< Triangle nodes.

  //! Default constructor.
  ActData_DeltaMTriangle() : ActData_DeltaMObject()
  {
    Nodes[0] = Nodes[1] = Nodes[2] = 0;
  }

  virtual void AddTo(Handle(ActData_Mesh)& theMesh);
  virtual void RemoveFrom(Handle(ActData_Mesh)& theMesh);
};

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMQuadrangle
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_DeltaMQuadrangle, ActData_DeltaMObject)

//! \ingroup AD_DF
//!
//! Quadrangle face playing as a modification entity.
class ActData_DeltaMQuadrangle : public ActData_DeltaMObject
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_DeltaMQuadrangle, ActData_DeltaMObject)

public:

  Standard_Integer Nodes[4]; //!< Quadrangle nodes

  //! Default constructor.
  ActData_DeltaMQuadrangle() : ActData_DeltaMObject()
  {
    Nodes[0] = Nodes[1] = Nodes[2] = Nodes[3] = 0;
  }

  virtual void AddTo(Handle(ActData_Mesh)& theMesh);
  virtual void RemoveFrom(Handle(ActData_Mesh)& theMesh);
};

//-----------------------------------------------------------------------------
// Class: ActData_DeltaMMesh
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_DeltaMMesh, ActData_DeltaMObject)

//! \ingroup AD_DF
//!
//! Entire mesh playing as a modification entity.
class ActData_DeltaMMesh : public ActData_DeltaMObject
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_DeltaMMesh, ActData_DeltaMObject)

public:

  Handle(ActData_Mesh) OldMesh; //!< Old Mesh DS.
  Handle(ActData_Mesh) NewMesh; //!< New Mesh DS.

  //! Default constructor.
  ActData_DeltaMMesh() : ActData_DeltaMObject() {}

  virtual void AddTo(Handle(ActData_Mesh)& theMesh);
  virtual void RemoveFrom(Handle(ActData_Mesh)& theMesh);
};

//-----------------------------------------------------------------------------
// Data types used to form a queue of modification requests
//-----------------------------------------------------------------------------

//! \ingroup AD_DF
//!
//! Modification types.
enum ActData_DeltaMType
{
  DeltaMType_Undef = 1, //!< Undefined type.
  DeltaMType_Added,     //!< Something has been added.
  DeltaMType_Removed    //!< Something has been removed.
};

//! \ingroup AD_DF
//!
//! Structure representing a single modification request.
struct ActData_DeltaMRequest
{
  ActData_DeltaMType Type;             //!< Modification type.
  Handle(ActData_DeltaMObject) Entity; //!< Modified entity.

  //! Default constructor.
  ActData_DeltaMRequest() : Type(DeltaMType_Undef), Entity(NULL) {}

  //! Complete constructor.
  //! \param theType [in] modification type.
  //! \param theEntity [in] modification entity.
  ActData_DeltaMRequest(const ActData_DeltaMType theType,
                        const Handle(ActData_DeltaMObject)& theEntity)
  : Type(theType), Entity(theEntity)
  {}

  //! Assignment constructor.
  //! \param theRequest [in] other request.
  ActData_DeltaMRequest(const ActData_DeltaMRequest& theRequest)
  {
    this->operator=(theRequest);
  }

  //! Assignment operator.
  //! \param theRequest [in] other Modification Request object.
  //! \return this one.
  ActData_DeltaMRequest& operator=(const ActData_DeltaMRequest& theRequest)
  {
    Type = theRequest.Type;
    Entity = theRequest.Entity;
    return *this;
  }
};

//! \ingroup AD_DF
//!
//! Ordered collection of modification requests. Request recorded first must
//! be applied first. Thus we can record and replay the sequence of user
//! actions in their actual order.
typedef NCollection_Sequence<ActData_DeltaMRequest> ActData_DeltaMQueue;

#endif
