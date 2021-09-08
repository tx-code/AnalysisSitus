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

// Own include
#include <ActData_CAFConverterFw.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_CAFConversionAsset.h>

//! Performs conversion of the passed Data Model from version 0.4.0 to
//! version 0.5.0 from the framework's perspective.
//! \param theModel [in] Data Model to convert.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_ConversionLibrary::v040_to_v050(Handle(ActAPI_IModel)& theModel,
                                                         const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Actualize both framework & application versions by setting their actual values
  ActData_CAFConversionAsset(theModel).ActualizeVersions();

  return Standard_True;
}

//! Performs conversion of the passed Data Model from version 0.5.0 to
//! version 0.6.0 from the framework's perspective.
//! \param theModel [in] Data Model to convert.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_ConversionLibrary::v050_to_v060(Handle(ActAPI_IModel)& theModel,
                                                         const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Actualize both framework & application versions by setting their actual values
  ActData_CAFConversionAsset(theModel).ActualizeVersions();

  return Standard_True;
}

//! Performs conversion of the passed Data Model from version 0.6.0 to
//! version 0.7.0 from the framework's perspective.
//! \param theModel [in] Data Model to convert.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_ConversionLibrary::v060_to_v070(Handle(ActAPI_IModel)& theModel,
                                                         const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Actualize both framework & application versions by setting their actual values
  ActData_CAFConversionAsset(theModel).ActualizeVersions();

  return Standard_True;
}

//! Performs conversion of the passed Data Model from version 0.7.0 to
//! version 0.8.0 from the framework's perspective.
//! \param theModel [in] Data Model to convert.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_ConversionLibrary::v070_to_v080(Handle(ActAPI_IModel)& theModel,
                                                         const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Actualize both framework & application versions by setting their actual values
  ActData_CAFConversionAsset(theModel).ActualizeVersions();

  return Standard_True;
}

//! Performs conversion of the passed Data Model from version 0.8.0 to
//! version 1.0.0 from the framework's perspective.
//! \param theModel [in] Data Model to convert.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_ConversionLibrary::v080_to_v100(Handle(ActAPI_IModel)& theModel,
                                                         const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Actualize both framework & application versions by setting their actual values
  ActData_CAFConversionAsset(theModel).ActualizeVersions();

  return Standard_True;
}
