//-----------------------------------------------------------------------------
// Created on: February 2012
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
#include <ActData_Application.h>

// OCCT includes
#include <TColStd_SequenceOfExtendedString.hxx>

//! Accessor for static instance of CAF Application.
//! \return instance of CAF Application.
Handle(ActData_Application) ActData_Application::Instance()
{
  static Handle(ActData_Application) anApp;
  if ( anApp.IsNull() )
    anApp = new ActData_Application();
  return anApp;
}

//! Default constructor.
ActData_Application::ActData_Application() : TDocStd_Application()
{
}

//! Enumerates the accepted formats which might be used by persistence
//! mechanism.
//! \param theFormats [out] collection of accepted formats to be populated
//!        by this method.
void ActData_Application::Formats(TColStd_SequenceOfExtendedString& theFormats) 
{
  theFormats.Append( TCollection_ExtendedString(ACTBinFormat) );
}

//! Name of the resources file containing descriptions of the accepted
//! formats.
//! \return relative filename.
Standard_CString ActData_Application::ResourcesName()
{
  return Standard_CString("Resources");
}
