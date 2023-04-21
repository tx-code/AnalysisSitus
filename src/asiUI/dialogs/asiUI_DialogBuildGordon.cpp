//-----------------------------------------------------------------------------
// Created on: 13 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Krylova Elizaveta
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
#include "asiUI_DialogBuildGordon.h"

// asiUI includes
#include "asiUI_SelectFile.h"

// asiAlgo includes
#include "asiAlgo_BuildGordonSurf.h"
#include <asiAlgo_AppSurfUtils.h>
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
#include <QToolButton>
#include <QFormLayout>
//
#include <Standard_WarningsRestore.hxx>

// OpenCascade includes
#include <BRep_Builder.hxx>

//-----------------------------------------------------------------------------

#define CONTROL_EDIT_WIDTH 80
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

static const char* const image1_data[] = {
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
".......ddddddddd.................ddddddddd......",
".......ddddddddd.................ddddddddd......",
".......ddd.............................ddd......",
".......ddd.............................ddd......",
".......ddd.............................ddd......",
".......ddd.............fff.............ddd......",
".......ddd.............fff.............ddd......",
".......................fff......................",
".......................fff......................",
".......................fff......................",
".......................fff......................",
"...............fffffffffffffffffff..............",
"...............fffffffffffffffffff..............",
".......................fff......................",
".......................fff......................",
".......................fff......................",
".......................fff......................",
".......................fff......................",
".......................fff......................",
".......ddd.............fff.............ddd......",
".......ddd.............fff.............ddd......",
".......ddd.............................ddd......",
".......ddd.............................ddd......",
".......ddd.............................ddd......",
".......ddddddddd.................ddddddddd......",
".......ddddddddd.................ddddddddd......",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",};

//-----------------------------------------------------------------------------

asiUI_DialogBuildGordon::asiUI_DialogBuildGordon(const Handle(asiUI_WidgetFactory)& wf,
                                                 const Handle(asiEngine_Model)&     model,
                                                 asiUI_ViewerPart*                  pViewer,
                                                 ActAPI_ProgressEntry               progress,
                                                 ActAPI_PlotterEntry                plotter,
                                                 QWidget*                           parent)
: QDialog             (parent),
  m_pViewer           (pViewer),
  m_model             (model),
  m_blockPointsChange (false),
  m_pProfileSelector  (nullptr),
  m_pGuideSelector    (nullptr),
  m_progress          (progress),
  m_plotter           (plotter)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Group box for edge constraints.
  QGroupBox* pGroupEConstraints = new QGroupBox("Edge constraints");

  // Group box for advanced settings.
  QGroupBox* pGroupAdvanced = new QGroupBox("Advanced");

  // Group box for parameters.
  QFrame* bButtonsFrame   = new QFrame;
  QFrame* bButtonsFrameAC = new QFrame;

  // Selected profiles.
  m_widgets.pProfiles = new asiUI_LineEdit();

  // Selected guides.
  m_widgets.pGuides = new asiUI_LineEdit();

  // Number of Discr Points.
  QValidator *validator     = new QIntValidator();
  m_widgets.pNumDiscrPoints = new asiUI_LineEdit();
  m_widgets.pNumDiscrPoints->setValidator(validator);

  // UV degrees.
  m_widgets.pUDegree = wf->CreateEditor("UDegree", this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  m_widgets.pVDegree = wf->CreateEditor("VDegree", this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");
  m_widgets.pApply->setFocusPolicy( Qt::NoFocus );

  auto icon = QIcon(QPixmap((const char**)image1_data));
  m_widgets.pChooseProfile = new QPushButton();
  m_widgets.pChooseProfile->setIcon( QIcon(QPixmap((const char**)image1_data)) );
  m_widgets.pChooseProfile->setMaximumWidth(35);

  m_widgets.pChooseGuides = new QPushButton();
  m_widgets.pChooseGuides->setIcon( QIcon(QPixmap((const char**)image1_data)) );
  m_widgets.pChooseGuides->setMaximumWidth(35);

  m_widgets.pClose = new QPushButton("Close");
  m_widgets.pClose->setFocusPolicy( Qt::NoFocus );

  // Sizing.
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction.
  connect( m_widgets.pChooseProfile, SIGNAL( clicked() ),
           this,                     SLOT  ( onProfile() ) );
  connect( m_widgets.pChooseGuides, SIGNAL( clicked() ),
           this,                    SLOT  ( onGuide() ) );
  connect( m_widgets.pApply, SIGNAL( clicked() ),
           this,             SLOT  ( onApply() ) );
  connect( m_widgets.pClose, SIGNAL( clicked() ),
           this,             SLOT  ( close  () ) );

  //---------------------------------------------------------------------------
  // Layout
  //---------------------------------------------------------------------------

  // Edge constraints.
  {
    QGridLayout* pGrid = new QGridLayout(pGroupEConstraints);
    pGrid->setSpacing(10);
    //
    pGrid->addWidget(new QLabel("Profiles"),   0, 0);
    pGrid->addWidget(m_widgets.pProfiles,      0, 1);
    pGrid->addWidget(m_widgets.pChooseProfile, 0, 2);
    //
    pGrid->addWidget(new QLabel("Guides"),     1, 0);
    pGrid->addWidget(m_widgets.pGuides,        1, 1);
    pGrid->addWidget(m_widgets.pChooseGuides,  1, 2);
    //
    pGrid->setColumnStretch(0, 0);
    pGrid->setColumnStretch(1, 1);
    pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Advanced.
  {
    QVBoxLayout* pAdvLayout    = new QVBoxLayout(pGroupAdvanced);
    QFrame*      pGridFrameTop = new QFrame;
    QFrame*      pGridFrameBot = new QFrame;

    pAdvLayout->setAlignment(Qt::AlignTop);

    QLabel* pNumPtsLbl = new QLabel("Num. discr. points");

    // Top grid.
    QGridLayout* pGridLayoutTop = new QGridLayout(pGridFrameTop);
    pGridLayoutTop->setSpacing(10);
    pGridLayoutTop->setContentsMargins(10, 0, 10, 0);
    //
    pGridLayoutTop->addWidget(pNumPtsLbl,                0, 0);
    pGridLayoutTop->addWidget(m_widgets.pNumDiscrPoints, 0, 1);

    // Layout with degrees.
    QHBoxLayout* pGridLayoutBot = new QHBoxLayout(pGridFrameBot);
    pGridLayoutBot->setSpacing(10);
    pGridLayoutBot->setContentsMargins(10, 0, 10, 0);
    //
    m_widgets.pUDegree->AddTo(pGridLayoutBot);
    m_widgets.pVDegree->AddTo(pGridLayoutBot);
    //
    pGridLayoutBot->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    //pAdvLayout->addWidget(pGridFrameTop);
    pAdvLayout->addWidget(pGridFrameBot);
  }

  // Layout for buttons.
  {
    QHBoxLayout* pButtonsLayout = new QHBoxLayout(bButtonsFrameAC);
    pButtonsLayout->setSpacing(10);
    //
    pButtonsLayout->addWidget(m_widgets.pApply);
    pButtonsLayout->addWidget(m_widgets.pClose);
  }

  // Selectors.
  m_pProfileSelector = new asiUI_DialogBuildGordonSelectEdges(m_model, this, m_pViewer, true);
  m_pGuideSelector   = new asiUI_DialogBuildGordonSelectEdges(m_model, this, m_pViewer, false);

  //---------------------------------------------------------------------------
  // Progress indication
  //---------------------------------------------------------------------------

  m_widgets.pProgressFrame = new QWidget(this);

  // Progress indicator.
  m_widgets.pProgressBar = new QProgressBar;
  m_widgets.pProgressBar->setRange(0, 0);
  m_widgets.pProgressBar->setTextVisible(false);
  m_widgets.pProgressBar->setAlignment(Qt::AlignHCenter);
  m_widgets.pProgressBar->setMinimumWidth(200);
  m_widgets.pProgressFrame->hide();

  // Cancel button
  QIcon cancelIcon(":icons/asitus/asitus_cancel_icon_16x16.png");
  //
  QToolButton* pCancelButton = new QToolButton(m_widgets.pProgressFrame);
  pCancelButton->setIcon(cancelIcon);
  pCancelButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  pCancelButton->setSizePolicy( QSizePolicy(QSizePolicy::Preferred,
                                            QSizePolicy::Expanding) );
  //
  connect( pCancelButton, SIGNAL( clicked() ), this, SLOT( onCancel() ) );

  // Configure layout.
  QHBoxLayout* progressLayout = new QHBoxLayout();
  progressLayout->setMargin(2);
  progressLayout->setSpacing(3);
  progressLayout->addWidget(m_widgets.pProgressBar, 10, Qt::AlignVCenter);
  progressLayout->addWidget(pCancelButton, 0, Qt::AlignVCenter);
  //
  m_widgets.pProgressFrame->setLayout(progressLayout);

#if defined USE_MOBIUS
  // Progress entry.
  m_mobProgress = mobius::core_ProgressEntry( new asiUI_MobiusProgressNotifier(m_progress,
                                                                               m_widgets.pProgressBar) );
#endif

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(pGroupEConstraints);
  m_pMainLayout->addWidget(pGroupAdvanced);
  m_pMainLayout->addWidget(m_widgets.pProgressFrame);
  m_pMainLayout->addWidget(bButtonsFrame);
  m_pMainLayout->addWidget(bButtonsFrameAC);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  // Set good initial size.
  this->setMinimumSize( QSize(150, 250) );

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::NonModal);
  //this->setWindowFlag(Qt::WindowStaysOnTopHint);
  this->setWindowTitle("Build Gordon surface");
  this->setWindowIcon( QIcon( QPixmap( (const char**) image0_data ) ) );
}

//-----------------------------------------------------------------------------

asiUI_DialogBuildGordon::~asiUI_DialogBuildGordon()
{}

//-----------------------------------------------------------------------------

void asiUI_DialogBuildGordon::onEdgePicked(bool isProfile)
{
  if ( !m_model || !m_pViewer )
    return;

  this->show();

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  // Get selected edges.
  const std::vector<int>& eids = m_pProfileSelector->PickedEdgeIds;
  //
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  //
  if ( aag.IsNull() )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "Active part is null. Please, load a model before setting edge constraints.");
    return;
  }

  const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();

  // Collect edges.
  BRep_Builder bbuilder;
  TopoDS_Compound edgeComp;
  bbuilder.MakeCompound(edgeComp);
  //
  QStringList edgeIds;
  for ( const auto edgeId : eids )
  {
    if ( edgeId >= 1 && edgeId <= allEdges.Extent() )
    {
      bbuilder.Add( edgeComp, allEdges(edgeId) );
      edgeIds.append( QString::number(edgeId) );
    }
    else
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Edge with ID %1 does not exist in the model."
                                                << edgeId);
    }
  }

  if (isProfile)
  {
    m_widgets.pProfiles->setText(edgeIds.join(" "));
  }
  else
  {
    m_widgets.pGuides->setText(edgeIds.join(" "));
  }
}

