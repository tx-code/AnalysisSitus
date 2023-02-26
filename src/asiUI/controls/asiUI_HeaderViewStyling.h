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

#ifndef asiUI_HeaderViewStyling_HeaderFile
#define asiUI_HeaderViewStyling_HeaderFile

// asiUI includes
#include <asiUI.h>

// Qt includes
#pragma warning(push, 0)
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOptionHeader>
#include <QStyleOptionViewItem>
#pragma warning(pop)

// --------------------------------------------------------------------------------------
// Style options
// --------------------------------------------------------------------------------------

//! Define optional styling for header elements.
//! Element with caption style will look like usual header section title (e.g. column or row)
//! Element with value style will look like item of a view (e.g. cell of table view)
enum asiUI_HeaderItemStyle
{
  HeaderItemStyle_Caption,
  HeaderItemStyle_Value
};

//! \ingroup GUI
//!
//! Header style item with properties supported by advanced header
//! view, for example wordWrapping, wrapSize.
class asiUI_EXPORT asiUI_HeaderStyleOption : public QStyleOptionHeader
{
public:

  enum StyleOptionVersion { Version = QStyleOptionHeader::Version + 1 };

// Constructor and destructor:
public:

  //! Default constructor.
  asiUI_HeaderStyleOption()
  : QStyleOptionHeader(Version),
    itemStyle(HeaderItemStyle_Caption),
    wordwrap(false),
    wrapSize(0),
    orientation(0),
    angle(0.0)
  {}

  //! Constructor.
  //! \param theItemStyle [in] the header item style.
  //! \param theWordWrap [in] the word wrap flag.
  //! \param theWrapSize [in] the wrapping size.
  //! \param theOrientation [in] the header orientation.
  //! \param theAngle [in] the text rotation angle.
  asiUI_HeaderStyleOption(const asiUI_HeaderItemStyle theStyle,
                           const bool theWordWrap,
                           const int theWrapSize,
                           const int theOrientation,
                           const double theAngle)
  : QStyleOptionHeader(Version),
    itemStyle(theStyle),
    wordwrap(theWordWrap),
    wrapSize(theWrapSize),
    orientation(theOrientation),
    angle(theAngle)
  {}

  //! Copy constructor.
  //! \param theOther [in] the other style.
  asiUI_HeaderStyleOption(const asiUI_HeaderStyleOption& theOther) : QStyleOptionHeader(Version)
  {
    *this = theOther;
  }

  //! Default destructor.
  ~asiUI_HeaderStyleOption()
  {}

public:

  int  itemStyle;
  bool wordwrap;
  int  wrapSize;
  int  orientation;
  double angle;
};

// --------------------------------------------------------------------------------------
// Style
// --------------------------------------------------------------------------------------

//! \ingroup GUI
//!
//! Header drawing style providing support for additional header features, such as
//! word wrapping, wrap sizing, etc.
class asiUI_EXPORT asiUI_HeaderStyle : public QProxyStyle
{
public:

  asiUI_HeaderStyle(QStyle* theStyle);

public:

  void drawControl(QStyle::ControlElement theElement,
                   const QStyleOption* theOption,
                   QPainter* thePainter,
                   const QWidget* theWidget) const;

  QSize sizeFromContents(ContentsType theContentType,
                         const QStyleOption* theOption,
                         const QSize& theContentsSize,
                         const QWidget* theWidget = 0) const;

public:


};

#endif
