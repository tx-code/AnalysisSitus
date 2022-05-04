//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

#ifndef asiData_DiscrFaceNode_h
#define asiData_DiscrFaceNode_h

// asiData includes
#include <asiData.h>

// asiAlgo includes
#include <asiAlgo_DiscrModel.h>

// Active Data includes
#include <asiData_FaceNodeBase.h>

//-----------------------------------------------------------------------------

//! Node representing a discrete face.
class asiData_DiscrFaceNode : public asiData_FaceNodeBase
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_DiscrFaceNode, asiData_FaceNodeBase)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_DiscrFaceNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //-------------------------//
    PID_DiscrModel = asiData_FaceNodeBase::PID_Last, //! Discrete model.
  //-------------------------//
    PID_Last = PID_DiscrModel + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of this Node ensuring its correct
  //! allocation in a heap.
  //! \return new instance of the Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

public:

  //! Performs initial actions required to make Node WELL-FORMED.
  asiData_EXPORT virtual void
    Init();

public:

  //! \return discrete model.
  asiData_EXPORT Handle(asiAlgo::discr::Model)
    GetDiscrModel() const;

  //! Sets the discrete model to store.
  //! \param[in] model discrete model to store.
  asiData_EXPORT void
    SetDiscrModel(const Handle(asiAlgo::discr::Model)& model);

protected:

  //! Default ctor. Registers all involved Parameters.
  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_DiscrFaceNode();

};

#endif
