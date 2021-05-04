//-----------------------------------------------------------------------------
// Created on: 19 December 2020
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

#ifndef asiAsm_XdeApp_h
#define asiAsm_XdeApp_h

// asiAsm includes
#include <asiAsm.h>

// OCCT includes
#include <XCAFApp_Application.hxx>

//-----------------------------------------------------------------------------

#define BinXCAF "BinXCAF"

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! \ingroup ASIASM
//!
//! XCAF Application to manage XDE Documents.
class App : public XCAFApp_Application
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(App, XCAFApp_Application)

public:

  //! \return the static instance of XCAF Application.
  asiAsm_EXPORT static Handle(App)
    Instance();

  //! \return name of resources file.
  asiAsm_EXPORT virtual const char*
    ResourcesName() override;

protected:

  //! Ctor.
  asiAsm_EXPORT
    App();

};

} // xde
} // asiAsm

#endif
