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

#ifndef _DFBrowser_LabelNode_h
#define _DFBrowser_LabelNode_h

#include <Handle_DFBrowser_LabelNode.hxx>
#include <DFBrowser_DFNode.hxx>

#include <Standard_CString.hxx>
#include <TDF_Label.hxx>
#include <Handle_DFBrowser_ListNode.hxx>
#include <DFBrowser_NodeType.hxx>

class DFBrowser_LabelNode : public DFBrowser_DFNode
{
 public:

  asiUI_EXPORT DFBrowser_LabelNode();

  asiUI_EXPORT virtual DFBrowser_NodeType GetType() const;

  asiUI_EXPORT virtual void AddSub(Handle(DFBrowser_DFNode)& theNode);

  asiUI_EXPORT virtual Handle(DFBrowser_DFNode) Sub() const;

  asiUI_EXPORT void DelSub(Handle(DFBrowser_DFNode)& theNode);

  asiUI_EXPORT virtual void Update();

  asiUI_EXPORT virtual const TCollection_AsciiString & Name();

  inline const TDF_Label & Label() const
  {
    return myLabel;
  }

  inline void Label(const TDF_Label& theLabel)
  {
    myLabel = theLabel;
  }

  inline const Handle(DFBrowser_DFNode) & Child() const
  {
    return myChild;
  }

  inline const Handle(DFBrowser_LabelNode) & FirstLabel() const
  {
    return myFirstLabel;
  }

  asiUI_EXPORT void AddList(const Standard_Boolean theAdd);

  inline const Handle(DFBrowser_ListNode) & List() const
  {
    return myList;
  }

  asiUI_EXPORT virtual void Del();

  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_LabelNode, DFBrowser_DFNode)

 private: 

  TDF_Label myLabel;
  Handle(DFBrowser_DFNode)    myChild;
  Handle(DFBrowser_LabelNode) myFirstLabel;
  Handle(DFBrowser_ListNode)  myList;
};

#endif
