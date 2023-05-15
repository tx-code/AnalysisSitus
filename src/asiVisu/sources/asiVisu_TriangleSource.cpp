//-----------------------------------------------------------------------------
// Created on: 20 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Julia Slyadneva
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
#include <asiVisu_TriangleSource.h>

// VTK includes
#include <vtkTriangle.h>
#include <vtkCellArray.h>

#if defined USE_MOBIUS
  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_TriangleSource)

//-----------------------------------------------------------------------------

asiVisu_TriangleSource::asiVisu_TriangleSource()
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_TriangleSource::~asiVisu_TriangleSource()
{}

//-----------------------------------------------------------------------------

void asiVisu_TriangleSource::SetPoints(const vtkSmartPointer<vtkPoints>& points)
{
  m_points = points;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

int asiVisu_TriangleSource::RequestData(vtkInformation*        request,
                                        vtkInformationVector** inputVector,
                                        vtkInformationVector*  outputVector)
{
  if (!m_points)
    return 0;

  vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();

  //for (size_t i = 0; i < iNumVerts; i += 3)
  //{
  //  vtkTriangle* triangle1 = vtkTriangle::New();
  //  triangle1->GetPointIds()->SetId(0, i);
  //  triangle1->GetPointIds()->SetId(1, i + 1);
  //  triangle1->GetPointIds()->SetId(2, i + 2);
  //  newPolys->InsertNextCell(triangle1);
  //}
  triangle->GetPointIds()->SetId(0, 0);
  triangle->GetPointIds()->SetId(1, 1);
  triangle->GetPointIds()->SetId(2, 2);

  //double* coord = *m_points->GetPoint(0);
  //std::cout << coord[0] << "  " << coord[1] << "  " << coord[2] << std::endl;
  //std::cout << coords[1][0] << "  " << coords[1][1] << "  " << coords[1][2] << std::endl;
  //std::cout << coords[2][0] << "  " << coords[2][1] << "  " << coords[2][2] << std::endl;

  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
  triangles->InsertNextCell(triangle);

  // Get output polygonal data from the information vector.
  vtkPolyData* trianglePolyData = vtkPolyData::GetData(outputVector);
  trianglePolyData->Allocate();

  // Add the geometry and topology to the polydata
  trianglePolyData->SetPoints(m_points);
  trianglePolyData->SetPolys(triangles);

  return Superclass::RequestData(request, inputVector, outputVector);
}

