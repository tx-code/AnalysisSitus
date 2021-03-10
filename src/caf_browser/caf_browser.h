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

#ifndef caf_browser_h
#define caf_browser_h

#define CAFBrowser_NotUsed(x)

#ifdef _WIN32
  #ifdef caf_browser_EXPORTS
    #define CAFBrowser_EXPORT __declspec(dllexport)
  #else
    #define CAFBrowser_EXPORT __declspec(dllimport)
  #endif
#else
  #define CAFBrowser_EXPORT
#endif

#endif
