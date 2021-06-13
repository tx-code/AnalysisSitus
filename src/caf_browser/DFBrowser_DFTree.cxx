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

// Own include
#include <DFBrowser_DFTree.hxx>

// DF Browser includes
#include <DFBrowser_LabelNode.hxx>
#include <DFBrowser_Attr.hxx>
#include <DFBrowser_AttrNode.hxx>
#include <DFBrowser_GUI.hxx>

// OCCT includes
#include <TDocStd_Document.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>

static Standard_Integer myNbBrowsers = 0;

DFBrowser_DFTree::DFBrowser_DFTree (const Handle(TDocStd_Document)& theDoc)
: myDoc(theDoc),
  myRoot(new DFBrowser_LabelNode()),
  myAttr(new DFBrowser_Attr())
{
  myNbBrowsers++;
  if (!myDoc->Main().IsNull())
    myRoot->Label(myDoc->Main().Father());
  myRoot->Tree(this);
  myGUI = new DFBrowser_GUI(this);
}

DFBrowser_DFTree::DFBrowser_DFTree ()
: myRoot(new DFBrowser_LabelNode()),
  myAttr(new DFBrowser_Attr())
{
  myNbBrowsers++;
  myRoot->Tree(this);
  myGUI = new DFBrowser_GUI(this);
}

Handle(DFBrowser_LabelNode) DFBrowser_DFTree::Node (const TDF_Label& theLabel) const
{
  Handle(DFBrowser_LabelNode) aRes = myRoot;
  while(!aRes.IsNull()) {
    if (theLabel == aRes->Label()) return aRes;
    aRes = aRes->FirstLabel();
    while(!aRes.IsNull()) {
      if (theLabel.IsDescendant(aRes->Label())) break;
      aRes = Handle(DFBrowser_LabelNode)::DownCast(aRes->Next());
    }
  }
  return aRes;
}

Handle(DFBrowser_AttrNode) DFBrowser_DFTree::Node (const Handle(TDF_Attribute)& theAttr) const
{
  Handle(DFBrowser_AttrNode) aRes;
  Handle(DFBrowser_LabelNode) aLab = Node(theAttr->Label());
  if (aLab.IsNull()) return aRes;
  aRes = Handle(DFBrowser_AttrNode)::DownCast(aLab->Child());
  while(!aRes.IsNull()) {
    if (aRes->Attribute() == theAttr) return aRes;
    aRes = Handle(DFBrowser_AttrNode)::DownCast(aRes->Next());
  }
  return aRes;
}

Standard_Integer DFBrowser_DFTree::NbBrowsers ()
{
  return myNbBrowsers;
}

void DFBrowser_DFTree::Update ()
{
  myRoot->Update();
  myGUI->Update();
}

void DFBrowser_DFTree::Close ()
{
  myNbBrowsers--;

  if ( !myRoot.IsNull() )
  {
    myRoot->Del();
    myRoot.Nullify();
  }

  if ( !myDoc.IsNull() )
  {
    myDoc.Nullify();
  }

  if ( !myAttr.IsNull() )
  {
    myAttr.Nullify();
  }
}

void DFBrowser_DFTree::AddDocName (TCollection_AsciiString& theName) const
{
  if (myDoc->IsSaved()) theName = myDoc->GetName();
  else theName = "document";
}
