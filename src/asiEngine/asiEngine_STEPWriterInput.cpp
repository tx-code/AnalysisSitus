//-----------------------------------------------------------------------------
// Created on: 28 May 2019
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
#include <asiEngine_STEPWriterInput.h>

#if !defined BUILD_ALGO_ONLY
  // asiVisu includes
  #include <asiVisu_PrsManager.h>
  #include <asiVisu_Utils.h>
#endif

//-----------------------------------------------------------------------------

asiEngine_STEPWriterInput::asiEngine_STEPWriterInput(const Handle(asiEngine_Model)& M)
: asiAlgo_WriteSTEPWithMetaInput (),
  m_model                        (M),
  m_api                          (M)
{
  m_metadata = m_api.GetMetadata();
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiEngine_STEPWriterInput::GetShape() const
{
  return m_model->GetPartNode()->GetShape(false);
}

//-----------------------------------------------------------------------------

TopLoc_Location asiEngine_STEPWriterInput::GetLocation() const
{
  gp_Trsf T = m_model->GetPartNode()->GetTransformationMx();
  return TopLoc_Location(T);
}

//-----------------------------------------------------------------------------

int asiEngine_STEPWriterInput::GetNumSubShapes() const
{
  asiData_MetadataAttr::t_shapeColorMap map;
  m_metadata->GetShapeColorMap(map);

  return map.Extent();
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiEngine_STEPWriterInput::GetSubShape(const int zeroBasedIdx) const
{
  asiData_MetadataAttr::t_shapeColorMap map;
  m_metadata->GetShapeColorMap(map);
  //
  return map.FindKey(zeroBasedIdx + 1);
}

//-----------------------------------------------------------------------------

bool asiEngine_STEPWriterInput::HasColor(const TopoDS_Shape& shape) const
{
  asiData_MetadataAttr::t_shapeColorMap map;
  m_metadata->GetShapeColorMap(map);

  if ( !map.Contains(shape) )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

Quantity_Color
  asiEngine_STEPWriterInput::GetColor(const TopoDS_Shape& shape) const
{
  asiData_MetadataAttr::t_shapeColorMap map;
  m_metadata->GetShapeColorMap(map);

  if ( !map.Contains(shape) )
    return Quantity_Color(1., 1., 1., Quantity_TOC_RGB);

  const int icolor = map.FindFromKey(shape);

  ActAPI_Color color = ActAPI_Color::IntToColor(icolor);

  return Quantity_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB);
}

//-----------------------------------------------------------------------------

Quantity_Color asiEngine_STEPWriterInput::GetCommonColor() const
{
  const int icolor = m_model->GetPartNode()->GetColor();

  ActAPI_Color color = ActAPI_Color::IntToColor(icolor);
  return Quantity_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB);
}

//-----------------------------------------------------------------------------

bool asiEngine_STEPWriterInput::HasCommonColor() const
{
  return true;
}
