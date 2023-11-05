//-----------------------------------------------------------------------------
// Created on: 04 November 2023
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

#ifndef asiAlgo_ReapproxContour_HeaderFile
#define asiAlgo_ReapproxContour_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OCCT includes
#include <Geom_Curve.hxx>
#include <gp_Lin.hxx>
#include <NCollection_Sequence.hxx>
#include <TColgp_HSequenceOfPnt.hxx>
#include <TopoDS_Wire.hxx>

#define BarrierAngleDeg 5

//-----------------------------------------------------------------------------

//! Contour re-approximation (healing) is a common tool which is designed to
//! fix the following problems in the input wires:
//!
//! - Too many edges (e.g. a very fine chain of straight line segments);
//! - Self-interactions (overlapping and mutual intersections).
//!
//! In the modeling routines where a polygonal wire is used as an input, contour
//! healing can be necessary to improve performance and quality of the result.
//! Such healing is a three-stage process. It consists of the following phases:
//!
//! - Segmentation,
//! - Re-approximation,
//! - Join.
//!
//! Segmentation is used to break down the contour onto the pieces with
//! distinct principal directions. To do this, the input wire is exploded to
//! edges and discretized with a prescribed deflection. All near-coincident
//! points are removed to get rid of coarse self-interactions. The principal
//! directions are used for monotonic parameterization which allows elimination
//! of loops and small self-interactions at re-approximation stage. Join
//! operation is done in order to assemble a connected wire once all pieces
//! are re-approximated with respect to their principal directions. If the
//! principal directions cannot be detected (e.g. for G1 holes), the contour
//! healing procedure still can be used to eliminate small edges.
class asiAlgo_ReapproxContour : public ActAPI_IAlgorithm
{
public:

  struct Segment
  {
    Segment() : HasPrincipal(false) {}

    bool                          HasPrincipal;
    gp_Lin                        Principal;
    Handle(TColgp_HSequenceOfPnt) Pts;
  };

public:

  //! Constructor.
  //! \param[in] Contour             the input contour to re-approximate.
  //! \param[in] precision           the precision to use.
  //! \param[in] barrierAngleDeg     the barrier angle for segmentation.
  //! \param[in] barrierSegmentRatio the barrier segmentation ratio.
  //! \param[in] progress            the progress notifier.
  //! \param[in] plotter             the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_ReapproxContour(const TopoDS_Shape&  Contour,
                            const double         precision,
                            const double         barrierAngleDeg     = BarrierAngleDeg,
                            const bool           barrierSegmentRatio = false,
                            ActAPI_ProgressEntry progress            = nullptr,
                            ActAPI_PlotterEntry  plotter             = nullptr);

public:

  //! \return center point of the contour.
  const gp_Pnt& Center() const
  {
    return m_center;
  }

  //! \return contour orientation vector.
  const gp_Vec& Orientation() const
  {
    return m_ori;
  }

public:

  //! Performs actual re-approximation.
  //! \param[out] Wire                the resulting wire (re-approximated contour).
  //! \param[in]  useSegments         the Boolean flag that indicates whether to use segmentation
  //!                                 of wire by edges.
  //! \param[in]  useAccumulatedAngle the Boolean flag that indicates whether to use accumulated
  //!                                 angle for segmentation.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    operator()(TopoDS_Wire& Wire,
               const bool   useSegments,
               const bool   useAccumulatedAngle);

protected:

  //! Performs segmentation of the original contour onto point sets with
  //! associated principal directions.
  //! \param[out] Segments            the segmentation result.
  //! \param[in]  useSegments         the Boolean flag that indicates whether to use
  //!                                 segmentation of wire by edges.
  //! \param[in]  useAccumulatedAngle the Boolean flag that indicates whether to use
  //!                                 accumulated angle for segmentation.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    doSegmentation(NCollection_Sequence<Segment>& Segments,
                   const bool                     useSegments,
                   const bool                     useAccumulatedAngle);

  //! Approximates the extracted segments using projection to principals for
  //! parameterization.
  //! \param[in]  Segments the point sets to approximate.
  //! \param[out] Curves   the approximation results (one-to-one for each segment).
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    doApprox(const NCollection_Sequence<Segment>&      Segments,
             NCollection_Sequence<Handle(Geom_Curve)>& Curves);

  //! Prepares a wire for the given collection of curves.
  //! \param[in]  Curves the curves to make edges and subsequently a wire from.
  //! \param[out] W      the resulting wire.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    doWire(const NCollection_Sequence<Handle(Geom_Curve)>& Curves,
           TopoDS_Wire&                                    W) const;

private:

  //! Makes the passed contour coarser in a sense that it looses the points whose
  //! distances from each other are too small with respect to the given chord
  //! length.
  //! \param[in]  source     the source points.
  //! \param[in]  resolution the max allowed chord length.
  //! \param[out] result     the resulting points.
  asiAlgo_EXPORT void
    makeCoarser(const TColgp_SequenceOfPnt& source,
                const double                resolution,
                TColgp_SequenceOfPnt&       result) const;

private:

  TopoDS_Shape m_contour;       //!< Original contour.
  double       m_fPrec;         //!< Precision.
  double       m_fBarrierAngle; //!< Barrier angle for segmentation.
  gp_Pnt       m_center;        //!< Center point of the contour.
  gp_Vec       m_ori;           //!< Orientation vector.
  bool         m_bClosed;       //!< Indicates whether the contour is closed.
  bool         m_bSegmentRatio; //!< Take into account ratio between neighbor segments when decide to join them into a single curve.

};

#endif
