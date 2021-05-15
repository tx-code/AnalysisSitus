//-----------------------------------------------------------------------------
// Created on: 14 May 2020
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

#ifndef asiAlgo_RecognizeCavities_h
#define asiAlgo_RecognizeCavities_h

// asiAlgo includes
#include <asiAlgo_Recognizer.h>

// Standard includes
#include <unordered_map>

//-----------------------------------------------------------------------------

//! Recognizes all negative features in a CAD part. Their shape
//! does not matter. A feature to recognize is expected to terminate
//! at inner contours of its base faces.
class asiAlgo_RecognizeCavities : public asiAlgo_Recognizer
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_RecognizeCavities, asiAlgo_Recognizer)

public:

  //! Ctor with a shape.
  //! \param[in] shape    the shape to recognize.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_RecognizeCavities(const TopoDS_Shape&  shape,
                              ActAPI_ProgressEntry progress = nullptr,
                              ActAPI_PlotterEntry  plotter  = nullptr);

  //! Ctor with AAG.
  //! \param[in] aag      the AAG instance for the shape to recognize.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_RecognizeCavities(const Handle(asiAlgo_AAG)& aag,
                              ActAPI_ProgressEntry       progress = nullptr,
                              ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Sets the max allowed feature size.
  //! \param[in] maxSize the feature size to set.
  asiAlgo_EXPORT void
    SetMaxSize(const double maxSize);

  //! \return the max allowed feature size.
  asiAlgo_EXPORT double
    GetMaxSize() const;

  //! \return cavity features distributed by their base faces.
  asiAlgo_EXPORT const std::vector< std::pair<asiAlgo_Feature, asiAlgo_Feature> >&
    GetCavities() const;

public:

  //! Performs recognition.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

protected:

  //! Returns all seed faces for the recognizer. A seed face should have
  //! only convex dihedral angles at its inner contours.
  //! \param[out] seeds the found seed faces.
  asiAlgo_EXPORT void
    findSeeds(asiAlgo_Feature& seeds);

  //! Collects cavity features and their base faces.
  asiAlgo_EXPORT void
    collectCavities();

protected:

  //! Max allowed feature size.
  double m_fMaxSize;

  //! Recognized cavities and their base faces.
  std::vector< std::pair<asiAlgo_Feature, asiAlgo_Feature> > m_cavities;

};

#endif
