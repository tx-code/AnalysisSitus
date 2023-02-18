//-----------------------------------------------------------------------------
// Created on: 02 December 2015
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
#include <asiData_FaceNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

asiData_FaceNode::asiData_FaceNode() : asiData_FaceNodeBase()
{
  REGISTER_PARAMETER(Bool, PID_ShowOriTips);
  REGISTER_PARAMETER(Real, PID_UScaleCoeff);
  REGISTER_PARAMETER(Real, PID_VScaleCoeff);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_FaceNode::Instance()
{
  return new asiData_FaceNode();
}

//-----------------------------------------------------------------------------

void asiData_FaceNode::Init(const bool resetScaling)
{
  asiData_FaceNodeBase::init();

  this->SetShowOriTips(true);

  if ( resetScaling )
  {
    this->SetUScaleCoeff(1.);
    this->SetVScaleCoeff(1.);
  }

  this->InitParameter(PID_ShowOriTips, "Show tips", "", ParameterFlag_IsVisible, true);
}

//-----------------------------------------------------------------------------

void asiData_FaceNode::SetShowOriTips(const bool on)
{
  ActParamTool::AsBool( this->Parameter(PID_ShowOriTips) )->SetValue(on);
}

//-----------------------------------------------------------------------------

bool asiData_FaceNode::GetShowOriTips() const
{
  return ActParamTool::AsBool( this->Parameter(PID_ShowOriTips) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_FaceNode::SetUScaleCoeff(const double coeff)
{
  ActParamTool::AsReal( this->Parameter(PID_UScaleCoeff) )->SetValue(coeff);
}

//-----------------------------------------------------------------------------

double asiData_FaceNode::GetUScaleCoeff() const
{
  return ActParamTool::AsReal( this->Parameter(PID_UScaleCoeff) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_FaceNode::SetVScaleCoeff(const double coeff)
{
  ActParamTool::AsReal( this->Parameter(PID_VScaleCoeff) )->SetValue(coeff);
}

//-----------------------------------------------------------------------------

double asiData_FaceNode::GetVScaleCoeff() const
{
  return ActParamTool::AsReal( this->Parameter(PID_VScaleCoeff) )->GetValue();
}
