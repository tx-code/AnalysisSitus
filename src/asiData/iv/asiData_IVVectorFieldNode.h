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

#ifndef asiData_IVVectorFieldNode_h
#define asiData_IVVectorFieldNode_h

// asiData includes
#include <asiData.h>

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>

// Active Data includes
#include <ActData_BaseNode.h>

//! Data Node representing a vector field in IV (Imperative Viewer).
class asiData_IVVectorFieldNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_IVVectorFieldNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_IVVectorFieldNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //------------------//
  // Common           //
  //------------------//
    PID_Name,         //!< Name of the Node.
  //------------------//
  // Geometry         //
  //------------------//
    PID_Points,       //!< Coordinates of vector positions.
    PID_Vectors,      //!< Coordinates of vectors.
  //------------------//
  // Presentation     //
  //------------------//
    PID_GroupPrs,     //!< Presentation group.
    PID_DrawTip,      //!< Whether to draw tip for the vectors.
    PID_HasColor,     //!< Indicates whether the Color Parameter is active.
    PID_Color,        //!< Color.
    PID_ScaleCoeff,   //!< Scaling coefficient.
  //------------------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

// Generic naming support:
public:

  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& name);

// Handy accessors to the stored data:
public:

  asiData_EXPORT Handle(asiAlgo_BaseCloud<double>)
    GetPoints() const;

  asiData_EXPORT void
    SetPoints(const Handle(asiAlgo_BaseCloud<double>)& pointCloud);

  asiData_EXPORT Handle(asiAlgo_BaseCloud<double>)
    GetVectors() const;

  asiData_EXPORT void
    SetVectors(const Handle(asiAlgo_BaseCloud<double>)& vectorCloud);

  //! Sets the property indicating whether to draw the orientation tip
  //! for the vectors.
  //! \param[in] on true/false.
  asiData_EXPORT void
    SetDrawTip(const bool on);

  //! \return stored value of orientation tip flag.
  asiData_EXPORT bool
    GetDrawTip() const;

  //! Sets the property indicating whether this Node has the Color Parameter activated.
  asiData_EXPORT void
    SetHasColor(const bool);

  //! \return the Boolean flag indicating whether this Node has the Color Parameter activated.
  asiData_EXPORT bool
    HasColor() const;

  //! Sets the color.
  asiData_EXPORT void
    SetColor(const int);

  //! \return color.
  asiData_EXPORT int
    GetColor() const;

  //! Sets the scaling coefficient.
  asiData_EXPORT void
    SetScaleCoeff(const double);

  //! \return scaling coefficient.
  asiData_EXPORT double
    GetScaleCoeff() const;

// Initialization:
public:

  asiData_EXPORT void
    Init();

protected:

  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_IVVectorFieldNode();

};

#endif
