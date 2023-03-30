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

#ifndef asiUI_DatumClipboardTool_HeaderFile
#define asiUI_DatumClipboardTool_HeaderFile

// asiUI includes
#include <asiUI.h>
#include <asiUI_DatumClipboardTableData.h>
#include <asiUI_HeaderView.h>
#include <asiUI_HeaderViewAPI.h>

// Qt includes
#pragma warning(push, 0)
#include <QVector>
#include <QVariant>
#include <QItemSelection>
#pragma warning(pop)

class QString;
class QTreeView;
class QTableView;
class QModelIndex;
class QAbstractItemView;

#pragma warning(disable: 4251) // "Class XXX needs to have dll-interface..."

//! \ingroup GUI
//!
//! Class provides tool methods for working with clipboard buffer, and copying
//! or pasting its data. It provides clipboard data in plain text form, or as
//! a tabulation-newline formatted text representation supported by table-text
//! processors. It also provides services for copying and inserting formatted
//! data within the selected region of table widgets.
class asiUI_EXPORT asiUI_DatumClipboardTool
{
public:

  class ViewIndexRectangle;
  class HeaderIndexRectangle;

// clipboard data exchange
public:

  static void ClearClipboard();

  static QString GetClipboardText();

  static asiUI_DatumClipboardTableData GetClipboardData();

  static void SetClipboardText(const QString& theText);

  static void SetClipboardData(const asiUI_DatumClipboardTableData& theData);

// copy methods
public:

  static asiUI_DatumClipboardTableData
    CopyToData(QAbstractItemView* theView);

  static bool
    CopyToClipboard(QAbstractItemView* theView);

// data paste methods
public:

  static bool
    PasteFromData(QAbstractItemView* theView,
                  const asiUI_DatumClipboardTableData& theData);

  static bool
    PasteFromClipboard(QAbstractItemView* theView);

// Paste Region Size.
// These methods inclapsulate logic related to estimation (depending on current selection)
// of the region of item view that will be (or could be) filled with data from specified
// data buffer.
// Data table-like layout is expected.
public:

  static QRect
    GetPasteRuleRegion(const QRect& theSiblingSelection);

  static QModelIndexList
    GetIndexesFromRect(QAbstractItemView* theView,
                       const QRect& theSiblingSelection,
                       int& theRowsMissing,
                       int& theColsMissing);

// Check copy and paste restriction rules.
public:

  static bool
    IsCopyPossible(QAbstractItemView* theView,
                   const QModelIndexList& theIndexes);

  static bool
    IsPastePossible(QAbstractItemView* theView,
                    const QModelIndexList& theIndexes);

// contiguous selection for known view types
public:

  static QRect
    GetContiguous(QAbstractItemView* theView,
                  const QItemSelection& theItemRange,
                  QModelIndexList& theOrdered);

  static ViewIndexRectangle
    GetSelectionRectangle(QAbstractItemView* theView);

  static HeaderIndexRectangle
    GetSelectionRectangle(asiUI_HeaderView* theView,
                          const bool theTitles,
                          const bool theBands);

private:

  static QRect
    contiguousRect(const QTableView* theView,
                   const QItemSelection& theItemRange,
                   QModelIndexList& theOrdered);

  static QRect
    contiguousRect(const QTreeView* theView,
                   const QItemSelection& theItemRange,
                   QModelIndexList& theOrdered);

  static QModelIndexList
    indexesOfRegion(const QTableView* theView,
                    const QRect& theSiblingSelection,
                    int& theRowsMissing,
                    int& theColsMissing);

  static QModelIndexList
    indexesOfRegion(const QTreeView* theView,
                    const QRect& theSiblingSelection,
                    int& theRowsMissing,
                    int& theColsMissing);

  static asiUI_HeaderIndexList
    topIndexes(const QAbstractItemModel* theModel,
               const Qt::Orientation theHeader,
               const int theSection);
};

