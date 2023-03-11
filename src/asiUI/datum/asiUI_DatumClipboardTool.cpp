//-----------------------------------------------------------------------------
// Created on: 01 November 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2012-present, Sergey Slyadnev
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

// asiUI includes
#include <asiUI_DatumClipboardTool.h>
#include <asiUI_ItemRoles.h>

#pragma warning(push, 0)
#include <QApplication>
#include <QClipboard>
#include <QTreeView>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#pragma warning(pop)

// ----------------------------------------------------------------------------
//                              CLIPBOARD TOOL
// ----------------------------------------------------------------------------

//! Clear clipboard buffer.
void asiUI_DatumClipboardTool::ClearClipboard()
{
  QClipboard* aClipboard = QApplication::clipboard();
  if ( aClipboard )
    aClipboard->clear();
}

//! Get clipboard text as a plain string.
//! \return clipboard text or a empty string if
//!         clipboard doesn't contain any text.
QString asiUI_DatumClipboardTool::GetClipboardText()
{
  QClipboard* aClipboard = QApplication::clipboard();
  return aClipboard ? aClipboard->text() : QString();
}

//! Get table data formatted from a tabulation-newline formatted
//! text representation.
//! \return table data.
asiUI_DatumClipboardTableData asiUI_DatumClipboardTool::GetClipboardData()
{
  QClipboard* aClipboard = QApplication::clipboard();
  return asiUI_DatumClipboardTableData(aClipboard ? aClipboard->text() : QString());
}

//! Set clipboard text string.
//! \param theText [in] the text string.
void asiUI_DatumClipboardTool::SetClipboardText(const QString& theText)
{
  QClipboard* aClipboard = QApplication::clipboard();
  if ( aClipboard )
  {
    aClipboard->setText(theText);
  }
}

//! Set clipboard text data from a tabulation-newline table data
//! representation.
//! \param theData [in] the text formatted as a table data.
void asiUI_DatumClipboardTool::SetClipboardData(const asiUI_DatumClipboardTableData& theData)
{
  QClipboard* aClipboard = QApplication::clipboard();
  if ( aClipboard )
  {
    aClipboard->setText( theData.ToTSVString() );
  }
}

// ============================================================================
//                   COPY TO CLIPBOARD METHODS
// ============================================================================

//! Copy selected item data to data chunk buffer.
//! \param theTable [in] the table which contains the copied data.
//! \return return copied data, or empty data container if selection rejected.
asiUI_DatumClipboardTableData
  asiUI_DatumClipboardTool::CopyToData(QAbstractItemView* theView)
{
  QAbstractItemModel* anItemModel = theView->model();
  QItemSelectionModel* aSelModel = theView->selectionModel();
  if ( !anItemModel || !aSelModel )
    return asiUI_DatumClipboardTableData(0,0);

  QModelIndexList aSIndexes; // selection indexes
  QRect aContiguous = GetContiguous( theView, aSelModel->selection(), aSIndexes );
  if ( !aContiguous.isValid() )
    return asiUI_DatumClipboardTableData(0,0);

  int aNbRows = aContiguous.height();
  int aNbCols = aContiguous.width();

  asiUI_DatumClipboardTableData aDataChunk(aNbRows, aNbCols);

  QModelIndexList::ConstIterator it = aSIndexes.constBegin();
  for ( int aRow = 0; aRow < aNbRows; ++aRow )
  {
    for ( int aCol = 0; aCol < aNbCols; ++aCol, ++it )
    {
      const QModelIndex& aIndex = (*it);

      QModelIndexList aCheckList;
      aCheckList << aIndex;

      if ( !IsCopyPossible(theView, aCheckList) )
        continue;

      QVariant aData2Copy;
      aData2Copy = aIndex.data(DatumViewRole_CopyRole);
      if ( !aData2Copy.isValid() )
      {
        aData2Copy = aIndex.data(Qt::DisplayRole);
      }
      // Access data chunk for buffering via Data Adaptor if any
      aDataChunk[aRow][aCol] = aData2Copy;
    }
  }

  return aDataChunk;
}

//! Copy contiguously selected item data to clipboard.
//! This is a convenience method to write into clipboard
//! buffer directly. It should internally invoke method to work
//! with data block, as it is more general one.
//! \param theTable [in] the table which contains the copied data.
//! \return return true if ok, or false if selection rejected.
bool asiUI_DatumClipboardTool::CopyToClipboard(QAbstractItemView* theView)
{
  asiUI_DatumClipboardTableData aDataChunk = CopyToData(theView);
  if ( aDataChunk.IsEmpty() )
    return false;

  SetClipboardData(aDataChunk);
  return true;
}

// ============================================================================
//                   PASTE FROM CLIPBOARD METHODS
// ============================================================================

