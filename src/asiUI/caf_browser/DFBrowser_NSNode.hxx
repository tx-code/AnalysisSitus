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

#ifndef _DFBrowser_NSNode_h
#define _DFBrowser_NSNode_h

#include <Handle_DFBrowser_NSNode.hxx>
#include <DFBrowser_AttrNode.hxx>
#include <TNaming_NamedShape.hxx>
#include <TopoDS_Shape.hxx>
#include <Handle_DFBrowser_DFNode.hxx>

class DFBrowser_NSNode : public DFBrowser_AttrNode
{
 public:

  asiUI_EXPORT DFBrowser_NSNode();

  asiUI_EXPORT virtual void AddSub(Handle(DFBrowser_DFNode)& theNode);

  asiUI_EXPORT virtual Handle(DFBrowser_DFNode) Sub() const;

  asiUI_EXPORT virtual void Update();

  asiUI_EXPORT Handle(TNaming_NamedShape) NamedShape() const;

  asiUI_EXPORT void NamedShape(const Handle(TNaming_NamedShape)& theNS);

  inline Standard_Boolean Structure() const
  {
    return myStructure;
  }

  inline void Structure(const Standard_Boolean theSet)
  {
    if (!myIsShape) myStructure = theSet;
  }

  asiUI_EXPORT void SetShape(const TopoDS_Shape& theNewShape,
                                const TopoDS_Shape& theOldShape,
                                const Standard_Boolean theVertices,
                                const Standard_Boolean theNew);

  inline const TopoDS_Shape & GetShape() const
  {
    return myShape;
  }

  asiUI_EXPORT virtual void Del();

  DEFINE_STANDARD_RTTI_INLINE(DFBrowser_NSNode, DFBrowser_AttrNode)

 private:

  Standard_Boolean myStructure;
  Handle(DFBrowser_DFNode) myFirst;
  Standard_Boolean myIsShape;
  TopoDS_Shape myShape;
};

#endif
