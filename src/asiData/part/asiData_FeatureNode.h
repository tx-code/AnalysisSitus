//-----------------------------------------------------------------------------
// Created on: 06 December 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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

#ifndef asiData_FeatureNode_h
#define asiData_FeatureNode_h

// asiData includes
#include <asiData.h>

// Active Data includes
#include <ActData_BaseNode.h>

// OpenCascade includes
#include <TColStd_HPackedMapOfInteger.hxx>

//-----------------------------------------------------------------------------

//! Persistent feature descriptor.
class asiData_FeatureNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_FeatureNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_FeatureNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //------------------//
    PID_Name,         //!< Name of the Node.
    PID_FeatureId,    //!< Feature ID.
    PID_Mask,         //!< Sub-shape IDs comprising a feature.
    PID_Color,        //!< Color associated with the feature.
    PID_Comment,      //!< Free text comment.
  //------------------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of the Node ensuring its correct
  //! allocation in a heap.
  //! \return new instance of the Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

// Initialization:
public:

  //! Performs initial actions required to make this Node WELL-FORMED.
  asiData_EXPORT void
    Init();

// Generic naming support:
public:

  //! \return feature name.
  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  //! Sets feature name.
  //! \param[in] name the feature name to set.
  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& name);

// Convenience methods:
public:

  //! \return feature ID (domain-specific index).
  asiData_EXPORT int
    GetFeatureId() const;

  //! Sets feature ID to store.
  //! \param[in] featureId the feature ID to set.
  asiData_EXPORT void
    SetFeatureId(const int featureId);

  //! \return feature mask.
  asiData_EXPORT Handle(TColStd_HPackedMapOfInteger)
    GetMask() const;

  //! Accessor for the feature elements.
  //! \param[out] mask the returned mask.
  asiData_EXPORT void
    GetMask(TColStd_PackedMapOfInteger& mask) const;

  //! Sets the feature mask.
  //! \param[in] mask the elements comprising the feature.
  asiData_EXPORT void
    SetMask(const Handle(TColStd_HPackedMapOfInteger)& mask);

  //! Sets the feature mask.
  //! \param[in] mask the elements comprising the feature.
  asiData_EXPORT void
    SetMask(const TColStd_PackedMapOfInteger& mask);

  //! Sets color for the feature.
  //! \param[in] icolor the color to set.
  asiData_EXPORT void
    SetColor(const int icolor);

  //! \return the integer representation of feature's color.
  asiData_EXPORT int
    GetColor() const;

  //! Sets free text comment associated with the feature.
  //! \param[in] comment the comment to set.
  asiData_EXPORT void
    SetComment(const TCollection_AsciiString& comment);

  //! \return the free text comment associated with the feature.
  asiData_EXPORT TCollection_AsciiString
    GetComment() const;

protected:

  //! Default constructor. Registers all involved Parameters.
  //! Allocation is allowed only via Instance() method.
  asiData_EXPORT
    asiData_FeatureNode();

};

#endif
