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
#include <Geom_BSplineCurve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>

#include <mobius/geom_BSplineSurface.h>
#include <mobius/cascade.h>

//-----------------------------------------------------------------------------

//! JOIN operator for concatenating the passed natural-bounded surfaces into
//! a single surface.
class asiAlgo_ConvertToBezier : public ActAPI_IAlgorithm
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_ConvertToBezier, ActAPI_IAlgorithm)

  //! Constructor.
  //! \param[in] wViewerPart   part viewer.
  asiAlgo_ConvertToBezier(ActAPI_ProgressEntry progress,
                          ActAPI_PlotterEntry  plotter) : ActAPI_IAlgorithm(progress, plotter) {}

  //! Convert Curve to Bezier curve, degree 3, continuity C1.
  //! \param[in] curve curve.
  //! \param[in] f     first u parameter.
  //! \param[in] l     last u parameter.
  asiAlgo_EXPORT bool
    Perform(const Handle(Geom_Curve)& curve, double f, double l);

  //! Convert Surface to Bezier surface, degree (3, 3), continuity C1.
  //! \param[in] surface  surface.
  //! \param[in] toApprox whether to approximate or not.
  asiAlgo_EXPORT bool
    Perform(const Handle(Geom_Surface)& surface, bool toApprox = false);

  //! Get result surface.
  asiAlgo_EXPORT Handle(Geom_Surface)
    GetSurface() const;

  //! Get result curve.
  asiAlgo_EXPORT Handle(Geom_Curve)
    GetCurve() const;

private:

  Handle(Geom_Curve)          m_bCurve;   // result curve.
  Handle(Geom_BSplineSurface) m_bSurface; // result surface.
};

#endif
