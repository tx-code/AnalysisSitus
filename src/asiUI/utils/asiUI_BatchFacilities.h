//-----------------------------------------------------------------------------
// Created on: 19 April 2020
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

#ifndef asiUI_BatchFacilities_h
#define asiUI_BatchFacilities_h

// asiUI includes
#include <asiUI_BatchNotifier.h>

// asiEngine includes
#include <asiEngine_Model.h>

// asiTcl includes
#include <asiTcl_Interp.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>

//-----------------------------------------------------------------------------

//! Common tools and objects for batch execution.
class asiUI_BatchFacilities : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiUI_BatchFacilities, Standard_Transient)

public:

  Handle(asiEngine_Model) Model;    //!< Data Model instance.
  Handle(asiTcl_Interp)   Interp;   //!< Tcl interpreter.
  Handle(ActAPI_ILogger)  Logger;   //!< Logger.
  ActAPI_ProgressEntry    Progress; //!< Progress notifier.
  ActAPI_PlotterEntry     Plotter;  //!< Imperative plotter.

public:

  //! \return single instance of facilities.
  asiUI_EXPORT static Handle(asiUI_BatchFacilities)
    Instance(const bool initBatch = true,
             const bool initInterp = true,
             const bool overrideTclChannels = false);

protected:

  //! Ctor.
  asiUI_EXPORT
    asiUI_BatchFacilities(const bool initBatch = true,
                          const bool initInterp = true,
                          const bool overrideTclChannels = false);

};

#endif
