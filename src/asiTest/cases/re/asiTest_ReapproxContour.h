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

#ifndef asiTest_ReapproxContour_HeaderFile
#define asiTest_ReapproxContour_HeaderFile

// asiTest includes
#include <asiTest_CaseIDs.h>
#include <asiTest_TclTestCase.h>

//! Test functions for reapproximating polygonal contours.
class asiTest_ReapproxContour : public asiTest_TclTestCase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_ReapproxContour;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "asiTest_ReapproxContour";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "re";
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
    ; // Put semicolon here for convenient adding new functions above ;)
  }

private:

  static outcome runTestScript(const int   funcID,
                               const char* filename);

private:

  static outcome test001(const int funcID, const bool);
  static outcome test002(const int funcID, const bool);
  static outcome test003(const int funcID, const bool);
  static outcome test004(const int funcID, const bool);
  static outcome test005(const int funcID, const bool);
  static outcome test006(const int funcID, const bool);
  static outcome test007(const int funcID, const bool);
  static outcome test008(const int funcID, const bool);

};

#endif
