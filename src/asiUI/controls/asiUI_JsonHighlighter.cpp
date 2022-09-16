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
#include <asiUI_JsonEditor.h>

// Qt includes
#pragma warning(push, 0)
#include <QTextDocument>
#pragma warning(pop)

// STL includes
#include <set>

namespace
{
  //-----------------------------------------------------------------------------

  QColor keyColor()   { return QColor("#9cdcfe"); }

  //-----------------------------------------------------------------------------

  QColor valueColor() { return QColor("#ce9178"); }

  //-----------------------------------------------------------------------------

  QColor digitColor() { return QColor("#b5cea8"); }

  //-----------------------------------------------------------------------------

  QColor keywordColor() { return QColor(53, 140, 214); }

  //-----------------------------------------------------------------------------

  QColor collapseColor() { return QColor(128, 128, 128); }

  //-----------------------------------------------------------------------------

  QColor highlightColor() { return QColor(24, 70, 93); }

  //-----------------------------------------------------------------------------

  QColor invalidUnderlineColor() { return QColor(255, 0, 0); }
}

//-----------------------------------------------------------------------------
asiUI_JsonHighlighterBlock::asiUI_JsonHighlighterBlock(const int         startPosition,
                                                       const int         stopPosition,
                                                       const QTextBlock& textBlock)
 : StartPosition(startPosition), StopPosition(stopPosition), TextBlock(textBlock)
{
}

//-----------------------------------------------------------------------------

asiUI_JsonHighlighter::asiUI_JsonHighlighter(asiUI_JsonEditor* editor)
: QSyntaxHighlighter(editor->document()),
  m_editor(editor),
  m_underlined(false)
{
  m_typeFormat[FormatType_Doc] = QTextCharFormat();

  m_typeFormat[FormatType_Key] = QTextCharFormat();
  m_typeFormat[FormatType_Key].setFontWeight(QFont::Bold);
  m_typeFormat[FormatType_Key].setForeground(keyColor());

  m_typeFormat[FormatType_Value] = QTextCharFormat();
  m_typeFormat[FormatType_Value].setFontWeight(QFont::Bold);
  m_typeFormat[FormatType_Value].setForeground(valueColor());

  m_typeFormat[FormatType_Digit] = QTextCharFormat();
  m_typeFormat[FormatType_Digit].setFontWeight(QFont::Bold);
  m_typeFormat[FormatType_Digit].setForeground(digitColor());

  m_typeFormat[FormatType_Keyword] = QTextCharFormat();
  m_typeFormat[FormatType_Keyword].setFontWeight(QFont::Bold);
  m_typeFormat[FormatType_Keyword].setForeground(keywordColor());

  m_typeFormat[FormatType_Collapse] = QTextCharFormat();
  m_typeFormat[FormatType_Collapse].setFontFixedPitch(false);
  m_typeFormat[FormatType_Collapse].setFontStyleHint(QFont::Times);
  m_typeFormat[FormatType_Collapse].setForeground(collapseColor());

  m_typeFormat[FormatType_Highlight] = QTextCharFormat();
  m_typeFormat[FormatType_Highlight].setBackground(highlightColor());

  m_typeExpression[FormatType_Key]       = QRegularExpression("\"\\w+\":");
  m_typeExpression[FormatType_Value]     = QRegularExpression("\"[^\"]*\"");
  m_typeExpression[FormatType_Digit]     = QRegularExpression("-?[0-9]*(.?[0-9]+(e[+-]?[0-9]+)?)");
  m_typeExpression[FormatType_Keyword]   = QRegularExpression("true|false|null");
  m_typeExpression[FormatType_Collapse]  = QRegularExpression(asiUI_JsonEditor::collapseText());
  m_typeExpression[FormatType_Highlight] = QRegularExpression(); // not necessary as defined by cursor
}

//-----------------------------------------------------------------------------

asiUI_JsonHighlighter::~asiUI_JsonHighlighter()
{
  m_highlighted.clear();
}

//-----------------------------------------------------------------------------

void asiUI_JsonHighlighter::setUnderlined(const bool value,
                                          const bool toUpdate)
{
  m_underlined = value;

  for (auto& pair : m_typeFormat)
  {
    if (m_underlined)
    {
      m_typeFormat[pair.first].setUnderlineStyle(QTextCharFormat::WaveUnderline);
      m_typeFormat[pair.first].setUnderlineColor(invalidUnderlineColor());
    }
    else
    {
      m_typeFormat[pair.first].setUnderlineStyle(QTextCharFormat::NoUnderline);
    }
  }

  if (toUpdate)
    rehighlight();
}

//-----------------------------------------------------------------------------
void asiUI_JsonHighlighter::setHighlighted(const std::list<asiUI_JsonHighlighterBlock>& values)
{
  std::set<QTextBlock> blockToHighlight;
  // rehighlight previuos blocks
  for (auto& element : m_highlighted)
  {
    if (blockToHighlight.find(element.TextBlock) != blockToHighlight.end())
      continue;
    blockToHighlight.insert(element.TextBlock);
  }

  m_highlighted = values;

  // highlight new blocks
  for (auto& element : m_highlighted)
  {
    if (blockToHighlight.find(element.TextBlock) != blockToHighlight.end())
      continue;
    blockToHighlight.insert(element.TextBlock);
  }

  rehighlight();
}

//-----------------------------------------------------------------------------
void asiUI_JsonHighlighter::highlightBlockForType(const QString& text,
                                                  const FormatType formatType)
{
  QRegularExpressionMatchIterator iter = m_typeExpression[formatType].globalMatch(text);

  while (iter.hasNext())
  {
    QRegularExpressionMatch match = iter.next();
    setFormat(match.capturedStart(), match.capturedLength(), m_typeFormat[formatType]);
  }
}

//-----------------------------------------------------------------------------

void asiUI_JsonHighlighter::highlightBlock(const QString& text)
{
  setFormat(0, text.length(), m_typeFormat[FormatType_Doc]);

  highlightBlockForType(text, FormatType_Collapse);
  highlightBlockForType(text, FormatType_Keyword);
  highlightBlockForType(text, FormatType_Digit);
  highlightBlockForType(text, FormatType_Value);
  highlightBlockForType(text, FormatType_Key);

  if (!m_highlighted.empty())
  {
    QTextBlock curBlock = currentBlock();
    for (auto& element : m_highlighted)
    {
      if (element.TextBlock != curBlock)
        continue;
      int startId = element.StartPosition - element.TextBlock.position();
      int endId = element.StopPosition - element.TextBlock.position();
      setFormat(startId,
                endId - startId,
                m_typeFormat[FormatType_Highlight]);
    }
  }
}
