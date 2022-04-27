//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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
#include <asiVisu_DiscrFacePrs.h>

// asiVisu includes
#include <asiVisu_DiscrFaceDataProvider.h>
#include <asiVisu_DiscrFacePipeline.h>

// VTK includes
#include <vtkActor.h>
#include <vtkMapper.h>

//! Creates a Presentation object for the passed Discrete Face Node.
//! \param[in] N Grid Node to create a Presentation for.
asiVisu_DiscrFacePrs::asiVisu_DiscrFacePrs(const Handle(ActAPI_INode)& N)
: asiVisu_DefaultPrs(N)
{
  Handle(asiData_DiscrFaceNode) discrFace = Handle(asiData_DiscrFaceNode)::DownCast(N);

  // Create Data Provider.
  Handle(asiVisu_DiscrFaceDataProvider) DP = new asiVisu_DiscrFaceDataProvider(discrFace);

  // Create pipeline.
  Handle(asiVisu_DiscrFacePipeline) PL = new asiVisu_DiscrFacePipeline();
  //
  PL->Actor()->SetPickable(0);

  this->addPipeline        ( Pipeline_DiscrFace, PL );
  this->assignDataProvider ( Pipeline_DiscrFace, DP );
}

//! Factory method for Presentation.
//! \param[in] N Discrete Face Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_DiscrFacePrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_DiscrFacePrs(N);
}
