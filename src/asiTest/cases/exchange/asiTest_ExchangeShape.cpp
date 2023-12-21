//-----------------------------------------------------------------------------
// Created on: 28 June 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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
#include <asiTest_ExchangeShape.h>

//-----------------------------------------------------------------------------

outcome asiTest_ExchangeShape::runTestScript(const int   funcID,
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

//! Test scenario 1.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_load_step_1(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/STEP/step_read_1.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 2.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_write_step_2(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/STEP/step_write_2.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 3.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_write_step_3(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/STEP/step_write_3.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 4.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_load_iges_1(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/IGES/iges_read_1.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 5.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_load_brep_1(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/BREP/brep_read_1.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 6.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_write_brep_2(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/BREP/brep_write_2.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 7.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ExchangeShape::test_write_brep_3(const int funcID, const bool)
{
  return runTestScript(funcID, "exchange/BREP/brep_write_3.tcl");
}
