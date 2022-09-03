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
#include <asiUI_JsonEditor.h>
#include <asiUI_JsonHighlighter.h>

// Qt includes
#pragma warning(push, 0)
#include <QJsonDocument>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QWheelEvent>
#pragma warning(pop)

// STL includes
#include <map>

namespace
{
  //-----------------------------------------------------------------------------

  int iconMargin()               { return 4; }

  //-----------------------------------------------------------------------------

  int iconSize()                 { return 16; }

  //-----------------------------------------------------------------------------

  QColor backgroundColor()       { return QColor("#1E1E1E"); }

  //-----------------------------------------------------------------------------

  QColor highlightColor()        { return QColor("#18465d"); }

  //-----------------------------------------------------------------------------

  QColor lineNumberColor()       { return QColor(128, 128, 128); }

  //-----------------------------------------------------------------------------

  QColor invalidUnderlineColor() { return QColor(255, 0, 0); }

  //-----------------------------------------------------------------------------

  void printJsonBlocks(const asiUI_JsonBlocks& markers)
  {
    std::cout << "blocks: " << markers.size() << std::endl;
    for (auto& pair : markers)
    {
      asiUI_JsonBlock block = pair.second;
      std::cout << block.m_isCollapsed << " start, end: "
                << block.m_blockNumber + 1 << ", "
                << block.m_blockNumberClose + 1 << std::endl;
    }
  }

  //-----------------------------------------------------------------------------

  void printBlockParents(const asiUI_ListOfListOfInt& blockParents)
  {
    std::cout << "parents: " << blockParents.size() << std::endl;
    for (auto& pair : blockParents)
    {
      asiUI_ListOfInt parents = pair.second;

      std::cout << pair.first + 1 << ": ";
      for (auto& id : parents)
      {
        std::cout << id + 1 << " ";
      }
      std::cout << std::endl;
    }
  }

  //-----------------------------------------------------------------------------

  void printCollapsed(const std::set<int>& collapsedBlocks)
  {
    std::cout << "collapsed: " << collapsedBlocks.size() << std::endl;
    for (auto& pair : collapsedBlocks)
    {
      std::cout << pair << " ";
    }
    if (collapsedBlocks.size() > 0)
      std::cout << std::endl;
  }

  //-----------------------------------------------------------------------------

  void printPositions(const asiUI_MapIntToInt& positions)
  {
    std::cout << "positions: " << positions.size() << std::endl;
    for (auto& pair : positions)
    {
       std::cout << pair.first + 1 << " : " << pair.second << std::endl;
    }
  }
}

//-----------------------------------------------------------------------------

//! Widget to paint line number of the plain text.
class asiUI_JsonLineNumberArea : public QWidget
{
public:
  //! Constructor.
  //! \param[in] editor the source text editor
  asiUI_JsonLineNumberArea(asiUI_JsonEditor* editor)
   : QWidget(editor), m_codeEditor(editor) {}

  //! Returns recommended size for the widget.
  QSize sizeHint() const override
  {
    return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
  }

protected:
  //! Paints line numbers.
  //! \param[in] event paint event
  void paintEvent(QPaintEvent* event) override
  {
    m_codeEditor->paintLineNumberArea(event);
  }

private:
  asiUI_JsonEditor* m_codeEditor; //!< source text editor
};

//-----------------------------------------------------------------------------

//! Widget to paint collapse/expand markers for the rows of the plain editor text.
class asiUI_JsonLineMarkerArea : public QWidget
{
public:
  //! Constructor.
  asiUI_JsonLineMarkerArea(asiUI_JsonEditor* editor)
  : QWidget(editor), m_codeEditor(editor) {}

  //! Destructor.
  ~asiUI_JsonLineMarkerArea()
  {
    m_blockRects.clear();
    m_Rects.clear();
    m_blockParents.clear();
    m_blockPositions.clear();
  }

  //! Returns recommended size for the widget.
  QSize sizeHint() const override
  {
    return QSize(m_codeEditor->lineMarkerAreaWidth(), 0);
  }

  //! Returns true if the area is not filled yet or the text is empty.
  bool isEmpty()
  {
    return m_blockPositions.empty();
  }

