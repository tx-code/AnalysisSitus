//-----------------------------------------------------------------------------
// Created on: 31 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiData_Grid2dNode.h>

// asiData includes
#include <asiData_UniformGridParameter.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

asiData_Grid2dNode::asiData_Grid2dNode() : ActData_BaseNode()
{
  // Register standard Active Data Parameters.
  REGISTER_PARAMETER(Name, PID_Name);

  // Register custom Parameters specific to Analysis Situs.
  this->registerParameter(PID_UniformGrid, asiData_UniformGridParameter::Instance(), false);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_Grid2dNode::Instance()
{
  return new asiData_Grid2dNode();
}

//-----------------------------------------------------------------------------

void asiData_Grid2dNode::Init()
{
  this->InitParameter(PID_Name, "Name", "", ParameterFlag_IsVisible, true);
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_Grid2dNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_Grid2dNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_UniformGrid<float>) asiData_Grid2dNode::GetUniformGrid() const
{
  return Handle(asiData_UniformGridParameter)::DownCast( this->Parameter(PID_UniformGrid) )->GetGrid();
}

//-----------------------------------------------------------------------------

void asiData_Grid2dNode::SetUniformGrid(const Handle(asiAlgo_UniformGrid<float>)& grid)
{
  Handle(asiData_UniformGridParameter)::DownCast( this->Parameter(PID_UniformGrid) )->SetGrid(grid);
}
