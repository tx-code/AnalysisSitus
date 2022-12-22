//-----------------------------------------------------------------------------
// Created on: 11 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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
#include <asiUI_DialogFindVertex.h>

// asiUI includes
#include <asiUI_Common.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// asiEngine includes
#include <asiEngine_Part.h>

// OCCT includes
#include <TopExp.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>
#pragma warning(pop)

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
asiUI_DialogFindVertex::asiUI_DialogFindVertex(const Handle(asiEngine_Model)&             model,
                                               const vtkSmartPointer<asiVisu_PrsManager>& prsMgr,
                                               asiUI_ViewerPart*                          pViewer,
                                               QWidget*                                   parent)
//
: QDialog   (parent),
  m_pViewer (pViewer),
  m_model   (model),
  m_prsMgr  (prsMgr)
{
  // Main layout
  m_pMainLayout = new QVBoxLayout();

  // Group box for parameters
  QGroupBox* pGroup = new QGroupBox("Vertex to find");

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

  // Reactions
  connect( m_widgets.pUseAddress, SIGNAL( clicked() ), this, SLOT( onUseAddress () ) );
  connect( m_widgets.pFind,       SIGNAL( clicked() ), this, SLOT( onFind       () ) );

  //---------------------------------------------------------------------------
  // Line editors
  //---------------------------------------------------------------------------

  // Create layout
  QGridLayout* pGrid = new QGridLayout(pGroup);
  pGrid->setSpacing(5);
  //
  pGrid->addWidget(new QLabel("Use vertex address:"),       0, 0);
  pGrid->addWidget(new QLabel("Vertex indices (1-based):"), 1, 0);
  pGrid->addWidget(new QLabel("Vertex address:"),           2, 0);
  //
  pGrid->addWidget(m_widgets.pUseAddress, 0, 1);
  pGrid->addWidget(m_widgets.pIndex,      1, 1);
  pGrid->addWidget(m_widgets.pAddress,    2, 1);
  //
  m_widgets.pIndex->setToolTip("Ex: \"10 15 42\" or \"10, 15, 42\"");
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
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::WindowModal);
  this->setWindowTitle("Find vertex by index");

  // Adjust initial state
  this->onUseAddress();
}

//! Destructor.
asiUI_DialogFindVertex::~asiUI_DialogFindVertex()
{
  delete m_pMainLayout;
}

//-----------------------------------------------------------------------------

//! Reaction on clicking "Use address" checkbox.
void asiUI_DialogFindVertex::onUseAddress()
{
  m_widgets.pIndex->setEnabled( !m_widgets.pUseAddress->isChecked() );
  m_widgets.pAddress->setEnabled( m_widgets.pUseAddress->isChecked() );
}

//-----------------------------------------------------------------------------

//! Reaction on clicking "Find" button.
void asiUI_DialogFindVertex::onFind()
{
  const bool useAddress = m_widgets.pUseAddress->isChecked();
  //
  Handle(asiData_PartNode) part_n;
  TopoDS_Shape             part;
  //
  if ( !asiUI_Common::PartShape(m_model, part_n, part) ) return;

  const TopTools_IndexedMapOfShape&
    vertices = m_model->GetPartNode()->GetAAG()->RequestMapOfVertices();
  //
  TopTools_IndexedMapOfShape found;

  if ( useAddress )
  {
    TCollection_AsciiString vertex_addr = QStr2AsciiStr( m_widgets.pAddress->text() );
    vertex_addr.LowerCase();

    for ( int v = 1; v <= vertices.Extent(); ++v )
    {
      TCollection_AsciiString next_addr = asiAlgo_Utils::ShapeAddr( vertices(v) ).c_str();
      next_addr.LowerCase();

      if ( next_addr.IsEqual(vertex_addr) )
      {
        found.Add( vertices(v) );
        break;
      }
    }
  }
  else
  {
    QStringList vidList = m_widgets.pIndex->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);

    for ( const auto& vidStr : vidList )
    {
      // Read user inputs
      const int vertex_id = vidStr.toInt();

      if ( vertex_id > 0 && vertex_id <= vertices.Extent() )
      {
        const TopoDS_Shape& vertex = vertices.FindKey(vertex_id);
        found.Add(vertex);
      }
      else
      {
        std::cout << "Error: input index is out of range" << std::endl;
      }
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
    std::cout << "No vertex found" << std::endl;
  }

  // Close
  this->close();
}

//-----------------------------------------------------------------------------

void asiUI_DialogFindVertex::showEvent(QShowEvent* evt)
{
  QDialog::showEvent(evt);

  // Set initial focus to reduce excessive clicks.
  m_widgets.pIndex->setFocus();
}
