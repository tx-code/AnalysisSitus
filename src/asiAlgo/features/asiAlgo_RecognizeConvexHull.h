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

#pragma once

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_Recognizer.h>
#include <asiAlgo_RTCD.h>

class TopoDS_Face;

//-----------------------------------------------------------------------------

//! Recognizes all faces lying on a convex hull of the CAD part.
class asiAlgo_RecognizeConvexHull : public asiAlgo_Recognizer
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_RecognizeConvexHull, asiAlgo_Recognizer)

public:

  //! Ctor with a shape.
  //! \param[in] shape    the shape to recognize.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_RecognizeConvexHull(const TopoDS_Shape&  shape,
                                ActAPI_ProgressEntry progress = nullptr,
                                ActAPI_PlotterEntry  plotter  = nullptr);

  //! Ctor with AAG.
  //! \param[in] aag      the AAG instance for the shape to recognize.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_RecognizeConvexHull(const Handle(asiAlgo_AAG)& aag,
                                ActAPI_ProgressEntry       progress = nullptr,
                                ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Sets the resolution of a Cartesian grid that will be constructed
  //! in the parametric spaces of faces.
  //! \param[in] numSlices the number of slices along U and V axes.
  asiAlgo_EXPORT void
    SetGridResolution(const int numSlices);

  //! \return the grid resolution.
  asiAlgo_EXPORT int
    GetGridResolution() const;

  //! Sets the tolerance for point-on-surface classification.
  //! \param[in] tol the tolerance to set.
  asiAlgo_EXPORT void
    SetTolerance(const double tol);

  //! \return point-on-surface classification tolerance.
  asiAlgo_EXPORT double
    GetTolerance() const;

  //! Sets the Boolean flag indicating whether to use Eric Haines' algorithm
  //! for PMC tests instead of the good old OpenCascade's classifier.
  //! \param[in] on the Boolean value to set.
  asiAlgo_EXPORT void
    SetUseHaines(const bool on);

  //! \return true if the Haines' classification mode is enabled.
  asiAlgo_EXPORT bool
    GetUseHaines() const;

  //! Sets the Boolean flag indicating whether to store the result of face sampling
  //! in the dedicated attribute of AAG.
  //! \param[in] on the Boolean value to set.
  asiAlgo_EXPORT void
    SetCacheSampling(const bool on);

  //! \return whether the result of sampling should be cached.
  asiAlgo_EXPORT bool
    GetCacheSampling() const;

  //! \return the constructed hull.
  asiAlgo_EXPORT const Handle(Poly_Triangulation)&
    GetHullMesh() const;

  //! Returns the computed convex hull as a collection of planes
  //! in the format of `RTCD` ("Real-time collision detection")
  //! namespace. This representation is useful for computations
  //! based on halfspaces.
  asiAlgo_EXPORT void
    GetHullPlanes(std::vector<RTCD::Plane>& planes) const;

public:

  //! Performs recognition.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

protected:

  void addFeaturePts(const TopoDS_Face&                       face,
                     const Handle(asiAlgo_BaseCloud<double>)& pts) const;

protected:

  //! Cartesian grid resolution.
  int m_iGridPts;

  //! Tolerance for checking if a point is on a surface.
  double m_fToler;

  //! Alternative classification approach.
  bool m_bHaines;

  //! Flag whether to keep the result of sampling.
  bool m_bCacheSampl;

  //! Constructed hull as a mesh.
  Handle(Poly_Triangulation) m_hullMesh;
};

