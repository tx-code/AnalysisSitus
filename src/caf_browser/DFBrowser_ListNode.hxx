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

#include <Handle_DFBrowser_ListNode.hxx>
#include <DFBrowser_DFNode.hxx>

#include <Standard_CString.hxx>
#include <Handle_DFBrowser_AttrNode.hxx>
#include <DFBrowser_NodeType.hxx>

class DFBrowser_ListNode : public DFBrowser_DFNode
{
 public:

  CAFBrowser_EXPORT DFBrowser_ListNode();

  CAFBrowser_EXPORT virtual DFBrowser_NodeType GetType() const;

  CAFBrowser_EXPORT virtual void Update();

  CAFBrowser_EXPORT virtual const TCollection_AsciiString & Name();

  CAFBrowser_EXPORT virtual void AddSub(Handle(DFBrowser_DFNode)& theNode);

  CAFBrowser_EXPORT virtual Handle(DFBrowser_DFNode) Sub() const;

  inline const Handle(DFBrowser_AttrNode) & FirstAttribute() const
  {
    return myAttr;
  }

  inline void FirstAttribute(const Handle(DFBrowser_AttrNode)& theAttribute)
  {
    myAttr = theAttribute;
  }

  CAFBrowser_EXPORT Handle(DFBrowser_AttrNode) LastAttribute() const;

  CAFBrowser_EXPORT virtual void Del();

  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_ListNode, DFBrowser_DFNode)

 private: 

  Handle_DFBrowser_AttrNode myAttr;
};

#endif
