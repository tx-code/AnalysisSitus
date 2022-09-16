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

#ifndef asiUI_JsonHighlighter_h
#define asiUI_JsonHighlighter_h

// asiUI includes
#include <asiUI_CommonFacilities.h>

// Qt includes
#pragma warning(push, 0)
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#pragma warning(pop)

// STL includes
#include <list>

class asiUI_JsonEditor;
class QTextDocument;

//! Element defines the position in highlighter.
class asiUI_EXPORT asiUI_JsonHighlighterBlock
{
public:
  //! Constructor
  //! \param[in] startPosition place to start highlight
  //! \param[in] stopPosition  place to start highlight
  //! \param[in] textBlock     place to start highlight
  asiUI_JsonHighlighterBlock(const int         startPosition,
                             const int         stopPosition,
                             const QTextBlock& textBlock);

  int        StartPosition; //!< start highlight
  int        StopPosition;  //!< stop highlight
  QTextBlock TextBlock;     //!< text block of highlight
};

//! Syntax highlighter for text editor following Json rules.
class asiUI_JsonHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  enum FormatType
  {
    FormatType_Doc,       //!< underlined format of document
    FormatType_Key,       //!< words before : (key)
    FormatType_Value,     //!< words after : (value)
    FormatType_Digit,     //!< digits
    FormatType_Keyword,   //!< keywords of Json as 'true', 'false', 'null'
    FormatType_Collapse,  //!< collapsed format for '...'
    FormatType_Highlight, //!< format for values of search
  };


public:

  //! Constructor.
  //! \param[in] editor text editor widget.
  asiUI_JsonHighlighter(asiUI_JsonEditor* editor);

  //! Destructor.
  virtual ~asiUI_JsonHighlighter();

  //! Sets state that whole text should be underlined
  //! \param [in] value state to underline
  void setUnderlined(const bool value,
                     const bool toUpdate = true);

  //! Returns highlighted positions
  const std::list<asiUI_JsonHighlighterBlock>& highlighted() const
  { return m_highlighted; }

  //! Sets cursor positions to highlight block of it.
  void setHighlighted(const std::list<asiUI_JsonHighlighterBlock>& values);

  //! Highlights the given text block.
  //! \param[in] text a separate text block of the text editor document
  virtual void highlightBlock(const QString& text);

protected:
  //! Sets format for parameter text.
  //! \param[in] text source value to highlight
  //! \param[in] formatType a key of the collection of formats
  void highlightBlockForType(const QString&   text,
                             const FormatType formatType);

protected:
  asiUI_JsonEditor*                        m_editor;         //!< source editor
  bool                                     m_underlined;     //!< flag whether the text is underlined or not
  std::list<asiUI_JsonHighlighterBlock>    m_highlighted;    //!< positions to be highlighted
  std::map<FormatType, QTextCharFormat>    m_typeFormat;     //!< text formats to highlight
  std::map<FormatType, QRegularExpression> m_typeExpression; //!< expression for format
};

#endif
