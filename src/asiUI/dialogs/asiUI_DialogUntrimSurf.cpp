//-----------------------------------------------------------------------------
// Created on: 17 May 2023
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
#include "asiUI_DialogUntrimSurf.h"

// asiAlgo includes
#include <asiAlgo_UntrimSurf.h>

// asiEngine includes
#include <asiEngine_Part.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QGroupBox>
#include <QLabel>
//
#include <Standard_WarningsRestore.hxx>

// OpenCascade includes
#include <BRep_Builder.hxx>

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

asiUI_DialogUntrimSurf::asiUI_DialogUntrimSurf(const Handle(asiUI_WidgetFactory)& wf,
                                               const Handle(asiEngine_Model)&     model,
                                               asiUI_ViewerPart*                  pViewer,
                                               ActAPI_ProgressEntry               progress,
                                               ActAPI_PlotterEntry                plotter,
                                               QWidget*                           parent)
: QDialog        (parent),
  m_pViewer      (pViewer),
  m_model        (model),
  m_bDiagnostics (false),
  m_progress     (progress),
  m_plotter      (plotter)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters.
  QFrame*    pMethodPanel      = new QFrame;
  QGroupBox* pGroupInputGeom   = new QGroupBox("Input geometry");
  QGroupBox* pGroupAdvanced    = new QGroupBox("Advanced");
  QFrame*    bButtonsFrame     = new QFrame;
  QFrame*    pDiagnosticsFrame = new QFrame; // Diagnostics frame.

  // Controls.
  m_widgets.pFaces    = new asiUI_LineEdit();
  m_widgets.pEdges    = new asiUI_LineEdit();
  m_widgets.pNumUIsos = wf->CreateEditor("Num_UIsos", this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  m_widgets.pNumVIsos = wf->CreateEditor("Num_VIsos", this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  m_widgets.pUDegree  = wf->CreateEditor("UDegree",   this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  m_widgets.pVDegree  = wf->CreateEditor("VDegree",   this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);

  // Diagnostics dumps.
  m_widgets.pDiagnostics = new QCheckBox;

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");
  m_widgets.pApply->setFocusPolicy( Qt::NoFocus );

  m_widgets.pClose = new QPushButton("Close");
  m_widgets.pClose->setFocusPolicy( Qt::NoFocus );

  // Sizing.
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction.
  connect( m_widgets.pApply,       SIGNAL( clicked () ),
           this,                   SLOT  ( onApply () ) );
  connect( m_widgets.pClose,       SIGNAL( clicked () ),
           this,                   SLOT  ( close   () ) );
  connect( m_widgets.pDiagnostics, SIGNAL( clicked() ),
           this,                   SLOT  ( onDiagnostics() ) );

  //---------------------------------------------------------------------------
  // Layout
  //---------------------------------------------------------------------------

  // Edge constraints.
  {
    QGridLayout* pGrid = new QGridLayout(pGroupInputGeom);
    pGrid->setSpacing(10);
    //
    pGrid->addWidget(new QLabel("Faces to untrim"), 0, 0);
    pGrid->addWidget(m_widgets.pFaces,              0, 1);
    //
    pGrid->addWidget(new QLabel("Boundary edges"), 1, 0);
    pGrid->addWidget(m_widgets.pEdges,             1, 1);
    //
    pGrid->setColumnStretch(0, 0);
    pGrid->setColumnStretch(1, 1);
    pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Advanced.
  {
    QVBoxLayout* pAdvLayout     = new QVBoxLayout(pGroupAdvanced);
    QFrame*      pGridFrameBot1 = new QFrame;
    QFrame*      pGridFrameBot2 = new QFrame;

    pAdvLayout->setAlignment(Qt::AlignTop);

    // Bottom grid 1.
    QHBoxLayout* pGridLayoutBot1 = new QHBoxLayout(pGridFrameBot1);
    pGridLayoutBot1->setSpacing(10);
    pGridLayoutBot1->setContentsMargins(10, 0, 10, 0);
    //
    m_widgets.pNumUIsos->AddTo(pGridLayoutBot1);
    m_widgets.pNumVIsos->AddTo(pGridLayoutBot1);
    //
    pGridLayoutBot1->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // Bottom grid 2.
    QHBoxLayout* pGridLayoutBot2 = new QHBoxLayout(pGridFrameBot2);
    pGridLayoutBot2->setSpacing(10);
    pGridLayoutBot2->setContentsMargins(10, 0, 10, 0);
    //
    m_widgets.pUDegree->AddTo(pGridLayoutBot2);
    m_widgets.pVDegree->AddTo(pGridLayoutBot2);
    //
    pGridLayoutBot2->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    pAdvLayout->addWidget(pGridFrameBot1);
    //pAdvLayout->addWidget(pGridFrameBot2);
  }

  // Layout with diagnostics.
  {
    QLabel* pDiagnosticsLbl = new QLabel("Visual dumps");

    QHBoxLayout* pDiagnosticsLayout = new QHBoxLayout(pDiagnosticsFrame);
    //
    pDiagnosticsLayout->addWidget(pDiagnosticsLbl);
    pDiagnosticsLayout->addWidget(m_widgets.pDiagnostics);
    pDiagnosticsLayout->addStretch(1);

    pDiagnosticsLayout->setSpacing(10);
    pDiagnosticsLayout->setContentsMargins(10, 10, 10, 0);
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
  m_pMainLayout->addWidget(pGroupInputGeom);
  m_pMainLayout->addWidget(pGroupAdvanced);
  m_pMainLayout->addWidget(pDiagnosticsFrame);
  m_pMainLayout->addWidget(bButtonsFrame);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  // Set good initial size.
  this->setMinimumSize( QSize(450, 300) );

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::NonModal);
  //this->setWindowFlag(Qt::WindowStaysOnTopHint);
  this->setWindowTitle("Untrim surface");
  this->setWindowIcon( QIcon( QPixmap( (const char**) image0_data ) ) );

  if ( m_pViewer )
  {
    connect( m_pViewer, SIGNAL ( edgePicked(asiVisu_PickerResult*) ),
             this,      SLOT   ( onEdgePicked() ) );

    connect( m_pViewer, SIGNAL ( facePicked(asiVisu_PickerResult*) ),
             this,      SLOT   ( onFacePicked() ) );
  }
}

//-----------------------------------------------------------------------------

asiUI_DialogUntrimSurf::~asiUI_DialogUntrimSurf()
{}

//-----------------------------------------------------------------------------

void asiUI_DialogUntrimSurf::onEdgePicked()
{
  if ( !m_model || !m_pViewer )
    return;

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  // Get selected edges.
  TColStd_PackedMapOfInteger eids;
  partApi.GetHighlightedEdges(eids);
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
  for ( TColStd_PackedMapOfInteger::Iterator eit(eids); eit.More(); eit.Next() )
  {
    const int edgeId = eit.Key();

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

  m_widgets.pEdges->setText(edgeIds.join(" "));
}

//-----------------------------------------------------------------------------

void asiUI_DialogUntrimSurf::onFacePicked()
{
  if ( !m_model || !m_pViewer )
    return;

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  // Get selected faces.
  TColStd_PackedMapOfInteger fids;
  partApi.GetHighlightedFaces(fids);
  //
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  //
  if ( aag.IsNull() )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "Active part is null. Please, load a CAD model.");
    return;
  }

  const TopTools_IndexedMapOfShape& allFaces = aag->GetMapOfFaces();

  // Collect face IDs.
  QStringList faceIds;
  //
  for ( TColStd_PackedMapOfInteger::Iterator fit(fids); fit.More(); fit.Next() )
  {
    const int faceId = fit.Key();

    if ( faceId >= 1 && faceId <= allFaces.Extent() )
    {
      faceIds.append( QString::number(faceId) );
    }
    else
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Face with ID %1 does not exist in the model."
                                                << faceId);
    }
  }

  m_widgets.pFaces->setText(faceIds.join(" "));
}

