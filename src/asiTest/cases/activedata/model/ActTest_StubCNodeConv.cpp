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
#include <ActTest_StubCNodeConv.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// Active Data (auxiliary) includes
#include <ActAux_TimeStamp.h>

//-----------------------------------------------------------------------------
// Implementation of Data Node for testing purposes
//-----------------------------------------------------------------------------

//! Default constructor.
ActTest_StubCNodeConv::ActTest_StubCNodeConv()
{
  REGISTER_PARAMETER(Int,           PID_Int);
  REGISTER_PARAMETER(Bool,          PID_Bool);
  REGISTER_PARAMETER(Int,           PID_NEWInt1);
  REGISTER_PARAMETER(Shape,         PID_Shape);
  REGISTER_PARAMETER(RealArray,     PID_RealArray);
  REGISTER_PARAMETER(TreeFunction,  PID_TFunc);
  REGISTER_PARAMETER(AsciiString,   PID_AStr);
  REGISTER_PARAMETER(BoolArray,     PID_BoolArray);
  REGISTER_PARAMETER(StringArray,   PID_StrArray);
  REGISTER_PARAMETER(ComplexArray,  PID_ComplexArray);
  REGISTER_PARAMETER(IntArray,      PID_IntArray);
  REGISTER_PARAMETER(Int,           PID_NEWInt2);
  REGISTER_PARAMETER(ReferenceList, PID_RefList);
  REGISTER_PARAMETER(Group,         PID_Group);
  REGISTER_PARAMETER(Mesh,          PID_Mesh);
  REGISTER_PARAMETER(Int,           PID_NEWInt3);
  REGISTER_PARAMETER(Reference,     PID_Ref);
  REGISTER_PARAMETER(Selection,     PID_Selection);
  REGISTER_PARAMETER(TimeStamp,     PID_TimeStamp);
  REGISTER_PARAMETER(Int,           PID_NEWInt4);
}

//! This method will be called by Nodal Factory in order to allocate
//! Nodes automatically by request (e.g. in Model::FindNode routine).
//! \return new instance of our Dummy Node allocated in heap.
Handle(ActAPI_INode) ActTest_StubCNodeConv::Instance()
{
  return new ActTest_StubCNodeConv();
}

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString ActTest_StubCNodeConv::GetName()
{
  return TCollection_ExtendedString();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void ActTest_StubCNodeConv::SetName(const TCollection_ExtendedString& ActData_NotUsed(theName))
{
  // Do nothing...
}

//! Initializes Nodal Parameters with default values.
void ActTest_StubCNodeConv::Init()
{
  ActParamTool::AsInt          ( this->Parameter(PID_Int) )         ->SetValue ( 0 );
  ActParamTool::AsBool         ( this->Parameter(PID_Bool) )        ->SetValue ( Standard_False );
  ActParamTool::AsShape        ( this->Parameter(PID_Shape) )       ->SetShape ( TopoDS_Shape() );
  ActParamTool::AsInt          ( this->Parameter(PID_NEWInt1) )     ->SetValue ( 0 );
  ActParamTool::AsRealArray    ( this->Parameter(PID_RealArray) )   ->SetArray ( NULL );
  ActParamTool::AsAsciiString  ( this->Parameter(PID_AStr) )        ->SetValue ( TCollection_AsciiString() );
  ActParamTool::AsBoolArray    ( this->Parameter(PID_BoolArray) )   ->SetArray ( NULL );
  ActParamTool::AsStringArray  ( this->Parameter(PID_StrArray) )    ->SetArray ( NULL );
  ActParamTool::AsComplexArray ( this->Parameter(PID_ComplexArray) )->SetArray ( NULL );
  ActParamTool::AsIntArray     ( this->Parameter(PID_IntArray) )    ->SetArray ( NULL );
  ActParamTool::AsInt          ( this->Parameter(PID_NEWInt2) )     ->SetValue ( 0 );
  ActParamTool::AsMesh         ( this->Parameter(PID_Mesh) )        ->SetMesh  ( new ActData_Mesh );
  ActParamTool::AsInt          ( this->Parameter(PID_NEWInt3) )     ->SetValue ( 0 );
  ActParamTool::AsSelection    ( this->Parameter(PID_Selection) )   ->SetMask  ( new TColStd_HPackedMapOfInteger );
  ActParamTool::AsTimeStamp    ( this->Parameter(PID_TimeStamp) )   ->SetValue ( new ActAux_TimeStamp );
  ActParamTool::AsInt          ( this->Parameter(PID_NEWInt4) )     ->SetValue ( 0 );
}
