//-----------------------------------------------------------------------------
// Created on: 03 August 2021
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

#ifndef asiAlgo_BuildConvexHull_h
#define asiAlgo_BuildConvexHull_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <gp_Pnt.hxx>
#include <Poly_Triangulation.hxx>

// STL includes
#include <vector>

//! \brief Constructs convex hull on the given set of points.
class asiAlgo_BuildConvexHull : public ActAPI_IAlgorithm
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BuildConvexHull, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] notifier progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_BuildConvexHull(ActAPI_ProgressEntry notifier = nullptr,
                          ActAPI_PlotterEntry  plotter  = nullptr)
  : ActAPI_IAlgorithm(notifier, plotter)
  {}

public:

  //! Constructs convex hull for the given shape.
  //! \param[in]  shape the target shape.
  //! \param[out] hull  the resulting convex hull.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const TopoDS_Shape&         shape,
            Handle(Poly_Triangulation)& hull,
            const bool                  forceTriangulate = false);

  //! Constructs convex hull.
  //! \param[in]  data the input vector of points, data is stored in x0, y0, z0, x1, y1, z1 order.
  //! \param[out] hull the resulting convex hull.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const std::vector<double>&  data,
            Handle(Poly_Triangulation)& hull);

  //! Constructs convex hull.
  //! \param[in]  data the input vector of points.
  //! \param[out] hull the resulting convex hull.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const std::vector<gp_Pnt>&  data,
            Handle(Poly_Triangulation)& hull);

  //! Constructs convex hull.
  //! \param[in]  data the input vector of points.
  //! \param[out] hull the resulting convex hull.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const Handle(TColgp_HArray1OfPnt)& data,
            Handle(Poly_Triangulation)&        hull);

};

#endif
