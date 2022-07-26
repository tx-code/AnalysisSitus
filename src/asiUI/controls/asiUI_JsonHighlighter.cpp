//-----------------------------------------------------------------------------
// Created on: 16 June 2022
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
#include <asiUI_JsonHighlighter.h>

// Qt includes
#pragma warning(push, 0)
#include <QTextDocument>
#pragma warning(pop)

namespace
{
  //-----------------------------------------------------------------------------

  QColor keyColor()   { return QColor("#9cdcfe"); }

  //-----------------------------------------------------------------------------

  QColor valueColor() { return QColor("#ce9178"); }

  //-----------------------------------------------------------------------------

  QColor digitColor() { return QColor("#b5cea8"); }
}

//-----------------------------------------------------------------------------

asiUI_JsonHighlighter::asiUI_JsonHighlighter(QTextDocument* editor)
: QSyntaxHighlighter(editor)
{
}

//-----------------------------------------------------------------------------

asiUI_JsonHighlighter::~asiUI_JsonHighlighter()
{
}

//-----------------------------------------------------------------------------

void asiUI_JsonHighlighter::highlightBlock(const QString& text)
{
  QTextCharFormat classFormat; // for words before : (key)
  classFormat.setFontWeight(QFont::Bold);
  classFormat.setForeground(keyColor());

  QTextCharFormat classFormat2; // for words after : (value)
  classFormat2.setFontWeight(QFont::Bold);
  classFormat2.setForeground(valueColor());

  QTextCharFormat classFormat3; // for digits
  classFormat3.setFontWeight(QFont::Bold);
  classFormat3.setForeground(digitColor());

  QRegularExpression expression("\"\\w+\":"); // words before : (key)
  QRegularExpression expression2("\"[^\"]*\""); // words after : (value)
  QRegularExpression expression3("-?[0-9]+(.?[0-9]+)"); // digits
  QRegularExpressionMatchIterator iter = expression.globalMatch(text);
  QRegularExpressionMatchIterator iter2 = expression2.globalMatch(text);
  QRegularExpressionMatchIterator iter3 = expression3.globalMatch(text);
  while (iter3.hasNext())
  { // digits
    QRegularExpressionMatch match = iter3.next();
    setFormat(match.capturedStart(), match.capturedLength(), classFormat3);
  }
  while (iter2.hasNext())
  { // words after : (value)
    QRegularExpressionMatch match = iter2.next();
    setFormat(match.capturedStart(), match.capturedLength(), classFormat2);
  }
  while (iter.hasNext())
  { // words before : (key)
    QRegularExpressionMatch match = iter.next();
    setFormat(match.capturedStart(), match.capturedLength(), classFormat);
  }
}
