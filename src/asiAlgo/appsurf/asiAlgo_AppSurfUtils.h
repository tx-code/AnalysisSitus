//-----------------------------------------------------------------------------
// Created on: 18 March 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
#include <asiAlgo_BaseCloud.h>

#if defined USE_MOBIUS
  // Mobius includes
  #include <mobius/geom_PositionCloud.h>
#endif

// Active Data includes
#include <ActAPI_IPlotter.h>

// OpenCascade includes
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TopTools_HSequenceOfShape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Common function for PLATE, APPSURF1 and APPSURF2.
namespace asiAlgo_AppSurfUtils
{
  //! Constructs initial spliny plane with the specified numbers of U and V knots.
  //! The U and V degrees of the corresponding basis spline functions will be elevated
  //! up to the passed values of `degU` and `degV` arguments.
  asiAlgo_EXPORT Handle(Geom_BSplineSurface)
    PrepareInitialPlane(const Handle(asiAlgo_BaseCloud<double>)& pts,
                        const int                                numUKnots,
                        const int                                numVKnots,
                        const int                                degU,
                        const int                                degV);

#if defined USE_MOBIUS
  //! Prepares pinpoint constraints for approximation as a point cloud.
  asiAlgo_EXPORT void
    PrepareConstraints(const double                             prec,
                       const Handle(TopTools_HSequenceOfShape)& edges,
                       const Handle(asiAlgo_BaseCloud<double>)& extras,
                       const mobius::t_ptr<mobius::t_pcloud>&   pts);
#endif

  //! Measures the deviation between the passed B-spline surface and
  //! a scattered point cloud.
  asiAlgo_EXPORT void
    MeasureDeviation(const Handle(Geom_BSplineSurface)&       bsurf,
                     const Handle(asiAlgo_BaseCloud<double>)& pts,
                     double&                                  minDeviation,
                     double&                                  maxDeviation,
                     double&                                  avrDeviation,
                     gp_Pnt&                                  maxDevPt);

  //! Measures the deviation between two passed B-spline surfaces.
  //! \param[in]  resSurf  the surface constructed by an approximation algorithm.
  //! \param[in]  initSurf the initial surface to compare with.
  //! \param[out] maxDev   the computed max deviation.
  //! \param[in]  plotter  the plotter entry.
  asiAlgo_EXPORT void
    MeasureDeviation(const Handle(Geom_BSplineSurface)& resSurf,
                     const Handle(Geom_BSplineSurface)& initSurf,
                     double&                            maxDev,
                     ActAPI_PlotterEntry                plotter);

  //! Measures the deviation between two passed B-spline curves.
  //! \param[in]  resCurve  the B-spline curve constructed by an approximation algorithm.
  //! \param[in]  initCurve the initial curve to compare with.
  //! \param[out] maxDev    the computed max deviation.
  //! \param[in]  plotter   the plotter entry.
  asiAlgo_EXPORT void
    MeasureDeviation(const Handle(Geom_BSplineCurve)& resCurve,
                     const Handle(Geom_BSplineCurve)& initCurve,
                     double&                          maxDev,
                     ActAPI_PlotterEntry              plotter);
}
