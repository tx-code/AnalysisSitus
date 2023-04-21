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

// OCCT includes
#include <Geom_BSplineSurface.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_HSequenceOfShape.hxx>

// Mobius includes
#include <mobius/core_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! Runs APPSURF2 surface fitting algorithm.
class asiAlgo_AppSurf2 : public mobius::core_IAlgorithm
{
public:

  //! Default ctor.
  //! \param[in] progress the progress indicator.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_AppSurf2(mobius::core_ProgressEntry progress = nullptr,
                     mobius::core_PlotterEntry  plotter  = nullptr);

public:

  //! Set the passed extra points as supplementary pinpoint constraints.
  //! \param[in] points the point cloud to set.
  asiAlgo_EXPORT void
    SetExtraPoints(const Handle(asiAlgo_BaseCloud<double>)& points);

  //! Builds B-surface.
  //! \param[in]  edges   the collection of constraint edges.
  //! \param[out] support the constructed B-surface.
  //! \param[out] face    the constructed face.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const Handle(TopTools_HSequenceOfShape)& edges,
          Handle(Geom_BSplineSurface)&             support,
          TopoDS_Face&                             face);

public:

  //! Sets initial surface.
  //! \param[in] initSurf the initial surface to set.
  void SetInitSurf(const Handle(Geom_Surface)& initSurf)
  {
    m_initSurf = initSurf;
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

  //! \return all used pinpoint constraints.
  const Handle(asiAlgo_BaseCloud<double>)& GetConstraints() const
  {
    return m_pinPts;
  }

  //! Sets the number of U knots for the initial plane.
  //! \param[in] num the value to set.
  void SetNumUKnots(const int num)
  {
    m_iNumUKnots = num;
  }

  //! Sets the number of V knots for the initial plane.
  //! \param[in] num the value to set.
  void SetNumVKnots(const int num)
  {
    m_iNumVKnots = num;
  }

  //! Sets the U degree for the initial plane.
  //! \param[in] num the value to set.
  void SetDegreeU(const int num)
  {
    m_iDegU = num;
  }

  //! Sets the V degree for the initial plane.
  //! \param[in] num the value to set.
  void SetDegreeV(const int num)
  {
    m_iDegV = num;
  }

  //! Sets the edge discretization precision in model units.
  //! \param[in] prec the precision to set.
  void SetEdgeDiscrPrec(const double prec)
  {
    m_fEdgeDiscrPrec = prec;
  }

protected:

  double                            m_fEdgeDiscrPrec; //!< Number of discretization points.
  double                            m_fFairCoeff;     //!< Optional fairing coefficient.
  Handle(asiAlgo_BaseCloud<double>) m_extraPts;       //!< Extra pinpoint constraints.
  Handle(asiAlgo_BaseCloud<double>) m_pinPts;         //!< All pinpoint constraints.
  Handle(Geom_Surface)              m_initSurf;       //!< Initial surface.
  int                               m_iNumUKnots;     //!< Number of U knots.
  int                               m_iNumVKnots;     //!< Number of V knots.
  int                               m_iDegU;          //!< U degree.
  int                               m_iDegV;          //!< V degree.

};

#endif
