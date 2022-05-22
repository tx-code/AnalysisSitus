//-----------------------------------------------------------------------------
// Created on: 21 May 2022
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
#include <asiData_MetadataAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>

//-----------------------------------------------------------------------------
// Construction & settling-down routines
//-----------------------------------------------------------------------------

//! Default constructor.
asiData_MetadataAttr::asiData_MetadataAttr() : TDF_Attribute()
{}

//! Settles down new metadata attribute to the given OCAF Label.
//! \param[in] label TDF label to settle down the new attribute to.
//! \return newly created attribute settled down onto the target label.
Handle(asiData_MetadataAttr) asiData_MetadataAttr::Set(const TDF_Label& label)
{
  Handle(asiData_MetadataAttr) A;
  //
  if ( !label.FindAttribute(GUID(), A) )
  {
    A = new asiData_MetadataAttr();
    label.AddAttribute(A);
  }
  return A;
}

//-----------------------------------------------------------------------------
// Accessors for attribute's GUID
//-----------------------------------------------------------------------------

//! Returns statically defined GUID for the attribute.
//! \return statically defined GUID.
const Standard_GUID& asiData_MetadataAttr::GUID()
{
  static Standard_GUID AttrGUID("7FDA31BD-3FE3-4A00-A919-FC22DEE24DDA");
  return AttrGUID;
}

//! Accessor for GUID associated with this kind of OCAF attribute.
//! \return GUID of the OCAF attribute.
const Standard_GUID& asiData_MetadataAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------
// Attribute's kernel methods:
//-----------------------------------------------------------------------------

//! Creates new instance of the attribute which is not initially populated
//! with any data structures.
//! \return new instance of the attribute.
Handle(TDF_Attribute) asiData_MetadataAttr::NewEmpty() const
{
  return new asiData_MetadataAttr();
}

//! Performs data transferring from the given OCAF attribute to this one.
//! This method is mainly used by OCAF Undo/Redo kernel as a part of
//! backup functionality.
//! \param[in] mainAttr OCAF attribute to copy data from.
void asiData_MetadataAttr::Restore(const Handle(TDF_Attribute)& mainAttr)
{
  Handle(asiData_MetadataAttr) fromCasted = Handle(asiData_MetadataAttr)::DownCast(mainAttr);
  m_shapeColorMap = fromCasted->GetShapeColorMap();
}

//! Supporting method for Copy/Paste functionality. Performs full copying of
//! the underlying data.
//! \param[in] into       where to paste.
//! \param[in] relocTable relocation table.
void asiData_MetadataAttr::Paste(const Handle(TDF_Attribute)&       into,
                                 const Handle(TDF_RelocationTable)& asiData_NotUsed(relocTable)) const
{
  Handle(asiData_MetadataAttr) intoCasted = Handle(asiData_MetadataAttr)::DownCast(into);
  intoCasted->SetShapeColorMap(m_shapeColorMap);
}

//-----------------------------------------------------------------------------
// Accessors for domain-specific data
//-----------------------------------------------------------------------------

void asiData_MetadataAttr::SetColor(const TopoDS_Shape& shape,
                                    const int           icolor)
{
  this->Backup();

  int* pColor = m_shapeColorMap.ChangeSeek(shape);

  if ( !pColor )
    m_shapeColorMap.Add(shape, icolor);
  else
    *pColor = icolor;
}

//-----------------------------------------------------------------------------

int asiData_MetadataAttr::GetColor(const TopoDS_Shape& shape) const
{
  const int* pColor = m_shapeColorMap.Seek(shape);

  if ( !pColor )
    return 0;

  return *pColor;
}

//-----------------------------------------------------------------------------

void asiData_MetadataAttr::SetShapeColorMap(const t_shapeColorMap& map)
{
  this->Backup();

  m_shapeColorMap = map;
}

//-----------------------------------------------------------------------------

const asiData_MetadataAttr::t_shapeColorMap&
  asiData_MetadataAttr::GetShapeColorMap() const
{
  return m_shapeColorMap;
}
