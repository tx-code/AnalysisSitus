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
#include <asiUI_HeaderViewStyling.h>

// --------------------------------------------------------------------------------------
// Style
// --------------------------------------------------------------------------------------

//! Constructor.
//! \param theStyle [in] the style which delegates its
//! functions to this proxy.
asiUI_HeaderStyle::asiUI_HeaderStyle(QStyle* theStyle) : QProxyStyle(theStyle)
{
}

//! Draw header text with additional word wrap option.
//! \param theElement [in] the control element to paint.
//! \param theOption [in] the element's options.
//! \param thePainter [in] the painter to draw element.
//! \param theWidget [in] the owner widget.
void asiUI_HeaderStyle::drawControl(QStyle::ControlElement theElement,
                                     const QStyleOption* theOption,
                                     QPainter* thePainter,
                                     const QWidget* theWidget) const
{
  switch (theElement)
  {
    case CE_Header:
    {
      const asiUI_HeaderStyleOption*
        aHeaderOpt = qstyleoption_cast<const asiUI_HeaderStyleOption*>(theOption);

      QRegion aRegion = thePainter->clipRegion();

      thePainter->setClipRect(theOption->rect);

      switch ( aHeaderOpt->itemStyle )
      {
        case HeaderItemStyle_Caption :
        {
          proxy()->drawControl(CE_HeaderSection, aHeaderOpt, thePainter, theWidget);
        }
        break;

        case HeaderItemStyle_Value :
        {
          bool isSelected = (aHeaderOpt->state & State_Selected) != 0;
          bool isEnabled  = (aHeaderOpt->state & State_Enabled)  != 0;
          bool isActive   = (aHeaderOpt->state & State_Active)   != 0;

          QBrush aBgBrush = aHeaderOpt->palette.brush(QPalette::Normal, QPalette::Background);
          if ( isSelected && isActive )
          {
            aBgBrush = aHeaderOpt->palette.brush(QPalette::Normal, QPalette::Highlight);
          }
          else if ( isSelected && !isActive )
          {
            aBgBrush = aHeaderOpt->palette.brush(QPalette::Inactive, QPalette::Highlight);
          }
          else if ( !isActive && isEnabled )
          {
            aBgBrush = aHeaderOpt->palette.brush(QPalette::Inactive, QPalette::Background);
          }
          else if ( !isEnabled )
          {
            aBgBrush = aHeaderOpt->palette.brush(QPalette::Disabled, QPalette::Background);
          }

          // background
          thePainter->fillRect( aHeaderOpt->rect, aBgBrush );

          // grid lines
          const int aGridHint = styleHint(QStyle::SH_Table_GridLineColor, aHeaderOpt);
          const QColor aGridColor = static_cast<QRgb>(aGridHint);
          thePainter->setPen(aGridColor);
          thePainter->drawRect( aHeaderOpt->rect.adjusted(-1, -1, -1, -1) );
        }
        break;
      }

      asiUI_HeaderStyleOption aSubOption = *aHeaderOpt;

      aSubOption.rect = subElementRect(SE_HeaderLabel, aHeaderOpt, theWidget);

      if ( aSubOption.rect.isValid() )
      {
        thePainter->setClipRect(aSubOption.rect);

        double anAngle = aHeaderOpt ? aHeaderOpt->angle : 0.0;
        if ( anAngle != 0.0 )
        {
          double aDx = (double)aHeaderOpt->rect.left()  + (double)aHeaderOpt->rect.width()  / 2.0;
          double aDy = (double)aHeaderOpt->rect.top()   + (double)aHeaderOpt->rect.height() / 2.0;

          QTransform aRotation;
          aRotation.translate(aDx, aDy);
          aRotation.rotate(anAngle);
          aRotation.translate(-aDx, -aDy);
          thePainter->setRenderHint(QPainter::Antialiasing);
          thePainter->setRenderHint(QPainter::TextAntialiasing);
          thePainter->setMatrix( aRotation.toAffine() );
        }

        proxy()->drawControl(CE_HeaderLabel, &aSubOption, thePainter, theWidget);

        if ( aHeaderOpt->sortIndicator != QStyleOptionHeader::None )
        {
          aSubOption.rect = subElementRect(SE_HeaderArrow, theOption, theWidget);
          proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &aSubOption, thePainter, theWidget);
        }
      }

      thePainter->setClipRegion(aRegion);
      break;
    }

    case CE_HeaderLabel:
    {
      const asiUI_HeaderStyleOption*
        aHeaderOpt = qstyleoption_cast<const asiUI_HeaderStyleOption*>(theOption);

      if ( aHeaderOpt )
      {
        int aTxtFlags = aHeaderOpt->textAlignment | Qt::TextDontClip;

        if ( aHeaderOpt && aHeaderOpt->wordwrap )
        {
          aTxtFlags |= Qt::TextWordWrap;
        }

        QFont aFont = thePainter->font();

        if ( aHeaderOpt->state & QStyle::State_On )
        {
          aFont.setBold(true);
        }
        if ( aHeaderOpt && aHeaderOpt->angle != 0.0 )
        {
          aFont.setStyleStrategy(QFont::ForceIntegerMetrics);
        }

        thePainter->setFont(aFont);

        QPalette::ColorRole aTextColorRole = QPalette::ButtonText;
        if ( aHeaderOpt->itemStyle == HeaderItemStyle_Value )
        {
          bool isSelected = (aHeaderOpt->state & State_Selected) != 0;
          if ( isSelected )
          {
            aTextColorRole = QPalette::HighlightedText;
          }
          else
          {
            aTextColorRole = QPalette::Text;
          }
        }

        drawItemText(thePainter,
                     aHeaderOpt->rect,
                     aTxtFlags,
                     aHeaderOpt->palette,
                     (aHeaderOpt->state & State_Enabled) != 0,
                     aHeaderOpt->text,
                     aTextColorRole);
      }

      break;
    }

    default:
      QProxyStyle::drawControl(theElement, theOption, thePainter, theWidget);
      break;
  }
}

