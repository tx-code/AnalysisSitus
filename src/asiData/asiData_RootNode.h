//-----------------------------------------------------------------------------
// Created on: 27 November 2015
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

#ifndef asiData_RootNode_h
#define asiData_RootNode_h

// A-Situs includes
#include <asiData.h>

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_SelectionParameter.h>
#include <ActData_Utils.h>

//-----------------------------------------------------------------------------

//! Root Node in the project hierarchy. This Node contains global properties.
class asiData_RootNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_RootNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_RootNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //--------------------------//
    PID_Name,                 //!< Name of the Node.
    PID_IsCoincidentTopo,     //!< Indicates whether to resolve coincident topology in 3D.
    PID_GroupHlr,             //!< "HLR" group.
    PID_PrsMeshHlr,           //!< Indicates whether HLR is enabled in the active renderer.
    PID_IsEnabledHiddenInHlr, //!< Indicates whether HLR should output hidden edges as well.
    PID_HlrTimeout,           //!< Timeout for precise HLR (in milliseconds).
  //--------------------------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of the Node ensuring its correct
  //! allocation in a heap.
  //! \return new instance of the Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

// Generic naming support:
public:

  //! Accessor for the Node's name.
  //! \return name of the Node.
  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  //! Sets name for the Node.
  //! \param[in] name name to set.
  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& name);

  //! \return true if real-time HLR is enabled.
  asiData_EXPORT bool
    IsMeshHlr() const;

  //! Sets real-time HLR mode for the active renderer.
  //! \param[in] isHlr the Boolean value to set.
  asiData_EXPORT void
    SetMeshHlr(const bool isHlr);

  //! Enables/disables resolving the coincident topology in VTK mapper.
  //! \param[in] on value to set.
  asiData_EXPORT void
    SetResolveCoincidentTopo(const bool on);

  //! \return true if coincident topology in 3D is resolved.
  asiData_EXPORT bool
    IsResolveCoincidentTopo() const;

  //! Enables/disables the extraction of hidden edges in the precise HLR mode.
  //! \param[in] on value to set.
  asiData_EXPORT void
    SetEnabledHiddenInHlr(const bool on);

  //! \return true if the hidden edge extraction mode is enabled for precise HLR.
  asiData_EXPORT bool
    IsEnabledHiddenInHlr() const;

  //! Sets timeout (in ms) for HLR projections.
  //! \param[in] timeout the timeout to set.
  asiData_EXPORT void
    SetHlrTimeout(const int timeout);

  //! \return timeout (in ms) for HLR projections.
  asiData_EXPORT int
    GetHlrTimeout() const;

// Initialization:
public:

  //! Performs initializations required to make this Node WELL-FORMED.
  asiData_EXPORT void
    Init();

protected:

  //! Default constructor. Registers all involved Parameters.
  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_RootNode();

};

#endif
