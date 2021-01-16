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

#ifndef asiTcl_Variable_h
#define asiTcl_Variable_h

// asiTcl includes
#include <asiTcl.h>

// OpenCascade includes
#include <Standard_Type.hxx>

//-----------------------------------------------------------------------------

//! Variable in a Tcl session.
class asiTcl_Variable : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiTcl_Variable, Standard_Transient)

public:

  //! Ctor.
  asiTcl_EXPORT
    asiTcl_Variable();

public:

  //! Sets variable name.
  //! \param[in] name the name to set.
  asiTcl_EXPORT void
    SetName(const std::string& name);

  //! \return variable name.
  asiTcl_EXPORT const std::string&
    GetName() const;

public:

  //! \return brief description "what is" this object.
  virtual std::string WhatIs() const = 0;

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  virtual void Dump(std::ostream& out) const = 0;

protected:

  std::string m_name; //!< Name of the variable.

};

#endif
