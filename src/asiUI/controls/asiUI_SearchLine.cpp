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

// Own include
#include <asiUI_SearchLine.h>

// Qt includes
#pragma warning(push, 0)
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#pragma warning(pop)

//-----------------------------------------------------------------------------

asiUI_SearchLine::asiUI_SearchLine(QWidget* parent)
: QLineEdit(parent)
{
  connect (this, SIGNAL(returnPressed()),             this, SIGNAL(searchActivated()));
  connect (this, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

  reset();
}

//-----------------------------------------------------------------------------

asiUI_SearchLine::~asiUI_SearchLine()
{}

//-----------------------------------------------------------------------------
void asiUI_SearchLine::reset()
{
  setText(QString());
  setPlaceholderText("Type command");
}

//-----------------------------------------------------------------------------

void asiUI_SearchLine::paintEvent(QPaintEvent* event)
{
  QLineEdit::paintEvent(event);

  QPainter p(this);

  QStyleOptionFrame option;
  initStyleOption(&option);
  QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);

  int height = r.height();
  int marginOfLine = 5; /*defined in style.qss*/
  int iconSize = 20; /*size of the icon*/

  int centerOfLine = 0.5 * (height + 2 * marginOfLine);
  int centerOfIcon = 0.5 * iconSize;

  int margin = (centerOfLine - centerOfIcon) - marginOfLine;

  QImage img(":icons/asitus/search.svg");
  img = img.scaled(iconSize, iconSize);
  QPoint pos(r.right() - iconSize - margin, r.top() + margin);
  p.drawImage(pos, img);
}

//-----------------------------------------------------------------------------

void asiUI_SearchLine::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
  {
    reset();
    emit searchDeactivated();
    return;
  }
  QLineEdit::keyPressEvent(event);
}

//-----------------------------------------------------------------------------

void asiUI_SearchLine::onTextChanged(const QString& text)
{
  emit searchChanged();
  if (text.isEmpty())
  {
    emit searchDeactivated();
  }
}