  //! Recomputes markers.
  //! \param[in] collapsedBlocks container of collapsed block numbers
  void recompute(const std::set<int>& collapsedBlocks)
  {
    if (m_codeEditor->isEditBlocked())
      return;

    m_codeEditor->calculateMarkers(collapsedBlocks, m_blockRects, m_blockParents, m_blockPositions);
  }

  //! Returns true if the block is visible
  //! \param[in] blockNumber index of block in text document
  bool isBlockVisible(const int blockNumber)
  {
    if (m_blockParents.find(blockNumber) == m_blockParents.end())
      return true;

    for (auto& id : m_blockParents[blockNumber])
    {
      if (m_blockRects[id].m_isCollapsed)
        return false;
    }
    return true;
  }

  //! Returns block information by text block number
  //! \param[in] blockNumber index of block in text document
  //! \param[out] block      block information
  //! \return true if the block is found by the index
  bool blockRect(const int blockNumber, asiUI_JsonBlock& block) const
  {
    if (m_blockRects.find(blockNumber) == m_blockRects.end())
      return false;

    block = m_blockRects.at(blockNumber);
    return true;
  }

  //! Returns number of blocks of the document.
  int numberOfBlocks() const { return (int)m_blockPositions.size(); }

  //! Returns block position.
  //! \param[in] blockNumber index of block in text document
  //! \param[in] position    the index of the block's first character within the document.
  //! \return true if the position is found by the index
  bool blockPosition(const int blockNumber, int& position)
  {
    if (m_blockPositions.find(blockNumber) == m_blockPositions.end())
      return false;

    position = m_blockPositions.at(blockNumber);
    return true;
  }

protected:
  //! Paints markers for the event rect.
  //! \param[in] event paint event
  void paintEvent(QPaintEvent* event) override
  {
    m_codeEditor->paintMarkerAreaRects(event, m_blockRects, m_Rects);
  }

  //! Processes mouse press and double click to expand/collapse clicked marker if it's clicked.
  //! \param[in] event an event
  bool event(QEvent* event) override
  {
    if (event->type() == QEvent::MouseButtonPress
     || event->type() == QEvent::MouseButtonDblClick)
    {
      auto mouseEvent = dynamic_cast<QMouseEvent*>(event);

      QRect blockRect;
      int blockNumber = -1;
      for (auto& pair : m_Rects)
      {
        blockRect = pair.second;
        if (!blockRect.contains(mouseEvent->pos()))
          continue;
        blockNumber = pair.first;
      }
      if (blockNumber < 0)
        return false;

      asiUI_JsonBlock blockR = m_blockRects.at(blockNumber);

      blockR.m_isCollapsed = !blockR.m_isCollapsed;
      m_blockRects[blockNumber] = blockR;

      QTextBlock textBlock = m_codeEditor->document()->findBlockByNumber(blockNumber);
      bool wasBlocked = m_codeEditor->editBlocked(true);
      m_codeEditor->changeTextToCollapse(textBlock, blockR.m_isCollapsed);
      m_codeEditor->editBlocked(wasBlocked);

      updateCollapseToVisibility(blockNumber);
      // update underline to calculate scroll bar correctly
      m_codeEditor->updateJsonUnderline();

      return true;
    }
    return QWidget::event(event);
  }

  //! Updates visibility state of the blocks for the given block between open and close block indices.
  //! \param[in] blockNumber index of block in text document
  void updateCollapseToVisibility(const int blockNumber)
  {
    if (m_blockRects.find(blockNumber) == m_blockRects.end())
      return;

    const QTextDocument* doc = m_codeEditor->document();
    asiUI_JsonBlock blockRect = m_blockRects.at(blockNumber);
    for (int i = blockNumber + 1; i < blockRect.m_blockNumberClose; i++)
    {
      doc->findBlockByNumber(i).setVisible(isBlockVisible(i));
    }
  }

private:
  asiUI_JsonEditor*     m_codeEditor;     //!< source text editor.
  asiUI_JsonBlocks      m_blockRects;     //!< contains block number to json block item. Only for collapsible.
  asiUI_MapIntToRect    m_Rects;          //!< contains block number to rect. It's only for visible text.
  asiUI_ListOfListOfInt m_blockParents;   //!< contains block number to list of block number parents.
  asiUI_MapIntToInt     m_blockPositions; //!< the index of the block's first character within the document.
};

