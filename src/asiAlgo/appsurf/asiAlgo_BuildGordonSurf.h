//-----------------------------------------------------------------------------
// Created on: 29 March 2023
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

#ifndef asiAlgo_BuildGordonSurf_h
#define asiAlgo_BuildGordonSurf_h

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// Mobius includes
#include <mobius/core_XYZ.h>

// OCCT includes
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <math_Matrix.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

//-----------------------------------------------------------------------------

//! Builds Gordon surface.
class asiAlgo_BuildGordonSurf : public ActAPI_IAlgorithm
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BuildGordonSurf, ActAPI_IAlgorithm)

public:

  enum Status
  {
    Status_Ok                              = 0x0000,
    Status_InconsistentOrientationOfCurves = 0x0001
  };

public:

  //! Checks deviation between the constructed surfaces and the initial
  //! curve network.
  //! \param[in]  surf     the surface of interest.
  //! \param[in]  uEdges   the U edges of the curve network.
  //! \param[in]  vEdges   the V edges of the curve network.
  //! \param[out] bndDev   the computed deviation along the boundary curves.
  //! \param[out] innerDev the computed deviation along the inner curves.
  //! \param[out] maxDev   the computed max deviation.
  //! \param[in]  plotter  the plotter entry.
  asiAlgo_EXPORT static void
    CheckDeviation(const Handle(Geom_BSplineSurface)& surf,
                   const std::vector<TopoDS_Edge>&    uEdges,
                   const std::vector<TopoDS_Edge>&    vEdges,
                   double&                            bndDev,
                   double&                            innerDev,
                   double&                            maxDev,
                   ActAPI_PlotterEntry                plotter);

public:

  //! Ctor.
  //! \param[in] progress the progress indicator.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BuildGordonSurf(ActAPI_ProgressEntry progress = nullptr,
                            ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Builds Gordon surface as a B-spline surface.
  //! \param[in]  uEdges  the collection of boundary edges in the U direction.
  //! \param[in]  vEdges  the collection of boundary edges in the V direction.
  //! \param[out] support the constructed B-surface.
  //! \param[out] face    the constructed face.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const std::vector<TopoDS_Edge>& uEdges,
          const std::vector<TopoDS_Edge>& vEdges,
          Handle(Geom_BSplineSurface)&    support,
          TopoDS_Face&                    face);

public:

  //! \return max achieved error.
  double GetMaxError() const
  {
    return m_fMaxError;
  }

protected:

  //! Reapproximates the passed curves to ensure they all have identical
  //! parameterization.
  //! \param[in]  curves the curves to reapproximate.
  //! \param[out] result the reapproximated curves.
  //! \param[out] params the chosen parameterization for points.
  //! \param[out] knots  the chosen knot vector.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    reapproxCurves(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                   std::vector<Handle(Geom_BSplineCurve)>&       result,
                   std::vector<double>&                          params,
                   std::vector<double>&                          knots) const;

  //! Computes intersection parameters between curves.
  asiAlgo_EXPORT bool
    computeCurveIntersections(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
                              const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
                              math_Matrix&                                  uParams,
                              math_Matrix&                                  vParams) const;

protected:

  double m_fMaxError; //!< Max achieved approximation error.

};

#endif
