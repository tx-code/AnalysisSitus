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

#ifndef _DFBrowser_ListNode_h
#define _DFBrowser_ListNode_h

#include <DFBrowser_DFNode.hxx>

#include <Standard_CString.hxx>
#include <DFBrowser_NodeType.hxx>
#include <DFBrowser_AttrNode.hxx>

class DFBrowser_ListNode : public DFBrowser_DFNode
{
  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_ListNode, DFBrowser_DFNode)

public:

  asiUI_EXPORT DFBrowser_ListNode();

  asiUI_EXPORT virtual DFBrowser_NodeType GetType() const;

  asiUI_EXPORT virtual void Update();

  asiUI_EXPORT virtual const TCollection_AsciiString & Name();

  asiUI_EXPORT virtual void AddSub(Handle(DFBrowser_DFNode)& theNode);

  asiUI_EXPORT virtual Handle(DFBrowser_DFNode) Sub() const;

  inline const Handle(DFBrowser_AttrNode) & FirstAttribute() const
  {
    return myAttr;
  }

  inline void FirstAttribute(const Handle(DFBrowser_AttrNode)& theAttribute)
  {
    myAttr = theAttribute;
  }

  asiUI_EXPORT Handle(DFBrowser_AttrNode) LastAttribute() const;

  asiUI_EXPORT virtual void Del();

 private:

  Handle(DFBrowser_AttrNode) myAttr;
};

#endif
