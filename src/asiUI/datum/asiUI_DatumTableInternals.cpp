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
#include <asiUI_DatumTable.h>

// Qt includes
#pragma warning(push, 0)
#include <QHeaderView>
#include <QStandardItemModel>
#pragma warning(pop)

// ----------------------------------------------------------------------------
//              internal type: ArrayOfCellDatums
// ----------------------------------------------------------------------------

asiUI_DatumTable::ArrayOfCellDatums::ArrayOfCellDatums()
{
  m_rows = 0;
  m_cols = 0;
}

void asiUI_DatumTable::ArrayOfCellDatums::Resize(const int theRows, const int theCols)
{
  m_rows = theRows;
  m_cols = theCols;
  m_datums.resize(theCols);
  for (int it = 0; it < m_datums.size(); ++it)
  {
    m_datums[it].resize(theRows);
  }
}

void asiUI_DatumTable::ArrayOfCellDatums::InsertRows(const int theAt, const int theRows)
{
  m_rows += theRows;
  for (int it = 0; it < m_datums.size(); ++it)
  {
    m_datums[it].insert( theAt, theRows, QString() );
  }
}

void asiUI_DatumTable::ArrayOfCellDatums::InsertCols(const int theAt, const int theCols)
{
  m_cols += theCols;
  m_datums.insert( theAt, theCols, VectorOfDatums() );
  for (int it = theAt; it < theAt + theCols; ++it)
  {
    m_datums[it].resize(m_rows);
  }
}

void asiUI_DatumTable::ArrayOfCellDatums::RemoveRows(const int theAt, const int theRows)
{
  m_rows = qMax(0, m_rows - theRows);
  for (int it = 0; it < m_datums.size(); ++it)
  {
    m_datums[it].remove(theAt, theRows);
  }
}

void asiUI_DatumTable::ArrayOfCellDatums::RemoveCols(const int theAt, const int theCols)
{
  m_cols = qMax(0, m_cols - theCols);
  m_datums.remove(theAt, theCols);
}

void asiUI_DatumTable::ArrayOfCellDatums::Set(const int theRow, const int theCol, const QString& theDatum)
{
  m_datums[theCol][theRow] = theDatum;
}

QString asiUI_DatumTable::ArrayOfCellDatums::Get(const int theRow, const int theCol) const
{
  if ( !m_datums.size() )
    return QString();

  if ( theRow >= m_datums[0].size() )
    return QString();

  return m_datums[theCol][theRow];
}

// ----------------------------------------------------------------------------
//              internal type: EditorSet
// ----------------------------------------------------------------------------

int asiUI_DatumTable::EditorSet::Rows() const
{
  return m_rowDatums.size();
}

int asiUI_DatumTable::EditorSet::Cols() const
{
  return m_colDatums.size();
}

void asiUI_DatumTable::EditorSet::Resize(const int theRows, const int theCols)
{
  m_rowDatums.resize(theRows);
  m_colDatums.resize(theCols);
  m_itemDatums.Resize(theRows, theCols);
}

void asiUI_DatumTable::EditorSet::InsertRows(const int theAt, const int theRows)
{
  m_rowDatums.insert( theAt, theRows, QString() );
  m_itemDatums.InsertRows(theAt, theRows);
}

void asiUI_DatumTable::EditorSet::InsertCols(const int theAt, const int theCols)
{
  m_colDatums.insert( theAt, theCols, QString() );
  m_itemDatums.InsertCols(theAt, theCols);
}

void asiUI_DatumTable::EditorSet::RemoveRows(const int theAt, const int theRows)
{
  m_rowDatums.remove(theAt, theRows);
  m_itemDatums.RemoveRows(theAt, theRows);
}

void asiUI_DatumTable::EditorSet::RemoveCols(const int theAt, const int theCols)
{
  m_colDatums.remove(theAt, theCols);
  m_itemDatums.RemoveCols(theAt, theCols);
}

void asiUI_DatumTable::EditorSet::SetDefaultEditor(const QString& theDatum)
{
  m_defaultDatum = theDatum;
}

void asiUI_DatumTable::EditorSet::SetRowEditor(const int theRow, const QString& theDatum)
{
  m_rowDatums[theRow] = theDatum;
}

void asiUI_DatumTable::EditorSet::SetColEditor(const int theCol, const QString& theDatum)
{
  m_colDatums[theCol] = theDatum;
}

void asiUI_DatumTable::EditorSet::SetItemEditor(const int theRow, const int theCol, const QString& theDatum)
{
  m_itemDatums.Set(theRow, theCol, theDatum);
}

const QString& asiUI_DatumTable::EditorSet::GetDefaultEditor() const
{
  return m_defaultDatum;
}

QString asiUI_DatumTable::EditorSet::GetRowEditor(const int theRow) const
{
  if ( theRow >= m_rowDatums.size() )
    return QString();
  //
  return m_rowDatums[theRow];
}

QString asiUI_DatumTable::EditorSet::GetColEditor(const int theCol) const
{
  if ( theCol >= m_colDatums.size() )
    return QString();
  //
  return m_colDatums[theCol];
}

QString asiUI_DatumTable::EditorSet::GetEditor(const int theRow, const int theCol) const
{
  const QString& anItemDatum = m_itemDatums.Get(theRow, theCol);
  if ( !anItemDatum.isNull() )
  {
    return anItemDatum;
  }

  if ( theCol >= m_colDatums.size() )
    return QString();
  //
  const QString& aColDatum = m_colDatums[theCol];
  if ( !aColDatum.isNull() )
  {
    return aColDatum;
  }

  if ( theRow >= m_rowDatums.size() )
    return QString();
  //
  const QString& aRowDatum = m_rowDatums[theRow];
  if ( !aRowDatum.isNull() )
  {
    return aRowDatum;
  }

  return m_defaultDatum;
}
