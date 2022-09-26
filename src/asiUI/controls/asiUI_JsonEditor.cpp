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
#include <asiUI_JsonSearchThread.h>

// Qt includes
#pragma warning(push, 0)
#include <QResizeEvent>
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

  QColor backgroundColor()       { return QColor("#1E1E1E"); }

  //-----------------------------------------------------------------------------

  //QColor highlightColor()        { return QColor("#18465d"); }

  //-----------------------------------------------------------------------------

  QColor lineNumberColor()       { return QColor(128, 128, 128); }

  //-----------------------------------------------------------------------------

  //void printJsonBlocks(const asiUI_JsonBlocks& markers)
  //{
  //  std::cout << "blocks: " << markers.size() << std::endl;
  //  for (auto& pair : markers)
  //  {
  //    asiUI_JsonBlock block = pair.second;
  //    std::cout << block.m_isCollapsed << " start, end: "
  //              << block.m_blockNumber + 1 << ", "
  //              << block.m_blockNumberClose + 1 << ", "
  //              << block.m_positionOpen << ", "
  //              << block.m_positionClose
  //              << std::endl;
  //  }
  //}

  ////-----------------------------------------------------------------------------

  //void printBlockParents(const asiUI_ListOfListOfInt& blockParents)
  //{
  //  std::cout << "parents: " << blockParents.size() << std::endl;
  //  for (auto& pair : blockParents)
  //  {
  //    asiUI_ListOfInt parents = pair.second;

  //    std::cout << pair.first + 1 << ": ";
  //    for (auto& id : parents)
  //    {
  //      std::cout << id + 1 << " ";
  //    }
  //    std::cout << std::endl;
  //  }
  //}

  ////-----------------------------------------------------------------------------

  //void printCollapsed(const asiUI_JsonBlocks& collapsedBlocks)
  //{
  //  std::cout << "collapsed: " << collapsedBlocks.size() << std::endl;
  //  for (auto& item : collapsedBlocks)
  //  {
  //    std::cout << item.first << " ";
  //  }
  //  if (collapsedBlocks.size() > 0)
  //    std::cout << std::endl;
  //}

  //-----------------------------------------------------------------------------

  /*void printPositions(const asiUI_MapIntToInt& positions)
  {
    std::cout << "positions: " << positions.size() << std::endl;
    for (auto& pair : positions)
    {
       std::cout << pair.first + 1 << " : " << pair.second << std::endl;
    }
  }*/
}

//-----------------------------------------------------------------------------