//-----------------------------------------------------------------------------

asiUI_JsonEditor::asiUI_JsonEditor(QWidget* parent)
: QPlainTextEdit(parent),
  m_documentValid(true),
  m_immediateValidate(true),
  m_editBlocked(false)
{
  m_highlighter = new asiUI_JsonHighlighter(document());
  m_lineNumberArea = new asiUI_JsonLineNumberArea(this);
  m_lineMarkerArea = new asiUI_JsonLineMarkerArea(this);

  connect(this,       SIGNAL(blockCountChanged(int)),
          this,       SLOT(updateLineNumberAreaWidth()));
  connect(this,       SIGNAL(updateRequest(const QRect&, int)),
          this,       SLOT(updateLineNumberArea(const QRect&, int)));
  connect(document(), SIGNAL(contentsChange(int, int, int)),
          this,       SLOT(updateOnContentsChange(int, int, int)));

  document()->setUndoRedoEnabled(false);

  updateLineNumberAreaWidth();

  QTextCharFormat fmt = currentCharFormat();
  fmt.setFontFixedPitch(true);
  fmt.setFontStyleHint(QFont::Monospace);
  fmt.setFontFamily("Consolas");
  updateJsonUnderline();

  setCurrentCharFormat(fmt);
}

//-----------------------------------------------------------------------------

asiUI_JsonEditor::~asiUI_JsonEditor()
{
  m_highlighter = nullptr;
  m_lineNumberArea = nullptr;
  m_lineMarkerArea = nullptr;
}

//-----------------------------------------------------------------------------

int asiUI_JsonEditor::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10)
  {
    max /= 10;
    ++digits;
  }

  return 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

//-----------------------------------------------------------------------------

