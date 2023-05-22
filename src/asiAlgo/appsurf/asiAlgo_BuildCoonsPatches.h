//-----------------------------------------------------------------------------
// Created on: 15 May 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Elizaveta Krylova
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

#ifndef asiAlgo_BuildCoonsSurfPatches_h
#define asiAlgo_BuildCoonsSurfPatches_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <math_Matrix.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
//-----------------------------------------------------------------------------

//! Utility to build Coons (4-sided) surfaces in form of B-surfaces.
class asiAlgo_BuildCoonsPatches : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BuildCoonsPatches, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] c0       curve "on the left".
  //! \param[in] c1       curve "on the right".
  //! \param[in] b0       curve "on the bottom".
  //! \param[in] b1       curve "on the top".
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BuildCoonsPatches(ActAPI_ProgressEntry progress = nullptr,
                              ActAPI_PlotterEntry  plotter  = nullptr);

public:
  //! Builds Patches.
  //! \param[in]  uEdges       the collection of boundary edges in the U direction.
  //! \param[in]  vEdges       the collection of boundary edges in the V direction.
  //! \param[out] profileEdges the constructed trimmed edges.
  //! \param[out] guidesEdges  the constructed trimmed edges.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    asiAlgo_BuildCoonsPatches::Build(const std::vector<TopoDS_Edge>& profiles,
                                     const std::vector<TopoDS_Edge>& guides,
                                     std::vector<TopoDS_Edge>&       profileEdges,
                                     std::vector<TopoDS_Edge>&       guidesEdges);

  //! Reapproximates the passed curves to ensure they all have identical
  //! parameterization.
  //! \param[in]  curves the curves to reapproximate.
  //! \param[out] result the reapproximated curves.
  //! \param[out] params the chosen parameterization for points.
  //! \param[out] knots  the chosen knot vector.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
  asiAlgo_BuildCoonsPatches::reapproxCurves(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                                            std::vector<Handle(Geom_BSplineCurve)>&       result,
                                            std::vector<double>&                          params,
                                            std::vector<double>&                          knots) const;

  //! trim curves using points.
  //! \param[in]  uCurves        profiles to trim.
  //! \param[in]  vCurves        guides to trim.
  //! \param[in]  uParams        u params for trimming uCurves in the correct parts.
  //! \param[in]  vParams        v params for trimming vCurves in the correct parts.
  //! \param[out] uTrimmedCurves trimmed profiles.
  //! \param[out] vTrimmedCurves trimmed guides.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    computeCurvesFromIntersections(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
                                   const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
                                   const math_Matrix&                            uParams,
                                   const math_Matrix&                            vParams,
                                   std::vector<Handle(Geom_Curve)>&              uTrimmedCurves,
                                   std::vector<Handle(Geom_Curve)>&              vTrimmedCurves) const;
};

#endif
