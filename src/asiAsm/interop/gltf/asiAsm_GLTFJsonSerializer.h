//-----------------------------------------------------------------------------
// Created on: 08 January 2021
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

#pragma once

// asiAsm includes
#include <asiAsm.h>

// Rapidjson includes (should be hidden in cpp)
#if defined USE_RAPIDJSON
  #include <rapidjson/prettywriter.h>
  #include <rapidjson/ostreamwrapper.h>
#endif

namespace asiAsm {
namespace xde {

//! JSON serializer declared in AS header to avoid inclusion of rapidjson header
//! files in the dependent libs. This `glTFJsonSerializer.h` should be
//! included in cpp files only.
//!
//! We use "pretty writer" here to make glTF's JSON files human-readable (i.e.,
//! have all these whitespaces and newlines).
class glTFJsonSerializer : public rapidjson::PrettyWriter<rapidjson::OStreamWrapper>
{
public:

  //! Ctor accepting the output stream.
  //! \param[in,out] ostr the output stream to serialize data into.
  glTFJsonSerializer(rapidjson::OStreamWrapper& ostr)
  : rapidjson::PrettyWriter<rapidjson::OStreamWrapper>(ostr)
  {}

};

} // xde
} // asiAsm
