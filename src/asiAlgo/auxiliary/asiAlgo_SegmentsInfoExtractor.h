//-----------------------------------------------------------------------------
// Created on: 24 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Kiselev
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

#ifndef asiAlgo_SegmentsInfoExtractor_h
#define asiAlgo_SegmentsInfoExtractor_h

// asiAlgo include
#include <asiAlgo.h>
#include <asiAlgo_SegmentsInfo.h>

// OCCT includes
#include <Geom_Curve.hxx>
#include <gp_Pln.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>

// STL includes
#include <vector>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Class to extract segments information from the passed wire.
class asiAlgo_SegmentsInfoExtractor
{
public:

  //! Constructor.
  //! Extracts information about segments (edges) from the passed wire.
  //! \param[in] wire   the wire to extract segments information from.
  //! \param[in] normal the normal for the plane which contains the passed wire.
  asiAlgo_EXPORT
    asiAlgo_SegmentsInfoExtractor(const TopoDS_Wire& wire,
                                  const gp_Dir&      normal);

  //! Constructor.
  //! Extracts information about segments (edges) from the passed wire.
  //! Computes the normal by itself using points from the wire.
  //!
  //! \param[in] wire the wire to extract segments information from.
  asiAlgo_EXPORT
    asiAlgo_SegmentsInfoExtractor(const TopoDS_Wire& wire);

  //! Destructor.
  asiAlgo_EXPORT
    ~asiAlgo_SegmentsInfoExtractor();

  //! Returns resulting vector of segments infomation data structures.
  asiAlgo_EXPORT
    const asiAlgo_SegmentsInfoVec& Result() { return m_infoVec; }

private:

  double angleToDegree(const double& angle);

  double angleTo(const TopoDS_Edge& edge,
                  const TopoDS_Edge& nextEdge,
                  const gp_Dir&      normal);

  bool curveDirection(const Handle(Geom_Curve)& curve,
                      const gp_Pnt&             commonPoint,
                      const gp_Pnt&             firstPoint,
                      const gp_Pnt&             secondPoint,
                      gp_Dir&                   curveDir);

  void curveAngularity(const Handle(Geom_Curve)& curve,
                        const double              U1,
                        const double              U2,
                        double&                   edgeRadius,
                        double&                   edgeAngle);

  void angularity(const TopoDS_Edge& edge,
                  double&            edgeRadius,
                  double&            edgeAngle);

private:

  asiAlgo_SegmentsInfoExtractor();

private:

  asiAlgo_SegmentsInfoVec m_infoVec; //!< Resulting vector of segments information.
  double                  m_tol;     //!< Machinery precision.
};

#endif
