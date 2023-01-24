//-----------------------------------------------------------------------------
// Created on: 25 January 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Elizaveta Krylova
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
#include <asiAlgo_BuildOBB.h>

// asiAlgo includes
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <GProp_GProps.hxx>

// STL includes
#include <vector>

//-----------------------------------------------------------------------------

asiAlgo_BuildOBB::asiAlgo_BuildOBB(const Handle(asiAlgo_AAG)& aag,
                                   ActAPI_ProgressEntry       progress,
                                   ActAPI_PlotterEntry        plotter)
  //
  : ActAPI_IAlgorithm(progress, plotter),
    m_aag(aag)
{
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildOBB::Perform()
{
  // Find a better orientation.
  asiAlgo_OrientCnc orient(m_aag);
  //
  if (!orient.Perform())
  {
    return false;
  }

  // Make another shape.
  BRepBuilderAPI_Transform transform = BRepBuilderAPI_Transform(orient.GetTrsf());
  transform.Perform(m_aag->GetMasterShape(), true);
  //
  const TopoDS_Shape& oriented = transform.Shape();
  //
  tl::optional<gp_Ax3> ax = orient.GetAxes();
  //
  ax->Transform(orient.GetTrsf());
  //
  double xMin, yMin, zMin, xMax, yMax, zMax;
  asiAlgo_Utils::Bounds(oriented, xMin, yMin, zMin, xMax, yMax, zMax);
  // Protect from degenerated bbox.
  const double precision = Precision::Confusion();
  if (Abs(xMin - xMax) < precision ||
      Abs(yMin - yMax) < precision ||
      Abs(zMin - zMax) < precision)
  {
    xMin -= precision;
    yMin -= precision;
    zMin -= precision;
    xMax += precision;
    yMax += precision;
    zMax += precision;
  }

  // Set placement and corner positions to the result.
  gp_Pnt corner_min = gp_XYZ(xMin, yMin, zMin);
  gp_Pnt corner_max = gp_XYZ(xMax, yMax, zMax);
  //
  if (ax)
  {
    m_obb.Placement = *ax;
  }
  m_obb.Trsf           = orient.GetTrsf();
  m_obb.LocalCornerMin = corner_min;
  m_obb.LocalCornerMax = corner_max;

  return true;
}

//-----------------------------------------------------------------------------

const asiAlgo_OBB& asiAlgo_BuildOBB::GetResult() const
{
  return m_obb;
}

//-----------------------------------------------------------------------------

gp_Trsf asiAlgo_BuildOBB::GetResultTrsf() const
{
  gp_Trsf T;
  T.SetTransformation(m_obb.Placement);
  T.Invert();
  return T;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_BuildOBB::GetResultBox() const
{
  BRepPrimAPI_MakeBox mkOBB(m_obb.LocalCornerMin, m_obb.LocalCornerMax);
  gp_Trsf T = m_obb.Trsf.Inverted();
  return BRepBuilderAPI_Transform(mkOBB.Solid(), T, 1);
}
