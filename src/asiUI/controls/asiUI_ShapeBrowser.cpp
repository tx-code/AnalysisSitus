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

// Own include
#include <asiUI_ShapeBrowser.h>

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

QIcon
  GetItemIcon(const TopoDS_Shape& shape)
{
  static QIcon icoCompound (":icons/asitus/shape_compound_icon_16x16");
  static QIcon icoSolid    (":icons/asitus/shape_solid_icon_16x16");
  static QIcon icoShell    (":icons/asitus/shape_shell_icon_16x16");
  static QIcon icoFace     (":icons/asitus/shape_face_icon_16x16");
  static QIcon icoWire     (":icons/asitus/shape_wire_icon_16x16");
  static QIcon icoEdge     (":icons/asitus/shape_edge_icon_16x16");
  static QIcon icoVertex   (":icons/asitus/shape_vertex_icon_16x16");
  static QIcon icoOther    (":icons/asitus/shape_other_icon_16x16");

  switch ( shape.ShapeType() )
  {
    case TopAbs_COMPOUND: return icoCompound;
    case TopAbs_SOLID:    return icoSolid;
    case TopAbs_SHELL:    return icoShell;
    case TopAbs_FACE:     return icoFace;
    case TopAbs_WIRE:     return icoWire;
    case TopAbs_EDGE:     return icoEdge;
    case TopAbs_VERTEX:   return icoVertex;
    default:              break;
  }

  return icoOther;
}

//-----------------------------------------------------------------------------

asiUI_ShapeBrowser::asiUI_ShapeBrowser(const TopoDS_Shape&            shape,
                                       const Handle(asiEngine_Model)& model,
                                       ActAPI_ProgressEntry           progress,
                                       QWidget*                       parent)
