//-----------------------------------------------------------------------------
// Created on: 31 July 2021
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

#ifndef asiData_Grid2dNode_h
#define asiData_Grid2dNode_h

// asiData includes
#include <asiData.h>

// asiAlgo includes
#include <asiAlgo_UniformGrid.h>

// Active Data includes
#include <ActData_BaseNode.h>

//-----------------------------------------------------------------------------

//! Node representing a two-dimensional grid.
class asiData_Grid2dNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_Grid2dNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_Grid2dNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //-------------------------//
    PID_Name,                //!< Name of the Node.
    PID_UniformGrid,         //!< The uniform grid.
  //-------------------------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of this Node ensuring its correct
  //! allocation in a heap.
  //! \return new instance of the Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

public:

  //! Performs initial actions required to make Node WELL-FORMED.
  asiData_EXPORT void
    Init();

// Generic naming support:
public:

  //! Accessor for the Node's name.
  //! \return name of the Node.
  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  //! Sets name for the Node.
  //! \param[in] name name to set.
  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString&);

public:

  //! \return uniform grid.
  asiData_EXPORT Handle(asiAlgo_UniformGrid<float>)
    GetUniformGrid() const;

  //! Sets the uniform grid to store.
  //! \param[in] grid uniform grid to store.
  asiData_EXPORT void
    SetUniformGrid(const Handle(asiAlgo_UniformGrid<float>)& grid);

protected:

  //! Default ctor. Registers all involved Parameters.
  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_Grid2dNode();

};

#endif
