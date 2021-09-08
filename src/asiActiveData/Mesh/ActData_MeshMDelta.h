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

#ifndef ActData_MeshMDelta_HeaderFile
#define ActData_MeshMDelta_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_MeshDeltaEntities.h>

// OCCT includes
#include <Standard_OStream.hxx>
#include <TDF_DeltaOnModification.hxx>

// Active Data forward declarations
class ActData_MeshAttr;

DEFINE_STANDARD_HANDLE(ActData_MeshMDelta, TDF_DeltaOnModification)

//! \ingroup AD_DF
//!
//! Modification Delta for Mesh Attribute. Each Modification Delta manages
//! so called Modification Queue represented internally by simple OCCT
//! sequence. Modification Queue in-turn stores Modification Requests, all
//! having dual nature. E.g. Addition Request is dual for Removal Request
//! and vice versa.
//!
//! Conceptually Modification Delta represents an atomic portion of changes
//! which are to be applied on the actual Mesh DS currently stored in the
//! corresponding TDF Attribute. It can be INVERTED in order to exchange
//! the actual Modification Queue by its dual representation.
class ActData_MeshMDelta : public TDF_DeltaOnModification
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MeshMDelta, TDF_DeltaOnModification)

// Construction:
public:

  ActData_EXPORT
    ActData_MeshMDelta(const Handle(ActData_MeshAttr)& ActualAttr);

// Kernel routines:
public:

  ActData_EXPORT virtual void
    Apply();

  ActData_EXPORT void
    Clean();

  ActData_EXPORT Handle(ActData_MeshMDelta)
    DeepCopy() const;

  ActData_EXPORT void
    Invert();

// Modification requests:
public:

  ActData_EXPORT void
    ReplacedMesh(const Handle(ActData_Mesh)& OldMesh,
                 const Handle(ActData_Mesh)& NewMesh);

  ActData_EXPORT void
    AddedNode(const Standard_Integer ID,
              const Standard_Real X,
              const Standard_Real Y,
              const Standard_Real Z);

  ActData_EXPORT void
    AddedTriangle(const Standard_Integer ID,
                  Standard_Address Nodes);

  ActData_EXPORT void
    AddedQuadrangle(const Standard_Integer ID,
                    Standard_Address Nodes);

  ActData_EXPORT void
    RemovedNode(const Standard_Integer ID);

  ActData_EXPORT void
    RemovedTriangle(const Standard_Integer ID);

  ActData_EXPORT void
    RemovedQuadrangle(const Standard_Integer ID);

// Debugging:
public:

  ActData_EXPORT virtual Standard_OStream&
    Dump(Standard_OStream& theOut) const;

private:

  //! Ordered collection of Modification Requests.
  ActData_DeltaMQueue m_queue;

};

#endif
