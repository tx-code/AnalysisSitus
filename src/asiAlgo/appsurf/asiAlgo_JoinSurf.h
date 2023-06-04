//-----------------------------------------------------------------------------
// Created on: 26 May 2023
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

#ifndef asiAlgo_JoinSurf_h
#define asiAlgo_JoinSurf_h

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

//! JOIN operator for concatenating the passed natural-bounded surfaces into
//! a single surface.
class asiAlgo_JoinSurf : public ActAPI_IAlgorithm
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_JoinSurf, ActAPI_IAlgorithm)

public:

  //! Default ctor.
  //! \param[in] progress the progress indicator.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_JoinSurf(ActAPI_ProgressEntry progress = nullptr,
                     ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Joins the passed B-surfaces.
  //! \param[in]  faces   the B-spline faces to join.
  //! \param[out] support the constructed B-surface.
  //! \param[out] face    the constructed face.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const Handle(TopTools_HSequenceOfShape)& faces,
          Handle(Geom_BSplineSurface)&             support,
          TopoDS_Face&                             face);

public:

  //! Sets the number of profile isolines on the 1-st surface.
  //! \param[in] num the value to set.
  void SetNumProfilesS1(const int num)
  {
    m_iNumProfilesS1 = num;
  }

  //! Sets the number of profile isolines on the 2-nd surface.
  //! \param[in] num the value to set.
  void SetNumProfilesS2(const int num)
  {
    m_iNumProfilesS2 = num;
  }

  //! Sets the number of guide curves.
  //! \param[in] num the value to set.
  void SetNumGuides(const int num)
  {
    m_iNumGuides = num;
  }

  //! Sets the boundary offset w.r.t. the common edge between the surfaces
  //! to join.
  //! \param[in] val the value to set.
  void SetBoundaryOffset(const double val)
  {
    m_fBndOffset = val;
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

  int    m_iNumProfilesS1; //!< Number of profile isolines on S1.
  int    m_iNumProfilesS2; //!< Number of profile isolines on S2.
  int    m_iNumGuides;     //!< Number of guide curves.
  double m_fBndOffset;     //!< Boundary offset.
  double m_fMaxError;      //!< Max computed deviation.

  std::vector<TopoDS_Edge> m_guides;   //!< Guide edges.
  std::vector<TopoDS_Edge> m_profiles; //!< Profile edges.

};

#endif
