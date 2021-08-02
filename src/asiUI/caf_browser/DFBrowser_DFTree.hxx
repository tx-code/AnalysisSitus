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

#ifndef _DFBrowser_DFTree_h
#define _DFBrowser_DFTree_h

// CAF Browser includes
#include <asiUI.h>

#include <Standard_Transient.hxx>

#include <TCollection_AsciiString.hxx>
#include <DFBrowser_PtrGUI.hxx>
#include <DFBrowser_Attr.hxx>
#include <DFBrowser_AttrNode.hxx>
#include <DFBrowser_LabelNode.hxx>
#include <TDocStd_Document.hxx>

class TDF_Label;

class DFBrowser_DFTree : public Standard_Transient
{
  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_DFTree, Standard_Transient)

public:

  asiUI_EXPORT
    DFBrowser_DFTree(const Handle(TDocStd_Document)& theDoc);

  asiUI_EXPORT
    DFBrowser_DFTree();

  asiUI_EXPORT
    Handle(DFBrowser_AttrNode) Node(const Handle(TDF_Attribute)& theAttr) const;

  asiUI_EXPORT
    Handle(DFBrowser_LabelNode) Node(const TDF_Label& theLabel) const;

public:

  inline const Handle(DFBrowser_Attr) & Attr() const
  {
    return myAttr;
  }

  inline const Handle(DFBrowser_LabelNode) & Root() const
  {
    return myRoot;
  }

  asiUI_EXPORT static Standard_Integer NbBrowsers();

  asiUI_EXPORT void Update();

  asiUI_EXPORT void Close();

  asiUI_EXPORT void AddDocName(TCollection_AsciiString& theName) const;

 private:

  Handle(TDocStd_Document) myDoc;
  Handle(DFBrowser_LabelNode) myRoot;
  Handle(DFBrowser_Attr) myAttr;
  DFBrowser_PtrGUI myGUI;
};

#endif
