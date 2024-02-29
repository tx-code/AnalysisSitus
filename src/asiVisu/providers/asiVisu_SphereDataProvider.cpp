//-----------------------------------------------------------------------------
// Created on: 20 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023, Julia Slyadneva
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
#include <asiVisu_SphereDataProvider.h>

// asiData includes
#include <asiData_ThicknessNode.h>
#include <asiData_ClearanceNode.h>

#if defined USE_MOBIUS
// Mobius includes
#include <mobius/cascade.h>
using namespace mobius;
#endif
 
//-----------------------------------------------------------------------------

//! Ctor.
//! \param N [in] source Node.
asiVisu_SphereDataProvider::asiVisu_SphereDataProvider(const Handle(ActAPI_INode)& N)
: asiVisu_DataProvider(),
  m_node(N),
  m_diameter(0.),
  m_loc()
{
}

//-----------------------------------------------------------------------------

//! \return ID of the associated Data Node.
ActAPI_DataObjectId asiVisu_SphereDataProvider::GetNodeID() const
{
  if ( !m_node.IsNull() )
    return m_node->GetId();

  return ActAPI_DataObjectId();
}

//-----------------------------------------------------------------------------

void asiVisu_SphereDataProvider::SetScalarAndCoords(const double scalar,
                                                    const double v0[3],
                                                    const double v1[3],
                                                    const double v2[3])
{
  m_diameter = scalar;

  t_xyz p;
  poly_MeshUtils::ComputeCenter({ v0[0], v0[1], v0[2] },
                                { v1[0], v1[1], v1[2] },
                                { v2[0], v2[1], v2[2] }, p);

  t_xyz N;
  poly_MeshUtils::ComputeNormal({ v0[0], v0[1], v0[2] },
                                { v1[0], v1[1], v1[2] },
                                { v2[0], v2[1], v2[2] }, N);

  N.Reverse();

  core_XYZ c = p + N.Normalized() * (GetDiameter() / 2.);

  m_loc = cascade::GetOpenCascadePnt(c);

  m_points[0][0] = v0[0];
  m_points[0][1] = v0[1];
  m_points[0][2] = v0[2];

  m_points[1][0] = v1[0];
  m_points[1][1] = v1[1];
  m_points[1][2] = v1[2];

  m_points[2][0] = v2[0];
  m_points[2][1] = v2[1];
  m_points[2][2] = v2[2];
}

//-----------------------------------------------------------------------------

double asiVisu_SphereDataProvider::GetDiameter() const
{
  return m_diameter;
}

//-----------------------------------------------------------------------------

gp_Pnt asiVisu_SphereDataProvider::GetLocation() const
{
  return m_loc;
}

//-----------------------------------------------------------------------------

void asiVisu_SphereDataProvider::GetPoints(double coords[][3]) const
{
  coords[0][0] = m_points[0][0];
  coords[0][1] = m_points[0][1];
  coords[0][2] = m_points[0][2];

  coords[1][0] = m_points[1][0];
  coords[1][1] = m_points[1][1];
  coords[1][2] = m_points[1][2];

  coords[2][0] = m_points[2][0];
  coords[2][1] = m_points[2][1];
  coords[2][2] = m_points[2][2];
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList) asiVisu_SphereDataProvider::translationSources() const
{
  ActAPI_ParameterStream out;
  return out.List;
}