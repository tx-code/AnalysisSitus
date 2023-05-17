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
#include <TopoDS_Face.hxx>
#include <TopTools_HSequenceOfShape.hxx>

//-----------------------------------------------------------------------------

//! UNTRIM operator for turning rectangular trimmed surfaces into geometrically
//! close surfaces in natural bounds.
class asiAlgo_UntrimSurf : public ActAPI_IAlgorithm
{
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

protected:

  //! Finds the rail curves among the given collection of edges
  //! to prepare for Coons surface construction.
  asiAlgo_EXPORT bool
    sortEdges(const Handle(TopTools_HSequenceOfShape)& edges,
              Handle(Geom_BSplineCurve)&               c0,
              Handle(Geom_BSplineCurve)&               c1,
              Handle(Geom_BSplineCurve)&               b0,
              Handle(Geom_BSplineCurve)&               b1) const;

protected:

  int m_iNumUKnots; //!< Number of U knots.
  int m_iNumVKnots; //!< Number of V knots.
  int m_iDegU;      //!< U degree.
  int m_iDegV;      //!< V degree.

};

#endif
