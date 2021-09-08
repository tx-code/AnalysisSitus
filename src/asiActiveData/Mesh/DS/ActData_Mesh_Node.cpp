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
#include <ActData_Mesh_Node.h>

// Mesh includes
#include <ActData_Mesh_Position.h>
#include <ActData_Mesh_SpacePosition.h>

// OCCT includes
#include <gp_Pnt.hxx>

//-----------------------------------------------------------------------------

static const Handle(ActData_Mesh_Position)& StaticInstancePosition()
{
  static Handle(ActData_Mesh_SpacePosition) staticpos;
  if (staticpos.IsNull())
    staticpos = new ActData_Mesh_SpacePosition();
  return staticpos;
}

//-----------------------------------------------------------------------------

ActData_Mesh_Node::ActData_Mesh_Node(const Standard_Integer ID,
                     const Standard_Real    x,
                     const Standard_Real    y,
                     const Standard_Real    z)
: ActData_Mesh_Element ( ID, 1 ),
  myPnt        ( x, y, z),
  myPosition   ( StaticInstancePosition() )
{
}

//-----------------------------------------------------------------------------

void ActData_Mesh_Node::SetPnt (const gp_Pnt& P, const Standard_Boolean isClearNormals)
{
  myPnt = P;
  if (isClearNormals) {
    ClearNormal();
    ActData_Mesh_ListOfElements::Iterator anIter (InverseElements());
    for (; anIter.More(); anIter.Next())
      if (!anIter.Value().IsNull())
        anIter.Value()->ClearNormal();
  }
}

//-----------------------------------------------------------------------------

void ActData_Mesh_Node::RemoveInverseElement(const Handle(ActData_Mesh_Element)& parent)
{
  // create copy of handle to avoid entity nullifying till the end of cycle
  Handle(ActData_Mesh_Element) elem = parent;
  ActData_Mesh_ListOfElements::Iterator itLstInvCnx(myInverseElements);
  while (itLstInvCnx.More()) {
    Handle(ActData_Mesh_Element)& ME = itLstInvCnx.Value();
    //the same elements can belong to the different meshes in one mesh tree
    //if (ME->IsSame(elem))
    if (ME == elem)
      myInverseElements.Remove(itLstInvCnx);
    else
      itLstInvCnx.Next();
  }
}

//-----------------------------------------------------------------------------

void ActData_Mesh_Node::Print(Standard_OStream& OS) const
{
  OS << "Node < " << myID << " > : X = " << myPnt.X() << " Y = " << myPnt.Y() << " Z = " << myPnt.Z() << std::endl;
}
