//-----------------------------------------------------------------------------
// Created on: 06 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Natalia Ermolaeva
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

#ifndef asiUI_SearchLine_h
#define asiUI_SearchLine_h

// asiUI includes
#include <asiUI_CommonFacilities.h>

// Qt includes
#pragma warning(push, 0)
#include <QLineEdit>
#pragma warning(pop)

class QKeyEvent;
class QPaintEvent;

#pragma warning(disable : 4251)

//! Extending the line edit with the search button in the right part of the line.
//! It emits signal about search change by text typing and about search deactivating by
//! entering the empty text or pressing Escape button.

class asiUI_EXPORT asiUI_SearchLine : public QLineEdit
{
  Q_OBJECT

public:

  //! Creates a new instance of Json view.
  //! \param[in] placeHolderText invitation that is shown when the line is empty
  //! \param[in] parent          parent widget (if any).
  asiUI_SearchLine(const QString& placeHolerText, QWidget* parent = nullptr);

  //! Destructor.
  virtual ~asiUI_SearchLine();

  //! Sets the empty text and fill place holder.
  void reset();

  //! Paints the line edit an the button on the right.
  //! \param[in] event painting event
  void paintEvent(QPaintEvent* event) override;

  //! Processes the key event escape.
  //! \param [in] event
  void keyPressEvent(QKeyEvent *event) override;

signals:
  //! Signal about activation the search.
  void searchEntered();

  //! Signal about activation the search.
  void searchChanged(const QString& text);

  //! Signal about deactivation the search.
  void searchDeactivated();

  //! Signal about key 'Up' clicked in the search.
  void searchUp();

  //! Signal about key 'Down' clicked in the search.
  void searchDown();

protected slots:
  //! Processing text change.
  //! \param [in] current text
  void onTextChanged(const QString& text);

private:
  QString m_placeHolderText; //!< text for place holder
};

#pragma warning(default : 4251)

#endif
