//-----------------------------------------------------------------------------
// Created on: 03 December 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiUI_ObjectBrowser.h>

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_DependencyGraph.h>
#include <asiUI_DialogFeatureComments.h>
#include <asiUI_DialogDump.h>
#include <asiUI_DialogOCAFDump.h>
#include <asiUI_DialogPipelines.h>
#include <asiUI_ShapeBrowser.h>
#include <asiUI_ViewerPart.h>

// asiEngine includes
#include <asiEngine_IV.h>
#include <asiEngine_Part.h>
#include <asiEngine_PatchJointAdaptor.h>
#include <asiEngine_RE.h>
#include <asiEngine_Tessellation.h>

// asiVisu includes
#include <asiVisu_PartPrs.h>

// asiAlgo includes
#include <asiAlgo_JSON.h>
#include <asiAlgo_MeshConvert.h>
#include <asiAlgo_PatchJointAdaptor.h>
#include <asiAlgo_Utils.h>
#include <asiAlgo_WriteREK.h>
#include <asiAlgo_WriteSVG.h>

// OpenCascade includes
#include <BOPAlgo_Builder.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TDF_Tool.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItemIterator>
#pragma warning(pop)

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

#define TREEVIEW_MINSIZE 200

//-----------------------------------------------------------------------------

namespace
{
  void PropagateFlag(QTreeWidgetItem* pItem, Qt::CheckState state)
  {
    if ( pItem->flags() & Qt::ItemIsUserCheckable )
      pItem->setCheckState(1, state);

    // Proceed with its children.
    for ( int c = 0; c < pItem->childCount(); ++c )
      PropagateFlag(pItem->child(c), state);
  }

  QTreeWidgetItem*
    GetItem(QTreeWidget*               pTreeWidget,
            const ActAPI_DataObjectId& nodeId)
  {
    QTreeWidgetItemIterator it(pTreeWidget);
    //
    while ( *it )
    {
      QString itemData = (*it)->data(0, BrowserRoleNodeId).toString();
      if ( QStr2AsciiStr(itemData) == nodeId )
      {
        return *it;
      }
      else
        ++it;
    }

    return nullptr;
  }
}

//-----------------------------------------------------------------------------

asiUI_ObjectBrowser::asiUI_ObjectBrowser(const Handle(asiEngine_Model)& model,
                                         ActAPI_ProgressEntry           progress,
                                         QWidget*                       parent)
: QTreeWidget(parent), m_model(model), m_progress(progress)
{
  // Configure.
  this->setMinimumWidth(TREEVIEW_MINSIZE);
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->header()->hide();
  this->setColumnCount(2);
  this->setAutoExpandDelay(0);

  // Populate.
  this->Populate();

  // Configure selection.
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectRows);

  /* Reactions. */

  connect( this, SIGNAL( itemSelectionChanged() ), this, SLOT( onSelectionChanged() ) );

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  connect( this, SIGNAL( customContextMenuRequested(QPoint) ), this, SLOT( onContextMenu(QPoint) ) );

  connect( this->model(), SIGNAL( dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&) ),
           this,          SLOT( onDataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&) ) );
}

//-----------------------------------------------------------------------------

