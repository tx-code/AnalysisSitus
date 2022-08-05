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

#include <asiUI_DialogCommandsCommandItem.h>
#include <asiUI_DialogCommandsPluginItem.h>

//-----------------------------------------------------------------------------

QVariant asiUI_DialogCommandsCommandItem::initValue(const int role) const
{
  if (role == Qt::DisplayRole ||
      role == Qt::EditRole ||
      role == Qt::ToolTipRole)
  {
    asiUI_DialogCommandsPluginItemPtr parentItem = itemDynamicCast<asiUI_DialogCommandsPluginItem>(parent());
    asiUI_TclCommandParsed value = parentItem->value(row());
    switch (column())
    {
      case 0: return value.Name.c_str();
      case 1: return value.Arguments.c_str();
      case 2:
      {
        if (role != Qt::ToolTipRole)
        {
          // remove spaces in the start and end of the help text
          return QString(value.Description.c_str()).simplified();
        }
        else
          return value.Description.c_str();
      }
    }
  }
  return QVariant();
}

//-----------------------------------------------------------------------------

int asiUI_DialogCommandsCommandItem::initRowCount() const
{
  return 0;
}

//-----------------------------------------------------------------------------

asiUI_TreeItemPtr asiUI_DialogCommandsCommandItem::createChild(int /*row*/, int /*column*/)
{
  return asiUI_TreeItemPtr();
}

