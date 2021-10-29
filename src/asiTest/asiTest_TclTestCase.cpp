//-----------------------------------------------------------------------------
// Created on: 25 June 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

// Own include
#include <asiTest_TclTestCase.h>

// asiTest includes
#include <asiTest_CommonFacilities.h>

// asiTcl includes
#include <asiTcl_Interp.h>

// OpenCascade includes
#include <OSD_Directory.hxx>
#include <OSD_FileIterator.hxx>

//-----------------------------------------------------------------------------

outcome asiTest_TclTestCase::evaluate(const TCollection_AsciiString& scriptFilename)
{
  outcome res;

  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Execute script.
  const int ret = cf->Interp->Eval( asiTcl_SourceCmd(scriptFilename) );

  return (ret == TCL_OK) ? res.success() : res.failure();
}

//-----------------------------------------------------------------------------

outcome asiTest_TclTestCase::evaluateAll(const TCollection_AsciiString& dir)
{
  outcome res;

  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  int ret = TCL_OK;

  OSD_Path path(dir);
  //
  for ( OSD_FileIterator fi(path, "*.tcl"); fi.More(); fi.Next() )
  {
    OSD_Path filePath;
    fi.Values().Path(filePath);

    TCollection_AsciiString filename;
    filePath.SystemName(filename);
    filename = dir + "/" + filename;

    cf->Progress.SendLogMessage(LogNotice(Normal) << "Next script to execute: '%1'."
                                                  << filename);

    // Execute script.
    ret = cf->Interp->Eval( asiTcl_SourceCmd(filename) );

    // Check result.
    if ( ret != TCL_OK )
    {
      cf->Interp->PrintLastError();
      break;
    }
  }

  return (ret == TCL_OK) ? res.success() : res.failure();
}
