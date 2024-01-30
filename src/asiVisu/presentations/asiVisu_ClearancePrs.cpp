//-----------------------------------------------------------------------------
// Created on: 14 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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
#include <asiVisu_ClearancePrs.h>

// asiVisu includes
#include <asiVisu_ClearanceDataProvider.h>
#include <asiVisu_MeshEScalarPipeline.h>
#include <asiVisu_MeshResultUtils.h>
#include <asiVisu_SphereDataProvider.h>
#include <asiVisu_SpherePipeline.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkMapper.h>
#include <vtkProperty.h>
#pragma warning(pop)

//-----------------------------------------------------------------------------

asiVisu_ClearancePrs::asiVisu_ClearancePrs(const Handle(ActAPI_INode)& N)
: asiVisu_DefaultPrs(N)
{
  Handle(asiData_ClearanceNode)
    CN = Handle(asiData_ClearanceNode)::DownCast(N);

  // Create Data Provider.
  Handle(asiVisu_ClearanceDataProvider) DP  = new asiVisu_ClearanceDataProvider(CN);
  Handle(asiVisu_SphereDataProvider)    SDS = new asiVisu_SphereDataProvider(CN);

  // Create Pipeline.
  this->addPipeline        ( Pipeline_Main, new asiVisu_MeshEScalarPipeline );
  this->assignDataProvider ( Pipeline_Main, DP );

  this->addPipeline(Pipeline_Sphere, new asiVisu_SpherePipeline);
  this->assignDataProvider(Pipeline_Sphere, SDS);

  //
  this->GetPipeline(Pipeline_Sphere)->Actor()->SetVisibility(0);

  // Initialize scalar bar.
  m_scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
  asiVisu_MeshResultUtils::InitScalarBarWidget(m_scalarBarWidget, 0);
}

//-----------------------------------------------------------------------------

void asiVisu_ClearancePrs::afterUpdatePipelines() const
{
  m_scalarBarWidget->On();

  // Pipeline for the field.
  Handle(asiVisu_MeshEScalarPipeline)
    pl = Handle(asiVisu_MeshEScalarPipeline)::DownCast( this->GetPipeline(Pipeline_Main) );

  // Initialize scalar bar actor.
  m_scalarBarWidget->GetScalarBarActor()->SetLookupTable( pl->Mapper()->GetLookupTable() );
  m_scalarBarWidget->GetScalarBarActor()->SetTitle("Clearance distribution");
}

//-----------------------------------------------------------------------------

void asiVisu_ClearancePrs::renderPipelines(vtkRenderer* renderer) const
{
  if ( !m_scalarBarWidget->GetCurrentRenderer() )
  {
    m_scalarBarWidget->SetInteractor( renderer->GetRenderWindow()->GetInteractor() );
    m_scalarBarWidget->SetDefaultRenderer(renderer);
    m_scalarBarWidget->SetCurrentRenderer(renderer);
  }
}

//-----------------------------------------------------------------------------

void asiVisu_ClearancePrs::deRenderPipelines(vtkRenderer*) const
{
  m_scalarBarWidget->Off();
}

//-----------------------------------------------------------------------------

//! Callback for highlighting.
void asiVisu_ClearancePrs::highlight(vtkRenderer*                        renderer,
                                     const Handle(asiVisu_PickerResult)& pickRes,
                                     const asiVisu_SelectionNature       /*selNature*/) const
{
  Handle(asiVisu_CellPickerResult)
    cellPickerRes = Handle(asiVisu_CellPickerResult)::DownCast(pickRes);

  const TColStd_PackedMapOfInteger& elemIds = cellPickerRes->GetPickedElementIds();

  if ( elemIds.IsEmpty() )
    return;

  this->GetPipeline(Pipeline_Sphere)->Actor()->SetVisibility(1);

  const int facetId = elemIds.GetMinimalMapped();

  // Pipeline for the field.
  Handle(asiVisu_SpherePipeline)
    pl = Handle(asiVisu_SpherePipeline)::DownCast( this->GetPipeline(Pipeline_Sphere) );

  Handle(asiVisu_SphereDataProvider) ds = Handle(asiVisu_SphereDataProvider)::DownCast(dataProvider(Pipeline_Sphere));
  ds->SetFacetId(facetId, false);

  pl->SetInput(ds);

  renderer->AddActor( pl->Actor() );
}

//-----------------------------------------------------------------------------

//! Callback for highlighting reset.
void asiVisu_ClearancePrs::unHighlight(vtkRenderer*                  renderer,
                                       const asiVisu_SelectionNature /*selNature*/) const
{
  // Pipeline for the field.
  Handle(asiVisu_SpherePipeline)
    pl = Handle(asiVisu_SpherePipeline)::DownCast(this->GetPipeline(Pipeline_Sphere));

  renderer->RemoveActor(pl->Actor());
}

