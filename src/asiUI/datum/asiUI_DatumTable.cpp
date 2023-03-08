//-----------------------------------------------------------------------------
// Created on: 26 February 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Anton Poletaev, Sergey Slyadnev
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
#include <asiUI_DatumTable.h>
#include <asiUI_DatumClipboardTableData.h>
#include <asiUI_DatumClipboardTool.h>

// asiUI includes
#include <asiUI_HeaderView.h>

// Qt includes
#pragma warning(push, 0)
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#pragma warning(pop)

//! Constructor for empty datum table.
//! \param theFactory [in] factory instance.
//! \param theParent [in] parent widget.
asiUI_DatumTable::asiUI_DatumTable(const Handle(asiUI_WidgetFactory)& theFactory,
                                   QWidget* theParent)
: asiUI_DatumViewBase<QTableView>(theParent),
  m_CursorProp(LeftToRight),
  m_CornerWidget(NULL),
  m_bGeomUpdateBlock(false)
{
  asiUI_DatumViewModel* aModel       = new asiUI_DatumViewModel(this);
  asiUI_DatumViewItem*  aPrototype   = new asiUI_DatumViewItem();
  asiUI_DatumViewDelegate *aDelegate = new asiUI_DatumViewDelegate(this);

  // note that init will call virtual method of this, not its successors
  init(theFactory, aModel, aPrototype, aDelegate);

  // override header views
  QHeaderView* aRowHeader = new QHeaderView(Qt::Vertical, this);
  QHeaderView* aColHeader = new QHeaderView(Qt::Horizontal, this);
  aRowHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
  this->setVerticalHeader(aRowHeader);
  this->setHorizontalHeader(aColHeader);

  m_CanExpandOnPaste[Qt::Horizontal] = true;
  m_CanExpandOnPaste[Qt::Vertical]   = true;

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect( this, SIGNAL( customContextMenuRequested(QPoint) ),
           this, SLOT  ( ContextMenu(QPoint) ) );
}

//! Default destructor.
asiUI_DatumTable::~asiUI_DatumTable()
{
}

//! Get value of the table item.
//! If the datum is assigned, the returned value is the one converted to the SI 
//! unit system. This value can differ with the displayed string.
//! \param theRow [in] the table's row.
//! \param theColumn [in] the table's column.
//! \return SI unit system value or null value if invalid row or column
//!         specified.
QVariant asiUI_DatumTable::GetValue(const int theRow, const int theColumn) const
{
  return getValue( itemModel()->index(theRow, theColumn) );
}

//! Set value for the table item.
//! If the datum is assigned, the value will be considered as the one converted to the
//! SI unit system. This value can differ with the displayed string.
//! \param theRow [in] the table's row.
//! \param theColumn [in] the table's column.
//! \param theValue [in] the SI unit system value.
void asiUI_DatumTable::SetValue(const int theRow,
                                 const int theColumn,
                                 const QVariant& theValue)
{
  setValue( itemModel()->index(theRow, theColumn), theValue );
}

//! Get displayed string.
//! The value corresponds to the editor's contents that corresponds to the value
//! converted to local unit system.
//! \param theRow [in] the table's row.
//! \param theColumn [in] the table's column.
//! \return displayed string or null string if invalid row or column is specified.
QString asiUI_DatumTable::GetString(const int theRow, const int theColumn) const
{
  return getString( itemModel()->index(theRow, theColumn) );
}

//! Set displayed string.
//! The value corresponds to the editor's contents which is a value converted
//! to local unit system.
//! \param theRow [in] the table's row.
//! \param theColumn [in] the table's column.
void asiUI_DatumTable::SetString(const int theRow,
                                  const int theColumn,
                                  const QString& theString)
{
  setString( itemModel()->index(theRow, theColumn), theString );
}

//! The method sets propagation of edit cursor position 
//! over cells when user presses return key or finishes
//! cell edition.
//! \param theDir the direction to propagate the cursor.
void asiUI_DatumTable::SetCursorPropagation(const CursorPropagation theDir)
{
  m_CursorProp = theDir;
}

//! The method gets propagation of edit cursor position 
//! over cells when user presses return key or finishes
//! cell edition.
//! \return the direction to propagate the cursor.
asiUI_DatumTable::CursorPropagation 
  asiUI_DatumTable::GetCursorPropagation() const
{
  return m_CursorProp;
}

//! Set list of user-defined titles for the columns.
//! If null string specified for a column, the title will be 
//! generated automatically by the assigned datum editor for
//! the column.
//! \param theTitles [in] list of title strings.
void asiUI_DatumTable::SetColumnTitles(const QStringList& theTitles)
{
  asiUI_DatumViewModel* aModel = itemModel();

  m_Titles[Qt::Horizontal] = StringVector::fromList(theTitles);
  m_Titles[Qt::Horizontal].resize( aModel->columnCount() );

  if ( aModel->columnCount() > 0 )
  {
    updateHeaders( Qt::Horizontal, 0, aModel->columnCount() - 1 );
  }
}

//! Set list of user-defined titles for the rows.
//! If null string specified for a row, the title will be 
//! generated automatically by the assigned datum editor for
//! the row.
//! \param theTitles [in] list of title strings.
void asiUI_DatumTable::SetRowTitles(const QStringList& theTitles)
{
  asiUI_DatumViewModel* aModel = itemModel();

  m_Titles[Qt::Vertical] = StringVector::fromList(theTitles);
  m_Titles[Qt::Vertical].resize( aModel->rowCount() );

  if ( aModel->rowCount() > 0 )
  {
    updateHeaders( Qt::Vertical, 0, aModel->rowCount() - 1 );
  }
}

//! Sets state whether the table can be expanded in rows/columns by pasting values
//! from the clipboard. If not, the values out of the table size will be ignored.
//! \param theOrientation [in] horizontal means columns, vertial - rows.
//! \param theValue[in]
void asiUI_DatumTable::SetCanExpandOnPaste(const Qt::Orientation theOrientation,
                                           const bool            theValue)
{
  m_CanExpandOnPaste[theOrientation] = theValue;
}

//! Get user-defined title string for the column.
//! \param theColumn [in] the column index.
QString asiUI_DatumTable::GetColumnTitle(const int theColumn) const
{
  ASSERT_RAISE( theColumn >= 0 && theColumn < itemModel()->columnCount(), "index is out of range" );

  return m_Titles[Qt::Horizontal].value(theColumn);
}

//! Set user-defined title string for the column.
//! If null string specified for a column, the title will be 
//! generated automatically by the assigned datum editor for
//! the column.
//! \param theColumn [in] the column index.
//! \param theLabel [in] the title string.
void asiUI_DatumTable::SetColumnTitle(const int theColumn, const QString& theTitle)
{
  ASSERT_RAISE( theColumn >= 0 && theColumn < itemModel()->columnCount(), "index is out of range" );

  m_Titles[Qt::Horizontal][theColumn] = theTitle;

  updateHeaders(Qt::Horizontal, theColumn, theColumn);
}

