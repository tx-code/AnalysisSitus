//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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
#include <asiData_DiscrFaceNode.h>

// asiData includes
#include <asiData_DiscrModelParameter.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

asiData_DiscrFaceNode::asiData_DiscrFaceNode() : asiData_FaceNodeBase()
{
  // Register custom Parameters specific to Analysis Situs.
  this->registerParameter(PID_DiscrModel, asiData_DiscrModelParameter::Instance(), false);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_DiscrFaceNode::Instance()
{
  return new asiData_DiscrFaceNode();
}

//-----------------------------------------------------------------------------

void asiData_DiscrFaceNode::Init()
{
  asiData_FaceNodeBase::init();
}

//-----------------------------------------------------------------------------

Handle(asiAlgo::discr::Model) asiData_DiscrFaceNode::GetDiscrModel() const
{
  return Handle(asiData_DiscrModelParameter)::DownCast( this->Parameter(PID_DiscrModel) )->GetDiscrModel();
}

//-----------------------------------------------------------------------------

void asiData_DiscrFaceNode::SetDiscrModel(const Handle(asiAlgo::discr::Model)& model)
{
  Handle(asiData_DiscrModelParameter)::DownCast( this->Parameter(PID_DiscrModel) )->SetDiscrModel(model);
}

