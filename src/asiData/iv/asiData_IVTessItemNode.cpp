//-----------------------------------------------------------------------------
// Created on: 23 May 2016
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
#include <asiData_IVTessItemNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_IVTessItemNode::asiData_IVTessItemNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,  PID_Name);
  REGISTER_PARAMETER(Mesh,  PID_Mesh);
  REGISTER_PARAMETER(Group, PID_GroupPrs);
  REGISTER_PARAMETER(Int,   PID_Color);
  REGISTER_PARAMETER(Int,   PID_EdgesColor);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_IVTessItemNode::Instance()
{
  return new asiData_IVTessItemNode();
}

//! Performs initial actions required to make Node WELL-FORMED.
void asiData_IVTessItemNode::Init()
{
  this->InitParameter(PID_Name, "Name");

  this->SetMesh       (new ActData_Mesh);
  this->SetColor      (190 << 16 | 190 << 8 | 190); // Initial color.
  this->SetEdgesColor (0   << 16 | 0   << 8 | 0);   // Initial edge color.

  // Initialize Parameter flags.
  this->InitParameter(PID_GroupPrs,   "Presentation",   "",               ParameterFlag_IsVisible, true);
  this->InitParameter(PID_Color,      "Color",          "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter(PID_EdgesColor, "Color of edges", "PrsCustomColor", ParameterFlag_IsVisible, true);
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_IVTessItemNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void asiData_IVTessItemNode::SetName(const TCollection_ExtendedString& theName)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}

//-----------------------------------------------------------------------------
// Handy accessors
//-----------------------------------------------------------------------------

//! \return stored tessellation.
Handle(ActData_Mesh) asiData_IVTessItemNode::GetMesh() const
{
  return ActParamTool::AsMesh( this->Parameter(PID_Mesh) )->GetMesh();
}

//! Sets tessellation to store.
//! \param mesh [in] tessellation to store.
void asiData_IVTessItemNode::SetMesh(const Handle(ActData_Mesh)& mesh)
{
  ActParamTool::AsMesh( this->Parameter(PID_Mesh) )->SetMesh(mesh);
}

//! Sets color.
//! \param color [in] color to set.
void asiData_IVTessItemNode::SetColor(const int color) const
{
  ActParamTool::AsInt( this->Parameter(PID_Color) )->SetValue(color);
}

//! Accessor for the stored color value.
//! \return color value.
int asiData_IVTessItemNode::GetColor() const
{
  return ActParamTool::AsInt( this->Parameter(PID_Color) )->GetValue();
}


//! Sets edges color.
//! \param color [in] color to set.
void asiData_IVTessItemNode::SetEdgesColor(const int color) const
{
  ActParamTool::AsInt(this->Parameter(PID_EdgesColor))->SetValue(color);
}

//! Accessor for the stored edges color value.
//! \return color value.
int asiData_IVTessItemNode::GetEdgesColor() const
{
  return ActParamTool::AsInt(this->Parameter(PID_EdgesColor))->GetValue();
}