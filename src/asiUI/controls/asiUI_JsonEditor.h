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

#ifndef asiUI_JsonEditor_h
#define asiUI_JsonEditor_h

// asiUI includes
#include <asiUI_CommonFacilities.h>
#include <asiUI_JsonBlock.h>

// Qt includes
#pragma warning(push, 0)
#include <QPlainTextEdit>
#pragma warning(pop)

#include <set>

class asiUI_JsonLineNumberArea;
class asiUI_JsonLineMarkerArea;
class asiUI_JsonHighlighter;

class QKeyEvent;
class QPaintEvent;
class QWheelEvent;

//! The class is the plain text editor extended with:
//! - column of line numbers,
//! - column with expand/collapse buttons on brace and bracket symbols.
//! - text highlight with color for json format,
//! It has json format valdiating. It validates the whole document.
//! If it's not valid, the whole text is underlined with red waved line.
class asiUI_EXPORT asiUI_JsonEditor : public QPlainTextEdit
{
  Q_OBJECT

public:

  //! Constructor.
  //! \param[in] parent parent widget (if any).
  asiUI_JsonEditor(QWidget* parent = nullptr);

  //! Destructor.
  virtual ~asiUI_JsonEditor();

  //! Sets whether the Json validity should be checked by any text change.
  //! If it's false, to check just call updateValidity.
  //! \param[in] value check validity state.
  void setImmediateValidate(const bool value) { m_immediateValidate = value; }

  //! Checks whether the text is valid in terms of Json format.
  //! Underlines the text with red waved line if not valid.
  void updateValidity();

protected:
  //! Changes flag about blocking changes processing.
  //! \param[in] value check validity state.
  //! \return previous value
  bool editBlocked(const bool value);

  //! Returns true if the changes processing is blocked.
  bool isEditBlocked() const { return m_editBlocked; }

  //! Returns width for the line number column. It calculates number of digits.
  int lineNumberAreaWidth();

  //! Returns width for the marker area column.
  int lineMarkerAreaWidth();

  //! Underlines the text with red waved line if not valid.
  void updateJsonUnderline();

  //! Fills containers of values by the current document.
  //! \param[in]  collapsedBlocks container of collapsed block numbers
  //! \param[out] markers         container of collapsable blocks
  //! \param[out] blockParents    container of block hierarchy
  //! \param[out] blockPositions  container of each block position
  void calculateMarkers(const std::set<int>&   collapsedBlocks,
                        asiUI_JsonBlocks&      markers,
                        asiUI_ListOfListOfInt& blockParents,
                        asiUI_MapIntToInt&     blockPositions) const;

  //! Changes the editor text to move text block into collapsed or expanded state.
  //! Appends ' ...' if collapsed, remove it if it's expanded.
  //! \param[in] textBlock  block to process
  //! \param[in] toCollapse state whether the block becomes collapsed or expanded
  void changeTextToCollapse(const QTextBlock& textBlock,
                            const bool        toCollapse);

  //! Paints line numbers depending on the event rect.
  //! \param[in] event paint event
  void paintLineNumberArea(QPaintEvent* event);

  //! Paints markers area depending on the event rect.
  //! \param[in] event   paint event
  //! \param[in] markers information about collapsed blocks
  //! \param[out] rects  painter marker rectangles to check later which one is clicked
  void paintMarkerAreaRects(QPaintEvent*            event,
                            const asiUI_JsonBlocks& markers,
                            asiUI_MapIntToRect&     rects);

protected:
  //! Scales text if event has Up or Down key with Ctrl modifier.
  //! \param[in] event key info
  void keyPressEvent(QKeyEvent *event);

  //! Updates geometry of line number and marker areas.
  //! \param[in] event resize info
  void resizeEvent(QResizeEvent* event) override;

  //! Scales text if event has Ctrl modifier.
  //! \param[in] event wheel info
  void wheelEvent(QWheelEvent *event) override;

private slots:
  //! Updates viewport margins by line number and marker areas width.
  void updateLineNumberAreaWidth();

  //! Updates width of the line number and marker areas.
  //! \param[in] rect area in view port that need to be updated
  //! \param[in] dy   number of scrolled pixels
  void updateLineNumberArea(const QRect& rect,
                            int          dy);

  //! Updates markers area depending on the document change.
  //! \param[in] position     the character in the document where the change occurred
  //! \param[in] charsRemoved the number of characters removed
  //! \param[in] charsAdded   the number of characters added
  void updateOnContentsChange(int position,
                              int charsRemoved,
                              int charsAdded);

  //! Zoom text font in editor.
  //! \param[in] positive flag whether to increase text.
  void zoomText(bool positive);

private:
  asiUI_JsonHighlighter* m_highlighter;       //!< class to highlight Json forfmat
  QWidget*               m_lineNumberArea;    //!< control to paint line numbers
  QWidget*               m_lineMarkerArea;    //!< control to paint collapse/expand markers

  bool                   m_documentValid;     //!< flag whether the text is valid as a Json document
  bool                   m_immediateValidate; //!< flag whether the validation is performed by text edit
  bool                   m_editBlocked;       //!< flag whether editing is processed

  friend asiUI_JsonLineNumberArea;
  friend asiUI_JsonLineMarkerArea;
};

#endif