//! Get user-defined title string for the row.
//! \param theRow [in] the row index.
QString asiUI_DatumTable::GetRowTitle(const int theRow) const
{
  ASSERT_RAISE( theRow >= 0 && theRow < itemModel()->rowCount(), "index is out of range" );

  return m_Titles[Qt::Vertical][theRow];
}

//! Set user-defined title string for the row.
//! If null string specified for a row, the title will be 
//! generated automatically by the assigned datum editor for
//! the row.
//! \param theRow [in] the row index.
//! \param theLabel [in] the title string.
void asiUI_DatumTable::SetRowTitle(const int theRow, const QString& theTitle)
{
  ASSERT_RAISE( theRow >= 0 && theRow < itemModel()->rowCount(), "index is out of range" );

  m_Titles[Qt::Vertical][theRow] = theTitle;

  updateHeaders( Qt::Vertical, theRow, theRow );
}

//! Adds header group to the section hierarchy tree for the given header.
//! \param theHeader [in] the header to add the group.
//! \param theGroup [in] the group index.
void asiUI_DatumTable::AddHeaderGroupSection(const Qt::Orientation theHeader,
                                              const asiUI_HeaderIndex& theGroup)
{
  itemModel()->AddHeaderGroup(theGroup, theHeader);
}

//! Removes header group from the section hierarchy tree for the given header.
//! \param theHeader [in] the header to remove the group from.
//! \param theGroup [in] the group index.
void asiUI_DatumTable::RemoveHeaderGroupSection(const Qt::Orientation theHeader,
                                                 const asiUI_HeaderIndex& theGroup)
{
  itemModel()->RemoveHeaderGroup(theGroup, theHeader);
}

//! Sets data for the header item specified by its index.
//! \param theHeader [in] the header type.
//! \param theIndex [in] the header item index.
//! \param theValue [in] the value to set.
//! \param theRole [in] the role.
void asiUI_DatumTable::SetHeaderItemData(const Qt::Orientation theHeader,
                                          const asiUI_HeaderIndex& theIndex,
                                          const QVariant& theValue,
                                          const int theRole)
{
  itemModel()->SetHeaderItemData(theHeader, theIndex, theValue, theRole);
}

//! Queries data from the given header item.
//! \param theHeader [in] the header type.
//! \param theIndex [in] the header item index.
//! \param theRole [in] the role.
QVariant asiUI_DatumTable::HeaderItemData(const Qt::Orientation theHeader,
                                           const asiUI_HeaderIndex& theIndex,
                                           const int theRole) const
{
  return itemModel()->HeaderItemData(theHeader, theIndex, theRole);
}

//! Sets item to be used at header.
//! \param theHeader [in] the header type.
//! \param theIndex [in] the header item index.
//! \param theItem [in] the item to set.
void asiUI_DatumTable::SetHeaderItem(const Qt::Orientation theHeader,
                                      const asiUI_HeaderIndex& theIndex,
                                      QStandardItem* theItem)
{
  itemModel()->SetHeaderItem(theHeader, theIndex, theItem);
}

//! Queries item used at header.
//! \param theHeader [in] the header type.
//! \param theIndex [in] the header item index.
//! \return the used item.
QStandardItem* asiUI_DatumTable::HeaderItem(const Qt::Orientation theHeader,
                                             const asiUI_HeaderIndex& theIndex) const
{
  return itemModel()->HeaderItem(theHeader, theIndex);
}

//! Changes number of rows for header band view.
//! \param theHeader [in] the header.
//! \param theRows [in] the rows to show.
void asiUI_DatumTable::SetHeaderBandsCount(const Qt::Orientation theHeader, const int theRows)
{
  itemModel()->SetBandRowCount(theHeader, theRows);

  m_BandEditors[theHeader].Resize( theRows, m_BandEditors[theHeader].Cols() );
}

//! Returns number of rows or columns for header band view.
//! \param theHeader [in] the header.
//! \return the requested number of columns / lines
Standard_Integer asiUI_DatumTable::GetHeaderBandsCount(const Qt::Orientation theHeader) const
{
  return itemModel()->BandRowsCount(theHeader); 
}

//! Sets default editor for the band for the given header.
//! \param theHeader [in] the header.
//! \param theDatum [in] the datum editor.
void asiUI_DatumTable::SetHeaderBandDatum(const Qt::Orientation theHeader, const QString& theDatum)
{
  m_BandEditors[theHeader].SetDefaultEditor(theDatum);
}

//! Sets editor for the whole section in the band for the given header.
//! \param theHeader [in] the header.
//! \param theCol [in] the header column.
//! \param theDatum [in] the datum editor.
void asiUI_DatumTable::SetHeaderBandSectDatum(const Qt::Orientation theHeader,
                                               const QString& theDatum,
                                               const int theHeaderSectIdx)
{
  m_BandEditors[theHeader].SetColEditor(theHeaderSectIdx, theDatum);
}

//! Sets editor for the whole header band line for the given header.
//! \param theHeader [in] the header.
//! \param theDatum [in] the datum editor.
//! \param theBandLineIndex [in] the index of band line.
void asiUI_DatumTable::SetHeaderBandLineDatum(const Qt::Orientation theHeader,
                                               const QString& theDatum,
                                               const int theBandLineIndex)
{
  m_BandEditors[theHeader].SetRowEditor(theBandLineIndex, theDatum);
}

//! Sets editor for the header band item for the given header.
//! \param theHeader [in] the header.
//! \param theDatum [in] the datum editor.
//! \param theIndex [in] the band header item index.
void asiUI_DatumTable::SetHeaderBandDatum(const Qt::Orientation theHeader,
                                           const QString& theDatum,
                                           const asiUI_HeaderIndex& theIndex)
{
  ASSERT_RAISE( theIndex.IsBand(), "index is of invalid type" );
  ASSERT_RAISE( theIndex.GetBandLine() >= 0 
             && theIndex.GetBandLine() < itemModel()->BandRowsCount(theHeader),
                "index is out of range" );

  m_BandEditors[theHeader].SetItemEditor( theIndex.GetBandLine(), theIndex.GetSection(), theDatum );
}

//! Resets hierarchy header structure.
//! \param theHeader [in] the header to reset.
void asiUI_DatumTable::ResetHierarchyHeader(const Qt::Orientation theHeader)
{
  itemModel()->ResetHierarchyHeader(theHeader);
}

//! Resets band header structure.
//! \param theHeader [in] the header to reset.
void asiUI_DatumTable::ResetBandRowHeader(const Qt::Orientation theHeader)
{
  itemModel()->ResetBandHeader(theHeader);
}

