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

#ifndef ActData_Mesh_Group_HeaderFile
#define ActData_Mesh_Group_HeaderFile

// Mesh includes
#include <ActData_Mesh_Element.h>

class ActData_Mesh;

DEFINE_STANDARD_HANDLE(ActData_Mesh_Group, ActData_Mesh_Object)

//-----------------------------------------------------------------------------

typedef NCollection_List<Handle(ActData_Mesh_Group)> ActData_Mesh_ListOfGroups;

//-----------------------------------------------------------------------------

class ActData_Mesh_Group : public ActData_Mesh_Object
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_Mesh_Group, ActData_Mesh_Object)

public:

  //! constructor
  ActData_Mesh_Group(const Handle(ActData_Mesh)& aMesh);

  //! create a  sub  group.
  //! uses  a private constructor to create an instance of the
  //! subgroup and attach it the parent group.
  ActData_EXPORT Handle(ActData_Mesh_Group) AddSubGroup();

  //! remove aGroup from the list of Children
  //! if the subgroup does not belong to this, it returns False
  //! (True otherwise)
  ActData_EXPORT virtual Standard_Boolean RemoveSubGroup (const Handle(ActData_Mesh_Group)& aGroup);

  //! remove this from its parent
  //! if this has no parent then it returns False (True otherwise)
  ActData_EXPORT virtual Standard_Boolean RemoveFromParent();

  //! clear the group
  //! once the group is cleared, the type is set to All
  //! but the referenced mesh remains.
  void Clear();

  //! add an element to the group
  ActData_EXPORT void Add (const Handle(ActData_Mesh_Element)& ME);

  //! remove an element from the group
  //! raises if the element is not in the group
  ActData_EXPORT void Remove (const Handle(ActData_Mesh_Element)& ME);

  //! check if the group is empty
  Standard_Boolean IsEmpty() const;

  //! return numner of elements in the group
  Standard_Integer Extent() const;

  //! return current element type
  //! if the group is empty, returns All
  ActData_Mesh_ElementType Type() const;

  //! check if the group contains the mesh element
  Standard_Boolean Contains (const Handle(ActData_Mesh_Element)& ME) const;

  //! check if the group contains the mesh element
  const ActData_Mesh_MapOfElements& Elements() const;

private:

  //! constructor used internally to create subgroup
  ActData_Mesh_Group(const Handle(ActData_Mesh_Group)& parent);

  Handle(ActData_Mesh) myMesh;
  ActData_Mesh_ElementType     myType;
  ActData_Mesh_MapOfElements   myElements;
  Handle(ActData_Mesh_Group)   myParent;
  ActData_Mesh_ListOfGroups    myChildren;

};


//=======================================================================
//function : ActData_Mesh_Group
//purpose  : 
//=======================================================================

inline ActData_Mesh_Group::ActData_Mesh_Group(const Handle(ActData_Mesh)& aMesh)
  : myMesh(aMesh),myType(ActData_Mesh_ET_All)
{
}

//=======================================================================
//function : ActData_Mesh_Group
//purpose  : 
//=======================================================================

inline ActData_Mesh_Group::ActData_Mesh_Group(const Handle(ActData_Mesh_Group)& parent)
  : myMesh(parent->myMesh),myType(ActData_Mesh_ET_All),myParent(parent)
{
}

//=======================================================================
//function : Clear
//purpose  : 
//=======================================================================

inline void ActData_Mesh_Group::Clear()
{
  myElements.Clear();
  myType = ActData_Mesh_ET_All;
}

//=======================================================================
//function : IsEmpty
//purpose  : 
//=======================================================================

inline Standard_Boolean ActData_Mesh_Group::IsEmpty() const
{
  return myElements.IsEmpty();
}

//=======================================================================
//function : Extent
//purpose  : 
//=======================================================================

inline Standard_Integer ActData_Mesh_Group::Extent() const
{
  return myElements.Extent();
}

//=======================================================================
//function : Type
//purpose  : 
//=======================================================================

inline ActData_Mesh_ElementType ActData_Mesh_Group::Type() const
{
  return myType;
}

//=======================================================================
//function : Contains
//purpose  : 
//=======================================================================

inline Standard_Boolean ActData_Mesh_Group::Contains(const Handle(ActData_Mesh_Element)& ME) const
{
  return myElements.Contains(ME);
}

//=======================================================================
//function : Elements
//purpose  : 
//=======================================================================

inline const ActData_Mesh_MapOfElements& ActData_Mesh_Group::Elements() const
{
  return myElements;
}

#endif // _ActData_MeshGroup_HeaderFile
