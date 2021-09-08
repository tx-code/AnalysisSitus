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
#include <ActData_Mesh_Group.h>

// OCCT includes
#include <Standard_TypeMismatch.hxx>

//=======================================================================
//function : AddSubGroup
//purpose  : 
//=======================================================================
Handle(ActData_Mesh_Group) ActData_Mesh_Group::AddSubGroup()
{
  Handle(ActData_Mesh_Group) subgroup = new ActData_Mesh_Group(this);
  if (!subgroup.IsNull())
    myChildren.Append(subgroup);
  return subgroup;
}

//=======================================================================
//function : RemoveSubGroup
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh_Group::RemoveSubGroup(const Handle(ActData_Mesh_Group)& aGroup)
{
  ActData_Mesh_ListOfGroups::Iterator itgroup(myChildren);
  for (; itgroup.More(); itgroup.Next()) {
    Handle(ActData_Mesh_Group) subgroup = itgroup.Value();
    if (subgroup == aGroup) {
      myChildren.Remove(itgroup);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : RemoveFromParent
//purpose  : 
//=======================================================================

Standard_Boolean ActData_Mesh_Group::RemoveFromParent()
{
  if (myParent.IsNull())
    return Standard_False;

  return (myParent->RemoveSubGroup(this));
}

//=======================================================================
//function : Add
//purpose  : 
//=======================================================================

void ActData_Mesh_Group::Add(const Handle(ActData_Mesh_Element)& ME)
{
  // the type of the group is determined by the first element added
  if (myElements.IsEmpty())
    myType = ME->GetType();
  else if (ME->GetType() != myType)
    Standard_TypeMismatch::Raise("ActData_Mesh_Group::Add");
  myElements.Add(ME);
}

//=======================================================================
//function : Remove
//purpose  : 
//=======================================================================

void ActData_Mesh_Group::Remove(const Handle(ActData_Mesh_Element)& ME)
{
  myElements.Remove(ME);
  if (myElements.IsEmpty())
    myType = ActData_Mesh_ET_All;
}
