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

#include <asiUI_DialogCommandsPluginItem.h>
#include <asiUI_DialogCommandsCommandItem.h>
#include <asiUI_DialogCommandsRootItem.h>

//-----------------------------------------------------------------------------

asiUI_TclCommandParsed asiUI_DialogCommandsPluginItem::value(const int rowId) const
{
  asiUI_DialogCommandsRootItemPtr parentItem = itemDynamicCast<asiUI_DialogCommandsRootItem>(parent());
  asiUI_TclPluginToCommands value = parentItem->value(row());

  return value.Commands[rowId];
}

//-----------------------------------------------------------------------------

QVariant asiUI_DialogCommandsPluginItem::initValue(const int role) const
{
  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    if (column() == 0)
    {
      asiUI_DialogCommandsRootItemPtr parentItem = itemDynamicCast<asiUI_DialogCommandsRootItem>(parent());
      asiUI_TclPluginToCommands value = parentItem->value(row());
      return value.PluginName.c_str();
    }
  }
  return QVariant();
}

//-----------------------------------------------------------------------------

int asiUI_DialogCommandsPluginItem::initRowCount() const
{
  asiUI_DialogCommandsRootItemPtr parentItem = itemDynamicCast<asiUI_DialogCommandsRootItem>(parent());
  asiUI_TclPluginToCommands value = parentItem->value(row());
  return (int)value.Commands.size();
}

//-----------------------------------------------------------------------------

asiUI_TreeItemPtr asiUI_DialogCommandsPluginItem::createChild(int row, int column)
{
  return asiUI_DialogCommandsCommandItem::CreateItem(currentItem(), row, column);
}