//! Provide appropriate size hint for drawing header items with wrapping mode turned on or off.
//! \param theContentType [in] the content type.
//! \param theOption [in] the styling option.
//! \param theContentsSize [in] the contents size.
//! \param theWidget [in] the styling widget.
QSize asiUI_HeaderStyle::sizeFromContents(ContentsType theContentType,
                                           const QStyleOption* theOption,
                                           const QSize& theContentsSize,
                                           const QWidget* theWidget) const
{
  if ( theContentType != CT_HeaderSection )
  {
    return QProxyStyle::sizeFromContents(theContentType, theOption, theContentsSize, theWidget);
  }

  // ========================
  // Extract specific options
  // ========================

  bool    isWordWrap     = false;
  int     aWrapSize      = 0;
  int     aOrientation   = 0;
  double  anAngle        = 0;
  int     aTextAlignment = 0;
  QString aText;

  const asiUI_HeaderStyleOption* aOptions = qstyleoption_cast<const asiUI_HeaderStyleOption*>(theOption);

  if ( aOptions )
  {
    isWordWrap     = aOptions->wordwrap;
    aWrapSize      = aOptions->wrapSize;
    aOrientation   = aOptions->orientation;
    anAngle        = aOptions->angle;
    aTextAlignment = aOptions->textAlignment;
    aText          = aOptions->text;
  }

  // ===================================
  // Prepare size measurement properties
  // ===================================

  // get section size
  int aSectLength = aWrapSize;
  int aSectWidth  = aOrientation == Qt::Horizontal ? aSectLength : 0;
  int aSectHeight = aOrientation == Qt::Vertical   ? aSectLength : 0;
  int aTxtFlags = aTextAlignment;
  if ( isWordWrap )
  {
    aTxtFlags |= Qt::TextWordWrap;
  }
  double anAngleRad = ( anAngle / 180.0 ) * 3.14;
  if ( anAngle != 0.0 )
  {
    // from horizon angles above 45deg switch boundary location
    // horizontal to vertical boundaries and vice-versa
    if ( qAbs( sin( anAngleRad ) ) > 0.707 )
    {
      int aSwitchWidth  = aSectWidth;
      int aSwitchHeight = aSectHeight;
      aSectWidth  = aSwitchHeight;
      aSectHeight = aSwitchWidth;
    }
  }

  // get final section size rect
  QRect aContentRect( QPoint(0, 0), QSize(aSectWidth, aSectHeight) );
  QRect aSubContRect( QPoint(0, 0), QSize(aSectWidth, aSectHeight) );
  if ( aOptions )
  {
    asiUI_HeaderStyleOption aSubOpt(*aOptions);
    aSubOpt.rect = aContentRect;
    aSubContRect = subElementRect(SE_HeaderLabel, &aSubOpt, theWidget);
  }

  QRect aTxtRect = theOption->fontMetrics.boundingRect(aContentRect, aTxtFlags, aText);

  if ( anAngle != 0.0 )
  {
    double aTransformWidth  = aTxtRect.width();
    double aTransformHeight = aTxtRect.height();
    double aCos = qAbs( cos( anAngleRad ) );
    double aSin = qAbs( sin( anAngleRad ) );
    aTxtRect.setWidth ( (int)ceil( aTransformWidth * aCos ) + (int)ceil( aTransformHeight * aSin ) );
    aTxtRect.setHeight( (int)ceil( aTransformWidth * aSin ) + (int)ceil( aTransformHeight * aCos ) );
  }

  QSize aTxtSize = aTxtRect.size();

  // apply margin
  int aVerMargin = aContentRect.width()  - aSubContRect.width();
  int aHorMargin = aContentRect.height() - aSubContRect.height();

  return QSize( aVerMargin + aTxtSize.width(), aHorMargin + aTxtSize.height() );
}
