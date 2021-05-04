//-----------------------------------------------------------------------------
// Created on: 26 December 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <asiUI_DialogXdeSummary.h>

// asiAlgo includes
#include <asiAlgo_Timer.h>

// asiAsm includes
#include <asiAsm_XdeGraph.h>

// asiUI includes
#include <asiUI_StyledTextEdit.h>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>

//-----------------------------------------------------------------------------

#define BTN_MIN_WIDTH 120

//-----------------------------------------------------------------------------

asiUI_DialogXdeSummary::asiUI_DialogXdeSummary(const Handle(asiAsm::xde::Doc)& xdeDoc,
                                               ActAPI_ProgressEntry            progress,
                                               QWidget*                        parent)
: QDialog(parent), m_asmModel(xdeDoc), m_progress(progress)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Group box for text editor.
  QGroupBox* pGroup = new QGroupBox("Summary");

  // Editors.
  m_widgets.pEditor = new asiUI_StyledTextEdit();
  //
  QVBoxLayout* boxLayout = new QVBoxLayout(pGroup);
  boxLayout->addWidget(m_widgets.pEditor);

  /* Buttons */

  m_widgets.pClose = new QPushButton("Close");

  // Sizing.
  m_widgets.pClose->setMaximumWidth(BTN_MIN_WIDTH);

  // Reaction.
  connect( m_widgets.pClose, SIGNAL( clicked() ), this, SLOT( onClose() ) );

  /* Layouts */

  // Configure main layout.
  m_pMainLayout->addWidget(pGroup);
  m_pMainLayout->addWidget(m_widgets.pClose);
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::NonModal);
  this->setWindowTitle("Assembly summary");

  // Fill editor with data.
  this->initialize();
}

//-----------------------------------------------------------------------------

asiUI_DialogXdeSummary::~asiUI_DialogXdeSummary()
{
  delete m_pMainLayout;
  m_widgets.Release();
}

//-----------------------------------------------------------------------------

void asiUI_DialogXdeSummary::initialize()
{
  if ( m_asmModel.IsNull() )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "Assembly model is not initialized." );
    this->close();
    return;
  }

  std::ostringstream buff;

  TIMER_NEW
  TIMER_GO

  // Prepare assembly graph.
  Handle(asiAsm::xde::Graph)
    asmGraph = new asiAsm::xde::Graph(m_asmModel);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_progress, "Build assembly graph")

  TIMER_RESET
  TIMER_GO

  // Get summary.
  int numRoots                = 0;
  int numSubassemblyInstances = 0;
  int numSubassemblies        = 0;
  int numPartInstances        = 0;
  int numParts                = 0;
  //
  asmGraph->CalculateSummary(numRoots,
                             numSubassemblyInstances,
                             numSubassemblies,
                             numPartInstances,
                             numParts);

  // Calculate number of scene objects.
  Handle(asiAsm::xde::HAssemblyItemIdsMap)
    items = new asiAsm::xde::HAssemblyItemIdsMap;
  //
  m_asmModel->GetLeafAssemblyItems(items);
  //
  const int numObjects = items->Extent();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_progress, "Calculate assembly summary")

  // Dump.
  buff << "Number of roots: "                     << numRoots                << "\n";
  buff << "Number of (sub)assembly occurrences: " << numSubassemblyInstances << "\n";
  buff << "Number of (sub)assemblies: "           << numSubassemblies        << "\n";
  buff << "Number of part occurrences: "          << numPartInstances        << "\n";
  buff << "Number of parts: "                     << numParts                << "\n";
  buff << "Number of leaf assembly items: "       << numObjects              << "\n";

  // Print to widget.
  m_widgets.pEditor->setText( buff.str().c_str() );
}

//-----------------------------------------------------------------------------

void asiUI_DialogXdeSummary::onClose()
{
  this->close();
}
