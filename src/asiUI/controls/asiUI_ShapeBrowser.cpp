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
                                       ActAPI_PlotterEntry            plotter,
                                       QWidget*                       parent)
: QTreeWidget (parent),
  m_shape     (shape),
  m_model     (model),
  m_progress  (progress),
  m_plotter   (plotter)
{
  // Configure.
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->setColumnCount(2);
  this->setAutoExpandDelay(0);
  this->setHeaderLabels( QStringList() << "Shape" << "Global index" );

  // Configure selection.
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::GetSelectedIds(std::vector<int>& ids) const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() )
    return;

  QListIterator<QTreeWidgetItem*> it(items);
  //
  while ( it.hasNext() )
  {
    QTreeWidgetItem* item = it.next();
    ids.push_back( item->data(0, BrowserRoleShapeId).toInt() );
  }
}

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

  /* Populate the context menu. */

  // Get IDs of the selected tree nodes.
  std::vector<int> selectedIds;
  this->GetSelectedIds(selectedIds);

  if ( !selectedIds.empty() )
  {
    menu->addAction( "Display", this, SLOT( onDisplay() ) );
  }

  menu->addSeparator();
  menu->addAction( "Copy name(s)",      this, SLOT( onCopyNames() ) );
  menu->addAction( "Copy global ID(s)", this, SLOT( onCopyGlobalIds() ) );

  menu->popup( this->mapToGlobal(pos) );
}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::onCopyNames()
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

void asiUI_ShapeBrowser::onCopyGlobalIds()
{
  std::vector<int> selectedIds;
  this->GetSelectedIds(selectedIds);

  QStringList text;
  //
  for ( auto id : selectedIds )
  {
    text << QString::number(id);
  }

  QApplication::clipboard()->setText( text.join("\t") );
}

//-----------------------------------------------------------------------------

void asiUI_ShapeBrowser::onDisplay()
{
  if ( m_plotter.Access().IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Display is disabled: no plotter available.");
    return;
  }

  // Get IDs of the selected tree nodes.
  std::vector<int> selectedIds;
  this->GetSelectedIds(selectedIds);

  // Display each shape.
  for ( auto id : selectedIds )
  {
    if ( id < 0 || id > m_subShapesMap.Extent() )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Index %1 is out of range." << id);
      continue;
    }

    TopoDS_Shape shape = ( (id == 0) ? m_shape : m_subShapesMap(id) );

    TCollection_AsciiString name = asiAlgo_Utils::ShapeTypeStr(shape).c_str();
    name += "_";
    name += id;

    m_plotter.REDRAW_SHAPE(name, shape);
  }
}
