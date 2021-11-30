//-----------------------------------------------------------------------------
// Created on: 10 September 2021
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
#include <asiVisu_IVVectorFieldPrs.h>

// asiVisu includes
#include <asiVisu_IVVectorFieldDataProvider.h>
#include <asiVisu_VectorsPipeline.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

asiVisu_IVVectorFieldPrs::asiVisu_IVVectorFieldPrs(const Handle(ActAPI_INode)& N)
: asiVisu_DefaultPrs(N)
{
  Handle(asiData_IVVectorFieldNode)
    node = Handle(asiData_IVVectorFieldNode)::DownCast(N);

  // Create data provider.
  Handle(asiVisu_IVVectorFieldDataProvider)
    DP = new asiVisu_IVVectorFieldDataProvider(node);

  // Create and register a pipeline.
  this->addPipeline        ( Pipeline_Main, new asiVisu_VectorsPipeline );
  this->assignDataProvider ( Pipeline_Main, DP );

  // Tune props.
  Handle(asiVisu_VectorsPipeline)
    PL = Handle(asiVisu_VectorsPipeline)::DownCast( this->GetPipeline(Pipeline_Main) );
  //
  PL->Mapper()->ScalarVisibilityOff();
}

//-----------------------------------------------------------------------------

Handle(asiVisu_Prs) asiVisu_IVVectorFieldPrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_IVVectorFieldPrs(N);
}

//-----------------------------------------------------------------------------

void asiVisu_IVVectorFieldPrs::Colorize(const ActAPI_Color& color) const
{
  Handle(asiVisu_VectorsPipeline)
    pl = Handle(asiVisu_VectorsPipeline)::DownCast( this->GetPipeline(Pipeline_Main) );

  pl->Actor()->GetProperty()->SetColor( color.Red(),
                                        color.Green(),
                                        color.Blue() );
}

//-----------------------------------------------------------------------------

//! Callback for updating of Presentation pipelines invoked after the
//! kernel update routine completes.
void asiVisu_IVVectorFieldPrs::afterUpdatePipelines() const
{
  Handle(asiData_IVVectorFieldNode)
    N = Handle(asiData_IVVectorFieldNode)::DownCast( this->GetNode() );

  /* Actualize color */

  if ( N->HasColor() )
  {
    ActAPI_Color color = asiVisu_Utils::IntToColor( N->GetColor() );
    this->Colorize(color);
  }
  else
    this->Colorize( ActAPI_Color(Quantity_NOC_WHITE) );
}
