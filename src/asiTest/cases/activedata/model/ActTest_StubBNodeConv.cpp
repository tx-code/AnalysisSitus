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
#include <ActTest_StubBNodeConv.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------
// Implementation of Data Node for testing purposes
//-----------------------------------------------------------------------------

//! Default constructor.
ActTest_StubBNodeConv::ActTest_StubBNodeConv()
{
  REGISTER_PARAMETER(Name,          PID_Name);
  REGISTER_PARAMETER(Int,           PID_Int);
  REGISTER_PARAMETER(Int,           PID_NEWInt1);
  REGISTER_PARAMETER(Real,          PID_Real);
  REGISTER_PARAMETER(TreeFunction,  PID_TFunc);
  REGISTER_PARAMETER(Int,           PID_NEWInt2);
  REGISTER_PARAMETER(ReferenceList, PID_RefList);
}

//! This method will be called by Nodal Factory in order to allocate
//! Nodes automatically by request (e.g. in Model::FindNode routine).
//! \return new instance of our Dummy Node allocated in heap.
Handle(ActAPI_INode) ActTest_StubBNodeConv::Instance()
{
  return new ActTest_StubBNodeConv();
}

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString ActTest_StubBNodeConv::GetName()
{
  return ActData_ParameterFactory::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void ActTest_StubBNodeConv::SetName(const TCollection_ExtendedString& theName)
{
  ActData_ParameterFactory::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}


//! Intializes the basic set of Nodal Parameters.
void ActTest_StubBNodeConv::Init()
{
  ActData_ParameterFactory::AsInt ( this->Parameter(PID_Int) )    ->SetValue(0);
  ActData_ParameterFactory::AsInt ( this->Parameter(PID_NEWInt1) )->SetValue(0);
  ActData_ParameterFactory::AsReal( this->Parameter(PID_Real)    )->SetValue(0.0);
  ActData_ParameterFactory::AsInt ( this->Parameter(PID_NEWInt2) )->SetValue(0);
}
