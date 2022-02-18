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
#include <ActTest_StubBNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------
// Implementation of Data Node for testing purposes
//-----------------------------------------------------------------------------

//! Default constructor.
ActTest_StubBNode::ActTest_StubBNode()
{
  REGISTER_PARAMETER(Name,          PID_Name);
  REGISTER_PARAMETER(Int,           PID_Int);
  REGISTER_PARAMETER(Real,          PID_Real);
  REGISTER_PARAMETER(TreeFunction,  PID_TFunc);
  REGISTER_PARAMETER(Reference,     PID_Ref);
  REGISTER_PARAMETER(ReferenceList, PID_RefList);
}

//! This method will be called by Nodal Factory in order to allocate
//! Nodes automatically by request (e.g. in Model::FindNode routine).
//! \return new instance of our Dummy Node allocated in heap.
Handle(ActAPI_INode) ActTest_StubBNode::Instance()
{
  return new ActTest_StubBNode();
}

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString ActTest_StubBNode::GetName()
{
  return ActData_ParameterFactory::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void ActTest_StubBNode::SetName(const TCollection_ExtendedString& theName)
{
  ActData_ParameterFactory::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}

//! Initializes the basic set of Nodal Parameters.
//! \param theIntVal [in] integer value to set.
//! \param theRealVal [in] real value to set.
void ActTest_StubBNode::Init(const Standard_Integer theIntVal,
                             const Standard_Real    theRealVal)
{
  ActData_ParameterFactory::AsInt ( this->Parameter(PID_Int) )->SetValue(theIntVal);
  ActData_ParameterFactory::AsReal( this->Parameter(PID_Real) )->SetValue(theRealVal);
}

//! Accessor for the integer value.
//! \return requested value.
Standard_Integer ActTest_StubBNode::GetIntValue() const
{
  return ActData_ParameterFactory::AsInt( this->Parameter(PID_Int) )->GetValue();
}

//! Accessor for the real value.
//! \return requested value.
Standard_Real ActTest_StubBNode::GetRealValue() const
{
  return ActData_ParameterFactory::AsReal( this->Parameter(PID_Real) )->GetValue();
}
