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

#ifndef ActData_Mesh_IDFactory_HeaderFile
#define ActData_Mesh_IDFactory_HeaderFile

// Mesh includes
#include <ActData_Mesh_Object.h>

// OCCT includes
#include <NCollection_Vector.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

class ActData_Mesh_Element;

DEFINE_STANDARD_HANDLE(ActData_Mesh_IDFactory, ActData_Mesh_Object)

//! \ingroup AD_DF
//!
//! ID factory for mesh objects.
class ActData_Mesh_IDFactory : public ActData_Mesh_Object
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_Mesh_IDFactory, ActData_Mesh_Object)

public:

  ActData_EXPORT ActData_Mesh_IDFactory();

  //! returns a free identifier for mesh from
  //! the pool of ID
  ActData_EXPORT Standard_Integer GetFreeID();

  //! free the ID and give it back to the pool of ID
  ActData_EXPORT void ReleaseID (const Standard_Integer ID);

  //! bind the ID with the mesh element
  //! returns False if the ID is already bound.
  //! In this case the element is not replaced
  ActData_EXPORT Standard_Boolean BindID (const Standard_Integer ID, const Handle(ActData_Mesh_Element)& elem);

  //! returns the MeshElement associated with ID
  //! Returns NULL handle the ID is not bound
  ActData_EXPORT Handle(ActData_Mesh_Element) MeshElement (const Standard_Integer ID) const;

  //! returns the iterator of the whole collection
  //! of mesh entities
  ActData_EXPORT NCollection_Vector<Handle(ActData_Mesh_Element)>::Iterator Iterator() const;

private:

  NCollection_Vector<Handle(ActData_Mesh_Element)> myElements;
  TColStd_PackedMapOfInteger               myPoolOfID;
  Standard_Integer                         myMaxID;

};

#endif