//! Insert clipboard data into the selected region of the specified view.
//! Warning: no check for contiguous selection!
//! Data is pasted only to the selected items, out-of-bounds data is dropped off.
//! \param theView [in] the view to paste clipboard data.
//! \param theData [in] the inserted data block.
//! \return Boolean flag indicating whether the data has been inserted successfully
//!         or not.
bool asiUI_DatumClipboardTool::PasteFromData(QAbstractItemView* theView,
                                         const asiUI_DatumClipboardTableData& theData)
{
  QAbstractItemModel* anItemModel = theView->model();
  QItemSelectionModel* aSelModel = theView->selectionModel();
  if ( !anItemModel || !aSelModel )
    return false;

  QModelIndexList aSIndexes; // selected indexes
  QRect aContiguous = GetContiguous( theView, aSelModel->selection(), aSIndexes );
  if ( !aContiguous.isValid() )
    return false;

  // check out the maximum data insertion bounds
  int aNbRows = qMin( theData.GetRowCount(), aContiguous.height() );
  int aNbCols = qMin( theData.GetColumnCount(), aContiguous.width() );

  // truncate indexes falling out of clipboard buffer range
  QModelIndexList::Iterator it = aSIndexes.begin();
  for ( int aRow = 0; aRow < aContiguous.height(); ++aRow )
  {
    for ( int aCol = 0; aCol < aContiguous.width(); ++aCol )
    {
      if ( aRow >= aNbRows || aCol >= aNbCols )
        aSIndexes.erase(it);
      else
        ++it;
    }
  }

  // paste data as is...
  it = aSIndexes.begin();
  for ( int aRow = 0; aRow < aNbRows; ++aRow )
  {
    for ( int aCol = 0; aCol < aNbCols; ++aCol, ++it )
    {
      const QModelIndex& aIndex = (*it);

      QModelIndexList aCheckList;
      aCheckList << aIndex;

      if ( !IsPastePossible(theView, aCheckList) )
        continue;

      // If Data Adaptor is accessible, then use it to pull the data 
      // from buffer to table view
      QVariant aDataChunk = theData.GetData(aRow, aCol);
      anItemModel->setData(aIndex, aDataChunk, Qt::DisplayRole );
    }
  }

  return true;
}

//! Insert clipboard data into the selected region of the specified view.
//! The selection is required to be contiguous.
//! Data is pasted only to the selected items, out-of-bounds data is dropped off.
//! This is a convenience method to paste from clipboard
//! buffer directly. It should internally invoke method to work
//! with data block, as it is more general one.
//! \param theView [in] the view to paste clipboard data.
//! \return Boolean flag indicating whether the data has been inserted successfully
//!         or not.
bool asiUI_DatumClipboardTool::PasteFromClipboard(QAbstractItemView* theView)
{
  return PasteFromData(theView, GetClipboardData());
}

// ============================================================================
//                   PASTE REGION SIZE ESTIMATION METHODS
// ============================================================================

//! Returns paste rectangle in accordance to selected rect with
//! Excel-like insertion rules.
//! \param theSiblingSelection [in] table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! absoulte row and column indexes.
//! \return formatted rectangle.
QRect asiUI_DatumClipboardTool::GetPasteRuleRegion(const QRect& theSiblingSelection)
{
  asiUI_DatumClipboardTableData aDataChunk = GetClipboardData();

  // region required for populating the data
  QRect aPasteRegion( 
    theSiblingSelection.topLeft() ,
    QSize( aDataChunk.GetColumnCount(),
           aDataChunk.GetRowCount() ) );

  // truncate paste region in compliance with Excel rules
  // selected several elements in single column -->
  // populate the rows corresponding to the selected items
  // other rows are not populated
  if (theSiblingSelection.width() == 1 
   && theSiblingSelection.height() > 1)
  {
    aPasteRegion.setHeight( 
      qMin( aPasteRegion.height(), theSiblingSelection.height() ) );
  }
  // selected several elements in single row -->
  // populate the columns under the selected items
  // other columns are not populated
  else if (theSiblingSelection.height() == 1 
        && theSiblingSelection.width() > 1)
  {
    aPasteRegion.setWidth( 
      qMin( aPasteRegion.width(), theSiblingSelection.width() ) );
  }
  // multiple selection in rows and columns -->
  // populate the selected region only
  else if (theSiblingSelection.width() > 1 
        && theSiblingSelection.height() > 1)
  {
    aPasteRegion.setHeight( 
      qMin( aPasteRegion.height(), theSiblingSelection.height() ) );
    aPasteRegion.setWidth( 
      qMin( aPasteRegion.width(), theSiblingSelection.width() ) );
  }

  return aPasteRegion;
}

//! Return arranged list of indexes for the specified table-layout
//! rectangle. If indexes are missing - the number of missing rows
//! and columns to add returned through arguments.
//! \param theView [in] the items view.
//! \param theSiblingSelection [in] table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! \param theRowsMissing [out] number of missing rows in view.
//! \param theColsMissing [out] number of missing columns in view.
//! absoulte row and column indexes.
//! \return list of found hierarchically-arranged indexes that
//! correspond to table-like selection in the specified view
QModelIndexList asiUI_DatumClipboardTool::GetIndexesFromRect(
                                  QAbstractItemView* theView,
                                  const QRect& theSiblingSelection,
                                  int& theRowsMissing,
                                  int& theColsMissing)
{
  if ( theSiblingSelection.isEmpty() )
    return QModelIndexList();

  if ( QTableView* aView = qobject_cast<QTableView*>( theView ) )
    return indexesOfRegion(aView, theSiblingSelection, theRowsMissing, theColsMissing);
  else if ( QTreeView* aView2 = qobject_cast<QTreeView*>( theView ) )
    return indexesOfRegion(aView2, theSiblingSelection, theRowsMissing, theColsMissing);

  return QModelIndexList(); // unknown? - up to you to extend
}

// ============================================================================
//                 COPY / PASTE RESTRICTION RULES
// ============================================================================

