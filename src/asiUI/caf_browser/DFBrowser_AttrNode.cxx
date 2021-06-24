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

#include <DFBrowser_AttrNode.hxx>

#include <TDF_Attribute.hxx>
#include <DFBrowser_DFNode.hxx>
#include <DFBrowser_DFTree.hxx>
#include <DFBrowser_Attr.hxx>

DFBrowser_AttrNode::DFBrowser_AttrNode ()
{
  myPixmap = DFBrowser_STANDARD_ATTRIBUTE;
}

DFBrowser_NodeType DFBrowser_AttrNode::GetType () const
{
  return DFBrowser_ATTRIBUTE;
}

void DFBrowser_AttrNode::AddSub (Handle(DFBrowser_DFNode)& /*theNode*/)
{
  // standard attribute can't have substructure
}

Handle(DFBrowser_DFNode) DFBrowser_AttrNode::Sub () const
{
  return Handle(DFBrowser_DFNode)();
}

void DFBrowser_AttrNode::Update ()
{
  myName.Clear();
  Changed(Standard_True);
}

const TCollection_AsciiString & DFBrowser_AttrNode::Name ()
{
  if (myName.IsEmpty())
  {
    myName = Tree()->Attr()->LoadAttribute(myAttr);
    myColor = Tree()->Attr()->GetColor();
  }
  return myName;
}

void DFBrowser_AttrNode::Del ()
{
  myAttr.Nullify();
  DFBrowser_DFNode::Del();
}
