//-----------------------------------------------------------------------------
// Created on: 05 November 2023
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
#include <asiTest_ReapproxContour.h>

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::runTestScript(const int   funcID,
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

outcome asiTest_ReapproxContour::test001(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_01.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test002(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_02.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test003(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_03.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test004(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_04.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test005(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_05.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test006(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_06.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test007(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_07.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_ReapproxContour::test008(const int funcID, const bool)
{
  return runTestScript(funcID, "re/approx-contour_08.tcl");
}
