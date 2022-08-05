//-----------------------------------------------------------------------------
// Created on: 06 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Natalia Ermolaeva
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

#include <asiUI_TreeItem.h>

#pragma warning(push, 0)
#include <QStringList>
#pragma warning(pop)

const int asiUI_ItemRole_RowCountRole = Qt::UserRole + 1; //! additional column with row count for item (cached value)

//-----------------------------------------------------------------------------

asiUI_TreeItem::asiUI_TreeItem(asiUI_TreeItemPtr parent,
                               const int         row,
                               const int         column)
 : m_initialized(false)
{
  m_parent = parent;
  m_row    = row;
  m_column = column;
}

//-----------------------------------------------------------------------------

void asiUI_TreeItem::reset()
{
  for (PositionToItemHash::const_iterator it = m_childItems.begin(); it != m_childItems.end(); it++)
  {
    asiUI_TreeItemPtr item = it.value();
    if (item)
      item->reset();
  }
  m_initialized = false;
  m_cachedValues.clear();
}

//-----------------------------------------------------------------------------

asiUI_TreeItemPtr asiUI_TreeItem::child(int        row,
                                        int        column,
                                        const bool isToCreate)
{
  QPair<int, int> aPos = qMakePair(row, column);

  if (m_childItems.contains(aPos))
    return m_childItems[aPos];

  asiUI_TreeItemPtr item;
  if (isToCreate) {
    item = createChild(row, column);

    if (item)
      m_childItems[aPos] = item;
  }
  return item;
}

//-----------------------------------------------------------------------------

QVariant asiUI_TreeItem::data(const QModelIndex& /*index*/, int role) const
{
  return cachedValue(role);
}

//-----------------------------------------------------------------------------

int asiUI_TreeItem::rowCount() const
{
  return cachedValue(asiUI_ItemRole_RowCountRole).toInt();
}

//-----------------------------------------------------------------------------

asiUI_TreeItemPtr asiUI_TreeItem::createChild(int /*row*/, int /*column*/)
{
  return asiUI_TreeItemPtr();
}

//-----------------------------------------------------------------------------

const asiUI_TreeItemPtr asiUI_TreeItem::currentItem()
{
  return asiUI_TreeItemPtr(this);
}

//-----------------------------------------------------------------------------

QVariant asiUI_TreeItem::cachedValue(const int itemRole) const
{
  if (m_cachedValues.contains(itemRole))
    return m_cachedValues[itemRole];

  QVariant valueToCache;
  if (itemRole == asiUI_ItemRole_RowCountRole)
    valueToCache = initRowCount();
  else
    valueToCache = initValue(itemRole);

  m_cachedValues.insert(itemRole, valueToCache);
  return m_cachedValues.contains(itemRole) ? m_cachedValues[itemRole] : QVariant();
}

//-----------------------------------------------------------------------------

void asiUI_TreeItem::init()
{
  m_initialized = true;
}

//-----------------------------------------------------------------------------

QVariant asiUI_TreeItem::initValue(const int /*itemRole*/) const
{
  return QVariant();
}