//-----------------------------------------------------------------------------

void asiUI_DialogBuildGordon::onApply()
{
  if ( !m_model || !m_pViewer )
    return;

  /* ==================
   *  Read information.
   * ================== */

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  Handle(asiAlgo_AAG) aag = partApi.GetAAG();

  std::vector<TopoDS_Edge> profiles;
  std::vector<TopoDS_Edge> guides;

  if ( !aag.IsNull() )
  {
    const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();

    // Read profiles indices.
    QStringList eidList = m_widgets.pProfiles->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);
    //
    for ( const auto& eidStr : eidList )
    {
      const int eid = eidStr.toInt();

      if ( (eid > 0) && ( eid <= allEdges.Extent() ) )
      {
        const TopoDS_Shape& edge = allEdges.FindKey(eid);
        profiles.push_back(TopoDS::Edge(edge));
      }
      else
      {
        m_progress.SendLogMessage( LogErr(Normal) << "Input index %1 is out of range [1, %2]."
                                                  << eid << allEdges.Extent() );
      }
    }

    // Read guides indices.
    QStringList eidGList = m_widgets.pGuides->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);
    //
    for ( const auto& eidStr : eidGList )
    {
      const int eid = eidStr.toInt();

      if ( (eid > 0) && ( eid <= allEdges.Extent() ) )
      {
        const TopoDS_Shape& edge = allEdges.FindKey(eid);
        guides.push_back(TopoDS::Edge(edge));
      }
      else
      {
        m_progress.SendLogMessage( LogErr(Normal) << "Input index %1 is out of range [1, %2]."
          << eid << allEdges.Extent() );
      }
    }
  }

  // Read num of discr points.
  const int numOfDiscrPnts = m_widgets.pNumDiscrPoints->text().toInt();

  // U/V degrees.
  const int UDegree = QVariant( m_widgets.pUDegree->GetString() ).toInt();
  const int VDegree = QVariant( m_widgets.pVDegree->GetString() ).toInt();
  //
  if ( !UDegree || !VDegree )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Please, make sure to specify correct U and V surface degrees.");
    return;
  }

  /* ==========
   *  GORDON
   * ========== */

  Handle(Geom_BSplineSurface) resSurf;
  TopoDS_Face                 resFace;

  // Build Gordon surface.
  asiAlgo_BuildGordonSurf buildGordon( m_progress, m_plotter );
  //
  if ( !buildGordon.Build(profiles, guides, resSurf, resFace) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot build Gordon surface.");
  }

  if ( !resSurf.IsNull() )
    m_plotter.DRAW_SURFACE(resSurf, Color_Default, "gordonSurf");

  if ( !resFace.IsNull() )
    m_plotter.DRAW_SHAPE(resFace, Color_Default, "gordonFace");

  m_widgets.pProgressFrame->hide();
}

