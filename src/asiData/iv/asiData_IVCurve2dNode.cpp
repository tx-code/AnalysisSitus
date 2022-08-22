//-----------------------------------------------------------------------------
// Created on: 11 December(*) 2017
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
#include <asiData_IVCurve2dNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

//-----------------------------------------------------------------------------

asiData_IVCurve2dNode::asiData_IVCurve2dNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,  PID_Name);
  REGISTER_PARAMETER(Shape, PID_Curve);
  REGISTER_PARAMETER(Shape, PID_Surface);
  REGISTER_PARAMETER(Group, PID_GroupPrs);
  REGISTER_PARAMETER(Bool,  PID_HasColor);
  REGISTER_PARAMETER(Int,   PID_Color);
  REGISTER_PARAMETER(Bool,  PID_DrawOriTip);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_IVCurve2dNode::Instance()
{
  return new asiData_IVCurve2dNode();
}

//-----------------------------------------------------------------------------

void asiData_IVCurve2dNode::Init()
{
  // Initialize name Parameter
  this->InitParameter (PID_Name,       "Name",            "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_GroupPrs,   "Presentation",    "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_HasColor,   "Colorized",       "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_Color,      "Color",           "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_DrawOriTip, "Orientation tip", "",               ParameterFlag_IsVisible, true);
  //
  this->SetCONS(nullptr, nullptr, 0.0, 0.0);
  this->SetDrawOrientationTip(true);
  this->SetHasColor(false);
  this->SetColor(16777215);
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_IVCurve2dNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVCurve2dNode::SetName(const TCollection_ExtendedString& theName)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(theName);
}

//-----------------------------------------------------------------------------
// Handy accessors
//-----------------------------------------------------------------------------

Handle(Geom2d_Curve)
  asiData_IVCurve2dNode::GetCONS(Handle(Geom_Surface)& surface,
                                 double&               f,
                                 double&               l) const
{
  // Get stored curve
  TopoDS_Shape curveSh = ActParamTool::AsShape( this->Parameter(PID_Curve) )->GetShape();
  //
  if ( curveSh.IsNull() || curveSh.ShapeType() != TopAbs_EDGE )
    return nullptr;

  // Get stored host surface
  TopoDS_Shape surfSh = ActParamTool::AsShape( this->Parameter(PID_Surface) )->GetShape();
  //
  if ( surfSh.IsNull() || surfSh.ShapeType() != TopAbs_FACE )
    return nullptr;

  // Extract host surface
  const TopoDS_Face& F = TopoDS::Face(surfSh);

  // Extract edge and its host geometry
  const TopoDS_Edge& E = TopoDS::Edge(curveSh);

  // Get host surface
  surface = BRep_Tool::Surface(F);

  return BRep_Tool::CurveOnSurface(E, F, f, l);
}

//-----------------------------------------------------------------------------

void asiData_IVCurve2dNode::SetCONS(const Handle(Geom2d_Curve)& curve,
                                    const Handle(Geom_Surface)& surface,
                                    const double                f,
                                    const double                l)
{
  if ( !curve.IsNull() && !surface.IsNull() )
  {
    // Create a fictive face
    TopoDS_Face F = BRepBuilderAPI_MakeFace( surface, Precision::Confusion() );

    // Create a fictive edge to take advantage of topology Parameter of Active Data
    TopoDS_Edge E = BRepBuilderAPI_MakeEdge(curve, surface, f, l);

    // Store
    ActParamTool::AsShape( this->Parameter(PID_Curve) )->SetShape(E);
    ActParamTool::AsShape( this->Parameter(PID_Surface) )->SetShape(F);
  }
}

//-----------------------------------------------------------------------------

void asiData_IVCurve2dNode::SetDrawOrientationTip(const bool on)
{
  ActParamTool::AsBool(this->Parameter(PID_DrawOriTip))->SetValue(on);
}

//-----------------------------------------------------------------------------

bool asiData_IVCurve2dNode::GetDrawOrientationTip() const
{
  return ActParamTool::AsBool(this->Parameter(PID_DrawOriTip))->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVCurve2dNode::SetHasColor(const bool hasColor)
{
  ActParamTool::AsBool(this->Parameter(PID_HasColor))->SetValue(hasColor);
}

//-----------------------------------------------------------------------------

bool asiData_IVCurve2dNode::HasColor() const
{
  return ActParamTool::AsBool(this->Parameter(PID_HasColor))->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVCurve2dNode::SetColor(const int color)
{
  ActParamTool::AsInt(this->Parameter(PID_Color))->SetValue(color);
}

//-----------------------------------------------------------------------------

int asiData_IVCurve2dNode::GetColor() const
{
  return ActParamTool::AsInt(this->Parameter(PID_Color))->GetValue();
}
