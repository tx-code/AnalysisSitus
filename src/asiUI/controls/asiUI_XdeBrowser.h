//-----------------------------------------------------------------------------
// Created on: 19 December 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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

#ifndef asmUI_XdeBrowser_h
#define asmUI_XdeBrowser_h

// asiUI includes
#include <asiUI_CommonFacilities.h>

// asiAsm includes
#include <asiAsm_XdeGraph.h>

// Qt includes
#pragma warning(push, 0)
#include <QTreeWidget>
#pragma warning(pop)

#pragma warning(disable : 4251)

//-----------------------------------------------------------------------------

// Qt role to store node ID near the tree item.
#define BrowserRoleNodeId Qt::UserRole

//-----------------------------------------------------------------------------

//! Tree view for browsing XDE assembly structure.
class asiUI_EXPORT asiUI_XdeBrowser : public QTreeWidget
{
  Q_OBJECT

public:

  //! Creates a new instance of tree view.
  //! \param[in] model  XDE document to browse.
  //! \param[in] cf     common facilities.
  //! \param[in] parent parent widget (if any).
  asiUI_XdeBrowser(const Handle(asiAsm_XdeDoc)&          doc,
                   const Handle(asiUI_CommonFacilities)& cf,
                   QWidget*                              parent = nullptr);

  //! Dtor.
  virtual ~asiUI_XdeBrowser();

public:

  //! Populates tree view from the Data Model.
  void Populate();

  //! Searches for an item with the given index and set that item selected.
  //! \param[in] nodeId target Node ID.
  void SetSelectedAssemblyItemId(const asiAsm_XdeAssemblyItemId& nodeId);

  //! Returns selected item.
  //! \return selected assembly item ID.
  asiAsm_XdeAssemblyItemId GetSelectedAssemblyItemId() const;

  //! \return ID of the selected node in the graph.
  int GetSelectedNodeId() const;

protected:

  //! Adds all child items under the given root.
  //! \param[in] rootId root item in a Data Model.
  //! \param[in] rootUi root item in a tree view.
  void addChildren(const int        rootId,
                   QTreeWidgetItem* rootUi);

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

  //! Copies assembly item ID to clipboard.
  void onCopyAssemblyItemId();

  //! Prints available part representations.
  void onPrintPartRepresentations();

  //! Shows part in the part view.
  void onShowPart();

  //! Shows subassembly in the assembly view.
  void onShowSubassembly();

  //! Sets active part.
  void onSetActivePart();

protected:

  Handle(asiAsm_XdeDoc)          m_doc;      //!< XDE document instance.
  Handle(asiAsm_XdeGraph)        m_asmGraph; //!< Assembly graph.
  Handle(asiUI_CommonFacilities) m_cf;       //!< Common UI facilities.

};

#pragma warning(default : 4251)

#endif
