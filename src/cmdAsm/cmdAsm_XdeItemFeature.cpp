//-----------------------------------------------------------------------------
// Created on: 24 August 2023
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

// Own include
#include <cmdAsm_XdeItemFeature.h>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

cmdAsm_XdeItemFeature::cmdAsm_XdeItemFeature(const AssemblyItemId&  item,
                                             const asiAlgo_Feature& indices)
: asiTcl_Variable (),
  m_item          (item),
  m_indices       (indices)
{
}

//-----------------------------------------------------------------------------

cmdAsm_XdeItemFeature::~cmdAsm_XdeItemFeature()
{
}

//-----------------------------------------------------------------------------

const AssemblyItemId& cmdAsm_XdeItemFeature::GetItem() const
{
  return m_item;
}

//-----------------------------------------------------------------------------

void cmdAsm_XdeItemFeature::SetItem(const AssemblyItemId& item)
{
  m_item = item;
}

//-----------------------------------------------------------------------------

const asiAlgo_Feature& cmdAsm_XdeItemFeature::GetIndices() const
{
  return m_indices;
}

//-----------------------------------------------------------------------------

void cmdAsm_XdeItemFeature::SetIndices(const asiAlgo_Feature& indices)
{
  m_indices = indices;
}

//-----------------------------------------------------------------------------

std::string cmdAsm_XdeItemFeature::WhatIs() const
{
  return "XDE item feature";
}

//-----------------------------------------------------------------------------

void cmdAsm_XdeItemFeature::Dump(std::ostream& out) const
{
  (void) out; // Do nothing.
}
