/***************************************************************************
 *   Copyright (c) OPEN CASCADE SAS                                        *
 *                                                                         *
 *   This file is part of Open CASCADE Technology software library.        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 ***************************************************************************/

#ifndef _DFBrowser_Attr_h
#define _DFBrowser_Attr_h

#include <asiUI.h>

#include <DFBrowser_Attr.hxx>
#include <Standard_Transient.hxx>

#include <Standard_CString.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_Attribute.hxx>
#include <DFBrowser_Colors.hxx>

class TDF_Label;
class TopoDS_Shape;
class TCollection_ExtendedString;

class DFBrowser_Attr : public Standard_Transient
{
 public:

  asiUI_EXPORT DFBrowser_Attr();

  asiUI_EXPORT const TCollection_AsciiString & LoadLabel(const TDF_Label& theLabel,const Standard_Boolean theName = Standard_True);

  asiUI_EXPORT const TCollection_AsciiString & LoadAttribute(const Handle(TDF_Attribute)& theAttribute);

  asiUI_EXPORT const TCollection_AsciiString & LoadAttribute(const Handle(TDF_Attribute)& theAttribute,const Standard_Boolean theFull);

  inline DFBrowser_Colors GetColor() const
  {
    return myColor;
  }

  inline const TCollection_AsciiString & GetName() const
  {
    return myName;
  }

  inline Standard_Boolean HasError() const
  {
    return myError;
  }

  inline void Detailed(const Standard_Boolean theDetailed)
  {
    myDetailed = theDetailed;
  }

  asiUI_EXPORT void NamedShape(const Standard_Address theTree, const Standard_Address theNode);

  asiUI_EXPORT const TCollection_AsciiString & LoadShape(const TopoDS_Shape& theShape,const TDF_Label& theAccess);

  asiUI_EXPORT const TCollection_AsciiString & LoadVertices(const TopoDS_Shape& theShape);

  asiUI_EXPORT void Description(const Handle(TDF_Attribute) &theAttr,TCollection_AsciiString &theName);

  asiUI_EXPORT void Description(const TDF_Label &theLabel,TCollection_AsciiString &theName);

  asiUI_EXPORT void AddToName(const TCollection_ExtendedString& theString);

  asiUI_EXPORT void AddToName(const TCollection_AsciiString& theString);

  asiUI_EXPORT void AddToName(const Standard_CString theString);

  asiUI_EXPORT void AddToName(const Handle(TDF_Attribute)& theAttribute);

  asiUI_EXPORT void AddToName(const Standard_Integer theType,const Standard_Integer theCount);

  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_Attr, Standard_Transient)

 private:

  asiUI_EXPORT void AddList(const Standard_CString theString);

  asiUI_EXPORT void AddList(const Handle(TDF_Attribute)& theAttribute);

  TCollection_AsciiString myName;
  DFBrowser_Colors myColor;
  Standard_Integer myBrackets;
  Standard_Boolean myError;
  Standard_Boolean myDetailed;
};

#endif
