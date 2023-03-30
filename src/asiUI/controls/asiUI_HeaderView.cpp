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

// asiUI includes
#include <asiUI_HeaderView.h>
#include <asiUI_ItemRoles.h>

// Qt includes
#pragma warning(push, 0)
#include <QProxyStyle>
#include <QEvent>
#include <QHoverEvent>
#pragma warning(pop)

//! Constructor.
//! \param theHeaderType [in] the header orientation
//! type.
//! \param theParent [in] parent owner widget.
asiUI_HeaderView::asiUI_HeaderView(Qt::Orientation theHeaderType,
                                     QAbstractItemView* theParent)
: QHeaderView(theHeaderType, theParent),
  m_bandItemStyle(HeaderItemStyle_Value),
  m_bHideIfEmpty(true),
  m_dTextAngle(0.0),
  m_bWordWrap(false),
  m_bSelectSectionWithBands(true)
{
  setStyle( new asiUI_HeaderStyle(NULL) );
  setSectionsClickable(true);
  setHighlightSections(true);

  connect( this, SIGNAL( geometriesChanged() ), SLOT( onGeometriesChanged() ) );
  connect( this, SIGNAL( sectionResized(int, int, int) ), SLOT( onSectionResized(int, int, int) ) );
}

//! Get word wrap flag. The flag specifies whether the text
//! in headers can be divided into lines to fit into section.
//! \return the word wrap flag.
bool asiUI_HeaderView::IsWordWrap() const
{
  return m_bWordWrap;
}

//! Set word wrap flag. The flag specifies whether the text in headers can be
//! divided into lines to fit into section. This flag is false by default.
//! \param theIsWordWrap [in] the word wrap flag.
void asiUI_HeaderView::SetWordWrap(bool theIsWordWrap)
{
  m_bWordWrap = theIsWordWrap;
}

//! Get property flag that specifies whether the header should be hidden
//! if view does not contain any rows and columns.
//! \return the property flag.
bool asiUI_HeaderView::IsHideIfEmpty() const
{
  return m_bHideIfEmpty;
}

//! Set property flag that specifies whether the header should be hidden if view
//! does not contain any rows and columns. This flag is true by default.
//! \param theFlag [in] the property flag.
void asiUI_HeaderView::SetHideIfEmpty(const bool theFlag)
{
  m_bHideIfEmpty = theFlag;
}

//! Get angle of text rotation along all of the header sections.
//! \return text angle.
double asiUI_HeaderView::GetTextAngle() const
{
  return m_dTextAngle;
}

//! Sets angle of text rotation along all of the header sections.
//! \param theAngle [in] the angle of text rotation.
void asiUI_HeaderView::SetTextAngle(const double theAngle)
{
  m_dTextAngle = theAngle;
}

//! Get property flag that specifies whether the bands related to
//! section should be selected when user presses the section.
//! \return the property flag.
bool asiUI_HeaderView::IsSelectSectionWithBands() const
{
  return m_bSelectSectionWithBands;
}

//! Set property flag that specifies whether the bands related to
//! section should be selected when user presses the section.
//! \param theIsSelect [in] the property flag.
void asiUI_HeaderView::SetSelectSectionWithBands(const bool theIsSelect)
{
  m_bSelectSectionWithBands = theIsSelect;
}

//! Get style of band header items.
//! \return style.
asiUI_HeaderItemStyle asiUI_HeaderView::GetBandItemStyle() const
{
  return m_bandItemStyle;
}

//! Set style of band header items.
//! \param theStyle [in] the style of band header items.
void asiUI_HeaderView::SetBandItemStyle(const asiUI_HeaderItemStyle theStyle)
{
  m_bandItemStyle = theStyle;
}

//! Measures size of the section element.
//! \param theIndex [in] the index of the element.
QSize asiUI_HeaderView::SectionElementSize(const asiUI_HeaderIndex& theIndex) const
{
  return this->paintingRect(theIndex).size();
}

// ----------------------------------------------------------------------------
//                Header view implementation (interaction)
// ----------------------------------------------------------------------------

//! Handles mouse pressing events. Implements advanced selection for
//! hierarchy and band header sections. The default behavior is used
//! for standard item models, which do not implements extended item
//! identification.
//! \param theEvent [in] the mouse event.
void asiUI_HeaderView::mousePressEvent(QMouseEvent* theEvent)
{
  asiUI_HeaderViewDataApi* aHeaderExtensionAPI = dynamic_cast<asiUI_HeaderViewDataApi*>( model() );
  asiUI_HeaderViewSelectionApi* aSelectionAPI = dynamic_cast<asiUI_HeaderViewSelectionApi*>( selectionModel() );
  if ( !aHeaderExtensionAPI || !aSelectionAPI )
  {
    QHeaderView::mousePressEvent(theEvent);
    return;
  }

  if ( theEvent->button() != Qt::LeftButton )
  {
    return;
  }

  m_mouseCache.handle  = -1;
  m_mouseCache.action  = None;
  m_mouseCache.staticSelection.clear();
  m_mouseCache.sectionSize = 0;
  m_mouseCache.pressed = theEvent->pos();

  int aHandle = handleAt( theEvent->pos() );
  if ( aHandle >= 0 )
  {
    m_mouseCache.action = Resize;
    m_mouseCache.handle = aHandle;
    m_mouseCache.sectionSize = sectionSize(aHandle);
    return;
  }

  if ( sectionsClickable() )
  {
    asiUI_HeaderIndex aIndex = sectionAt( theEvent->pos() );
    if ( !aIndex.IsValid() )
    {
      return;
    }

    asiUI_HeaderIndexList aSelectionList;

    aSelectionList += aIndex;

    if ( m_bSelectSectionWithBands && aIndex.IsSection() )
    {
      asiUI_HeaderIndexList aBandIndexes;

      for ( int section = aIndex.GetFirstSection(); section <= aIndex.GetLastSection(); ++section )
      {
        for ( int band = 0; band < aHeaderExtensionAPI->BandRowsCount( orientation() ); ++band )
        {
          aSelectionList += asiUI_HeaderIndex::Band(section, band);
        }
      }
    }

    asiUI_HeaderIndexList aHeaderSelection = aSelectionAPI->HeaderSelection( orientation() );

    bool isToggling = ( theEvent->modifiers() & Qt::ControlModifier ) != 0;
    bool isSelecting = isToggling ? aHeaderSelection.indexOf(aIndex) == -1 : true;

    emit this->sectionPressed(aIndex);

    // emit usual signal to notify views
    if ( aIndex.IsUnitarySection() )
    {
      emit QHeaderView::sectionPressed( aIndex.GetSection() );
    }

    if ( isToggling )
    {
      aSelectionAPI->select( orientation(), aSelectionList, isSelecting ? QItemSelectionModel::Select : QItemSelectionModel::Deselect );
    }
    else
    {
      aSelectionAPI->select( orientation(), aSelectionList, QItemSelectionModel::ClearAndSelect );
    }

    m_mouseCache.action = Select;
    m_mouseCache.staticSelection = aHeaderSelection;

    QSet<int> aUpdateSet;
    for ( int aSectionIt = aIndex.GetFirstSection(); aSectionIt <= aIndex.GetLastSection(); ++aSectionIt )
    {
      aUpdateSet.insert(aSectionIt);
    }

    asiUI_HeaderIndexList::Iterator anIter = aHeaderSelection.begin();
    foreach( asiUI_HeaderIndex aPrevSelectionIndex, aHeaderSelection )
    {
      for ( int aSectionIt = aPrevSelectionIndex.GetFirstSection(); aSectionIt <= aPrevSelectionIndex.GetLastSection(); ++aSectionIt )
      {
        aUpdateSet.insert(aSectionIt);
      }
    }

    foreach( int aSectionIdx, aUpdateSet )
    {
      updateSection(aSectionIdx);
    }

    return;
  }
}

