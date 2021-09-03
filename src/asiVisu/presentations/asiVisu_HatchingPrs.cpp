//-----------------------------------------------------------------------------
// Created on: 03 September 2021
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

// Own include
#include <asiVisu_HatchingPrs.h>

// asiVisu includes
#include <asiVisu_HatchingDataProvider.h>
#include <asiVisu_HatchingPipeline.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

//! Creates a Presentation object for the passed Hatching Node.
//! \param N [in] Face Contour Node to create a Presentation for.
asiVisu_HatchingPrs::asiVisu_HatchingPrs(const Handle(ActAPI_INode)& N)
: asiVisu_Prs(N)
{
  Handle(asiData_HatchingNode)
    hatching_n = Handle(asiData_HatchingNode)::DownCast(N);

  // Create Data Provider for face hatching
  Handle(asiVisu_HatchingDataProvider)
    dp = new asiVisu_HatchingDataProvider(hatching_n);

  // Pipeline for face hatching
  Handle(asiVisu_HatchingPipeline) pl = new asiVisu_HatchingPipeline;
  //
  this->addPipeline        (Pipeline_Main, pl);
  this->assignDataProvider (Pipeline_Main, dp);

  // Configure presentation
  pl -> Actor()->GetProperty()->SetLineWidth(1.0f);
  pl -> Actor()->GetProperty()->SetLighting(false);
  
  // Make hatching be visualized always on top of the scene
  pl->Mapper()->SetResolveCoincidentTopologyToPolygonOffset();
  pl->Mapper()->SetRelativeCoincidentTopologyLineOffsetParameters(0,-66000);
  pl->Mapper()->SetRelativeCoincidentTopologyPolygonOffsetParameters(0,-66000);
  pl->Mapper()->SetRelativeCoincidentTopologyPointOffsetParameter(-66000);
}

//! Factory method for Presentation.
//! \param[in] N Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_HatchingPrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_HatchingPrs(N);
}

//! Returns true if the Presentation is visible, false -- otherwise.
//! \return true/false.
bool asiVisu_HatchingPrs::IsVisible() const
{
  return m_node->HasUserFlags(NodeFlag_IsPresentationVisible);
}

//-----------------------------------------------------------------------------

//! Callback for initialization of Presentation pipelines.
void asiVisu_HatchingPrs::beforeInitPipelines()
{
  // Do nothing...
}

//! Callback for initialization of Presentation pipelines.
void asiVisu_HatchingPrs::afterInitPipelines()
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked before the
//! kernel update routine starts.
void asiVisu_HatchingPrs::beforeUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked after the
//! kernel update routine completes.
void asiVisu_HatchingPrs::afterUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for highlighting.
void asiVisu_HatchingPrs::highlight(vtkRenderer*,
                                    const Handle(asiVisu_PickerResult)&,
                                    const asiVisu_SelectionNature) const
{}

//! Callback for highlighting reset.
void asiVisu_HatchingPrs::unHighlight(vtkRenderer*,
                                      const asiVisu_SelectionNature) const
{}

//! Callback for rendering.
void asiVisu_HatchingPrs::renderPipelines(vtkRenderer*) const
{}

//! Callback for de-rendering.
void asiVisu_HatchingPrs::deRenderPipelines(vtkRenderer*) const
{}
