//-----------------------------------------------------------------------------
// Created on: 27 May 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Sergey Slyadnev
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
#include <asiData_MetadataNode.h>

// asiData includes
#include <asiData_MetadataParameter.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <TDF_ChildIterator.hxx>
#include <TNaming_NamedShape.hxx>

#ifdef METADATA_DEBUG
#include <TDF_Tool.hxx>
#endif // !METADATA_DEBUG


//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_MetadataNode::asiData_MetadataNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name, PID_Name);

  // Register parameter types specific to Analysis Situs.
  this->registerParameter(PID_Metadata, asiData_MetadataParameter::Instance(), false);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_MetadataNode::Instance()
{
  return new asiData_MetadataNode();
}

//! Performs initial actions required to make this Node WELL-FORMED.
void asiData_MetadataNode::Init()
{
  // Initialize name Parameter
  this->InitParameter(PID_Name, "Name");
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_MetadataNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param[in] name name to set.
void asiData_MetadataNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------

void asiData_MetadataNode::SetColor(const TopoDS_Shape& shape,
                                    const int           icolor)
{
  Handle(asiData_MetadataParameter)
    P = Handle(asiData_MetadataParameter)::DownCast( this->Parameter(PID_Metadata) );

  P->SetColor(shape, icolor);
}

int asiData_MetadataNode::GetColor(const TopoDS_Shape& shape) const
{
  Handle(asiData_MetadataParameter)
    P = Handle(asiData_MetadataParameter)::DownCast( this->Parameter(PID_Metadata) );

  return P->GetColor(shape);
}

void asiData_MetadataNode::GetShapeColorMap(asiData_MetadataAttr::t_shapeColorMap& map) const
{
  Handle(asiData_MetadataParameter)
    P = Handle(asiData_MetadataParameter)::DownCast( this->Parameter(PID_Metadata) );

  P->GetShapeColorMap(map);
}

void asiData_MetadataNode::Clear()
{
  Handle(asiData_MetadataParameter)
    P = Handle(asiData_MetadataParameter)::DownCast( this->Parameter(PID_Metadata) );

  P->SetShapeColorMap( asiData_MetadataAttr::t_shapeColorMap() );
}