//! Handles mouse moving events. Implements advanced selection for
//! hierarchy and band header sections. The default behavior is used
//! for standard item models, which do not implements extended item
//! identification.
//! \param theEvent [in] the mouse event.
void asiUI_HeaderView::mouseMoveEvent(QMouseEvent* theEvent)
{
  asiUI_HeaderViewDataApi* aHeaderExtensionAPI = dynamic_cast<asiUI_HeaderViewDataApi*>( model() );
  asiUI_HeaderViewSelectionApi* aSelectionAPI = dynamic_cast<asiUI_HeaderViewSelectionApi*>( selectionModel() );
  if ( !aHeaderExtensionAPI || !aSelectionAPI )
  {
    QHeaderView::mouseMoveEvent(theEvent);
    return;
  }

  if ( m_mouseCache.action == None )
  {
    int aHandle = handleAt( theEvent->pos() );

    bool aHasCursor = testAttribute(Qt::WA_SetCursor);

    if ( aHandle != -1 && (sectionResizeMode(aHandle) == Interactive) )
    {
      if ( !aHasCursor )
      {
        setCursor( orientation() == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor );
      }
    }
    else if ( aHasCursor )
    {
      unsetCursor();
    }

    return;
  }

  switch ( m_mouseCache.action )
  {
    case Select :
    {
      QPoint aP1  = m_mouseCache.pressed;
      QPoint aP2  = theEvent->pos();
      int aLeft   = qMin( aP1.x(), aP2.x() );
      int aRight  = qMax( aP1.x(), aP2.x() );
      int aTop    = qMin( aP1.y(), aP2.y() );
      int aBottom = qMax( aP1.y(), aP2.y() );

      asiUI_HeaderIndexList aSelection = sectionsAt( QRect(aLeft, aTop, aRight - aLeft, aBottom - aTop) );

      if ( m_bSelectSectionWithBands )
      {
        asiUI_HeaderIndexSet aBands;
        asiUI_HeaderIndexList::Iterator anIt = aSelection.begin();
        for ( ; anIt != aSelection.end(); ++anIt )
        {
          asiUI_HeaderIndex& anIndex = *anIt;

          if ( !anIndex.IsSection() )
          {
            continue;
          }

          for ( int section = anIndex.GetFirstSection(); section <= anIndex.GetLastSection(); ++section )
          {
            for ( int band = 0; band < aHeaderExtensionAPI->BandRowsCount( orientation() ); ++band )
            {
              aBands += asiUI_HeaderIndex::Band(section, band);
            }
          }
        }

        aSelection = ( aSelection.toSet() + aBands ).toList();
      }

      if ( ( theEvent->modifiers() & Qt::ControlModifier ) != 0 )
      {
        aSelection += ( aSelection.toSet() + m_mouseCache.staticSelection.toSet() ).toList();
      }

      aSelectionAPI->select( orientation(), aSelection, QItemSelectionModel::ClearAndSelect );
    }
    break;

    case Resize :
    {
      int aCurrPos = orientation() == Qt::Horizontal ? theEvent->pos().x() : theEvent->pos().y();
      int aPressPos = orientation() == Qt::Horizontal ? m_mouseCache.pressed.x() : m_mouseCache.pressed.y();
      int aDelta = aCurrPos - aPressPos;

      resizeSection( m_mouseCache.handle, qMax( m_mouseCache.sectionSize + aDelta, minimumSectionSize() ) );
    }
    break;

    default: break;
  }
}

//! Handles mouse releasing events. Implements advanced selection for
//! hierarchy and band header sections. The default behavior is used
//! for standard item models, which do not implements extended item
//! identification.
//! \param theEvent [in] the mouse event.
void asiUI_HeaderView::mouseReleaseEvent(QMouseEvent* theEvent)
{
  QHeaderView::mouseReleaseEvent(theEvent);

  m_mouseCache.action = None;
  m_mouseCache.pressed = QPoint();
  m_mouseCache.staticSelection.clear();
  m_mouseCache.handle = -1;
  m_mouseCache.sectionSize = 0;
}

//! Handles mouse double clicking events. Implements advanced selection for
//! hierarchy and band header sections. The default behavior is used
//! for standard item models, which do not implements extended item
//! identification.
//! \param theEvent [in] the mouse event.
void asiUI_HeaderView::mouseDoubleClickEvent(QMouseEvent* theEvent)
{
  int aHandleAt = handleAt( theEvent->pos() );
  if ( aHandleAt < 0 )
  {
    return;
  }

  QHeaderView::mouseDoubleClickEvent(theEvent);
}

//! Reimplemented method.
//! Repaint header if current changes.
//! \param theCurrent [in] the current index.
//! \param thePrevious [in] the previous index.
void asiUI_HeaderView::currentChanged(const QModelIndex& theCurrent, const QModelIndex& thePrevious)
{
  QHeaderView::currentChanged(theCurrent, thePrevious);

  viewport()->update();
}

//! Reimplemented method.
//! Repaint header if selection changes.
//! \param theSelected [in] the selected indices.
//! \param theDeselected [in] the deselected indices.
void asiUI_HeaderView::selectionChanged(const QItemSelection& theSelected, const QItemSelection& theDeselected)
{
  QHeaderView::selectionChanged(theSelected, theDeselected);

  viewport()->update();
}

