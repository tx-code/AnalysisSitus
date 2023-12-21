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
outcome asiTest_ComputeNegativeVolume::test_negative_volume_1(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_1.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 2.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_2(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_2.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 3.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_3(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_3.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 4.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_4(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_4.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 5.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_5(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_5.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 6.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_6(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_6.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 7.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_7(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_7.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 8.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_8(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_8.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 9.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_9(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_9.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 10.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_10(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_10.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 11.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_11(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_11.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 12.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_12(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_12.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 13.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_13(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_13.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 14.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_14(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_14.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 15.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_15(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_15.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 16.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_16(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_16.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 17.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_17(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_17.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 18.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_18(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_18.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 19.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_19(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_19.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 20.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_20(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_20.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 21.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_21(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_21.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 22.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_22(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_22.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 23.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_23(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_23.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 24.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_24(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_24.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 25.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_25(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_25.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 26.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_26(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_26.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 27.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_27(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_27.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 28.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_28(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_28.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 29.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_29(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_29.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 30.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_30(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_30.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 31.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_31(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_31.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 32.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_32(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_32.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 33.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_33(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_33.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 34.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_34(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_34.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 35.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_35(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_35.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 36.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_36(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_36.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 37.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_37(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_37.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 38.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_38(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_38.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 39.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_39(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_39.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 40.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_40(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_40.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 41.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_41(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_41.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 42.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_42(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_42.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 43.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_43(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_43.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 44.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_44(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_44.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 45.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_45(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_45.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 46.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_46(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_46.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 47.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_47(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_47.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 48.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_48(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_48.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 49.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_49(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_49.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 50.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_50(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_50.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 51.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_51(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_51.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 52.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_52(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_52.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 53.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_53(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_53.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 54.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_54(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_54.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 55.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_55(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_55.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 56.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_56(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_56.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 57.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_57(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_57.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 58.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_58(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_58.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 59.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_59(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_59.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 60.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_60(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_60.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 61.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_61(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_61.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 62.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_62(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_62.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 63.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_63(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_63.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 64.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_64(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_64.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 65.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_65(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_65.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 66.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_66(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_66.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 67.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_67(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_67.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 68.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_68(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_68.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 69.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_69(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_69.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 70.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_70(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_70.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 71.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_71(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_71.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 72.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_72(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_72.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 73.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_73(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_73.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 74.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_74(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_74.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 75.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_75(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_75.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 76.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_76(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_76.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 77.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_77(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_77.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 78.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_78(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_78.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 79.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_79(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_79.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 80.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_80(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_80.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 81.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_81(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_81.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 82.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_82(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_82.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 83.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_ComputeNegativeVolume::test_negative_volume_83(const int funcID, const bool)
{
  return runTestScript(funcID, "inspection/negative-volume/negative_volume_83.tcl");
}