//-----------------------------------------------------------------------------

void asiUI_DialogBuildGordon::onCancel()
{
  m_mobProgress.AskCancel();
}

//-----------------------------------------------------------------------------

void asiUI_DialogBuildGordon::onProfile()
{
  this->hide();
  //
  m_pProfileSelector->PickedEdgeIds.clear();
  m_pProfileSelector->PickedEdgeGidsMap.Clear();
  m_pProfileSelector->show();
}

//-----------------------------------------------------------------------------

void asiUI_DialogBuildGordon::onGuide()
{
  this->hide();
  //
  m_pGuideSelector->PickedEdgeIds.clear();
  m_pGuideSelector->PickedEdgeGidsMap.Clear();
  m_pGuideSelector->show();
}

//-----------------------------------------------------------------------------

asiUI_DialogBuildGordonSelectEdges::asiUI_DialogBuildGordonSelectEdges(const Handle(asiEngine_Model)& model,
                                                                       asiUI_DialogBuildGordon*       mainDialog,
                                                                       asiUI_ViewerPart*              pViewer,
                                                                       bool                           isProfile,
                                                                       QWidget*                       parent)
: QDialog      (parent),
  m_model      (model),
  m_mainDialog (mainDialog),
  m_pViewer    (pViewer),
  m_isProfile  (isProfile)
{
  m_mainDialog->hide();

  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters.
  QFrame* bButtonsFrameAC = new QFrame;

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");
  m_widgets.pApply->setFocusPolicy( Qt::NoFocus );

  m_widgets.pClose = new QPushButton("Close");
  m_widgets.pClose->setFocusPolicy( Qt::NoFocus );

  // Sizing.
  m_widgets.pApply->setMinimumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMinimumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction.
  connect( m_widgets.pApply, SIGNAL ( clicked() ),
           this,             SLOT   ( onApply() ) );
  connect( m_widgets.pClose, SIGNAL ( clicked() ),
           this,             SLOT   ( onCancel() ) );
  connect( m_pViewer,        SIGNAL ( edgePicked(asiVisu_PickerResult*) ),
           this,             SLOT   ( onEdgePicked(asiVisu_PickerResult*) ) );

  // Layout for buttons.
  {
    QHBoxLayout* pButtonsLayout = new QHBoxLayout(bButtonsFrameAC);
    pButtonsLayout->setSpacing(10);
    //
    pButtonsLayout->addWidget(m_widgets.pApply);
    pButtonsLayout->addWidget(m_widgets.pClose);
  }

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(bButtonsFrameAC);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  // Set good initial size.
  this->setMinimumSize( QSize(150, 70) );

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::NonModal);
  this->setWindowFlag(Qt::WindowStaysOnTopHint);
  this->setWindowTitle("Choose edges");
  this->setWindowIcon( QIcon( QPixmap( (const char**) image1_data ) ) );
}

