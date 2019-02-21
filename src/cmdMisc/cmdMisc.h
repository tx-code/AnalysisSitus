//-----------------------------------------------------------------------------
// Created on: 23 August 2017
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

#ifndef cmdMisc_h
#define cmdMisc_h

#define cmdMisc_NotUsed(x) x

#ifdef cmdMisc_EXPORTS
  #define cmdMisc_EXPORT __declspec(dllexport)
#else
  #define cmdMisc_EXPORT __declspec(dllimport)
#endif

//-----------------------------------------------------------------------------

// asiTcl includes
#include <asiTcl_Interp.h>

#ifdef USE_MOBIUS
  #include <mobius/geom_Surface.h>
#endif

//-----------------------------------------------------------------------------

//! Miscellaneous commands. This package is an ideal fit for those commands
//! which are not mature enough to be passed to cmdEngine module. Basically,
//! you are encouraged to put here experimental stuff to see how it goes and
//! decide whether to industrialize your work or throw it away.
class cmdMisc
{
public:

  cmdMisc_EXPORT static void
    Factory(const Handle(asiTcl_Interp)&      interp,
            const Handle(Standard_Transient)& data);

#ifdef USE_MOBIUS
public:

  cmdMisc_EXPORT static void
    DrawSurfPts(const Handle(asiTcl_Interp)&             interp,
                const mobius::ptr<mobius::geom_Surface>& surface,
                const TCollection_AsciiString&           name);
#endif

public:

  cmdMisc_EXPORT static void
    Commands_Coons(const Handle(asiTcl_Interp)&      interp,
                   const Handle(Standard_Transient)& data);

};

#endif
