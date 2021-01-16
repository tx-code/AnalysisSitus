//-----------------------------------------------------------------------------
// Created on: 18 December 2020
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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

#ifndef cmdAsm_XdeModel_h
#define cmdAsm_XdeModel_h

// cmdAsm includes
#include <cmdAsm.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// asiTcl includes
#include <asiTcl_Variable.h>

//-----------------------------------------------------------------------------

//! XDE model in a Tcl session.
class cmdAsm_XdeModel : public asiTcl_Variable
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(cmdAsm_XdeModel, asiTcl_Variable)

public:

  //! Ctor.
  //! \param[in] doc the document to set.
  cmdAsm_EXPORT
    cmdAsm_XdeModel(const Handle(asiAsm_XdeDoc)& doc = nullptr);

public:

  //! Sets XDE document.
  //! \param[in] doc the document to set.
  cmdAsm_EXPORT void
    SetDocument(const Handle(asiAsm_XdeDoc)& doc);

  //! \return XDE document.
  cmdAsm_EXPORT const Handle(asiAsm_XdeDoc)&
    GetDocument() const;

public:

  //! \return brief description "what is" this object.
  cmdAsm_EXPORT virtual std::string
    WhatIs() const;

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  cmdAsm_EXPORT virtual void
    Dump(std::ostream& out) const;

protected:

  Handle(asiAsm_XdeDoc) m_doc; //!< Owned XDE document.

};

#endif