//! Handles window events. Provides hovering and selection for particular parts of
//! the header (e.g. a range of hierarchy sections, band header items.
//! \param theEvent [in] the handled event.
bool asiUI_HeaderView::event(QEvent* theEvent)
{
  QList<int> aUpdateList;

  switch ( theEvent->type() )
  {
    // entering viewport
    case QEvent::HoverEnter :
    {
      QHoverEvent* aHoverEvent = static_cast<QHoverEvent*>(theEvent);

      m_hoverPos = aHoverEvent->pos();
      m_hoverSection = logicalIndexAt(m_hoverPos);

      if ( m_hoverSection != -1 )
      {
        aUpdateList << m_hoverSection;
      }
    }
    break;

    // leaving viewport
    case QEvent::Leave :
    case QEvent::HoverLeave :
    {
      if ( m_hoverSection != -1)
      {
        aUpdateList << m_hoverSection;
        m_hoverSection = -1;
        m_hoverPos = QPoint();
      }
    }
    break;

    // moving cursor over the viewport
    case QEvent::HoverMove :
    {
      QHoverEvent* aHoverEv = static_cast<QHoverEvent*>(theEvent);
      int anOldHover = m_hoverSection;
      m_hoverPos = aHoverEv->pos();
      m_hoverSection = logicalIndexAt( m_hoverPos );
      if ( m_hoverSection != anOldHover && anOldHover != -1 )
      {
        aUpdateList << anOldHover;
      }

      if (m_hoverSection != -1)
      {
        aUpdateList << m_hoverSection;
      }
    }
    break;

    default:
      break;
  }

  int aUpdateNb = aUpdateList.size();
  bool doUpdate = aUpdateNb > 0;
  bool doCompleteUpdate = aUpdateNb > 1 || (doUpdate && !getHierarchyHeaders( aUpdateList.first() ).isEmpty());
  if ( doCompleteUpdate )
  {
    viewport()->update();
  }
  else if ( doUpdate )
  {
    updateSection( aUpdateList.first() );
  }

  return QHeaderView::event(theEvent);
}

// ----------------------------------------------------------------------------
//                Header view implementation (painting)
// ----------------------------------------------------------------------------

//! Redefines painting event, the painting procedure separates painting of
//! sections, section groups and band rows to different method calls.
//! Also, the painting is redefined for "empty" header state.
//! \sa SetHideIfEmpty, IsHideIfEmpty
//! \param theEvent [in] the paint event.
void asiUI_HeaderView::paintEvent(QPaintEvent* theEvent)
{
  // paint empty header
  if ( count() == 0 )
  {
    if ( m_bHideIfEmpty )
    {
      return;
    }

    QPainter aPainter( viewport() );
    QStyleOption aOpt;
    aOpt.init(this);

    if ( orientation() == Qt::Horizontal )
    {
      aOpt.state |= QStyle::State_Horizontal;
    }
    else
    {
      aOpt.state &= ~QStyle::State_Horizontal;
    }

    aOpt.rect = QRect( 0, 0, viewport()->width(), viewport()->height() );

    style()->drawControl(QStyle::CE_HeaderEmptyArea, &aOpt, &aPainter, this);
  }

  // default section painting is redefined (void)
  // paint other elements with default approach
  QHeaderView::paintEvent(theEvent);

  // paint viewed sections and header bands
  QPainter aPainter( viewport() );

  QRect aViewedRect = theEvent->rect().translated( dirtyRegionOffset() );

  // get section painting ranges
  bool isColumnHeader = orientation() == Qt::Horizontal;
  int aVisualRangeStarts = visualIndexAt( isColumnHeader ? aViewedRect.left()  : aViewedRect.top() );
  int aVisualRangeEnds   = visualIndexAt( isColumnHeader ? aViewedRect.right() : aViewedRect.bottom() );
  if ( aVisualRangeStarts < 0 )
  {
    aVisualRangeStarts = isRightToLeft() ? count() - 1 : 0;
  }
  if ( aVisualRangeEnds < 0 )
  {
    aVisualRangeEnds = isRightToLeft() ? 0 : count() - 1;
  }

  this->paintSections(&aPainter, QPoint(), aVisualRangeStarts, aVisualRangeEnds);
}

//! Paints the viewed set of sections.
//! \param thePainter [in] the painter.
//! \param theOffset [in] the viewport offset.
//! \param theFirst [in] the first visual index in the set.
//! \param theLast [in] the last visual index in the set.
void asiUI_HeaderView::paintSections(QPainter* thePainter, const QPoint& theOffset, int theFirst, int theLast) const
{
  asiUI_HeaderIndexSet aSections;

  // collect header indexes to be painted
  for ( int it = theFirst; it <= theLast; ++it )
  {
    int aLogicalIdx = logicalIndex(it);

    aSections += asiUI_HeaderIndex::UnitarySection(aLogicalIdx);
    aSections += getHierarchyHeaders(aLogicalIdx);
    aSections += getBandHeaders(aLogicalIdx);
  }

  // draw each one header item separately in the associated rectangles
  asiUI_HeaderIndexSet::Iterator anIt = aSections.begin();
  for ( ; anIt != aSections.end(); ++anIt )
  {
    const asiUI_HeaderIndex& anIndex = *anIt;

    if ( !anIndex.IsValid() )
    {
      continue;
    }

    QRect aPaintingRect = paintingRect(anIndex);

    asiUI_HeaderStyleOption aStyleOption =
      getStyleOptions( anIndex, aPaintingRect.translated(theOffset), false );

    thePainter->save();

    style()->drawControl(QStyle::CE_Header, &aStyleOption, thePainter, this);

    thePainter->restore();
  }
}

//! Evaluates section viewport position. The size hint cache should be computed.
//! \param theIndex [in] the index to find viewport position.
QPoint asiUI_HeaderView::viewportPosition(const asiUI_HeaderIndex& theIndex) const
{
  if ( !theIndex.IsValid() )
  {
    return QPoint();
  }

  bool isColumnHeader = orientation() == Qt::Horizontal;

  int aLogicalIdx  = theIndex.GetSection();
  int aBandIdx     = theIndex.GetBandLine();
  int aViewportPos = sectionViewportPosition(aLogicalIdx);
  int aOffset = 0;

  if ( theIndex.IsBand() )
  {
    aOffset = aBandIdx > 0
      ? m_measureCache.hierarchyHeight + m_measureCache.bandRowBottom.value(aBandIdx - 1, 0)
      : m_measureCache.hierarchyHeight;
  }
  else
  {
    asiUI_HeaderIndexSet aTopSections = getHierarchyHeaders(theIndex);
    asiUI_HeaderIndexSet::Iterator anIt = aTopSections.begin();
    for ( ; anIt != aTopSections.end(); ++anIt )
    {
      aOffset += isColumnHeader
        ? m_measureCache.size.value( *anIt, QSize(0, 0) ).height()
        : m_measureCache.size.value( *anIt, QSize(0, 0) ).width();
    }
  }

  if ( theIndex.IsBand() )
  {
    int aViewHeight = orientation() == Qt::Horizontal ? viewport()->height() : viewport()->width();
    int aFreeSpace = aViewHeight - ( m_measureCache.hierarchyHeight + m_measureCache.bandRowsHeight );
    aOffset += aFreeSpace;
  }

  return isColumnHeader
    ? QPoint( aViewportPos, aOffset )
    : QPoint( aOffset, aViewportPos );
}

