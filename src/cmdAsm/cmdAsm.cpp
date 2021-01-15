//-----------------------------------------------------------------------------
// Created on: 18 December 2020
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

// cmdAsm includes
#include <cmdAsm.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>
//-----------------------------------------------------------------------------

Handle(asiEngine_Model)        cmdAsm::model = nullptr;
Handle(asiUI_CommonFacilities) cmdAsm::cf    = nullptr;

//-----------------------------------------------------------------------------

void cmdAsm::Factory(const Handle(asiTcl_Interp)&      interp,
                     const Handle(Standard_Transient)& data)
{
  //static const char* group = "cmdAsm";

  /* ==========================
   *  Initialize UI facilities
   * ========================== */

  // Get common facilities.
  Handle(asiUI_CommonFacilities)
    passedCF = Handle(asiUI_CommonFacilities)::DownCast(data);
  //
  if ( passedCF.IsNull() )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "[cmdAsm] UI facilities are not available. GUI may not be updated.");
  else
    cf = passedCF;

  /* ================================
   *  Initialize Data Model instance
   * ================================ */

  model = Handle(asiEngine_Model)::DownCast( interp->GetModel() );
  //
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "[cmdAsm] Data Model instance is null or not of asiEngine_Model kind.");
    return;
  }

  /* =====================
   *  Add custom commands
   * ===================== */

  // Load sub-modules.
  Commands_XDE (interp, data);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdAsm)
