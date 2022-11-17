//-----------------------------------------------------------------------------
// Created on: 17 November 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

#ifndef asiAlgo_OrientCnc_h
#define asiAlgo_OrientCnc_h

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// std includes
#include <unordered_map>

//-----------------------------------------------------------------------------

//! This class attempts to find a proper orientation for the passed shape in
//! the assumption that it has a bunch of typical CNC features. This algorithm is
//! designed in a way to stay independent from feature recognizers and any specific
//! feature types, so it works on elementary B-rep elements and props of the model.
class asiAlgo_OrientCnc : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_OrientCnc, ActAPI_IAlgorithm)

public:

  //! Constructs the reorientation tool.
  //! \param[in] aag      the attributed adjacency graph.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_OrientCnc(const Handle(asiAlgo_AAG)& aag,
                      ActAPI_ProgressEntry       progress = nullptr,
                      ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Orients the master part of the AAG based on heuristics for main machining part.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! \return the computed transformation to apply to the shape.
  const gp_Trsf& GetTrsf() const
  {
    return m_T;
  }

protected:

  Handle(asiAlgo_AAG) m_aag; //!< The AAG instance.
  gp_Trsf             m_T;   //!< The computed transformation.

};

#endif
