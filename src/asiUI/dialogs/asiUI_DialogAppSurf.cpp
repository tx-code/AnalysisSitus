//-----------------------------------------------------------------------------
// Created on: 26 February 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
#include "asiUI_DialogAppSurf.h"

// asiAlgo includes
#include <asiAlgo_AppSurf2.h>
#include <asiAlgo_PlateOnEdges.h>

// asiEngine includes
#include <asiEngine_Part.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
//
#include <Standard_WarningsRestore.hxx>

//-----------------------------------------------------------------------------

#define CONTROL_EDIT_WIDTH 100
#define CONTROL_BTN_WIDTH 150
#define CONTROL_TBL_WIDTH 350

static const char* const image0_data[] = { 
"48 48 10 1",
". c None",
"a c #000000",
"g c #008200",
"h c #008284",
"e c #840000",
"b c #0f9edb",
"c c #848284",
"# c #c6c3c6",
"d c #52cbff",
"f c #ffffff",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
".......................#aa......................",
".......................abb...............#aaac..",
".......................abb.............cbdbabda.",
"..ccccccccc#####abdddddddddbbeeaa#.caabbbb...#dc",
"..ddddddddddddddddddddd#####ddddbbbaaaebba....ba",
"..bbbbbbbbdddddddddddd#ffff#dddbbbeaaegbba....ba",
"..fcccaaabbbbbddddddd##ffff#dcbbeeaaaeabbb...#d#",
"..........caaebbbbdddd#fff##dcbbaeaeabaebbbaeda.",
"............#aegbbbdddd####dcbbeaaeebegaaaaaa#..",
"..............aaaabbbbdd###bbbeeaaaac...........",
"...............#aeaeebbbbbbbbbebaa..............",
"..................aeeaeebbbbbeac................",
"...................#aaeebeeaac..................",
"......................cbbea.....................",
".......................dba#.....................",
"......................#bbag.....................",
"......................adbaa.....................",
"......................bdbaa.....................",
".....................cddbbea....................",
".....................dddbbaec...................",
"...................cdddddbaaae..................",
"................#bddddddbbeaaaaaf...............",
"................cccccccccbbhb.e.c...............",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................"};

//-----------------------------------------------------------------------------

asiUI_DialogAppSurf::asiUI_DialogAppSurf(const Handle(asiUI_WidgetFactory)& wf,
                                         const Handle(asiEngine_Model)&     model,
                                         asiUI_ViewerPart*                  pViewer,
                                         ActAPI_ProgressEntry               progress,
                                         ActAPI_PlotterEntry                plotter,
                                         QWidget*                           parent)
: QDialog    (parent),
  m_pViewer  (pViewer),
  m_model    (model),
  m_progress (progress),
  m_plotter  (plotter)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters.
  QFrame*    pMethodPanel        = new QFrame;
  QGroupBox* pGroupEConstraints  = new QGroupBox("Edge constraints");
  QGroupBox* pGroupPPConstraints = new QGroupBox("Pin-point constraints");
  QGroupBox* pGroupAdvanced      = new QGroupBox("Advanced");

  // Method.
  m_widgets.pMethodSel = new QComboBox();
  //
  m_widgets.pMethodSel->addItem("PLATE",    Method_PLATE);
  m_widgets.pMethodSel->addItem("APPSURF1", Method_APPSURF1);
  m_widgets.pMethodSel->addItem("APPSURF2", Method_APPSURF2);

  // Selected edges.
  m_widgets.pEdges = new asiUI_LineEdit();

  // Table with point coordinates.
  m_widgets.pPoints = wf->CreateDatumTable(0, 0);
  //
  m_widgets.pPoints->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );
  m_widgets.pPoints->setMinimumWidth(CONTROL_TBL_WIDTH);

  m_widgets.pPoints->SetColumnCount(3);
  m_widgets.pPoints->SetRowCount(1);
  m_widgets.pPoints->SetColumnEditor(0, "POI_X");
  m_widgets.pPoints->SetColumnEditor(1, "POI_Y");
  m_widgets.pPoints->SetColumnEditor(2, "POI_Z");
  m_widgets.pPoints->SetEditByClick(false);

  m_widgets.pInitialSurfaceLabel = new QLabel("Initial surface", this);
  m_widgets.pInitialSurface = new asiUI_LineEdit(this);

  m_widgets.pFairingCoeffLabel = new QLabel("Fairing coeff.", this);
  m_widgets.pFairingCoeff = new asiUI_LineEdit(this);

  m_widgets.pNumItersLabel = new QLabel("Num. iters", this);
  m_widgets.pNumIters = new asiUI_LineEdit(this);

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");

  // Sizing.
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction.
  connect( m_widgets.pMethodSel, SIGNAL( currentIndexChanged (const int) ), this, SLOT( onChangeMethod (const int) ) );
  connect( m_widgets.pApply,     SIGNAL( clicked             ()          ), this, SLOT( onApply        ()          ) );

  //---------------------------------------------------------------------------
  // Layout
  //---------------------------------------------------------------------------

  // Method.
  {
    QGridLayout* pGrid = new QGridLayout(pMethodPanel);
    pGrid->setSpacing(10);
    //
    pGrid->addWidget(new QLabel("Method:"), 0, 0);
    pGrid->addWidget(m_widgets.pMethodSel,  0, 1);
    //
    pGrid->setColumnStretch(0, 0);
    pGrid->setColumnStretch(1, 1);
    pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Edge constraints.
  {
    QVBoxLayout* pLayout = new QVBoxLayout(pGroupEConstraints);
    pLayout->setSpacing(10);
    //
    pLayout->addWidget(m_widgets.pEdges);
    //
    pLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Pin-point constraints.
  {
    QVBoxLayout* pLayout = new QVBoxLayout(pGroupPPConstraints);
    pLayout->setSpacing(10);
    //
    pLayout->addWidget(m_widgets.pPoints);
    //
    pLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Advanced.
  {
    QGridLayout* pLayout = new QGridLayout(pGroupAdvanced);
    pLayout->setSpacing(10);
    //
    pLayout->addWidget(m_widgets.pInitialSurfaceLabel, 0, 0);
    pLayout->addWidget(m_widgets.pInitialSurface,      0, 1);

    pLayout->addWidget(m_widgets.pFairingCoeffLabel,   1, 0);
    pLayout->addWidget(m_widgets.pFairingCoeff,        1, 1);

    pLayout->addWidget(m_widgets.pNumItersLabel,       2, 0);
    pLayout->addWidget(m_widgets.pNumIters,            2, 1);
    //
    pLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(pMethodPanel);
  m_pMainLayout->addWidget(pGroupEConstraints);
  m_pMainLayout->addWidget(pGroupPPConstraints);
  m_pMainLayout->addWidget(pGroupAdvanced);
  m_pMainLayout->addWidget(m_widgets.pApply);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  // Set good initial size.
  this->setMinimumSize( QSize(650, 600) );

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::WindowModal);
  this->setWindowTitle("Fit surface");
  this->setWindowIcon( QIcon( QPixmap( (const char**) image0_data ) ) );

  // Adjust initial state.
  this->onChangeMethod(0);

  connect( m_pViewer, SIGNAL ( edgePicked(asiVisu_PickerResult*) ),
           this,      SLOT   ( onEdgePicked() ) );

  connect( m_pViewer, SIGNAL ( facePicked(asiVisu_PickerResult*) ),
           this,      SLOT   ( onFacePicked(asiVisu_PickerResult*) ) );
}

//-----------------------------------------------------------------------------

asiUI_DialogAppSurf::~asiUI_DialogAppSurf()
{}

//-----------------------------------------------------------------------------
void asiUI_DialogAppSurf::onEdgePicked()
{
  if (!m_model || !m_pViewer)
    return;

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  // Get selected edges.
  TColStd_PackedMapOfInteger eids;
  partApi.GetHighlightedEdges(eids);

  // Collect edges.
  Handle(TopTools_HSequenceOfShape) hedges = new TopTools_HSequenceOfShape;
  //

  QStringList edgeIds;
  for ( TColStd_PackedMapOfInteger::Iterator eit(eids); eit.More(); eit.Next() )
  {
    const int edgeId = eit.Key();

    edgeIds.append(QString::number(edgeId));
    //// Get edge.
    //const TopoDS_Shape&
    //  edge = partApi.GetAAG()->RequestMapOfEdges()(edgeId);

    //hedges->Append(edge);
  }

  m_widgets.pEdges->setText(edgeIds.join(" "));
}

//-----------------------------------------------------------------------------

#include <asiVisu_PartNodeInfo.h>
void asiUI_DialogAppSurf::onFacePicked(asiVisu_PickerResult* pickRes)
{
  if (!m_model || !m_pViewer)
    return;

    // Check if part is picked
  asiVisu_PartNodeInfo* nodeInfo = asiVisu_PartNodeInfo::Retrieve( pickRes->GetPickedActor() );
  //
  if ( pickRes->GetPickedActor() && !nodeInfo )
    return;

  Handle(asiData_PartNode) geom_n = m_model->GetPartNode();

  // Get index of the active sub-shape.
  const int
    globalId = geom_n->GetFaceRepresentation()->GetAnySelectedFace();
  //
  if ( globalId == 0 )
    return;

  m_widgets.pInitialSurface->setText(QString::number(globalId));
}

//-----------------------------------------------------------------------------

void asiUI_DialogAppSurf::onChangeMethod(const int methodIdx)
{
  m_method = (Method) methodIdx;
}

//-----------------------------------------------------------------------------

void asiUI_DialogAppSurf::onApply()
{
  if (!m_model || !m_pViewer)
    return;

  /* ==================
   *  Read constraints.
   * ================== */

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  Handle(asiAlgo_AAG) aag = partApi.GetAAG();

  const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();
  Handle(TopTools_HSequenceOfShape) hedges   = new TopTools_HSequenceOfShape;

  // Read edge indices.
  QStringList eidList = m_widgets.pEdges->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);
  //
  for ( const auto& eidStr : eidList )
  {
    const int eid = eidStr.toInt();

    if ( (eid > 0) && ( eid <= allEdges.Extent() ) )
    {
      const TopoDS_Shape& edge = allEdges.FindKey(eid);
      hedges->Append(edge);
    }
    else
    {
      m_progress.SendLogMessage( LogErr(Normal) << "Input index %1 is out of range [1, %2]."
                                                << eid << allEdges.Extent() );
    }
  }

  /* =====================
   *  Approximate surface.
   * ===================== */

  Handle(Geom_BSplineSurface) surf;

  if ( m_method == Method_PLATE )
  {
    // Prepare interpolation tool.
    asiAlgo_PlateOnEdges interpAlgo(partApi.GetShape(), m_progress, m_plotter);
    //
    interpAlgo.SetFairingCoeff(1e-3);

    // Interpolate.
    if ( !interpAlgo.BuildSurf(hedges, GeomAbs_C0, surf) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Interpolation failed.");
      return;
    }
  }

  else if ( m_method == Method_APPSURF2 )
  {
    // Prepare approximation tool.
    asiAlgo_AppSurf2 approxAlgo(m_progress, m_plotter);
    //
    approxAlgo.SetFairingCoeff(1e-3);

    // Interpolate.
    if ( !approxAlgo.BuildSurf(hedges, surf) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "APPSURF2 failed.");
      return;
    }
  }
  else
  {
    m_progress.SendLogMessage(LogErr(Normal) << "The selected method is not currently supported.");
    return;
  }

  if ( !surf.IsNull() )
    m_plotter.DRAW_SURFACE(surf, Color_Default, "filling");
}
