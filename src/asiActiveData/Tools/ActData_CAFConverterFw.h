//-----------------------------------------------------------------------------
// Created on: June 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

#ifndef ActData_CAFConverterFw_HeaderFile
#define ActData_CAFConverterFw_HeaderFile

// Active Data includes
#include <ActData_CAFConverterBase.h>

//! \ingroup AD_DF
//!
//! Versions supported by converter.
enum ActData_VersionLog
{
  VersionLog_Lot1Iteration4 = 0x00400,
  VersionLog_Lot2Iteration1 = 0x00500,
  VersionLog_Lot2Iteration2 = 0x00600,
  VersionLog_Lot2Iteration3 = 0x00700,
  VersionLog_Production1    = 0x00800,
  VersionLog_Production2    = 0x10000,
  VersionLog_Current = VersionLog_Production1
};

//! \ingroup AD_DF
//!
//! Namespace for all available conversion routines.
namespace ActData_ConversionLibrary
{
  Standard_Boolean v040_to_v050(Handle(ActAPI_IModel)& theModel,
                                const Handle(Message_ProgressIndicator)& theProgress);

  Standard_Boolean v050_to_v060(Handle(ActAPI_IModel)& theModel,
                                const Handle(Message_ProgressIndicator)& theProgress);

  Standard_Boolean v060_to_v070(Handle(ActAPI_IModel)& theModel,
                                const Handle(Message_ProgressIndicator)& theProgress);

  Standard_Boolean v070_to_v080(Handle(ActAPI_IModel)& theModel,
                                const Handle(Message_ProgressIndicator)& theProgress);

  Standard_Boolean v080_to_v100(Handle(ActAPI_IModel)& theModel,
                                const Handle(Message_ProgressIndicator)& theProgress);
}

#endif
