//-----------------------------------------------------------------------------
// Created on: 21 January 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef asiUI_DialogDump_h
#define asiUI_DialogDump_h

// asiUI includes
#include <asiUI_StyledTextEdit.h>
#include <asiUI_SearchLine.h>

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#pragma warning(pop)

class asiUI_JsonEditor;

//! Text view for any dumps.
class asiUI_DialogDump : public QDialog
{
  Q_OBJECT

public:

  //! Ctor.
  //! \param[in] title  the dialog title to set.
  //! \param[in] parent the optional parent widget.
  asiUI_EXPORT
    asiUI_DialogDump(const QString& title,
                     QWidget*       parent = nullptr);

  //! Dtor.
  asiUI_EXPORT virtual
    ~asiUI_DialogDump();

public:

  //! Populates the widget's text area with the passed string buffer.
  //! \param[in] buff the text buffer to dump.
  asiUI_EXPORT void
    Populate(const std::string& buff);

public slots:

  //! Reaction on close.
  void onClose();

protected:

  QVBoxLayout* m_pMainLayout; //!< Layout of the widget.

  //! Widgets.
  struct t_base_widgets
  {
    asiUI_SearchLine* pSearchLine; //!< search control.
    asiUI_JsonEditor* pEditor;     //!< Text editor.
    QPushButton*      pClose;      //!< Close button.

    t_base_widgets() : pSearchLine(nullptr),
                       pEditor (nullptr),
                       pClose  (nullptr)
    {}

    void Release()
    {
      pEditor = nullptr;
      delete pSearchLine; pSearchLine = nullptr;
      delete pClose;      pClose = nullptr;
    }
  };

  t_base_widgets m_widgets; //!< Involved widgets.

};

#endif