//! Reset headers to default string of base view.
//! \param theOrientation [in] header's orientation.
//! \param theStartIdx [in] the start index.
//! \param theEndIdx [in] the end index.
void asiUI_DatumTable::resetHeaders(const Qt::Orientation& theOrientation,
                                     const int theStartIdx,
                                     const int theEndIdx)
{
  asiUI_DatumViewModel* aModel = itemModel();

  int aMaxCount = ( theOrientation == Qt::Vertical ) ?
    aModel->rowCount() : aModel->columnCount();

  ASSERT_RAISE(theStartIdx <= theEndIdx &&
               theStartIdx >= 0 &&
               theEndIdx < aMaxCount, "passed invalid indexes");

  for ( int aIdx = theStartIdx; aIdx <= theEndIdx; aIdx++ )
  {
    // this is not quite straightforward logic - it forces the model
    // to revert header string to its default value
    if ( theOrientation == Qt::Vertical )
      aModel->setVerticalHeaderItem(aIdx, NULL);
    else
      aModel->setHorizontalHeaderItem(aIdx, NULL);
  }
}

//! Update headers for the index range.
//! \param theOrientation [in] header's orientation.
//! \param theStartIdx [in] the start index.
//! \param theEndIdx [in] the end index.
void asiUI_DatumTable::updateHeaders(const Qt::Orientation& theOrientation,
                                      const int theStartIdx,
                                      const int theEndIdx)
{
  asiUI_DatumViewModel* aModel = itemModel();

  int aMaxCount = ( theOrientation == Qt::Vertical ) ?
    aModel->rowCount() : aModel->columnCount();

  const VectorOfStrings& aTitleVector = m_Titles[theOrientation];

  ASSERT_RAISE(theStartIdx <= theEndIdx &&
               theStartIdx >= 0 &&
               theEndIdx < aMaxCount, "passed invalid indexes");

  disconnect( itemModel(),
    SIGNAL( headerDataChanged(Qt::Orientation, int, int) ), this,
    SLOT( onHeaderDataChanged(Qt::Orientation, int, int) ) );

  for ( int aIdx = theStartIdx; aIdx <= theEndIdx; aIdx++ )
  {
    QString aDatum = theOrientation == Qt::Vertical
      ? m_ViewEditors.GetRowEditor(aIdx) 
      : m_ViewEditors.GetColEditor(aIdx);

    QString aLabel = labelString(aDatum, aTitleVector[aIdx]);
    if ( !aLabel.isNull() )
    {
      aModel->setHeaderData(aIdx, theOrientation, aLabel);
    }
  }

  connect( itemModel(),
    SIGNAL( headerDataChanged(Qt::Orientation, int, int) ),
    SLOT( onHeaderDataChanged(Qt::Orientation, int, int) ) );
}

//! Re-implement method to add support for custom corner widgets.
void asiUI_DatumTable::updateGeometries()
{
  if (m_bGeomUpdateBlock)
  {
    return;
  }

  QTableView::updateGeometries();

  if ( !m_CornerWidget )
  {
    return;
  }

  m_bGeomUpdateBlock = true;

  // update cornerWidget
  if ( horizontalHeader()->isHidden() || verticalHeader()->isHidden() )
  {
    m_CornerWidget->setHidden(true);
  }
  else
  {
    QSize aSizeHint = m_CornerWidget->sizeHint();

    int aWidth  = verticalHeader()->width();
    int aHeight = horizontalHeader()->height();

    int aDx = 0;
    int aDy = 0;
    if ( aSizeHint.width() > aWidth )
    {
      aDx = aSizeHint.width() - aWidth;
      aWidth = aSizeHint.width();
    }
    if ( aSizeHint.height() > aHeight )
    {
      aDy = aSizeHint.height() - aHeight;
      aHeight = aSizeHint.height();
    }

    this->setViewportMargins(aWidth, aHeight, 0, 0);

    QRect aVg = viewport()->geometry();
    int aLeft = aVg.left() - aWidth;
    int aTop  = aVg.top()  - aHeight;

    horizontalHeader()->setGeometry(aLeft + aWidth, aTop, horizontalHeader()->width(), aHeight);
    verticalHeader()->setGeometry(aLeft, aTop + aHeight, aWidth, verticalHeader()->height());

    m_CornerWidget->setHidden(false);
    m_CornerWidget->setGeometry(aLeft, aTop, aWidth, aHeight);
  }

  m_bGeomUpdateBlock = false;
}

//! Restrict outside modification of header data for
//! rows and column that have user-defined titles.
//! \param theOrientation [in] the header orientation.
//! \param theFirst [in] the first changed header.
//! \param theLast [in] the last changed header.
void asiUI_DatumTable::onHeaderDataChanged(Qt::Orientation theOrientation,
                                            int theFirst,
                                            int theLast)
{
  updateHeaders(theOrientation, theFirst, theLast);
}

//! Synchronize internal data structures when number of columns changes.
//! \param theParent [in] the parent index for new columns.
//! \param theStart [in] the first inserted column index.
//! \param theEnd [in] the last inserted column index.
void asiUI_DatumTable::onColumnsInserted(QModelIndex /*theParent*/, int theStart, int theEnd)
{
  const int aNum = theEnd - theStart + 1;

  // insert new entries into datum and title arrays
  m_ViewEditors.InsertCols(theStart, aNum);
  m_Titles[Qt::Horizontal].insert( theStart, aNum, QString() );
  m_BandEditors[Qt::Horizontal].InsertCols(theStart, aNum);
}

//! Synchronize internal data structures when number of columns changes.
//! \param theParent [in] the parent index for removed columns.
//! \param theStart [in] the first removed column index.
//! \param theEnd [in] the last removed column index.
void asiUI_DatumTable::onColumnsRemoved(QModelIndex /*theParent*/, int theStart, int theEnd)
{
  const int aNum = theEnd - theStart + 1;

  // remove entries from datum and title arrays
  m_ViewEditors.RemoveCols(theStart, aNum);
  m_Titles[Qt::Horizontal].remove(theStart, aNum);
  m_BandEditors[Qt::Horizontal].RemoveCols(theStart, aNum);
}

//! Synchronize internal data structures when number of rows changes.
//! \param theParent [in] the parent index for new rows.
//! \param theStart [in] the first inserted row index.
//! \param theEnd [in] the last inserted row index.
void asiUI_DatumTable::onRowsInserted(QModelIndex /*theParent*/, int theStart, int theEnd)
{
  const int aNum = theEnd - theStart + 1;

  // insert new entries into datum and title arrays
  if ( aNum > 0 )
  {
    m_ViewEditors.InsertRows(theStart, aNum);
    m_Titles[Qt::Vertical].insert( theStart, aNum, QString() );
    m_BandEditors[Qt::Vertical].InsertCols(theStart, aNum);
  }
}

