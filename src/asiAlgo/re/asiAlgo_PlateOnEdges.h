//-----------------------------------------------------------------------------
// Created on: 09 March 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef asiAlgo_PlateOnEdges_h
#define asiAlgo_PlateOnEdges_h

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Geom_BSplineSurface.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_HSequenceOfShape.hxx>

class GeomPlate_BuildPlateSurface;

//-----------------------------------------------------------------------------

//! Utility to build a plate surface on the given edge set.
class asiAlgo_PlateOnEdges : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_PlateOnEdges, ActAPI_IAlgorithm)

public:

  //! Default ctor.
  //! \param[in] progress progress indicator.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_PlateOnEdges(ActAPI_ProgressEntry progress = nullptr,
                         ActAPI_PlotterEntry  plotter  = nullptr);

  //! Constructs the tool initializing it with AAG.
  //! \param[in] aag      Attributed Adjacency Graph (AAG).
  //! \param[in] progress progress indicator.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_PlateOnEdges(const Handle(asiAlgo_AAG)& aag,
                         ActAPI_ProgressEntry       progress = nullptr,
                         ActAPI_PlotterEntry        plotter  = nullptr);

  //! Constructs the tool initializing it with the B-Rep shape in question.
  //! \param[in] shape    shape in question.
  //! \param[in] progress progress indicator.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_PlateOnEdges(const TopoDS_Shape&  shape,
                         ActAPI_ProgressEntry progress = nullptr,
                         ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Set the passed extra points as supplementary pinpoint constraints.
  //! \param[in] points the point cloud to set.
  asiAlgo_EXPORT void
    SetExtraPoints(const Handle(asiAlgo_BaseCloud<double>)& points);

  //! Constructs TPS (Thin Plate Spline) approximation for the passed edges.
  //! \param[in,out] edges       collection of edges. The edges can be
  //!                            modified on connecting to wires.
  //! \param[in]     continuity  desired order of cross-boundary continuity.
  //! \param[out]    support     reconstructed support b-surface.
  //! \param[out]    result      reconstructed face.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(Handle(TopTools_HSequenceOfShape)& edges,
          const unsigned int                 continuity,
          Handle(Geom_BSplineSurface)&       support,
          TopoDS_Face&                       result);

  //! Constructs TPS (Thin Plate Spline) approximation for the passed edges.
  //! \param[in]  edgeIndices edges to build the approximation surface for.
  //! \param[in]  continuity  desired order of cross-boundary continuity.
  //! \param[out] edges       collection of edges corresponding to the passed
  //!                         indices.
  //! \param[out] support     reconstructed support b-surface.
  //! \param[out] result      reconstructed face.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const TColStd_PackedMapOfInteger&  edgeIndices,
          const unsigned int                 continuity,
          Handle(TopTools_HSequenceOfShape)& edges,
          Handle(Geom_BSplineSurface)&       support,
          TopoDS_Face&                       result);

  //! Constructs TPS (Thin Plate Spline) approximation for the passed edges.
  //! \param[in]  edgeIndices edges to build the approximation surface for.
  //! \param[in]  continuity  desired order of cross-boundary continuity.
  //! \param[out] support     reconstructed support b-surface.
  //! \param[out] result      reconstructed face.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Build(const TColStd_PackedMapOfInteger&  edgeIndices,
          const unsigned int                 continuity,
          Handle(Geom_BSplineSurface)&       support,
          TopoDS_Face&                       result);

  //! Builds host b-surface.
  //! \param[in]  edgeIndices edges to build the approximation surface for.
  //! \param[in]  continuity  desired order of cross-boundary continuity.
  //! \param[out] edges       collection of edges corresponding to the passed
  //!                         indices.
  //! \param[out] support     reconstructed support b-surface.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    BuildSurf(const TColStd_PackedMapOfInteger&  edgeIndices,
              const unsigned int                 continuity,
              Handle(TopTools_HSequenceOfShape)& edges,
              Handle(Geom_BSplineSurface)&       support);

  //! Builds host b-surface.
  //! \param[in]  edges      collection of edges.
  //! \param[in]  continuity desired order of cross-boundary continuity.
  //! \param[out] support    reconstructed support b-surface.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    BuildSurf(const Handle(TopTools_HSequenceOfShape)& edges,
              const unsigned int                       continuity,
              Handle(Geom_BSplineSurface)&             support);

  //! Builds face on the passed edges and host surface.
  //! \param[in]  edges   collection of edges.
  //! \param[in]  support support b-surface.
  //! \param[out] result  reconstructed face.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    BuildFace(Handle(TopTools_HSequenceOfShape)& edges,
              const Handle(Geom_BSplineSurface)& support,
              TopoDS_Face&                       result);

public:

  //! Sets initial surface.
  //! \param[in] initSurf the initial surface to set.
  void SetInitSurf(const Handle(Geom_Surface)& initSurf)
  {
    m_initSurf = initSurf;
  }

  //! Sets fairing coefficient.
  //! \param[in] fairCoeff coefficient to set.
  void SetFairingCoeff(const double fairCoeff)
  {
    m_fFairCoeff = fairCoeff;
  }

  //! \return fairing coefficient.
  double GetFairingCoeff() const
  {
    return m_fFairCoeff;
  }

  //! \return all used pinpoint constraints.
  const Handle(asiAlgo_BaseCloud<double>)& GetConstraints() const
  {
    return m_pinPts;
  }

  //! Sets the edge discretization precision in model units.
  //! \param[in] prec the precision to set.
  void SetEdgeDiscrPrec(const double prec)
  {
    m_fEdgeDiscrPrec = prec;
  }

protected:

  //! Adds pinpoint constraints to the given plate builder.
  asiAlgo_EXPORT void
    fillConstraints(const Handle(TopTools_HSequenceOfShape)& edges,
                    const unsigned int                       continuity,
                    GeomPlate_BuildPlateSurface&             builder);

protected:

  TopoDS_Shape                      m_shape;          //!< Working shape.
  Handle(asiAlgo_AAG)               m_aag;            //!< AAG.
  double                            m_fEdgeDiscrPrec; //!< Number of discretization points.
  double                            m_fFairCoeff;     //!< Optional fairing coefficient.
  Handle(asiAlgo_BaseCloud<double>) m_extraPts;       //!< Extra pinpoint constraints.
  Handle(asiAlgo_BaseCloud<double>) m_pinPts;         //!< All pinpoint constraints.
  Handle(Geom_Surface)              m_initSurf;       //!< Initial surface.

};

#endif
