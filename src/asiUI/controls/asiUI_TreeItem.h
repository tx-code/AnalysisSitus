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

#ifndef asiUI_TreeItem_H
#define asiUI_TreeItem_H

#include <Standard.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Handle.hxx>

#pragma warning(push, 0)
#include <QExplicitlySharedDataPointer>
#include <QHash>
#include <QMap>
#include <QModelIndex>
#include <QObject>
#include <QPair>
#include <QSharedData>
#include <QVariant>
#pragma warning(pop)

class asiUI_TreeItem;

typedef QExplicitlySharedDataPointer<asiUI_TreeItem> asiUI_TreeItemPtr;

//! Declaration of an abstract interface of model item.
class asiUI_TreeItem : public QSharedData
{
public:

  //! Destructor
  virtual ~asiUI_TreeItem() {}

  //! Returns whether the item is already initialized.
  bool isInitialized() const { return m_initialized; }

  //! Sets the item internal initialized state to the true.
  Standard_EXPORT virtual void init();

  //! Resets the item and the child items content. Sets the initialized state to false.
  Standard_EXPORT virtual void reset();

  //! Gets the parent of the item, or asiUI_TreeItemPtr() if it has no parent.
  //! \return pointer to the item
  asiUI_TreeItemPtr parent() const { return m_parent; };

  //! Returns the row of the item in the parent.
  int row() const { return m_row; }

  //! Returns the column of the item in the parent.
  int column() const { return m_column; }

  //! Gets a child tree item in the given position. Find an item in the children hash.
  //! Creates a new child item, if there is no a cached item in the given position.
  //! \param [in] row        the row of the child item
  //! \param [in] column     the column of the child item
  //! \param [in] isToCreate the flag whether the item should be created if it is not created yet
  //! \return the child item or asiUI_TreeItemPtr() if it does not exist
  Standard_EXPORT asiUI_TreeItemPtr child(int        row,
                                          int        column,
                                          const bool isToCreate = true);

  //! Returns the data stored under the given role for the current item.
  //! \param [in] index the item model index
  //! \param [in] role  the item model role
  virtual QVariant data(const QModelIndex& index,
                        int                role = Qt::DisplayRole) const;

  //! Returns number of rows where the children are.
  //! \return the row count
  int rowCount() const;

protected:

  //! Creates a tree item.
  //! \param [in] parent the parent item
  //! \param [in] row    the item row position in the parent item
  //! \param [in] column the item column position in the parent item
  Standard_EXPORT asiUI_TreeItem(asiUI_TreeItemPtr parent,
                                 const int         row,
                                 const int         column);

  //! Initializes the current item.
  virtual void initItem() const {}

  //! Creates a child item in the given position.
  //! \param [in] row    the child row position
  //! \param [in] column the child column position
  //! \return the created item
  virtual asiUI_TreeItemPtr createChild(int row, int column);

  //! Returns the current item wrapped by a shared pointer.
  Standard_EXPORT const asiUI_TreeItemPtr currentItem();

  //! Returns the cached value for the role.
  //! Init the value if it is requested the first time.
  //! \param [in] itemRole a value role
  Standard_EXPORT QVariant cachedValue(const int itemRole) const;

  //! Returns the number of children. It should be reimplemented in child.
  virtual int initRowCount() const = 0;

  //! Returns data value for the role. It should be reimplemented in child.
  //! \param [in] itemRole a value role
  Standard_EXPORT virtual QVariant initValue(const int itemRole) const;

private:

  typedef QHash< QPair<int, int>, asiUI_TreeItemPtr > PositionToItemHash;
  PositionToItemHash          m_childItems;   //!< the hash of item children

  mutable QMap<int, QVariant> m_cachedValues; //!< cached values, should be cleared by reset
  asiUI_TreeItemPtr           m_parent;       //!< the parent item
  int                         m_row;          //!< the item row position in the parent item
  int                         m_column;       //!< the item column position in the parent item
  bool                        m_initialized;  //!< the state whether the item content is already initialized
};

//! Returns an explicitly shared pointer to the pointer held by other, using a
//! dynamic cast to type X to obtain an internal pointer of the appropriate type.
//! If the dynamic_cast fails, the object returned is null.
//! \param [in] item a source item
template <class X, class T> QExplicitlySharedDataPointer<X> itemDynamicCast(const QExplicitlySharedDataPointer<T>& item)
{
  X* ptr = dynamic_cast<X*>(item.data());

  QExplicitlySharedDataPointer<X> result;
  result = ptr;

  return result;
}

#endif
