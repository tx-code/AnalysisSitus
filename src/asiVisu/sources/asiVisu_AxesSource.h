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

#ifndef asiVisu_AxesSource_h
#define asiVisu_AxesSource_h

// asiVisu includes
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>

//! Data source for 3D axes frame.
class asiVisu_AxesSource : public vtkPolyDataAlgorithm
{
// RTTI and construction:
public:

  vtkTypeMacro(asiVisu_AxesSource, vtkPolyDataAlgorithm);
  static asiVisu_AxesSource* New();

// Kernel methods:
public:

  bool SetInputAxes(const gp_Pnt& origin,
                    const gp_Dir& dx,
                    const gp_Dir& dy,
                    const gp_Dir& dz,
                    const double  scale);

protected:

  virtual int RequestData(vtkInformation*        request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector*  outputVector);

protected:

  vtkIdType
    registerPoint(const gp_Pnt& P,
                  vtkPolyData*  polyData);

  vtkIdType
    registerAxis(const gp_Dir&      dir,
                 const asiVisu_Axis type,
                 vtkPolyData*       polyData);

protected:

  asiVisu_AxesSource();
  ~asiVisu_AxesSource();

private:

  asiVisu_AxesSource(const asiVisu_AxesSource&);
  asiVisu_AxesSource& operator=(const asiVisu_AxesSource&);

protected:

  gp_Pnt m_origin;
  gp_Dir m_dx, m_dy, m_dz;
  double m_fScaleCoeff;

};

#endif
