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
#include <asiVisu_IVCurve2dDataProvider.h>

// asiData includes
#include <asiData_IVCurve2dNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

asiVisu_IVCurve2dDataProvider::asiVisu_IVCurve2dDataProvider(const Handle(ActAPI_INode)& N)
: asiVisu_CurveDataProvider(),
  m_node(N)
{
}

//-----------------------------------------------------------------------------

Handle(Standard_Type) asiVisu_IVCurve2dDataProvider::GetCurveType() const
{
  double f, l;
  return this->GetCurve2d(f, l)->DynamicType();
}

//-----------------------------------------------------------------------------

Handle(Geom2d_Curve) asiVisu_IVCurve2dDataProvider::GetCurve2d(double& f, double& l) const
{
  Handle(asiData_IVCurve2dNode)
    curve_n = Handle(asiData_IVCurve2dNode)::DownCast(m_node);
  //
  if ( curve_n.IsNull() || !curve_n->IsWellFormed() )
    return nullptr;

  Handle(Geom_Surface) surf;
  return curve_n->GetCONS(surf, f, l);
}

//-----------------------------------------------------------------------------

Handle(Geom_Curve) asiVisu_IVCurve2dDataProvider::GetCurve(double&, double&) const
{
  return nullptr;
}

//-----------------------------------------------------------------------------

ActAPI_DataObjectId asiVisu_IVCurve2dDataProvider::GetNodeID() const
{
  return m_node->GetId();
}

//-----------------------------------------------------------------------------

Handle(asiVisu_IVCurve2dDataProvider) asiVisu_IVCurve2dDataProvider::Clone() const
{
  return new asiVisu_IVCurve2dDataProvider(m_node);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList) asiVisu_IVCurve2dDataProvider::translationSources() const
{
  // Resulting Parameters
  ActParamStream out;

  Handle(asiData_IVCurve2dNode)
    curve_n = Handle(asiData_IVCurve2dNode)::DownCast(m_node);
  //
  if ( curve_n.IsNull() || !curve_n->IsWellFormed() )
    return out;

  // Register Parameter as sensitive
  out << curve_n->Parameter(asiData_IVCurve2dNode::PID_Curve)
      << curve_n->Parameter(asiData_IVCurve2dNode::PID_Surface)
      << curve_n->Parameter(asiData_IVCurve2dNode::PID_DrawOriTip);

  return out;
}

//-----------------------------------------------------------------------------

bool asiVisu_IVCurve2dDataProvider::GetDrawOrientationTip() const
{
  if (m_node.IsNull())
    return false;

  Handle(asiData_IVCurve2dNode)
    curve_n = Handle(asiData_IVCurve2dNode)::DownCast(m_node);
  //
  if ( curve_n.IsNull() || !curve_n->IsWellFormed() )
    return false;

  return curve_n->GetDrawOrientationTip();
}
