//-----------------------------------------------------------------------------
// Created on: 31 July 2021
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
#include <asiVisu_Grid2dPrs.h>

// asiVisu includes
#include <asiVisu_Grid2dDataProvider.h>
#include <asiVisu_Grid2dPipeline.h>

// VTK includes
#include <vtkActor.h>
#include <vtkMapper.h>

//! Creates a Presentation object for the passed Grid Node.
//! \param[in] N Grid Node to create a Presentation for.
asiVisu_Grid2dPrs::asiVisu_Grid2dPrs(const Handle(ActAPI_INode)& N)
: asiVisu_DefaultPrs(N)
{
  Handle(asiData_Grid2dNode) grid_n = Handle(asiData_Grid2dNode)::DownCast(N);

  // Create Data Provider.
  Handle(asiVisu_Grid2dDataProvider) DP = new asiVisu_Grid2dDataProvider(grid_n);

  // Create pipeline.
  Handle(asiVisu_Grid2dPipeline) PL = new asiVisu_Grid2dPipeline;
  //
  PL->Actor()->SetPickable(0);

  this->addPipeline        ( Pipeline_UniformGrid, PL );
  this->assignDataProvider ( Pipeline_UniformGrid, DP );
}

//! Factory method for Presentation.
//! \param[in] N Grid Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_Grid2dPrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_Grid2dPrs(N);
}
