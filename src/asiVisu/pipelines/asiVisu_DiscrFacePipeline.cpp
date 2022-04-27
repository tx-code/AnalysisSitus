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

// Own include
#include <asiVisu_DiscrFacePipeline.h>

// asiVisu includes
#include <asiVisu_DiscrFaceDataProvider.h>

// VTK includes
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyData.h>

// OCCT includes
#include <TColStd_HPackedMapOfInteger.hxx>

//-----------------------------------------------------------------------------

asiVisu_DiscrFacePipeline::asiVisu_DiscrFacePipeline()
//
: asiVisu_Pipeline( vtkSmartPointer<vtkDataSetMapper>::New(),
                    vtkSmartPointer<vtkActor>::New() )
{
  // Create source here in order to be able to reuse it in other pipelines.
  m_source = vtkSmartPointer<asiVisu_DiscrFaceSource>::New();
}

//-----------------------------------------------------------------------------

void asiVisu_DiscrFacePipeline::SetInput(const Handle(asiVisu_DataProvider)& DP)
{
  Handle(asiVisu_DiscrFaceDataProvider)
    provider = Handle(asiVisu_DiscrFaceDataProvider)::DownCast(DP);

  /* ===========================
   *  Validate input Parameters.
   * =========================== */

  Handle(asiAlgo::discr::Model) model = provider->GetDiscrModel();
  const TColStd_PackedMapOfInteger faceIDs = provider->GetFaceIDs()->Map();
  //
  if ( model.IsNull() )
  {
    // Pass empty data set in order to have valid pipeline.
    vtkSmartPointer<vtkPolyData> dummyDS = vtkSmartPointer<vtkPolyData>::New();
    this->SetInputData(dummyDS);
    this->Modified(); // Update modification timestamp.
    return; // Do nothing.
  }

  /* ============================
   *  Prepare polygonal data set.
   * ============================ */

  if ( provider->MustExecute( this->GetMTime() ) )
  {
    m_source->SetDiscrModel ( model );
    m_source->SetFaceIDs    ( faceIDs );

    // Initialize pipeline.
    this->SetInputConnection( m_source->GetOutputPort() );
  }

  // Update modification timestamp.
  this->Modified();
}

//-----------------------------------------------------------------------------

void asiVisu_DiscrFacePipeline::callback_add_to_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

void asiVisu_DiscrFacePipeline::callback_remove_from_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

void asiVisu_DiscrFacePipeline::callback_update()
{}