//-----------------------------------------------------------------------------

void asiUI_DialogUntrimSurf::onApply()
{
  if ( !m_model || !m_pViewer )
    return;

  // Number of U/V isos.
  const int numUIsos = QVariant( m_widgets.pNumUIsos->GetString() ).toInt();
  const int numVIsos = QVariant( m_widgets.pNumVIsos->GetString() ).toInt();

  // U/V degrees.
  const int UDegree = QVariant( m_widgets.pUDegree->GetString() ).toInt();
  const int VDegree = QVariant( m_widgets.pVDegree->GetString() ).toInt();
  //
  if ( !UDegree || !VDegree )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Please, make sure to specify correct U and V surface degrees.");
    return;
  }

  /* ==================
   *  Read constraints.
   * ================== */

  asiEngine_Part partApi( m_model, m_pViewer->PrsMgr() );

  Handle(asiAlgo_AAG) aag = partApi.GetAAG();

  Handle(TopTools_HSequenceOfShape) hedges = new TopTools_HSequenceOfShape;
  Handle(TopTools_HSequenceOfShape) hfaces = new TopTools_HSequenceOfShape;

  if ( !aag.IsNull() )
  {
    const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();
    const TopTools_IndexedMapOfShape& allFaces = aag->GetMapOfFaces();

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
        m_progress.SendLogMessage( LogErr(Normal) << "Input edge index %1 is out of range [1, %2]."
                                                  << eid << allEdges.Extent() );
      }
    }

    // Read face indices.
    QStringList fidList = m_widgets.pFaces->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);
    //
    for ( const auto& fidStr : fidList )
    {
      const int fid = fidStr.toInt();

      if ( (fid > 0) && ( fid <= allFaces.Extent() ) )
      {
        const TopoDS_Shape& face = allFaces.FindKey(fid);
        hfaces->Append(face);
      }
      else
      {
        m_progress.SendLogMessage( LogErr(Normal) << "Input face index %1 is out of range [1, %2]."
                                                  << fid << allFaces.Extent() );
      }
    }
  }

  /* ================
   *  Untrim surface.
   * ================ */

  Handle(Geom_BSplineSurface) surf;
  TopoDS_Face                 face;

  // Prepare untrimming tool.
  asiAlgo_UntrimSurf UNTRIM(m_progress, m_bDiagnostics ? m_plotter : nullptr);
  //
  UNTRIM.SetNumUIsos (numUIsos);
  UNTRIM.SetNumVIsos (numVIsos);
  UNTRIM.SetDegreeU  (UDegree);
  UNTRIM.SetDegreeV  (VDegree);

  // Perform.
  if ( !UNTRIM.Build(hfaces, hedges, surf, face) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "UNTRIM failed.");
    return;
  }

  m_progress.SendLogMessage(LogInfo(Normal) << "UNTRIM operation is done.");

  // Visually dump guide and profile curves.
  BRep_Builder    bbuilder;
  TopoDS_Compound guidesComp, profilesComp;
  //
  bbuilder.MakeCompound(guidesComp);
  bbuilder.MakeCompound(profilesComp);
  //
  const std::vector<TopoDS_Edge>& guides   = UNTRIM.GetGuides();
  const std::vector<TopoDS_Edge>& profiles = UNTRIM.GetProfiles();
  //
  for ( const auto& guide : guides )
  {
    bbuilder.Add(guidesComp, guide);
  }
  //
  for ( const auto& profile : profiles )
  {
    bbuilder.Add(profilesComp, profile);
  }
  //
  m_plotter.REDRAW_SHAPE("guides",   guidesComp,   Color_Red);
  m_plotter.REDRAW_SHAPE("profiles", profilesComp, Color_Green);

  if ( !surf.IsNull() )
    m_plotter.DRAW_SURFACE(surf, Color_Default, "untrimSurf");

  if ( !face.IsNull() )
    m_plotter.DRAW_SHAPE(face, Color_Default, "untrimFace");
}

//-----------------------------------------------------------------------------

void asiUI_DialogUntrimSurf::onDiagnostics()
{
  m_bDiagnostics = m_widgets.pDiagnostics->isChecked();
}