//! Styling options for header elements.
//! \param theIndex [in] the index of the hierarchy item.
//! \param theRect [in] the precomputed item painting rect (for mouse hovering style).
//! \param theIsMeasure [in] the flag identifies whether the options are used for measuring.
asiUI_HeaderStyleOption
  asiUI_HeaderView::getStyleOptions(const asiUI_HeaderIndex& theIndex,
                                     const QRect& theRect,
                                     const bool toMeasure) const
{
  Qt::Orientation aOrientation = this->orientation();

  // ==============================
  // Access user-defined properties
  // ==============================

  QMap<int, QVariant> aValueByRole;

  asiUI_HeaderViewDataApi* aHeaderExtensionAPI = dynamic_cast<asiUI_HeaderViewDataApi*>( model() );

  if ( !aHeaderExtensionAPI )
  {
    aValueByRole[Qt::DisplayRole]          = model()->headerData( theIndex.GetSection(), aOrientation, Qt::DisplayRole );
    aValueByRole[Qt::FontRole]             = model()->headerData( theIndex.GetSection(), aOrientation, Qt::FontRole );
    aValueByRole[Qt::DecorationRole]       = model()->headerData( theIndex.GetSection(), aOrientation, Qt::DecorationRole );
    aValueByRole[Qt::TextAlignmentRole]    = model()->headerData( theIndex.GetSection(), aOrientation, Qt::TextAlignmentRole );
    aValueByRole[Qt::TextColorRole]        = model()->headerData( theIndex.GetSection(), aOrientation, Qt::TextColorRole );
    aValueByRole[Qt::BackgroundColorRole]  = model()->headerData( theIndex.GetSection(), aOrientation, Qt::BackgroundColorRole );
    aValueByRole[HeaderViewRole_TextAngle] = model()->headerData( theIndex.GetSection(), aOrientation, HeaderViewRole_TextAngle );
  }
  else
  {
    aValueByRole[Qt::DisplayRole]          = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, Qt::DisplayRole );
    aValueByRole[Qt::FontRole]             = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, Qt::FontRole );
    aValueByRole[Qt::DecorationRole]       = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, Qt::DecorationRole );
    aValueByRole[Qt::TextAlignmentRole]    = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, Qt::TextAlignmentRole );
    aValueByRole[Qt::TextColorRole]        = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, Qt::TextColorRole );
    aValueByRole[Qt::BackgroundColorRole]  = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, Qt::BackgroundColorRole );
    aValueByRole[HeaderViewRole_TextAngle] = aHeaderExtensionAPI->HeaderItemData( aOrientation, theIndex, HeaderViewRole_TextAngle );
  }

  // data specific options
  QVariant& aContents       = aValueByRole[Qt::DisplayRole];
  QVariant aFontHint        = aValueByRole[Qt::FontRole];
  QVariant anIconHint       = aValueByRole[Qt::DecorationRole];
  QVariant anAlignmentHint  = aValueByRole[Qt::TextAlignmentRole];
  QVariant aTextColor       = aValueByRole[Qt::TextColorRole];
  QVariant aBgColor         = aValueByRole[Qt::BackgroundColorRole];
  QVariant aCustomTextAngle = aValueByRole[HeaderViewRole_TextAngle];

  // Font property
  QFont aFont = aFontHint.isValid()
              && aFontHint.canConvert<QFont>()
               ? aFontHint.value<QFont>() : QFont( this->font() );

  if ( toMeasure )
  {
    aFont.setBold(true);
  }

  // Icon property
  QIcon anIcon = anIconHint.isValid()
              && anIconHint.canConvert<QIcon>()
               ? anIconHint.value<QIcon>() : QIcon();

  // Text alignment property
  Qt::Alignment anAlignment = /* anAlignmentHint.isValid()
                           && anAlignmentHint.canConvert<int>()
                            ? anAlignmentHint.value<int>() : */ Qt::AlignCenter;

  double aTextAngle = m_dTextAngle;

  // override default text angle
  if ( aCustomTextAngle.isValid() && aCustomTextAngle.canConvert<double>() )
  {
    aTextAngle = aCustomTextAngle.toDouble();
  }

  // ============================================
  // Initialize basics properties of option style
  // ============================================

  // Compute section span
  double aSectionSpan = 0;
  for ( int anIt = theIndex.GetFirstSection(); anIt <= theIndex.GetLastSection(); ++anIt )
  {
    aSectionSpan += this->sectionSize(anIt);
  }
  int aWrapSize = (int) (aSectionSpan);

  // Choose appropriate option style type
  asiUI_HeaderStyleOption aStyleOption;
  initStyleOption(&aStyleOption);
  aStyleOption.text          = aContents.toString();
  aStyleOption.textAlignment = anAlignment;
  aStyleOption.fontMetrics   = QFontMetrics(aFont);
  aStyleOption.icon          = anIcon;
  aStyleOption.orientation   = aOrientation;
  aStyleOption.wrapSize      = aWrapSize;
  aStyleOption.wordwrap      = m_bWordWrap;
  aStyleOption.angle         = aTextAngle;
  aStyleOption.palette       = palette();
  if ( theIndex.IsBand() )
  {
    aStyleOption.itemStyle = m_bandItemStyle;
  }

  // =====================
  // view specific options
  // =====================

  if ( isEnabled() )
  {
    aStyleOption.state |= QStyle::State_Enabled;
  }

  if ( window()->isActiveWindow() )
  {
    aStyleOption.state |= QStyle::State_Active;
  }

  // =====================
  // Other item properties
  // =====================

  // Set up coloring
  if ( aTextColor.isValid() && aTextColor.canConvert<QColor>() )
  {
    aStyleOption.palette.setColor( QPalette::ButtonText, aTextColor.value<QColor>() );
    aStyleOption.palette.setColor( QPalette::Text, aTextColor.value<QColor>() );
  }

  if ( aBgColor.isValid() && aBgColor.canConvert<QColor>() )
  {
    aStyleOption.palette.setColor( QPalette::Background, aTextColor.value<QColor>() );
  }

  // ===============================================================================
  // Method can be used for measuring items. In this case rectangle should be empty.
  // ===============================================================================

  if ( !toMeasure )
  {
    aStyleOption.rect = theRect;
  }

  // ==================================
  // Determine section position options
  // ==================================

  bool isCaptionStyle = !theIndex.IsBand() || m_bandItemStyle != HeaderItemStyle_Value;
  if ( isCaptionStyle )
  {
    int aSection = visualIndex( theIndex.GetSection() );

    if ( count() == 1 )
    {
      aStyleOption.position = QStyleOptionHeader::OnlyOneSection;
    }
    else
    {
      if ( aSection == 0 )
      {
        aStyleOption.position = QStyleOptionHeader::Beginning;
      }
      else if ( aSection == count() - 1 )
      {
        aStyleOption.position = QStyleOptionHeader::End;
      }
      else
      {
        aStyleOption.position = QStyleOptionHeader::Middle;
      }
    }
  }

  QItemSelectionModel* aSelectionModel = selectionModel();

  if ( isCaptionStyle && aSelectionModel )
  {
    int aSection = visualIndex( theIndex.GetSection() );

    bool isPreviousSelected =
      orientation() == Qt::Horizontal
        ? aSelectionModel->isColumnSelected( logicalIndex(aSection - 1), rootIndex() )
        : aSelectionModel->isRowSelected(    logicalIndex(aSection - 1), rootIndex() );

    bool isNextSelected =
      orientation() == Qt::Horizontal
        ? aSelectionModel->isColumnSelected( logicalIndex(aSection + 1), rootIndex() )
        : aSelectionModel->isRowSelected(    logicalIndex(aSection + 1), rootIndex() );

    if ( isPreviousSelected && isNextSelected )
    {
      aStyleOption.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
    }
    else
    {
      if ( isPreviousSelected )
      {
        aStyleOption.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
      }
      else
      {
        aStyleOption.selectedPosition =
          isNextSelected
            ? QStyleOptionHeader::NextIsSelected
            : QStyleOptionHeader::NotAdjacent;
      }
    }
  }

  // =================================
  // Determine section selection state
  // =================================

  // advanced selection detection interface
  asiUI_HeaderViewSelectionApi*
    aSelectionAPI = dynamic_cast<asiUI_HeaderViewSelectionApi*>(aSelectionModel);

  // styling related to selection
  if ( aSelectionModel && sectionsClickable() && highlightSections() )
  {
    bool isSelected = true;
    bool isIntersecting = true;
    if ( !aSelectionAPI )
    {
      for ( int aLogicalIdx = theIndex.GetFirstSection(); aLogicalIdx <= theIndex.GetLastSection(); ++aLogicalIdx )
      {
        isSelected &= orientation() == Qt::Horizontal
          ? aSelectionModel->isColumnSelected( aLogicalIdx, rootIndex() )
          : aSelectionModel->isRowSelected( aLogicalIdx, rootIndex() );

        isIntersecting &= orientation() == Qt::Horizontal
          ? aSelectionModel->columnIntersectsSelection( aLogicalIdx, rootIndex() )
          : aSelectionModel->rowIntersectsSelection( aLogicalIdx, rootIndex() );
      }
    }
    else
    {
      isSelected = aSelectionAPI->HeaderSelection( orientation(), false ).indexOf(theIndex) != -1;
      isIntersecting =
        isSelected || aSelectionAPI->HeaderSelection( orientation(), true ).indexOf(theIndex) != -1;
    }

    if ( isIntersecting && isCaptionStyle )
    {
      aStyleOption.state |= QStyle::State_On;
    }

    if ( isSelected )
    {
      aStyleOption.state |= QStyle::State_Sunken;
      aStyleOption.state |= QStyle::State_Selected;
    }
  }

  // ===============================
  // Mouse highlighting for sections
  // ===============================

  if ( !m_hoverPos.isNull() )
  {
    bool isMouseHighlight = sectionsClickable() && highlightSections();
    if ( isMouseHighlight && aSelectionAPI )
    {
      isMouseHighlight = theRect.contains(m_hoverPos);
    }
    else if ( isMouseHighlight )
    {
      isMouseHighlight = orientation() == Qt::Horizontal
        ? theRect.left() < m_hoverPos.x() && theRect.right() > m_hoverPos.x()
        : theRect.top()  < m_hoverPos.y() && theRect.bottom() > m_hoverPos.y();
    }

    if ( isMouseHighlight )
    {
      aStyleOption.state |= QStyle::State_MouseOver;
    }
  }

  return aStyleOption;
}

