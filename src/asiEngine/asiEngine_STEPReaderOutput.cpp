//-----------------------------------------------------------------------------
// Created on: 02 June 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Sergey Slyadnev
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
#include <asiEngine_STEPReaderOutput.h>

// asiVisu includes
#include <asiVisu_PrsManager.h>
#include <asiVisu_Utils.h>

//-----------------------------------------------------------------------------

asiEngine_STEPReaderOutput::asiEngine_STEPReaderOutput(const Handle(asiEngine_Model)& M)
: asiAlgo_ReadSTEPWithMetaOutput (),
  m_model                        (M),
  m_api                          (M)
{}

//-----------------------------------------------------------------------------

void asiEngine_STEPReaderOutput::SetShape(const TopoDS_Shape& shape)
{
  m_api.Update( shape, nullptr, !m_model->GetPartNode()->IsKeepTessParams() );
}

//-----------------------------------------------------------------------------

void asiEngine_STEPReaderOutput::SetColor(const TopoDS_Shape&   subshape,
                                          const Quantity_Color& color,
                                          const ColorAttachment attch)
{
  // Convert color to a persistent form.
  const int iCol = asiVisu_Utils::ColorToInt( color.Red(), color.Green(), color.Blue() );

  // Part color.
  if ( (subshape.ShapeType() < TopAbs_FACE) && (attch == ColorAttachment_SURFACE) )
  {
    m_model->GetPartNode()->SetColor(iCol);
  }

  // Feature color.
  else if ( subshape.ShapeType() >= TopAbs_FACE )
  {
    if ( (subshape.ShapeType() == TopAbs_FACE) && (attch != ColorAttachment_SURFACE) )
      return;

    if ( (subshape.ShapeType() == TopAbs_EDGE) && (attch != ColorAttachment_CURVE) )
      return;

    if ( (subshape.ShapeType() == TopAbs_VERTEX) && (attch != ColorAttachment_POINT) )
      return;

    // Store color.
    m_api.GetMetadata()->SetColor(subshape, iCol);
  }
}
