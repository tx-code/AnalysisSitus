//-----------------------------------------------------------------------------
// Created on: 23 August 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiUI_Console.h>

// asiUI includes
#include <asiUI_Common.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OCCT includes
#include <Standard_Version.hxx>
#include <TCollection_AsciiString.hxx>

// Tcl includes
#include <tcl.h>

// Qt includes
#pragma warning(push, 0)
#include <QAbstractItemView>
#include <QDesktopWidget>
#include <QLabel>
#include <QScrollBar>
#include <QTextBlock>
#pragma warning(pop)

namespace
{
  //-----------------------------------------------------------------------------

  QString findCommandArguments(const std::vector<asiTcl_CommandInfo>& commands,
                               const QString&                         curCommand)
  {
    if (curCommand.isEmpty())
      return "";

    for ( size_t commandIter = 0; commandIter < commands.size(); commandIter++ )
    {
      QString command(commands[commandIter].Name.c_str());
      if (command != curCommand)
        continue;

      QString arguments = asiUI_Console::commandArguments(commands[commandIter]);
      return arguments.trimmed(); // remove spaces in the start and end of the help text
    }
    return "";
  }
}

//-----------------------------------------------------------------------------

static QString READY_PROMPT = "> ";
static QString DOTS_PROMPT  = "... ";

//-----------------------------------------------------------------------------

asiUI_Console::asiUI_Console(const Handle(asiTcl_Interp)& interp,
                             QWidget*                     parent)
//
: asiUI_StyledTextEdit (parent),
  m_interp             (interp),
  m_pCompleter         (nullptr),
  m_descriptionShown   (false),
  m_description        (nullptr)
{
  this->setUndoRedoEnabled ( false );
  this->setLineWrapMode    ( QTextEdit::WidgetWidth );
  this->setWordWrapMode    ( QTextOption::WrapAnywhere );
  this->setAcceptRichText  ( false );
  this->setReadOnly        ( false );
  this->setUndoRedoEnabled ( true );
  this->setFont            ( QFont("monospace", 9) );
  //
  this->viewport()->unsetCursor(); // Unset busy cursor.

  /* ========================
   *  Prepare auto-completer.
   * ======================== */

  // Collect commands.
  std::vector<asiTcl_CommandInfo> commands;
  m_interp->GetAvailableCommands(commands);

  // Gather all command names for sorting.
  QStringList commandNames;
  //
  for ( int c = 0; c < (int) commands.size(); ++c )
  {
    QString cmd = CStr2QStr(commands[c].Name.c_str());
    m_CmdToDescription[cmd] = findCommandArguments(commands, cmd);
    commandNames << cmd;
  }

  // Sorts the list of strings in ascending order.
  commandNames.sort();

  // Construct and initialize the completer.
  m_pCompleter = new QCompleter(commandNames, this);
  m_pCompleter->setWidget(this);
  m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  m_pCompleter->setMaxVisibleItems(15);
  m_pCompleter->popup()->installEventFilter(this);
  //
  QObject::connect(m_pCompleter, QOverload<const QString&>::of(&QCompleter::activated),
                   this,         &asiUI_Console::insertCompletion);

  QObject::connect(m_pCompleter, SIGNAL(highlighted(const QString&)),
                   this,         SLOT(completerHighlighted()));

  /* =========================
   *  Add initialization text.
   * ========================= */

  this->addText( QString("OpenCascade version: %1.%2.%3\n")
                 .arg(OCC_VERSION_MAJOR)
                 .arg(OCC_VERSION_MINOR)
                 .arg(OCC_VERSION_MAINTENANCE), false, false );

  this->addText( QString("Tcl version: %1 [%2.%3.%4.%5]\n"
                         "Type 'show-commands' to display available commands.\n***")
                 .arg(TCL_VERSION)
                 .arg(TCL_MAJOR_VERSION)
                 .arg(TCL_MINOR_VERSION)
                 .arg(TCL_RELEASE_LEVEL)
                 .arg(TCL_RELEASE_SERIAL), false, false );

  // Add prompt
  m_prompt = READY_PROMPT;
  this->addText(READY_PROMPT, true, false);
}

//-----------------------------------------------------------------------------

QCompleter* asiUI_Console::completer() const
{
  return m_pCompleter;
}

//-----------------------------------------------------------------------------

