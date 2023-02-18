//-----------------------------------------------------------------------------
// Created on: 02 December 2015
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

#ifndef asiData_FaceNode_h
#define asiData_FaceNode_h

// asiData includes
#include <asiData_FaceNodeBase.h>

//-----------------------------------------------------------------------------

//! Node representing a single b-rep face.
class asiData_FaceNode : public asiData_FaceNodeBase
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_FaceNode, asiData_FaceNodeBase)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_FaceNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
    PID_ShowOriTips = asiData_FaceNodeBase::PID_Last, //!< Whether to show orientation tips for the edges.
    PID_UScaleCoeff,                                  //!< Scaling in U curvilinear direction.
    PID_VScaleCoeff,                                  //!< Scaling in V curvilinear direction.
    PID_Last        = PID_ShowOriTips + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of Face Node ensuring its correct
  //! allocation in a heap.
  //! \return new instance of the Face Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

public:

  //! Performs initial actions required to make Node WELL-FORMED.
  asiData_EXPORT void
    Init(const bool resetScaling = true);

public:

  //! Sets a flag indicating whether the orientation tips should
  //! be visualized or not.
  //! \param[in] on the Boolean value to set.
  asiData_EXPORT void
    SetShowOriTips(const bool on);

  //! \return the flag indicating whether the orientation tips for
  //!         edges are supposed to be visualized or not.
  asiData_EXPORT bool
    GetShowOriTips() const;

  //! Sets a double coefficient for scaling U curvilinear direction.
  //! \param[in] coeff the scaling coefficient to set.
  asiData_EXPORT void
    SetUScaleCoeff(const double coeff);

  //! \return the coefficient for scaling U curvilinear direction.
  asiData_EXPORT double
    GetUScaleCoeff() const;

  //! Sets a double coefficient for scaling V curvilinear direction.
  //! \param[in] coeff the scaling coefficient to set.
  asiData_EXPORT void
    SetVScaleCoeff(const double coeff);

  //! \return the coefficient for scaling V curvilinear direction.
  asiData_EXPORT double
    GetVScaleCoeff() const;

protected:

  //! Default ctor. Registers all involved Parameters.
  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_FaceNode();

};

#endif
