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
// Web: http://analysissitus.org
//-----------------------------------------------------------------------------

#ifndef ActData_CAFConverter_HeaderFile
#define ActData_CAFConverter_HeaderFile

// Active Data includes
#include <ActData_CAFConverterBase.h>
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_IProgressNotifier.h>

DEFINE_STANDARD_HANDLE(ActData_CAFConverter, Standard_Transient)

//! \ingroup AD_DF
//!
//! Utility class containing conversion methods to support backward
//! compatibility mechanism at the level of the FRAMEWORK.
class ActData_CAFConverter : public Standard_Transient
{
public:

  DEFINE_STANDARD_RTTI_INLINE(ActData_CAFConverter, Standard_Transient)

public:

  ActData_EXPORT
    ActData_CAFConverter(const ActData_ConversionStream& theCStream);

  ActData_EXPORT Standard_Boolean
    Perform(Handle(ActAPI_IModel)& theModel,
            const Standard_Integer theOldVer,
            const Standard_Integer theNewVer,
            const Handle(Message_ProgressIndicator)& theProgress);

private:

  void registerConversionRoutine(const ActData_ConversionTuple& theTuple);

private:

  ActData_ConversionMap m_conversionMap;

};

#endif