asiUI_ObjectBrowser::~asiUI_ObjectBrowser()
{}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::SetPlotter(ActAPI_PlotterEntry plotter)
{
  m_plotter = plotter;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::AddAssociatedViewer(asiUI_Viewer* pViewer)
{
  m_viewers.push_back(pViewer);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::SetParameterEditor(const Handle(asiUI_IParamEditor)& editor)
{
  m_paramEditor = editor;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::Populate()
{
  // Get selected Node (if any).
  Handle(ActAPI_INode) selN;
  this->selectedNode(selN);

  // Clean up the existing contents.
  this->clear();

  // Add root node.
  Handle(ActAPI_INode) root_n = m_model->GetRootNode();
  //
  if ( root_n.IsNull() || !root_n->IsWellFormed() )
    return;
  //
  QTreeWidgetItem* root_ui = new QTreeWidgetItem( QStringList() << ExtStr2QStr( root_n->GetName() ) );
  root_ui->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  root_ui->setData( 0, BrowserRoleNodeId, AsciiStr2QStr( root_n->GetId() ) );
  //
  this->addTopLevelItem(root_ui);

  const bool isHidden = ( root_n->GetUserFlags() & NodeFlag_IsHiddenInBrowser ) > 0;
  root_ui->setHidden(isHidden);

  // Add child nodes.
  this->addChildren(root_n, root_ui);

  // Expand tree.
  this->expandAll();

  // Reselect Node.
  if ( !selN.IsNull() )
    this->SetSelectedNode( selN->GetId() );

  // Beautify.
  this->resizeColumnToContents(0);
  this->resizeColumnToContents(1);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::UpdateSelectedNode()
{
  Handle(ActAPI_INode) N;
  QTreeWidgetItem*     pItem = nullptr;

  if ( !this->selectedNode(N, pItem) )
    return;

  if ( pItem )
  {
    pItem->setText( 0, ExtStr2QStr( N->GetName() ) );

    // Do any other actualization on the widget item.
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::SetSelectedNode(const ActAPI_DataObjectId& nodeId)
{
  // Clear current selection.
  this->selectionModel()->clearSelection();

  // Find the Node asked for selection and select it.
  QTreeWidgetItemIterator it(this);
  //
  while ( *it )
  {
    QString itemData = (*it)->data(0, BrowserRoleNodeId).toString();
    if ( QStr2AsciiStr(itemData) == nodeId )
    {
      (*it)->setSelected(true);
      break;
    }
    else
      ++it;
  }
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiUI_ObjectBrowser::GetSelectedNode() const
{
  Handle(ActAPI_INode) selected;
  if ( !this->selectedNode(selected) )
    return nullptr;

  return selected;
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiUI_ObjectBrowser::GetSelectedNode(QTreeWidgetItem*& pItem) const
{
  Handle(ActAPI_INode) selected;
  if ( !this->selectedNode(selected, pItem) )
    return nullptr;

  return selected;
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HNodeList) asiUI_ObjectBrowser::GetSelectedNodes() const
{
  Handle(ActAPI_HNodeList) selected;
  if ( !this->selectedNodes(selected) )
    return nullptr;

  return selected;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::addChildren(const Handle(ActAPI_INode)& root_n,
                                      QTreeWidgetItem*            root_ui)
{
  if ( root_n.IsNull() )
    return;

  // Allow adding bad-formed Nodes, just mark them specifically.
  if ( !root_n->IsWellFormed() )
    root_ui->setBackgroundColor(0, Qt::darkRed);

  // Loop over the children.
  for ( Handle(ActAPI_IChildIterator) cit = root_n->GetChildIterator(); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) child_n = cit->Value();

    // Create child.
    QTreeWidgetItem* child_ui = new QTreeWidgetItem( QStringList() << ExtStr2QStr( child_n->GetName() ) );

    // Configure child.
    child_ui->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    //
    if ( child_n->GetUserFlags() & NodeFlag_IsPresentedInPartView )
    {
      child_ui->setFlags(child_ui->flags() | Qt::ItemIsUserCheckable);
      child_ui->setCheckState(1, (child_n->GetUserFlags() & NodeFlag_IsPresentationVisible) ? Qt::Checked : Qt::Unchecked);
    }
    //
    child_ui->setData( 0, BrowserRoleNodeId, AsciiStr2QStr( child_n->GetId() ) );

    // Add child.
    root_ui->addChild(child_ui);

    // Check user flags which may customize visibility of an item.
    const bool isHidden = ( child_n->GetUserFlags() & NodeFlag_IsHiddenInBrowser ) > 0;
    child_ui->setHidden(isHidden);

    // Repeat recursively.
    this->addChildren(child_n, child_ui);
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::deleteChildrenUI(QTreeWidgetItem* root_ui)
{
  if ( root_ui == nullptr )
  {
    return;
  }

  QList<QTreeWidgetItem*> children = root_ui->takeChildren();
  QList<QTreeWidgetItem*>::const_iterator itChildren = children.cbegin();

  for ( ; itChildren != children.cend(); ++itChildren )
  {
    QTreeWidgetItem* child = *itChildren;
    if ( child == nullptr )
    {
      continue;
    }

    deleteChildrenUI(child);

    if ( root_ui != nullptr && root_ui != child && child != nullptr )
    {
      // The removed item will not be deleted.
      root_ui->removeChild(child);
      delete child;
    }
  }

}

//-----------------------------------------------------------------------------

bool asiUI_ObjectBrowser::deleteChildrenNodes(const Handle(ActAPI_INode)& root_n)
{
  if ( root_n.IsNull() || !root_n->IsWellFormed() || root_n->GetUserFlags() & NodeFlag_IsStructural )
    return false;

  for ( Handle(ActAPI_IChildIterator) cit = root_n->GetChildIterator(); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) child_n = cit->Value();

    if ( child_n.IsNull()                                ||
         !child_n->IsWellFormed()                        ||
         child_n->GetUserFlags() & NodeFlag_IsStructural )
    {
      return false;
    }

    if ( child_n->GetChildIterator()->More() && !deleteChildrenNodes(child_n) )
    {
      return false;
    }

    for ( size_t k = 0; k < m_viewers.size(); ++k )
    {
      if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(child_n) )
        m_viewers[k]->PrsMgr()->DeRenderPresentation(child_n);
    }

    if ( !child_n->GetParentNode().IsNull() && child_n->GetParentNode()->IsWellFormed() &&
         !child_n->GetParentNode()->RemoveChildNode(child_n) )
    {
      return false;
    }

    if ( !m_model->DeleteNode(child_n) )
    {
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiUI_ObjectBrowser::checkIsNotStructural(const Handle(ActAPI_INode)& root_n)
{
  if ( root_n.IsNull() || !root_n->IsWellFormed() || root_n->GetUserFlags() & NodeFlag_IsStructural )
    return false;

  for ( Handle(ActAPI_IChildIterator) cit = root_n->GetChildIterator(); cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) child_n = cit->Value();

    if ( child_n.IsNull() || !child_n->IsWellFormed() || child_n->GetUserFlags() & NodeFlag_IsStructural )
    {
      return false;
    }

    if ( child_n->GetChildIterator()->More() && !checkIsNotStructural(child_n) )
    {
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onDataChanged(const QModelIndex&  topLeft,
                                        const QModelIndex&  asiUI_NotUsed(bottomRight),
                                        const QVector<int>& asiUI_NotUsed(roles))
{
  if ( topLeft.column() == 1 )
  {
    QTreeWidgetItem*         item_ui = this->itemFromIndex(topLeft);
    TCollection_AsciiString  entry   = QStr2AsciiStr( item_ui->data(0, BrowserRoleNodeId).toString() );
    Handle(ActAPI_INode)     N       = m_model->FindNode(entry);
    Handle(ActAPI_HNodeList) nodes   = new ActAPI_HNodeList;
    nodes->Append(N);

    if ( item_ui->checkState(1) == Qt::Checked )
    {
      this->showNodes(nodes);
    }
    else
    {
      this->hideNodes(nodes);
    }
  }
}

//-----------------------------------------------------------------------------

//! Reaction on selection in a tree view.
void asiUI_ObjectBrowser::onSelectionChanged()
{
  // Populate parameter editor.
  ActAPI_DataObjectIdList selIds;
  //
  if ( !m_paramEditor.IsNull() )
  {
    Handle(ActAPI_HNodeList) nodes = this->GetSelectedNodes();
    //
    if ( nodes.IsNull() || nodes->Length() != 1 )
      m_paramEditor->SetParameters( nullptr );
    else
      m_paramEditor->SetParameters( nodes->First()->Parameters() );

    // Collect IDs of the selected Nodes to pass them to the listeners.
    if ( !nodes.IsNull() )
      for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
        selIds.Append( nit.Value()->GetId() );
  }

  emit nodesSelected(selIds);
}

//-----------------------------------------------------------------------------

//! Reaction on context menu opening.
//! \param[in] pos position.
void asiUI_ObjectBrowser::onContextMenu(QPoint pos)
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  // Create and populate the menu.
  QMenu* pMenu = new QMenu(this);
  //
  this->populateContextMenu(selected_n, pMenu);
  //
  if ( !pMenu->isEmpty() )
    pMenu->popup( this->mapToGlobal(pos) );
}

//-----------------------------------------------------------------------------

//! Reaction on "show" action.
void asiUI_ObjectBrowser::onShow()
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  // Modify UI.
  this->blockSignals(true);
  this->model()->blockSignals(true);
  {
    QList<QTreeWidgetItem*> items = this->selectedItems();
    for ( QList<QTreeWidgetItem*>::iterator iit = items.begin(); iit != items.end(); ++iit )
    {
      if ( (*iit)->flags() & Qt::ItemIsUserCheckable )
        (*iit)->setCheckState(1, Qt::Checked);
    }
  }
  this->blockSignals(false);
  this->model()->blockSignals(false);

  // Modify Data Model.
  this->showNodes(selected_n);
}

//-----------------------------------------------------------------------------

//! Reaction on "show only" action.
void asiUI_ObjectBrowser::onShowOnly()
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  this->blockSignals(true);
  this->model()->blockSignals(true);
  {
    // Hide all items.
    for ( int i = 0; i < this->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem* pItem = this->topLevelItem(i);
      PropagateFlag(pItem, Qt::Unchecked);
    }

    // Check items which go visible.
    QList<QTreeWidgetItem*> items = this->selectedItems();
    for ( QList<QTreeWidgetItem*>::iterator iit = items.begin(); iit != items.end(); ++iit )
    {
      if ( (*iit)->flags() & Qt::ItemIsUserCheckable )
        (*iit)->setCheckState(1, Qt::Checked);
    }
  }
  this->blockSignals(false);
  this->model()->blockSignals(false);

  // Modify Data Model.
  this->showOnlyNodes(selected_n);
}

//-----------------------------------------------------------------------------

//! Reaction on "hide" action.
void asiUI_ObjectBrowser::onHide()
{
  Handle(ActAPI_HNodeList) selected_n;
  if ( !this->selectedNodes(selected_n) ) return;

  // Modify UI.
  this->blockSignals(true);
  this->model()->blockSignals(true);
  {
    QList<QTreeWidgetItem*> items = this->selectedItems();
    for ( QList<QTreeWidgetItem*>::iterator iit = items.begin(); iit != items.end(); ++iit )
    {
      if ( (*iit)->flags() & Qt::ItemIsUserCheckable )
        (*iit)->setCheckState(1, Qt::Unchecked);
    }
  }
  this->blockSignals(false);
  this->model()->blockSignals(false);

  // Modify Data Model.
  this->hideNodes(selected_n);
}

//-----------------------------------------------------------------------------

//! Reaction on "manage pipelines" action.
void asiUI_ObjectBrowser::onManagePipelines()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Iterate over the associated viewers to find presentation.
  Handle(asiVisu_Prs)                 prs;
  vtkSmartPointer<asiVisu_PrsManager> prsMgr;
  //
  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      prsMgr = m_viewers[k]->PrsMgr();
      prs    = prsMgr->GetPresentation(selected_n);
      break;
    }

  // Show dialog to manage presentation.
  if ( !prs.IsNull() )
  {
    asiUI_DialogPipelines* pDlg = new asiUI_DialogPipelines(prs, prsMgr, m_progress, this);
    pDlg->show();
  }
  else
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot manage pipelines for a non-presented Node.");
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onShowExecutionGraph()
{
  asiUI_DependencyGraph*
    pGraph = new asiUI_DependencyGraph(m_model);
  //
  pGraph->Render();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onPrintMetadataContents()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_MetadataNode) ) )
    return;

  Handle(asiData_MetadataNode)
    meta_n = Handle(asiData_MetadataNode)::DownCast(selected_n);

  asiData_MetadataAttr::t_shapeColorMap map;
  meta_n->GetShapeColorMap(map);

  m_progress.SendLogMessage(LogInfo(Normal) << "Num. of metadata records: %1."
                                            << map.Extent());
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onHidePartEdges()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
    return;

  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      Handle(asiVisu_PartPrs)
        prs = Handle(asiVisu_PartPrs)::DownCast( m_viewers[k]->PrsMgr()->GetPresentation(selected_n) );

      prs->ContourActor()->SetVisibility(0);

      m_viewers[k]->Repaint();
    }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onShowPartEdges()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
    return;

  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      Handle(asiVisu_PartPrs)
        prs = Handle(asiVisu_PartPrs)::DownCast( m_viewers[k]->PrsMgr()->GetPresentation(selected_n) );

      prs->ContourActor()->SetVisibility(1);

      m_viewers[k]->Repaint();
    }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onResetPartPrs()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
    return;

  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
    {
      m_viewers[k]->PrsMgr()->DeletePresentation(selected_n);
      m_viewers[k]->PrsMgr()->Actualize(selected_n, false, true);
    }
}
//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToSVG()
{
  Handle(ActAPI_HNodeList) sel;
  if ( !this->selectedNodes(sel) ) return;

  TopoDS_Compound curvesComp;
  BRep_Builder bbuilder;
  bbuilder.MakeCompound(curvesComp);

  for ( ActAPI_NodeList::Iterator nit(*sel); nit.More(); nit.Next() )
  {
    Handle(ActAPI_INode) node = nit.Value();
    //
    if ( !node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
      continue;

    Handle(Geom_Curve)
      curve = Handle(asiData_IVCurveNode)::DownCast(node)->GetCurve();

    bbuilder.Add( curvesComp, BRepBuilderAPI_MakeEdge(curve) );
  }

  QString
    filename = asiUI_Common::selectGraphicsFile(asiUI_Common::OpenSaveAction_Save);

  if ( !asiAlgo_WriteSVG::Write( curvesComp, QStr2AsciiStr(filename), 1e-3 ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to save SVG.");
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onCopyAsJSON()
{
  Handle(ActAPI_INode) sel;
  if ( !this->selectedNode(sel) ) return;

  // Dump either curve or surface to JSON.
  std::ostringstream jsonStream;
  //
  if ( sel->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    asiAlgo_JSON::DumpCurve(Handle(asiData_IVCurveNode)::DownCast(sel)->GetCurve(),
                            jsonStream);
  }
  else if ( sel->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    asiAlgo_JSON::DumpSurface(Handle(asiData_IVSurfaceNode)::DownCast(sel)->GetSurface(),
                              jsonStream);
  }
  else
    return;

  // Copy to clipboard.
  QApplication::clipboard()->setText( jsonStream.str().c_str() );
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToBREP()
{
  Handle(ActAPI_HNodeList) sel;
  if ( !this->selectedNodes(sel) ) return;

  Handle(ActAPI_INode) selected_n = sel->First();

  // Outcome filename.
  QString filename = asiUI_Common::selectBRepFile(asiUI_Common::OpenSaveAction_Save);

  // Shape to write.
  TopoDS_Shape shape;

  /* TOPOLOGY */
  if ( selected_n->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
  {
    Handle(asiData_IVTopoItemNode)
      topoNode = Handle(asiData_IVTopoItemNode)::DownCast(selected_n);

    shape = topoNode->GetShape();
  }

  /* CURVE */
  else if ( selected_n->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    TopoDS_Compound curvesComp;
    BRep_Builder bbuilder;
    bbuilder.MakeCompound(curvesComp);

    for ( ActAPI_NodeList::Iterator nit(*sel); nit.More(); nit.Next() )
    {
      Handle(ActAPI_INode) node = nit.Value();
      //
      if ( !node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
        continue;

      Handle(Geom_Curve)
        curve = Handle(asiData_IVCurveNode)::DownCast(node)->GetCurve();

      bbuilder.Add( curvesComp, BRepBuilderAPI_MakeEdge(curve) );
    }

    shape = curvesComp;
  }

  // Save shape.
  if ( shape.IsNull() || !asiAlgo_Utils::WriteBRep( shape, QStr2AsciiStr(filename) ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot save shape.");
    return;
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSetAsPart()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Convert to the supported type.
  Handle(asiData_IVTopoItemNode)
    topoNode = Handle(asiData_IVTopoItemNode)::DownCast(selected_n);

  TopoDS_Shape shapeToSet;
  double       linDefl = 0., angDefl = 0.;

  if ( !topoNode.IsNull() )
  {
    // Get shape to convert.
    shapeToSet = topoNode->GetShape();
    linDefl    = topoNode->GetLinearDeflection();
    angDefl    = topoNode->GetAngularDeflection();
  }
  else
  {
    Handle(asiData_IVCurveNode)
      curveNode = Handle(asiData_IVCurveNode)::DownCast(selected_n);
    //
    if ( !curveNode.IsNull() )
    {
      double f, l;
      Handle(Geom_Curve) C = curveNode->GetCurve(f, l);

      shapeToSet = BRepBuilderAPI_MakeEdge(C, f, l);
    }
    else
    {
      Handle(asiData_IVSurfaceNode)
        surfNode = Handle(asiData_IVSurfaceNode)::DownCast(selected_n);
      //
      if ( !surfNode.IsNull() )
      {
        Handle(Geom_Surface) S = surfNode->GetSurface();

        shapeToSet = BRepBuilderAPI_MakeFace(S, Precision::Confusion());
      }
    }
  }

  // Modify Data Model.
  Handle(asiData_PartNode) part_n;
  m_model->OpenCommand();
  {
    part_n = asiEngine_Part(m_model).Update(shapeToSet);
    //
    part_n->SetLinearDeflection(linDefl);
    part_n->SetAngularDeflection(angDefl);
    part_n->AddUserFlags(NodeFlag_IsPresentationVisible);
  }
  m_model->CommitCommand();

  // Check the part item.
  QTreeWidgetItem*
    pPartItem = ::GetItem( this, part_n->GetId() );
  //
  if ( pPartItem )
  {
    if ( pPartItem->flags() & Qt::ItemIsUserCheckable )
      pPartItem->setCheckState(1, Qt::Checked);
  }

  // Uncheck the topo item.
  QTreeWidgetItem*
    pTargetItem = ::GetItem( this, selected_n->GetId() );
  //
  if ( pTargetItem )
  {
    if ( pTargetItem->flags() & Qt::ItemIsUserCheckable )
      pTargetItem->setCheckState(1, Qt::Unchecked);
  }

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Clear topological item.
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
      m_viewers[k]->PrsMgr()->DeRenderPresentation(selected_n);

    // Actualize part.
    if ( dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]) )
      m_viewers[k]->PrsMgr()->Actualize(part_n);
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onExploreShape()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  TopoDS_Shape shape;

  if ( selected_n->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
  {
    shape = Handle(asiData_IVTopoItemNode)::DownCast(selected_n)->GetShape();
  }
  else if ( selected_n->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
  {
    shape = Handle(asiData_PartNode)::DownCast(selected_n)->GetShape();
  }

  // Prepare browser.
  asiUI_ShapeBrowser*
    pBrowser = new asiUI_ShapeBrowser(shape, m_model, m_progress, m_plotter, nullptr);
  //
  pBrowser->Populate();

  // Open UI dialog.
  QWidget* pDlg = new QDialog(this);
  //
  pDlg->setWindowTitle("Shape browser");
  //
  QVBoxLayout* pDlgLayout = new QVBoxLayout;
  pDlgLayout->setAlignment(Qt::AlignTop);
  pDlgLayout->setContentsMargins(10, 10, 10, 10);
  //
  pDlgLayout->addWidget(pBrowser);
  pDlg->setLayout(pDlgLayout);
  //
  pDlg->show();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToXYZ()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_IVPointSetNode) ) )
    return;

  Handle(asiData_IVPointSetNode)
    ptsNode = Handle(asiData_IVPointSetNode)::DownCast(selected_n);

  QString filename = asiUI_Common::selectXYZFile(asiUI_Common::OpenSaveAction_Save);

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = ptsNode->GetPoints();

  // Save points.
#if defined WIN32
  if ( !pts->SaveAs( QStr2ExtStr(filename).ToWideString() ) )
#else
  if ( !pts->SaveAs( QStr2StdStr(filename).c_str() ) )
#endif
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot save point cloud.");
    return;
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onSaveToREK()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_OctreeNode) ) )
    return;

  Handle(asiData_OctreeNode)
    N = Handle(asiData_OctreeNode)::DownCast(selected_n);

  // Get the uniform grid to save to REK.
  Handle(asiAlgo_UniformGrid<float>) grid = N->GetUniformGrid();
  //
  if ( grid.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "No uniform grid is available.");
    return;
  }

  QString filename = asiUI_Common::selectREKFile(asiUI_Common::OpenSaveAction_Save);

  // Save the grid.
  asiAlgo_WriteREK WriteREK( QStr2StdStr(filename) );
  //
  if ( !WriteREK.Write(grid) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot write REK file.");
    return;
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onDumpContents()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  asiUI_DialogOCAFDump*
    pDumpProject = new asiUI_DialogOCAFDump(m_model, m_progress);
  //
  pDumpProject->SetNode(selected_n);
  pDumpProject->Populate();
  //
  pDumpProject->show();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onPrintParameters()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Loop over the parameters.
  TCollection_AsciiString dump("Parameters of \'");
  dump += selected_n->GetName();
  dump += "\' [";
  dump += selected_n->GetId();
  dump += "]";
  dump += " (";
  dump += selected_n->DynamicType()->Name();
  dump += ")";
  dump += ":\n";
  //
  for ( Handle(ActAPI_IParamIterator) pit = selected_n->GetParamIterator(); pit->More(); pit->Next() )
  {
    const Handle(ActAPI_IUserParameter)& P = pit->Value();

    TCollection_AsciiString paramEntry;
    TDF_Tool::Entry(P->RootLabel(), paramEntry);

    dump += "\t";
    dump += "[";
    dump += pit->Key();
    dump += "] ";
    dump += P->DynamicType()->Name();
    dump += " (";
    dump += paramEntry;
    dump += ")";
    dump += "\n";
  }

  m_progress.SendLogMessage(LogInfo(Normal) << dump);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onCopyName()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Set to clipboard.
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText( ExtStr2QStr( selected_n->GetName() ) );

  // Notify.
  m_progress.SendLogMessage( LogInfo(Normal) << "Selected Node's name: '%1'."
                                             << selected_n->GetName() );
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onCopyID()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  // Set to clipboard.
  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText( ExtStr2QStr( selected_n->GetId() ) );

  // Notify.
  m_progress.SendLogMessage( LogInfo(Normal) << "Selected Node's ID: %1."
                                             << selected_n->GetId() );
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onDeleteNodes()
{
  std::vector<std::pair<Handle(ActAPI_INode), QTreeWidgetItem*>> nodesWithWidgets;
  if ( !selectedNodes(nodesWithWidgets) || nodesWithWidgets.empty())
  {
    return;
  }

  bool isAborted = false;
  bool isDeleted = false;
  m_model->OpenCommand();

  std::vector<std::pair<Handle(ActAPI_INode), QTreeWidgetItem*>>::const_iterator itNodes =
    nodesWithWidgets.cbegin();
  for ( ; itNodes != nodesWithWidgets.cend(); ++itNodes )
  {
    Handle(ActAPI_INode) selected_n = itNodes->first;
    QTreeWidgetItem* root_ui = itNodes->second;

    if ( selected_n.IsNull()               ||
         !selected_n->IsWellFormed()       ||
         !checkIsNotStructural(selected_n) ||
         root_ui == nullptr )
    {
      continue;
    }

    QTreeWidgetItem* parentOfSelectedItem = root_ui->parent();

    // Delete node.
    {
      if ( selected_n->GetUserFlags() & NodeFlag_IsStructural ||
           !deleteChildrenNodes(selected_n) )
      {
        isAborted = true;
        m_model->AbortCommand();
        break;
      }

      for ( size_t k = 0; k < m_viewers.size(); ++k )
      {
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(selected_n) )
          m_viewers[k]->PrsMgr()->DeRenderPresentation(selected_n);
      }

      if ( !m_model->DeleteNode(selected_n) )
      {
        isAborted = true;
        m_model->AbortCommand();
        break;
      }
    }

    deleteChildrenUI(root_ui);

    if ( parentOfSelectedItem != nullptr && parentOfSelectedItem != root_ui && root_ui != nullptr )
    {
      // The removed item will not be deleted.
      parentOfSelectedItem->removeChild(root_ui);
      delete root_ui;
    }
    else if ( parentOfSelectedItem == nullptr && root_ui != nullptr )
    {
      // QTreeWidget::removeItemWidget() already calls the deleteLater().
      this->removeItemWidget(root_ui, 0);
    }

    isDeleted = true;
  }

  if ( m_model->HasOpenCommand() && !isAborted && isDeleted )
    m_model->CommitCommand();
  else if ( m_model->HasOpenCommand() )
    m_model->AbortCommand();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onComputeNorms(const bool doElemNorms)
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_TessNode) ) )
    return;

  Handle(asiData_TessNode)
    tessNode = Handle(asiData_TessNode)::DownCast(selected_n);

  // Modify Data Model.
  Handle(asiData_MeshNormsNode) tessNormsNode;
  //
  m_model->OpenCommand();
  {
    tessNormsNode = asiEngine_Tessellation(m_model).ComputeNorms(tessNode, doElemNorms);
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Actualize in Part Viewer.
    asiUI_ViewerPart* pViewerPart = dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]);
    //
    if ( pViewerPart )
      pViewerPart->PrsMgr()->Actualize(tessNormsNode);
  }
  //
  this->Populate();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onComputeNodalNorms()
{
  this->onComputeNorms(false);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onComputeElementalNorms()
{
  this->onComputeNorms(true);
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onConvertToTris()
{
#if defined USE_MOBIUS
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_TessNode) ) )
    return;

  Handle(asiData_TessNode)
    tessNode = Handle(asiData_TessNode)::DownCast(selected_n);

  // Convert to Poly triangulation.
  Handle(Poly_Triangulation) polyTris;
  asiAlgo_MeshConvert::FromPersistent(tessNode->GetMesh(), polyTris);

  // Modify Data Model.
  m_model->OpenCommand();
  {
    m_model->GetTriangulationNode()->SetTriangulation( cascade::GetMobiusMesh(polyTris) );
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Actualize in Part Viewer.
    asiUI_ViewerPart* pViewerPart = dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]);
    //
    if ( pViewerPart )
      pViewerPart->PrsMgr()->Actualize( m_model->GetTriangulationNode() );
  }
#else
  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");
#endif
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onConvertToTess()
{
#if defined USE_MOBIUS
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_TriangulationNode) ) )
    return;

  Handle(asiData_TriangulationNode)
    trisNode = Handle(asiData_TriangulationNode)::DownCast(selected_n);

  // Convert to Poly triangulation.
  Handle(ActData_Mesh) mesh;
  asiAlgo_MeshConvert::ToPersistent(cascade::GetOpenCascadeMesh( trisNode->GetTriangulation() ), mesh);

  // Modify Data Model.
  m_model->OpenCommand();
  {
    m_model->GetTessellationNode()->SetMesh(mesh);
  }
  m_model->CommitCommand();

  // Update UI.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    // Actualize in Part Viewer.
    asiUI_ViewerPart* pViewerPart = dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]);
    //
    if ( pViewerPart )
      pViewerPart->PrsMgr()->Actualize( m_model->GetTessellationNode() );
  }
