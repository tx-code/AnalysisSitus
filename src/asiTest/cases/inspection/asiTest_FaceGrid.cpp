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

// Own include
#include <asiTest_FaceGrid.h>

//-----------------------------------------------------------------------------

outcome asiTest_FaceGrid::runTestScript(const int   funcID,
                                        const char* filename)
{
  // Get filename of script to execute.
  TCollection_AsciiString fullFilename = GetFilename(filename);

  // Execute test script.
  outcome res = evaluate(fullFilename, DescriptionFn(), funcID);

  // Set description variables.
  SetVarDescr("filename", fullFilename, ID(), funcID);
  SetVarDescr("time", res.elapsedTimeSec, ID(), funcID);

  // Return status.
  return res;
}

//-----------------------------------------------------------------------------

//! Test scenario 01.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_01(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_01.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 02.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_02(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_02.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 03.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_03(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_03.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 04.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_04(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_04.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 05.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_05(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_05.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 06.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_06(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_06.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 07.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_07(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_07.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 08.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_08(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_08.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 09.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_09(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_09.tcl");
}

//-----------------------------------------------------------------------------

//! Test scenario 10.
//! \param[in] funcID ID of the Test Function.
//! \return true in case of success, false -- otherwise.
outcome asiTest_FaceGrid::test_face_grid_10(const int funcID)
{
  return runTestScript(funcID, "inspection/face-grid/face_grid_10.tcl");
}
