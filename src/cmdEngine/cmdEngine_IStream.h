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

#ifndef cmdEngine_IStream_h
#define cmdEngine_IStream_h

// cmdEngine include
#include <cmdEngine.h>

// asiTcl includes
#include <asiTcl_Variable.h>

//-----------------------------------------------------------------------------

//! IStream in a Tcl session.
//! The user is responsible for creating and deleting a stream.
class cmdEngine_IStream : public asiTcl_Variable
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(cmdEngine_IStream, asiTcl_Variable)

public:

  //! Ctor.
  //! \param[in] stream the stream to set.
  cmdEngine_EXPORT
    cmdEngine_IStream(std::istream* stream);

  //! Dtor.
  cmdEngine_EXPORT virtual
    ~cmdEngine_IStream();

public:

  //! Sets stream.
  //! \param[in] stream the stream to set.
  cmdEngine_EXPORT void
    SetStream(std::istream* stream);

  //! \return stream.
  cmdEngine_EXPORT std::istream*
    GetStream() const;

public:

  //! \return brief description "what is" this object.
  cmdEngine_EXPORT virtual std::string
    WhatIs() const;

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  cmdEngine_EXPORT virtual void
    Dump(std::ostream& out) const;

protected:

  std::istream* m_stream; //!< Stream.

};

#endif