QSize asiUI_Console::sizeHint() const
{
  QDesktopWidget desktop;
  const int side   = std::min( desktop.height(), desktop.width() );
  const int width  = (int) (side*0.25);
  const int height = (int) (side*0.1);

  QSize s(width, height);
  return s;
}

//-----------------------------------------------------------------------------

void asiUI_Console::addCommand(QString command)
{
  textCursor().insertText(command);
}

//-----------------------------------------------------------------------------

QString asiUI_Console::commandArguments(const asiTcl_CommandInfo& commandTcl)
{
  QString command = commandTcl.Name.c_str();
  QString help = commandTcl.Help.c_str();
  if (help.indexOf(command) == -1)
  {
    return "";
  }

  QString arguments = help;
  int lastIndexOfCommandName = arguments.indexOf(command);
  if (lastIndexOfCommandName >= 0)
    arguments = arguments.mid(lastIndexOfCommandName + command.length(), arguments.length());

  int indexOnNewLine = arguments.indexOf('\n');
  if (indexOnNewLine >= 0)
    arguments = arguments.mid(0, indexOnNewLine);

  return arguments;
}


//-----------------------------------------------------------------------------

bool asiUI_Console::eventFilter( QObject* o, QEvent* e )
{
  if ( o == m_pCompleter->popup() && e->type() == QEvent::Hide )
  {
    m_pCompleter->popup()->setCurrentIndex( QModelIndex() );
    hideCommandDescription();
  }
  return asiUI_StyledTextEdit::eventFilter( o, e );
}

//-----------------------------------------------------------------------------

void asiUI_Console::keyPressEvent(QKeyEvent* e)
{
  QTextCursor c = this->textCursor();

  if ( m_pCompleter && m_pCompleter->popup()->isVisible() )
  {
    // The following keys are forwarded by the completer to the widget
    switch ( e->key() )
    {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        e->ignore();
        return; // let the completer do default behavior
      default:
        break;
    }
  }

  /* =========================================
   *  Executing commands, cursor moving logic.
   * ========================================= */

  bool isProcessed = false;
  switch ( e->key() )
  {
    case Qt::Key_Up:
    {
      if ( e->modifiers() == Qt::ControlModifier )
        this->zoomIn();
      else
        asiUI_StyledTextEdit::keyPressEvent(e);

      isProcessed = true;
      break;
    }
    case Qt::Key_Down:
    {
      if ( e->modifiers() == Qt::ControlModifier )
        this->zoomOut();
      else
        asiUI_StyledTextEdit::keyPressEvent(e);

      isProcessed = true;
      break;
    }
    case Qt::Key_Return:
    case Qt::Key_Enter:
    {
      // <SHIFT> modifier allows insertion of blank row instead of command execution.
      bool doInsertBlankRow = false;
      if ( e->modifiers() == Qt::ShiftModifier )
        doInsertBlankRow = true;

      if ( doInsertBlankRow )
      {
        // Add next block with a new prompt
        c.insertBlock();
        c.insertText( READY_PROMPT, this->currentCharFormat() );
      }
      else
      {
        TCollection_AsciiString cmdName = this->currentCommand(c);

        this->adoptSourceCmd(cmdName, cmdName);

        if ( !this->eval(cmdName) )
          m_interp->GetProgress().SendLogMessage(LogErr(Normal) << "\t %1 ... TCL_ERROR" << cmdName);
        else
          m_interp->GetProgress().SendLogMessage(LogNotice(Normal) << "\t %1 ... TCL_OK" << cmdName);

        // The following piece of code realizes "intelligent" movement of cursor.
        // The code checks whether next line is available by consulting block
        // number. If the block number if different after "Down" movement, then
        // it means that another line exists (if it does not, the cursor will not
        // move). If another line exists, we check the text at this line. If
        // the text is nothing but a prompt prefix ("> "), then we do not insert
        // new block, but simply let the cursor move to this prompt line
        // and reuse it so.

        const int bbefore = c.blockNumber();
        c.movePosition(QTextCursor::EndOfBlock);
        c.movePosition(QTextCursor::Down);;
        const int bafter = c.blockNumber();
        //
        if ( bbefore == bafter ) // No next block exists, so the cursor did not move
        {
          // To avoid breaking command words if <Enter> is pressed not in the
          // end of line
          c.movePosition(QTextCursor::End);
          this->setTextCursor(c);

          // Add next block with a new prompt
          this->addText(READY_PROMPT, true, false);
        }
        else
        {
          QString nextStr = c.block().text(); // Check text at the next line
          //
          if ( nextStr == READY_PROMPT ) // If that's not a prompt, then work as usually
            this->setTextCursor(c); // If that's new line is a prompt, reuse it
        }
      }

      isProcessed = true;
      break;
    }
    default:
      break;
  }

  /* =================================
   *  Auto-completion stuff goes then.
   * ================================= */

  const bool
    isShortcut = ( e->modifiers().testFlag(Qt::ControlModifier) && (e->key() == Qt::Key_Space) );

  // Do not process the shortcut when we have a completer.
  // Do not process the already processed key events.
  if ( !isShortcut && !isProcessed )
  {
    asiUI_StyledTextEdit::keyPressEvent(e);
  }

  const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
                           e->modifiers().testFlag(Qt::ShiftModifier);
  if ( ctrlOrShift && e->text().isEmpty() )
    return;

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;' []\\="); // end of word
  const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = this->wordUnderCursor();

  if ( !isShortcut && ( hasModifier ||
                        e->text().isEmpty() ||
                        completionPrefix.length() < 2 ||
                        eow.contains( e->text().right(1) ) ) )
  {
    m_pCompleter->popup()->hide();
    return;
  }

  if ( completionPrefix != m_pCompleter->completionPrefix() )
  {
    // store text of the selected item
    QString selCompletion;
    QModelIndexList selIndices = m_pCompleter->popup()->selectionModel()->selectedIndexes();
    if ( !selIndices.isEmpty() )
    {
      selCompletion = m_pCompleter->popup()->model()->data( selIndices.first() ).toString();
    }
    m_pCompleter->setCompletionPrefix(completionPrefix);

    if ( !selCompletion.isEmpty() )
    {
      // restore selection by stored selected text if the item exists
      QModelIndex selIndex;

      QAbstractItemModel* model = m_pCompleter->popup()->model();
      for ( int i = 0; i < model->rowCount(); i++ )
      {
        QModelIndex index = model->index( i, 0 );
        if ( model->data( index ).toString() != selCompletion )
          continue;
        selIndex = index;
        break;
      }
      if ( selIndex.isValid() )
        m_pCompleter->popup()->setCurrentIndex( selIndex );
    }
  }
  QRect cr = cursorRect();
  cr.setWidth(  m_pCompleter->popup()->sizeHintForColumn(0)
              + m_pCompleter->popup()->verticalScrollBar()->sizeHint().width() );
  m_pCompleter->complete(cr); // popup it up!
  completerHighlighted();
}

