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

#ifndef asiUI_HeaderView_HeaderFile
#define asiUI_HeaderView_HeaderFile

// asiUI includes
#include <asiUI_Common.h>
#include <asiUI_HeaderViewApi.h>
#include <asiUI_HeaderViewStyling.h>

// Qt includes
#pragma warning(push, 0)
#include <QMap>
#include <QPainter>
#include <QHeaderView>
#include <QProxyStyle>
#include <QStyleOptionHeader>
#include <QAbstractProxyModel>
#pragma warning(pop)

#pragma warning(disable: 4251) // "Class XXX needs to have dll-interface..."

//! \ingroup GUI
//!
//! Header view that provides additional features,
//! such as text word wrapping, custom representation
//! for empty item view, etc.
class asiUI_EXPORT asiUI_HeaderView : public QHeaderView
{
  Q_OBJECT

public:

  // Selection flags for
  enum HeaderSelectionFlags
  {
    SelectionFlags_Joint,
    SelectionFlags_Separate
  };

  // Short-cut for shared pointer
  typedef QSharedPointer<QStyleOption> QStyleOptionPtr;

// Qt properties
public:

  Q_PROPERTY(bool wordWrap READ IsWordWrap WRITE SetWordWrap)
  Q_PROPERTY(bool hideIfEmpty READ IsHideIfEmpty WRITE SetHideIfEmpty)
  Q_PROPERTY(double textAngle READ GetTextAngle WRITE SetTextAngle)
  Q_PROPERTY(bool selectSectionWithBands READ IsSelectSectionWithBands WRITE SetSelectSectionWithBands)
  Q_PROPERTY(asiUI_HeaderItemStyle bandItemStyle READ GetBandItemStyle WRITE SetBandItemStyle)

public:

  asiUI_HeaderView(Qt::Orientation theHeaderType, QAbstractItemView* theParent);

public:

  bool IsWordWrap() const;

  void SetWordWrap(bool theIsWordWrap);

  bool IsHideIfEmpty() const;

  void SetHideIfEmpty(const bool theFlag);

  double GetTextAngle() const;

  void SetTextAngle(const double theAngle);

  bool IsSelectSectionWithBands() const;

  void SetSelectSectionWithBands(const bool theIsSelect);

  asiUI_HeaderItemStyle GetBandItemStyle() const;

  void SetBandItemStyle(const asiUI_HeaderItemStyle theStyle);

  QSize SectionElementSize(const asiUI_HeaderIndex& theIndex) const;

public:

  void setModel(QAbstractItemModel* theModel);

protected:

  virtual void mousePressEvent(QMouseEvent* theEvent);
  virtual void mouseMoveEvent(QMouseEvent* theEvent);
  virtual void mouseReleaseEvent(QMouseEvent* theEvent);
  virtual void mouseDoubleClickEvent(QMouseEvent* theEvent);

protected:

  bool event(QEvent* theEvent);

  void paintEvent(QPaintEvent* theEvent);

  virtual void paintSections(QPainter* thePainter, const QPoint& theOffset, int theFirst, int theLast) const;

  virtual QPoint viewportPosition(const asiUI_HeaderIndex& theIndex) const;

  asiUI_HeaderStyleOption
    getStyleOptions(const asiUI_HeaderIndex& theIndex,
                    const QRect& theRect,
                    const bool toMeasure) const;

protected:

  virtual QSize sizeHint() const;

  virtual QSize sectionSizeFromContents(int theSection) const;

  QSize bounds(const QVariant& theSizeHint,
               const QStyleOption* theItemOptions,
               const QHeaderView* theView) const;

  QRect paintingRect(const asiUI_HeaderIndex& theIndex) const;

  int handleAt(const QPoint& theCursor) const;

  asiUI_HeaderIndex sectionAt(const QPoint& theCursor) const;

  asiUI_HeaderIndexList sectionsAt(const QRect& theRect) const;

protected:

  bool usesExtendedFeatures() const;
  void updateSectionCache(const int theSection) const;

protected:

  asiUI_HeaderIndexSet getHierarchyHeaders(const asiUI_HeaderIndex& theIndex) const;
  asiUI_HeaderIndexSet getHierarchyHeaders(const int theLogicalIdx) const;
  asiUI_HeaderIndexSet getBandHeaders(const int theLogicalIdx) const;

protected slots:

  void onGeometriesChanged();
  void onSectionResized(int, int, int);
  void onSectionsInserted(QModelIndex, int, int);
  void onSectionsRemoved(QModelIndex, int, int);
  void onHeaderDataChanged(Qt::Orientation, int, int);
  virtual void doItemsLayout();
  virtual void currentChanged(const QModelIndex& theCurrent, const QModelIndex& thePrevious);
  virtual void selectionChanged(const QItemSelection& theSelected, const QItemSelection& theDeselected);

signals:

  void sectionPressed(const asiUI_HeaderIndex& theIndex);

private:

  //! void implementation of default section painting (not applicable).
  //! \param thePainter [in] the painter.
  //! \param theRect [in] the section rect.
  //! \param theSection [in] the section.
  void paintSection(QPainter* asiUI_NotUsed(thePainter),
                    const QRect& asiUI_NotUsed(theRect),
                    int asiUI_NotUsed(theSection)) const {}

// internal types definition
private:

  typedef QMap<int, int> HeightByRow;
  typedef QMap<asiUI_HeaderIndex, QSize> SizeByIndex;
  struct MeasureCache
  {
    void reset()
    {
      hierarchyHeight = 0;
      bandRowsHeight  = 0;
      bandRowBottom.clear();
      size.clear();
    }

    int           bandRowsHeight;
    HeightByRow   bandRowBottom;
    int           hierarchyHeight;
    SizeByIndex   size;
  };

  enum MouseAction
  {
    Select,
    Resize,
    // Swap, not supported
    None
  };

  struct MouseCache
  {
    MouseCache() { action = None; }
    MouseAction           action;
    int                   handle;
    int                   sectionSize;
    QPoint                pressed;
    asiUI_HeaderIndexList staticSelection;
  };

private:

  asiUI_HeaderItemStyle m_bandItemStyle;
  bool                  m_bSelectSectionWithBands;
  bool                  m_bWordWrap;
  double                m_dTextAngle;
  bool                  m_bHideIfEmpty;
  int                   m_hoverSection;
  QPoint                m_hoverPos;
  mutable MeasureCache  m_measureCache;
  mutable MouseCache    m_mouseCache;
};

#pragma warning(default: 4251) // "Class XXX needs to have dll-interface..."

#endif