: QTreeWidget (parent),
  m_shape     (shape),
  m_model     (model),
  m_progress  (progress)
{
  // Configure.
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->setColumnCount(2);
  this->setAutoExpandDelay(0);
  this->setHeaderLabels( QStringList() << "Shape" << "Global index" );

  // Configure selection.
  this->setSelectionMode(QAbstractItemView::MultiSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Reactions.
  connect( this, SIGNAL( itemSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
  //
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  //
  connect( this, SIGNAL( customContextMenuRequested(QPoint) ), this, SLOT( onContextMenu(QPoint) ) );
}

//-----------------------------------------------------------------------------

asiUI_ShapeBrowser::~asiUI_ShapeBrowser()
{}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::Populate()
{
  // Check shape.
  if ( m_shape.IsNull() )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "Shape is null." );
    return;
  }

  // Clean up the existing contents.
  this->clear();

  // Index shape.
  TopExp::MapShapes(m_shape, m_subShapesMap);

  // Add root tree item.
  QTreeWidgetItem*
    rootUi = new QTreeWidgetItem( QStringList() << asiAlgo_Utils::ShapeTypeStr(m_shape).c_str()
                                                << "0" );
  //
  rootUi->setFlags ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  rootUi->setData  ( 0, BrowserRoleShapeId, 0 ); // Zero is reserved for the shape itself.
  rootUi->setIcon  ( 0, GetItemIcon(m_shape) );
  //
  this->addTopLevelItem(rootUi);
  this->addChildren(m_shape, rootUi);

  this->adjustSize();
}
//
////-----------------------------------------------------------------------------
//
//void asiUI_XdeBrowser::SetSelectedAssemblyItemId(const asiAsm_XdeAssemblyItemId& asiUI_NotUsed(nodeId))
//{
//}
//
////-----------------------------------------------------------------------------
//
//asiAsm_XdeAssemblyItemId asiUI_XdeBrowser::GetSelectedAssemblyItemId() const
//{
//  QList<QTreeWidgetItem*> items = this->selectedItems();
//  if ( !items.length() || items.length() > 1 )
//    return asiAsm_XdeAssemblyItemId();
//
//  QTreeWidgetItem* item = items.at(0);
//
//  // Loop over the parents to gather all persistent IDs.
//  asiAsm_XdeAssemblyItemId result;
//  do
//  {
//    // Get ID and type of the node in the assembly graph.
//    const int                 nid  = item->data(0, BrowserRoleShapeId).toInt();
//    asiAsm_XdeGraph::NodeType type = m_asmGraph->GetNodeType(nid);
//
//    // The assembly item ID does not contain prototypes' IDs except
//    // the root one by convention.
//    if ( ( (type != asiAsm_XdeGraph::NodeType_Part) &&
//           (type != asiAsm_XdeGraph::NodeType_Subassembly) ) || !item->parent() )
//    {
//      result.Prepend( QStr2AsciiStr( item->text(1) ) );
//    }
//
//    // Go upper in the hierarchy.
//    item = item->parent();
//  }
//  while ( item );
//
//  // Return the collected path as an assembly item ID.
//  return result;
//}
//
////-----------------------------------------------------------------------------
//
//int asiUI_XdeBrowser::GetSelectedNodeId() const
//{
//  QList<QTreeWidgetItem*> items = this->selectedItems();
//  if ( !items.length() || items.length() > 1 )
//    return 0;
//
//  QTreeWidgetItem* item = items.at(0);
//
//  return item->data(0, BrowserRoleShapeId).toInt();
//}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::addChildren(const TopoDS_Shape& shape,
                                     QTreeWidgetItem*    shapeUi)
{
  if ( shape.IsNull() )
    return;

  // Add child tree items.
  for ( TopoDS_Iterator it(shape); it.More(); it.Next() )
  {
    const TopoDS_Shape& childShape = it.Value();
    //
    QTreeWidgetItem*
      childUi = new QTreeWidgetItem( QStringList() << asiAlgo_Utils::ShapeTypeStr(childShape).c_str()
                                                   << QString::number( m_subShapesMap.FindIndex(childShape) ) );
    //
    childUi->setFlags ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    childUi->setData  ( 0, BrowserRoleShapeId, m_subShapesMap.FindIndex(childShape) );
    childUi->setIcon  ( 0, GetItemIcon(childShape) );
    //
    shapeUi->addChild(childUi);

    // Repeat recursively.
    this->addChildren(childShape, childUi);
  }
}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::onSelectionChanged()
{
  emit nodeSelected();
}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::onContextMenu(QPoint pos)
{
  QMenu* menu = new QMenu(this);

  //// Get ID of the corresponding graph node.
  //const int nid = this->GetSelectedNodeId();

  /* Populate the context menu. */

  menu->addAction( "Copy name(s)",   this, SLOT( onCopyName() ) );
  menu->addAction( "Copy global ID", this, SLOT( onCopyGlobalId() ) );

  //if ( nid )
  //{
  //  if ( m_asmGraph->GetNodeType(nid) == asiAsm_XdeGraph::NodeType_Part )
  //  {
  //    menu->addAction( "Print representations", this, SLOT( onPrintPartRepresentations() ) );
  //    menu->addAction( "Show part",             this, SLOT( onShowPart() ) );
  //    menu->addAction( "Set as active part",    this, SLOT( onSetActivePart() ) );
  //  }

  //  menu->addAction( "Show (sub)assembly", this, SLOT( onShowSubassembly() ) );
  //}

  menu->popup( this->mapToGlobal(pos) );
}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::onCopyName()
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() )
    return;

  QClipboard* clipboard = QApplication::clipboard();

  QStringList text;
  QListIterator<QTreeWidgetItem*> it(items);
  //
  while ( it.hasNext() )
  {
    QTreeWidgetItem* item = it.next();
    text << item->text(0);
  }

  clipboard->setText( text.join("\t") );
}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::onCopyGlobalId()
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() )
    return;

  QClipboard* clipboard = QApplication::clipboard();

  QStringList text;
  QListIterator<QTreeWidgetItem*> it(items);
  //
  while ( it.hasNext() )
  {
    QTreeWidgetItem* item = it.next();
    text << QString::number( item->data(0, BrowserRoleShapeId).toInt() );
  }

  clipboard->setText( text.join("\t") );
}