//-----------------------------------------------------------------------------

void asiUI_Console::addText(const QString& str,
                            const bool     newBlock,
                            const bool     isError)
{
  QTextCursor cursor = this->textCursor();
  QTextCharFormat cf = this->currentCharFormat();

  this->moveCursor( QTextCursor::End );
  if ( newBlock )
    cursor.insertBlock();
  if ( isError )
    cf.setForeground( QBrush( Qt::red ) );

  cursor.insertText( str, cf );
  this->moveCursor( QTextCursor::End );
  this->ensureCursorVisible();
}

//-----------------------------------------------------------------------------

bool asiUI_Console::eval(const TCollection_AsciiString& cmd)
{
  const bool isOk = ( m_interp->Eval(cmd) == TCL_OK );

  return isOk;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString asiUI_Console::currentCommand(const QTextCursor& cursor) const
{
  // Get text from text area
  QString cmd = cursor.block().text().trimmed();

  // Cut prompt prefix
  cmd = cmd.remove( 0, this->promptSize() );

  return QStr2AsciiStr(cmd);
}

//-----------------------------------------------------------------------------

QTextLine asiUI_Console::currentTextLine(const QTextCursor& cursor) const
{
  const QTextBlock block = cursor.block();
  if ( !block.isValid() )
    return QTextLine();

  const QTextLayout* layout = block.layout();
  if ( !layout )
    return QTextLine();

  const int relativePos = cursor.position() - block.position();
  return layout->lineForTextPosition(relativePos);
}

//-----------------------------------------------------------------------------

QString asiUI_Console::wordUnderCursor() const
{
  QString result;

  QTextCursor tc = this->textCursor();

  if (tc.positionInBlock() == 0)
  {
    // there is no a word under cursor when it's in the beginning on a new row
    return QString();
  }

  bool endOfWord = false;
  do
  {
    tc.movePosition(QTextCursor::PreviousCharacter);
    tc.select(QTextCursor::WordUnderCursor);
    QString selectedWord = tc.selectedText();
    result.prepend(selectedWord);

    if ( selectedWord == "-" )
    {
      tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, 2);
    }
    else
    {
      tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, 2);
      tc.select(QTextCursor::WordUnderCursor);
      QString prevWord = tc.selectedText();

      if ( prevWord != "-" )
      {
        endOfWord = true;
      }
      else
      {
        result.prepend("-");
        tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, 2);
      }
    }
  }
  while ( !endOfWord );

  return result;
}

