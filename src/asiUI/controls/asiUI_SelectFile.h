//-----------------------------------------------------------------------------
// Created on: 08 March 2023
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

#ifndef asiUI_SelectFile_h
#define asiUI_SelectFile_h

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_CommonFacilities.h>

// Qt includes
#pragma warning(push, 0)
#include <QImage>
#include <QLineEdit>
#pragma warning(pop)

class QKeyEvent;
class QPaintEvent;

#pragma warning(disable : 4251)

//! Extending the line edit with the open button in the right part of the line.
//! It opens the dialog to select the file and fill the line edit control.
class asiUI_EXPORT asiUI_SelectFile : public QLineEdit
{
  Q_OBJECT

public:

  //! Creates a new instance of Json view.
  //! \param[in] pFilter        filter for extensions.
  //! \param[in] pOpenTitle;    title for open dialog.
  //! \param[in] pPreferredName preferred filename.
  //! \param[in] icon           icon to show in the line edit.
  //! \param[in] pAction;       open/save action.
  //! \param[in] parent         parent widget (if any).
  asiUI_SelectFile(const QString&       filter,
                   const QString&       openTitle,
                   const QString&       preferredName,
                   const QImage&        image,
                   const asiUI_Common::OpenSaveAction action,
                   QWidget*             parent = nullptr);

  //! Destructor.
  virtual ~asiUI_SelectFile();

  //! Sets the empty text and fill place holder.
  void reset();

  //! Receives events to an object and should return true if the event e was recognized and processed.
  //! It processes the mouse key press on the button.
  bool event(QEvent* event) override;

  //! Paints the line edit an the button on the right.
  //! \param[in] event painting event
  void paintEvent(QPaintEvent* event) override;

protected:
  //! Opens dialog to select file and fill line edit by the selected name.
  void selectFileName();

private:
  QString                      m_filter;        //!< filter for extensions.
  QString                      m_openTitle;     //!< title for open dialog.
  QString                      m_preferredName; //!< preferred filename.
  QImage                       m_image;         //!< icon to show in line edit.
  asiUI_Common::OpenSaveAction m_action; //!< open/save action.

  QRect                        m_buttonRect;    //!< icon rect
};

#pragma warning(default : 4251)

#endif