//! Redefined layout to improve performance of collapse item in the editor.
class asiUI_Layout : public QPlainTextDocumentLayout
{
public:
  asiUI_Layout(QTextDocument *doc) : QPlainTextDocumentLayout(doc) {}
  ~asiUI_Layout() {}

public:
  QRectF blockBoundingRect(const QTextBlock &block) const
  {
    // improves performance on very long file as it's called in line number and marker repaint
    if (!block.isVisible())
      return QRectF();

    return QPlainTextDocumentLayout::blockBoundingRect(block);
  }
};

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
  void recompute(const asiUI_JsonBlocks& collapsedBlocks)
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

  //! Sets block information for the text block number
  //! \param[in] blockNumber index of block in text document
  //! \param[out] block      block information
  void setBlockRect(const int blockNumber, const asiUI_JsonBlock& block)
  {
    m_blockRects[blockNumber] = block;
  }

  //!< Returns block items.
  const asiUI_JsonBlocks blockRects() const { return m_blockRects; }

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

  void setBlockCollapsed(const int blockNumber, const bool toCollapse)
  {
    asiUI_JsonBlock blockR = m_blockRects.at(blockNumber);

    blockR.m_isCollapsed = toCollapse;
    m_blockRects[blockNumber] = blockR;

    QTextBlock textBlock = m_codeEditor->document()->findBlockByNumber(blockNumber);
    bool wasBlocked = m_codeEditor->editBlocked(true);
    m_codeEditor->changeTextToCollapse(textBlock, blockR, blockR.m_isCollapsed);
    m_codeEditor->editBlocked(wasBlocked);

    updateCollapseToVisibility(blockNumber);

    m_codeEditor->emulateAdjustScrollbars();
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
      setBlockCollapsed(blockNumber, !blockR.m_isCollapsed);
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
  m_searchStarted(false),
  m_immediateValidate(true),
  m_editBlocked(false)
{
  document()->setDocumentLayout(new asiUI_Layout(document()));

  m_highlighter = new asiUI_JsonHighlighter(this);
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

  setCurrentCharFormat(fmt);

  m_searchThread = new asiUI_JsonSearchThread();
  connect(m_searchThread, SIGNAL(finished()), this, SLOT(searchFinished()));
}

//-----------------------------------------------------------------------------

asiUI_JsonEditor::~asiUI_JsonEditor()
{
  m_highlighter = nullptr;
  m_lineNumberArea = nullptr;
  m_lineMarkerArea = nullptr;

  delete m_searchThread;
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
  return fontMetrics().height() + 2 * iconMargin();
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
      changeTextToCollapse(clonnedDoc->findBlockByNumber(blockR.m_blockNumber), blockR, false);
    }
  }

  QJsonDocument loadDoc(QJsonDocument::fromJson(clonnedDoc->toPlainText().toUtf8()));
  m_highlighter->setUnderlined(loadDoc.isNull());
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::expandAllBlocks()
{
  bool wasBlocked = editBlocked(true);
  auto markerArea = lineMarkerArea();
  for (auto& blockRect : markerArea->blockRects())
  {
    asiUI_JsonBlock blockR = blockRect.second;
    if (!blockR.m_isCollapsed)
      continue;
    markerArea->setBlockCollapsed(blockR.m_blockNumber, false);
  }
  editBlocked(wasBlocked);

  repaint();
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::emulateAdjustScrollbars()
{
  // to update length of the vertical scroll bar, it should be text changed or
  // document set or resize event. The implementation is QPlainTextEditPrivate::_q_adjustScrollbars
  // This update also is required when we collapse/expand blocks.
  // So, this method emulated resize event to adjust size.
  QResizeEvent event(QSize(size().width() + 10, size().height()), size());
  resizeEvent(&event);
  repaint();
}

//-----------------------------------------------------------------------------

QTextCursor findNext(const QTextCursor&                           currentCursor,
                     const std::list<asiUI_JsonHighlighterBlock>& indices,
                     const QString&                               value)
{
  if (indices.empty())
    return QTextCursor();

  int currentBlockPos = currentCursor.block().position();
  int currentCursorPos = currentCursor.position();

  for (auto& element : indices)
  {
    int elementBlockPos = element.TextBlock.position();
    int elementStartPos = element.StartPosition;

    if (elementBlockPos < currentBlockPos)
      continue;
    else if (elementBlockPos == currentBlockPos)
    {
      if (elementStartPos > currentCursorPos)
      {
        QTextCursor cursor(element.TextBlock);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                            elementStartPos - elementBlockPos);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                            value.length());
        return cursor;
      }
    }
    else // next block
    {
      int blockPos = element.TextBlock.position();
      QTextCursor cursor(element.TextBlock);
      cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                          elementStartPos - blockPos);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                          value.length());
      return cursor;
    }
  }

  auto& element = indices.front();
  int blockPos = element.TextBlock.position();
  int elementStartPos = element.StartPosition;
  QTextCursor cursor(element.TextBlock);
  cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
  cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                      elementStartPos - blockPos);
  cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                      value.length());
  return cursor;
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::searchEntered()
{
  if (m_highlighter->highlighted().size() == 0)
  {
    expandAllBlocks();
    selectNextFound();
  }
  else
  {
    // indices are already found and highlighted, set selected the next one
    expandAllBlocks();
    QTextCursor cursorOfSearch = findNext(textCursor(), m_highlighter->highlighted(), m_searchValue);
    setTextCursor(cursorOfSearch);
    if (!cursorOfSearch.isNull())
    {
      ensureCursorVisible();
    }
  }
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::searchChanged(const QString& value)
{
  m_searchValue = value.toLower();
  expandAllBlocks();

  // unhighlight previous value
  m_highlighter->setHighlighted(std::list<asiUI_JsonHighlighterBlock>());

  // select next found fragment
  selectNextFound();

  // search/highlight all matched values
  if (m_searchValue.isEmpty())
  {
    stopSearch();
  }
  else if (!m_searchStarted)
  {
    startSearch();
  }
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::searchDeactivated()
{
  m_searchValue = "";

  stopSearch();
}

//-----------------------------------------------------------------------------
QString asiUI_JsonEditor::collapseText()
{
  return " ... ";
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

  asiUI_JsonLineMarkerArea* markerArea = lineMarkerArea();
  if (markerArea->isEmpty())
  {
    // initialize
    asiUI_JsonBlocks collapsedBlocks;
    markerArea->recompute(collapsedBlocks);

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

  int blockCount = doc->blockCount();
  int blockCountPrev = markerArea->numberOfBlocks();

  asiUI_JsonBlocks collapsedBlocks;
  // collapsed before edited item
  for (int i = 0; i < editedBlockNumber; i++)
  {
    asiUI_JsonBlock block;
    if (!markerArea->blockRect(i, block))
      continue;
    if (block.m_isCollapsed)
      collapsedBlocks[i] = block;
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
      collapsedBlocks[i + deltaOfCount] = block;
  }
  lineMarkerArea()->recompute(collapsedBlocks);

  if (m_immediateValidate)
  {
    m_editBlocked = true;
    updateValidity();
    m_editBlocked = false;
  }

  stopSearch();
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::searchFinished()
{
  std::list<QTextCursor> indices = m_searchThread->matchedIndices();

  std::list<asiUI_JsonHighlighterBlock> matchedIndices;
  for (auto& cursor : indices)
  {
    QTextBlock textBlock = document()->findBlock(cursor.position());
    int startId = cursor.selectionStart();
    int endId = cursor.selectionEnd();

    matchedIndices.push_back(asiUI_JsonHighlighterBlock(startId, endId,textBlock));
  }

  m_highlighter->setHighlighted(matchedIndices);
  m_searchStarted = false;

  if (!m_searchValue.isEmpty() &&
      m_searchValue != m_searchThread->searchValue())
  {
    startSearch();
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

bool findUnions(const int               blockNumber,
                const QTextDocument*    document,
                const int               startInBlock,
                const asiUI_JsonBlocks& collapsedBlocks,
                asiUI_ListOfInt&        listOfParents,
                asiUI_ListOfListOfInt&  blockParents,
                asiUI_JsonBlock&        block)
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
    block.m_isBrace = isBrace;
    block.m_blockNumberClose = blockNumber;

    block.m_isCollapsed = collapsedBlocks.find(blockNumber) != collapsedBlocks.end();
    if (!block.m_isCollapsed)
    {
      block.m_positionOpen = indexOfStart;
      block.m_positionClose = indexOfEnd;
      block.m_collapsedInRow = blockText.mid(indexOfStart + 1, indexOfEnd - (indexOfStart + 1));
    }
    else
    {
      asiUI_JsonBlock collapsedBlock = collapsedBlocks.at(blockNumber);
      block.m_positionOpen = collapsedBlock.m_positionOpen;
      block.m_positionClose = collapsedBlock.m_positionClose;
      block.m_collapsedInRow = collapsedBlock.m_collapsedInRow;
    }

    // opened and closed in one block. Collapse shown only when there's some symbol between start and end.
    return indexOfEnd - indexOfStart > 1;
  }

  int endId = 0;
  listOfParents.push_back(blockNumber);
  int blockNumberClose;
  findEndInBlocks(blockNumber + 1, document, isBrace, -1, listOfParents, blockParents, endId, blockNumberClose);
  block.m_isBrace = isBrace;
  block.m_isCollapsed = collapsedBlocks.find(blockNumber) != collapsedBlocks.end();
  block.m_blockNumberClose = blockNumberClose;
  block.m_positionOpen = indexOfStart;
  block.m_positionClose = indexOfEnd;

  return blockNumberClose > blockNumber;
}

//-----------------------------------------------------------------------------

bool asiUI_JsonEditor::editBlocked(const bool value)
{
  bool wasBlocked = m_editBlocked;
  m_editBlocked = value;
  return wasBlocked;
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::calculateMarkers(const asiUI_JsonBlocks& collapsedBlocks,
                                        asiUI_JsonBlocks&       markers,
                                        asiUI_ListOfListOfInt&  blockParents,
                                        asiUI_MapIntToInt&      blockPositions) const
{
  markers.clear();
  QTextBlock block = document()->firstBlock();
  int blockNumber = block.blockNumber();

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
      if (findUnions(blockNumber, document(), -1, collapsedBlocks, listOfParents, blockParents, rect))
      {
        markers[blockNumber] = rect;
      }
    }
    blockPositions[blockNumber] = block.position();
    block = block.next();
    ++blockNumber;
  }
  //printCollapsed(collapsedBlocks);
  //printJsonBlocks(markers);
  //printBlockParents(blockParents);
  //printPositions(blockPositions);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::changeTextToCollapse(const QTextBlock& textBlock,
                                            const asiUI_JsonBlock& block,
                                            const bool toCollapse)
{
  QTextCursor cursor = QTextCursor(textBlock);

  int textLength = textBlock.text().length();
  if (!toCollapse)
    textLength -= collapseText().length();
  if (block.m_positionOpen == textLength - 1)
  {
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    if (toCollapse)
    {
      cursor.insertText(collapseText());
    }
    else // to expand
    {
      cursor.clearSelection();
      cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveMode::KeepAnchor, collapseText().length());
      cursor.removeSelectedText();
    }
  }
  else
  {
    if (toCollapse)
    {
      cursor.clearSelection();

      int startPos = block.m_positionOpen  + 1;
      int stopPos  = block.m_positionClose;

      cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, startPos);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, stopPos - startPos);
      cursor.removeSelectedText();

      cursor.insertText(collapseText());
    }
    else // to expand
    {
      cursor.clearSelection();

      int startPos = block.m_positionOpen  + 1;

      cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, startPos);

      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveMode::KeepAnchor, collapseText().length());
      cursor.removeSelectedText();

      cursor.insertText(block.m_collapsedInRow);
    }
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
  bool blockVisible = lineMarkerArea()->isBlockVisible(blockNumber);
  while (block.isValid() && top <= event->rect().bottom())
  {
    if (bottom >= event->rect().top())
    {
      if (blockVisible && markers.find(blockNumber) != markers.end())
      {
        asiUI_JsonBlock markerRect = markers.at(blockNumber);

        int height = fontMetrics().height();
        int width = height;
        QImage img(!markerRect.m_isCollapsed ? ":icons/asitus/arrow_down.svg" : ":icons/asitus/arrow_right.svg");
        QRect rect = QRect(iconMargin(), top, width, height);
        if (!img.isNull())
        {
          painter.drawImage(rect, img);
        }
        else
        {
          painter.drawPixmap(rect, style()->standardIcon(
            !markerRect.m_isCollapsed ? QStyle::SP_ArrowDown : QStyle::SP_ArrowRight).pixmap(width, height));
        }
        if (recalculateRect)
          rects[blockNumber] = rect;
      }
    }

    ++blockNumber;
    block = document()->findBlockByNumber(blockNumber);
    blockVisible = lineMarkerArea()->isBlockVisible(blockNumber);
    if (blockVisible)
    {
      top = bottom;
      bottom = top + qRound(blockBoundingRect(block).height());
    }
  }

  painter.setPen(lineNumberColor());
  painter.drawLine(evRect.right() - 1, evRect.top(), evRect.right() - 1, 5000/*infinite*/);
}

