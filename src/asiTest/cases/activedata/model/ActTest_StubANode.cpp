//-----------------------------------------------------------------------------
// Created on: 17 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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
//    * Neither the name of the copyright holder(s) nor the
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
//-----------------------------------------------------------------------------

// Own include
#include <ActTest_StubANode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------
// Implementation of Data Node for testing purposes
//-----------------------------------------------------------------------------

//! Default constructor.
ActTest_StubANode::ActTest_StubANode()
{
  REGISTER_PARAMETER     (Name,         PID_Name);
  REGISTER_PARAMETER     (Shape,        PID_DummyShapeA);
  REGISTER_PARAMETER     (Shape,        PID_DummyShapeB);
  REGISTER_PARAMETER_EXPR(Real,         PID_Real);
  REGISTER_PARAMETER     (TreeFunction, PID_TFunc);
  REGISTER_PARAMETER     (Reference,    PID_Ref);
}

//! This method will be called by Nodal Factory in order to allocate
//! Nodes automatically by request (e.g. in Model::FindNode routine).
//! \return new instance of our Dummy Node allocated in heap.
Handle(ActAPI_INode) ActTest_StubANode::Instance()
{
  return new ActTest_StubANode();
}

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString ActTest_StubANode::GetName()
{
  return ActData_ParameterFactory::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void ActTest_StubANode::SetName(const TCollection_ExtendedString& theName)
{
  ActData_ParameterFactory::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}

//! Initializes the basic set of Nodal Parameters.
//! \param theShapeA [in] first shape to set.
//! \param theShapeB [in] second shape to set.
//! \param theVal    [in] value to set.
void ActTest_StubANode::Init(const TopoDS_Shape& theShapeA,
                             const TopoDS_Shape& theShapeB,
                             const Standard_Real theVal)
{
  ActData_ParameterFactory::AsShape( this->Parameter(PID_DummyShapeA) )->SetShape(theShapeA);
  ActData_ParameterFactory::AsShape( this->Parameter(PID_DummyShapeB) )->SetShape(theShapeB);
  ActData_ParameterFactory::AsReal ( this->Parameter(PID_Real)        )->SetValue(theVal);
}

//! Accessor for the first shape.
//! \return requested shape.
TopoDS_Shape ActTest_StubANode::GetShapeA() const
{
  return ActData_ParameterFactory::AsShape( this->Parameter(PID_DummyShapeA) )->GetShape();
}

//! Accessor for the second shape.
//! \return requested shape.
TopoDS_Shape ActTest_StubANode::GetShapeB() const
{
  return ActData_ParameterFactory::AsShape( this->Parameter(PID_DummyShapeB) )->GetShape();
}

//! Accessor for the value.
//! \return requested value.
Standard_Real ActTest_StubANode::GetValue() const
{
  return ActData_ParameterFactory::AsReal( this->Parameter(PID_Real) )->GetValue();
}

//! Returns true if this Data Node has connected Tree Function
//! Parameter, false -- otherwise.
//! \return true/false.
Standard_Boolean ActTest_StubANode::HasConnectedFunction() const
{
  return ActData_ParameterFactory::AsTreeFunction( this->Parameter(PID_TFunc) )->IsConnected();
}
