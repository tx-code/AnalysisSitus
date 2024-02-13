//-----------------------------------------------------------------------------
// Created on: 16 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Elizaveta Krylova
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

#ifndef asiAlgo_ConvertToBezier_h
#define asiAlgo_ConvertToBezier_h

// asiAlgo includes
#include <asiAlgo.h>

// ActAPI includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Converts curves and surfaces to 3-degree Bezier form whenever possible.
class asiAlgo_ConvertToBezier : public ActAPI_IAlgorithm
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_ConvertToBezier, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_ConvertToBezier(ActAPI_ProgressEntry progress,
                          ActAPI_PlotterEntry  plotter) : ActAPI_IAlgorithm(progress, plotter) {}

public:

  //! Converts the passed curve to Bezier curve, degree 3, continuity C1.
  //! \param[in] curve the curve to convert.
  //! \return the converted curve or null if conversion failed.
  asiAlgo_EXPORT Handle(Geom_Curve)
    Perform(const Handle(Geom_Curve)& curve);

  //! Converts the passed surface to Bezier surface, degree (3, 3), continuity C1.
  //! \param[in] surface  the surface to convert.
  //! \param[in] toApprox the Boolean flag indicating whether approximation is allowed
  //!                     to reduce polynomial degrees.
  //! \return the converted surface or null if conversion failed.
  asiAlgo_EXPORT Handle(Geom_Surface)
    Perform(const Handle(Geom_Surface)& surface,
            const bool                  toApprox = false);

public:

  //! \return max achieved error.
  double GetMaxError() const
  {
    return m_fMaxError;
  }

protected:

  double m_fMaxError; //!< Max computed deviation.

};

#endif