//! Synchronize internal data structures when number of rows changes.
//! \param theParent [in] the parent index for removed row.
//! \param theStart [in] the first removed row index.
//! \param theEnd [in] the last removed row index.
void asiUI_DatumTable::onRowsRemoved(QModelIndex /*theParent*/, int theStart, int theEnd)
{
  const int aNum = theEnd - theStart + 1;

  // remove entries from datum and title arrays
  if ( aNum > 0 )
  {
    m_ViewEditors.RemoveRows(theStart, aNum);
    m_Titles[Qt::Vertical].remove(theStart, aNum);
    m_BandEditors[Qt::Vertical].RemoveCols(theStart, aNum);
  }
}

//-----------------------------------------------------------------------------

void asiUI_DatumTable::ContextMenu(QPoint pos)
{
  if (selectionModel()->selectedIndexes().isEmpty())
    return;

  // Create and populate the menu.
  QMenu* pMenu = new QMenu(this);

  QAction* pCopyAction  = new QAction("Copy", this);
  QAction* pPasteAction = new QAction("Paste", this);

  pMenu->addAction(pCopyAction);
  pMenu->addAction(pPasteAction);

  connect( pCopyAction,  SIGNAL( triggered() ), this, SLOT( CopyClipboardData  () ) );
  connect( pPasteAction, SIGNAL( triggered() ), this, SLOT( PasteClipboardData () ) );

  if ( !pMenu->isEmpty() )
    pMenu->popup( this->viewport()->mapToGlobal(pos) );
}

//! Copy values from the table into clipboard if possible.
void asiUI_DatumTable::CopyClipboardData()
{
  //std::cout << "Copy key pressed" << std::endl;

  // Perform copying to intermediate buffer
  asiUI_DatumClipboardTool::CopyToClipboard(this);
}

