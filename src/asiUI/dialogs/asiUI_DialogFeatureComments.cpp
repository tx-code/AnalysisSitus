//-----------------------------------------------------------------------------
// Created on: 20 October 2022
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
#include <asiUI_DialogFeatureComments.h>

// asiUI includes
#include <asiUI_Common.h>

// Qt includes
#pragma warning(push, 0)
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>
#pragma warning(pop)

//-----------------------------------------------------------------------------

#define BTN_MIN_WIDTH 120

//-----------------------------------------------------------------------------
// Construction & destruction
//-----------------------------------------------------------------------------

asiUI_DialogFeatureComments::asiUI_DialogFeatureComments(const Handle(ActAPI_IModel)&       model,
                                                         const Handle(asiData_FeatureNode)& node,
                                                         ActAPI_ProgressEntry               progress,
                                                         ActAPI_PlotterEntry                plotter,
                                                         QWidget*                           parent)
: QDialog    (parent),
  m_model    (model),
  m_node     (node),
  m_progress (progress),
  m_plotter  (plotter)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Editors.
  m_widgets.pText = new asiUI_StyledTextEdit();

  // Frame for text.
  QFrame* pTextGroup = new QFrame;

  // JSON definition layout.
  QVBoxLayout* pBoxLayout = new QVBoxLayout(pTextGroup);
  pBoxLayout->addWidget(m_widgets.pText);

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");
  m_widgets.pClose = new QPushButton("Close");

  // Layout for buttons.
  QFrame*      bButtonsFrame  = new QFrame;
  QHBoxLayout* pButtonsLayout = new QHBoxLayout(bButtonsFrame);
  //
  pButtonsLayout->addWidget(m_widgets.pApply);
  pButtonsLayout->addWidget(m_widgets.pClose);

  // Sizing.
  m_widgets.pApply->setMaximumWidth(BTN_MIN_WIDTH);
  m_widgets.pClose->setMaximumWidth(BTN_MIN_WIDTH);

  // Reactions.
  connect( m_widgets.pApply, SIGNAL( clicked() ), this, SLOT( onApply() ) );
  connect( m_widgets.pClose, SIGNAL( clicked() ), this, SLOT( onClose() ) );

  //---------------------------------------------------------------------------
  // Layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(pTextGroup);
  m_pMainLayout->addWidget(bButtonsFrame);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::NonModal);
  this->setWindowTitle("Feature comments");

  // Set good initial size.
  this->setMinimumSize( QSize(600, 600) );
}

//-----------------------------------------------------------------------------

//! Destructor.
asiUI_DialogFeatureComments::~asiUI_DialogFeatureComments()
{
  delete m_pMainLayout;
  m_widgets.Release();
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------

//! Fills editor with data.
void asiUI_DialogFeatureComments::Initialize()
{
  if ( m_node.IsNull() )
    return;

  // Set text.
  m_widgets.pText->setText( AsciiStr2QStr( m_node->GetComment() ) );
}

//-----------------------------------------------------------------------------
// Slots
//-----------------------------------------------------------------------------

//! Reaction on apply.
void asiUI_DialogFeatureComments::onApply()
{
  TCollection_AsciiString comment = QStr2AsciiStr( m_widgets.pText->toPlainText() );

  m_model->OpenCommand();
  {
    m_node->SetComment(comment);
  }
  m_model->CommitCommand();

  this->close();
}

//! Reaction on close.
void asiUI_DialogFeatureComments::onClose()
{
  this->close();
}