// ----------------------------------------------------------------------------
//                Header view implementation (measurement)
// ----------------------------------------------------------------------------

//! Provide size hint for empty headers to enable paint events.
//! This method implements the hide-if-empty option.
//! \sa SetHideIfEmpty, IsHideIfEmpty
QSize asiUI_HeaderView::sizeHint() const
{
  // size hint for empty header
  if ( count() == 0 )
  {
    if ( m_bHideIfEmpty )
    {
      return QSize();
    }

    QFont aFont = font();
    aFont.setBold(true);

    asiUI_HeaderStyleOption aHeaderOption;
    initStyleOption(&aHeaderOption);
    aHeaderOption.section = 0;

    if ( orientation() == Qt::Horizontal )
    {
      aHeaderOption.state |= QStyle::State_Horizontal;
    }
    else
    {
      aHeaderOption.state &= ~QStyle::State_Horizontal;
    }

    aHeaderOption.fontMetrics = QFontMetrics(aFont);
    aHeaderOption.text = "X";

    return style()->sizeFromContents( QStyle::CT_HeaderSection, &aHeaderOption, QSize(), this );
  }

  return QHeaderView::sizeHint();
}

//! Evaluates size of area of the section in header view. This area includes, in addition to
//! the section itself, the areas of multi-row header part, and overlapping part of
//! header group hierarchy.
//! \param theSection [in] the logical index of the section to evaluate.
//! \return the size hint.
QSize asiUI_HeaderView::sectionSizeFromContents(int theSection) const
{
  // If header does not use features of extended header view API
  // then the column size can be calculated using standard qt
  // implementation of header view in optimized way (only hundred of first and
  // last sections are taken into account) - caching size only for the
  // current section
  if ( !usesExtendedFeatures() )
  {
    updateSectionCache(theSection);
  }
  else if ( m_measureCache.size.isEmpty() )
  {
    // Otherwise we need to update section size hints for all section
    // at once to keep hierarchy and other stuff correct
    for ( int aSection = 0; aSection < count(); ++aSection )
    {
      updateSectionCache(aSection);
    }
  }

  // To make this method compatible with requirements of QHeaderView base class
  // layer we should concatenate cached sizes of all sections incorporated
  // into hierarchy spanned over the given logical index (int theSection)
  int aMeasureWidth  = 0;
  int aMeasureHeight = 0;
  bool isColumnHeader = orientation() == Qt::Horizontal;

  // ========================================================
  // Concatenate section sizes measured for band header items
  // ========================================================

  asiUI_HeaderViewDataApi* aHeaderExtensionAPI = dynamic_cast<asiUI_HeaderViewDataApi*>( model() );

  if ( aHeaderExtensionAPI )
  {
    for ( int row = 0; row < aHeaderExtensionAPI->BandRowsCount( orientation() ); ++row )
    {
      asiUI_HeaderIndex anIndex = asiUI_HeaderIndex::Band(theSection, row);

      if ( isColumnHeader )
      {
        aMeasureWidth   = qMax( aMeasureWidth, m_measureCache.size.value(anIndex).width() );
        aMeasureHeight += m_measureCache.size.value(anIndex).height();
      }
      else
      {
        aMeasureWidth  += m_measureCache.size.value(anIndex).width();
        aMeasureHeight  = qMax( aMeasureHeight, m_measureCache.size.value(anIndex).height() );
      }
    }
  }

  // =============================================================
  // Concatenate section sizes measured for hierarchy header items
  // =============================================================

  m_measureCache.bandRowsHeight = qMax(m_measureCache.bandRowsHeight, aMeasureHeight);

  asiUI_HeaderIndex aSectionIndex = asiUI_HeaderIndex::UnitarySection(theSection);
  asiUI_HeaderIndexList aMeasureIndexes;
  aMeasureIndexes << aSectionIndex;
  aMeasureIndexes << getHierarchyHeaders(theSection).toList();

  int aMeasureHeader = 0;

  asiUI_HeaderIndexList::iterator anIt = aMeasureIndexes.begin();
  for ( ; anIt != aMeasureIndexes.end(); ++anIt )
  {
    asiUI_HeaderIndex& anIndex = *anIt;

    int aGroupSpan = anIndex.GetLastSection() - anIndex.GetFirstSection() + 1;

    if ( isColumnHeader )
    {
      aMeasureWidth   = qMax( aMeasureWidth, m_measureCache.size.value(anIndex).width() / aGroupSpan );
      aMeasureHeight += m_measureCache.size.value(anIndex).height();
    }
    else
    {
      aMeasureWidth  += m_measureCache.size.value(anIndex).width();
      aMeasureHeight  = qMax( aMeasureHeight, m_measureCache.size.value(anIndex).height() / aGroupSpan );
    }

    aMeasureHeader += isColumnHeader
      ? m_measureCache.size.value(anIndex).height()
      : m_measureCache.size.value(anIndex).width();
  }

  return QSize(aMeasureWidth, aMeasureHeight);
}