//! Copy values from the clipboard into the table if possible.
void asiUI_DatumTable::PasteClipboardData()
{
  //std::cout << "Paste key pressed" << std::endl;

  QItemSelectionModel* aSelModel = this->selectionModel();

  // table-layout like selection rectangle
  QModelIndexList anArrangedIndexes;
  QRect aSelectionRect = asiUI_DatumClipboardTool::GetContiguous(
    this, aSelModel->selection(), anArrangedIndexes);

  if ( !aSelectionRect.isValid() )
    return;

  QModelIndex aTopLeft  = anArrangedIndexes.first();
  QModelIndex aBotRight = anArrangedIndexes.last();

  // get paste rectangle in accordance
  // with selection made and Excel rules
  QRect aRuledRect = asiUI_DatumClipboardTool::GetPasteRuleRegion(aSelectionRect);

  // warn if selected region is bigger than paste data
  if ( aSelectionRect.height() > aRuledRect.height() ||
       aSelectionRect.width()  > aRuledRect.width() )
  {
    if ( QMessageBox::warning(NULL,
           "Selected region is too large.",
           "Selected region is larger than the clipboard data.\nDo you want to paste the data anyway?",
           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
      return;
  }

  // get model indexes under selection and number of
  // rows and columns missing to fit the whole insertion rect
  int aMissingRows = 0;
  int aMissingCols = 0;

  QModelIndexList aRuledIndexes = 
    asiUI_DatumClipboardTool::GetIndexesFromRect(this, aRuledRect, aMissingRows, aMissingCols);

  // Let operation to decide whether the tables can be re-sized
  int aCanExpandRows = 0;
  int aCanExpandCols = 0;
  if ( QTableView* aTableView = qobject_cast<QTableView*>( this ) )
  {
    // only row expansion is allowed
    aCanExpandRows = m_CanExpandOnPaste[Qt::Vertical]   ? aMissingRows : 0;
    aCanExpandCols = m_CanExpandOnPaste[Qt::Horizontal] ? aMissingCols : 0;
  }

  // get final rectangle for insertion
  QRect aFinalRect = aRuledRect.adjusted(0, 0,
    aCanExpandCols - aMissingCols, aCanExpandRows - aMissingRows);

  if ( aRuledRect.right()  > aFinalRect.right() 
    || aRuledRect.bottom() > aFinalRect.bottom() )
  {
    if ( QMessageBox::warning(NULL,
           "Clipboard data does not fit into table.",
           "Clipboard data does not fit into the table. Only part of it will be copied.\
Do you want to paste the data anyway?",
           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
      return;
  }

  // expand table and appropriately modify selection
  if ( QTableView* aTableView = qobject_cast<QTableView*>( this ) )
  {
    // Expand table
    if ( aCanExpandRows > 0 || aCanExpandCols > 0 )
    {
      //expandTableOnPaste(aTableView, aCanExpandRows, aCanExpandCols);
      QAbstractItemModel* aModel = aTableView->model();
      aModel->insertRows( aModel->rowCount(), aCanExpandRows );
      aModel->insertColumns( aModel->columnCount(), aCanExpandCols );
    }

    // change selection in accordance with the pasted region
    QAbstractItemModel* aModel = aTableView->model();
    QModelIndex aTopLeft2 = aModel->index( aFinalRect.top(), aFinalRect.left() );
    QModelIndex aBotRight2 = aModel->index( aFinalRect.bottom(), aFinalRect.right() );

    aTableView->selectionModel()->select(
      QItemSelection(aTopLeft2, aBotRight2), QItemSelectionModel::ClearAndSelect);
  }
  else
  {
    // no expansion possible here, provide coherent selection
    QItemSelectionModel* aSelectionModel = this->selectionModel();
    aSelectionModel->clearSelection();

    QModelIndexList::ConstIterator aIndexIt = aRuledIndexes.constBegin();
    for ( ; aIndexIt != aRuledIndexes.constEnd(); aIndexIt++ )
      aSelectionModel->select( (*aIndexIt), QItemSelectionModel::Select );
  }

  // Paste data into destination Table View
  asiUI_DatumClipboardTool::PasteFromClipboard(this);
}

//! Synchronize internal data structures with the new model.
//! Connect signals of newly passed model to internal slots for tracking events of
//! inserting / removing rows and columns. Once any of these events happens,
//! the internal data structures should be adjusted.
//! \param theModel [in] the model to set.
void asiUI_DatumTable::setModel(QAbstractItemModel* theModel)
{
  if ( model() == theModel )
  {
    return;
  }

  if ( QAbstractItemModel* aBeforeModel = itemModel() )
  {
    disconnect( aBeforeModel,
      SIGNAL( headerDataChanged(Qt::Orientation, int, int) ), this,
      SLOT( onHeaderDataChanged(Qt::Orientation, int, int) ) );

    disconnect( aBeforeModel,
      SIGNAL( columnsInserted(QModelIndex, int, int) ), this,
      SLOT( onColumnsInserted(QModelIndex, int, int) ) );

    disconnect( aBeforeModel,
      SIGNAL( columnsRemoved(QModelIndex, int, int) ), this,
      SLOT( onColumnsRemoved(QModelIndex, int, int) ) );

    disconnect( aBeforeModel,
      SIGNAL( rowsInserted(QModelIndex, int, int) ), this,
      SLOT( onRowsInserted(QModelIndex, int, int) ) );

    disconnect( aBeforeModel,
      SIGNAL( rowsRemoved(QModelIndex, int, int) ), this,
      SLOT( onRowsRemoved(QModelIndex, int, int) ) );
  }

  if ( QAbstractItemModel* aAfterModel = itemModel(theModel) )
  {
    connect( aAfterModel,
      SIGNAL( headerDataChanged(Qt::Orientation, int, int) ),
      SLOT( onHeaderDataChanged(Qt::Orientation, int, int) ) );

    connect( aAfterModel,
      SIGNAL( columnsInserted(QModelIndex, int, int) ), this,
      SLOT( onColumnsInserted(QModelIndex, int, int) ) );

    connect( aAfterModel,
      SIGNAL( columnsRemoved(QModelIndex, int, int) ), this,
      SLOT( onColumnsRemoved(QModelIndex, int, int) ) );

    connect( aAfterModel,
      SIGNAL( rowsInserted(QModelIndex, int, int) ), this,
      SLOT( onRowsInserted(QModelIndex, int, int) ) );

    connect( aAfterModel,
      SIGNAL( rowsRemoved(QModelIndex, int, int) ), this,
      SLOT( onRowsRemoved(QModelIndex, int, int) ) );

    // synchronize internal data structures
    const int aRows = aAfterModel->rowCount();
    const int aCols = aAfterModel->columnCount();

    m_Titles[Qt::Vertical].resize(aRows);
    m_Titles[Qt::Horizontal].resize(aCols);
    m_ViewEditors.Resize( aRows, aCols );
    m_BandEditors[Qt::Vertical]  .Resize( m_BandEditors[Qt::Vertical].Rows(),   aRows );
    m_BandEditors[Qt::Horizontal].Resize( m_BandEditors[Qt::Horizontal].Rows(), aCols );
  }

  asiUI_DatumViewBase<QTableView>::setModel(theModel);
}

//! Move cursor taking into account propagation
//! preferences. Warning up-to-down doesn't take into
//! account table spans.
//! \param theAction [in] the cursor action.
//! \param theModifiers [in] the keyboard modifiers.
QModelIndex asiUI_DatumTable::moveCursor(CursorAction theAction,
                                          Qt::KeyboardModifiers theModifiers)
{
  switch ( theAction )
  {
    case MoveNext:
    {
      if ( m_CursorProp == Hold ) // hold right there
        return currentIndex();

      if ( m_CursorProp == LeftToRight ) // nothing specific
        return asiUI_DatumViewBase<QTableView>::moveCursor(theAction, theModifiers);

      /* ===============================
       *  Provide down-cell propagation
       * =============================== */

      QModelIndex aCurrIdx = currentIndex();
      QModelIndex aDownNoWrap = 
        asiUI_DatumViewBase<QTableView>::moveCursor(MoveDown, theModifiers);

      // no wrap required
      if ( !aCurrIdx.isValid() || aDownNoWrap != aCurrIdx )
        return aDownNoWrap; 

      QAbstractItemModel* aModel = itemModel();
      const int aRowCount = aModel->rowCount();
      const int aColCount = aModel->columnCount();

      // provide cursor wrap to next row
      int aRow = 0; // at top
      int aCol = aCurrIdx.column() + 1;

      // find cell to settle on
      while ( aCol < aColCount && isColumnHidden(aCol) )
        aCol += 1;
      while ( aRow < aRowCount && isRowHidden(aRow) )
        aRow += 1;

      // return found cell if in bounds
      QModelIndex aWrapIdx = aModel->index(aRow, aCol);
      if ( aWrapIdx.isValid() )
        return aWrapIdx;

      // return first locable one
      aRow = 0;
      aCol = 0;

      // find cell to settle on
      while ( aCol < aColCount && isColumnHidden(aCol) )
        aCol += 1;
      while ( aRow < aRowCount && isRowHidden(aRow) )
        aRow += 1;

      return aModel->index(aRow, aCol);
    }
    break;

    case MovePrevious:
    {
      if ( m_CursorProp == Hold ) // hold right there
        return currentIndex();

      if ( m_CursorProp == LeftToRight ) // nothing specific
        return asiUI_DatumViewBase<QTableView>::moveCursor(theAction, theModifiers);

      /* ===============================
       *  Provide up-cell propagation
       * =============================== */

      QModelIndex aCurrIdx = currentIndex();
      QModelIndex aUpNoWrap =
        asiUI_DatumViewBase<QTableView>::moveCursor(MoveUp, theModifiers);

      // no wrap required
      if ( !aCurrIdx.isValid() || aUpNoWrap != aCurrIdx )
        return aUpNoWrap; 

      QAbstractItemModel* aModel = itemModel();
      const int aRowCount = aModel->rowCount();
      const int aColCount = aModel->columnCount();

      // provide cursor wrap to previous row
      int aRow = aRowCount - 1; // at bottom
      int aCol = aCurrIdx.column() - 1;

      // find cell to settle on
      while ( aCol >= 0 && isColumnHidden(aCol) )
        aCol -= 1;
      while ( aRow >= 0 && isRowHidden(aRow) )
        aRow -= 1;

      // return found cell if in bounds
      QModelIndex aWrapIdx = aModel->index(aRow, aCol);
      if ( aWrapIdx.isValid() )
        return aWrapIdx;

      // return last locable one
      aRow = aRowCount - 1;
      aCol = aColCount - 1;

      // find cell to settle on
      while ( aCol >= 0 && isColumnHidden(aCol) )
        aCol -= 1;
      while ( aRow >= 0 && isRowHidden(aRow) )
        aRow -= 1;

      return aModel->index(aRow, aCol);
    }
    break;

    default:
      break;
  }

  // perform default movement
  return asiUI_DatumViewBase<QTableView>::moveCursor(theAction, theModifiers);
}

//! Update items for the given index range.
//! \param theStartRow [in] the first row index in the range.
//! \param theEndRow [in] the last row index in the range.
//! \param theStartColumn [in] the first column index in the range.
//! \param theEndColumn [in] the last column index in the range.
//! \param theSkipEdited [in] skip the cells being edited or not.
void asiUI_DatumTable::updateItems(const int theStartRow,    const int theEndRow,
                                    const int theStartColumn, const int theEndColumn,
                                    const bool theSkipEdited)
{
  asiUI_DatumViewModel* aModel = itemModel();

  int aRowCount = aModel->rowCount();
  int aColumnCount = aModel->columnCount();

  ASSERT_RAISE(theStartRow    <= theEndRow    &&
               theStartColumn <= theEndColumn &&
               theStartRow    >= 0            && 
               theStartColumn >= 0            &&
               theEndRow      < aRowCount     &&
               theEndColumn   < aColumnCount,
               "invalid indexes given");

  // collect index range
  QModelIndexList anUpdateList;
  for ( int aRowIt = theStartRow; aRowIt <= theEndRow; aRowIt++ )
    for ( int aColIt = theStartColumn; aColIt <= theEndColumn; aColIt++ )
      anUpdateList << aModel->index(aRowIt, aColIt);

  asiUI_DatumViewBase<QTableView>::updateItems(anUpdateList, theSkipEdited);
}

//! Get number of rows in the table.
//! \return number of rows.
int asiUI_DatumTable::GetRowCount() const
{
  return itemModel()->rowCount();
}

//! Get number of columns in the table.
//! \return number of columns.
int asiUI_DatumTable::GetColumnCount() const
{
  return itemModel()->columnCount();
}

//! Set number of rows for the table.
//! \param theRows [in] new number of rows.
void asiUI_DatumTable::SetRowCount(const int theRows)
{
  QStandardItemModel* aModel = itemModel();

  int aOldCount = aModel->rowCount();

  aModel->setRowCount(theRows);

  if ( aModel->signalsBlocked() )
  {
    onRowsRemoved( QModelIndex(), 0, aOldCount - 1 );
    onRowsInserted( QModelIndex(), 0, theRows - 1 );
  }

  if ( theRows > aOldCount )
  {
    updateHeaders(Qt::Vertical, aOldCount, theRows - 1);
  }
}

//! Set number of columns for the table.
//! \param theCols [in] new number of columns.
void asiUI_DatumTable::SetColumnCount(const int theCols)
{
  QStandardItemModel* aModel = itemModel();

  int aOldCount = aModel->columnCount();

  aModel->setColumnCount(theCols);

  if ( aModel->signalsBlocked() )
  {
    onColumnsRemoved( QModelIndex(), 0, aOldCount - 1 );
    onColumnsInserted( QModelIndex(), 0, theCols - 1 );
  }

  if ( theCols > aOldCount )
  {
    updateHeaders(Qt::Horizontal, aOldCount, theCols - 1);
  }
}

//! Access data item at the specified row and column index.
//! \param theRow [in] the data row.
//! \param theCol [in] the data col.
asiUI_DatumViewItem* asiUI_DatumTable::AccessItem(const int theRow, const int theCol) const
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theRow >= 0 && theRow < aModel->rowCount(), "row index is out of range" );
  ASSERT_RAISE( theCol >= 0 && theCol < aModel->columnCount(), "column index is out of range" );

  return accessItem( aModel->index(theRow, theCol) );
}

//! Set new item prototype for the view.
//! \param thePrototype [in] the prototype to set up.
void asiUI_DatumTable::SetItemPrototype(const asiUI_DatumViewItem* thePrototype)
{
  setItemPrototype(thePrototype);
}

//! Get list of selected view item indexes.
//! \return list of selected indexes.
asiUI_DatumViewApi::IndexList asiUI_DatumTable::GetSelectedIndexes() const
{
  asiUI_DatumViewApi::IndexList aResult;
  QModelIndexList aSelection = selectionModel()->selectedIndexes();
  QModelIndexList::Iterator anIt = aSelection.begin();
  for ( ; anIt != aSelection.end(); anIt++ )
  {
    QModelIndex& anIndex = *anIt;
    aResult << ItemIndex( anIndex.row(), anIndex.column() );
  }

  return aResult;
}

//! Set an indexes from the list are selected.
//! \param theList [in] the list of indexes.
//! \return true if success, otherwise - false.
bool asiUI_DatumTable::SelectIndexes(const asiUI_DatumViewApi::IndexList &theList)
{
  QItemSelection anItemSel;
  asiUI_DatumViewApi::IndexList::const_iterator it;
  for ( it = theList.begin(); it != theList.end(); ++it )
  {
    int aRow = it->Row;
    int aCol = it->Col;

    QModelIndex aCell = MapItemToView( aRow, aCol );

    if ( aRow > this->GetRowCount() || aCol > this->GetColumnCount() )
    {
      this->clearSelection();
      return false;
    }

    if ( !aCell.isValid() )
    {
      this->clearSelection();
      return false;
    }

    anItemSel.merge( QItemSelection( aCell, aCell ), QItemSelectionModel::Select );
  }

  selectionModel()->select( anItemSel, QItemSelectionModel::ClearAndSelect );
  return true;
}

//! Get index of data element at specified view's index.
//! \param the item index.
//! \return index of data element at specified view's index.
asiUI_DatumViewApi::ItemIndex
  asiUI_DatumTable::MapViewToItem(const QModelIndex& theIndex) const
{
  return ItemIndex( theIndex.row(), theIndex.column() );
}

//! Get view index of data element at specified
//! row and column index.
//! \param theRow [in] the data row.
//! \param theCol [in] the data col.
//! \return view item model index.
QModelIndex asiUI_DatumTable::MapItemToView(const int theRow, const int theCol) const
{
  return model()->index(theRow, theCol);
}

//! Return dictionary item identifier associated with the item.
//! The id is used during value formatting, and for editor
//! instantiation routines.
//! \param theIndex [in] the item index.
QString asiUI_DatumTable::getItemDatum(const QModelIndex& theIndex) const
{
  return m_ViewEditors.GetEditor( theIndex.row(), theIndex.column() );
}

//! Return dictionary item identifier associated with the header section.
//! The id is used during value formatting, and for editor
//! instantiation routines.
//! \param theHeader [in] the header type.
//! \param theIndex [in] the index of the header section.
QString asiUI_DatumTable::getHeaderDatum(const Qt::Orientation theHeader, const asiUI_HeaderIndex& theIndex) const
{
  if ( theIndex.IsBand() )
  {
    return m_BandEditors[theHeader].GetEditor( theIndex.GetBandLine(), theIndex.GetSection() );
  }

  return QString();
}

//! Refresh units in headers when unit system changes.
void asiUI_DatumTable::refreshUnitsInLabels()
{
  if ( GetColumnCount() > 0 )
  {
    updateHeaders( Qt::Horizontal, 0, GetColumnCount() - 1 );
  }

  if ( GetRowCount() > 0 )
  {
    updateHeaders( Qt::Vertical, 0, GetRowCount() -1 );
  }
}

//! Processes Copy/Paste keys combinations.
void asiUI_DatumTable::keyPressEvent(QKeyEvent *theEvent)
{
  switch ( theEvent->key() )
  {
    case Qt::Key_Copy :
    case Qt::Key_C :
    {
      if (theEvent->modifiers().testFlag(Qt::ControlModifier))
      {
        CopyClipboardData();
      }
      break;
    }

    case Qt::Key_Paste :
    case Qt::Key_V :
    {
      if (theEvent->modifiers().testFlag(Qt::ControlModifier))
      {
        PasteClipboardData();
      }
      break;
    }

    default:
      asiUI_DatumViewBase<QTableView>::keyPressEvent(theEvent);
  }
}

//! Emit appropriate signal when value changed.
//! \param theIndex [in] the model index of changed item.
//! \param theValue [in] the new value.
void asiUI_DatumTable::emitValueChanged(const QModelIndex& theIndex,
                                         const QVariant& theValue)
{
  emit ValueChanged(theIndex.row(), theIndex.column(), theValue);
}

//! Emit signal when value is started to be changed.
//! \param theIndex [in] the model index of changed item.
//! \param theWidget [in] the editor widget.
void asiUI_DatumTable::emitValueChanging(const QModelIndex& theIndex,
                                          QWidget* theWidget)
{
  emit ValueChanging(theIndex.row(), theIndex.column(), theWidget);
}

//! Emit signal when datum is closed and editing is finished.
//! \param theIndex [in] the model index of changed item.
//! \param theWidget [in] the editor widget.
void asiUI_DatumTable::emitDatumClosed(const QModelIndex& theIndex,
                                        QWidget* theWidget)
{
  emit DatumClosed(theIndex.row(), theIndex.column(), theWidget);
}

//! Emit signal when selection changes.
//! \param theSelected [in] the new selection.
//! \param theDeselected [in] the previous selection.
void asiUI_DatumTable::emitSelectionChanged(const QItemSelection& theSelected,
                                             const QItemSelection& theDeselected)
{
  emit SelectionChanged(theSelected, theDeselected);
}

//! Emit signal when custom selector is triggered for datum item.
//! \param theIndex [in] the model index of triggered item.
void asiUI_DatumTable::emitCustomSelectorTriggered(const QModelIndex& theIndex)
{
  emit CustomSelectorTriggered( theIndex.row(), theIndex.column() );
}

//! Appends a new row after the selected row or to the end if no row is selected (or multiple rows are selected).
void asiUI_DatumTable::InsertRow()
{
  int posAt = -1;
  QModelIndexList aSelection = selectionModel()->selectedIndexes();
  QModelIndexList::Iterator anIt = aSelection.begin();
  for ( ; anIt != aSelection.end(); anIt++ )
  {
    QModelIndex& anIndex = *anIt;
    posAt = std::max(posAt, anIndex.row());
  }
  int newRowId = posAt < 0 ? GetRowCount() : posAt + 1;
  InsertRows( newRowId, 1 );

  if ( GetColumnCount() > 0 )
  {
    updateItems( newRowId, newRowId, 0, GetColumnCount() - 1 );
  }
}

//! Insert rows into the table at the specified index.
//! \param theAt [in] insertion row index.
//! \param theRows [in] number of inserted rows.
void asiUI_DatumTable::InsertRows(const int theAt, const int theRows)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theAt >= 0 && theAt <= aModel->rowCount(), "invalid index specified" );

  aModel->insertRows(theAt, theRows);

  if ( aModel->signalsBlocked() )
  {
    onRowsInserted( QModelIndex(), theAt, theRows - theAt - 1 );
  }
}

