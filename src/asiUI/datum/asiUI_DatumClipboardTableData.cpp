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

#include <asiUI_DatumClipboardTableData.h>

// Qt includes
#pragma warning(push, 0)
#include <QFile>
#include <QStringList>
#include <QTextStream>
#pragma warning(pop)

#define COMMA ", "
#define NL "\n"
#define TAB "\t"
#define SPACE " "

//! Constructs table data from a formatted text string.
//! \param theTSVString [in] the TSV-formatted string.
asiUI_DatumClipboardTableData::asiUI_DatumClipboardTableData(const QString& theTSVString)
{
  FromTSVString(theTSVString);
}

//! Constructs table data of a predefined size.
//! \param theRows [in] the number of rows.
//! \param theCols [in] the number of columns.
asiUI_DatumClipboardTableData::asiUI_DatumClipboardTableData(const int theRows, const int theCols)
{
  m_Data.clear();
  m_Data.resize(theRows);

  QVector<VariantArray>::Iterator aDataIt = m_Data.begin();
  for ( ; aDataIt != m_Data.end(); aDataIt++ )
  {
    VariantArray& anArray = *aDataIt;
    anArray.clear();
    anArray.resize(theCols);
  }
}

//! Copy constructor.
//! \param theOther [in] the copied object
asiUI_DatumClipboardTableData::asiUI_DatumClipboardTableData(const asiUI_DatumClipboardTableData& theOther)
{
  const int aRowNb = theOther.GetRowCount();
  m_Data.clear();
  m_Data.resize(aRowNb);

  for ( int i = 0; i < aRowNb; i++ )
    m_Data[i] = theOther[i];
}

//! Assignement operator.
//! \param theOther [in] the copied object.
void asiUI_DatumClipboardTableData::operator = (const asiUI_DatumClipboardTableData& theOther)
{
  const int aRowNb = theOther.GetRowCount();
  m_Data.clear();
  m_Data.resize(aRowNb);

  for ( int i = 0; i < aRowNb; i++ )
    m_Data[i] = theOther[i];
}

//! Converts table data to a TSV format.
//! \return TSV-formatted text string.
QString asiUI_DatumClipboardTableData::ToTSVString() const
{
  QString aString;

  QVector<VariantArray>::ConstIterator aDataIt = m_Data.constBegin();
  for ( ; aDataIt != m_Data.constEnd(); aDataIt++ )
  {
    const VariantArray& aRow = *aDataIt;
    VariantArray::ConstIterator aValIt = aRow.constBegin();
    for ( ; aValIt != aRow.constEnd(); aValIt++ )
    {
      aString += (*aValIt).toString().replace(NL, SPACE);
      aString += TAB;
    }
    aString.chop(1);

    aString += NL;
  }

  return aString;
}

//! Converts formatted string to a table data.
//! \param theString [in] the formatted string.
void asiUI_DatumClipboardTableData::FromTSVString(const QString& theTSVString)
{
  m_Data.clear();

  QString aText = theTSVString;
  if ( aText.endsWith(NL) )
    aText.chop(1);

  QStringList aRows = aText.split(NL);

  int aMaxLength = 0;

  // insert string data
  QStringList::ConstIterator aRowIt = aRows.constBegin();
  for ( ; aRowIt != aRows.constEnd(); aRowIt++ )
  {
    VariantArray aRow = VariantArray();

    QStringList aCols = (*aRowIt).split(TAB);
    if ( aCols.length() > aMaxLength )
      aMaxLength = aCols.length();

    QStringList::ConstIterator aColIt = aCols.constBegin();
    for ( ; aColIt != aCols.constEnd(); aColIt++ )
      aRow.append(*aColIt);

    m_Data.append(aRow);
  }

  // make length consistent
  QVector<VariantArray>::Iterator aDataIt = m_Data.begin();
  for ( ; aDataIt != m_Data.end(); aDataIt++ )
  {
    VariantArray& aRow = *aDataIt;

    while ( aRow.size() < aMaxLength )
      aRow.append(QVariant());
  }
}

//! Gets table data for a specified row and column.
//! \param theRow [in] the row.
//! \param theCol [in] the column.
QVariant asiUI_DatumClipboardTableData::GetData(const int theRow, const int theCol) const
{
  //ASSERT_RAISE(theRow >= 0 && theRow < GetRowCount(),
  //  "Invalid row index specified");
  //ASSERT_RAISE(theCol >= 0 && theCol < GetColumnCount(),
  //  "Invalid column index specified");

  return m_Data[theRow][theCol];
}

//! Sets table data for a specified row and column.
//! \param theRow [in] the row.
//! \param theCol [in] the column.
void asiUI_DatumClipboardTableData::SetData(const int theRow,
                                        const int theCol,
                                        const QVariant& theData)
{
  //ASSERT_RAISE(theRow >= 0 && theRow < GetRowCount(),
  //  "Invalid row index specified");
  //ASSERT_RAISE(theCol >= 0 && theCol < GetColumnCount(),
  //  "Invalid column index specified");

  m_Data[theRow][theCol] = theData;
}

//! Gets number of data rows.
//! \return number of data rows.
int asiUI_DatumClipboardTableData::GetRowCount() const
{
  return m_Data.size();
}

//! Gets number of data columns.
//! \return number of data columns.
int asiUI_DatumClipboardTableData::GetColumnCount() const
{
  return m_Data.size() > 0 ? m_Data[0].size() : 0;
}

//! Gets table data row.
//! \return row data.
const asiUI_DatumClipboardTableData::VariantArray&
  asiUI_DatumClipboardTableData::GetRow(const int theRow) const
{
  return m_Data[theRow];
}

//! Gets table data row.
//! \return row data.
asiUI_DatumClipboardTableData::VariantArray&
  asiUI_DatumClipboardTableData::operator[](int theRow)
{
  return m_Data[theRow];
}

//! Gets table data row.
//! \return row data.
const asiUI_DatumClipboardTableData::VariantArray&
  asiUI_DatumClipboardTableData::operator[] (int theRow) const
{
  return m_Data[theRow];
}

//! Dumps the contents of the buffer to CSV file.
//! \param theFilename [in] CSV filename.
//! \return true in case of success, false -- otherwise.
bool asiUI_DatumClipboardTableData::DumpToCSV(const QString& theFilename) const
{
  QFile FILE(theFilename);
  if ( !FILE.open(QIODevice::WriteOnly | QIODevice::Text) )
    return false;

  /* =======================
   *  Prepare file contents
   * ======================= */

  QString aStringContents;
  int aNbRows = this->GetRowCount();
  int aNbCols = this->GetColumnCount();
  for ( int aRow = 0; aRow < aNbRows; ++aRow )
  {
    for ( int aCol = 0; aCol < aNbCols; ++aCol )
    {
      QVariant aVal = m_Data[aRow][aCol];
      aStringContents = aStringContents.append( aVal.toString() );
      if ( aCol < aNbCols )
        aStringContents = aStringContents.append(COMMA);
    }
    aStringContents = aStringContents.append(NL);
  }

  /* =======================
   *  Dump contents to file
   * ======================= */

  QTextStream STREAM(&FILE);
  STREAM << aStringContents;
  FILE.close();

  return true;
}
