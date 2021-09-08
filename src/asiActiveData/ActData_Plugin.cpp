//-----------------------------------------------------------------------------
// Created on: May 2012
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
#include <ActData_Plugin.h>

// Active Data includes
#include <ActData_BinStorageDriver.h>
#include <ActData_BinRetrievalDriver.h>

// OCCT includes
#include <BinLDrivers.hxx>
#include <Plugin_Macro.hxx>
#include <Standard_GUID.hxx>

#undef COUT_DEBUG

#ifdef _WIN32
#pragma warning(disable: 4190)
#endif

//! Class exporting the entry point into Active Data for dynamic
//! loading of Storage/Retrieval Drivers.
class ActDataDrivers 
{
public:

  ActData_EXPORT static Handle(Standard_Transient)
    Factory(const Standard_GUID& theGUID);

};

static Standard_GUID BinStorageDriver  ("0ac06b28-65c5-428e-86a5-2a35fbe7c4a4");
static Standard_GUID BinRetrievalDriver("0ac06b28-65c5-428e-86a5-2a35fbe7c4a5");

//! Entry point for Plugin. Returns Storage/Retrieval Driver by the passed
//! GUID.
//! \param theGUID [in] Driver's GUID.
//! \return Driver instance.
Handle(Standard_Transient) ActDataDrivers::Factory(const Standard_GUID& theGUID)
{
  if ( theGUID == BinStorageDriver )
  {
#if defined ACT_DEBUG && defined COUT_DEBUG
    std::cout << "ActDataDrivers: BINARY Storage Plugin" << std::endl;
#endif
    static Handle(ActData_BinStorageDriver) DRV_Storage = new ActData_BinStorageDriver();
    return DRV_Storage;
  }

  if ( theGUID == BinRetrievalDriver )
  {
#if defined ACT_DEBUG && defined COUT_DEBUG
    std::cout << "ActDataDrivers: BINARY Retrieval Plugin" << std::endl;
#endif
    static Handle(ActData_BinRetrievalDriver) DRV_Retrieval = new ActData_BinRetrievalDriver();
    return DRV_Retrieval;
  }

  return BinLDrivers::Factory(theGUID);
}

// Declare entry point
PLUGIN(ActDataDrivers)

#ifdef _WIN32
#pragma warning(default: 4190)
#endif
