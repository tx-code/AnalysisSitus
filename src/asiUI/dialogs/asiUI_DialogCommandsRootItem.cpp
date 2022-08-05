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

#include <asiUI_DialogCommandsRootItem.h>
#include <asiUI_DialogCommandsPluginItem.h>

//-----------------------------------------------------------------------------

void asiUI_DialogCommandsRootItem::setValues(const std::vector<asiUI_TclPluginToCommands>& values)
{
  m_values = values;
}

//-----------------------------------------------------------------------------

asiUI_TclPluginToCommands asiUI_DialogCommandsRootItem::value(const int rowId) const
{
  if (rowId >= m_values.size())
    return asiUI_TclPluginToCommands("", std::vector<asiUI_TclCommandParsed>());

  return m_values[rowId];
}

//-----------------------------------------------------------------------------

QVariant asiUI_DialogCommandsRootItem::initValue(const int /*role*/) const
{
  return QVariant(); // this item is not visible
}

//-----------------------------------------------------------------------------

int asiUI_DialogCommandsRootItem::initRowCount() const
{
  return (int)m_values.size();
}

//-----------------------------------------------------------------------------

asiUI_TreeItemPtr asiUI_DialogCommandsRootItem::createChild(int row, int column)
{
  return asiUI_DialogCommandsPluginItem::CreateItem(currentItem(), row, column);
}

