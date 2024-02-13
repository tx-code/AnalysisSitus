//-----------------------------------------------------------------------------
// Created on: 17 April 2023
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

#ifndef asiAlgo_MeshSlice_h
#define asiAlgo_MeshSlice_h

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <gp_Ax1.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Slices the given CAD/mesh model along the specified direction by the
//! passed number of planes.
class asiAlgo_MeshSlice : public ActAPI_IAlgorithm
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_MeshSlice, ActAPI_IAlgorithm)

public:

  //! Ctor accepting a shape.
  //! \param[in] shape    the shape to slice.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_MeshSlice(const TopoDS_Shape&  shape,
                      ActAPI_ProgressEntry progress,
                      ActAPI_PlotterEntry  plotter);

  //! Ctor accepting a triangulation to slice.
  //! \param[in] tris     the triangulation to slice.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_MeshSlice(const Handle(Poly_Triangulation)& tris,
                      ActAPI_ProgressEntry              progress,
                      ActAPI_PlotterEntry               plotter);

public:

  //! Slices the input model along the specified axis by the given
  //! number of planes.
  //! \param[in] numSlices the number of slices to use.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const int numSlices);

public:

  //! Sets the axis along which slicing should be done.
  //! \param[in] axis the axis to set.
  void SetAxis(const gp_Ax1& axis)
  {
    m_axis = axis;
  }

  //! \return the constructed slicing faces.
  const std::vector<Handle(TopTools_HSequenceOfShape)>&
    GetResultFaces() const
  {
    return m_faces;
  }

  //! Sets the maximization mode.
  void SetMaximize(const bool on)
  {
    m_bMaximize = on;
  }

  //! Gets min parameter value.
  //! \return min parameter value.
  double GetParamMin() const { return m_tMin; }

  //! Gets max parameter value.
  //! \return max parameter value.
  double GetParamMax() const { return m_tMax; }

protected:

  //! Input shape.
  TopoDS_Shape m_shape;

  //! Slicing faces.
  std::vector<Handle(TopTools_HSequenceOfShape)> m_faces;

  //! Optional axis.
  tl::optional<gp_Ax1> m_axis;

  //! Boolean flag indicating whether to maximize edges.
  bool m_bMaximize;

  //! Min parameter.
  double m_tMin;

  //! Max parameter.
  double m_tMax;
};

#endif