int asiUI_JsonEditor::lineMarkerAreaWidth()
{
  return iconSize() + 2 * iconMargin();
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::updateLineNumberAreaWidth()
{
  setViewportMargins(lineNumberAreaWidth() + lineMarkerAreaWidth(), 0, 0, 0);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::updateLineNumberArea(const QRect &rect, int dy)
{
  if (dy)
  {
    m_lineNumberArea->scroll(0, dy);
    m_lineMarkerArea->scroll(0, dy);
  }
  else
  {
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    m_lineMarkerArea->update(0, rect.y(), m_lineMarkerArea->width(), rect.height());
  }
  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth();
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::updateValidity()
{
  QTextDocument* clonnedDoc = document()->clone();
  for (int i = 0; i < clonnedDoc->blockCount(); i++)
  {
    asiUI_JsonBlock blockR;
    if (!((asiUI_JsonLineMarkerArea*)m_lineMarkerArea)->blockRect(i, blockR))
      continue;

    if (blockR.m_isCollapsed)
    {
      changeTextToCollapse(clonnedDoc->findBlockByNumber(blockR.m_blockNumber), false);
    }
  }

  QJsonDocument loadDoc(QJsonDocument::fromJson(clonnedDoc->toPlainText().toUtf8()));
  m_documentValid = !loadDoc.isNull();

  updateJsonUnderline();
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->modifiers() == Qt::ControlModifier &&
      (event->key() == Qt::Key_Up ||
       event->key() == Qt::Key_Down))
  {
    zoomText(event->key() == Qt::Key_Up);
  }
  else
    QPlainTextEdit::keyPressEvent(event);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::resizeEvent(QResizeEvent *e)
{
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left(),
                                      cr.top(),
                                      lineNumberAreaWidth(),
                                      cr.height()));
  m_lineMarkerArea->setGeometry(QRect(cr.left() + lineNumberAreaWidth(),
                                      cr.top(),
                                      lineMarkerAreaWidth(),
                                      cr.height()));
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::wheelEvent(QWheelEvent *event)
{
  if (event->modifiers() == Qt::ControlModifier)
    zoomText(event->angleDelta().y() > 0);
  else
    QPlainTextEdit::wheelEvent(event);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::updateOnContentsChange(int position,
                                              int /*charsRemoved*/,
                                              int /*charsAdded*/)
{
  if (m_editBlocked)
    return;

  asiUI_JsonLineMarkerArea* markerArea = (asiUI_JsonLineMarkerArea*)m_lineMarkerArea;
  if (markerArea->isEmpty())
  {
    std::set<int> collapsedBlocks;
    ((asiUI_JsonLineMarkerArea*)m_lineMarkerArea)->recompute(collapsedBlocks);

    m_editBlocked = true;
    updateValidity();
    m_editBlocked = false;
    return;
  }

  int editedBlockNumber = -1;
  QTextDocument* doc = document();
   for (int i = 0; i < doc->blockCount(); i++)
  {
    int currentPosition;
    if (!markerArea->blockPosition(i, currentPosition))
    {
      editedBlockNumber = markerArea->numberOfBlocks() + 1; // new added block
      break; // changed document has more blocks that before this change
    }
    if (currentPosition > position)
    {
      editedBlockNumber = i - 1;
      break;
    }
  }
  if (editedBlockNumber <= 0)
    return;

  asiUI_JsonLineMarkerArea* lineMarkerArea = ((asiUI_JsonLineMarkerArea*)m_lineMarkerArea);

  int blockCount = doc->blockCount();
  int blockCountPrev = lineMarkerArea->numberOfBlocks();

  std::set<int> collapsedBlocks;
  // collapsed before edited item
  for (int i = 0; i < editedBlockNumber; i++)
  {
    asiUI_JsonBlock block;
    if (!markerArea->blockRect(i, block))
      continue;
    if (block.m_isCollapsed)
      collapsedBlocks.insert(i);
  }
  // collapsed after edited item
  int deltaOfCount = blockCount - blockCountPrev;
  int deltaOfStart = blockCount > blockCountPrev ? 0 : -deltaOfCount;
  for (int i = editedBlockNumber + deltaOfStart; i < blockCountPrev; i++)
  {
    asiUI_JsonBlock block;
    if (!markerArea->blockRect(i, block))
      continue;
    if (block.m_isCollapsed)
      collapsedBlocks.insert(i + deltaOfCount);
  }
  ((asiUI_JsonLineMarkerArea*)m_lineMarkerArea)->recompute(collapsedBlocks);

  if (m_immediateValidate)
  {
    m_editBlocked = true;
    updateValidity();
    m_editBlocked = false;
  }
}

//-----------------------------------------------------------------------------

int findStart(const QString& blockText,
              const int      indexStart,
              const bool     fromEnd,
              bool&          isBrace)
{
  if (indexStart >= blockText.length())
    return -1;

  int indexOfStart1 = fromEnd ? blockText.lastIndexOf("{", indexStart)
                              : blockText.indexOf("{", indexStart);
  int indexOfStart2 = fromEnd ? blockText.lastIndexOf("[", indexStart)
                              : blockText.indexOf("[", indexStart);

  if (indexOfStart1 == -1 && indexOfStart2 == -1)
    return -1;

  isBrace = indexOfStart1 > indexOfStart2;
  return isBrace ? indexOfStart1 : indexOfStart2;
}

//-----------------------------------------------------------------------------

int findEnd(const QString& blockText,
            const int      indexStart,
            const bool&    isBrace)
{
  if (indexStart >= blockText.length())
    return -1;

  int indexOfEnd = isBrace ? blockText.indexOf("}", indexStart)
                           : blockText.indexOf("]", indexStart);
  return indexOfEnd;
}

//-----------------------------------------------------------------------------

void findEndInBlocks(const int              startBlockNumber,
                     const QTextDocument*   document,
                     bool                   isBrace,
                     const int              indexOfStart,
                     asiUI_ListOfInt&       listOfParents,
                     asiUI_ListOfListOfInt& blockParents,
                     int&                   endId,
                     int&                   endBlockId)
{
  QTextBlock block = document->findBlockByNumber(startBlockNumber);;
  if (!block.isValid())
    return;

  int blockNumber = block.blockNumber();
  int indexOfStartSearch = indexOfStart + 1;
  while (block.isValid())
  {
    QString blockText = block.text();
    int filledBlockNumber = -1;
    if (blockParents.find(blockNumber) == blockParents.end())
    {
      filledBlockNumber = blockNumber;
      blockParents[blockNumber] = listOfParents;
    }
    bool isBraceNext;
    int indexOfStartNext = findStart(blockText, indexOfStartSearch, false, isBraceNext);
    if (indexOfStartNext > -1) // start in this row
    {
      int endIdNext = 0;
      int endBlockIdNext = 0;

      listOfParents.push_back(blockNumber);
      findEndInBlocks(blockNumber, document, isBraceNext, indexOfStartNext,
                      listOfParents, blockParents, endIdNext, endBlockIdNext);
      listOfParents.pop_back();

      if (endBlockIdNext >= 0) // closing of the block is found
      {
        for (int i = blockNumber; i < endBlockIdNext; i++)
        {
          block = block.next();
        }
        blockNumber = endBlockIdNext;
        indexOfStartSearch = endIdNext + 1;
        block = document->findBlockByNumber(blockNumber);
        blockText = block.text();
      }
    }
    int indexOfEnd = findEnd(blockText, indexOfStartSearch, isBrace);
    {
      if (indexOfEnd > -1)
      {
        if (filledBlockNumber == blockNumber)
         blockParents[blockNumber].pop_back();

        endId = indexOfEnd;
        endBlockId = blockNumber;
        return;
      }
    }
    block = block.next();
    ++blockNumber;
    indexOfStartSearch = 0;
  }
  endId = -1;
  endBlockId = -1;
}

//-----------------------------------------------------------------------------

bool findUnions(const int              blockNumber,
                const QTextDocument*   document,
                const int              startInBlock,
                asiUI_ListOfInt&       listOfParents,
                asiUI_ListOfListOfInt& blockParents,
                bool&                  isUnionBrace,
                int&                   blockNumberEnd)
{
  QTextBlock startblock = document->findBlockByNumber(blockNumber);
  QString blockText = startblock.text();

  bool isBrace;
  int indexOfStart = 0;

  indexOfStart = findStart(blockText, startInBlock, true, isBrace);
  if (indexOfStart == -1) // no start in this row
  {
    return false;
  }

  int indexOfEnd = findEnd(blockText, indexOfStart, isBrace);
  if (indexOfEnd > indexOfStart)
  {
    return false; // opened and closed
  }

  int endId = 0;
  listOfParents.push_back(blockNumber);
  findEndInBlocks(blockNumber + 1, document, isUnionBrace, -1, listOfParents, blockParents, endId, blockNumberEnd);

  return blockNumberEnd > blockNumber;
}

//-----------------------------------------------------------------------------

bool asiUI_JsonEditor::editBlocked(const bool value)
{
  bool wasBlocked = m_editBlocked;
  m_editBlocked = value;
  return wasBlocked;
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::calculateMarkers(const std::set<int>&   collapsedBlocks,
                                        asiUI_JsonBlocks&      markers,
                                        asiUI_ListOfListOfInt& blockParents,
                                        asiUI_MapIntToInt&     blockPositions) const
{
  markers.clear();
  QTextBlock block = document()->firstBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  blockParents.clear();
  blockPositions.clear();
  
  asiUI_ListOfInt listOfParents;
  while (block.isValid())
  {
    if (block.text().contains("{") || block.text().contains("["))
    {
      asiUI_JsonBlock rect;
      rect.m_blockNumber = blockNumber;
      listOfParents.clear();
      bool isBrace;
      int blockNumberClose;
      if (findUnions(blockNumber, document(), -1, listOfParents, blockParents, isBrace, blockNumberClose) &&
          rect.m_blockNumber != rect.m_blockNumberClose)
      {
        rect.m_isBrace = isBrace;
        rect.m_blockNumberClose = blockNumberClose;
        rect.m_isCollapsed = false;
        if (collapsedBlocks.find(blockNumber) != collapsedBlocks.end())
          rect.m_isCollapsed = true;
        markers[blockNumber] = rect;
      }
    }
    blockPositions[blockNumber] = block.position();
    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
  //printCollapsed(collapsedBlocks);
  //printJsonBlocks(markers);
  //printBlockParents(blockParents);
  //printPositions(blockPositions);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::changeTextToCollapse(const QTextBlock& textBlock,
                                            const bool toCollapse)
{
  QTextCursor cursor = QTextCursor(textBlock);
  cursor.movePosition(QTextCursor::EndOfBlock);
  if (toCollapse)
  {
    QTextCharFormat charFormat;
    charFormat.setForeground(lineNumberColor());
    cursor.insertText(" ...", charFormat);
  }
  else // to expand
  {
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);

    cursor.removeSelectedText();
  }
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::paintLineNumberArea(QPaintEvent *event)
{
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), backgroundColor());

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom())
  {
    if (block.isVisible() && bottom >= event->rect().top())
    {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(lineNumberColor());
      QFont fnt = font();
      fnt.setFixedPitch(true);
      fnt.setStyleHint(QFont::Monospace);
      fnt.setFamily("Consolas");
      painter.setFont(fnt);
      painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }
    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::paintMarkerAreaRects(QPaintEvent*            event,
                                            const asiUI_JsonBlocks& markers,
                                            asiUI_MapIntToRect&     rects)
{
  QPainter painter(m_lineMarkerArea);
  painter.fillRect(event->rect(), backgroundColor());

  QRect evRect = event->rect();

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  bool recalculateRect = event->rect().top() == 0; // do not update rects when cursor is painted only
  if (recalculateRect)
    rects.clear();
  while (block.isValid() && top <= event->rect().bottom())
  {
    bool blockVisible = ((asiUI_JsonLineMarkerArea*)m_lineMarkerArea)->isBlockVisible(blockNumber);
    if (bottom >= event->rect().top())
    {
      if (blockVisible && markers.find(blockNumber) != markers.end())
      {
        asiUI_JsonBlock markerRect = markers.at(blockNumber);

        int width = iconSize();
        int height = iconSize();
        QImage img(!markerRect.m_isCollapsed ? ":icons/asitus/arrow_down.svg" : ":icons/asitus/arrow_right.svg");
        int span = iconMargin();
        QRect rect = QRect(span, top, width, height);
        painter.drawImage(rect, img);

        if (recalculateRect)
          rects[blockNumber] = rect;
      }
    }

    block = document()->findBlockByNumber(blockNumber);
    if (blockVisible)
    {
      top = bottom;
      bottom = top + qRound(blockBoundingRect(block).height());
    }
    ++blockNumber;
  }

  painter.setPen(lineNumberColor());
  painter.drawLine(evRect.right() - 1, evRect.top(), evRect.right() - 1, 5000/*infinite*/);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::updateJsonUnderline()
{
  bool editBlockedPrev = m_editBlocked;
  m_editBlocked = true;

  QTextCursor cursor(document());
  cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  QTextCharFormat fmt = currentCharFormat();
  if (m_documentValid)
  {
    fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);
  }
  else
  {
    fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    fmt.setUnderlineColor(invalidUnderlineColor());
  }
  cursor.setCharFormat(fmt);

  QTextDocument* doc = document();
  for (int i = 0; i < doc->blockCount(); i++)
  {
    asiUI_JsonBlock blockR;
    if (!((asiUI_JsonLineMarkerArea*)m_lineMarkerArea)->blockRect(i, blockR))
      continue;

    if (blockR.m_isCollapsed)
    {
      QTextBlock textBlock = doc->findBlockByNumber(i);
      cursor = QTextCursor(textBlock);
      cursor.movePosition(QTextCursor::EndOfBlock);
      {
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor);

        fmt = QTextCharFormat();
        fmt.setForeground(lineNumberColor());
        fmt.setFontFixedPitch(false);
        fmt.setFontStyleHint(QFont::Times);
        cursor.setCharFormat(fmt);
      }
    }
  }
  m_editBlocked = editBlockedPrev;
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::zoomText(bool positive)
{
  if (positive)
    zoomIn();
  else
    zoomOut();
}
