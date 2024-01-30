//-----------------------------------------------------------------------------
// Created on: 27 November 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiData_RootNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

asiData_RootNode::asiData_RootNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,  PID_Name);
  REGISTER_PARAMETER(Bool,  PID_IsCoincidentTopo);
  REGISTER_PARAMETER(Group, PID_GroupHlr);
  REGISTER_PARAMETER(Bool,  PID_PrsMeshHlr);
  REGISTER_PARAMETER(Bool,  PID_IsEnabledHiddenInHlr);
  REGISTER_PARAMETER(Int,   PID_HlrTimeout);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_RootNode::Instance()
{
  return new asiData_RootNode();
}

//-----------------------------------------------------------------------------

void asiData_RootNode::Init()
{
  this->InitParameter(PID_Name,                 "Name",                  "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_IsCoincidentTopo,     "Resolve coin. topo",    "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_GroupHlr,             "HLR",                   "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_PrsMeshHlr,           "Render HLR for meshes", "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_IsEnabledHiddenInHlr, "Project hidden edges",  "", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_HlrTimeout,           "Projection timeout",    "", ParameterFlag_IsVisible, true);

  // Set defaults.
  this->SetMeshHlr               (false);
  this->SetResolveCoincidentTopo (false);
  this->SetEnabledHiddenInHlr    (false);
  this->SetHlrTimeout            (500);
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_RootNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_RootNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------

bool asiData_RootNode::IsMeshHlr() const
{
  return ActParamTool::AsBool( this->Parameter(PID_PrsMeshHlr) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_RootNode::SetMeshHlr(const bool isHlr)
{
  ActParamTool::AsBool( this->Parameter(PID_PrsMeshHlr) )->SetValue(isHlr);
}

//-----------------------------------------------------------------------------

void asiData_RootNode::SetResolveCoincidentTopo(const bool on)
{
  ActParamTool::AsBool( this->Parameter(PID_IsCoincidentTopo) )->SetValue(on);
}

//-----------------------------------------------------------------------------

bool asiData_RootNode::IsResolveCoincidentTopo() const
{
  return ActParamTool::AsBool( this->Parameter(PID_IsCoincidentTopo) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_RootNode::SetEnabledHiddenInHlr(const bool on)
{
  ActParamTool::AsBool( this->Parameter(PID_IsEnabledHiddenInHlr) )->SetValue(on);
}

//-----------------------------------------------------------------------------

bool asiData_RootNode::IsEnabledHiddenInHlr() const
{
  return ActParamTool::AsBool( this->Parameter(PID_IsEnabledHiddenInHlr) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_RootNode::SetHlrTimeout(const int timeout)
{
  ActParamTool::AsInt( this->Parameter(PID_HlrTimeout) )->SetValue(timeout);
}

//-----------------------------------------------------------------------------

int asiData_RootNode::GetHlrTimeout() const
{
  return ActParamTool::AsInt( this->Parameter(PID_HlrTimeout) )->GetValue();
}