//! Hint method to compute the item cell size for header section.
//! \param theSizeHint [in] the user specified size hint.
//! \param theItemOptions [in] the style options.
//! \param theView [in] the view.
QSize asiUI_HeaderView::bounds(const QVariant& theSizeHint,
                                const QStyleOption* theItemOptions,
                                const QHeaderView* theView) const
{
  QSize aSizeHint = theSizeHint.isValid()
                 && theSizeHint.canConvert<QSize>()
                  ? theSizeHint.value<QSize>() : QSize();

  QSize aStyleHint = theView->style()->sizeFromContents(QStyle::CT_HeaderSection, theItemOptions, QSize(), theView);

  return aStyleHint.expandedTo(aSizeHint);
}

//! Method returns measured rectangle for the index.
//! The rect should be equal to the one that user sees.
//! \param theIndex [in] the header item index.
//! \return header item rectangle in viewport.
QRect asiUI_HeaderView::paintingRect(const asiUI_HeaderIndex& theIndex) const
{
  int aSpanning = 0;
  for ( int aRangeIt = theIndex.GetFirstSection(); aRangeIt <= theIndex.GetLastSection(); ++aRangeIt )
  {
    aSpanning += sectionSize(aRangeIt);
  }

  QRect anIndexRect( viewportPosition(theIndex), m_measureCache.size.value(theIndex) );

  if ( orientation() == Qt::Horizontal )
  {
    anIndexRect.setWidth(aSpanning);
  }
  else
  {
    anIndexRect.setHeight(aSpanning);
  }

  int aViewHeight = orientation() == Qt::Horizontal ? viewport()->height() : viewport()->width();

  int aFreeSpace = aViewHeight - ( m_measureCache.hierarchyHeight + m_measureCache.bandRowsHeight );

  if ( theIndex.IsUnitarySection() )
  {
    // Enlarge last section to cover free space
    if ( orientation() == Qt::Horizontal )
    {
      anIndexRect.setBottom( qMax( anIndexRect.bottom(), m_measureCache.hierarchyHeight - 1 + aFreeSpace ) );
    }
    else
    {
      anIndexRect.setRight( qMax( anIndexRect.right(), m_measureCache.hierarchyHeight - 1 + aFreeSpace ) );
    }
  }

  return anIndexRect;
}

//! Method returns index of resizing bar located at the passed cursor position.
//! \param theCursor [in] the cursor position.
//! \return resizing bar index under the cursor.
int asiUI_HeaderView::handleAt(const QPoint& theCursor) const
{
  int aPos = orientation() == Qt::Horizontal ? theCursor.x() : theCursor.y();
  int aVisualAt = visualIndexAt( aPos );
  if ( aVisualAt < 0 )
  {
    return -1;
  }

  int aMargin = this->style()->pixelMetric(QStyle::PM_HeaderGripMargin, 0, this);

  int aVisualAtLeft = visualIndexAt( aPos - aMargin );
  int aHandle = -1;

  if ( aVisualAtLeft >= 0 && aVisualAtLeft != aVisualAt )
  {
    aHandle = isRightToLeft() ? logicalIndex(aVisualAt) : logicalIndex(aVisualAtLeft);
  }

  if ( aHandle < 0 )
  {
    int aVisualAtRight = visualIndexAt( aPos + aMargin );
    if ( aVisualAtRight < 0 && aVisualAt == count() - 1 )
    {
      aHandle = logicalIndex(aVisualAt);
    }
    else if ( aVisualAtRight >= 0 && aVisualAtRight != aVisualAt )
    {
      aHandle = isRightToLeft() ? logicalIndex(aVisualAtRight) : logicalIndex(aVisualAt);
    }
  }

  if ( aHandle < 0 )
  {
    return -1;
  }

  asiUI_HeaderIndex aIndex = sectionAt(theCursor);

  if ( aHandle >= aIndex.GetFirstSection() && aHandle < aIndex.GetLastSection() )
  {
    return -1;
  }

  return aHandle;
}

