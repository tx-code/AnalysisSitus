//-----------------------------------------------------------------------------
// Created on: 24 January 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Julia Slyadneva
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

#pragma once

// asiAlgo includes
#include <asiAlgo_FeatureAttrFace.h>
#include <asiAlgo_UniformGrid.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Attribute to set a uniform grid to a face.
class asiAlgo_AttrFaceUniformGrid : public asiAlgo_FeatureAttrFace
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_AttrFaceUniformGrid, asiAlgo_FeatureAttrFace)

public:

  //! Creates attribute with feature ID.
  //! \param[in] ugrid     uniform grid
  //! \param[in] featureId 1-based feature ID.
  asiAlgo_AttrFaceUniformGrid(const Handle(asiAlgo_UniformGrid<float>)& ugrid,
                              const int                                 featureId = 0)
    : asiAlgo_FeatureAttrFace(featureId),
      m_grid(ugrid)
  {}

public:

  //! \return static GUID associated with this type of attribute.
  static const Standard_GUID& GUID()
  {
    static Standard_GUID guid("36427147-AB36-4512-A376-61BA9014251A");
    return guid;
  }

  //! \return GUID associated with this type of attribute.
  virtual const Standard_GUID& GetGUID() const override
  {
    return GUID();
  }

  //! \return human-readable name of the attribute.
  virtual const char* GetName() const override
  {
    return "Uniform grid";
  }

  //! \return a uniform grid cached by the attribute.
  const Handle(asiAlgo_UniformGrid<float>)& GetGrid() const
  {
    return m_grid;
  }
 
private:

  Handle(asiAlgo_UniformGrid<float>) m_grid;
};
