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

#ifndef asiUI_TreeModel_H
#define asiUI_TreeModel_H

#include <Standard.hxx>
#include <asiUI_TreeItem.h>

#pragma warning(push, 0)
#include <QAbstractItemModel>
#include <QExplicitlySharedDataPointer>
#include <QMap>
#include <QModelIndex>
#include <QVariant>
#include <QVector>
#pragma warning(pop)

//! Customization of Qt abstract model to define tree of items.
//! Each item is a child of asiUI_TreeItem. Items gives the whole information
//! about own parameters and list of sub items.
class asiUI_TreeModel : public QAbstractItemModel
{
public:

  //! Constructor
  //! \param [in] parent the parent object
  Standard_EXPORT asiUI_TreeModel(QObject* parent = 0);

  //! Destructor
  virtual ~asiUI_TreeModel() {}

  //! Returns the item shared pointer by the model index
  //! if it is in the index internal pointer
  //! /param [in] id a model index
  Standard_EXPORT static asiUI_TreeItemPtr getItemByIndex(const QModelIndex& id);

  //! Resets the model items content. Calls the same method of the root item.
  //! It leads to reset of all child items.
  Standard_EXPORT virtual void reset();

  //! Returns the model root item.
  //! It is realized for OCAFBrowser
  asiUI_TreeItemPtr rootItem(const int column) const { return m_rootItems[column]; }

  //! Emits the layoutChanged signal from outside of this class.
  Standard_EXPORT void emitLayoutChanged();

  //! Returns true if the tree view model contains highlighted items. This highlight is set manually.
  bool hasHighlighted() { return !m_highlightedIndices.isEmpty(); }

  //! Sets items of the indices highlighted in the model.
  //! \param [in] indices a list of tree model indices
  void setHighlighted(const QModelIndexList& indices = QModelIndexList()) { m_highlightedIndices = indices; }

  //! Returns the index of the item in the model specified by the given row, column and parent index.
  //! \param [in] row the index row position
  //! \param [in] colummn the index column position
  //! \param [in] parent the parent index
  Standard_EXPORT virtual QModelIndex index(int                row,
                                            int                column,
                                            const QModelIndex& parent = QModelIndex()) const override;

  //! Returns the data stored under the given role for the item referred to by the index.
  //! \param [in] id   a model index
  //! \param [in] role an enumeration value of role for data obtaining
  Standard_EXPORT virtual QVariant data(const QModelIndex& id,
                                        int                role = Qt::DisplayRole) const override;

  //! Returns the parent index by the child index. Founds the item, saved in the index;
  //! obtains the parent item by the item. Create a new index by the item and containing it.
  //! \param [in] id a model index
  Standard_EXPORT virtual QModelIndex parent(const QModelIndex& id) const override;

  //! Returns the item flags for the given index. The base class implementation returns a combination of flags that
  //! enables the item (ItemIsEnabled) and allows it to be selected (ItemIsSelectable)
  //! \param [in] id the model index
  Standard_EXPORT virtual Qt::ItemFlags flags(const QModelIndex& id) const override;

  //! Returns the header data for the given role and section in the header with the specified orientation.
  //! \param [in] section the header section. For horizontal headers - column number, for vertical headers - row number.
  //! \param [in] orientation a header orientation
  //! \param [in] role a data role
  Standard_EXPORT virtual QVariant headerData(int             section,
                                              Qt::Orientation orientation,
                                              int             role = Qt::DisplayRole) const override;

  //! Returns the number of rows under the given parent. When the parent is valid it means that rowCount is returning
  //! the number of children of parent.
  //! \param [in] parent a parent model index
  Standard_EXPORT virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  //! Returns count of columns in the model.
  //! \param [in] parent an index of the parent item
  Standard_EXPORT virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  //! Returns selected items in the cell of given orientation.
  //! \param [in] indices     a container of selected indices
  //! \param [in] cellId      column index if orientation is horizontal, row index otherwise
  //! \param [in] orientation an orientation to apply the cell index
  Standard_EXPORT static QModelIndexList selected(const QModelIndexList& indices,
                                                  const int              cellId,
                                                  const Qt::Orientation  orientation = Qt::Horizontal);

  //! Returns single selected item in the cell of given orientation. If the orientation is Horizontal,
  //! in the cell id column, one row should be selected.
  //! \param [in] indices     a container of selected indices
  //! \param [in] cellId      column index if orientation is horizontal, row index otherwise
  //! \param [in] orientation an orientation to apply the cell index
  Standard_EXPORT static QModelIndex singleSelected(const QModelIndexList& indices,
                                                    const int              cellId,
                                                    const Qt::Orientation  orientation = Qt::Horizontal);

  //! Returns selected tree model items for indices.
  //! \param [in] indices a container of selected indices
  Standard_EXPORT static QList<asiUI_TreeItemPtr> selectedItems(const QModelIndexList& indices);

  //! Sets header properties item.
  //! \param [in] columnId    a column index
  //! \param [in] sectionName a section value
  //! \param [in] rootItem    the root item
  Standard_EXPORT void setRootItem(const int                columnId,
                                   const QString&           sectionName,
                                   const asiUI_TreeItemPtr& rootItem);

protected:
  //! Converts the item shared pointer to void* type
  //! \param [in] item of tree
  Standard_EXPORT static void* getIndexValue(const asiUI_TreeItemPtr& item);

protected:

  QMap<int, asiUI_TreeItemPtr> m_rootItems;          //!< container of root items, for each column own root item.
  QMap<int, QString>           m_headerValues;       //!< container of column id to header name.
  QModelIndexList              m_highlightedIndices; //!< tree model indices that should be visualized as highlighted.
};

#endif
