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

#ifndef ActData_MeshAttr_HeaderFile
#define ActData_MeshAttr_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_MeshMDelta.h>

// OCCT includes
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>

// Mesh includes
#include <ActData_Mesh.h>

DEFINE_STANDARD_HANDLE(ActData_MeshAttr, TDF_Attribute)

//! \ingroup AD_DF
//!
//! OCAF Attribute representing mesh data.
class ActData_MeshAttr : public TDF_Attribute
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MeshAttr, TDF_Attribute)

// Construction & settling-down routines:
public:

  ActData_EXPORT ActData_MeshAttr();

  ActData_EXPORT static Handle(ActData_MeshAttr)
    Set(const TDF_Label& Label);

// GUID accessors:
public:

  ActData_EXPORT static const Standard_GUID&
    GUID();

  ActData_EXPORT virtual const Standard_GUID&
    ID() const;

// Attribute's kernel methods:
public:

  ActData_EXPORT virtual Handle(TDF_Attribute)
    NewEmpty() const;

  ActData_EXPORT virtual void
    Restore(const Handle(TDF_Attribute)& MainAttr);

  ActData_EXPORT virtual Standard_Boolean
    BeforeUndo(const Handle(TDF_AttributeDelta)& Delta,
               const Standard_Boolean doForce = Standard_False);

  ActData_EXPORT virtual Standard_Boolean
    AfterUndo(const Handle(TDF_AttributeDelta)& Delta,
              const Standard_Boolean doForce = Standard_False);

  ActData_EXPORT virtual void
    BeforeCommitTransaction();

  ActData_EXPORT virtual void
    Paste(const Handle(TDF_Attribute)& Into,
          const Handle(TDF_RelocationTable)& RelocTable) const;

  ActData_EXPORT virtual Handle(TDF_DeltaOnModification)
    DeltaOnModification(const Handle(TDF_Attribute)& BackUp) const;

  ActData_EXPORT virtual Handle(TDF_DeltaOnAddition)
    DeltaOnAddition() const;

  ActData_EXPORT void
    DeltaModeOn();

  ActData_EXPORT void
    DeltaModeOff();

// Accessors for domain-specific data:
public:

  ActData_EXPORT void
    NewEmptyMesh();

  ActData_EXPORT void
    SetMesh(const Handle(ActData_Mesh)& Mesh,
            const Standard_Boolean doDelta = Standard_True);

  ActData_EXPORT Handle(ActData_Mesh)&
    GetMesh();

// Manipulations with mesh:
public:

  ActData_EXPORT Standard_Integer
    AddNode(const Standard_Real X, const Standard_Real Y, const Standard_Real Z);

  ActData_EXPORT Standard_Boolean
    AddNodeWithID(const Standard_Real X,
                  const Standard_Real Y,
                  const Standard_Real Z,
                  const Standard_Integer ID);

  ActData_EXPORT Standard_Boolean
    RemoveNode(const Standard_Integer ID);

  ActData_EXPORT Standard_Integer
    AddElement(Standard_Address Nodes, const Standard_Integer NbNodes);

  ActData_EXPORT Standard_Boolean
    AddElementWithID(Standard_Address Nodes,
                     const Standard_Integer NbNodes,
                     const Standard_Integer ID);

  ActData_EXPORT Standard_Boolean
    RemoveElement(const Standard_Integer ID);

// Internal kernel methods:
private:

  void assertModificationAllowed();

// Internal members:
private:

  //! Stored Mesh DS.
  Handle(ActData_Mesh) m_mesh;

  //! Transient Modification Delta being passed from the beginning
  //! of each transaction (it is empty in the beginning) till its
  //! end (it is populated with Modification Requests at the end).
  Handle(ActData_MeshMDelta) m_delta;

  //! Indicates whether DELTALIZATION is enabled or not.
  Standard_Boolean m_bDeltaEnabled;

};

#endif
