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

#ifndef asiTest_ConvertCanonical_HeaderFile
#define asiTest_ConvertCanonical_HeaderFile

// asiTest includes
#include <asiTest_CaseIDs.h>
#include <asiTest_TclTestCase.h>

//! Test functions for canonical conversion.
class asiTest_ConvertCanonical : public asiTest_TclTestCase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_ConvertCanonical;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "asiTest_ConvertCanonical";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "editing";
  }

  //! Returns the IDs of the test cases to generate reference data for.
  static void GenRefIds(std::set<int>& genrefIds)
  {
    (void) genrefIds;
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param[out] functions output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &test001
              << &test002
              << &test003
              << &test004
              << &test005
              << &test006
              << &test007
              << &test008
              << &test009
              << &test010
              << &test011
              << &test012
              << &test013
              << &test014
              << &test015
              << &test016
              << &test017
              << &test018
              << &test019
              << &test020
              << &test021
              << &test022
    ; // Put semicolon here for convenient adding new functions above ;)
  }

private:

  static outcome runTestScript(const int   funcID,
                               const char* filename);

  static outcome runTest(const double tol,
                         const int    funcID,
                         const char*  shortFilename);

private:

  static outcome test001(const int funcID, const bool);
  static outcome test002(const int funcID, const bool);
  static outcome test003(const int funcID, const bool);
  static outcome test004(const int funcID, const bool);
  static outcome test005(const int funcID, const bool);
  static outcome test006(const int funcID, const bool);
  static outcome test007(const int funcID, const bool);
  static outcome test008(const int funcID, const bool);
  static outcome test009(const int funcID, const bool);
  static outcome test010(const int funcID, const bool);
  static outcome test011(const int funcID, const bool);
  static outcome test012(const int funcID, const bool);
  static outcome test013(const int funcID, const bool);
  static outcome test014(const int funcID, const bool);
  static outcome test015(const int funcID, const bool);
  static outcome test016(const int funcID, const bool);
  static outcome test017(const int funcID, const bool);
  static outcome test018(const int funcID, const bool);
  static outcome test019(const int funcID, const bool);
  static outcome test020(const int funcID, const bool);
  static outcome test021(const int funcID, const bool);
  static outcome test022(const int funcID, const bool);

};

#endif
