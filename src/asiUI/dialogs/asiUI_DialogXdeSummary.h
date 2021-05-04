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

#ifndef asiUI_DialogXdeSummary_h
#define asiUI_DialogXdeSummary_h

// asiUI includes
#include <asiUI.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#pragma warning(pop)

#pragma warning(disable : 4251)

//-----------------------------------------------------------------------------

//! Text view for assembly summary.
class asiUI_EXPORT asiUI_DialogXdeSummary : public QDialog
{
  Q_OBJECT

public:

  //! Ctor.
  //! \param[in] xdeDoc   assembly model.
  //! \param[in] progress progress notifier.
  //! \param[in] parent   parent widget.
  asiUI_DialogXdeSummary(const Handle(asiAsm::xde::Doc)& xdeDoc,
                         ActAPI_ProgressEntry            progress,
                         QWidget*                        parent = nullptr);

  //! Dtor.
  virtual
    ~asiUI_DialogXdeSummary();

protected:

  //! Fills editor with data.
  virtual void initialize();

public slots:

  //! Reaction on close.
  void onClose();

protected:

  QVBoxLayout* m_pMainLayout; //!< Layout of the widget.

  //! Widgets.
  struct t_widgets
  {
    QTextEdit*   pEditor; //!< Text editor.
    QPushButton* pClose;  //!< Close button.

    t_widgets() : pEditor (nullptr),
                  pClose  (nullptr)
    {}

    void Release()
    {
      delete pEditor; pEditor = nullptr;
      delete pClose;  pClose  = nullptr;
    }
  };

  t_widgets                m_widgets;  //!< Involved widgets.
  Handle(asiAsm::xde::Doc) m_asmModel; //!< Assembly model.
  ActAPI_ProgressEntry     m_progress; //!< Progress notifier.

};

#endif
