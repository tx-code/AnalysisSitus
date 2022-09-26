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

#ifndef asiTest_ComputeNegativeVolume_HeaderFile
#define asiTest_ComputeNegativeVolume_HeaderFile

// asiTest includes
#include <asiTest_CaseIDs.h>
#include <asiTest_TclTestCase.h>

//! Test functions for calculating negative volumes.
class asiTest_ComputeNegativeVolume : public asiTest_TclTestCase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_ComputeNegativeVolume;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "asiTest_ComputeNegativeVolume";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "inspection";
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param[out] functions output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &test_negative_volume_1
              << &test_negative_volume_2
              << &test_negative_volume_3
              << &test_negative_volume_4
              << &test_negative_volume_5
              << &test_negative_volume_6
              << &test_negative_volume_7
              << &test_negative_volume_8
              << &test_negative_volume_9
              << &test_negative_volume_10
              << &test_negative_volume_11
              << &test_negative_volume_12
              << &test_negative_volume_13
              << &test_negative_volume_14
              << &test_negative_volume_15
              << &test_negative_volume_16
              << &test_negative_volume_17
              << &test_negative_volume_18
              << &test_negative_volume_19
              << &test_negative_volume_20
              << &test_negative_volume_21
              << &test_negative_volume_22
              << &test_negative_volume_23
              << &test_negative_volume_24
              << &test_negative_volume_25
              << &test_negative_volume_26
              << &test_negative_volume_27
              << &test_negative_volume_28
              << &test_negative_volume_29
              << &test_negative_volume_30
              << &test_negative_volume_31
              << &test_negative_volume_32
              << &test_negative_volume_33
              << &test_negative_volume_34
              << &test_negative_volume_35
              << &test_negative_volume_36
              << &test_negative_volume_37
              << &test_negative_volume_38
              << &test_negative_volume_39
              << &test_negative_volume_40
              << &test_negative_volume_41
              << &test_negative_volume_42
              << &test_negative_volume_43
              << &test_negative_volume_44
              << &test_negative_volume_45
              << &test_negative_volume_46
              << &test_negative_volume_47
              << &test_negative_volume_48
              << &test_negative_volume_49
              << &test_negative_volume_50
              << &test_negative_volume_51
              << &test_negative_volume_52
              << &test_negative_volume_53
              << &test_negative_volume_54
              << &test_negative_volume_55
              << &test_negative_volume_56
              << &test_negative_volume_57
              << &test_negative_volume_58
              << &test_negative_volume_59
              << &test_negative_volume_60
              << &test_negative_volume_61
              << &test_negative_volume_62
              << &test_negative_volume_63
              << &test_negative_volume_64
    ; // Put semicolon here for convenient adding new functions above ;)
  }

private:

  static outcome runTestScript(const int   funcID,
                               const char* filename);

  static outcome test_negative_volume_1(const int funcID);
  static outcome test_negative_volume_2(const int funcID);
  static outcome test_negative_volume_3(const int funcID);
  static outcome test_negative_volume_4(const int funcID);
  static outcome test_negative_volume_5(const int funcID);
  static outcome test_negative_volume_6(const int funcID);
  static outcome test_negative_volume_7(const int funcID);
  static outcome test_negative_volume_8(const int funcID);
  static outcome test_negative_volume_9(const int funcID);
  static outcome test_negative_volume_10(const int funcID);
  static outcome test_negative_volume_11(const int funcID);
  static outcome test_negative_volume_12(const int funcID);
  static outcome test_negative_volume_13(const int funcID);
  static outcome test_negative_volume_14(const int funcID);
  static outcome test_negative_volume_15(const int funcID);
  static outcome test_negative_volume_16(const int funcID);
  static outcome test_negative_volume_17(const int funcID);
  static outcome test_negative_volume_18(const int funcID);
  static outcome test_negative_volume_19(const int funcID);
  static outcome test_negative_volume_20(const int funcID);
  static outcome test_negative_volume_21(const int funcID);
  static outcome test_negative_volume_22(const int funcID);
  static outcome test_negative_volume_23(const int funcID);
  static outcome test_negative_volume_24(const int funcID);
  static outcome test_negative_volume_25(const int funcID);
  static outcome test_negative_volume_26(const int funcID);
  static outcome test_negative_volume_27(const int funcID);
  static outcome test_negative_volume_28(const int funcID);
  static outcome test_negative_volume_29(const int funcID);
  static outcome test_negative_volume_30(const int funcID);
  static outcome test_negative_volume_31(const int funcID);
  static outcome test_negative_volume_32(const int funcID);
  static outcome test_negative_volume_33(const int funcID);
  static outcome test_negative_volume_34(const int funcID);
  static outcome test_negative_volume_35(const int funcID);
  static outcome test_negative_volume_36(const int funcID);
  static outcome test_negative_volume_37(const int funcID);
  static outcome test_negative_volume_38(const int funcID);
  static outcome test_negative_volume_39(const int funcID);
  static outcome test_negative_volume_40(const int funcID);
  static outcome test_negative_volume_41(const int funcID);
  static outcome test_negative_volume_42(const int funcID);
  static outcome test_negative_volume_43(const int funcID);
  static outcome test_negative_volume_44(const int funcID);
  static outcome test_negative_volume_45(const int funcID);
  static outcome test_negative_volume_46(const int funcID);
  static outcome test_negative_volume_47(const int funcID);
  static outcome test_negative_volume_48(const int funcID);
  static outcome test_negative_volume_49(const int funcID);
  static outcome test_negative_volume_50(const int funcID);
  static outcome test_negative_volume_51(const int funcID);
  static outcome test_negative_volume_52(const int funcID);
  static outcome test_negative_volume_53(const int funcID);
  static outcome test_negative_volume_54(const int funcID);
  static outcome test_negative_volume_55(const int funcID);
  static outcome test_negative_volume_56(const int funcID);
  static outcome test_negative_volume_57(const int funcID);
  static outcome test_negative_volume_58(const int funcID);
  static outcome test_negative_volume_59(const int funcID);
  static outcome test_negative_volume_60(const int funcID);
  static outcome test_negative_volume_61(const int funcID);
  static outcome test_negative_volume_62(const int funcID);
  static outcome test_negative_volume_63(const int funcID);
  static outcome test_negative_volume_64(const int funcID);

};

#endif
