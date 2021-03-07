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

#ifndef _DFBrowser_AttrNode_h
#define _DFBrowser_AttrNode_h

#include <Handle_DFBrowser_AttrNode.hxx>
#include <DFBrowser_DFNode.hxx>
#include <TDF_Attribute.hxx>
#include <Standard_CString.hxx>

class DFBrowser_AttrNode : public DFBrowser_DFNode
{
 public:

  CAFBrowser_EXPORT DFBrowser_AttrNode();

  CAFBrowser_EXPORT virtual DFBrowser_NodeType GetType() const;

  CAFBrowser_EXPORT virtual void AddSub(Handle(DFBrowser_DFNode)& theNode);

  CAFBrowser_EXPORT virtual Handle(DFBrowser_DFNode) Sub() const;

  CAFBrowser_EXPORT virtual const TCollection_AsciiString & Name();

  CAFBrowser_EXPORT virtual void Update();

  inline const Handle(TDF_Attribute)& Attribute() const
  {
    return myAttr;
  }

  inline void Attribute(const Handle(TDF_Attribute)& theAttr)
  {
    myAttr = theAttr;
  }

  CAFBrowser_EXPORT virtual void Del();

  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_AttrNode, DFBrowser_DFNode)

 private:

  Handle(TDF_Attribute) myAttr;
};

#endif