//! Insert columns into the table at the specified index.
//! \param theAt [in] insertion column index.
//! \param theCols [in] number of inserted columns.
void asiUI_DatumTable::InsertColumns(const int theAt, const int theCols)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theAt >= 0 && theAt <= aModel->columnCount(), "invalid index specified" );

  aModel->insertColumns(theAt, theCols);

  if ( aModel->signalsBlocked() )
  {
    onColumnsInserted( QModelIndex(), theAt, theCols - theAt - 1 );
  }
}

//! Removes the currently selected row.
//! If a single cell is selected, the corresponding row is said to be selected;
//! If multiple rows are selected (or multiple cells in different rows), then all rows are removed.
//! If nothing is selected, it removes the latest row.
void asiUI_DatumTable::RemoveRows()
{
  std::set<int>  rowsToRemoveUnique;
  std::list<int> rowsToRemove;

  QModelIndexList aSelection = selectionModel()->selectedIndexes();
  QModelIndexList::Iterator anIt = aSelection.begin();
  for ( ; anIt != aSelection.end(); anIt++ )
  {
    QModelIndex& anIndex = *anIt;
    if (rowsToRemoveUnique.find(anIndex.row()) != rowsToRemoveUnique.end())
      continue;
    rowsToRemoveUnique.insert(anIndex.row());
    rowsToRemove.push_back(anIndex.row());
  }
  rowsToRemove.sort();
  rowsToRemove.reverse();

  // remove the latest row
  if (rowsToRemove.empty() && GetRowCount() > 0)
    rowsToRemove.push_back(GetRowCount() - 1);

  for (int row : rowsToRemove)
  {
    RemoveRows(row, 1);
  }
}

