//-----------------------------------------------------------------------------
// Created on: 03 August 2021
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

#ifndef asiAlgo_FeatureAttrConvexHull_h
#define asiAlgo_FeatureAttrConvexHull_h

// asiAlgo includes
#include <asiAlgo_FeatureAttrFace.h>

//-----------------------------------------------------------------------------

//! Attribute to mark a face as belonging to the convex hull.
class asiAlgo_FeatureAttrConvexHull : public asiAlgo_FeatureAttrFace
{
public:

  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_FeatureAttrConvexHull, asiAlgo_FeatureAttrFace)

public:

  //! Creates attribute with feature ID.
  //! \param[in] featureId 1-based feature ID.
  asiAlgo_FeatureAttrConvexHull(const int featureId = 0) : asiAlgo_FeatureAttrFace(featureId)
  {}

public:

  //! \return static GUID associated with this type of attribute.
  static const Standard_GUID& GUID()
  {
    static Standard_GUID guid("7D64A5BE-8A8E-420F-B9CC-8874CC0E7A3A");
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
    return "Convex-hull face";
  }

};

#endif
