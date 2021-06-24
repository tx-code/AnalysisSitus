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

#ifndef _DFBrowser_h
#define _DFBrowser_h

#include <asiUI.h>

#include <TDocStd_Document.hxx>

class DFBrowser
{
 public:

  asiUI_EXPORT static Standard_Integer DFBrowserCall(const Handle(TDocStd_Document)& theDoc);
  //! Loads DFBrowser functionality for not Draw based application. Used for plugin. <br>
  asiUI_EXPORT static Standard_Integer Factory(const Handle(TDocStd_Document)& theDoc);
};



#endif
