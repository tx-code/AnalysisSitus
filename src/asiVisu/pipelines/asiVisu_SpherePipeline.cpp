//-----------------------------------------------------------------------------
// Created on: 19 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023, Julia Slyadneva
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
#include <asiVisu_SpherePipeline.h>

// asiVisu includes
#include <asiVisu_NodeInfo.h>
#include <asiVisu_Utils.h>
#include <asiVisu_SphereDataProvider.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiVisu_SpherePipeline::asiVisu_SpherePipeline()
//
: asiVisu_Pipeline   ( vtkSmartPointer<vtkPolyDataMapper>::New(),
                       vtkSmartPointer<vtkActor>::New() )
{
  m_appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

  // Initialize Data Source.
  m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();

  // Make the surface smooth.
  m_sphereSource->SetPhiResolution(100);
  m_sphereSource->SetThetaResolution(100);

  m_triSource = vtkSmartPointer<asiVisu_TriangleSource>::New();
  //vtkSmartPointer<vtkPolyData> input1 = vtkSmartPointer<vtkPolyData>::New();
  //input1->ShallowCopy(m_sphereSource->GetOutput());

  m_appendFilter->AddInputConnection(m_sphereSource->GetOutputPort());
  m_appendFilter->AddInputConnection(m_triSource->GetOutputPort());
  //m_appendFilter->Update();

  this->SetInputConnection(m_appendFilter->GetOutputPort());
}

//-----------------------------------------------------------------------------

//! Sets input data for the pipeline.
//! \param[in] dataProvider Data Provider.
void asiVisu_SpherePipeline::SetInput(const Handle(asiVisu_DataProvider)& dataProvider)
{
  Handle(asiVisu_SphereDataProvider)
    DP = Handle(asiVisu_SphereDataProvider)::DownCast(dataProvider);

  gp_Pnt loc = DP->GetLocation();
  m_sphereSource->SetCenter(loc.X(), loc.Y(), loc.Z());

  const double d = DP->GetDiameter();
  m_sphereSource->SetRadius(d * 0.5);

  double coords[3][3];
  DP->GetPoints(coords);
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

//std::cout << coords[0][0] << "  " << coords[0][1] << "  " << coords[0][2] << std::endl;
//std::cout << coords[1][0] << "  " << coords[1][1] << "  " << coords[1][2] << std::endl;
//std::cout << coords[2][0] << "  " << coords[2][1] << "  " << coords[2][2] << std::endl;

  points->InsertNextPoint(coords[0]);
  points->InsertNextPoint(coords[1]);
  points->InsertNextPoint(coords[2]);

  m_triSource->SetPoints(points);

  // Update modification timestamp.
  this->Modified();
}

//-----------------------------------------------------------------------------

//! Callback for AddToRenderer() routine. Good place to adjust visualization
//! properties of the pipeline's actor.
void asiVisu_SpherePipeline::callback_add_to_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

//! Callback for RemoveFromRenderer() routine.
void asiVisu_SpherePipeline::callback_remove_from_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

//! Callback for Update() routine.
void asiVisu_SpherePipeline::callback_update()
{}
