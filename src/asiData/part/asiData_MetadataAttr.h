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

#ifndef asiData_MetadataAttr_h
#define asiData_MetadataAttr_h

// asiData includes
#include <asiData.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>

// OCCT includes
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>

//! OCAF attribute for metadata, such as colors.
class asiData_MetadataAttr : public TDF_Attribute
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_MetadataAttr, TDF_Attribute)

public:

  typedef NCollection_IndexedDataMap<TopoDS_Shape,
                                     int,
                                     asiAlgo_ShapePartnerHasher> t_shapeColorMap;

// Construction & settling-down routines:
public:

  asiData_EXPORT
    asiData_MetadataAttr();

  asiData_EXPORT static Handle(asiData_MetadataAttr)
    Set(const TDF_Label& Label);

// GUID accessors:
public:

  asiData_EXPORT static const Standard_GUID&
    GUID();

  asiData_EXPORT virtual const Standard_GUID&
    ID() const;

// Attribute's kernel methods:
public:

  asiData_EXPORT virtual Handle(TDF_Attribute)
    NewEmpty() const;

  asiData_EXPORT virtual void
    Restore(const Handle(TDF_Attribute)& mainAttr);

  asiData_EXPORT virtual void
    Paste(const Handle(TDF_Attribute)&       into,
          const Handle(TDF_RelocationTable)& relocTable) const;

// Accessors for domain-specific data:
public:

  asiData_EXPORT void
    SetColor(const TopoDS_Shape& shape,
             const int           icolor);

  asiData_EXPORT int
    GetColor(const TopoDS_Shape& shape) const;

  asiData_EXPORT void
    SetShapeColorMap(const t_shapeColorMap& map);

  asiData_EXPORT const t_shapeColorMap&
    GetShapeColorMap() const;

// Members:
private:

  //! Stored map of colors.
  t_shapeColorMap m_shapeColorMap;

};

#endif