//! Remove rows from the table at the specified index.
//! \param theAt [in] remove row index.
//! \param theRows [in] number of removed rows.
void asiUI_DatumTable::RemoveRows(const int theAt, const int theRows)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theAt >= 0 && theAt < aModel->rowCount(), "invalid index specified" );

  aModel->removeRows(theAt, theRows);

  if ( aModel->signalsBlocked() )
  {
    onRowsRemoved( QModelIndex(), theAt, theRows - theAt - 1 );
  }
}

//! Remove columns from the table at the specified index.
//! \param theAt [in] remove column index.
//! \param theCols [in] number of removed columns.
void asiUI_DatumTable::RemoveColumns(const int theAt, const int theCols)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theAt >= 0 && theAt < aModel->columnCount(), "invalid index specified" );

  aModel->removeColumns(theAt, theCols);

  if ( aModel->signalsBlocked() )
  {
    onColumnsRemoved( QModelIndex(), theAt, theCols - theAt - 1 );
  }
}

//! Return base data view pointer.
//! \return base view pointer.
QAbstractItemView* asiUI_DatumTable::Widget()
{
  return this;
}

// ----------------------------------------------------------------------------
//                         Datum Setters/Getters
// ----------------------------------------------------------------------------

//! Get dictionary string of the default datum editor for the table.
//! The default editor is used in case if there is no cell, column or row
//! editor is specified for the table's item.
//! \return dictionary id of the default datum editor or empty string if
//! no default editor is specified.
QString asiUI_DatumTable::GetTableEditor() const
{
  return m_ViewEditors.GetDefaultEditor();
}

//! Set dictionary id string for the default datum editor of the table.
//! The default editor is used in case if there is no cell, column or row
//! editor is specified for the table's item.
//! \param theDatum [in] the dictionary id of the default editor.
void asiUI_DatumTable::SetTableEditor(const QString& theDatum)
{
  m_ViewEditors.SetDefaultEditor(theDatum);

  // update editors for whole table
  QStandardItemModel* aModel = itemModel();

  int aRowCount = aModel->rowCount();
  int aColumnCount = aModel->columnCount();

  if ( aRowCount > 0 && aColumnCount > 0 )
  {
    updateItems(0, aRowCount - 1, 0, aColumnCount - 1);
  }
}