//! \ingroup GUI
//!
//! Container representing contiguous rectangular selection area of view items.
//! The purpose of these containers is to check header / view selection for
//! matching by section indexes (that is why here a rectangle with its bounds).
//! Another role is to give the data indexes when composing clipboard buffer.
//! This selection area can be laid "as is" onto any tabular layout, and is
//! generally used for copy-paste operations into EXCEL/CSV format.
//! The area can be empty, which means that nothing is selected or invalid,
//! which means that the contiguous rectangle can not be constructed. The statuses
//! have applicative role and are specified externally.
//! Rectangle contains and returns model indexes, from which the rectangle is being
//! composed of. Due to the fact that rectangular area contains QModelIndexes, which
//! are temporary by their nature - it should not be kept for a long time.
class asiUI_EXPORT asiUI_DatumClipboardTool::ViewIndexRectangle
{
public:

  ViewIndexRectangle();

  ViewIndexRectangle(const int theLeft,
                     const int theTop,
                     const int theHorLength,
                     const int theVerLength);

  QModelIndex GetIndexAt(const int theHorPos, const int theVerPos) const;

  void SetIndexAt(const int theHorPos, const int theVerPos, const QModelIndex& theIndex);

  //! \return Rectangular area.
  inline QRect GetRectangle() const
  {
    return QRect(m_left, m_top, m_horLength, m_verLength);
  }

  //! checks whether the rectangular area is invalid.
  //! \return TRUE if the region is invalid.
  inline bool IsValid() const
  {
    return m_isValid;
  }

  //! checks whether the rectangular area is empty.
  //! \return TRUE if the region is empty.
  inline bool IsEmpty() const
  {
    return !IsValid() || ( m_horLength <= 0 || m_verLength <= 0 );
  }

private:

  typedef QMap<QPair<int,int>, QModelIndex> IndexMap;
  IndexMap m_indexes;
  bool m_isValid;
  int m_left;
  int m_top;
  int m_horLength;
  int m_verLength;
};

//! \ingroup GUI
//!
//! Container representing contiguous rectangular selection area of header items.
//! The purpose of these containers is to check header / view selection for
//! matching by section indexes (that is why here a rectangle with its bounds).
//! Another role is to give the data indexes when composing clipboard buffer.
//! This selection area can be laid "as is" onto any tabular layout, and is
//! generally used for copy-paste operations into EXCEL/CSV format.
//! The area can be empty, which means that nothing is selected or invalid,
//! which means that the contiguous rectangle can not be constructed. The statuses
//! have applicative role and are specified externally.
//! Region contains and returns header indexes, from which the rectangle is being
//! composed of. Due to the fact that rectangular area contains asiUI_HeaderIndexes, which
//! are temporary by their nature - it should not be kept for a long time.
class asiUI_EXPORT asiUI_DatumClipboardTool::HeaderIndexRectangle
{
public:

  HeaderIndexRectangle();

  HeaderIndexRectangle(const int theLeft,
                       const int theTop,
                       const int theHorLength,
                       const int theVerLength);

  asiUI_HeaderIndex GetIndexAt(const int theHorPos, const int theVerPos) const;

  void SetIndexAt(const int theHorPos, const int theVerPos, const asiUI_HeaderIndex& theIndex);

  //! \return Rectangular area.
  inline QRect GetRectangle() const
  {
    return QRect(m_left, m_top, m_horLength, m_verLength);
  }

  //! checks whether the rectangular area is invalid.
  //! \return TRUE if the region is invalid.
  inline bool IsValid() const
  {
    return m_isValid;
  }

  //! checks whether the rectangular area is empty.
  //! \return TRUE if the region is empty.
  inline bool IsEmpty() const
  {
    return !IsValid() || ( m_horLength <= 0 || m_verLength <= 0 );
  }

private:

  typedef QMap<QPair<int,int>, asiUI_HeaderIndex> IndexMap;
  IndexMap m_indexes;
  bool m_isValid;
  int m_left;
  int m_top;
  int m_horLength;
  int m_verLength;
};

#pragma warning(default: 4251) // "Class XXX needs to have dll-interface..."

#endif
