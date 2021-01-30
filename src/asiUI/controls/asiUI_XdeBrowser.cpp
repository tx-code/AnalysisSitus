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

// Own include
#include <asiUI_XdeBrowser.h>

// asiAlgo includes
#include <asiAlgo_Timer.h>

// asiAsm includes
#include <asiAsm_XdePartRepr.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_IV.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItemIterator>
#pragma warning(pop)

#define TREEVIEW_MINSIZE 200

//-----------------------------------------------------------------------------

TCollection_AsciiString GetItemLabel(const int                      id,
                                     const Handle(asiAsm_XdeGraph)& asmGraph,
                                     const Handle(asiAsm_XdeDoc)&   asmModel)
{
  // Get name of the persistent object.
  TCollection_ExtendedString name;
  asmModel->GetObjectName(asmGraph->GetPersistentId(id), name);

  // Generate label.
  return TCollection_AsciiString(name);
}

//-----------------------------------------------------------------------------

QIcon
  GetItemIcon(const int                      id,
              const Handle(asiAsm_XdeGraph)& asmGraph)
{
  static QIcon icoRoot         (":icons/asitus/asm_root_icon_16x16");
  static QIcon icoSubassembly  (":icons/asitus/asm_subassembly_icon_16x16");
  static QIcon icoPartInstance (":icons/asitus/asm_part_instance_icon_16x16");
  static QIcon icoPart         (":icons/asitus/asm_part_icon_16x16");

  asiAsm_XdeGraph::NodeType nodeType = asmGraph->GetNodeType(id);

  if ( asmGraph->GetRoots().Contains(id) )
    return icoRoot;

  if ( nodeType == asiAsm_XdeGraph::NodeType_Subassembly )
    return icoSubassembly;

  if ( (nodeType == asiAsm_XdeGraph::NodeType_PartOccurrence) ||
       (nodeType == asiAsm_XdeGraph::NodeType_SubassemblyOccurrence) )
    return icoPartInstance;

  if ( nodeType == asiAsm_XdeGraph::NodeType_Part )
    return icoPart;

  return QIcon();
}

//-----------------------------------------------------------------------------

asiUI_XdeBrowser::asiUI_XdeBrowser(const Handle(asiAsm_XdeDoc)&          doc,
                                   const Handle(asiUI_CommonFacilities)& cf,
                                   QWidget*                              parent)
: QTreeWidget (parent),
  m_doc       (doc),
  m_cf        (cf)
{
  // Configure.
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->setColumnCount(2);
  this->setAutoExpandDelay(0);
  this->setHeaderLabels( QStringList() << "Assembly item" << "Persistent ID" );

  // Configure selection.
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Reactions.
  connect( this, SIGNAL( itemSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
  //
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  //
  connect( this, SIGNAL( customContextMenuRequested(QPoint) ), this, SLOT( onContextMenu(QPoint) ) );
}

//-----------------------------------------------------------------------------

asiUI_XdeBrowser::~asiUI_XdeBrowser()
{}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::Populate()
{
  // Check XDE document.
  if ( m_doc.IsNull() )
  {
    m_cf->Progress.SendLogMessage( LogErr(Normal) << "XDE document is not initialized." );
    return;
  }

  // Prepare assembly graph.
  m_asmGraph = new asiAsm_XdeGraph(m_doc);

  // Clean up the existing contents.
  this->clear();

  // Add tree items.
  const TColStd_PackedMapOfInteger& roots = m_asmGraph->GetRoots();
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger rit(roots); rit.More(); rit.Next() )
  {
    const int rootId = rit.Key();
    //
    QTreeWidgetItem*
      rootUi = new QTreeWidgetItem( QStringList() << GetItemLabel(rootId,
                                                                  m_asmGraph,
                                                                  m_doc).ToCString() );
    //
    rootUi->setText  ( 1, m_asmGraph->GetPersistentId(rootId).ToCString() );
    rootUi->setFlags ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    rootUi->setData  ( 0, BrowserRoleNodeId, rootId );
    rootUi->setIcon  ( 0, GetItemIcon(rootId, m_asmGraph) );
    //
    this->addTopLevelItem(rootUi);
    this->addChildren(rootId, rootUi);
  }

  this->adjustSize();
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::SetSelectedAssemblyItemId(const asiAsm_XdeAssemblyItemId& asiUI_NotUsed(nodeId))
{
}

//-----------------------------------------------------------------------------

asiAsm_XdeAssemblyItemId asiUI_XdeBrowser::GetSelectedAssemblyItemId() const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() || items.length() > 1 )
    return asiAsm_XdeAssemblyItemId();

  QTreeWidgetItem* item = items.at(0);

  // Loop over the parents to gather all persistent IDs.
  asiAsm_XdeAssemblyItemId result;
  do
  {
    // Get ID and type of the node in the assembly graph.
    const int                 nid  = item->data(0, BrowserRoleNodeId).toInt();
    asiAsm_XdeGraph::NodeType type = m_asmGraph->GetNodeType(nid);

    // The assembly item ID does not contain prototypes' IDs except
    // the root one by convention.
    if ( ( (type != asiAsm_XdeGraph::NodeType_Part) &&
           (type != asiAsm_XdeGraph::NodeType_Subassembly) ) || !item->parent() )
    {
      result.Prepend( QStr2AsciiStr( item->text(1) ) );
    }

    // Go upper in the hierarchy.
    item = item->parent();
  }
  while ( item );

  // Return the collected path as an assembly item ID.
  return result;
}

