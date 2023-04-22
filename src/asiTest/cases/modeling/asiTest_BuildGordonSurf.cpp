//-----------------------------------------------------------------------------
// Created on: 17 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
#include <asiTest_BuildGordonSurf.h>

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::runTestScript(const int   funcID,
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

outcome asiTest_BuildGordonSurf::test01(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_01.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test02(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_02.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test03(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_03.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test04(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_04.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test05(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_05.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test06(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_06.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test07(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_07.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test08(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_08.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test09(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_09.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test10(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_10.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_BuildGordonSurf::test11(const int funcID)
{
  return runTestScript(funcID, "re/build-gordon_11.tcl");
}
