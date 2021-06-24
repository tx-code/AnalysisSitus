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

#ifndef _DFBrowser_QTGUI_h
#define _DFBrowser_QTGUI_h

#include <asiUI.h>

#include <Standard_Address.hxx>
#include <TCollection_AsciiString.hxx>
#include <DFBrowser_PtrGUI.hxx>
#include <DFBrowser_Picture.hxx>

class DFBrowser_QTGUI
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

  asiUI_EXPORT DFBrowser_QTGUI(const DFBrowser_PtrGUI& theGUI);

  asiUI_EXPORT void Update();

  const TCollection_AsciiString & Filter() const;

  asiUI_EXPORT void Close();

  asiUI_EXPORT void Description(const TCollection_AsciiString &theDescription);

  asiUI_EXPORT const TCollection_AsciiString & Description() const;

  asiUI_EXPORT static Standard_Address Pixmap(const DFBrowser_Picture thePicture);
};

#endif