//-----------------------------------------------------------------------------

bool asiUI_Console::adoptSourceCmd(const TCollection_AsciiString& cmd,
                                   TCollection_AsciiString&       adopted) const
{
  std::vector<std::string> argv;
  asiAlgo_Utils::Str::Split(cmd.ToCString(), " ", argv);

  // Change back slashes with forward slashes.
  if ( argv[0] == "source" )
  {
    if ( argv.size() == 2 )
    {
      std::replace(argv[1].begin(), argv[1].end(), '\\', '/');
      //
      adopted = TCollection_AsciiString( argv[0].c_str() )
              + " "
              + TCollection_AsciiString( argv[1].c_str() );

      return true;
    }
  }

  adopted = cmd; // Keep as-is.
  return false;
}

//-----------------------------------------------------------------------------

void asiUI_Console::insertCompletion(const QString& completion)
{
  QTextCursor cursor = this->textCursor();

  const int extra = completion.length() - m_pCompleter->completionPrefix().length();

  cursor.movePosition ( QTextCursor::Left );
  cursor.movePosition ( QTextCursor::EndOfWord );
  cursor.insertText   ( completion.right(extra) );
  //
  this->setTextCursor(cursor);
}

//-----------------------------------------------------------------------------

void asiUI_Console::completerHighlighted()
{
  QAbstractItemView* view = m_pCompleter->popup();
  QModelIndex curIndex = view->currentIndex();
  QString completion = curIndex.isValid() ? view->model()->data( curIndex ).toString() : QString();

  QString highlightedDescription = m_CmdToDescription.contains( completion )
                                 ? m_CmdToDescription[completion] : QString();
  QModelIndexList selIndices = view->selectionModel()->selectedIndexes();
  if ( highlightedDescription.isEmpty() || selIndices.isEmpty() )
  {
    if ( m_descriptionShown )
      hideCommandDescription();
    return;
  }

  showCommandDescription( highlightedDescription, view->visualRect( curIndex ) );
}

//-----------------------------------------------------------------------------

void asiUI_Console::showCommandDescription( const QString& description,
                                            const QRect&   selectedRect )
{
  QAbstractItemView* view = m_pCompleter->popup();

  m_descriptionShown = true;
  if ( !m_description )
  {
    m_description = new QLabel( view );
    m_description->setStyleSheet(
      QString::fromUtf8( "background-color: rgb(30, 30, 30); color: rgb(230, 230, 230); border: 1px solid #76797C;" ) );
    m_description->setWindowFlags( Qt::ToolTip );
  }
  m_description->setText( description );

  QFontMetrics fmetrics( m_description->font() );
  m_description->setFixedWidth( fmetrics.width( description ) + 2 * 2/*description margin*/ + 2 * 1 /*border width*/ );

  QStyleOptionSlider opt;
  QPoint rightTop = view->mapToGlobal( QPoint( selectedRect.right(), selectedRect.top() ) );

  int slider_thick = 0;
  bool isScrollBarVisible = m_pCompleter->completionCount() > m_pCompleter->maxVisibleItems();
  if ( isScrollBarVisible )
  {
    slider_thick = m_pCompleter->popup()->verticalScrollBar()->sizeHint().width();
  }

  int x = rightTop.x() + slider_thick + 2 * 2/*description margin*/;
  int y = rightTop.y();
  m_description->move( x, y );
  m_description->show();
}

//-----------------------------------------------------------------------------

void asiUI_Console::hideCommandDescription()
{
  if ( !m_description )
    return;
  m_description->hide();
  m_descriptionShown = false;
}