#else
  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");
#endif
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onDumpJoint()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_ReEdgeNode) ) )
    return;

  // Get Edge Node.
  Handle(asiData_ReEdgeNode)
    edgeNode = Handle(asiData_ReEdgeNode)::DownCast(selected_n);

  // Check if the Node is regular.
  asiEngine_RE reApi(m_model, m_progress, m_plotter);
  //
  int valencef = 0, valencel = 0;
  //
  if ( reApi.IsRegular(edgeNode) )
    m_progress.SendLogMessage(LogNotice(Normal) << "Edge is regular.");
  else
    m_progress.SendLogMessage(LogNotice(Normal) << "Edge is irregular.");

  m_progress.SendLogMessage(LogInfo(Normal) << "Valence of the first vertex is %1." << valencef);
  m_progress.SendLogMessage(LogInfo(Normal) << "Valence of the last vertex is %1." << valencel);

  // Get coedges.
  Handle(ActAPI_HParameterList) backRefs = edgeNode->GetReferrers();
  //
  for ( ActAPI_HParameterList::Iterator it(*backRefs); it.More(); it.Next() )
  {
    const Handle(ActAPI_IUserParameter)& refParam = it.Value();
    Handle(asiData_ReCoedgeNode)         refNode  = Handle(asiData_ReCoedgeNode)::DownCast( refParam->GetNode() );

    if ( !refNode.IsNull() )
      m_progress.SendLogMessage( LogInfo(Normal) << "Referring coedge of edge %1 is %2 (%3)."
                                                 << edgeNode->GetId() << refNode->GetId() << refNode->GetName() );
  }

  // Analyze joint.
  asiEngine_PatchJointAdaptor jointAdaptor(m_model, m_progress, m_plotter);
  //
  if ( !jointAdaptor.Init(edgeNode) )
    return;

  // Dump graphically.
  jointAdaptor.DumpGraphically();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onUnifyKnotsAndAlign()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_ReEdgeNode) ) )
    return;

  // Get Edge Node.
  Handle(asiData_ReEdgeNode)
    edgeNode = Handle(asiData_ReEdgeNode)::DownCast(selected_n);

  // Analyze joint.
  asiEngine_PatchJointAdaptor jointAdaptor(m_model, m_progress, m_plotter);
  //
  if ( !jointAdaptor.Init(edgeNode) )
    return;

  // Extract isos by performing pure geometric analysis.
  Handle(Geom_BSplineCurve) isoLeft, isoRight;
  bool                      isoLeftU, isoRightU, isoLeftMin, isoRightMin, areOpposite;
  //
  if ( !jointAdaptor.ExtractIsos(isoLeft, isoLeftU, isoLeftMin,
                                 isoRight, isoRightU, isoRightMin,
                                 areOpposite) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to extract isos.");
    return; // Failure.
  }

  // Make surfaces compatible.
  if ( !jointAdaptor.UnifySurfaces(isoLeft, isoLeftU, isoRight, isoRightU, areOpposite) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to unify surfaces.");
    return;
  }

  // Align poles.
  if ( !jointAdaptor.AlignControlPoles(isoLeft, isoLeftU, isoLeftMin,
                                       isoRight, isoRightU, isoRightMin,
                                       areOpposite) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to align poles of surfaces.");
    return;
  }

  const Handle(asiData_RePatchNode)& leftPatchNode  = jointAdaptor.GetPatchLeft();
  const Handle(asiData_RePatchNode)& rightPatchNode = jointAdaptor.GetPatchRight();

  // Store the updated surfaces.
  m_model->OpenCommand();
  {
    leftPatchNode->SetSurface( jointAdaptor.GetSurfaceLeft() );
    rightPatchNode->SetSurface( jointAdaptor.GetSurfaceRight() );
  }
  m_model->CommitCommand();

  // Actualize presentations.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    if ( dynamic_cast<asiUI_ViewerPart*>(m_viewers[k]) )
    {
      m_viewers[k]->PrsMgr()->Actualize(leftPatchNode);
      m_viewers[k]->PrsMgr()->Actualize(rightPatchNode);
    }
  }
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onFeatureComments()
{
  Handle(ActAPI_INode) selected_n;
  if ( !this->selectedNode(selected_n) ) return;

  if ( !selected_n->IsKind( STANDARD_TYPE(asiData_FeatureNode) ) )
    return;

  // Get Feature Node.
  Handle(asiData_FeatureNode)
    featNode = Handle(asiData_FeatureNode)::DownCast(selected_n);

  asiUI_DialogFeatureComments*
    pDumpDlg = new asiUI_DialogFeatureComments(m_model, featNode, m_progress, m_plotter);
  //
  pDumpDlg->Initialize();
  pDumpDlg->show();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onFeatureCheckCommon()
{
  Handle(ActAPI_HNodeList) ns;
  if ( !this->selectedNodes(ns) ) return;

  int             iter = 0;
  asiAlgo_Feature cmnFids;

  for ( ActAPI_HNodeList::Iterator nit(*ns); nit.More(); nit.Next() )
  {
    const Handle(ActAPI_INode)& n = nit.Value();

    if ( !n->IsKind( STANDARD_TYPE(asiData_FeatureNode) ) )
      continue;

    Handle(asiData_FeatureNode)
      fn = Handle(asiData_FeatureNode)::DownCast(n);

    if ( !iter )
    {
      fn->GetMask(cmnFids);
    }
    else
    {
      asiAlgo_Feature operand;
      fn->GetMask(operand);

      cmnFids.Intersect(operand);
    }

    iter++;
  }

  if ( !cmnFids.IsEmpty() )
    m_progress.SendLogMessage(LogInfo(Normal) << "Common face IDs: %1." << cmnFids);
  else
    m_progress.SendLogMessage(LogInfo(Normal) << "No common face IDs found in the selected features.");
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onFeatureCheckRepeated()
{
  Handle(ActAPI_HNodeList) ns;
  if ( !this->selectedNodes(ns) ) return;

  // Collect all features.
  std::vector<asiAlgo_Feature> features;
  //
  for ( ActAPI_HNodeList::Iterator nit(*ns); nit.More(); nit.Next() )
  {
    const Handle(ActAPI_INode)& n = nit.Value();

    if ( !n->IsKind( STANDARD_TYPE(asiData_FeatureNode) ) )
      continue;

    Handle(asiData_FeatureNode)
      fn = Handle(asiData_FeatureNode)::DownCast(n);

    asiAlgo_Feature feature;
    fn->GetMask(feature);
    //
    features.push_back(feature);
  }

  // Check for repetitions.
  bool anyRepetitions = false;
  //
  for ( size_t i = 0; i < features.size(); ++i )
  {
    for ( size_t j = i + 1; j < features.size(); ++j )
    {
      if ( features[i].HasIntersection(features[j]) )
      {
        asiAlgo_Feature cmnFids = features[i];
        cmnFids.Intersect(features[j]);

        Handle(asiData_FeatureNode) fn1 = Handle(asiData_FeatureNode)::DownCast(ns->Value(int(i + 1)));
        Handle(asiData_FeatureNode) fn2 = Handle(asiData_FeatureNode)::DownCast(ns->Value(int(j + 1)));

        if ( !anyRepetitions )
          anyRepetitions = true;

        m_progress.SendLogMessage(LogInfo(Normal) << "Repeated face IDs: %1 (features \"%2\" and \"%3\")."
                                                  << cmnFids << fn1->GetName() << fn2->GetName());
      }
    }
  }

  if ( !anyRepetitions )
    m_progress.SendLogMessage(LogInfo(Normal) << "No repeated face IDs found in the selected features.");
}

//-----------------------------------------------------------------------------

//! Populates context menu with actions.
//! \param[in]     activeNodes currently active Nodes.
//! \param[in,out] pMenu       menu to populate.
void asiUI_ObjectBrowser::populateContextMenu(const Handle(ActAPI_HNodeList)& activeNodes,
                                              QMenu*                          pMenu)
{
  if ( activeNodes.IsNull() || activeNodes->IsEmpty() )
    return;

  // Get type of the first Node.
  const int             numSelected = activeNodes->Length();
  Handle(ActAPI_INode)  node        = activeNodes->First();
  Handle(Standard_Type) type        = node->DynamicType();

  // Check whether other Nodes are of the same type.
  bool isOfSameType = true;
  bool hasNotStructuralNode = false;
  //
  for ( ActAPI_HNodeList::Iterator nit(*activeNodes); nit.More(); nit.Next() )
  {
    const Handle(ActAPI_INode)& N = nit.Value();
    //
    if ( N->DynamicType() != type )
    {
      isOfSameType = false;
    }

    if ( this->checkIsNotStructural(N) )
    {
      hasNotStructuralNode = true;
    }
  }
  //
  if ( hasNotStructuralNode )
  {
    pMenu->addAction( "Delete", this, SLOT( onDeleteNodes() ) );
  }
  //
  if ( !isOfSameType )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "Selected Nodes are of different types.");
    return;
  }

  // Iterate over the associated viewer to find the one where the selected
  // Node is presented.
  bool isPresented = false;
  for ( size_t k = 0; k < m_viewers.size(); ++k )
  {
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(node) )
    {
      isPresented = true;
      break;
    }
  }

  if ( numSelected == 1 )
  {
    pMenu->addAction( "Dump contents",    this, SLOT( onDumpContents    () ) );
    pMenu->addAction( "Print parameters", this, SLOT( onPrintParameters () ) );
    pMenu->addAction( "Copy name",        this, SLOT( onCopyName        () ) );
    pMenu->addAction( "Copy ID",          this, SLOT( onCopyID          () ) );
  }
  //
  if ( node->IsKind( STANDARD_TYPE(asiData_RootNode) ) )
  {
    pMenu->addSeparator();
    pMenu->addAction( "Show execution graph", this, SLOT( onShowExecutionGraph () ) );
  }
  //
  if ( node->IsKind( STANDARD_TYPE(asiData_MetadataNode) ) )
  {
    pMenu->addSeparator();
    pMenu->addAction( "Print contents", this, SLOT( onPrintMetadataContents () ) );
  }
  //
  if ( node->IsKind( STANDARD_TYPE(asiData_FeatureNode) ) )
  {
    if ( numSelected == 1 )
    {
      pMenu->addSeparator();
      pMenu->addAction( "Comments...", this, SLOT( onFeatureComments () ) );
    }
    else
    {
      pMenu->addSeparator();
      pMenu->addAction( "Check for common faces",   this, SLOT( onFeatureCheckCommon   () ) );
      pMenu->addAction( "Check for repeated faces", this, SLOT( onFeatureCheckRepeated () ) );
    }
  }
  //
  if ( isPresented )
  {
    pMenu->addSeparator();
    pMenu->addAction( "Show",      this, SLOT( onShow     () ) );
    pMenu->addAction( "Show only", this, SLOT( onShowOnly () ) );
    pMenu->addAction( "Hide",      this, SLOT( onHide     () ) );

    if ( numSelected == 1 )
    {
      pMenu->addAction( "Manage pipelines...", this, SLOT( onManagePipelines () ) );

      if ( node->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Hide edges",         this, SLOT( onHidePartEdges () ) );
        pMenu->addAction( "Show edges",         this, SLOT( onShowPartEdges () ) );
        pMenu->addAction( "Reset presentation", this, SLOT( onResetPartPrs  () ) );
        pMenu->addAction( "Explore...",         this, SLOT( onExploreShape () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) ||
           node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Copy as JSON",   this, SLOT( onCopyAsJSON () ) );
        pMenu->addAction( "Set as part",    this, SLOT( onSetAsPart  () ) );

        if ( node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
        {
          pMenu->addSeparator();
          pMenu->addAction( "Make partition", this, SLOT( onPartition () ) );
        }
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Save to BREP...", this, SLOT( onSaveToBREP   () ) );
        pMenu->addAction( "Set as part",     this, SLOT( onSetAsPart    () ) );
        pMenu->addAction( "Explore...",      this, SLOT( onExploreShape () ) );
        pMenu->addSeparator();
        pMenu->addAction( "Make partition",  this, SLOT( onPartition    () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_IVPointSetNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Save to XYZ...", this, SLOT( onSaveToXYZ () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_OctreeNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Save to REK...", this, SLOT( onSaveToREK () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_TessNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Compute nodal norms",      this, SLOT( onComputeNodalNorms     () ) );
        pMenu->addAction( "Compute elemental norms",  this, SLOT( onComputeElementalNorms () ) );
        pMenu->addAction( "Convert to triangulation", this, SLOT( onConvertToTris         () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_TriangulationNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Convert to tessellation",  this, SLOT( onConvertToTess () ) );
      }

      if ( node->IsKind( STANDARD_TYPE(asiData_ReEdgeNode) ) )
      {
        pMenu->addSeparator();
        pMenu->addAction( "Dump joint info",       this, SLOT( onDumpJoint          () ) );
        pMenu->addAction( "Unify knots and align", this, SLOT( onUnifyKnotsAndAlign () ) );
      }
    }

    if ( node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
    {
      pMenu->addSeparator();
      pMenu->addAction( "Save to SVG...", this, SLOT( onSaveToSVG() ) );
      pMenu->addAction( "Save to BREP...", this, SLOT( onSaveToBREP() ) );
    }
  }
}

//-----------------------------------------------------------------------------

//! Returns the currently active Node.
//! \param[out] Node  requested Node.
//! \param[out] pItem UI item corresponding to the selected Node.
//! \return true in case of success, false -- otherwise.
bool asiUI_ObjectBrowser::selectedNode(Handle(ActAPI_INode)& Node,
                                       QTreeWidgetItem*&     pItem) const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() || items.length() > 1 )
    return false;

  pItem = items.at(0);
  TCollection_AsciiString entry = QStr2AsciiStr( pItem->data(0, BrowserRoleNodeId).toString() );

  // Take the corresponding data object
  Handle(ActAPI_INode) selected_n = m_model->FindNode(entry);
  //
  if ( selected_n.IsNull() || !selected_n->IsWellFormed() )
  {
    std::cout << "Error: selected Node is invalid" << std::endl;

    m_progress.SendLogMessage(LogWarn(Normal) << "Selected Node is invalid.");
    pItem->setBackgroundColor(0, Qt::darkRed);
    return false;
  }

  // Set result
  Node = selected_n;
  return true;
}

//-----------------------------------------------------------------------------

//! Returns the currently active Node.
//! \param[out] Node requested Node.
//! \return true in case of success, false -- otherwise.
bool asiUI_ObjectBrowser::selectedNode(Handle(ActAPI_INode)& Node) const
{
  QTreeWidgetItem* pItem = nullptr;
  return this->selectedNode(Node, pItem);
}

//-----------------------------------------------------------------------------

//! Returns the currently active Nodes.
//! \param[out] Nodes requested Nodes.
//! \return true in case of success, false -- otherwise.
bool asiUI_ObjectBrowser::selectedNodes(Handle(ActAPI_HNodeList)& Nodes) const
{
  Nodes = new ActAPI_HNodeList;

  std::vector<std::pair<Handle(ActAPI_INode), QTreeWidgetItem*>> nodesWithWidgets;
  if ( !selectedNodes(nodesWithWidgets) )
  {
    return false;
  }

  std::vector<std::pair<Handle(ActAPI_INode), QTreeWidgetItem*>>::const_iterator itNodes =
    nodesWithWidgets.cbegin();
  for ( ; itNodes != nodesWithWidgets.cend(); ++itNodes )
  {
    Nodes->Append(itNodes->first);
  }

  return true;
}

//-----------------------------------------------------------------------------

//! Returns the currently active Nodes with widgets.
//! \param[out] nodesWithWidgets requested Nodes.
//! \return true in case of success, false -- otherwise.
//!
bool asiUI_ObjectBrowser::
  selectedNodes(std::vector<std::pair<Handle(ActAPI_INode),
                                      QTreeWidgetItem*>>& nodesWithWidgets) const
{
  QList<QTreeWidgetItem*> items = this->selectedItems();
  if ( !items.length() )
    return false;

  for ( QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it )
  {
    QTreeWidgetItem* item = *it;
    TCollection_AsciiString entry = QStr2AsciiStr( item->data(0, BrowserRoleNodeId).toString() );

    // Take the corresponding data object.
    Handle(ActAPI_INode) selected_n = m_model->FindNode(entry);
    //
    if ( selected_n.IsNull() || !selected_n->IsWellFormed() )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Selected Node is invalid");
      item->setBackgroundColor(0, Qt::darkRed);
      continue;
    }

    std::pair<Handle(ActAPI_INode), QTreeWidgetItem*> pairNodeWidget(selected_n, item);
    nodesWithWidgets.push_back(pairNodeWidget);
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::showNodes(const Handle(ActAPI_HNodeList)& nodes)
{
  m_model->OpenCommand();
  {
    for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      N->AddUserFlags(NodeFlag_IsPresentationVisible);

      // Iterate over the associated viewers to find the one where the selected
      // Node is presented.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(N) )
        {
          m_viewers[k]->PrsMgr()->DeRenderPresentation(N);
          m_viewers[k]->PrsMgr()->Actualize(N);
        }

      emit show( N->GetId() );
    }
  }
  m_model->CommitCommand();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::showOnlyNodes(const Handle(ActAPI_HNodeList)& nodes)
{
  /* ============================
   *  Gather the Nodes to affect.
   * ============================ */

  Handle(ActAPI_HNodeList) allNodes = new ActAPI_HNodeList; // Nodes to affect.

  // Remove visibility flags from all Nodes.
  Handle(ActAPI_INode) root_n = m_model->GetRootNode();
  //
  for ( Handle(ActAPI_IChildIterator) cit = root_n->GetChildIterator(true);
        cit->More();
        cit->Next() )
  {
    Handle(ActAPI_INode) N = cit->Value();
    //
    if ( !N.IsNull() &&
          N->HasUserFlags(NodeFlag_IsPresentationVisible) &&
          N->HasUserFlags(NodeFlag_IsPresentedInPartView) )
    {
      allNodes->Append(N);
    }
  }

  /* =================================
   *  Perform Data Model modification.
   * ================================= */

  m_model->OpenCommand();
  {
    for ( ActAPI_HNodeList::Iterator nit(*allNodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      // Set inivisible state in the Data Model.
      N->RemoveUserFlags(NodeFlag_IsPresentationVisible);

      // Derender all presentations.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] )
          m_viewers[k]->PrsMgr()->DeRenderPresentation(N);
    }

    // For the Nodes to keep visible, do the opposite.
    for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      N->AddUserFlags(NodeFlag_IsPresentationVisible);

      // Iterate over the associated viewers to find the one where the selected
      // Node is presented.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(N) )
          m_viewers[k]->PrsMgr()->Actualize(N);

      emit showOnly( N->GetId() );
    }
  }
  m_model->CommitCommand();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::hideNodes(const Handle(ActAPI_HNodeList)& nodes)
{
  m_model->OpenCommand();
  {
    for ( ActAPI_HNodeList::Iterator nit(*nodes); nit.More(); nit.Next() )
    {
      const Handle(ActAPI_INode)& N = nit.Value();

      N->RemoveUserFlags(NodeFlag_IsPresentationVisible);

      // Iterate over the associated viewers to find the one where the selected
      // Node is presented.
      for ( size_t k = 0; k < m_viewers.size(); ++k )
        if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(N) )
        {
          m_viewers[k]->PrsMgr()->DeRenderPresentation(N);
          m_viewers[k]->Repaint();
        }

      emit hide( N->GetId() );
    }
  }
  m_model->CommitCommand();
}

//-----------------------------------------------------------------------------

void asiUI_ObjectBrowser::onPartition()
{
  Handle(ActAPI_INode) sel;
  if ( !this->selectedNode(sel) ) return;

  TopoDS_Shape cutter;

  // Check type.
  Handle(asiData_IVSurfaceNode)
    surfNode = Handle(asiData_IVSurfaceNode)::DownCast(sel);
  //
  if ( surfNode.IsNull() )
  {
    Handle(asiData_IVTopoItemNode)
      shapeNode = Handle(asiData_IVTopoItemNode)::DownCast(sel);
    //
    if ( shapeNode.IsNull() )
      return;

    cutter = shapeNode->GetShape();
  }
  else
  {
    cutter = BRepBuilderAPI_MakeFace( surfNode->GetSurface(), Precision::Confusion() );
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = m_model->GetPartNode();
  TopoDS_Shape             partSh   = partNode->GetShape();

  BOPAlgo_Builder bopBuilder;
  //
  bopBuilder.AddArgument(partSh);
  bopBuilder.AddArgument(cutter);
  bopBuilder.Perform();

  const TopoDS_Shape& res = bopBuilder.Shape();

  // Update part.
  m_model->OpenCommand();
  {
    asiEngine_Part partApi(m_model);
    partApi.Update(res);
  }
  m_model->CommitCommand();

  // Actualize.
  for ( size_t k = 0; k < m_viewers.size(); ++k )
    if ( m_viewers[k] && m_viewers[k]->PrsMgr()->IsPresented(partNode) )
    {
      m_viewers[k]->PrsMgr()->DeletePresentation(partNode);
      m_viewers[k]->PrsMgr()->Actualize(partNode, false, true);
    }
}
