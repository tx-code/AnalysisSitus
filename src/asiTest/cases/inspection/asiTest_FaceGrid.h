//-----------------------------------------------------------------------------
// Created on: 12 May 2022
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

#ifndef asiTest_FaceGrid_HeaderFile
#define asiTest_FaceGrid_HeaderFile

// asiTest includes
#include <asiTest_CaseIDs.h>
#include <asiTest_TclTestCase.h>

//! Test functions for constructing face grid.
class asiTest_FaceGrid : public asiTest_TclTestCase
{
public:

  //! Returns Test Case ID.
  //! \return ID of the Test Case.
  static int ID()
  {
    return CaseID_FaceGrid;
  }

  //! Returns filename for the description.
  //! \return filename for the description of the Test Case.
  static std::string DescriptionFn()
  {
    return "asiTest_FaceGrid";
  }

  //! Returns Test Case description directory.
  //! \return description directory for the Test Case.
  static std::string DescriptionDir()
  {
    return "inspection";
  }

  //! Returns the IDs of the test cases to generate reference data for.
  static void GenRefIds(std::set<int>& genrefIds)
  {
    (void)genrefIds;
  }

  //! Returns pointers to the Test Functions to launch.
  //! \param[out] functions output collection of pointers.
  static void Functions(asiTestFunctions& functions)
  {
    functions << &test_face_grid_01
              << &test_face_grid_02
              << &test_face_grid_03
              << &test_face_grid_04
              << &test_face_grid_05
              << &test_face_grid_06
              << &test_face_grid_07
              << &test_face_grid_08
              << &test_face_grid_09
              << &test_face_grid_10
    ; // Put semicolon here for convenient adding new functions above ;)
  }

private:

  static outcome runTestScript(const int   funcID,
                               const char* filename);

  static outcome test_face_grid_01(const int funcID, const bool);
  static outcome test_face_grid_02(const int funcID, const bool);
  static outcome test_face_grid_03(const int funcID, const bool);
  static outcome test_face_grid_04(const int funcID, const bool);
  static outcome test_face_grid_05(const int funcID, const bool);
  static outcome test_face_grid_06(const int funcID, const bool);
  static outcome test_face_grid_07(const int funcID, const bool);
  static outcome test_face_grid_08(const int funcID, const bool);
  static outcome test_face_grid_09(const int funcID, const bool);
  static outcome test_face_grid_10(const int funcID, const bool);

};

#endif