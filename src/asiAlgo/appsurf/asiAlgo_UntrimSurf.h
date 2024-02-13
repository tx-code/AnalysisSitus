//-----------------------------------------------------------------------------
// Created on: 17 May 2023
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

#ifndef asiAlgo_UntrimSurf_h
#define asiAlgo_UntrimSurf_h

// asiAlgo includes
#include <asiAlgo.h>

// ActAPI includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_HSequenceOfShape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! UNTRIM operator for turning a rectangular trimmed surface into a
//! surface in natural bounds.
class asiAlgo_UntrimSurf : public ActAPI_IAlgorithm
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_UntrimSurf, ActAPI_IAlgorithm)

public:

  //! Default ctor.
  //! \param[in] progress the progress indicator.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_UntrimSurf(ActAPI_ProgressEntry progress = nullptr,
                       ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Performs untrimming for the given B-surface.
  //! \param[in]  faces   the B-spline faces to untrim.
  //! \param[in]  edges   the collection of constraint edges.
  //! \param[out] support the constructed B-surface.
  //! \param[out] face    the constructed face.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const Handle(TopTools_HSequenceOfShape)& faces,
          const Handle(TopTools_HSequenceOfShape)& edges,
          Handle(Geom_BSplineSurface)&             support,
          TopoDS_Face&                             face);

public:

  //! Sets the number of U isolines for the constructed surface.
  //! \param[in] num the value to set.
  void SetNumUIsos(const int num)
  {
    m_iNumUIsos = num;
  }

  //! Sets the number of V isolines for the constructed surface.
  //! \param[in] num the value to set.
  void SetNumVIsos(const int num)
  {
    m_iNumVIsos = num;
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

  //! \return max achieved error.
  double GetMaxError() const
  {
    return m_fMaxError;
  }

  //! \return the constructed guide edges.
  const std::vector<TopoDS_Edge>& GetGuides() const
  {
    return m_guides;
  }

  //! \return the constructed profile edges.
  const std::vector<TopoDS_Edge>& GetProfiles() const
  {
    return m_profiles;
  }

protected:

  //! Finds the rail curves among the given collection of edges
  //! to prepare for Coons surface construction.
  asiAlgo_EXPORT bool
    sortEdges(const Handle(TopTools_HSequenceOfShape)& edges,
              Handle(Geom_BSplineCurve)&               c0,
              Handle(Geom_BSplineCurve)&               c1,
              Handle(Geom_BSplineCurve)&               b0,
              Handle(Geom_BSplineCurve)&               b1) const;

  //! Reapproximates constraint curves to make them more suitable
  //! for Coons fitting.
  asiAlgo_EXPORT bool
    reapproxCurves(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                   std::vector<Handle(Geom_BSplineCurve)>&       result) const;

protected:

  int    m_iNumUIsos; //!< Number of U isolines.
  int    m_iNumVIsos; //!< Number of V isolines.
  int    m_iDegU;     //!< U degree.
  int    m_iDegV;     //!< V degree.
  double m_fMaxError; //!< Max computed deviation of the untrimmed surface from the initial one.

  std::vector<TopoDS_Edge> m_guides;   //!< Guide edges.
  std::vector<TopoDS_Edge> m_profiles; //!< Profile edges.

};

#endif