//-----------------------------------------------------------------------------

void asiUI_JsonEditor::zoomText(bool positive)
{
  if (positive)
    zoomIn();
  else
  {
    // do not perform scroll out when new size is less than minimum possible value.
    float newSize = font().pointSizeF() - 1/*range*/;
    if (newSize < 1)
      return;
    zoomOut();
  }
  ensureCursorVisible();
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::startSearch()
{
  m_searchThread->setSearchValue(m_searchValue);
  m_searchThread->setDocument(document()->clone());
  m_searchThread->start();
  m_searchStarted = true;
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::stopSearch()
{
  if (m_searchStarted)
  {
    m_searchThread->terminate();
    m_searchStarted = false;
  }
  m_highlighter->setHighlighted(std::list<asiUI_JsonHighlighterBlock>());
}

//-----------------------------------------------------------------------------
void asiUI_JsonEditor::selectNextFound()
{
  QTextCursor cursor = textCursor();
  int pos = cursor.anchor();
  cursor.clearSelection();
  cursor.setPosition(pos);
  QTextCursor cursorOfSearch = document()->find(m_searchValue, cursor);
  setTextCursor(cursorOfSearch);
  if (!cursorOfSearch.isNull())
  {
    ensureCursorVisible();
  }
}

//-----------------------------------------------------------------------------

asiUI_JsonLineMarkerArea* asiUI_JsonEditor::lineMarkerArea() const
{
  return (asiUI_JsonLineMarkerArea*)m_lineMarkerArea;
}
