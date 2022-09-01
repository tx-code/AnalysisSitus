//-----------------------------------------------------------------------------
// Created on: 01 September 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022, Andrey Voevodin
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
#include <asiVisu_MeshPipelineBase.h>

// asiVisu includes
#include <asiVisu_MeshSource.h>
#include <asiVisu_MeshUtils.h>

// VTK includes
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

asiVisu_MeshPipelineBase::asiVisu_MeshPipelineBase(const vtkSmartPointer<asiVisu_MeshSource>& source)
//
: asiVisu_Pipeline ( vtkSmartPointer<vtkPolyDataMapper>::New(),
                     vtkSmartPointer<vtkActor>::New() )
{
  // Initialize Data Source
  if ( !source.GetPointer() )
    m_source = vtkSmartPointer<asiVisu_MeshSource>::New();
  else
    m_source = source;
}

//-----------------------------------------------------------------------------

asiVisu_MeshPipelineBase::~asiVisu_MeshPipelineBase(){}

//-----------------------------------------------------------------------------

//! Callback for AddToRenderer() routine. Good place to adjust visualization
//! properties of the pipeline's actor.
void asiVisu_MeshPipelineBase::callback_add_to_renderer(vtkRenderer*)
{
  this->Actor()->GetProperty()->SetInterpolationToPhong();
}

//-----------------------------------------------------------------------------

//! Callback for RemoveFromRenderer() routine.
void asiVisu_MeshPipelineBase::callback_remove_from_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

//! Callback for Update() routine.
void asiVisu_MeshPipelineBase::callback_update()
{
  asiVisu_MeshUtils::InitMapper(m_mapper, ARRNAME_MESH_ITEM_TYPE);
}
