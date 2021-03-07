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

#include <caf_browser.h>

#include <Handle_DFBrowser_DFTree.hxx>
#include <Handle_DFBrowser_DFNode.hxx>
#include <DFBrowser_Picture.hxx>
#include <DFBrowser_Level.hxx>
#include <DFBrowser_AttrPrs.hxx>

class DFBrowser_DFTree;
class DFBrowser_DFNode;

class DFBrowser_GUI
{
 public:

  void* operator new(size_t,void* anAddress) 
  {
    return anAddress;
  }
  void* operator new(size_t size) 
  {
    return Standard::Allocate(size); 
  }
  void  operator delete(void *anAddress) 
  {
    if (anAddress) Standard::Free((Standard_Address&)anAddress); 
  }

  CAFBrowser_EXPORT DFBrowser_GUI(const Handle(DFBrowser_DFTree)& theTree);

  CAFBrowser_EXPORT void Selected(const Handle(DFBrowser_DFNode)& theNode);

  CAFBrowser_EXPORT void DoubleClicked(const Handle(DFBrowser_DFNode)& theNode) const;

  CAFBrowser_EXPORT const Handle(DFBrowser_DFNode) & First() const;

  CAFBrowser_EXPORT Handle(DFBrowser_DFNode) Next(const Handle(DFBrowser_DFNode)& theNode) const;

  CAFBrowser_EXPORT Handle(DFBrowser_DFNode) NextVisible(const Handle(DFBrowser_DFNode)& theNode) const;

  CAFBrowser_EXPORT Standard_Integer NodeDepth(const Handle(DFBrowser_DFNode)& theNode) const;

  CAFBrowser_EXPORT void Update() const;

  CAFBrowser_EXPORT Standard_Address Pixmap(const DFBrowser_Picture theID) const;

  inline const Handle(DFBrowser_DFTree) & Tree() const
  {
    return myTree;
  }

  CAFBrowser_EXPORT void SetLevel(const DFBrowser_Level theLevel);

  CAFBrowser_EXPORT void SetAttrPrs(const DFBrowser_AttrPrs thePrs);

  CAFBrowser_EXPORT Standard_Boolean DoFilter(const Handle(DFBrowser_DFNode)& theFrom);

  CAFBrowser_EXPORT void ShowNS(const Standard_Boolean theShow);

  CAFBrowser_EXPORT void Close();

 private:

  Handle_DFBrowser_DFTree myTree;
  Standard_Address myFLGUI;
  Standard_Address myQTGUI;
};

#endif
