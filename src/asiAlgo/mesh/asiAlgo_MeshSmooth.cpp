//-----------------------------------------------------------------------------
// Created on: 30 June 2021
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
#include <asiAlgo_MeshSmooth.h>

// asiAlgo includes
#include <asiAlgo_MeshConvert.h>

// VTK includes
#ifdef USE_VTK
  #include <vtkSmoothPolyDataFilter.h>
  #include <vtkPolyData.h>
#endif

//-----------------------------------------------------------------------------

#ifdef USE_VTK

bool asiAlgo_MeshSmooth::DoVTK(const Handle(Poly_Triangulation)& source,
                               const int                         nbIter,
                               Handle(Poly_Triangulation)&       result,
                               ActAPI_ProgressEntry              notifier)
{
  // Convert to VTK polygonal data which can be then processed.
  vtkSmartPointer<vtkPolyData> input;
  //
  if ( !asiAlgo_MeshConvert::ToVTK(source, input) )
  {
    notifier.SendLogMessage(LogErr(Normal) << "Cannot convert to VTK polygonal data.");
    return false;
  }

  // Prepare smoothing algorithm & run
  vtkSmartPointer<vtkSmoothPolyDataFilter> algo = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
  algo->SetInputData(input);
  algo->SetFeatureEdgeSmoothing(0);
  algo->SetBoundarySmoothing(0);
  algo->SetNumberOfIterations(nbIter);
  algo->Update();

  // Access smoothing result.
  vtkSmartPointer<vtkPolyData> smoothed = vtkSmartPointer<vtkPolyData>::New();
  smoothed->ShallowCopy( algo->GetOutput() );

  // Convert to a persistent form.
  if ( !asiAlgo_MeshConvert::FromVTK(smoothed, result) )
  {
    notifier.SendLogMessage(LogErr(Normal) << "Cannot convert the smoothed mesh to a persistent form.");
    return false;
  }

  return true;
}

#endif // USE_VTK