//! Method returns header index located at the passed cursor position.
//! \param theCursor [in] the cursor position.
//! \return item index under the cursor.
asiUI_HeaderIndex asiUI_HeaderView::sectionAt(const QPoint& theCursor) const
{
  int aLogicalIdx = logicalIndexAt( orientation() == Qt::Horizontal ? theCursor.x() : theCursor.y() );
  if ( aLogicalIdx < 0 )
  {
    return asiUI_HeaderIndex();
  }

  asiUI_HeaderIndexList aCandidates;
  aCandidates << asiUI_HeaderIndex::UnitarySection(aLogicalIdx);
  aCandidates << getHierarchyHeaders(aLogicalIdx).toList();
  aCandidates << getBandHeaders(aLogicalIdx).toList();

  asiUI_HeaderIndexList::iterator anIt = aCandidates.begin();
  for ( ; anIt != aCandidates.end(); ++anIt )
  {
    asiUI_HeaderIndex& anIndex = *anIt;

    if ( !paintingRect(anIndex).contains(theCursor) )
    {
      continue;
    }

    return anIndex;
  }

  return asiUI_HeaderIndex();
}

//! Collects header item indexes which intersects with the given rectangle.
//! \param theRect [in] the selection rectangle.
asiUI_HeaderIndexList asiUI_HeaderView::sectionsAt(const QRect& theRect) const
{
  asiUI_HeaderIndexList aSections;

  int aFirstIdx = logicalIndexAt( orientation() == Qt::Horizontal ? theRect.left() : theRect.top() );
  int aLastIdx  = logicalIndexAt( orientation() == Qt::Horizontal ? theRect.right() : theRect.bottom() );

  if ( aFirstIdx < 0 )
  {
    aFirstIdx = 0;
  }

  if ( aLastIdx < 0 )
  {
    aLastIdx = count() - 1;
  }

  for ( int aLogicalIdx = aFirstIdx; aLogicalIdx <= aLastIdx; ++aLogicalIdx )
  {
    asiUI_HeaderIndexList aCandidates;
    aCandidates << asiUI_HeaderIndex::UnitarySection(aLogicalIdx);
    aCandidates << getHierarchyHeaders(aLogicalIdx).toList();
    aCandidates << getBandHeaders(aLogicalIdx).toList();

    asiUI_HeaderIndexList::iterator anIt = aCandidates.begin();
    for ( ; anIt != aCandidates.end(); ++anIt )
    {
      asiUI_HeaderIndex& anIndex = *anIt;

      if ( !theRect.intersects( paintingRect(anIndex) ) )
      {
        continue;
      }

      aSections += anIndex;
    }
  }

  return aSections;
}

//! Method checks if the header view uses features of extended header API.
//! If yes, then the standard qt optimization on limited number of section
//! size hints should be avoided.
//! \return TRUE if header view uses features of extended header API.
bool asiUI_HeaderView::usesExtendedFeatures() const
{
  asiUI_HeaderViewDataApi* aHeaderExtensionAPI = dynamic_cast<asiUI_HeaderViewDataApi*>( model() );

  // not using extended API
  if ( !aHeaderExtensionAPI )
  {
    return false;
  }

  // 1) no hierarchy defined - each section is a individual column or row
  // 2) no band rows defined
  if ( aHeaderExtensionAPI->SectionCount( orientation(), asiUI_HeaderIndex() ) == count()
    && aHeaderExtensionAPI->BandRowsCount( orientation() ) == 0 )
  {
    return false;
  }

  return true;
}

//! Compute and cache (sub)section size of the give section.
//! \param theSection [in] the section to recalculate.
void asiUI_HeaderView::updateSectionCache(const int theSection) const
{
  // ====================================
  // Check if anything need to be updated
  // ====================================

  asiUI_HeaderIndex aSectionIndex = asiUI_HeaderIndex::UnitarySection(theSection);

  if ( m_measureCache.size.contains(aSectionIndex) )
  {
    return;
  }

  // ==========================================
  // Check if standard size hint should be used
  // ==========================================

  asiUI_HeaderViewDataApi* aHeaderExtensionAPI = dynamic_cast<asiUI_HeaderViewDataApi*>( model() );

  if ( !aHeaderExtensionAPI )
  {
    QVariant aHint = model()->headerData( theSection, orientation(), Qt::SizeHintRole );

    asiUI_HeaderStyleOption aStyleOption = getStyleOptions( aSectionIndex, QRect(), true );

    QSize aSectionSize = bounds(aHint, &aStyleOption, this);

    int aSectionHeight = orientation() == Qt::Horizontal
      ? aSectionSize.height()
      : aSectionSize.width();

    m_measureCache.size.insert( aSectionIndex, aSectionSize );
    m_measureCache.hierarchyHeight = qMax(m_measureCache.hierarchyHeight, aSectionHeight);
    return;
  }

  // ===============================
  // Compute and cache section sizes
  // ===============================

  int aMeasureWidth  = 0;
  int aMeasureHeight = 0;

  bool isColumnHeader = orientation() == Qt::Horizontal;

  // measure and cache sizes of band header items
  for ( int row = 0; row < aHeaderExtensionAPI->BandRowsCount( orientation() ); ++row )
  {
    asiUI_HeaderIndex anIndex = asiUI_HeaderIndex::Band(theSection, row);

    QVariant aHint = aHeaderExtensionAPI->HeaderItemData( orientation(), anIndex, Qt::SizeHintRole );

    asiUI_HeaderStyleOption aStyleOption = getStyleOptions( anIndex, QRect(), true );

    QSize aBounds = bounds(aHint, &aStyleOption, this);

    if ( isColumnHeader )
    {
      aMeasureWidth   = qMax( aMeasureWidth, aBounds.width() );
      aMeasureHeight += aBounds.height();
    }
    else
    {
      aMeasureWidth  += aBounds.width();
      aMeasureHeight  = qMax( aMeasureHeight, aBounds.height() );
    }

    // measure maximum row bounds
    m_measureCache.size.insert( anIndex, aBounds );
    m_measureCache.bandRowBottom[row] =
      qMax( m_measureCache.bandRowBottom.value(row, 0),
            isColumnHeader ? aMeasureHeight : aMeasureWidth );
  }

  m_measureCache.bandRowsHeight = qMax(m_measureCache.bandRowsHeight, aMeasureHeight);

  // measure section header size and its top level hierarchy neighbors
  asiUI_HeaderIndexList aMeasureIndexes;
  aMeasureIndexes << aSectionIndex;
  aMeasureIndexes << getHierarchyHeaders(theSection).toList();

  int aMeasureHeader = 0;

  asiUI_HeaderIndexList::iterator anIt = aMeasureIndexes.begin();
  for ( ; anIt != aMeasureIndexes.end(); ++anIt )
  {
    asiUI_HeaderIndex& anIndex = *anIt;

    int aGroupSpan = anIndex.GetLastSection() - anIndex.GetFirstSection() + 1;

    QVariant aHint = aHeaderExtensionAPI->HeaderItemData( orientation(), anIndex, Qt::SizeHintRole );

    asiUI_HeaderStyleOption aStyleOption = getStyleOptions( anIndex, QRect(), true );

    QSize aBounds = bounds(aHint, &aStyleOption, this);

    if ( isColumnHeader )
    {
      aMeasureWidth   = qMax( aMeasureWidth, aBounds.width() / aGroupSpan );
      aMeasureHeight += aBounds.height();
    }
    else
    {
      aMeasureWidth  += aBounds.width();
      aMeasureHeight  = qMax( aMeasureHeight, aBounds.height() / aGroupSpan );
    }

    aMeasureHeader += isColumnHeader ? aBounds.height() : aBounds.width();

    m_measureCache.size.insert( anIndex, aBounds );
  }

  m_measureCache.hierarchyHeight = qMax(m_measureCache.hierarchyHeight, aMeasureHeader);
}

