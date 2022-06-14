//-----------------------------------------------------------------------------
// Created on: 19 April 2022
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
#include <asiEngine_IVTopoItemSTEPWriterInput.h>

// asiVisu includes
#include <asiVisu_Utils.h>

//-----------------------------------------------------------------------------

asiEngine_IVTopoItemSTEPWriterInput::
  asiEngine_IVTopoItemSTEPWriterInput(const Handle(asiData_IVTopoItemNode)& topoItemIV,
                                      const Handle(asiEngine_Model)&        M)
  : asiAlgo_WriteSTEPWithMetaInput(),
  m_topoItem(topoItemIV),
  m_model(M)
{}

//-----------------------------------------------------------------------------

TopoDS_Shape asiEngine_IVTopoItemSTEPWriterInput::GetShape() const
{
  TopoDS_Shape shape;

  if ( !m_topoItem.IsNull() && m_topoItem->IsWellFormed() )
  {
    shape = m_topoItem->GetShape();
  }

  return shape;
}

//-----------------------------------------------------------------------------

TopLoc_Location asiEngine_IVTopoItemSTEPWriterInput::GetLocation() const
{
  return TopLoc_Location();
}

//-----------------------------------------------------------------------------

int asiEngine_IVTopoItemSTEPWriterInput::GetNumSubShapes() const
{
  return 0;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiEngine_IVTopoItemSTEPWriterInput::GetSubShape(const int /*zeroBasedIdx*/) const
{
  return TopoDS_Shape();
}

//-----------------------------------------------------------------------------

bool asiEngine_IVTopoItemSTEPWriterInput::HasColor(const TopoDS_Shape& /*shape*/) const
{
  if ( m_topoItem.IsNull() || !m_topoItem->IsWellFormed() )
  {
    return false;
  }

  return m_topoItem->HasColor();
}

//-----------------------------------------------------------------------------

Quantity_Color
asiEngine_IVTopoItemSTEPWriterInput::GetColor(const TopoDS_Shape& /*shape*/) const
{
  if ( m_topoItem.IsNull() || !m_topoItem->IsWellFormed() )
  {
    return Quantity_Color(1., 1., 1., Quantity_TOC_RGB);
  }

  ActAPI_Color color = asiVisu_Utils::IntToColor(m_topoItem->GetColor());

  return Quantity_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB);
}

//-----------------------------------------------------------------------------

Quantity_Color asiEngine_IVTopoItemSTEPWriterInput::
  GetCommonColor() const
{
  ActAPI_Color color = asiVisu_Utils::IntToColor( m_topoItem->GetColor() );
  return Quantity_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB);
}

//-----------------------------------------------------------------------------

bool asiEngine_IVTopoItemSTEPWriterInput::
  HasCommonColor() const
{
  return true;
}