//-----------------------------------------------------------------------------

int asiUI_XdeBrowser::GetSelectedNodeId() const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() || items.length() > 1 )
    return 0;

  QTreeWidgetItem* item = items.at(0);

  return item->data(0, BrowserRoleNodeId).toInt();
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::addChildren(const int        rootId,
                                   QTreeWidgetItem* rootUi)
{
  if ( !m_asmGraph->HasChildren(rootId) )
    return;

  // Add child tree items.
  const TColStd_PackedMapOfInteger& children = m_asmGraph->GetChildren(rootId);
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger cit(children); cit.More(); cit.Next() )
  {
    const int childId = cit.Key();
    //
    QTreeWidgetItem*
      childUi = new QTreeWidgetItem( QStringList() << GetItemLabel(childId,
                                                                   m_asmGraph,
                                                                   m_doc).ToCString() );
    //
    childUi->setText  ( 1, m_asmGraph->GetPersistentId(childId).ToCString() );
    childUi->setFlags ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    childUi->setData  ( 0, BrowserRoleNodeId, childId );
    childUi->setIcon  ( 0, GetItemIcon(childId, m_asmGraph) );
    //
    rootUi->addChild(childUi);

    // Repeat recursively.
    this->addChildren(childId, childUi);
  }
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onSelectionChanged()
{
  emit nodeSelected();
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onContextMenu(QPoint pos)
{
  QMenu* menu = new QMenu(this);

  // Get ID of the corresponding graph node.
  const int nid = this->GetSelectedNodeId();

  // Complete context menu.
  // ...

  menu->addAction( "Copy name",             this, SLOT( onCopyName() ) );
  menu->addAction( "Copy assembly item ID", this, SLOT( onCopyAssemblyItemId() ) );

  if ( nid )
  {
    if ( m_asmGraph->GetNodeType(nid) == asiAsm_XdeGraph::NodeType_Part )
    {
      menu->addAction( "Print representations", this, SLOT( onPrintPartRepresentations() ) );
      menu->addAction( "Show part",             this, SLOT( onShowPart() ) );
      menu->addAction( "Set as active part",    this, SLOT( onSetActivePart() ) );
    }

    menu->addAction( "Show (sub)assembly", this, SLOT( onShowSubassembly() ) );
  }

  menu->popup( this->mapToGlobal(pos) );
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onCopyName()
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() || items.length() > 1 )
    return;

  QTreeWidgetItem* item = items.at(0);

  // Set to clipboard.
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText( item->text(0) );
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onCopyAssemblyItemId()
{
  // Get the selected assembly item ID.
  asiAsm_XdeAssemblyItemId aiid = this->GetSelectedAssemblyItemId();

  // Set to clipboard.
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText( AsciiStr2QStr( aiid.ToString() ) );

  // Notify.
  m_cf->Progress.SendLogMessage( LogInfo(Normal) << "Selected assembly item ID: %1"
                                                 << aiid.ToString() );
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onPrintPartRepresentations()
{
  // Get the selected node ID.
  const int nid = this->GetSelectedNodeId();

  // Get the corresponding part ID.
  asiAsm_XdePartId partId = m_asmGraph->GetPersistentId(nid);

  // Get available representations.
  std::vector<Handle(asiAsm_XdePartRepr)> reps;
  //
  m_doc->GetPartRepresentations(partId, reps);

  // Notify.
  for ( auto rep : reps )
  {
    m_cf->Progress.SendLogMessage( LogInfo(Normal) << "\tNext available representation for [%1]: %2"
                                                   << partId.Entry.ToCString()
                                                   << rep->ToString() );
  }
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onShowPart()
{
  // Get the selected node ID.
  const int nid = this->GetSelectedNodeId();

  // Get the corresponding part ID.
  asiAsm_XdePartId partId = m_asmGraph->GetPersistentId(nid);

  // Get boundary representation.
  Handle(asiAsm_XdePartBoundaryRepr) rep;
  //
  if ( !m_doc->GetPartRepresentation(partId, asiAsm_XdePartBoundaryRepr::GUID(), rep) )
  {
    m_cf->Progress.SendLogMessage(LogErr(Normal) << "The part %1 does not have a boundary representation.");
    return;
  }


  TIMER_NEW
  TIMER_GO

  TCollection_AsciiString ivName("part "); ivName += partId.Entry;
  //
  m_cf->Plotter.REDRAW_SHAPE( ivName, rep->GetShape() );

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_cf->Progress, "Show (sub)assembly")
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onShowSubassembly()
{
  // Get the selected node ID.
  const asiAsm_XdeAssemblyItemId& parentId = this->GetSelectedAssemblyItemId();

  TIMER_NEW
  TIMER_GO

  // Get leafs.
  Handle(asiAsm_XdeHAssemblyItemIdsMap) leafs = new asiAsm_XdeHAssemblyItemIdsMap;
  //
  m_doc->GetLeafAssemblyItems(parentId, leafs);

  // Draw each item individually.
  for ( asiAsm_XdeHAssemblyItemIdsMap::Iterator it(*leafs); it.More(); it.Next() )
  {
    const asiAsm_XdeAssemblyItemId& aiid = it.Value();

    // Get part.
    asiAsm_XdePartId part;
    m_doc->GetAsPartId(aiid, part);

    // Get color.
    Quantity_Color color;
    if ( !m_doc->GetColor(part, color) )
      color = Quantity_NOC_GREEN;

    // Draw subassembly
    TCollection_AsciiString ivName("assembly-item-"); ivName += aiid.ToString();
    //
    m_cf->Plotter.REDRAW_SHAPE( ivName, m_doc->GetShape(aiid),
                                ActAPI_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB) );

  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_cf->Progress, "Show (sub)assembly")
}

//-----------------------------------------------------------------------------

void asiUI_XdeBrowser::onSetActivePart()
{
  // Get the selected node ID.
  const int nid = this->GetSelectedNodeId();

  // Get the corresponding part ID.
  asiAsm_XdePartId partId = m_asmGraph->GetPersistentId(nid);

  // Get boundary representation.
  Handle(asiAsm_XdePartBoundaryRepr) rep;
  //
  if ( !m_doc->GetPartRepresentation(partId, asiAsm_XdePartBoundaryRepr::GUID(), rep) )
  {
    m_cf->Progress.SendLogMessage(LogErr(Normal) << "The part %1 does not have a boundary representation.");
    return;
  }

  // Update part.
  m_cf->Model->OpenCommand(); // tx start
  {
    asiEngine_Part(m_cf->Model).Update(rep->GetShape(), nullptr, true);
  }
  m_cf->Model->CommitCommand(); // tx commit

  // Actualize.
  m_cf->ViewerPart->PrsMgr()->Actualize(m_cf->Model->GetPartNode(), false, true);
}
