//-----------------------------------------------------------------------------
// Created on: 02 December 2015
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
#include <asiUI_ViewerDomain.h>

// asiUI includes
#include <asiUI_IV.h>

// asiAlgo includes
#include <asiAlgo_DeleteEdges.h>
#include <asiAlgo_JoinEdges.h>
#include <asiAlgo_Utils.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiVisu includes
#include <asiVisu_Utils.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRepresentation.h>
#pragma warning(pop)

// Qt-VTK includes
#include <asiVisu_QVTKWidget.h>

// Qt includes
#pragma warning(push, 0)
#include <QDesktopWidget>
#include <QVBoxLayout>
#pragma warning(pop)

// OCCT includes
#include <BRep_Tool.hxx>
#include <Geom2d_Line.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

//-----------------------------------------------------------------------------

//! Creates a new instance of viewer.
//! \param[in] model    Data Model instance.
//! \param[in] progress progress notifier.
//! \param[in] plotter  imperative plotter.
//! \param[in] parent   parent widget.
asiUI_ViewerDomain::asiUI_ViewerDomain(const Handle(asiEngine_Model)& model,
                                       ActAPI_ProgressEntry           progress,
                                       ActAPI_PlotterEntry            plotter,
                                       QWidget*                       parent)
//
: asiUI_Viewer(progress, plotter, parent), m_model(model), m_pPartViewer(nullptr)
{
  // Initialize presentation manager along with QVTK widget
  m_prs_mgr = vtkSmartPointer<asiVisu_PrsManager>::New();
  //
  m_prs_mgr->SetModel(model);
  m_prs_mgr->Initialize(this);
  m_prs_mgr->SetInteractionMode(asiVisu_PrsManager::InteractionMode_2D);
  m_prs_mgr->SetSelectionMode(SelectionMode_Workpiece);
  //
  if ( m_prs_mgr->GetCellPicker().Get() )
    m_prs_mgr->GetCellPicker()->SetTolerance(0.005);

  // Widgets and layouts
  asiVisu_QVTKWidget* pViewer     = m_prs_mgr->GetQVTKWidget();
  QHBoxLayout*        pBaseLayout = new QHBoxLayout(this);

  pBaseLayout->addWidget(pViewer);

  // Configure layout
  pBaseLayout->setSpacing(0);
  pBaseLayout->setContentsMargins(0, 0, 0, 0);

  /* ===================================
   *  Setting up picking infrastructure
   * =================================== */

  // Initialize Callback instance for Pick operation
  m_pickCallback = vtkSmartPointer<asiUI_PickCallback>::New();
  m_pickCallback->SetViewer(this);
  m_pickCallback->SetModel(m_model);
  m_pickCallback->SetPickerTypes(PickerType_Cell);

  // Initialize Callback instance for Domain operations
  m_domainCallback = vtkSmartPointer<asiUI_PDomainCallback>::New();
  m_domainCallback->SetViewer(this);

  // Set observer for picking
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_PICK_DEFAULT) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_PICK_DEFAULT, m_pickCallback);

  // Set observer for detection
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_DETECT_DEFAULT) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_DETECT_DEFAULT, m_pickCallback);

  // Set observer for edge removal
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_DELETE) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_DELETE, m_domainCallback);

  // Set observer for edge joining
  if ( !m_prs_mgr->GetImageInteractorStyle()->HasObserver(EVENT_JOIN) )
    m_prs_mgr->GetImageInteractorStyle()->AddObserver(EVENT_JOIN, m_domainCallback);

  // Get notified once any sensitive is picked on a section
  connect( m_pickCallback, SIGNAL( picked() ), this, SLOT( onDomainPicked() ) );

  // Get notified of edge removal
  connect( m_domainCallback, SIGNAL( killEdges() ), this, SLOT( onKillEdges() ) );

  // Get notified of edge joining
  connect( m_domainCallback, SIGNAL( joinEdges() ), this, SLOT( onJoinEdges() ) );

  /* =====================================
   *  Finalize initial state of the scene
   * ===================================== */

  // Initialize text widget used for annotations
  m_textWidget = vtkSmartPointer<vtkTextWidget>::New();
  m_textWidget->SelectableOff();
  //
  m_textWidget->SetInteractor      ( m_prs_mgr->GetRenderer()->GetRenderWindow()->GetInteractor() );
  m_textWidget->SetDefaultRenderer ( m_prs_mgr->GetRenderer() );
  m_textWidget->SetCurrentRenderer ( m_prs_mgr->GetRenderer() );
  //
  vtkTextRepresentation* textRep = vtkTextRepresentation::SafeDownCast( m_textWidget->GetRepresentation() );
  vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
  textRep->SetTextActor(textActor);
  //
  textRep->GetPositionCoordinate()->SetValue(0.3, 0.8);
  textRep->GetPosition2Coordinate()->SetValue(0.69, 0.19);
  //
  textActor->GetTextProperty()->SetJustificationToLeft();
  textActor->GetTextProperty()->SetVerticalJustificationToTop();

  // Enable context menu
  if ( pViewer )
  {
    pViewer->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( pViewer, SIGNAL ( customContextMenuRequested(const QPoint&) ),
             this,    SLOT   ( onContextMenu(const QPoint&) ) );

    this->onResetView();
  }
}

