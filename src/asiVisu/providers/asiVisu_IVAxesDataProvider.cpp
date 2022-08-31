//-----------------------------------------------------------------------------
// Created on: 31 August 2022
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
#include <asiVisu_IVAxesDataProvider.h>

//-----------------------------------------------------------------------------

//! Ctor.
asiVisu_IVAxesDataProvider::asiVisu_IVAxesDataProvider(const Handle(asiData_IVAxesNode)& node)
: asiVisu_AxesDataProvider(node)
{}

//-----------------------------------------------------------------------------

gp_Pnt asiVisu_IVAxesDataProvider::GetOrigin()
{
  return Handle(asiData_IVAxesNode)::DownCast(m_source)->GetOrigin();
}

//-----------------------------------------------------------------------------

gp_Dir asiVisu_IVAxesDataProvider::GetDX()
{
  return Handle(asiData_IVAxesNode)::DownCast(m_source)->GetDX();
}

//-----------------------------------------------------------------------------

gp_Dir asiVisu_IVAxesDataProvider::GetDY()
{
  return Handle(asiData_IVAxesNode)::DownCast(m_source)->GetDY();
}

//-----------------------------------------------------------------------------

gp_Dir asiVisu_IVAxesDataProvider::GetDZ()
{
  return Handle(asiData_IVAxesNode)::DownCast(m_source)->GetDZ();
}

//-----------------------------------------------------------------------------

bool asiVisu_IVAxesDataProvider::HasOrientationTip() const
{
  return false;
}

//-----------------------------------------------------------------------------

double asiVisu_IVAxesDataProvider::GetScaleCoeff() const
{
  return Handle(asiData_IVAxesNode)::DownCast(m_source)->GetScaleCoeff();
}

//-----------------------------------------------------------------------------

//! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
//! translation process.
//! \return source Parameters.
Handle(ActAPI_HParameterList) asiVisu_IVAxesDataProvider::translationSources() const
{
  ActParamStream out;

  out << m_source->Parameter(asiData_IVAxesNode::PID_Origin)
      << m_source->Parameter(asiData_IVAxesNode::PID_DX)
      << m_source->Parameter(asiData_IVAxesNode::PID_DY)
      << m_source->Parameter(asiData_IVAxesNode::PID_DZ)
      << m_source->Parameter(asiData_IVAxesNode::PID_ScaleCoeff);

  return out;
}
