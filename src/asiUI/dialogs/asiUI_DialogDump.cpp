//-----------------------------------------------------------------------------
// Created on: 21 January 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiUI_DialogDump.h>
#include <asiUI_JsonEditor.h>

// Qt includes
#pragma warning(push, 0)
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>
#pragma warning(pop)

//-----------------------------------------------------------------------------

#define BTN_MIN_WIDTH 120

//-----------------------------------------------------------------------------

asiUI_DialogDump::asiUI_DialogDump(const QString& title,
                                   QWidget*       parent)
: QDialog (parent)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Search control.
  m_widgets.pSearchLine = new asiUI_SearchLine("Search");

  // Editors.
  m_widgets.pEditor = new asiUI_JsonEditor(this);
  m_widgets.pEditor->setImmediateValidate(true);
  connect(m_widgets.pSearchLine, SIGNAL (searchEntered()),     m_widgets.pEditor, SLOT(searchEntered()));
  connect(m_widgets.pSearchLine, SIGNAL (searchDeactivated()), m_widgets.pEditor, SLOT(searchDeactivated()));
  connect(m_widgets.pSearchLine, SIGNAL (searchChanged(const QString&)),
          m_widgets.pEditor,     SLOT   (searchChanged(const QString&)));

  // Button.
  m_widgets.pClose = new QPushButton("Close");

  // Sizing.
  m_widgets.pClose->setMaximumWidth(BTN_MIN_WIDTH);

  // Reaction.
  connect( m_widgets.pClose, SIGNAL( clicked() ), this, SLOT( onClose() ) );
  m_widgets.pClose->setAutoDefault(false);

  // Set up main layout.
  m_pMainLayout->addWidget(m_widgets.pSearchLine);
  m_pMainLayout->addWidget(m_widgets.pEditor);
  m_pMainLayout->addWidget(m_widgets.pClose);
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);
  //
  this->setLayout         (m_pMainLayout);
  this->setWindowModality (Qt::WindowModal);
  this->setWindowTitle    (title);

  // Set good initial size
  this->resize( QSize(900, 600) );
  m_widgets.pSearchLine->setFocus();
}

//-----------------------------------------------------------------------------

asiUI_DialogDump::~asiUI_DialogDump()
{
  delete m_pMainLayout;
  m_widgets.Release();
}

//-----------------------------------------------------------------------------

void asiUI_DialogDump::Populate(const std::string& buff)
{
  m_widgets.pEditor->insertPlainText(buff.c_str());
  m_widgets.pEditor->moveCursor(QTextCursor::MoveOperation::Start);
  m_widgets.pEditor->ensureCursorVisible();
}

//-----------------------------------------------------------------------------

void asiUI_DialogDump::onClose()
{
  this->close();
}
