//-----------------------------------------------------------------------------
// Created on: 06 October 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiTest_ConvertCanonical.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::runTestScript(const int   funcID,
                                                const char* filename)
{
  // Get filename of script to execute.
  TCollection_AsciiString fullFilename = GetFilename(filename);

  // Execute test script.
  outcome res = evaluate(fullFilename, DescriptionFn(), funcID);

  // Set description variables.
  SetVarDescr("filename", fullFilename,       ID(), funcID);
  SetVarDescr("time",     res.elapsedTimeSec, ID(), funcID);

  // Return status.
  return res;
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::runTest(const double tol,
                                          const int    funcID,
                                          const char*  shortFilename)
{
  outcome res(DescriptionFn(), funcID);

  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Prepare filename.
  std::string
    filename = asiAlgo_Utils::Str::Slashed( asiAlgo_Utils::Env::AsiTestData() )
             + shortFilename;

  // Read shape.
  TopoDS_Shape shape;
  if ( !asiAlgo_Utils::ReadBRep(filename.c_str(), shape) )
  {
    cf->Progress.SendLogMessage( LogErr(Normal) << "Cannot read file %1."
                                                << filename.c_str() );
    return res.failure();
  }

  // Perform canonical conversion.
  asiAlgo_ConvertCanonicalSummary summary;
  //
  if ( !asiAlgo_Utils::ConvertCanonical(shape, tol, true, summary) )
  {
    cf->Progress.SendLogMessage(LogErr(Normal) << "Canonical conversion failed.");
    return res.failure();
  }

  return res.success();
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test001(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_001.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test002(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_002.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test003(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_003.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test004(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_004.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test005(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_005.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test006(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_006.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test007(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_007.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test008(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_008.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test009(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_009.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test010(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_010.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test011(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_011.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test012(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_012.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test013(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_013.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test014(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_014.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test015(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_015.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test016(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_016.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test017(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_017.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test018(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_018.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test019(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_019.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test020(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_020.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test021(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_021.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ConvertCanonical::test022(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/canrec/canrec_022.tcl");
}
