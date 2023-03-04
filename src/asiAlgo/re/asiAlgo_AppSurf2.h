//-----------------------------------------------------------------------------
// Created on: 04 March 2023
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

#ifndef asiAlgo_AppSurf2_h
#define asiAlgo_AppSurf2_h

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Geom_BSplineSurface.hxx>
#include <TopTools_HSequenceOfShape.hxx>

//-----------------------------------------------------------------------------

//! Runs APPSURF2 surface fitting algorithm.
class asiAlgo_AppSurf2 : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_AppSurf2, ActAPI_IAlgorithm)

public:

  //! Default ctor.
  //! \param[in] progress the progress indicator.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_AppSurf2(ActAPI_ProgressEntry progress = nullptr,
                     ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Set the passed extra points as supplementary pinpoint constraints.
  //! \param[in] points the point cloud to set.
  asiAlgo_EXPORT void
    SetExtraPoints(const Handle(asiAlgo_BaseCloud<double>)& points);

  //! Builds B-surface.
  //! \param[in]  edges   the collection of constraint edges.
  //! \param[out] support the constructed B-surface.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    BuildSurf(const Handle(TopTools_HSequenceOfShape)& edges,
              Handle(Geom_BSplineSurface)&             support);

public:

  //! Sets the number of discretization points used to convert the curve
  //! constraints to the pinpoint constraints.
  //! \param[in] numPts the number of points to set.
  void SetNumDiscrPoints(const int numPts)
  {
    m_iNumDiscrPts = numPts;
  }

  //! \return the used number of discretization points.
  int GetNumDiscrPoints() const
  {
    return m_iNumDiscrPts;
  }

  //! Sets fairing coefficient.
  //! \param[in] fairCoeff the coefficient to set.
  void SetFairingCoeff(const double fairCoeff)
  {
    m_fFairCoeff = fairCoeff;
  }

  //! \return the fairing coefficient.
  double GetFairingCoeff() const
  {
    return m_fFairCoeff;
  }

protected:

  int                               m_iNumDiscrPts; //!< Number of discretization points.
  double                            m_fFairCoeff;   //!< Optional fairing coefficient.
  Handle(asiAlgo_BaseCloud<double>) m_extraPts;     //!< Extra pinpoint constraints.

};

#endif
