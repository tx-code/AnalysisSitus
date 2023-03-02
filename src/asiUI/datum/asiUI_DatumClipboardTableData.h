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

#ifndef asiUI_DatumClipboardTableData_HeaderFile
#define asiUI_DatumClipboardTableData_HeaderFile

// asiUI includes
#include <asiUI.h>

// Qt includes
#pragma warning(push, 0)
#include <QVector>
#include <QVariant>
#pragma warning(pop)

//! \ingroup GUI
//!
//! Class representing intermediate buffer for clipboard copy/paste operation.
//! Converts tabulation-newline formatted text into a data array for
//! populating table widgets and provides a backward conversion as well.
class asiUI_DatumClipboardTableData
{
public:

  typedef QVector<QVariant> VariantArray;

public:

  asiUI_EXPORT
    asiUI_DatumClipboardTableData(const QString& theTSVString);

  asiUI_EXPORT
    asiUI_DatumClipboardTableData(const int theRows, const int theCols);

  asiUI_EXPORT
    asiUI_DatumClipboardTableData(const asiUI_DatumClipboardTableData& theOther);

  asiUI_EXPORT
    QString ToTSVString() const;

  asiUI_EXPORT
    void FromTSVString(const QString& theTSVString);

  asiUI_EXPORT
    QVariant GetData(const int theRow, const int theCol) const;

  asiUI_EXPORT
    void SetData(const int theRow,
                 const int theCol,
                 const QVariant& theData);

  asiUI_EXPORT
    int GetRowCount() const;

  asiUI_EXPORT
    int GetColumnCount() const;

  asiUI_EXPORT
    const VariantArray& GetRow(const int theRow) const;

  asiUI_EXPORT
    VariantArray& operator[] (int theRow);

  asiUI_EXPORT
    const VariantArray& operator[] (int theRow) const;

  //! Check if the table data is empty.
  inline bool IsEmpty() const
  {
    return ( GetRowCount() == 0 );
  }

  asiUI_EXPORT bool
    DumpToCSV(const QString& theFilename) const;

  asiUI_EXPORT void
    operator= (const asiUI_DatumClipboardTableData& theOther);

private:

  QVector<VariantArray> m_Data; //!< Tabulated data.
};

#endif
