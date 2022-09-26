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

#include <asiUI_TreeModel.h>

#include <asiUI_TreeItem.h>

#include <Standard_Transient.hxx>

#pragma warning(push, 0)
#include <QColor>
#pragma warning(pop)

namespace
{
  //-----------------------------------------------------------------------------

  QColor highlightColor()
  {
    return QColor(24, 70, 93);
  }
}

//-----------------------------------------------------------------------------

asiUI_TreeModel::asiUI_TreeModel(QObject* parent)
: QAbstractItemModel(parent)
{
}

//-----------------------------------------------------------------------------

asiUI_TreeItemPtr asiUI_TreeModel::getItemByIndex(const QModelIndex& id)
{
  asiUI_TreeItem* item = (asiUI_TreeItem*)id.internalPointer();
  return asiUI_TreeItemPtr(item);
}

//-----------------------------------------------------------------------------

void asiUI_TreeModel::reset()
{
  for (int colId = 0, nbColumns = columnCount(); colId < nbColumns; colId++)
  {
    asiUI_TreeItemPtr item = rootItem(colId);
    if (item)
      item->reset();
  }
}

//-----------------------------------------------------------------------------

QModelIndex asiUI_TreeModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  asiUI_TreeItemPtr parentItem;
  if (!parent.isValid())
    parentItem = rootItem(column);
  else
    parentItem = getItemByIndex(parent);

  asiUI_TreeItemPtr childItem = parentItem->child(row, column);
  return childItem ? createIndex(row, column, getIndexValue(childItem)) : QModelIndex();
}

//-----------------------------------------------------------------------------

QVariant asiUI_TreeModel::data(const QModelIndex& id, int role) const
{
  if (!id.isValid())
    return QVariant("undefined");

  QVariant itemData = getItemByIndex(id)->data(id, role);

  if (itemData.isNull() &&
      role == Qt::BackgroundRole &&
      m_highlightedIndices.contains(index(id.row(), 0, id.parent())))
  {
    itemData = highlightColor();
  }
  return itemData;
}

//-----------------------------------------------------------------------------

QModelIndex asiUI_TreeModel::parent(const QModelIndex& id) const
{
  if (!id.isValid())
    return QModelIndex();

  asiUI_TreeItemPtr childItem = getItemByIndex(id);
  asiUI_TreeItemPtr parentItem = childItem ? childItem->parent() : asiUI_TreeItemPtr();

  if (!parentItem)
    return QModelIndex();

  return createIndex(parentItem->row(), parentItem->column(), getIndexValue(parentItem));
}

//-----------------------------------------------------------------------------

Qt::ItemFlags asiUI_TreeModel::flags(const QModelIndex& id) const
{
  if (!id.isValid())
  {
    return Qt::ItemFlags();
  }
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

//-----------------------------------------------------------------------------

QVariant asiUI_TreeModel::headerData(int theSection, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QVariant();

  return m_headerValues[theSection];
}

//-----------------------------------------------------------------------------

int asiUI_TreeModel::rowCount(const QModelIndex& parent) const
{
  asiUI_TreeItemPtr parentItem  = !parent.isValid() ? rootItem (0) : getItemByIndex(parent);
  if (!parentItem)
    return 0;

  return parentItem ? parentItem->rowCount() : 0;
}

//-----------------------------------------------------------------------------

int asiUI_TreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return m_headerValues.size();
}

//-----------------------------------------------------------------------------

void asiUI_TreeModel::emitLayoutChanged()
{
  emit layoutChanged();
}

//-----------------------------------------------------------------------------

QModelIndexList asiUI_TreeModel::selected(const QModelIndexList& indices,
                                          const int cellId,
                                          const Qt::Orientation orientation)
{
  QModelIndexList selectedIndex;
  for (QModelIndexList::const_iterator it = indices.begin(); it != indices.end(); it++)
  {
    QModelIndex index = *it;
    if ((orientation == Qt::Horizontal && index.column() == cellId) ||
        (orientation == Qt::Vertical && index.row() == cellId))
      selectedIndex.append (index);
  }
  return selectedIndex;
}

//-----------------------------------------------------------------------------

QModelIndex asiUI_TreeModel::singleSelected(const QModelIndexList& indices,
                                            const int              cellId,
                                            const Qt::Orientation orientation)
{
  QModelIndexList selectedIndex = selected(indices, cellId, orientation);
  return selectedIndex.size() == 1 ? selectedIndex.first() : QModelIndex();
}

//-----------------------------------------------------------------------------

QList<asiUI_TreeItemPtr> asiUI_TreeModel::selectedItems(const QModelIndexList& indices)
{
  QList<asiUI_TreeItemPtr> items;

  for (QModelIndexList::const_iterator it = indices.begin(); it != indices.end(); it++)
  {
    asiUI_TreeItemPtr item = asiUI_TreeModel::getItemByIndex(*it);
    if (!item || items.contains(item))
      continue;
    items.append(item);
  }
  return items;
}

//-----------------------------------------------------------------------------

void asiUI_TreeModel::setRootItem(const int                columnId,
                                  const QString&           sectionName,
                                  const asiUI_TreeItemPtr& rootItem)
{
  if (sectionName.isEmpty())
  {
    // remove section
    m_headerValues.remove(columnId);
    m_rootItems.remove(columnId);
  }

  m_headerValues[columnId] = sectionName;
  m_rootItems.insert(columnId, rootItem);
}

//-----------------------------------------------------------------------------

void* asiUI_TreeModel::getIndexValue(const asiUI_TreeItemPtr& theItem)
{
  return theItem.data();
}
