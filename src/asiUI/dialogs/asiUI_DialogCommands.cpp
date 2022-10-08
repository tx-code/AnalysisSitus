//-----------------------------------------------------------------------------
// Created on: 19 September 2017
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
#include <asiUI_DialogCommands.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_CommonFacilities.h>
#include <asiUI_Console.h>
#include <asiUI_TclPluginToCommands.h>
#include <asiUI_DialogCommandsRootItem.h>
#include <asiUI_SearchLine.h>
#include <asiUI_TreeModel.h>

// asiVisu includes
#include <asiVisu_PrsManager.h>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QHeaderView>
#include <QStyleOptionViewItem>
#pragma warning(pop)

#define BTN_MIN_WIDTH 120

namespace
{
  //! Customization of tree view for painting it in always active state, like it has the focus.
  //! The puprose is having selected item in highlighted color in the tree even if the tree has no focus.
  //! It looks clear with a combination of search control that should have focus for entering a value.
  class asiUI_QTreeView : public QTreeView
  {
  public:
    asiUI_QTreeView() : QTreeView() {};

    void drawRow(QPainter*                   painter,
                 const QStyleOptionViewItem& option,
                 const QModelIndex&          index) const
    {
      // change tree view option to be active.
      const QStyleOptionViewItem* viOption = qstyleoption_cast<const QStyleOptionViewItem*>(&option);
      QStyleOptionViewItem viOptionCopy(*viOption);
      viOptionCopy.state |= QStyle::State_Active;

      QTreeView::drawRow(painter, viOptionCopy, index);
    }
  };

  //-----------------------------------------------------------------------------

  enum CommandsColumnType
  {
    CommandsColumnType_Name = 0,   //! name column
    CommandsColumnType_Parameters, //! command line arguments
    CommandsColumnType_Description //! command description
  };

  //-----------------------------------------------------------------------------

  void uniteCommandsByPlugin(const std::vector<asiTcl_CommandInfo>&  commandsFrom,
                             std::vector<asiUI_TclPluginToCommands>& commandsTo)
  {
    int nbCommands = (int)commandsFrom.size();

    std::vector<int> indices;
    indices.reserve(nbCommands);

    std::vector<std::string> commandNames;
    commandNames.reserve(nbCommands);
    for (int c = 0; c < nbCommands; ++c)
    {
      asiTcl_CommandInfo info = commandsFrom[c];
      indices.push_back(c);
      commandNames.push_back(info.Name);
    }

    // Sort commands by names.
    std::sort(indices.begin(), indices.end(),
      [&](const int a, const int b)
      {
        return commandNames[a] < commandNames[b];
      });

    std::vector<asiTcl_CommandInfo> sortCommandsFrom;
    sortCommandsFrom.reserve(nbCommands);
    for (int indexIter = 0; indexIter < (int)indices.size(); indexIter++)
    {
      sortCommandsFrom.push_back(commandsFrom[indices[indexIter]]);
    }

    // collect commands for each plugin name
    std::map<std::string, std::vector<asiUI_TclCommandParsed> > commands;
    for (int c = 0; c < nbCommands; ++c)
    {
      asiTcl_CommandInfo info = sortCommandsFrom[c];
      QString curCommand = info.Name.c_str();
      QString curHelp = info.Help.c_str();

      QString arguments;
      QString description;
      if (curHelp.indexOf(curCommand) == -1)
      {
        arguments = QString();
        description = curHelp;
      }
      else
      {
        int positionAfter;
        arguments = asiUI_Console::commandArguments(info, positionAfter);

        description = curHelp;
        description = description.mid(positionAfter);//curCommand.length() + arguments.length());
        description = description.trimmed();
        description = description.replace('\t', ' ');
      }
      arguments = arguments.trimmed(); // remove spaces in the start and end of the help text

      asiUI_TclCommandParsed infoCommand(info.Name,
                                         arguments.toStdString(),
                                         description.toStdString(),
                                         info.Filename);

      std::vector<asiUI_TclCommandParsed> groupCommands;
      if (commands.find(info.Group) != commands.end())
        groupCommands = commands.at(info.Group);
      groupCommands.push_back(infoCommand);
      commands[info.Group] = groupCommands;
    }

    // move collected elements into output container
    commandsTo.clear();
    for (std::map<std::string, std::vector<asiUI_TclCommandParsed> >::const_iterator cit = commands.cbegin();
         cit != commands.cend(); cit++)
    {
      commandsTo.push_back(asiUI_TclPluginToCommands(cit->first, cit->second));
    }
  }

  //-----------------------------------------------------------------------------

  QModelIndex findNext(const QModelIndex& index, const QModelIndexList& indices)
  {
    if (indices.isEmpty())
      return index;

    if (indices.contains(index)) // if the item belongs to the list, return the next item
    {
      for (int i = 0; i < indices.length() - 1; i++)
      {
        if (indices[i] == index)
          return indices[i+1];
      }
      return indices.first();
    }
    return indices.first(); // if next item is not found, return the first index
  }

