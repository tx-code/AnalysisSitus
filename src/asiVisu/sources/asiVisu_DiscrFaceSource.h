//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

#ifndef asiVisu_DiscrFaceSource_h
#define asiVisu_DiscrFaceSource_h

// asiVisu includes
#include <asiVisu.h>
#include <asiVisu_ShapeData.h>

// asiAlgo includes
#include <asiAlgo_DiscrModel.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkType.h>
#include <vtkPolyDataAlgorithm.h>

//-----------------------------------------------------------------------------

//! Source for a two-dimensional grid.
class asiVisu_DiscrFaceSource : public vtkPolyDataAlgorithm
{
public:

  vtkTypeMacro(asiVisu_DiscrFaceSource, vtkPolyDataAlgorithm)

  asiVisu_EXPORT static asiVisu_DiscrFaceSource*
    New();

// Kernel:
public:

  //! Sets disrete model.
  //! \param[in] model discrete model to set.
  asiVisu_EXPORT void
    SetDiscrModel(const Handle(asiAlgo::discr::Model)& model);

  //! \return discrete model.
  asiVisu_EXPORT const Handle(asiAlgo::discr::Model)&
    GetDiscrModel() const;

  //! Sets faceID.
  //! \param[in] faceIDs IDs of faces.
  asiVisu_EXPORT void
    SetFaceIDs(const TColStd_PackedMapOfInteger& faceID);

  //! \return faceID.
  asiVisu_EXPORT const TColStd_PackedMapOfInteger&
    GetFaceIDs() const;

public:

  //! Initializes source with diagnostic tools: progress notifier and
  //! imperative plotter.
  //! \param progress [in] progress notifier.
  //! \param plotter  [in] imperative plotter.
  void SetDiagnosticTools(ActAPI_ProgressEntry progress,
                          ActAPI_PlotterEntry  plotter)
  {
    m_progress = progress;
    m_plotter  = plotter;
  }

protected:

  //! This method (called by superclass) performs conversion of our native
  //! data structures to VTK polygonal data.
  //!
  //! \param[in]  request      describes "what" algorithm should do. This is
  //!                          typically just one key such as REQUEST_INFORMATION.
  //! \param[in]  inputVector  inputs of the algorithm.
  //! \param[out] outputVector outputs of the algorithm.
  //! \return status.
  asiVisu_EXPORT virtual int
    RequestData(vtkInformation*        request,
                vtkInformationVector** inputVector,
                vtkInformationVector*  outputVector);

private:

  //! \return visualization data.
  Handle(asiVisu_ShapeData) generatePolyLine() const;

private:

  //! Default constructor.
  asiVisu_DiscrFaceSource();

  //! Destructor.
  ~asiVisu_DiscrFaceSource();

private:

  asiVisu_DiscrFaceSource(const asiVisu_DiscrFaceSource&) = delete;
  asiVisu_DiscrFaceSource& operator=(const asiVisu_DiscrFaceSource&) = delete;

private:

  //! Discrete model.
  Handle(asiAlgo::discr::Model) m_discrModel;

  //! FaceIDs.
  TColStd_PackedMapOfInteger    m_faceID;

  //! Progress notifier.
  ActAPI_ProgressEntry          m_progress;

  //! Imperative plotter.
  ActAPI_PlotterEntry           m_plotter;

};

#endif