////-----------------------------------------------------------------------------
//
//void asiUI_XdeBrowser::onPrintPartRepresentations()
//{
//  // Get the selected node ID.
//  const int nid = this->GetSelectedNodeId();
//
//  // Get the corresponding part ID.
//  asiAsm_XdePartId partId = m_asmGraph->GetPersistentId(nid);
//
//  // Get available representations.
//  std::vector<Handle(asiAsm_XdePartRepr)> reps;
//  //
//  m_doc->GetPartRepresentations(partId, reps);
//
//  // Notify.
//  for ( auto rep : reps )
//  {
//    m_cf->Progress.SendLogMessage( LogInfo(Normal) << "\tNext available representation for [%1]: %2"
//                                                   << partId.Entry.ToCString()
//                                                   << rep->ToString() );
//  }
//}
//
////-----------------------------------------------------------------------------
//
//void asiUI_XdeBrowser::onShowPart()
//{
//  // Get the selected node ID.
//  const int nid = this->GetSelectedNodeId();
//
//  // Get the corresponding part ID.
//  asiAsm_XdePartId partId = m_asmGraph->GetPersistentId(nid);
//
//  // Get boundary representation.
//  Handle(asiAsm_XdePartBoundaryRepr) rep;
//  //
//  if ( !m_doc->GetPartRepresentation(partId, asiAsm_XdePartBoundaryRepr::GUID(), rep) )
//  {
//    m_cf->Progress.SendLogMessage(LogErr(Normal) << "The part %1 does not have a boundary representation.");
//    return;
//  }
//
//
//  TIMER_NEW
//  TIMER_GO
//
//  TCollection_AsciiString ivName("part "); ivName += partId.Entry;
//  //
//  m_cf->Plotter.REDRAW_SHAPE( ivName, rep->GetShape() );
//
//  TIMER_FINISH
//  TIMER_COUT_RESULT_NOTIFIER(m_cf->Progress, "Show (sub)assembly")
//}
//
////-----------------------------------------------------------------------------
//
//void asiUI_XdeBrowser::onShowSubassembly()
//{
//  // Get the selected node ID.
//  const asiAsm_XdeAssemblyItemId& parentId = this->GetSelectedAssemblyItemId();
//
//  TIMER_NEW
//  TIMER_GO
//
//  // Get leafs.
//  Handle(asiAsm_XdeHAssemblyItemIdsMap) leafs = new asiAsm_XdeHAssemblyItemIdsMap;
//  //
//  m_doc->GetLeafAssemblyItems(parentId, leafs);
//
//  // Draw each item individually.
//  for ( asiAsm_XdeHAssemblyItemIdsMap::Iterator it(*leafs); it.More(); it.Next() )
//  {
//    const asiAsm_XdeAssemblyItemId& aiid = it.Value();
//
//    // Get part.
//    asiAsm_XdePartId part;
//    m_doc->GetAsPartId(aiid, part);
//
//    // Get color.
//    Quantity_Color color;
//    if ( !m_doc->GetColor(part, color) )
//      color = Quantity_NOC_GREEN;
//
//    // Draw subassembly
//    TCollection_AsciiString ivName("assembly-item-"); ivName += aiid.ToString();
//    //
//    m_cf->Plotter.REDRAW_SHAPE( ivName, m_doc->GetShape(aiid),
//                                ActAPI_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB) );
//
//  }
//
//  TIMER_FINISH
//  TIMER_COUT_RESULT_NOTIFIER(m_cf->Progress, "Show (sub)assembly")
//}
//
////-----------------------------------------------------------------------------
//
//void asiUI_XdeBrowser::onSetActivePart()
//{
//  // Get the selected node ID.
//  const int nid = this->GetSelectedNodeId();
//
//  // Get the corresponding part ID.
//  asiAsm_XdePartId partId = m_asmGraph->GetPersistentId(nid);
//
//  // Get boundary representation.
//  Handle(asiAsm_XdePartBoundaryRepr) rep;
//  //
//  if ( !m_doc->GetPartRepresentation(partId, asiAsm_XdePartBoundaryRepr::GUID(), rep) )
//  {
//    m_cf->Progress.SendLogMessage(LogErr(Normal) << "The part %1 does not have a boundary representation.");
//    return;
//  }
//
//  // Update part.
//  m_cf->Model->OpenCommand(); // tx start
//  {
//    asiEngine_Part(m_cf->Model).Update(rep->GetShape(), nullptr, true);
//  }
//  m_cf->Model->CommitCommand(); // tx commit
//
//  // Actualize.
//  m_cf->ViewerPart->PrsMgr()->Actualize(m_cf->Model->GetPartNode(), false, true);
//}