//-----------------------------------------------------------------------------

//! Destructor.
asiUI_ViewerDomain::~asiUI_ViewerDomain()
{
}

//-----------------------------------------------------------------------------

//! \return size hint.
QSize asiUI_ViewerDomain::sizeHint() const
{
  QDesktopWidget desktop;
  const int side   = std::min( desktop.height(), desktop.width() );
  const int width  = (int) (side*0.25);
  const int height = (int) (side*0.25);

  QSize s(width, height);
  return s;
}

//-----------------------------------------------------------------------------

//! Updates viewer.
void asiUI_ViewerDomain::Repaint()
{
  m_prs_mgr->GetQVTKWidget()->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------

//! Resets view.
void asiUI_ViewerDomain::onResetView()
{
  asiVisu_Utils::CameraOnTop( m_prs_mgr->GetRenderer() );
  m_selectedEdgesCache = TColStd_PackedMapOfInteger();
  this->Repaint();
}

//-----------------------------------------------------------------------------

//! Callback for picking event.
void asiUI_ViewerDomain::onDomainPicked()
{
  std::map<int, TopoDS_Edge> edges;
  this->GetSelectedEdges(edges);

  auto itEdges = edges.cbegin();
  for ( ; itEdges != edges.cend(); ++itEdges )
  {
    const int edgeIdInFace = itEdges->first;

    // Prepare label
    TCollection_AsciiString TITLE = "(Edge #";
    TITLE += edgeIdInFace;
    TITLE += ", ";
    TITLE += asiAlgo_Utils::ShapeAddr(itEdges->second).c_str();
    TITLE += ", ";
    TITLE += asiAlgo_Utils::OrientationToString(itEdges->second);
    TITLE += " in face)\n";
    //
    double f, l;
    Handle(Geom_Curve) c3d = BRep_Tool::Curve(itEdges->second, f, l);
    //
    if ( !c3d.IsNull() )
    {
      TITLE += "3D: ";
      TITLE += c3d->DynamicType()->Name();
      TITLE += " [";
      TITLE += f;
      TITLE += ", ";
      TITLE += l;
      TITLE += "]\n";
    }

    ShapeAnalysis_Edge sae;
    Handle(Geom2d_Curve) c2d;
    double f2, l2;
    //
    if ( sae.PCurve(itEdges->second, m_selectedFaceCache, c2d, f2, l2, true) )
    {
      TITLE += "2D: ";
      TITLE += c2d->DynamicType()->Name();
      TITLE += " [";
      TITLE += f2;
      TITLE += ", ";
      TITLE += l2;
      TITLE += "]";

      gp_Pnt2d p2d1 = c2d->Value(f2);
      gp_Pnt2d p2d2 = c2d->Value(l2);

      TITLE += "\nCONS(";
      TITLE += f2;
      TITLE += ") = (";
      TITLE += p2d1.X();
      TITLE += ", ";
      TITLE += p2d1.Y();
      TITLE += ")";
      //
      TITLE += "\nCONS(";
      TITLE += l2;
      TITLE += ") = (";
      TITLE += p2d2.X();
      TITLE += ", ";
      TITLE += p2d2.Y();
      TITLE += ")";
    }

    TITLE += "\nLocation: ";
    TITLE += asiAlgo_Utils::LocationToString(itEdges->second.Location());
    TITLE += "\n";

    m_progress.SendLogMessage(LogInfo(Normal) << "%1" << TITLE);
  }

  if ( m_pPartViewer )
  {
    // Highlight in the Part viewer.
    asiEngine_Part( m_model,
                    m_pPartViewer->PrsMgr() ).HighlightEdges(m_selectedEdgesCache);
  }
}

//-----------------------------------------------------------------------------

//! Callback for edges removal.
void asiUI_ViewerDomain::onKillEdges()
{
  //Handle(asiData_PartNode) N = m_model->GetPartNode();
  ////
  //if ( N.IsNull() || !N->IsWellFormed() || N->GetShape().IsNull() )
  //  return;

  //TopoDS_Shape part = N->GetShape();

  //// Get edges
  //TopTools_IndexedMapOfShape selectedEdges;
  //asiEngine_Domain::GetHighlightedEdges(N, this->PrsMgr(), selectedEdges);

  //// Delete selected edges
  //asiAlgo_DeleteEdges eraser(part);
  //if ( !eraser.Perform(selectedEdges, true) )
  //{
  //  std::cout << "Error: cannot delete edges" << std::endl;
  //  return;
  //}

  //const TopoDS_Shape& result = eraser.Result();

  //// Save to model
  //m_model->OpenCommand();
  //{
  //  asiEngine_Part(m_model).Update(result);
  //}
  //m_model->CommitCommand();

  //m_selectedEdgesCache = TColStd_PackedMapOfInteger();

  //// Update viewer
  //this->PrsMgr()->DeleteAllPresentations();
  //this->PrsMgr()->Actualize( N.get() );

  //// Notify
  //emit partModified();
}

//-----------------------------------------------------------------------------

//! Callback for edges joining.
void asiUI_ViewerDomain::onJoinEdges()
{
  Handle(asiData_PartNode) N = m_model->GetPartNode();
  //
  if ( N.IsNull() || !N->IsWellFormed() || N->GetShape().IsNull() )
    return;

  if ( m_selectedEdgesCache.IsEmpty() || m_selectedFaceCache.IsNull() )
    return;

  TopoDS_Shape        part = N->GetShape();
  Handle(asiAlgo_AAG) aag  = N->GetAAG();

  m_progress.SendLogMessage(LogInfo(Normal) << "Trying to concatenate the selected edges...");

  // Get edges
  TopTools_IndexedMapOfShape selectedEdges;
  //
  for ( TColStd_PackedMapOfInteger::Iterator eit(m_selectedEdgesCache);
        eit.More(); eit.Next() )
  {
    const int eid = eit.Key();

    const TopoDS_Shape& edge = aag->RequestMapOfEdges()(eid);
    selectedEdges.Add( edge.Located( TopLoc_Location() ) );
  }

  // Join selected edges
  asiAlgo_JoinEdges joiner(part, m_progress, m_plotter);
  //
  if ( !joiner.Perform(selectedEdges, m_selectedFaceCache) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Edge concatenation failed.");
    return;
  }

  const TopoDS_Shape& result = joiner.Result();

  // Save to model
  m_model->OpenCommand();
  {
    const int fid = N->GetAAG()->RequestMapOfSubShapes().FindIndex(m_selectedFaceCache);

    asiEngine_Part partApi(m_model);
    partApi.Update(result);
    partApi.SetSelectedFace(fid);
  }
  m_model->CommitCommand();

  // Reset selection.
  m_selectedEdgesCache = TColStd_PackedMapOfInteger();

  // Update viewer
  Handle(asiUI_IV)
    IV = Handle(asiUI_IV)::DownCast( m_plotter.Access() );
  //
  IV->GetPrsMgr3d()->Actualize( N );
  IV->GetPrsMgr2d()->Actualize( N->GetFaceRepresentation() );
}

//-----------------------------------------------------------------------------

void asiUI_ViewerDomain::onContextMenu(const QPoint& pos)
{
  asiVisu_QVTKWidget* pViewer   = m_prs_mgr->GetQVTKWidget();
  QPoint              globalPos = pViewer->mapToGlobal(pos);

  emit contextMenu(globalPos);
}

//-----------------------------------------------------------------------------

void asiUI_ViewerDomain::GetSelectedEdges(std::map<int, TopoDS_Edge>& edges)
{
  Handle(asiData_PartNode) N = m_model->GetPartNode();
  //
  if ( N.IsNull() || !N->IsWellFormed() || N->GetShape().IsNull() )
  {
    m_selectedEdgesCache = TColStd_PackedMapOfInteger();

    if ( m_pPartViewer )
    {
      m_pPartViewer->PrsMgr()->ResetSelection();
    }

    return;
  }

  // Get a map of shapes.
  const TopTools_IndexedMapOfShape& subShapesMap = N->GetAAG()->RequestMapOfSubShapes();

  // Get face.
  const int fid = N->GetFaceRepresentation()->GetAnySelectedFace();
  //
  TopoDS_Face F;
  if ( (fid > 0) && ( fid <= subShapesMap.Extent() ) )
    F = TopoDS::Face( subShapesMap.FindKey(fid) );
  //
  if ( F.IsNull() )
  {
    m_selectedFaceCache.Nullify();
    m_selectedEdgesCache = TColStd_PackedMapOfInteger();

    if ( m_pPartViewer )
    {
      m_pPartViewer->PrsMgr()->ResetSelection();
    }

    return;
  }
  //
  m_selectedFaceCache = F;

  // Access picking results.
  const asiVisu_ActualSelection&          sel     = m_prs_mgr->GetCurrentSelection();
  const Handle(asiVisu_CellPickerResult)& pickRes = sel.GetCellPickerResult(SelectionNature_Persistent);
  //
  if ( pickRes.IsNull() )
  {
    m_selectedEdgesCache = TColStd_PackedMapOfInteger();

    if ( m_pPartViewer )
    {
      m_pPartViewer->PrsMgr()->ResetSelection();
    }

    return;
  }

  // Get the picked actor.
  const vtkSmartPointer<vtkActor>& actor = pickRes->GetPickedActor();
  //
  if ( !actor )
  {
    m_selectedEdgesCache = TColStd_PackedMapOfInteger();

    if ( m_pPartViewer )
    {
      m_pPartViewer->PrsMgr()->ResetSelection();
    }

    return;
  }

  // Access polygonal data mapper.
  vtkPolyDataMapper* pMapper = vtkPolyDataMapper::SafeDownCast( actor->GetMapper() );
  if ( !pMapper )
  {
    m_selectedEdgesCache = TColStd_PackedMapOfInteger();

    if ( m_pPartViewer )
    {
      m_pPartViewer->PrsMgr()->ResetSelection();
    }

    return;
  }

  // Access polygonal data
  vtkPolyData* pData = vtkPolyData::SafeDownCast( pMapper->GetInput() );
  if ( !pData )
  {
    m_selectedEdgesCache = TColStd_PackedMapOfInteger();

    if ( m_pPartViewer )
    {
      m_pPartViewer->PrsMgr()->ResetSelection();
    }

    return;
  }

  const TColStd_PackedMapOfInteger& cellGIDs = pickRes->GetPickedElementIds();

  // Get edges. We make an exploration loop here in order not to miss seams.
  int current_id = 0;
  TopTools_IndexedMapOfShape selected;
  TColStd_PackedMapOfInteger eids;
  //
  for ( TopExp_Explorer eexp(F.Oriented(TopAbs_FORWARD), TopAbs_EDGE); eexp.More(); eexp.Next() )
  {
    ++current_id;
    if ( cellGIDs.Contains(current_id) )
    {
      edges[current_id] = ( TopoDS::Edge( eexp.Current() ) );
      selected.Add(edges[current_id]);

      const int eid = N->GetAAG()->RequestMapOfEdges().FindIndex(edges[current_id]);
      eids.Add(eid);
    }
  }

  m_selectedEdgesCache = eids;
}
