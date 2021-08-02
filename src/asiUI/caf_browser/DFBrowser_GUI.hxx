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

#ifndef _DFBrowser_GUI_h
#define _DFBrowser_GUI_h

#include <asiUI.h>

#include <DFBrowser_Picture.hxx>
#include <DFBrowser_Level.hxx>
#include <DFBrowser_AttrPrs.hxx>
#include <DFBrowser_DFTree.hxx>
#include <DFBrowser_DFNode.hxx>

class DFBrowser_GUI
{
 public:

  asiUI_EXPORT DFBrowser_GUI(const Handle(DFBrowser_DFTree)& theTree);

  asiUI_EXPORT void Selected(const Handle(DFBrowser_DFNode)& theNode);

  asiUI_EXPORT void DoubleClicked(const Handle(DFBrowser_DFNode)& theNode) const;

  asiUI_EXPORT const Handle(DFBrowser_DFNode) & First() const;

  asiUI_EXPORT Handle(DFBrowser_DFNode) Next(const Handle(DFBrowser_DFNode)& theNode) const;

  asiUI_EXPORT Handle(DFBrowser_DFNode) NextVisible(const Handle(DFBrowser_DFNode)& theNode) const;

  asiUI_EXPORT Standard_Integer NodeDepth(const Handle(DFBrowser_DFNode)& theNode) const;

  asiUI_EXPORT void Update() const;

  asiUI_EXPORT Standard_Address Pixmap(const DFBrowser_Picture theID) const;

  inline const Handle(DFBrowser_DFTree) & Tree() const
  {
    return myTree;
  }

  asiUI_EXPORT void SetLevel(const DFBrowser_Level theLevel);

  asiUI_EXPORT void SetAttrPrs(const DFBrowser_AttrPrs thePrs);

  asiUI_EXPORT Standard_Boolean DoFilter(const Handle(DFBrowser_DFNode)& theFrom);

  asiUI_EXPORT void ShowNS(const Standard_Boolean theShow);

  asiUI_EXPORT void Close();

 private:

  Handle(DFBrowser_DFTree) myTree;
  Standard_Address myFLGUI;
  Standard_Address myQTGUI;
};

#endif
