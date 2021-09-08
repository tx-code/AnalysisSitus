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

#ifndef ActData_BinStorageDriver_HeaderFile
#define ActData_BinStorageDriver_HeaderFile

// Active Data includes
#include <ActData_Common.h>

// OCCT includes
#include <BinDrivers_DocumentStorageDriver.hxx>
#include <BinMDF_ADriverTable.hxx>
#include <Message_Messenger.hxx>

DEFINE_STANDARD_HANDLE(ActData_BinStorageDriver, BinDrivers_DocumentStorageDriver)

//! \ingroup AD_DF
//!
//! Storage Driver for ACT Document extended with custom Attributes.
class ActData_BinStorageDriver : public BinDrivers_DocumentStorageDriver
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BinStorageDriver, BinDrivers_DocumentStorageDriver)

// Construction:
public:

  ActData_EXPORT ActData_BinStorageDriver();

// Kernel:
public:

  ActData_EXPORT virtual Handle(BinMDF_ADriverTable)
    AttributeDrivers(const Handle(Message_Messenger)& theMsgDriver);

};

#endif