  //-----------------------------------------------------------------------------

  QModelIndex findPrev(const QModelIndex& index, const QModelIndexList& indices)
  {
    if (indices.isEmpty())
      return index;

    if (indices.contains(index)) // if the item belongs to the list, return the next item
    {
      for (int i = indices.length() - 1; i > 0; i--)
      {
        if (indices[i] == index)
          return indices[i-1];
      }
      return indices.last();
    }
    return indices.last(); // if next item is not found, return the first index
  }
}
//-----------------------------------------------------------------------------

//! Constructor.
//! \param interp   [in] Tcl interpreter.
//! \param notifier [in] progress notifier.
//! \param parent   [in] parent widget.
asiUI_DialogCommands::asiUI_DialogCommands(const Handle(asiTcl_Interp)& interp,
                                           ActAPI_ProgressEntry         notifier,
                                           QWidget*                     parent)
: QDialog    (parent),
  m_interp   (interp),
  m_notifier (notifier)
{
  // Window title
  setWindowTitle("Available commands");

  // Main layout
  m_pMainLayout = new QVBoxLayout();

  // Widgets
  m_widgets.pClose = new QPushButton("Close");
  m_widgets.pClose->setMaximumWidth(BTN_MIN_WIDTH);

  // Set search control
  m_widgets.pSearchLine = new asiUI_SearchLine("Type command");

  connect(m_widgets.pSearchLine, SIGNAL (returnPressed()),     this, SLOT(searchEntered()));
  connect(m_widgets.pSearchLine, SIGNAL (searchDeactivated()), this, SLOT(searchDeactivated()));
  connect(m_widgets.pSearchLine, SIGNAL (searchUp()),          this, SLOT(searchUp()));
  connect(m_widgets.pSearchLine, SIGNAL (searchDown()),        this, SLOT(searchDown()));
  connect(m_widgets.pSearchLine, SIGNAL (searchChanged(const QString&)),
          this,                  SLOT   (searchChanged(const QString&)));

  m_pMainLayout->addWidget(m_widgets.pSearchLine);

  // Configure tree view
  m_widgets.pCommandsView = new asiUI_QTreeView();
  m_widgets.pCommandsView->installEventFilter(this);
  //
  m_pMainLayout->addWidget(m_widgets.pCommandsView);
  asiUI_TreeModel* model = new asiUI_TreeModel();
  m_widgets.pCommandsView->setModel(model);
  connect(m_widgets.pCommandsView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClickedTableView(QModelIndex)));

  //
  m_pMainLayout->setAlignment(Qt::AlignTop);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);
  //
  this->setLayout(m_pMainLayout);

  // Initialize table
  this->initialize();

  // Set good initial size
  this->setMinimumSize( QSize(650, 600) );

  m_pMainLayout->addWidget(m_widgets.pClose);

  // Connect signals to slots
  connect(m_widgets.pClose, SIGNAL(clicked()), SLOT(onClose()));

  m_widgets.pClose->setAutoDefault(false);
  m_widgets.pSearchLine->setFocus();
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::setConsole(asiUI_Console* console)
{
  m_console = console;
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::doubleClickedTableView(QModelIndex index)
{
  asiUI_TreeItem* item = (asiUI_TreeItem*)index.internalPointer();
  QString command = item->data(QModelIndex((index.row(), 0, index.parent()))).toString();
  m_console->addCommand(command);
}

//-----------------------------------------------------------------------------

//! Destructor.
asiUI_DialogCommands::~asiUI_DialogCommands()
{
  delete m_pMainLayout;
  m_widgets.Release();
}

//-----------------------------------------------------------------------------

//! Initializes table of commands.
void asiUI_DialogCommands::initialize()
{
  // Collect variables.
  std::vector<asiTcl_CommandInfo> commands;
  m_interp->GetAvailableCommands(commands);

  std::vector<asiUI_TclPluginToCommands> commandsTo;
  uniteCommandsByPlugin(commands, commandsTo);

  asiUI_TreeModel* model = dynamic_cast<asiUI_TreeModel*>(m_widgets.pCommandsView->model());

  asiUI_DialogCommandsRootItemPtr item;
  item = asiUI_DialogCommandsRootItem::CreateItem(asiUI_TreeItemPtr(), 0, CommandsColumnType_Name);
  model->setRootItem(CommandsColumnType_Name, "Name", item);
  item->setValues(commandsTo);

  item = asiUI_DialogCommandsRootItem::CreateItem(asiUI_TreeItemPtr(), 0, CommandsColumnType_Parameters);
  model->setRootItem(CommandsColumnType_Parameters, "Parameters", item);

  item = asiUI_DialogCommandsRootItem::CreateItem(asiUI_TreeItemPtr(), 0, CommandsColumnType_Description);
  model->setRootItem(CommandsColumnType_Description, "Description", item);

  model->emitLayoutChanged();

  m_widgets.pCommandsView->header()->setDefaultSectionSize(200);
  m_widgets.pCommandsView->header()->setStretchLastSection(true);

  m_widgets.pCommandsView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  // to wrap text in the description column.
  m_widgets.pCommandsView->setWordWrap(true);
}

//-----------------------------------------------------------------------------

bool asiUI_DialogCommands::eventFilter( QObject* o, QEvent* e )
{
  if (o == m_widgets.pCommandsView && e->type() == QEvent::KeyPress)
  {
    QKeyEvent* aKeyEvent = static_cast<QKeyEvent*>(e);
    switch (aKeyEvent->key())
    {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      {
        selectMatchedIndex(true);
        return true;
      }
      default:
        break;
    }
  }
  return QDialog::eventFilter(o, e);
}

//-----------------------------------------------------------------------------

void findMatchedIndices(const QString matchedValue,
                        const asiUI_TreeModel* model,
                        QModelIndexList& matchedIndices)
{
  QString value = ".*" + matchedValue + ".*";
  matchedIndices = model->match(model->index(0,0),
                                Qt::DisplayRole,
                                value,
                                -1,
                                Qt::MatchRegExp | Qt::MatchWrap | Qt::MatchRecursive);
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::searchEntered()
{
  selectMatchedIndex(true);
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::selectMatchedIndex(const bool nextMatched)
{
  if (m_matchedIndices.length() > 0)
  {
    auto selectionModel = m_widgets.pCommandsView->selectionModel();
    auto selectedIndices = selectionModel->selectedIndexes();
    auto toSelectId = m_matchedIndices[0];
    if (!selectedIndices.isEmpty())
    {
      if (nextMatched)
        toSelectId = findNext(selectedIndices[0], m_matchedIndices);
      else
        toSelectId = findPrev(selectedIndices[0], m_matchedIndices);
    }
    selectionModel->select(toSelectId, QItemSelectionModel::SelectionFlag::Rows |
                                       QItemSelectionModel::SelectionFlag::ClearAndSelect);
    m_widgets.pCommandsView->scrollTo(toSelectId, QAbstractItemView::PositionAtCenter);
  }
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::searchChanged(const QString& text)
{
  m_matchedIndices.clear();
  m_searchValue = text.toLower();

  asiUI_TreeModel* model = dynamic_cast<asiUI_TreeModel*>(m_widgets.pCommandsView->model());
  auto selectionModel = m_widgets.pCommandsView->selectionModel();
  // store text of the selected item
  QString selectedText;
  QModelIndexList selIndices = selectionModel->selectedIndexes();
  if (!selIndices.isEmpty())
  {
    selectedText = model->data(selIndices.first()).toString();
  }

  findMatchedIndices(m_searchValue, model, m_matchedIndices);

  bool isSelectedFound = false;
  if (!selectedText.isEmpty())
  {
    // restore selection by stored selected text if the item exists
    for (int i = 0; i < m_matchedIndices.size(); i++)
    {
      QModelIndex index = m_matchedIndices[i];
      if (model->data(index).toString() != selectedText)
        continue;
      isSelectedFound = true;
      m_widgets.pCommandsView->scrollTo(index, QAbstractItemView::PositionAtCenter);
      break;
    }
  }
  if (!isSelectedFound)
  {
    if (m_matchedIndices.length() == 0)
    {
      selectionModel->clearSelection();
    }
    else
    {
      auto idToSelect = m_matchedIndices[0];
      if (!selIndices.isEmpty())
      {
        idToSelect = findNext(selIndices[0], m_matchedIndices);
      }
      selectionModel->select(idToSelect, QItemSelectionModel::SelectionFlag::Rows |
                                         QItemSelectionModel::SelectionFlag::ClearAndSelect);
      m_widgets.pCommandsView->scrollTo(idToSelect, QAbstractItemView::PositionAtCenter);
    }
  }

  model->setHighlighted(m_matchedIndices);
  model->emitLayoutChanged();
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::searchDeactivated()
{
  m_matchedIndices.clear();

  asiUI_TreeModel* model = dynamic_cast<asiUI_TreeModel*>(m_widgets.pCommandsView->model());
  model->setHighlighted(m_matchedIndices);
  model->emitLayoutChanged();
}

//-----------------------------------------------------------------------------
void asiUI_DialogCommands::searchUp()
{
  selectMatchedIndex(false);
}

//-----------------------------------------------------------------------------
void asiUI_DialogCommands::searchDown()
{
  selectMatchedIndex(true);
}

//-----------------------------------------------------------------------------

void asiUI_DialogCommands::onClose()
{
  this->close();
}