//! Collects header item indexes which have hierarchical relation for the
//! passed section index.
//! \param theLogicalIndex [in] the section index in header data model.
asiUI_HeaderIndexSet asiUI_HeaderView::getHierarchyHeaders(const asiUI_HeaderIndex& theIndex) const
{
  const asiUI_HeaderViewDataApi* aHeaderModelApi = dynamic_cast<const asiUI_HeaderViewDataApi*>( model() );

  if ( !aHeaderModelApi )
  {
    return asiUI_HeaderIndexSet();
  }

  asiUI_HeaderIndexSet aSections;
  asiUI_HeaderIndex aTopIndex = aHeaderModelApi->ParentSection( orientation(), theIndex );

  for ( ; aTopIndex.IsValid(); aTopIndex = aHeaderModelApi->ParentSection( orientation(), aTopIndex ) )
  {
    aSections.insert( aTopIndex );
  }

  return aSections;
}

//! Collects header item indexes which have hierarchical relation for the
//! passed section index.
//! \param theLogicalIndex [in] the section index in header data model.
asiUI_HeaderIndexSet asiUI_HeaderView::getHierarchyHeaders(const int theLogicalIdx) const
{
  return getHierarchyHeaders( asiUI_HeaderIndex::UnitarySection(theLogicalIdx) );
}

//! Collects header item indexes which are draw in
//! band header section under the passed section.
//! \param theLogicalIndex [in] the section index in header data model.
asiUI_HeaderIndexSet asiUI_HeaderView::getBandHeaders(const int theLogicalIdx) const
{
  const asiUI_HeaderViewDataApi* aHeaderModelApi = dynamic_cast<const asiUI_HeaderViewDataApi*>( model() );

  if ( !aHeaderModelApi )
  {
    return asiUI_HeaderIndexSet();
  }

  asiUI_HeaderIndexSet aSections;

  for ( int row = 0; row < aHeaderModelApi->BandRowsCount( orientation() ); ++row )
  {
    aSections += asiUI_HeaderIndex::Band(theLogicalIdx, row);
  }

  return aSections;
}

// ----------------------------------------------------------------------------
//                Header view implementation (internal events)
// ----------------------------------------------------------------------------

//! Sets item model for the header view.
//! Connects event callbacks.
void asiUI_HeaderView::setModel(QAbstractItemModel* theModel)
{
  if ( model() == theModel )
  {
    return;
  }

  QAbstractItemModel* aBeforeModel = model();

  if ( aBeforeModel )
  {
    if ( orientation() == Qt::Horizontal )
    {
      disconnect( aBeforeModel,
        SIGNAL( columnsInserted(QModelIndex, int, int) ), this,
        SLOT( onSectionsInserted(QModelIndex, int, int) ) );

      disconnect( aBeforeModel,
        SIGNAL( columnsRemoved(QModelIndex, int, int) ), this,
        SLOT( onSectionsRemoved(QModelIndex, int, int) ) );
    }
    else
    {
      disconnect( aBeforeModel,
        SIGNAL( rowsInserted(QModelIndex, int, int) ), this,
        SLOT( onSectionsInserted(QModelIndex, int, int) ) );

      disconnect( aBeforeModel,
        SIGNAL( rowsRemoved(QModelIndex, int, int) ), this,
        SLOT( onSectionsRemoved(QModelIndex, int, int) ) );
    }

    disconnect( aBeforeModel,
      SIGNAL( headerDataChanged( Qt::Orientation, int, int ) ), this,
      SLOT( onHeaderDataChanged( Qt::Orientation, int, int) ) );
  }

  QHeaderView::setModel(theModel);

  if ( theModel )
  {
    if ( orientation() == Qt::Horizontal )
    {
      connect( theModel,
        SIGNAL( columnsInserted(QModelIndex, int, int) ), this,
        SLOT( onSectionsInserted(QModelIndex, int, int) ) );

      connect( theModel,
        SIGNAL( columnsRemoved(QModelIndex, int, int) ), this,
        SLOT( onSectionsRemoved(QModelIndex, int, int) ) );
    }
    else
    {
      connect( theModel,
        SIGNAL( rowsInserted(QModelIndex, int, int) ), this,
        SLOT( onSectionsInserted(QModelIndex, int, int) ) );

      connect( theModel,
        SIGNAL( rowsRemoved(QModelIndex, int, int) ), this,
        SLOT( onSectionsRemoved(QModelIndex, int, int) ) );
    }

    connect( theModel,
      SIGNAL( headerDataChanged( Qt::Orientation, int, int ) ), this,
      SLOT( onHeaderDataChanged( Qt::Orientation, int, int) ) );
  }

  m_measureCache.reset();
}

//! Clears precomputed size hint.
//! Callback to invalidate size cache.
//! Invoked when the items layout is changed.
void asiUI_HeaderView::doItemsLayout()
{
  m_measureCache.reset();
  QHeaderView::doItemsLayout();
}

//! Clears precomputed size hint.
//! Callback to invalidate size cache.
//! Invoked when the headers geometry is changed.
void asiUI_HeaderView::onGeometriesChanged()
{
  m_measureCache.reset();
}

//! Clears precomputed size hint.
//! Callback to invalidate size cache.
//! Invoked when the section size is changed.
void asiUI_HeaderView::onSectionResized(int, int, int)
{
  m_measureCache.reset();
}

//! Clears precomputed size hint.
//! Callback to invalidate size cache.
//! Invoked when new sections added.
void asiUI_HeaderView::onSectionsInserted(QModelIndex, int, int)
{
  m_measureCache.reset();
}

//! Clears precomputed size hint.
//! Callback to invalidate size cache.
//! Invoked when sections are removed.
void asiUI_HeaderView::onSectionsRemoved(QModelIndex, int, int)
{
  m_measureCache.reset();
}

//! Clears precomputed size hint.
//! Callback to invalidate size cache.
//! Invoked when sections data changes.
void asiUI_HeaderView::onHeaderDataChanged(Qt::Orientation, int, int)
{
  m_measureCache.reset();
}
