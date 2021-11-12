//-----------------------------------------------------------------------------
// Created on: 09 September 2021
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
#include <asiData_IVVectorFieldNode.h>

// asiAlgo includes
#include <asiAlgo_PointCloudUtils.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_IVVectorFieldNode::asiData_IVVectorFieldNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,      PID_Name);
  REGISTER_PARAMETER(RealArray, PID_Points);
  REGISTER_PARAMETER(RealArray, PID_Vectors);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_IVVectorFieldNode::Instance()
{
  return new asiData_IVVectorFieldNode();
}

//! Performs initial actions required to make Node WELL-FORMED.
void asiData_IVVectorFieldNode::Init()
{
  // Initialize name Parameter
  this->InitParameter(PID_Name, "Name");
  //
  this->SetPoints(nullptr);
  this->SetVectors(nullptr);
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_IVVectorFieldNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param[in] name the name to set.
void asiData_IVVectorFieldNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------
// Handy API
//-----------------------------------------------------------------------------

//! \return stored point cloud representing the positions of vertices.
Handle(asiAlgo_BaseCloud<double>) asiData_IVVectorFieldNode::GetPoints() const
{
  Handle(TColStd_HArray1OfReal)
    coords = ActParamTool::AsRealArray( this->Parameter(PID_Points) )->GetArray();
  //
  return asiAlgo_PointCloudUtils::AsCloudd(coords);
}

//! Sets point cloud to store.
//! \param[in] points the points to store.
void asiData_IVVectorFieldNode::SetPoints(const Handle(asiAlgo_BaseCloud<double>)& points)
{
  Handle(TColStd_HArray1OfReal) arr = asiAlgo_PointCloudUtils::AsRealArray(points);
  //
  ActParamTool::AsRealArray( this->Parameter(PID_Points) )->SetArray( points.IsNull() ? nullptr : arr );
}

//! \return stored vector field as a base cloud of coordinate triples.
Handle(asiAlgo_BaseCloud<double>) asiData_IVVectorFieldNode::GetVectors() const
{
  Handle(TColStd_HArray1OfReal)
    coords = ActParamTool::AsRealArray( this->Parameter(PID_Vectors) )->GetArray();
  //
  return asiAlgo_PointCloudUtils::AsCloudd(coords);
}

//! Sets vector field to store.
//! \param[in] vectors the vector field to store.
void asiData_IVVectorFieldNode::SetVectors(const Handle(asiAlgo_BaseCloud<double>)& vectors)
{
  Handle(TColStd_HArray1OfReal) arr = asiAlgo_PointCloudUtils::AsRealArray(vectors);
  //
  ActParamTool::AsRealArray( this->Parameter(PID_Vectors) )->SetArray( vectors.IsNull() ? nullptr : arr );
}