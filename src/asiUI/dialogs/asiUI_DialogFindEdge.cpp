//-----------------------------------------------------------------------------
// Created on: 01 December 2016 (*)
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <asiUI_DialogFindEdge.h>

// GUI includes
#include <asiUI_Common.h>

// Geometry includes
#include <asiAlgo_Utils.h>

// Engine includes
#include <asiEngine_Part.h>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>

// OCCT includes
#include <TopExp.hxx>

//-----------------------------------------------------------------------------

#define CONTROL_EDIT_WIDTH 100
#define CONTROL_BTN_WIDTH 150

//-----------------------------------------------------------------------------
// Construction & destruction
//-----------------------------------------------------------------------------

//! Constructor.
//! \param model   [in] Data Model instance.
//! \param prsMgr  [in] presentation manager.
//! \param pViewer [in] connected viewer.
//! \param parent  [in] parent widget.
asiUI_DialogFindEdge::asiUI_DialogFindEdge(const Handle(asiEngine_Model)&             model,
                                           const vtkSmartPointer<asiVisu_PrsManager>& prsMgr,
                                           asiUI_ViewerPart*                          pViewer,
                                           QWidget*                                   parent)
//
: QDialog   (parent),
  m_model   (model),
  m_prsMgr  (prsMgr),
  m_pViewer (pViewer)
{
  // Main layout
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters
  QGroupBox* pGroup = new QGroupBox("Edge to find");

  // Editors
  m_widgets.pUseAddress = new QCheckBox();
  m_widgets.pIndex      = new asiUI_LineEdit();
  m_widgets.pAddress    = new asiUI_LineEdit();

  // Sizing
  m_widgets.pIndex  ->setMinimumWidth(CONTROL_EDIT_WIDTH);
  m_widgets.pAddress->setMinimumWidth(CONTROL_EDIT_WIDTH);

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pFind = new QPushButton("Find");

  // Sizing
  m_widgets.pFind->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction
  connect( m_widgets.pUseAddress, SIGNAL( clicked() ), this, SLOT( onUseAddress () ) );
  connect( m_widgets.pFind,       SIGNAL( clicked() ), this, SLOT( onFind       () ) );

  //---------------------------------------------------------------------------
  // Line editors
  //---------------------------------------------------------------------------

  // Create layout
  QGridLayout* pGrid = new QGridLayout(pGroup);
  pGrid->setSpacing(5);
  pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  //
  pGrid->addWidget(new QLabel("Use edge address:"),     0, 0);
  pGrid->addWidget(new QLabel("Edge index (1-based):"), 1, 0);
  pGrid->addWidget(new QLabel("Edge address:"),         2, 0);
  //
  pGrid->addWidget(m_widgets.pUseAddress, 0, 1);
  pGrid->addWidget(m_widgets.pIndex,      1, 1);
  pGrid->addWidget(m_widgets.pAddress,    2, 1);
  //
  pGrid->setColumnStretch(0, 0);
  pGrid->setColumnStretch(1, 1);
  pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout
  m_pMainLayout->addWidget(pGroup);
  m_pMainLayout->addWidget(m_widgets.pFind);
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::WindowModal);
  this->setWindowTitle("Find edge by index");

  // Adjust initial state
  this->onUseAddress();
}

//! Destructor.
asiUI_DialogFindEdge::~asiUI_DialogFindEdge()
{
  delete m_pMainLayout;
}

//-----------------------------------------------------------------------------

//! Reaction on clicking "Use address" checkbox.
void asiUI_DialogFindEdge::onUseAddress()
{
  m_widgets.pIndex->setEnabled( !m_widgets.pUseAddress->isChecked() );
  m_widgets.pAddress->setEnabled( m_widgets.pUseAddress->isChecked() );
}

//-----------------------------------------------------------------------------

//! Reaction on clicking "Find" button.
void asiUI_DialogFindEdge::onFind()
{
  const bool useAddress = m_widgets.pUseAddress->isChecked();
  //
  Handle(asiData_PartNode) part_n;
  TopoDS_Shape             part;
  //
  if ( !asiUI_Common::PartShape(m_model, part_n, part) ) return;

  TopTools_IndexedMapOfShape edges, found;
  TopExp::MapShapes(part, TopAbs_EDGE, edges);

  if ( useAddress )
  {
    TCollection_AsciiString edge_addr = QStr2AsciiStr( m_widgets.pAddress->text() );
    edge_addr.LowerCase();

    for ( int e = 1; e <= edges.Extent(); ++e )
    {
      TCollection_AsciiString next_addr = asiAlgo_Utils::ShapeAddr( edges(e) ).c_str();
      next_addr.LowerCase();

      if ( next_addr.IsEqual(edge_addr) )
      {
        found.Add( edges(e) );
        break;
      }
    }
  }
  else
  {
    // Read user inputs
    const int edge_id = m_widgets.pIndex->text().toInt();

    if ( edge_id > 0 && edge_id <= edges.Extent() )
    {
      const TopoDS_Shape& edge = edges.FindKey(edge_id);
      found.Add(edge);
    }
    else
    {
      std::cout << "Error: input index is out of range" << std::endl;
    }
  }

  if ( !found.IsEmpty() )
  {
    // Highlight
    asiEngine_Part(m_model, m_prsMgr).HighlightSubShapes(found);
    //
    m_pViewer->onSubShapesPicked(); // TODO: change with event
  }
  else
  {
    std::cout << "No edge found" << std::endl;
  }

  // Close
  this->close();
}