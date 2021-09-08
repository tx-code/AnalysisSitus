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

#ifndef ActData_Mesh_ElementsIterator_HeaderFile
#define ActData_Mesh_ElementsIterator_HeaderFile

// Mesh includes
#include <ActData_Mesh_Element.h>

//! The Iterator objet to iterate on all faces of a mesh
class ActData_Mesh_ElementsIterator 
{
public:

  //! Empty constructor
  ActData_Mesh_ElementsIterator();

  //! Constructor
  ActData_EXPORT ActData_Mesh_ElementsIterator(const Handle(ActData_Mesh)& theMesh, const ActData_Mesh_ElementType theType);

  //! Reset the Iterator on the faces of mesh <M>.
  //! This method allows to add another type to iterate,
  //! e.g., Face + Edge. The default value (Node) does not
  //! change the previously installed iterated type
  ActData_EXPORT void Initialize (const Handle(ActData_Mesh)& aMesh, const ActData_Mesh_ElementType anAddType = ActData_Mesh_ET_Node);

  //! Returns True if there is a current mesh element.
  Standard_Boolean More() const;

  //! Moves to the next face.
  void Next();

  //! Query the current value of the iterator
  const Handle(ActData_Mesh_Element)& GetValue() const;

  //! Destructor
  virtual void Destroy();
  virtual ~ActData_Mesh_ElementsIterator() { Destroy(); }

protected:

  
  //! If the current value of myIter is not relevant, perform
  //! internal search of the next relevant value.
  ActData_EXPORT virtual void findNext();

  Standard_Integer myType;
  ActData_Mesh_VectorOfElements::Iterator myIter;
  Handle(ActData_Mesh) myMesh;

};


//=======================================================================
//function : ActData_Mesh_ElementsIterator
//purpose  : Empty constructor
//=======================================================================

inline ActData_Mesh_ElementsIterator::ActData_Mesh_ElementsIterator ()
{}

//=======================================================================
//function : More
//purpose  : 
//=======================================================================

inline Standard_Boolean ActData_Mesh_ElementsIterator::More () const
{
  return myIter.More();
}

//=======================================================================
//function : Next
//purpose  : 
//=======================================================================

inline void ActData_Mesh_ElementsIterator::Next()
{
  myIter.Next();
  findNext();
}

//=======================================================================
//function : GetValue
//purpose  : 
//=======================================================================

inline const Handle(ActData_Mesh_Element)& ActData_Mesh_ElementsIterator::GetValue () const
{
  return myIter.Value();
}

//=======================================================================
//function : Destroy
//purpose  : 
//=======================================================================

inline void ActData_Mesh_ElementsIterator::Destroy ()
{
}

#endif
