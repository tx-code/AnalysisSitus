//-----------------------------------------------------------------------------
// Created on: 01 August 2021
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

#ifndef asiAlgo_SampleFace_h
#define asiAlgo_SampleFace_h

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_DiscrClassifier2d.h>
#include <asiAlgo_DiscrModel.h>
#include <asiAlgo_Membership.h>
#include <asiAlgo_UniformGrid.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <IntTools_FClass2d.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>

//-----------------------------------------------------------------------------

//! Samples the UV domain of a face by overlaying a uniform grid.
class asiAlgo_SampleFace : public ActAPI_IAlgorithm
{
public:

  //! PMC algorithm to use.
  enum PmcAlgo
  {
    PmcAlgo_Precise, //!< Precise OpenCascade-based classifier.
    PmcAlgo_Haines,  //!< Discrete Haines algorithm.
    PmcAlgo_Discrete //!< Discrete row-based algorithm.
  };

public:

  //! Converts the passed wire of the given face to a polygon.
  asiAlgo_EXPORT static bool
    Wire2Polygon(const TopoDS_Wire&  wire,
                 const TopoDS_Face&  face,
                 std::vector<gp_XY>& polygon);

public:

  //! Ctor.
  //! \param[in] face     the face in question.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_SampleFace(const TopoDS_Face&   face,
                       ActAPI_ProgressEntry progress = nullptr,
                       ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Sets a Boolean flag indicating whether the planar decomposition
  //! we are constructing should be a square.
  //! \param[in] square the Boolean value to set.
  asiAlgo_EXPORT void
    SetSquare(const bool square);

  //! Sets the PMC algorithm to use.
  //! \param[in] algo the algorithm to set.
  asiAlgo_EXPORT void
    SetPmcAlgo(const PmcAlgo algo);

  //! Performs face sampling.
  //! \param[in] numBins the number of discretization steps.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const int numBins);

  //! \return resulting grid.
  asiAlgo_EXPORT const Handle(asiAlgo_UniformGrid<float>)&
    GetResult() const;

  //! \return the sampled points in the modeling space.
  asiAlgo_EXPORT Handle(asiAlgo_BaseCloud<double>)
    GetResult3d() const;

  //! \return discrete model.
  asiAlgo_EXPORT const Handle(asiAlgo::discr::Model)&
    GetDiscrModel() const;

protected:

  //! Performs point-face classification.
  //! \param[in] PonS the point to classify.
  //! \return classification result.
  asiAlgo_Membership
    classify(const gp_Pnt2d& PonS);

protected:

  //! Indicates whether the uniform grid we construct should be a square.
  bool m_bSquare;

  //! Classification algorithm.
  PmcAlgo m_algo;

  //! Face to sample.
  TopoDS_Face m_face;

  //! Parametric bounds of the sampled face.
  double m_fUmin, m_fUmax, m_fVmin, m_fVmax;

  //! Precise classifier.
  IntTools_FClass2d m_class;

  //! Discrete classifier.
  Handle(asiAlgo::discr::Classifier2d) m_discrClass;

  //! Uniform grid which is the result of the uniform sampling.
  Handle(asiAlgo_UniformGrid<float>) m_grid;

  //! Discretized polygon.
  std::vector<gp_XY> m_polygon;

  //! Discrete model.
  Handle(asiAlgo::discr::Model) m_discrModel;

};

#endif
