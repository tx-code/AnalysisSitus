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

#ifndef cmdAsm_XdeItemFeature_h
#define cmdAsm_XdeItemFeature_h

// cmdAsm includes
#include <cmdAsm.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// asiTcl includes
#include <asiTcl_Variable.h>

//-----------------------------------------------------------------------------

//! Feature faces associated with an XDE assembly item in a Tcl session.
class cmdAsm_XdeItemFeature : public asiTcl_Variable
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(cmdAsm_XdeItemFeature, asiTcl_Variable)

public:

  //! Ctor.
  cmdAsm_EXPORT
    cmdAsm_XdeItemFeature(const asiAsm::xde::AssemblyItemId& item,
                          const asiAlgo_Feature&             indices);

  //! Dtor.
  cmdAsm_EXPORT virtual
    ~cmdAsm_XdeItemFeature();

public:

  cmdAsm_EXPORT const asiAsm::xde::AssemblyItemId&
    GetItem() const;

  cmdAsm_EXPORT void
    SetItem(const asiAsm::xde::AssemblyItemId& item);

  cmdAsm_EXPORT const asiAlgo_Feature&
    GetIndices() const;

  cmdAsm_EXPORT void
    SetIndices(const asiAlgo_Feature& indices);

public:

  //! \return brief description "what is" this object.
  cmdAsm_EXPORT virtual std::string
    WhatIs() const;

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  cmdAsm_EXPORT virtual void
    Dump(std::ostream& out) const;

protected:

  asiAsm::xde::AssemblyItemId m_item;
  asiAlgo_Feature             m_indices;

};

#endif
