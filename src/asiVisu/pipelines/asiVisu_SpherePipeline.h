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

#pragma once

// asiVisu includes
#include <asiVisu_Pipeline.h>
#include <asiVisu_Selection.h>
#include <asiVisu_ShapeDisplayModeFilter.h>
#include <asiVisu_ShapeRobustSource.h>
#include <asiVisu_TriangleSource.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#pragma warning(pop)

//-----------------------------------------------------------------------------

//! Visualization pipeline for sphere indicating a scalar assigned to a triangulation facet.
class asiVisu_SpherePipeline : public asiVisu_Pipeline
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_SpherePipeline, asiVisu_Pipeline)

public:

  asiVisu_EXPORT
    asiVisu_SpherePipeline();

public:

  asiVisu_EXPORT virtual void
    SetInput(const Handle(asiVisu_DataProvider)& dataProvider);

private:

  virtual void callback_add_to_renderer      (vtkRenderer* renderer);
  virtual void callback_remove_from_renderer (vtkRenderer* renderer);
  virtual void callback_update               ();

private:

  //! Copying prohibited.
  asiVisu_SpherePipeline(const asiVisu_SpherePipeline&);

  //! Assignment prohibited.
  asiVisu_SpherePipeline& operator=(const asiVisu_SpherePipeline&);

protected:

  vtkSmartPointer<vtkAppendPolyData>       m_appendFilter;
  vtkSmartPointer<vtkSphereSource>         m_sphereSource;
  vtkSmartPointer<asiVisu_TriangleSource>  m_triSource;
};

