//-----------------------------------------------------------------------------
// Created on: 14 May 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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

#ifndef asiAlgo_RecognizeCavitiesRule_h
#define asiAlgo_RecognizeCavitiesRule_h

// asiAlgo includes
#include <asiAlgo_RecognitionRule.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! Recognition rule for cavities.
class asiAlgo_RecognizeCavitiesRule : public asiAlgo_RecognitionRule
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_RecognizeCavitiesRule, asiAlgo_RecognitionRule)

public:

  //! Ctor accepting the AAG iterator.
  //! \param[in] it       the AAG iterator.
  //! \param[in] maxSize  the max allowed feature size.
  //! \param[in] progress the progress entry.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_RecognizeCavitiesRule(const Handle(asiAlgo_AAGIterator)& it,
                                  const double                       maxSize,
                                  ActAPI_ProgressEntry               progress = nullptr,
                                  ActAPI_PlotterEntry                plotter  = nullptr);

protected:

  //! Recognizes feature starting from the current position of AAG iterator.
  //! \param[out] featureFaces   the outcome feature faces.
  //! \param[out] featureIndices the indices of the outcome feature faces.
  //! \return true/false.
  asiAlgo_EXPORT virtual bool
    recognize(TopTools_IndexedMapOfShape& featureFaces,
              asiAlgo_Feature&            featureIndices) override;

protected:

  //! Initializes the internal data structures.
  asiAlgo_EXPORT void
    init();

  //! Completes feature growing from the given base face and starting from the
  //! given start face.
  //! \param[in]  startId        index of the face to start off.
  //! \param[in]  start_face_idx indexes of the starting faces.
  //! \param[out] featureFaces   feature faces.
  //! \param[out] featureIndices indices of the feature faces.
  //! \param[out] isOk           indicates whether the feature is Ok to propagate on.
  asiAlgo_EXPORT void
    propagate(const int                   startId,
              const asiAlgo_Feature&      start_face_idx,
              TopTools_IndexedMapOfShape& featureFaces,
              asiAlgo_Feature&            featureIndices,
              bool&                       isOk);

  //! Checks that given wire has convex connection with adjacent faces.
  //! \return true if adjacency is convex.
  asiAlgo_EXPORT bool
    isConvex(const int          seed_face_id,
             const TopoDS_Wire& wire);

  //! Checks that the feature candidate faces do not span the entire shape.
  //! \return true/false.
  asiAlgo_EXPORT bool
    isNotEntireShape(const TopTools_IndexedMapOfShape& newFeatureFaces,
                     const asiAlgo_Feature&            newFeatureIndices);

  //! Checks whether the passed feature candidate fits into the size criterion.
  //! \return true/false.
  asiAlgo_EXPORT bool
    isSizeOk(const TopTools_IndexedMapOfShape& newFeatureFaces);

protected:

  //! Max allowed feature size.
  double m_fMaxSize;

  //! Data map to improve access to the outer wire.
  NCollection_DataMap<TopoDS_Face, TopoDS_Wire> m_mapFaceOuterWire;

};

#endif