//------------------------------------------------------------------------------

void asiUI_DialogBuildGordonSelectEdges::onApply()
{
  this->accept();
  if (m_isProfile)
  {
    m_mainDialog->onEdgePicked(true);
  }
  else
  {
    m_mainDialog->onEdgePicked(false);
  }
  this->hide();
}

//------------------------------------------------------------------------------

void asiUI_DialogBuildGordonSelectEdges::onCancel()
{
  this->reject();
  this->hide();
  m_mainDialog->show();
}

//------------------------------------------------------------------------------

void asiUI_DialogBuildGordonSelectEdges::onEdgePicked(asiVisu_PickerResult* pickRes)
{
  Handle(asiVisu_CellPickerResult)
    cellPickerResult = Handle(asiVisu_CellPickerResult)::DownCast(pickRes);

  TColStd_PackedMapOfInteger
    pickedGids = cellPickerResult->GetPickedElementIds();
  //
  if ( pickedGids.IsEmpty() )
  {
    this->PickedEdgeGidsMap.Clear();
    this->PickedEdgeIds.clear();
  }

  // Get sub-shapes map.
  const TopTools_IndexedMapOfShape&
    allSubShapes = m_model->GetPartNode()->GetAAG()->RequestMapOfSubShapes();

  // Get map of edges.
  const TopTools_IndexedMapOfShape&
    allEdges = m_model->GetPartNode()->GetAAG()->RequestMapOfEdges();

  // Add only new IDs.
  pickedGids.Subtract(this->PickedEdgeGidsMap);
  //
  for ( TColStd_PackedMapOfInteger::Iterator git(pickedGids); git.More(); git.Next() )
  {
    const int gid = git.Key();
    this->PickedEdgeGidsMap.Add(gid);

    // Get sub-shape.
    const TopoDS_Shape& subShape = allSubShapes(gid);

    // Get edge index.
    const int eid = allEdges.FindIndex(subShape);

    this->PickedEdgeIds.push_back(eid);
  }
}
