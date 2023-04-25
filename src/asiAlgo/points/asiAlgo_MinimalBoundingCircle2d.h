//-----------------------------------------------------------------------------
// Created on: 25 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Andrey Voevodin
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

#ifndef asiAlgo_MinimalBoundingCircle2d_h
#define asiAlgo_MinimalBoundingCircle2d_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <gp_Pnt.hxx>
#include <gp_Circ2d.hxx>

// STL includes
#include <vector>

//! \brief Constructs bounding circle in 2d on the given set of points.
class asiAlgo_MinimalBoundingCircle2d : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_MinimalBoundingCircle2d, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] notifier progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_MinimalBoundingCircle2d(ActAPI_ProgressEntry notifier = nullptr,
                                  ActAPI_PlotterEntry  plotter  = nullptr)
  : ActAPI_IAlgorithm(notifier, plotter)
  {}

public:

  //! Constructs bounding circle.
  //! \param[in]  points   points.
  //! \param[out] circle2d bounding circle.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const std::vector<gp_Pnt2d>& points,
            gp_Circ2d&                   circle2d);

private:

  bool minCircleWithPoint(const std::vector<gp_Pnt2d>& points,
                          const gp_Pnt2d&              point,
                          gp_Circ2d&                   circle);

  bool minCircleWithPoints(const std::vector<gp_Pnt2d>& points,
                           const gp_Pnt2d&              point1,
                           const gp_Pnt2d&              point2,
                           gp_Circ2d&                   circle);

  gp_Circ2d circularOnTwoPoints(const gp_Pnt2d& point1,
                                const gp_Pnt2d& point2);

  bool isFormTriangle(const gp_Pnt2d& point1,
                      const gp_Pnt2d& point2,
                      const gp_Pnt2d& point3);

  double findMaxDist(const gp_Pnt2d& point1,
                     const gp_Pnt2d& point2,
                     const gp_Pnt2d& point3,
                     gp_Pnt2d&       maxPoint1,
                     gp_Pnt2d&       maxPoint2);

  bool circularOnThreePoints(const gp_Pnt2d& point1,
                             const gp_Pnt2d& point2,
                             const gp_Pnt2d& point3,
                             gp_Circ2d&      circle);

  bool isPointInCircle(const gp_Pnt2d&  point,
                       const gp_Circ2d& circle2d);

  void mix(const std::vector<gp_Pnt2d>& points,
           std::vector<gp_Pnt2d>&       mixedPoints);

};

#endif
