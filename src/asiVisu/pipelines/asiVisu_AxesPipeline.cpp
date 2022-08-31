//-----------------------------------------------------------------------------
// Created on: 30 August 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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
#include <asiVisu_AxesPipeline.h>

// asiVisu includes
#include <asiVisu_AxesDataProvider.h>
#include <asiVisu_AxesSource.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

asiVisu_AxesPipeline::asiVisu_AxesPipeline()
  : asiVisu_Pipeline( vtkSmartPointer<vtkPolyDataMapper>::New(),
                      vtkSmartPointer<vtkActor>::New() ),
    m_bMapperColorsSet(false)
{
  // Set line width.
  this->Actor()->GetProperty()->SetLineWidth(3);
  this->Actor()->GetProperty()->SetPointSize(8);
  //
  asiVisu_Utils::ApplyLightingRulesDark( this->Actor() );
}

//-----------------------------------------------------------------------------

void asiVisu_AxesPipeline::SetInput(const Handle(asiVisu_DataProvider)& dataProvider)
{
  Handle(asiVisu_AxesDataProvider)
    DP = Handle(asiVisu_AxesDataProvider)::DownCast(dataProvider);

  /* ============================
   *  Prepare polygonal data set
   * ============================ */

  if ( DP->MustExecute( this->GetMTime() ) )
  {
    vtkSmartPointer<asiVisu_AxesSource>
      axesSrc = vtkSmartPointer<asiVisu_AxesSource>::New();
    //
    axesSrc->SetInputAxes( DP->GetOrigin(),
                           DP->GetDX(),
                           DP->GetDY(),
                           DP->GetDZ(),
                           DP->GetScaleCoeff() );

    // Chain pipeline
    this->SetInputConnection( axesSrc->GetOutputPort() );
  }

  // Update modification timestamp
  this->Modified();
}

//-----------------------------------------------------------------------------

void asiVisu_AxesPipeline::callback_add_to_renderer(vtkRenderer* asiVisu_NotUsed(renderer))
{}

//-----------------------------------------------------------------------------

void asiVisu_AxesPipeline::callback_remove_from_renderer(vtkRenderer* asiVisu_NotUsed(renderer))
{}

//-----------------------------------------------------------------------------

void asiVisu_AxesPipeline::callback_update()
{
  if ( !m_bMapperColorsSet )
  {
    vtkSmartPointer<vtkLookupTable> lookup = asiVisu_Utils::InitAxesLookupTable();
    asiVisu_Utils::InitMapper(m_mapper, lookup, ARRNAME_AXES_SCALARS);
    m_bMapperColorsSet = true;
  }
}
