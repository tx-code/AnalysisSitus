//-----------------------------------------------------------------------------
// Created on: 30 August 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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
#include <asiData_IVAxesNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_IVAxesNode::asiData_IVAxesNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,      PID_Name);
  REGISTER_PARAMETER(RealArray, PID_Origin);
  REGISTER_PARAMETER(RealArray, PID_DX);
  REGISTER_PARAMETER(RealArray, PID_DY);
  REGISTER_PARAMETER(RealArray, PID_DZ);
  REGISTER_PARAMETER(Group,     PID_GroupPrs);
  REGISTER_PARAMETER(Bool,      PID_DrawTip);
  REGISTER_PARAMETER(Int,       PID_ColorX);
  REGISTER_PARAMETER(Int,       PID_ColorY);
  REGISTER_PARAMETER(Int,       PID_ColorZ);
  REGISTER_PARAMETER(Real,      PID_ScaleCoeff);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_IVAxesNode::Instance()
{
  return new asiData_IVAxesNode();
}

//! Performs initial actions required to make Node WELL-FORMED.
void asiData_IVAxesNode::Init()
{
  // Initialize properties.
  this->InitParameter (PID_Name,       "Name",            "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_GroupPrs,   "Presentation",    "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_ScaleCoeff, "Scale",           "",               ParameterFlag_IsVisible, true);

  /* Kept for potential future improvement (if someone would need that).
  this->InitParameter (PID_ColorX,     "Color X",         "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_ColorY,     "Color Y",         "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_ColorZ,     "Color Z",         "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_DrawTip,    "Orientation tip", "",               ParameterFlag_IsVisible, true);
  */

  ActParamTool::AsRealArray( this->Parameter(PID_Origin) ) ->SetArray(NULL);
  ActParamTool::AsRealArray( this->Parameter(PID_DX) )     ->SetArray(NULL);
  ActParamTool::AsRealArray( this->Parameter(PID_DY) )     ->SetArray(NULL);
  ActParamTool::AsRealArray( this->Parameter(PID_DZ) )     ->SetArray(NULL);

  this->SetDrawTip    ( true );
  this->SetColorDX    ( 255 << 16 | 0   << 8 | 0 );
  this->SetColorDY    ( 0   << 16 | 255 << 8 | 0 );
  this->SetColorDZ    ( 0   << 16 | 0   << 8 | 255 );
  this->SetScaleCoeff ( 1 );
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_IVAxesNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param[in] name the name to set.
void asiData_IVAxesNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------
// Handy API
//-----------------------------------------------------------------------------

gp_Pnt asiData_IVAxesNode::GetOrigin() const
{
  Handle(HRealArray)
    arr = ActParamTool::AsRealArray( this->Parameter(PID_Origin) )->GetArray();
  //
  if ( arr.IsNull() )
    return gp::Origin();

  return gp_Pnt( arr->Value(0), arr->Value(1), arr->Value(2) );
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetOrigin(const gp_Pnt& origin)
{
  Handle(ActData_RealArrayParameter)
    P = ActParamTool::AsRealArray( this->Parameter(PID_Origin) );

  Handle(HRealArray) arr = P->GetArray();
  //
  if ( arr.IsNull() )
    arr = new HRealArray(0, 2);

  arr->ChangeValue(0) = origin.X();
  arr->ChangeValue(1) = origin.Y();
  arr->ChangeValue(2) = origin.Z();

  P->SetArray(arr);
}

//-----------------------------------------------------------------------------

gp_Dir asiData_IVAxesNode::GetDX() const
{
  Handle(HRealArray)
    arr = ActParamTool::AsRealArray( this->Parameter(PID_DX) )->GetArray();
  //
  if ( arr.IsNull() )
    return gp::DX();

  return gp_Dir( arr->Value(0), arr->Value(1), arr->Value(2) );
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetDX(const gp_Dir& dx)
{
  Handle(ActData_RealArrayParameter)
    P = ActParamTool::AsRealArray( this->Parameter(PID_DX) );

  Handle(HRealArray) arr = P->GetArray();
  //
  if ( arr.IsNull() )
    arr = new HRealArray(0, 2);

  arr->ChangeValue(0) = dx.X();
  arr->ChangeValue(1) = dx.Y();
  arr->ChangeValue(2) = dx.Z();

  P->SetArray(arr);
}

//-----------------------------------------------------------------------------

gp_Dir asiData_IVAxesNode::GetDY() const
{
  Handle(HRealArray)
    arr = ActParamTool::AsRealArray( this->Parameter(PID_DY) )->GetArray();
  //
  if ( arr.IsNull() )
    return gp::DY();

  return gp_Dir( arr->Value(0), arr->Value(1), arr->Value(2) );
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetDY(const gp_Dir& dy)
{
  Handle(ActData_RealArrayParameter)
    P = ActParamTool::AsRealArray( this->Parameter(PID_DY) );

  Handle(HRealArray) arr = P->GetArray();
  //
  if ( arr.IsNull() )
    arr = new HRealArray(0, 2);

  arr->ChangeValue(0) = dy.X();
  arr->ChangeValue(1) = dy.Y();
  arr->ChangeValue(2) = dy.Z();

  P->SetArray(arr);
}

//-----------------------------------------------------------------------------

gp_Dir asiData_IVAxesNode::GetDZ() const
{
  Handle(HRealArray)
    arr = ActParamTool::AsRealArray( this->Parameter(PID_DZ) )->GetArray();
  //
  if ( arr.IsNull() )
    return gp::DZ();

  return gp_Dir( arr->Value(0), arr->Value(1), arr->Value(2) );
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetDZ(const gp_Dir& dz)
{
  Handle(ActData_RealArrayParameter)
    P = ActParamTool::AsRealArray( this->Parameter(PID_DZ) );

  Handle(HRealArray) arr = P->GetArray();
  //
  if ( arr.IsNull() )
    arr = new HRealArray(0, 2);

  arr->ChangeValue(0) = dz.X();
  arr->ChangeValue(1) = dz.Y();
  arr->ChangeValue(2) = dz.Z();

  P->SetArray(arr);
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetDrawTip(const bool on)
{
  ActParamTool::AsBool( this->Parameter(PID_DrawTip) )->SetValue(on);
}

//-----------------------------------------------------------------------------

bool asiData_IVAxesNode::GetDrawTip() const
{
  return ActParamTool::AsBool( this->Parameter(PID_DrawTip) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetColorDX(const int color)
{
  ActParamTool::AsInt( this->Parameter(PID_ColorX) )->SetValue(color);
}

//-----------------------------------------------------------------------------

int asiData_IVAxesNode::GetColorDX() const
{
  return ActParamTool::AsInt( this->Parameter(PID_ColorX) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetColorDY(const int color)
{
  ActParamTool::AsInt( this->Parameter(PID_ColorY) )->SetValue(color);
}

//-----------------------------------------------------------------------------

int asiData_IVAxesNode::GetColorDY() const
{
  return ActParamTool::AsInt( this->Parameter(PID_ColorY) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetColorDZ(const int color)
{
  ActParamTool::AsInt( this->Parameter(PID_ColorZ) )->SetValue(color);
}

//-----------------------------------------------------------------------------

int asiData_IVAxesNode::GetColorDZ() const
{
  return ActParamTool::AsInt( this->Parameter(PID_ColorZ) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVAxesNode::SetScaleCoeff(const double coeff)
{
  ActParamTool::AsReal( this->Parameter(PID_ScaleCoeff) )->SetValue(coeff);
}

//-----------------------------------------------------------------------------

double asiData_IVAxesNode::GetScaleCoeff() const
{
  return ActParamTool::AsReal( this->Parameter(PID_ScaleCoeff) )->GetValue();
}
