//-----------------------------------------------------------------------------
// Created on: 12 September 2022
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
#include <asiTest_ComputeNegativeVolume.h>

//-----------------------------------------------------------------------------

outcome asiTest_ComputeNegativeVolume::runTestScript(const int   funcID,
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
outcome asiTest_ComputeNegativeVolume::test_negative_volume_1(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_1.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 2.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_2(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_2.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 3.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_3(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_3.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 4.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_4(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_4.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 5.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_5(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_5.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 6.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_6(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_6.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 7.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_7(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_7.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 8.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_8(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_8.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 9.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_9(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_9.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 10.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_10(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_10.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 11.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_11(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_11.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 12.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_12(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_12.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 13.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_13(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_13.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 14.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_14(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_14.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 15.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_15(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_15.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 16.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_16(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_16.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 17.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_17(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_17.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 18.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_18(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_18.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 19.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_19(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_19.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 20.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_20(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_20.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 21.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_21(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_21.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 22.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_22(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_22.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 23.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_23(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_23.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 24.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_24(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_24.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 25.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_25(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_25.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 26.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_26(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_26.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 27.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_27(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_27.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 28.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_28(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_28.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 29.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_29(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_29.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 30.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_30(const int funcID)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_30.tcl");
}
