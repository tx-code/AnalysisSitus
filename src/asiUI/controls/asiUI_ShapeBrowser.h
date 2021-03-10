//-----------------------------------------------------------------------------
// Created on: 09 March 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef asiUI_ShapeBrowser_h
#define asiUI_ShapeBrowser_h

// asiUI includes
#include <asiUI_CommonFacilities.h>

// Qt includes
#pragma warning(push, 0)
#include <QTreeWidget>
#pragma warning(pop)

#pragma warning(disable : 4251)

//-----------------------------------------------------------------------------

// Custom Qt role for the global shape index in an indexed map.
#define BrowserRoleShapeId Qt::UserRole

//-----------------------------------------------------------------------------

//! Tree view for browsing shapes.
class asiUI_EXPORT asiUI_ShapeBrowser : public QTreeWidget
{
  Q_OBJECT

public:

  //! Creates a new instance of tree view.
  //! \param[in] shape    shape to browse.
  //! \param[in] model    data model instance.
  //! \param[in] progress progress notifier.
  //! \param[in] parent   parent widget (if any).
  asiUI_ShapeBrowser(const TopoDS_Shape&            shape,
                     const Handle(asiEngine_Model)& model,
                     ActAPI_ProgressEntry           progress = nullptr,
                     QWidget*                       parent   = nullptr);

  //! Dtor.
  virtual ~asiUI_ShapeBrowser();

public:

  //! Populates tree view.
  void Populate();

  ////! Searches for an item with the given index and set that item selected.
  ////! \param[in] nodeId target Node ID.
  //void SetSelectedAssemblyItemId(const asiAsm_XdeAssemblyItemId& nodeId);

  ////! Returns selected item.
  ////! \return selected assembly item ID.
  //asiAsm_XdeAssemblyItemId GetSelectedAssemblyItemId() const;

  ////! \return ID of the selected node in the graph.
  //int GetSelectedNodeId() const;

protected:

  //! Adds all child items under the given shape.
  //! \param[in] shape   the shape to add children for.
  //! \param[in] shapeUi the corresponding UI item.
  void addChildren(const TopoDS_Shape& shape,
                   QTreeWidgetItem*    shapeUi);

//-----------------------------------------------------------------------------
signals:

  void nodeSelected();

//-----------------------------------------------------------------------------
protected slots:

  //! Reaction on selection in a tree view.
  void onSelectionChanged();

  //! Reaction on context menu opening.
  //! \param[in] pos position.
  void onContextMenu(QPoint pos);

  //! Copies the name of the selected tree object to clipboard.
  void onCopyName();

  //! Copies the global subshape's ID to the clipboard.
  void onCopyGlobalId();

  ////! Prints available part representations.
  //void onPrintPartRepresentations();

  ////! Shows part in the part view.
  //void onShowPart();

  ////! Shows subassembly in the assembly view.
  //void onShowSubassembly();

  ////! Sets active part.
  //void onSetActivePart();

protected:

  TopoDS_Shape               m_shape;        //!< Shape to expand for browsing.
  Handle(asiEngine_Model)    m_model;        //!< Data model instance.
  ActAPI_ProgressEntry       m_progress;     //!< Progress notifier.
  TopTools_IndexedMapOfShape m_subShapesMap; //!< Indexed map of subshapes.

};

#pragma warning(default : 4251)

#endif
