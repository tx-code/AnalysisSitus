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
#include <asiVisu_AxesSource.h>

// asiVisu includes
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkDataObject.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_AxesSource)

//-----------------------------------------------------------------------------

//! Default constructor.
asiVisu_AxesSource::asiVisu_AxesSource() : vtkPolyDataAlgorithm(), m_fScaleCoeff(1.)
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline
}

//-----------------------------------------------------------------------------

//! Destructor.
asiVisu_AxesSource::~asiVisu_AxesSource()
{}

//-----------------------------------------------------------------------------

bool asiVisu_AxesSource::SetInputAxes(const gp_Pnt& origin,
                                      const gp_Dir& dx,
                                      const gp_Dir& dy,
                                      const gp_Dir& dz,
                                      const double  scale)
{
  m_origin      = origin;
  m_dx          = dx;
  m_dy          = dy;
  m_dz          = dz;
  m_fScaleCoeff = scale;

  this->Modified();
  return true;
}

//-----------------------------------------------------------------------------

int asiVisu_AxesSource::RequestData(vtkInformation*        request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector*  outputVector)
{
  /* ==============================
   *  Prepare involved collections
   * ============================== */

  vtkPolyData* polyOutput = vtkPolyData::GetData(outputVector);
  polyOutput->Allocate();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  polyOutput->SetPoints(points);

  // Add array for scalars.
  vtkCellData* cellData = polyOutput->GetCellData();
  //
  vtkSmartPointer<vtkIntArray> typeArr = asiVisu_Utils::InitIntArray(ARRNAME_AXES_SCALARS);
  cellData->AddArray(typeArr);

  /* =======================
   *  Populate VTK data set
   * ======================= */

  // Register glyphs representing images of two first knot intervals.
  this->registerAxis(m_dx, VisuAxis_X, polyOutput);
  this->registerAxis(m_dy, VisuAxis_Y, polyOutput);
  this->registerAxis(m_dz, VisuAxis_Z, polyOutput);

  return Superclass::RequestData(request, inputVector, outputVector);
}

//-----------------------------------------------------------------------------

vtkIdType asiVisu_AxesSource::registerPoint(const gp_Pnt& P,
                                            vtkPolyData*  polyData)
{
  // Access necessary arrays
  vtkPoints* points = polyData->GetPoints();

  // Push the point into VTK data set
  vtkIdType pid = points->InsertNextPoint( P.X(), P.Y(), P.Z() );

  return pid;
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_AxesSource::registerAxis(const gp_Dir&      dir,
                                   const asiVisu_Axis type,
                                   vtkPolyData*       polyData)
{
  // Register points.
  gp_Pnt P0 = m_origin;
  gp_Pnt P1 = m_origin.XYZ() + dir.XYZ()*m_fScaleCoeff;
  //
  std::vector<vtkIdType> pids;
  pids.push_back( this->registerPoint(P0, polyData) );
  pids.push_back( this->registerPoint(P1, polyData) );

  // Add cell.
  vtkIdType cellID =
    polyData->InsertNextCell( VTK_LINE, (int) pids.size(), &pids[0] );

  // Set scalar.
  vtkIntArray*
    typeArr = vtkIntArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_AXES_SCALARS) );
  //
  typeArr->InsertNextValue(type);

  return cellID;
}
