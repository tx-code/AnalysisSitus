//-----------------------------------------------------------------------------
// Created on: 16 February 2016
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/
//-----------------------------------------------------------------------------

// Own include
#include <gui_dialog_mesh.h>

// Common includes
#include <common_facilities.h>

// Mesh includes
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_MeshInfo.h>

// Engine includes
#include <asiEngine_Mesh.h>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>

// OCCT includes
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>

//-----------------------------------------------------------------------------

#define CONTROL_EDIT_WIDTH 100
#define CONTROL_BTN_WIDTH 150

//-----------------------------------------------------------------------------
// Construction & destruction
//-----------------------------------------------------------------------------

//! Constructor.
//! \param parent [in] parent widget.
gui_dialog_mesh::gui_dialog_mesh(QWidget* parent)
//
: QDialog(parent)
{
  // Main layout
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters
  QGroupBox* pGroup = new QGroupBox("Deflection");

  // Editors
  m_widgets.pLinearDeflection  = new gui_line_edit();
  m_widgets.pAngularDeflection = new gui_line_edit();

  // Sizing
  m_widgets.pLinearDeflection->setMinimumWidth(CONTROL_EDIT_WIDTH);
  m_widgets.pAngularDeflection->setMinimumWidth(CONTROL_EDIT_WIDTH);

  // Set mesh parameters from Data Model
  asiAlgo_MeshParams params;
  asiEngine_Mesh::GetParameters(params);

  // Default values
  m_widgets.pLinearDeflection->setText( QString::number(params.Deflection.Linear) );
  m_widgets.pAngularDeflection->setText( QString::number(params.Deflection.Angular) );

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pGenerate = new QPushButton("Generate");

  // Sizing
  m_widgets.pGenerate->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction
  connect( m_widgets.pGenerate, SIGNAL( clicked() ), this, SLOT( onGenerate() ) );

  //---------------------------------------------------------------------------
  // Line editors
  //---------------------------------------------------------------------------

  // Create layout
  QGridLayout* pGrid = new QGridLayout(pGroup);
  pGrid->setSpacing(5);
  pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  //
  pGrid->addWidget(new QLabel("Linear deflection:"),  0, 0);
  pGrid->addWidget(new QLabel("Angular deflection:"), 1, 0);
  //
  pGrid->addWidget(m_widgets.pLinearDeflection,  0, 1);
  pGrid->addWidget(m_widgets.pAngularDeflection, 1, 1);
  //
  pGrid->setColumnStretch(0, 0);
  pGrid->setColumnStretch(1, 1);
  pGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout
  m_pMainLayout->addWidget(pGroup);
  m_pMainLayout->addWidget(m_widgets.pGenerate);
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::WindowModal);
  this->setWindowTitle("Mesh parameters");
}

//! Destructor.
gui_dialog_mesh::~gui_dialog_mesh()
{
  delete m_pMainLayout;
}

//-----------------------------------------------------------------------------

//! Reaction on generation button.
void gui_dialog_mesh::onGenerate()
{
  // Access CAD to mesh
  TopoDS_Shape fullCAD = common_facilities::Instance()->Model_XDE->GetOneShape();

  // Read user inputs
  QVariant qlDefl = m_widgets.pLinearDeflection->text();
  QVariant qaDefl = m_widgets.pAngularDeflection->text();

  // Convert user inputs to doubles
  asiAlgo_MeshParams params;
  params.Deflection.Linear  = qlDefl.toDouble();
  params.Deflection.Angular = qaDefl.toDouble();

  // Save in Data Model
  Handle(xde_model) M = common_facilities::Instance()->Model_XDE;
  //
  M->OpenCommand();
  {
    asiEngine_Mesh::SaveParameters(params);
  }
  M->CommitCommand();

  // Tessellate CAD shape
  //
  std::cout << "Mesh linear defl. = " << params.Deflection.Linear << std::endl;
  std::cout << "Mesh angular defl. = " << params.Deflection.Angular << std::endl;
  //
  asiAlgo_MeshInfo info;
  if ( !asiAlgo_MeshGen::DoNative(fullCAD, params.Deflection.Linear, params.Deflection.Angular, info) )
  {
    std::cout << "Error: meshing failed" << std::endl;
    this->close();
  }
  //
  std::cout << "Meshing done. Visualizing..." << std::endl;
  info.Dump(std::cout);

  // Update visualization
  //
  common_facilities::Instance()->ViewerDMU->GetContext()->RemoveAll();
  common_facilities::Instance()->ViewerDMU->Visualize(common_facilities::Instance()->Model_XDE, AIS_Shaded);

  // Close
  this->close();
}
