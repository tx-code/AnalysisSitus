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
#include "asiUI_SelectFile.h"

// asiAlgo includes
#include <asiAlgo_AppSurf2.h>
#include <asiAlgo_PlateOnEdges.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiVisu includes
#include <asiVisu_PartNodeInfo.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QFile>
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
: QDialog             (parent),
  m_pViewer           (pViewer),
  m_model             (model),
  m_blockPointsChange (false),
  m_progress          (progress),
  m_plotter           (plotter)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters.
  QFrame*    pMethodPanel        = new QFrame;
  QGroupBox* pGroupEConstraints  = new QGroupBox("Edge constraints");
  QGroupBox* pGroupPPConstraints = new QGroupBox("Pin-point constraints");
  QGroupBox* pGroupAdvanced      = new QGroupBox("Advanced");
  QFrame*    bButtonsFrame  = new QFrame;

  // Method.
  m_widgets.pMethodSel = new QComboBox();
  //
  m_widgets.pMethodSel->addItem("PLATE",    Method_PLATE);
  m_widgets.pMethodSel->addItem("APPSURF1", Method_APPSURF1);
  m_widgets.pMethodSel->addItem("APPSURF2", Method_APPSURF2);

  // Selected edges.
  m_widgets.pEdges = new asiUI_LineEdit();

  // Filename for xyz data.
  m_widgets.pSelectXYZLabel = new QLabel("Filename (xyz)", this);
  m_widgets.pSelectXYZ = new asiUI_SelectFile("XYZ point cloud (*.xyz)",
                                              "Filename (xyz)",
                                              QString(),
                                              QImage(":icons/asitus/select_xyz.svg"),
                                              asiUI_Common::OpenSaveAction::OpenSaveAction_Open,
                                              this);

  // Table with point coordinates.
  m_widgets.pPoints = wf->CreateDatumTable(0, 0);
  //
  m_widgets.pPoints->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );
  m_widgets.pPoints->setMinimumWidth(CONTROL_TBL_WIDTH);

  m_widgets.pPoints->SetColumnCount(3);
  m_widgets.pPoints->SetCanExpandOnPaste(Qt::Horizontal, false);
  m_widgets.pPoints->SetRowCount(0);
  m_widgets.pPoints->SetColumnEditor(0, "POI_X");
  m_widgets.pPoints->SetColumnEditor(1, "POI_Y");
  m_widgets.pPoints->SetColumnEditor(2, "POI_Z");
  m_widgets.pPoints->SetEditByClick(false);

  // Buttons to manage the table
  m_widgets.pInsertRow = new QPushButton(QIcon(":icons/asitus/table_add.png"),    QString());
  m_widgets.pInsertRow->setToolTip("Add Row");
  m_widgets.pRemoveRow = new QPushButton(QIcon(":icons/asitus/table_remove.png"), QString());
  m_widgets.pRemoveRow->setToolTip("Delete Row" );

  m_widgets.pInitialSurfaceLabel = new QLabel("Initial surface", this);
  m_widgets.pInitialSurface = new asiUI_LineEdit(this);

  m_widgets.pFairingCoeff = wf->CreateEditor("Fairing_coeff", this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  m_widgets.pNumIters     = wf->CreateEditor("Num_iters",     this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");
  m_widgets.pClose = new QPushButton("Close");

  // Sizing.
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction.
  connect( m_widgets.pMethodSel, SIGNAL( currentIndexChanged (const int)                               ),
           this,                 SLOT  ( onChangeMethod      (const int)                               ) );
  connect( m_widgets.pSelectXYZ, SIGNAL( textChanged         (const QString&)                          ),
           this,                 SLOT  ( onXYZSelected       ()                                        ) );
  connect( m_widgets.pPoints,    SIGNAL( ValueChanged        ( const int, const int, const QVariant& ) ),
           this,                 SLOT  ( onPointsChanged     ()                                        ) );
  connect( m_widgets.pInsertRow, SIGNAL( clicked             ()                                        ),
           m_widgets.pPoints,    SLOT  ( InsertRow           ()                                        ) );
  connect( m_widgets.pRemoveRow, SIGNAL( clicked             ()                                        ),
           m_widgets.pPoints,    SLOT  ( RemoveRows          ()                                        ) );
  connect( m_widgets.pApply,     SIGNAL( clicked             ()                                        ),
           this,                 SLOT  ( onApply             ()                                        ) );
  connect( m_widgets.pClose,     SIGNAL( clicked             ()                                        ),
           this,                 SLOT  ( close               ()                                        ) );

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

    // xyz filename
    QFrame* selectFileFrame = new QFrame(pGroupPPConstraints);
    QHBoxLayout* selectFileFrameLayout = new QHBoxLayout(selectFileFrame);
    selectFileFrameLayout->addWidget(m_widgets.pSelectXYZLabel);
    selectFileFrameLayout->addWidget(m_widgets.pSelectXYZ);

    selectFileFrameLayout->setSpacing(10);
    selectFileFrameLayout->setMargin(0);

    pLayout->addWidget(selectFileFrame);
    //
    pLayout->addWidget(m_widgets.pPoints);

   // buttons
    QFrame* addRemFrame = new QFrame(pGroupPPConstraints);
    QHBoxLayout* addRemLayout = new QHBoxLayout(addRemFrame);

    addRemLayout->addStretch();
    addRemLayout->addWidget(m_widgets.pInsertRow);
    addRemLayout->addWidget(m_widgets.pRemoveRow);
    addRemLayout->setSpacing(10);
    addRemLayout->setMargin(0);

    pLayout->addWidget(addRemFrame);
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

    m_widgets.pFairingCoeff->AddTo(pLayout,            1, 0);
    m_widgets.pNumIters->AddTo    (pLayout,            2, 0);
    //
    pLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Layout for buttons.
  {
    QHBoxLayout* pButtonsLayout = new QHBoxLayout(bButtonsFrame);
    pButtonsLayout->setSpacing(10);
    //
    pButtonsLayout->addWidget(m_widgets.pApply);
    pButtonsLayout->addWidget(m_widgets.pClose);
  }

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(pMethodPanel);
  m_pMainLayout->addWidget(pGroupEConstraints);
  m_pMainLayout->addWidget(pGroupPPConstraints);
  m_pMainLayout->addWidget(pGroupAdvanced);
  m_pMainLayout->addWidget(bButtonsFrame);

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

  if (m_pViewer)
  {
    connect( m_pViewer, SIGNAL ( edgePicked(asiVisu_PickerResult*) ),
             this,      SLOT   ( onEdgePicked() ) );

    connect( m_pViewer, SIGNAL ( facePicked(asiVisu_PickerResult*) ),
             this,      SLOT   ( onFacePicked(asiVisu_PickerResult*) ) );
  }
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

void asiUI_DialogAppSurf::onXYZSelected()
{
  QString fileName = m_widgets.pSelectXYZ->text();

  if ( fileName.isEmpty() )
    return;

  QFile file(fileName);
  //
  if ( !file.exists() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' does not exist."
                                             << QStr2AsciiStr(fileName) );
    return;
  }

  // Load point cloud
  Handle(asiAlgo_BaseCloud<double>) cloud = new asiAlgo_BaseCloud<double>;
  //
  if ( !cloud->Load( fileName.toLatin1() ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot load point cloud from '%1'."
                                             << QStr2AsciiStr(fileName) );
    return;
  }
  m_progress.SendLogMessage(LogInfo(Normal) << "Point cloud was loaded successfully.");

  // filling table with test values
  m_blockPointsChange = true;
  {
    const int numPts = cloud->GetNumberOfElements();

    m_widgets.pPoints->SetRowCount(numPts);
    for ( int i = 0; i < numPts; ++i )
    {
      gp_XYZ xyz = cloud->GetElement(i);
      //
      for ( int j = 0; j < 3; j += 3 )
      {
        m_widgets.pPoints->SetValue( i, j,     QString::number( xyz.X() ) );
        m_widgets.pPoints->SetValue( i, j + 1, QString::number( xyz.Y() ) );
        m_widgets.pPoints->SetValue( i, j + 2, QString::number( xyz.Z() ) );
      }
    }
  }
  m_blockPointsChange = false;
}

//-----------------------------------------------------------------------------

void asiUI_DialogAppSurf::onPointsChanged()
{
  if ( m_blockPointsChange )
    return;

  m_widgets.pSelectXYZ->reset();
}

//-----------------------------------------------------------------------------

void asiUI_DialogAppSurf::onApply()
{
  if ( !m_model || !m_pViewer )
    return;

  // Fairing coefficient.
  const double lambda = QVariant( m_widgets.pFairingCoeff->GetString() ).toDouble();

  /* ==================
   *  Read constraints.
   * ================== */

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  Handle(asiAlgo_AAG) aag = partApi.GetAAG();

  Handle(TopTools_HSequenceOfShape) hedges = new TopTools_HSequenceOfShape;

  if ( !aag.IsNull() )
  {
    const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();

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
  }

  // Point constraints from the table.
  Handle(asiAlgo_BaseCloud<double>) extraPts = this->getTablePoints();
  //
  if ( !extraPts->IsEmpty() )
    m_plotter.REDRAW_POINTS("extraPts", extraPts->GetCoordsArray(), Color_Violet);

  /* =====================
   *  Approximate surface.
   * ===================== */

  Handle(asiAlgo_BaseCloud<double>) finalConstraints;
  Handle(Geom_BSplineSurface)       surf;
  TopoDS_Face                       face;

  if ( m_method == Method_PLATE )
  {
    // Prepare interpolation tool.
    asiAlgo_PlateOnEdges interpAlgo(partApi.GetShape(), m_progress, m_plotter);
    //
    interpAlgo.SetFairingCoeff (lambda);
    interpAlgo.SetExtraPoints  (extraPts);

    // Interpolate.
    if ( !interpAlgo.Build(hedges, GeomAbs_C0, surf, face) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Interpolation failed.");
      return;
    }

    finalConstraints = interpAlgo.GetConstraints();
  }

  else if ( m_method == Method_APPSURF2 )
  {
    // Prepare approximation tool.
    asiAlgo_AppSurf2 approxAlgo(m_progress, m_plotter);
    //
    approxAlgo.SetFairingCoeff (lambda);
    approxAlgo.SetExtraPoints  (extraPts);

    // Interpolate.
    if ( !approxAlgo.Build(hedges, surf, face) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "APPSURF2 failed.");
      return;
    }

    finalConstraints = approxAlgo.GetConstraints();
  }
  else
  {
    m_progress.SendLogMessage(LogErr(Normal) << "The selected method is not currently supported.");
    return;
  }

  if ( !surf.IsNull() )
    m_plotter.DRAW_SURFACE(surf, Color_Default, "fillingSurf");

  if ( !face.IsNull() )
    m_plotter.DRAW_SHAPE(face, Color_Default, "fillingFace");

  if ( !finalConstraints.IsNull() )
    m_plotter.REDRAW_POINTS("finalConstraints", finalConstraints->GetCoordsArray(), Color_Red);
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_BaseCloud<double>) asiUI_DialogAppSurf::getTablePoints() const
{
  Handle(asiAlgo_BaseCloud<double>) res = new asiAlgo_BaseCloud<double>;

  const int numRows = m_widgets.pPoints->GetRowCount();
  const int numCols = m_widgets.pPoints->GetColumnCount();

  for ( int i = 0; i < numRows; ++i )
  {
    for ( int j = 0; j < numCols; j += 3 )
    {
      const double x = m_widgets.pPoints->GetValue(i, j)    .toDouble();
      const double y = m_widgets.pPoints->GetValue(i, j + 1).toDouble();
      const double z = m_widgets.pPoints->GetValue(i, j + 2).toDouble();

      res->AddElement(x, y, z);
    }
  }

  return res;
}