//! Check that copy to clipboard is possible for the model index range.
//! Warning: no check for contiguous selection!
//! The method applies restriction rules provided by clipboard data adaptor.
//! \param theView [in] the view to copy data from.
//! \param theIndexes [in] the indexes to check.
//! \return boolean flag indicating whether the copy is possible or not.
bool asiUI_DatumClipboardTool::IsCopyPossible(
                                  QAbstractItemView* /*theView*/,
                                  const QModelIndexList& theIndexes)
{
  // check restriction rules
  const int aCopyMask = (Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  QModelIndexList::ConstIterator anIt = theIndexes.constBegin();

  for ( ; anIt != theIndexes.constEnd(); anIt++ )
  {
    const QModelIndex& anIndex = *anIt;
    if ( (anIndex.flags() & aCopyMask) != aCopyMask )
      return false;
  }

  return (theIndexes.size() > 0);
}

//! Check that view's index range is allowed for data insertion.
//! Warning: no check for contiguous selection!
//! The method applies restriction rules provided by clipboard data adaptor.
//! \param theView [in] the view to paste data to check general rules.
//! \param theIndexes [in] the indexes to check.
//! \return boolean flag indicating whether the paste is possible or not.
bool asiUI_DatumClipboardTool::IsPastePossible(
                                  QAbstractItemView* theView,
                                  const QModelIndexList& theIndexes)
{
  if ( theView->editTriggers() == QAbstractItemView::NoEditTriggers )
  {
    return false;
  }

  // check restriction rules
  const int aPasteMask = (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

  QModelIndexList::ConstIterator anIt = theIndexes.constBegin();

  for ( ; anIt != theIndexes.constEnd(); anIt++ )
  {
    const QModelIndex& anIndex = *anIt;
    if ( (anIndex.flags() & aPasteMask) != aPasteMask )
      return false;
  }

  return (theIndexes.size() > 0);
}

//! Check for contiguous selection in the know types of views.
//! These are:
//! - QTableView;
//! - QTreeView.
//! \param theView [in] the view to check.
//! \param theItemRange [in] the item selection range.
//! \param theSorted [out] list of items sorted contiguously
//! in rows and columns
//! \return table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! absoulte row and column indexes.
QRect asiUI_DatumClipboardTool::GetContiguous(QAbstractItemView* theView,
                                          const QItemSelection& theItemRange,
                                          QModelIndexList& theOrdered)
{
  if ( theItemRange.isEmpty() )
    return QRect();

  if ( QTableView* aView = qobject_cast<QTableView*>( theView ) )
    return contiguousRect(aView, theItemRange, theOrdered);
  else if ( QTreeView* aView2 = qobject_cast<QTreeView*>( theView ) )
    return contiguousRect(aView2, theItemRange, theOrdered);

  return QRect(); // unknown? - up to you to extend
}

//! Get rectangular selection area composed from the current item selection.
//! \param theView [in] the view.
//! \return rectangular selection area (if any).
asiUI_DatumClipboardTool::ViewIndexRectangle
  asiUI_DatumClipboardTool::GetSelectionRectangle(QAbstractItemView* theView)
{
  QItemSelectionModel* aModel = theView->selectionModel();
  if ( !aModel )
  {
    // empty selection
    return ViewIndexRectangle(0, 0, 0, 0);
  }

  QItemSelection aSelection = aModel->selection();
  if ( aSelection.isEmpty() )
  {
    // empty selection
    return ViewIndexRectangle(0, 0, 0, 0);
  }

  QModelIndexList aIndexes;

  QRect aRect;
  if ( QTableView* aView = qobject_cast<QTableView*>( theView ) )
  {
    aRect = contiguousRect(aView, aModel->selection(), aIndexes);
  }
  else if ( QTreeView* aView2 = qobject_cast<QTreeView*>( theView ) )
  {
    aRect = contiguousRect(aView2, aModel->selection(), aIndexes);
  }
  else
  {
    // invalid selection
    return ViewIndexRectangle();
  }

  if ( !aRect.isValid() )
  {
    // invalid selection
    return ViewIndexRectangle();
  }

  int aLeft      = aRect.left();
  int aTop       = aRect.top();
  int aHorLength = aRect.width();
  int aVerLength = aRect.height();

  ViewIndexRectangle aViewSelection(aLeft, aTop, aHorLength, aVerLength);

  QModelIndexList::Iterator aIndexIt = aIndexes.begin();

  for ( int aRow = 0; aRow < aRect.height(); ++aRow )
  {
    for ( int aCol = 0; aCol < aRect.width(); ++aCol, ++aIndexIt )
    {
      aViewSelection.SetIndexAt(aCol + aLeft, aRow + aTop, *aIndexIt);
    }
  }

  return aViewSelection;
}

//! Get rectangular selection area composed from the current header selection.
//! The header orientation is taken into account to return correct layout,
//! which corresponds to layout of view items.
//! The top rectangle bounds for horizontal header selection and left rectangle
//! bounds are not relative to the view, while {left, right} / {top, bottom}
//! correspond to section indexes in view. The purpose of the bounds is to
//! check whether the view rectangle matches to header ones by its column/row
//! position. The non-relative indexes are here for convenience of composing
//! clipboard buffer - these indexes can not be matched anyway.
//! \param theView [in] the header view.
//! \param theTitles [in] flag specifies whether to get titles selection.
//! \param theBands [in] flag specified whether to get bands selection.
//! \return rectangular selection area (if any).
asiUI_DatumClipboardTool::HeaderIndexRectangle
  asiUI_DatumClipboardTool::GetSelectionRectangle(asiUI_HeaderView* theView,
                                              const bool theTitles,
                                              const bool theBands)
{
  asiUI_HeaderViewSelectionApi* aSelectionAPI = 
    dynamic_cast<asiUI_HeaderViewSelectionApi*>( theView->selectionModel() );

  if ( !aSelectionAPI )
  {
    // empty selection
    return HeaderIndexRectangle(0, 0, 0, 0);
  }

  HeaderIndexRectangle aTitlesSelection(0, 0, 0, 0);
  HeaderIndexRectangle aBandsSelection(0, 0, 0, 0);

  bool isColumnHeader = theView->orientation() == Qt::Horizontal;

  /* ======================================================================
   *                 compose hierarchy titles header selection
   * ====================================================================== */

  if ( theTitles )
  {
    asiUI_HeaderIndexList aHeaderSelection =
      aSelectionAPI->HeaderSelection( theView->orientation(), true );

    // number of hierarchy levels
    int aLevels = 0;

    // collect and sort section ids under titles selection
    QSet<int> aSelectedIds;

    // remember title indexes and associated their level in hierarchy
    QMap<asiUI_HeaderIndex, int> aMapByLevel;

    // collect header indexes and remember covered sections
    asiUI_HeaderIndexList::Iterator aTitleIt = aHeaderSelection.begin();
    for ( ; aTitleIt != aHeaderSelection.end(); ++aTitleIt )
    {
      asiUI_HeaderIndex& anIndex = *aTitleIt;

      if ( !anIndex.IsSection() )
      {
        continue;
      }

      for ( int aSectionIt = anIndex.GetFirstSection(); aSectionIt <= anIndex.GetLastSection(); ++aSectionIt )
      {
        aSelectedIds += aSectionIt;
      }

      asiUI_HeaderIndexList aHierarchy =
        topIndexes( theView->model(), theView->orientation(), anIndex.GetFirstSection() );

      int aHierachyLevel = aHierarchy.indexOf(anIndex);
      if ( aHierachyLevel < 0 )
      {
        continue; // incorrect header selection
      }

      aLevels = qMax(aHierachyLevel, aLevels);

      aMapByLevel.insert(anIndex, aHierachyLevel);
    }

    if ( !aSelectedIds.isEmpty() )
    {
      int aSectionMin = INT_MAX;
      int aSectionMax = INT_MIN;

      // check for contiguous
      QSet<int>::Iterator aSectionIdsIt = aSelectedIds.begin();

      for ( ; aSectionIdsIt != aSelectedIds.end(); ++aSectionIdsIt )
      {
        aSectionMin = qMin(*aSectionIdsIt, aSectionMin);
        aSectionMax = qMax(*aSectionIdsIt, aSectionMax);
      }

      int aSectionsSpan = aSectionMax - aSectionMin + 1;

      if ( aSelectedIds.size() != aSectionsSpan )
      {
        return HeaderIndexRectangle();
      }

      // compose the header selection rectangle
      int aLeft = isColumnHeader ? aSectionMin : 0;
      int aTop  = isColumnHeader ? 0 : aSectionMin;
      int aHorLength = isColumnHeader ? aSectionsSpan : aLevels + 1;
      int aVerLength = isColumnHeader ? aLevels + 1 : aSectionsSpan;

      aTitlesSelection = HeaderIndexRectangle(aLeft, aTop, aHorLength, aVerLength);

      QMap<asiUI_HeaderIndex, int>::Iterator anIt = aMapByLevel.begin();

      for ( ; anIt != aMapByLevel.end(); ++anIt )
      {
        const asiUI_HeaderIndex& anIndex = anIt.key();

        int aHierarchyLevel = anIt.value();
        for ( int aSectionIt = anIndex.GetFirstSection(); aSectionIt <= anIndex.GetLastSection(); ++aSectionIt )
        {
          int aHorPos = isColumnHeader ? aSectionIt : aHorLength - aHierarchyLevel - 1;
          int aVerPos = isColumnHeader ? aVerLength - aHierarchyLevel - 1 : aSectionIt;
          aTitlesSelection.SetIndexAt( aHorPos, aVerPos, anIndex );
        }
      }
    }
  }

  /* =================================================================
   *                 compose bands header selection
   * ================================================================= */

  if ( theBands )
  {
    asiUI_HeaderIndexList aHeaderSelection =
      aSelectionAPI->HeaderSelection( theView->orientation(), false );

    // collect and sort section ids under bands selection
    QSet<int> aSectionIds;
    QSet<int> aBandRowsIds;

    // collect header bands indexes under sections
    QMap<int, asiUI_HeaderIndexList> aMapBySections;

    asiUI_HeaderIndexList::Iterator aBandsIt = aHeaderSelection.begin();
    for ( ; aBandsIt != aHeaderSelection.end(); ++aBandsIt )
    {
      asiUI_HeaderIndex& anIndex = *aBandsIt;
      if ( !anIndex.IsBand() )
      {
        continue;
      }

      int aSectionId = anIndex.GetSection();
      aSectionIds += aSectionId;
      aBandRowsIds += anIndex.GetBandLine();
      aMapBySections[aSectionId] += anIndex;
    }

    if ( !aSectionIds.isEmpty() )
    {
      // step over sorted sections one-by-one and check for gaps
      // additionally check that number of selected band rows in every
      // section is constant
      int aSectionMin = INT_MAX;
      int aSectionMax = INT_MIN;

      QSet<int>::Iterator aSectionIdsIt = aSectionIds.begin();

      for ( ; aSectionIdsIt != aSectionIds.end(); ++aSectionIdsIt )
      {
        aSectionMin = qMin(*aSectionIdsIt, aSectionMin);
        aSectionMax = qMax(*aSectionIdsIt, aSectionMax);

        if ( aMapBySections[*aSectionIdsIt].length() != aBandRowsIds.size() )
        {
          return HeaderIndexRectangle();
        }
      }

      int aSectionsSpan = aSectionMax - aSectionMin + 1;

      if ( aSectionIds.size() != aSectionsSpan )
      {
        return HeaderIndexRectangle();
      }

      // step over sorted band rows one-by-one and check for gaps
      int aBandMin = INT_MAX;
      int aBandMax = INT_MIN;

      QSet<int>::Iterator aBandIdsIt = aBandRowsIds.begin();

      for ( ; aBandIdsIt != aBandRowsIds.end(); ++aBandIdsIt )
      {
        aBandMin = qMin(*aBandIdsIt, aBandMin);
        aBandMax = qMax(*aBandIdsIt, aBandMax);
      }

      int aBandsSpan = aBandMax - aBandMin + 1;

      if ( aBandRowsIds.size() != aBandsSpan )
      {
        return HeaderIndexRectangle();
      }

      int aTopBand = INT_MAX;

      asiUI_HeaderIndexList& aLastSections = aMapBySections[aSectionMax];
      asiUI_HeaderIndexList::Iterator aSeekTopIt = aLastSections.begin();
      for ( ; aSeekTopIt != aLastSections.end(); ++aSeekTopIt )
      {
        aTopBand = qMin( aTopBand, (*aSeekTopIt).GetBandLine() );
      }

      // compose the header selection rectangle
      int aLeft = isColumnHeader ? aSectionMin : 0;
      int aTop  = isColumnHeader ? 0 : aSectionMin;
      int aHorLength = isColumnHeader ? aSectionsSpan : aBandsSpan;
      int aVerLength = isColumnHeader ? aBandsSpan : aSectionsSpan;

      aBandsSelection = HeaderIndexRectangle(aLeft, aTop, aHorLength, aVerLength);

      QMap<int, asiUI_HeaderIndexList>::Iterator anIt = aMapBySections.begin();

      for ( ; anIt != aMapBySections.end(); ++anIt )
      {
        int aSectionId = anIt.key();
        asiUI_HeaderIndexList& anIndexes = anIt.value();
        asiUI_HeaderIndexList::Iterator anIndexesIt = anIndexes.begin();
        for ( ; anIndexesIt != anIndexes.end(); ++anIndexesIt )
        {
          asiUI_HeaderIndex& anIndex = *anIndexesIt;
          int aHorPos = isColumnHeader ? aSectionId : anIndex.GetBandLine() - aTopBand;
          int aVerPos = isColumnHeader ? anIndex.GetBandLine() - aTopBand : aSectionId;
          aBandsSelection.SetIndexAt( aHorPos, aVerPos, anIndex );
        }
      }
    }
  }

  /* ======================================================================
   *                 compose final selection
   * ====================================================================== */

  if ( aBandsSelection.IsEmpty() && !aTitlesSelection.IsEmpty() )
  {
    return aTitlesSelection;
  }

  if ( !aBandsSelection.IsEmpty() && aTitlesSelection.IsEmpty() )
  {
    return aBandsSelection;
  }

  QRect aBandsR = aBandsSelection.GetRectangle();
  QRect aTitlesR = aTitlesSelection.GetRectangle();

  // check matching
  if ( isColumnHeader )
  {
    if ( aBandsR.left() != aTitlesR.left() || aBandsR.right() != aTitlesR.right() )
    {
      return HeaderIndexRectangle();
    }
  }
  else
  {
    if ( aBandsR.top() != aTitlesR.top() || aBandsR.bottom() != aTitlesR.bottom() )
    {
      return HeaderIndexRectangle();
    }
  }

  int aLeft = aBandsR.left();
  int aTop  = aBandsR.top();

  int aHorLength = isColumnHeader ? aBandsR.width() : aBandsR.width() + aTitlesR.width();
  int aVerLength = isColumnHeader ? aBandsR.height() + aTitlesR.height() : aBandsR.height();

  HeaderIndexRectangle aCompoundSelection(aLeft, aTop, aHorLength, aVerLength);

  // merge rectangles
  if ( isColumnHeader )
  {
    for ( int aColIt = 0; aColIt < aHorLength; ++aColIt )
    {
      int aHorPos = aColIt + aLeft;

      for ( int aRowIt = 0; aRowIt < aTitlesR.height(); ++aRowIt )
      {
        asiUI_HeaderIndex anIndexAt =
          aTitlesSelection.GetIndexAt(aHorPos, aRowIt);

        aCompoundSelection.SetIndexAt(aHorPos, aRowIt, anIndexAt);
      }

      for ( int aRowIt = 0; aRowIt < aBandsR.height(); ++aRowIt )
      {
        asiUI_HeaderIndex anIndexAt =
          aBandsSelection.GetIndexAt(aHorPos, aRowIt);

        aCompoundSelection.SetIndexAt(aHorPos, aRowIt + aTitlesR.height(), anIndexAt);
      }
    }
  }
  else
  {
    for ( int aRowIt = 0; aRowIt < aVerLength; ++aRowIt )
    {
      int aVerPos = aRowIt + aTop;

      for ( int aColIt = 0; aColIt < aTitlesR.width(); ++aColIt )
      {
        asiUI_HeaderIndex anIndexAt =
          aTitlesSelection.GetIndexAt(aColIt, aVerPos);

        aCompoundSelection.SetIndexAt(aColIt, aVerPos, anIndexAt);
      }

      for ( int aColIt = 0; aColIt < aBandsR.width(); ++aColIt )
      {
        asiUI_HeaderIndex anIndexAt =
          aBandsSelection.GetIndexAt(aColIt, aVerPos);

        aCompoundSelection.SetIndexAt(aColIt + aTitlesR.width(), aVerPos, anIndexAt);
      }
    }
  }

  return aCompoundSelection;
}

//! Check for contigous item selection range in table view.
//! \param theView [in] the table view.
//! \param theItemRange [in] the item selection range.
//! \param theOrdered [out] list of items sorted contiguously
//! in rows and columns
//! \return table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! absoulte row and column indexes.
QRect asiUI_DatumClipboardTool::contiguousRect(const QTableView* /*theView*/,
                                           const QItemSelection& theItemRange,
                                           QModelIndexList& theOrdered)
{
  // find top-left and bottom-right indexes.
  QModelIndexList aIndexes = theItemRange.indexes();
  QModelIndexList::ConstIterator anIt = aIndexes.constBegin();
  if ( aIndexes.isEmpty() )
    return QRect();

  QModelIndex aFirstIdx = aIndexes.first();
  int aMinRow = aFirstIdx.row();
  int aMaxRow = aFirstIdx.row();
  int aMinCol = aFirstIdx.column();
  int aMaxCol = aFirstIdx.column();

  for ( ; anIt != aIndexes.constEnd(); anIt++ )
  {
    const QModelIndex& anIndex = *anIt;
    aMinRow = qMin(anIndex.row(), aMinRow);
    aMinCol = qMin(anIndex.column(), aMinCol);
    aMaxRow = qMax(anIndex.row(), aMaxRow);
    aMaxCol = qMax(anIndex.column(), aMaxCol);
  }

  // compare estimated and actual number
  // of indexes for contiguous selection
  int aEstimatedRowNb = aMaxRow - aMinRow + 1;
  int aEstimatedColNb = aMaxCol - aMinCol + 1;
  int aEstimatedNb = aEstimatedRowNb * aEstimatedColNb;

  if ( aEstimatedNb != aIndexes.count() )
    return QRect();

  theOrdered = aIndexes;
  qSort(theOrdered);

  return QRect( QPoint(aMinCol, aMinRow), QPoint(aMaxCol, aMaxRow) );
}

// internal model index sort methods
namespace
{
  bool isLessInTree(const QModelIndex& theFirst, const QModelIndex& theSecond)
  {
    QVector<QModelIndex> aSeqOfFirstParents;
    QVector<QModelIndex> aSeqOfSecondParents;
    QModelIndex aParentIt;

    QModelIndex aFirstParent  = theFirst.parent();
    QModelIndex aSecondParent = theSecond.parent();

    aSeqOfFirstParents.prepend(theFirst);
    aSeqOfSecondParents.prepend(theSecond);

    // separate parentship
    if ( aFirstParent != aSecondParent )
    {
      while ( aFirstParent.isValid() )
      {
        aSeqOfFirstParents.prepend(aFirstParent);
        aFirstParent = aFirstParent.parent();
      }

      while ( aSecondParent.isValid() )
      {
        aSeqOfSecondParents.prepend(aSecondParent);
        aSecondParent = aSecondParent.parent();
      }
    }

    // items on same level
    if ( aSeqOfFirstParents.size() == aSeqOfSecondParents.size() )
    {
      // row is priority
      if ( theFirst.row() != theSecond.row() )
        return theFirst.row() < theSecond.row();

      // column is
      return theFirst.column() < theSecond.column();
    }

    int aSizeFirst  = aSeqOfFirstParents.size();
    int aSizeSecond = aSeqOfSecondParents.size();

    // items on different levels
    int aMinLevel = qMin( aSizeFirst - 1, aSizeSecond - 1 );

    QModelIndex aFirstAtLevel = aSeqOfFirstParents[aMinLevel];
    QModelIndex aSecondAtLevel = aSeqOfSecondParents[aMinLevel];

    if ( aFirstAtLevel.row() == aSecondAtLevel.row() )
      return (aSizeFirst < aSizeSecond);

    return aFirstAtLevel.row() < aSecondAtLevel.row();
  }
}

//! Check for contiguous item selection range in tree view.
//! \param theView [in] the tree view.
//! \param theItemRange [in] the item selection range.
//! \param theOrdered [out] list of items sorted contiguously
//! in rows and columns
//! \return table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! absoulte row and column indexes.
QRect asiUI_DatumClipboardTool::contiguousRect(const QTreeView* theView,
                                           const QItemSelection& theItemRange,
                                           QModelIndexList& theOrdered)
{
  theOrdered.clear();

  // find top-left and bottom-right indexes.
  QModelIndexList aIndexes = theItemRange.indexes();

  // avoid work with empty collection
  if ( aIndexes.empty() )
    return QRect();

  QModelIndexList::ConstIterator anIt = aIndexes.constBegin();
  QModelIndex aTopLeft = aIndexes.first();
  QModelIndex aBotRight = aIndexes.last();

  for ( ; anIt != aIndexes.constEnd(); anIt++ )
  {
    const QModelIndex& anIndex = *anIt;
    if ( isLessInTree(anIndex, aTopLeft) )
      aTopLeft = anIndex;
    if ( isLessInTree(aBotRight, anIndex) )
      aBotRight = anIndex;
  }

  // need here to estimate number of selection indexes
  int aEstimatedRowNb = 1;
  int aEstimatedColNb = aBotRight.column() - aTopLeft.column() + 1;

  QModelIndex aIndexBelow = aTopLeft;
  QModelIndex aSeekIndex = aBotRight.sibling( aBotRight.row(), 0 );

  // add to ordered index list
  int aTopRow = aTopLeft.row();
  int aTopCol = aTopLeft.column();
  int aColIt;
  for ( aColIt = aTopCol; aColIt < aTopCol + aEstimatedColNb; aColIt++ )
    theOrdered << aTopLeft.sibling(aTopRow, aColIt);

  if ( aTopLeft.parent() != aBotRight.parent() || aTopLeft.row() != aBotRight.row() )
  {
    while ( aIndexBelow != aSeekIndex )
    {
      aIndexBelow = theView->indexBelow(aIndexBelow);

      // add to ordered index list
      int aCurrRow = aIndexBelow.row();
      for ( aColIt = aTopCol; aColIt < aTopCol + aEstimatedColNb; aColIt++ )
        theOrdered << aIndexBelow.sibling(aCurrRow, aColIt);

      aEstimatedRowNb++;

      if ( aEstimatedRowNb > aIndexes.count() )
      {
        theOrdered.clear();
        return QRect(); // stop - further check is ridiculous
      }
    }
  }

  // compare estimated and actual number
  // of indexes for contiguous selection
  int aEstimatedNb = aEstimatedRowNb * aEstimatedColNb;
  if ( aEstimatedNb != aIndexes.count() )
  {
    theOrdered.clear();
    return QRect();
  }

  // find row number for first index
  int aRow = -1;
  int aCol = aTopLeft.column();
  QModelIndex aIndexIterator = aTopLeft;
  for ( ; aIndexIterator.isValid(); aRow++ )
    aIndexIterator = theView->indexAbove(aIndexIterator); // iterator --> next

  return QRect( QPoint(aCol, aRow), QSize(aEstimatedColNb, aEstimatedRowNb) );
}

//! Return arranged list of indexes for the specified table-layout
//! rectangle. If indexes are missing - the number of missing rows
//! and columns to add returned through arguments.
//! \param theView [in] the items view.
//! \param theSiblingSelection [in] table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! \param theRowsMissing [out] number of missing rows in view.
//! \param theColsMissing [out] number of missing columns in view.
//! absoulte row and column indexes.
//! \return list of found hierarchically-arranged indexes that
//! correspond to table-like selection in the specified view
QModelIndexList asiUI_DatumClipboardTool::indexesOfRegion(
                                  const QTableView* theView,
                                  const QRect& theSiblingSelection,
                                  int& theRowsMissing,
                                  int& theColsMissing)
{
  // access item model
  QAbstractItemModel* aItemModel = theView->model();

  // get number of rows and columns in the view
  int aRowNb = aItemModel->rowCount();
  int aColNb = aItemModel->columnCount();

  // truncate selection rectangle to views extent
  QRect aTableExtent( QPoint(0, 0), QSize(aColNb, aRowNb) );
  QRect aInnerRect = aTableExtent.intersected(theSiblingSelection);

  theColsMissing = theSiblingSelection.width() - aInnerRect.width();
  theRowsMissing = theSiblingSelection.height() - aInnerRect.height();

  // arrange model indexes in the area
  QModelIndexList aIndexes;
  for ( int aRowIt = aInnerRect.top(); aRowIt < aInnerRect.bottom(); aRowIt++ )
  {
    for ( int aColIt = aInnerRect.left(); aColIt < aInnerRect.right(); aColIt++ )
    {
      aIndexes << aItemModel->index(aRowIt, aColIt);
    }
  }

  return aIndexes;
}

//! Return arranged list of indexes for the specified table-layout
//! rectangle. If indexes are missing - the number of missing rows
//! and columns to add returned through arguments.
//! \param theView [in] the items view.
//! \param theSiblingSelection [in] table-layout single level selection
//! where items regardles of their level in subtrees represented with
//! \param theRowsMissing [out] number of missing rows in view.
//! \param theColsMissing [out] number of missing columns in view.
//! absoulte row and column indexes.
//! \return list of found hierarchically-arranged indexes that
//! correspond to table-like selection in the specified view
QModelIndexList asiUI_DatumClipboardTool::indexesOfRegion(
                                  const QTreeView* theView,
                                  const QRect& theSiblingSelection,
                                  int& theRowsMissing,
                                  int& theColsMissing)
{
  // access item model
  QAbstractItemModel* aItemModel = theView->model();

  // empty model case
  if ( aItemModel->rowCount() < 1 )
  {
    theRowsMissing = theSiblingSelection.bottom();
    theColsMissing = theSiblingSelection.right();
    return QModelIndexList();
  }

  // get number of rows and columns in the view
  int aRowNb = 0;
  int aColNb = aItemModel->columnCount();

  // iterate over viewed indexes to get visible row count
  QModelIndexList aIndexes;
  QModelIndex aIndexIterator = aItemModel->index(0, 0);
  for ( ; aIndexIterator.isValid(); aRowNb++ )
  {
    for ( int aColIt = 0; aColIt < aColNb; aColIt++ )
      aIndexes << aIndexIterator.sibling( aIndexIterator.row(), aColIt );

    aIndexIterator = theView->indexBelow(aIndexIterator); // iterator --> next
  }

  // truncate selection rectangle to views extent
  QRect aTableExtent( QPoint(0, 0), QSize(aColNb, aRowNb) );
  QRect aInnerRect = aTableExtent.intersected(theSiblingSelection);

  theColsMissing = theSiblingSelection.width() - aInnerRect.width();
  theRowsMissing = theSiblingSelection.height() - aInnerRect.height();

  // extract arranged indexes
  int aExtractFrom = ( aInnerRect.top() * aColNb + aInnerRect.left() );
  int aExtractTo   = ( aInnerRect.bottom() * aColNb + aInnerRect.right() );
  int aExtractNb   = (aExtractTo - aExtractFrom) + 1;
  aIndexes = aIndexes.mid(aExtractFrom, aExtractNb);

  // extract particular indexes
  QModelIndexList::Iterator aExtractIt = aIndexes.begin();
  QPoint aCurrIdx( aInnerRect.topLeft() );
  while( aCurrIdx != aInnerRect.bottomRight() )
  {
    if ( !aInnerRect.contains( aCurrIdx ) )
      aExtractIt = aIndexes.erase( aExtractIt );
    else
      aExtractIt++;

    aCurrIdx.rx()++;
    if ( aCurrIdx.x() > aInnerRect.right() )
    {
      aCurrIdx.ry()++;
      aCurrIdx.rx() = 0;
    }
  }

  return aIndexes;
}

//! Return all of the top level indexes associated with the passed section.
//! \param theModel [in] the header model.
//! \param theSection [in] the section to explore.
//! \param theHeader [in] the header.
asiUI_HeaderIndexList asiUI_DatumClipboardTool::topIndexes(const QAbstractItemModel* theModel,
                                                        const Qt::Orientation theHeader,
                                                        const int theSection)
{
  const asiUI_HeaderViewDataApi* aHeaderModelApi =
    dynamic_cast<const asiUI_HeaderViewDataApi*>(theModel);

  if ( !aHeaderModelApi )
  {
    return asiUI_HeaderIndexList();
  }

  asiUI_HeaderIndexList aSections;
  asiUI_HeaderIndex anIndex = asiUI_HeaderIndex::UnitarySection(theSection);
  while( anIndex.IsValid() )
  {
    aSections.append(anIndex);
    anIndex = aHeaderModelApi->ParentSection(theHeader, anIndex);
  }

  return aSections;
}

// ----------------------------------------------------------------------------
//                    Rectangular area (view indexes)
// ----------------------------------------------------------------------------


//! Construct null rectangular area to notify that nothing valid rectangle exists.
asiUI_DatumClipboardTool::ViewIndexRectangle::ViewIndexRectangle()
: m_isValid(false),
  m_left(0),
  m_top(0),
  m_horLength(0),
  m_verLength(0)
{
}

//! Construct the contiguous rectangular area at the specified view location.
//! \param theLeft [in] the left bounds.
//! \param theTop [in] the top bounds.
//! \param theHorLength [in] the horizontal length.
//! \param theVerLength [in] the vertical length bounds.
asiUI_DatumClipboardTool::ViewIndexRectangle::ViewIndexRectangle(const int theLeft,
                                                             const int theTop,
                                                             const int theHorLength,
                                                             const int theVerLength)
: m_isValid(true),
  m_left(theLeft),
  m_top(theTop),
  m_horLength(theHorLength),
  m_verLength(theVerLength)
{
}

//! Get view item index located at the specified position of the rectangular area.
//! \param theHorPos [in] the horizontal position.
//! \param theVerPos [in] the vertical position.
//! \return the contained model index.
QModelIndex asiUI_DatumClipboardTool::ViewIndexRectangle::GetIndexAt(const int theHorPos,
                                                                 const int theVerPos) const
{
  ASSERT_RAISE( theHorPos >= m_left && theHorPos < m_left + m_horLength, "invalid pos" );
  ASSERT_RAISE( theVerPos >= m_top  && theVerPos < m_top + m_verLength,  "invalid pos" );

  return m_indexes.value( QPair<int, int>(theHorPos, theVerPos), QModelIndex() );
}

//! Specify rectangular area item at the specified position.
//! \param theHorPos [in] the horizontal position.
//! \param theVerPos [in] the vertical position.
//! \param theIndex [in] the index to specify.
void asiUI_DatumClipboardTool::ViewIndexRectangle::SetIndexAt(const int theHorPos,
                                                          const int theVerPos,
                                                          const QModelIndex& theIndex)
{
  ASSERT_RAISE( theHorPos >= m_left && theHorPos < m_left + m_horLength, "invalid pos" );
  ASSERT_RAISE( theVerPos >= m_top  && theVerPos < m_top + m_verLength,  "invalid pos" );

  m_indexes[QPair<int, int>(theHorPos, theVerPos)] = theIndex;
}


// ----------------------------------------------------------------------------
//                    Rectangular area (header indexes)
// ----------------------------------------------------------------------------


//! Construct null rectangular area to notify that nothing valid rectangle exists.
asiUI_DatumClipboardTool::HeaderIndexRectangle::HeaderIndexRectangle()
: m_isValid(false),
  m_left(0),
  m_top(0),
  m_horLength(0),
  m_verLength(0)
{
}

//! Construct the contiguous rectangular area at the specified view location.
//! \param theLeft [in] the left bounds.
//! \param theTop [in] the top bounds.
//! \param theHorLength [in] the horizontal length.
//! \param theVerLength [in] the vertical length bounds.
asiUI_DatumClipboardTool::HeaderIndexRectangle::HeaderIndexRectangle(const int theLeft,
                                                                 const int theTop,
                                                                 const int theHorLength,
                                                                 const int theVerLength)
: m_isValid(true),
  m_left(theLeft),
  m_top(theTop),
  m_horLength(theHorLength),
  m_verLength(theVerLength)
{
}

//! Get view item index located at the specified position of the rectangular area.
//! \param theHorPos [in] the horizontal position.
//! \param theVerPos [in] the vertical position.
//! \return the contained model index.
asiUI_HeaderIndex
  asiUI_DatumClipboardTool::HeaderIndexRectangle::GetIndexAt(const int theHorPos,
                                                         const int theVerPos) const
{
  ASSERT_RAISE( theHorPos >= m_left && theHorPos < m_left + m_horLength, "invalid pos" );
  ASSERT_RAISE( theVerPos >= m_top  && theVerPos < m_top + m_verLength,  "invalid pos" );

  return m_indexes.value( QPair<int, int>(theHorPos, theVerPos), asiUI_HeaderIndex() );
}

//! Specify rectangular area item at the specified position.
//! \param theHorPos [in] the horizontal position.
//! \param theVerPos [in] the vertical position.
//! \param theIndex [in] the index to specify.
void asiUI_DatumClipboardTool::HeaderIndexRectangle::SetIndexAt(const int theHorPos,
                                                            const int theVerPos,
                                                            const asiUI_HeaderIndex& theIndex)
{
  ASSERT_RAISE( theHorPos >= m_left && theHorPos < m_left + m_horLength, "invalid pos" );
  ASSERT_RAISE( theVerPos >= m_top  && theVerPos < m_top + m_verLength,  "invalid pos" );

  m_indexes[QPair<int, int>(theHorPos, theVerPos)] = theIndex;
}