//! Get dictionary id string of the datum editor assigned for the table's row.
//! The row editor is used in case if there is no cell or column editor is
//! specified for the table's item. It has a higher priority than default
//! table editor.
//! \param theRow [in] row index.
//! \return dictionary id of the row's datum editor or empty string if
//!         no default editor is specified.
QString asiUI_DatumTable::GetRowEditor(const int theRow) const
{
  ASSERT_RAISE( theRow >= 0 && theRow < itemModel()->rowCount(), "index is out of range" );

  return m_ViewEditors.GetRowEditor(theRow);
}

//! Set dictionary id string of the datum editor for the table's row.
//! The row editor is used in case if there is no cell or column editor is
//! specified for the table's item. It has a higher priority than default
//! table editor.
//! \param theRow [in] row index.
//! \param theDatum [in] the dictionary id of the row editor.
void asiUI_DatumTable::SetRowEditor(const int theRow, const QString& theDatum)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theRow >= 0 && theRow < aModel->rowCount(), "index is out of range" );

  m_ViewEditors.SetRowEditor(theRow, theDatum);

  int aCols = aModel->columnCount();
  if ( aCols > 0 )
  {
    updateItems(theRow, theRow, 0, aCols - 1);
  }

  if ( theDatum.isNull() )
  {
    resetHeaders(Qt::Vertical, theRow, theRow);
  }
  else
  {
    updateHeaders(Qt::Vertical, theRow, theRow);
  }
}

//! Get dictionary id string of the datum editor assigned for the table's column.
//! The column editor is used in case if there is no cell editor is
//! specified for the table's item. It has a higher priority than row editor and
//! default table editor.
//! \param theCol [in] column index.
//! \return dictionary id of the column's datum editor or empty string if
//!         no default editor is specified.
QString asiUI_DatumTable::GetColumnEditor(const int theCol) const
{
  ASSERT_RAISE( theCol >= 0 && theCol < itemModel()->columnCount(), "index is out of range" );

  return m_ViewEditors.GetColEditor(theCol);
}

//! Set dictionary id string of the datum editor for the table's column.
//! The column editor is used in case if there is no cell editor is
//! specified for the table's item. It has a higher priority than row editor and
//! default table editor.
//! \param theCol [in] column index.
//! \param theDatum [in] the dictionary id of the column editor.
void asiUI_DatumTable::SetColumnEditor(const int theCol, const QString& theDatum)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theCol >= 0 && theCol < aModel->columnCount(), "index is out of range" );

  m_ViewEditors.SetColEditor(theCol, theDatum);

  int aRowCount = aModel->rowCount();
  if ( aRowCount > 0 )
  {
    updateItems(0, aRowCount - 1, theCol, theCol);
  }

  if ( theDatum.isNull() ) 
  {
    resetHeaders(Qt::Vertical, theCol, theCol);
  }
  else
  {
    updateHeaders(Qt::Horizontal, theCol, theCol);
  }
}

//! Get dictionary id string of the datum editor assigned for the table's cell.
//! The cell editor has a higher priority than column, row or default editor.
//! \param theRow [in] row index.
//! \param theCol [in] column index.
//! \return dictionary id of the cell's datum editor or empty string if
//!         no default editor is specified.
QString asiUI_DatumTable::GetCellEditor(const int theRow, const int theCol) const
{
  ASSERT_RAISE( theRow >= 0 && theRow < itemModel()->rowCount(), "row index is out of range" );
  ASSERT_RAISE( theCol >= 0 && theCol < itemModel()->columnCount(), "column index is out of range" );

  return m_ViewEditors.GetEditor(theRow, theCol);
}

//! Set dictionary id of the datum editor for the specified table's cell.
//! The cell editor has a higher priority than column, row or default editor.
//! \param theRow [in] row index.
//! \param theCol [in] column index.
//! \param theDatum [in] the dictionary id of the cell.
void asiUI_DatumTable::SetCellEditor(const int theRow,
                                      const int theCol,
                                      const QString& theDatum)
{
  QStandardItemModel* aModel = itemModel();

  ASSERT_RAISE( theRow >= 0 && theRow < aModel->rowCount(), "row index is out of range" );
  ASSERT_RAISE( theCol >= 0 && theCol < aModel->columnCount(), "column index is out of range" );

  m_ViewEditors.SetItemEditor(theRow, theCol, theDatum);

  updateItems(theRow, theRow, theCol, theCol);
}

//! Sets custom widget to display in header corner.
//! \param theWidget [in] the widget.
void asiUI_DatumTable::SetHeaderCornerWidget(QWidget* theWidget)
{
  m_CornerWidget = theWidget;
  m_CornerWidget->setParent(this);
}

//! Gets custom widget which is displayed in header corner.
//! \return widget.
QWidget* asiUI_DatumTable::GetHeaderCornerWidget() const
{
  return m_CornerWidget;
}

//! Get header with the specified orientation.
//! \param theOrientation [in] the header's orientation.
QHeaderView* asiUI_DatumTable::header(const Qt::Orientation theOrientation) const
{
  return theOrientation == Qt::Vertical
    ? verticalHeader() 
    : horizontalHeader();
}

//! Take into account advanced selection on "select all" command.
void asiUI_DatumTable::selectAll()
{
  QTableView::selectAll();

  asiUI_DatumViewModel* aModel = itemModel();
  if ( !aModel )
  {
    return;
  }

  asiUI_DatumViewSelectionModel* aSelModel = 
    dynamic_cast<asiUI_DatumViewSelectionModel*>( selectionModel() );

  if ( !aSelModel )
  {
    return;
  }

  if ( aModel->BandRowsCount(Qt::Horizontal) > 0 && aModel->columnCount() > 0 )
  {
    asiUI_HeaderIndexList aBandsSelection;

    for ( int aBand = 0; aBand < aModel->BandRowsCount( Qt::Horizontal ); ++aBand )
    {
      aBandsSelection += asiUI_HeaderIndex::Band(0, aBand);
    }

    aSelModel->select(Qt::Horizontal, aBandsSelection, QItemSelectionModel::Select | QItemSelectionModel::Columns);
  }

  if ( aModel->BandRowsCount(Qt::Vertical) > 0 && aModel->rowCount() > 0 )
  {
    asiUI_HeaderIndexList aBandsSelection;

    for ( int aBand = 0; aBand < aModel->BandRowsCount(Qt::Vertical); ++aBand )
    {
      aBandsSelection += asiUI_HeaderIndex::Band(0, aBand);
    }

    aSelModel->select(Qt::Vertical, aBandsSelection, QItemSelectionModel::Select | QItemSelectionModel::Columns);
  }
}
