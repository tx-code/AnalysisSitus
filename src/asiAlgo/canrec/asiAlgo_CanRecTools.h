//-----------------------------------------------------------------------------
// Created on: 30 April 2021
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

#ifndef asiAlgo_CanRecTools_h
#define asiAlgo_CanRecTools_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <TColgp_Array1OfPnt2d.hxx>

//-----------------------------------------------------------------------------

//! Canonical recognition utilities.
namespace asiAlgo_CanRecTools
{
  //! Checks if the passed array of two-dimensional points can be approximated
  //! with a straight line segment within the prescribed tolerance.
  //! \param[in]  pts   the points to check.
  //! \param[in]  toler the fitting tolerance to use.
  //! \param[out] dev   the achieved deviation.
  //! \param[out] lin   the best fit line.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsLinear(const TColgp_Array1OfPnt2d& pts,
             const double                toler,
             double&                     dev,
             gp_Lin2d&                   lin);

  //! Checks if the passed surface can be represented with an analytical
  //! cylinder.
  //!
  //! \param[in]  surface  the surface in question.
  //! \param[in]  uMinSurf the min U parameter.
  //! \param[in]  uMaxSurf the max U parameter.
  //! \param[in]  vMinSurf the min V parameter.
  //! \param[in]  vMaxSurf the max V parameter.
  //! \param[in]  toler    the tolerance to use.
  //! \param[out] cyl      the extracted cylinder.
  //! \param[out] uMinCyl  the U min parameter of the cylinder's domain.
  //! \param[out] uMaxCyl  the U max parameter of the cylinder's domain.
  //! \param[out] vMinCyl  the V min parameter of the cylinder's domain.
  //! \param[out] vMaxCyl  the V max parameter of the cylinder's domain.
  //! \param[in]  progress the progress notifier.
  //! \param[in]  plotter  the imperative plotter.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    IsCylindrical(const Handle(Geom_Surface)& surface,
                  const double                uMinSurf,
                  const double                uMaxSurf,
                  const double                vMinSurf,
                  const double                vMaxSurf,
                  const double                toler,
                  gp_Cylinder&                cyl,
                  double&                     uMinCyl,
                  double&                     uMaxCyl,
                  double&                     vMinCyl,
                  double&                     vMaxCyl,
                  ActAPI_ProgressEntry        progress = nullptr,
                  ActAPI_PlotterEntry         plotter  = nullptr);

}

#endif
