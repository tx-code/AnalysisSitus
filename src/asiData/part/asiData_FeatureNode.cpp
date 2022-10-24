//-----------------------------------------------------------------------------
// Created on: 06 December 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <asiData_FeatureNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_FeatureNode::Instance()
{
  return new asiData_FeatureNode();
}

//-----------------------------------------------------------------------------

asiData_FeatureNode::asiData_FeatureNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,        PID_Name);
  REGISTER_PARAMETER(Int,         PID_FeatureId);
  REGISTER_PARAMETER(Selection,   PID_Mask);
  REGISTER_PARAMETER(Int,         PID_Color);
  REGISTER_PARAMETER(AsciiString, PID_Comment);
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::Init()
{
  // Initialize Parameters with default values.
  this->SetFeatureId ( 0 );
  this->SetMask      ( nullptr );
  this->SetColor     ( -1 );
  this->SetComment   ( "" );

  // Initialize properties.
  this->InitParameter (PID_Name, "Name", "", ParameterFlag_IsVisible, true);

}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_FeatureNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::SetName(const TCollection_ExtendedString& theName)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}

//-----------------------------------------------------------------------------

int asiData_FeatureNode::GetFeatureId() const
{
  return ActParamTool::AsInt( this->Parameter(PID_FeatureId) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::SetFeatureId(const int featureId)
{
  ActParamTool::AsInt( this->Parameter(PID_FeatureId) )->SetValue(featureId);
}

//-----------------------------------------------------------------------------

Handle(TColStd_HPackedMapOfInteger) asiData_FeatureNode::GetMask() const
{
  return ActParamTool::AsSelection( this->Parameter(PID_Mask) )->GetMask();
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::GetMask(TColStd_PackedMapOfInteger& mask) const
{
  Handle(TColStd_HPackedMapOfInteger) hmask = this->GetMask();

  if ( !hmask.IsNull() )
    mask = hmask->Map();
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::SetMask(const Handle(TColStd_HPackedMapOfInteger)& mask)
{
  ActParamTool::AsSelection( this->Parameter(PID_Mask) )->SetMask(mask);
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::SetMask(const TColStd_PackedMapOfInteger& mask)
{
  this->SetMask( new TColStd_HPackedMapOfInteger(mask) );
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::SetColor(const int color)
{
  ActParamTool::AsInt( this->Parameter(PID_Color) )->SetValue(color);
}

//-----------------------------------------------------------------------------

int asiData_FeatureNode::GetColor() const
{
  return ActParamTool::AsInt( this->Parameter(PID_Color) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_FeatureNode::SetComment(const TCollection_AsciiString& comment)
{
  ActParamTool::AsAsciiString( this->Parameter(PID_Comment) )->SetValue(comment);
}

//-----------------------------------------------------------------------------

TCollection_AsciiString asiData_FeatureNode::GetComment() const
{
  return ActParamTool::AsAsciiString( this->Parameter(PID_Comment) )->GetValue();
}
