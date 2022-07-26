//-----------------------------------------------------------------------------
// Created on: 25 June 2022
// Created by: Andrey Voevodin
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

#ifndef cmdTest_h
#define cmdTest_h

#define cmdTest_NotUsed(x)

#ifdef _WIN32
  #ifdef cmdTest_EXPORTS
    #define cmdTest_EXPORT __declspec(dllexport)
  #else
    #define cmdTest_EXPORT __declspec(dllimport)
  #endif
#else
  #define cmdTest_EXPORT
#endif

//-----------------------------------------------------------------------------

// asiTcl includes
#include <asiTcl_Interp.h>

// asiEngine includes
#include <asiEngine_Model.h>

//-----------------------------------------------------------------------------

//! Tcl commands for working with CAD assemblies.
class cmdTest
{
public:

  //! Entry point to the plugin.
  //! \param[in] interp the Tcl interpretor.
  //! \param[in] data   the passed client's data.
  cmdTest_EXPORT static void
    Factory(const Handle(asiTcl_Interp)&      interp,
            const Handle(Standard_Transient)& data);

public:

  //! Tcl commands for working with mesh.
  //! \param[in] interp the Tcl interpretor.
  //! \param[in] data   the passed client's data.
  cmdTest_EXPORT static void
    Commands_Mesh(const Handle(asiTcl_Interp)&      interp,
                  const Handle(Standard_Transient)& data);

  //! Tcl commands for working with shape.
  //! \param[in] interp the Tcl interpretor.
  //! \param[in] data   the passed client's data.
  cmdTest_EXPORT static void
    Commands_Shape(const Handle(asiTcl_Interp)&      interp,
                   const Handle(Standard_Transient)& data);

public:

  static Handle(asiEngine_Model) model; //!< Data Model instance.

};

#endif
