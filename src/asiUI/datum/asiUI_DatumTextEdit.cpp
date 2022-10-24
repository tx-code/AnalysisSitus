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
#include <asiUI_DatumTextEdit.h>

// asiUI includes
#include <asiUI_Common.h>

// Qt includes
#pragma warning(push, 0)
#include <QVariant>
#include <QEvent>
#pragma warning(pop)

//-----------------------------------------------------------------------------

asiUI_DatumTextEdit::asiUI_DatumTextEdit(const QString& dicID,
                                         QWidget*       parent,
                                         const int      datumFlags)
: asiUI_Datum(parent)
{
  m_pEditor = new Editor( dicID, parent, convertFlags(datumFlags) );

  init(m_pEditor);
}

//-----------------------------------------------------------------------------

QDS_Datum* asiUI_DatumTextEdit::getDatum() const
{
  return m_pEditor;
}

//-----------------------------------------------------------------------------

//! Update unit system.
void asiUI_DatumTextEdit::onUnitsUpdated()
{
  m_pEditor->UpdateUnits();
}

//-----------------------------------------------------------------------------
// Editor control
//-----------------------------------------------------------------------------

//! Constructor. Initializes controls.
asiUI_DatumTextEdit::Editor::Editor(const QString& dicID,
                                    QWidget*       parent,
                                    const int      flags)
: QDS_Datum(dicID, parent, flags)
{}

//-----------------------------------------------------------------------------

//! Creates control widget
//! \param[in] parent parent widget for editor.
//! \return widget pointer.
QWidget* asiUI_DatumTextEdit::Editor::createControl(QWidget* parent)
{
  asiUI_StyledTextEdit* pEditor = new asiUI_StyledTextEdit(parent);

  pEditor->installEventFilter(this);

  return pEditor;
}

//-----------------------------------------------------------------------------

bool asiUI_DatumTextEdit::Editor::eventFilter(QObject* /*pObj*/,
                                              QEvent*  pEvent)
{
  if ( pEvent->type() == QEvent::KeyPress )
  {
    /*QDateTimeEdit* anEdit = dateTimeEdit();
    if ( anEdit && !anEdit->hasFocus() )
      anEdit->setFocus();*/
  }

  return false;
}

//! Returns the text editor control.
//! \return pointer to the text editor control.
asiUI_StyledTextEdit* asiUI_DatumTextEdit::Editor::textEdit() const
{
  return ::qobject_cast<asiUI_StyledTextEdit*>( controlWidget() );
}

//! Get string value from the text editor.
//! \return string value.
QString asiUI_DatumTextEdit::Editor::getString() const
{
  asiUI_StyledTextEdit* pEditor = this->textEdit();

  if ( !pEditor )
    return QString();

  return pEditor->toPlainText();
}

//! Set string value to the text editor.
//! \param[in] string the input value.
void asiUI_DatumTextEdit::Editor::setString(const QString& string)
{
  asiUI_StyledTextEdit* pEditor = this->textEdit();

  if ( !pEditor )
    return;

  pEditor->setText(string);
}

//! Sets text from QVariant value.
//! \param[in] value string as QVariant value.
void asiUI_DatumTextEdit::Editor::setValue(const QVariant& value)
{
  asiUI_StyledTextEdit* pEditor = this->textEdit();

  if ( !pEditor )
    return;

  pEditor->setText( value.toString() );
}

//! \return text as QVariant.
QVariant asiUI_DatumTextEdit::Editor::value() const
{
  asiUI_StyledTextEdit* pEditor = this->textEdit();

  if ( !pEditor )
    return QVariant();

  return QVariant( pEditor->toPlainText() );
}
