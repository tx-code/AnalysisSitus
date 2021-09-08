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
#include <ActData_Mesh_IDFactory.h>

// Mesh includes
#include <ActData_Mesh_Element.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>

//-----------------------------------------------------------------------------

ActData_Mesh_IDFactory::ActData_Mesh_IDFactory() : myMaxID(0)
{}

//-----------------------------------------------------------------------------

Standard_Integer ActData_Mesh_IDFactory::GetFreeID()
{
  TColStd_MapIteratorOfPackedMapOfInteger it(myPoolOfID);
  while (it.More()) {
    const Standard_Integer anID = it.Key();
    it.Next();
    myPoolOfID.Remove(anID);
    // We have to check that the ID is really free because it could be
    // already allocated by a direct call Mesh::AddElementWithID
    if (myElements(anID).IsNull())
      return anID;
  }
  return ++myMaxID;
}

//-----------------------------------------------------------------------------

void ActData_Mesh_IDFactory::ReleaseID(const Standard_Integer theID)
{
  myElements.ChangeValue(theID).Nullify();
  if (theID == myMaxID)
    --myMaxID;
  else
    myPoolOfID.Add(theID);
}

//-----------------------------------------------------------------------------

Standard_Boolean ActData_Mesh_IDFactory::BindID
                                (const Standard_Integer          ID,
                                 const Handle(ActData_Mesh_Element)& theElem)
{
  Standard_Boolean aResult (Standard_False);
  if (ID > 0) {
    if (ID < myElements.Length()) {
      Handle(ActData_Mesh_Element)& aTarget = myElements.ChangeValue(ID);
      if (aTarget.IsNull()) {
        aResult = Standard_True;
        aTarget = theElem;
      }
    } else {
      if (ID > myMaxID) {       // if ID has not been obtained via GetFreeID
        //MSV 8.11.06 for OCC13810: too much memory is allocated to
        // store free IDs. We reduce memory usage at the sacrifice 
        // of availability of these free IDs.
        //for (Standard_Integer id = myMaxID+1; id < ID; id++)
        //  myPoolOfID.Add (id);
        myMaxID = ID;
      }
      aResult = Standard_True;
      myElements.SetValue (ID, theElem);
    }
  }
  return aResult;
}

//-----------------------------------------------------------------------------

Handle(ActData_Mesh_Element) ActData_Mesh_IDFactory::MeshElement
                                        (const Standard_Integer ID) const
{
  if (ID > 0 && ID < myElements.Length())
    return myElements.Value (ID);
  return 0L;
}

//-----------------------------------------------------------------------------

NCollection_Vector<Handle(ActData_Mesh_Element)>::Iterator ActData_Mesh_IDFactory::Iterator() const
{
  return NCollection_Vector<Handle(ActData_Mesh_Element)>::Iterator(myElements);
}
