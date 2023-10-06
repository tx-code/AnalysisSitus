//-----------------------------------------------------------------------------
// Created on: 17 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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
#include <ActTest_TimeStamp.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs complex test on TimeStamp functionality.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_TimeStamp::cumulTest(const int funcID, const bool)
{
  TEST_PRINT_DECOR_L("Generating UniqueTime structure for current time")

  Handle(ActAux_TimeStamp) aTS1 = ActAux_TimeStampTool::Generate();

  TEST_PRINT_DECOR_L("Sleep for a while just for clear experiment")

#ifdef _WIN32
  Sleep(100); // msec
#else
  usleep(100000); // mksec
#endif

  TEST_PRINT_DECOR_L("Convert initial UniqueTime structure to CAF-suitable array")

  Handle(HIntArray) aCAFSuitedTM = ActAux_TimeStampTool::AsChunked(aTS1);

  TEST_PRINT_DECOR_L("Convert CAF-suitable array back to UniqueTime structure")

  Handle(ActAux_TimeStamp) aTS2 = ActAux_TimeStampTool::FromChunked(aCAFSuitedTM);

  // Take advantage of overloaded equality operator
  TEST_VERIFY( aTS1->IsEqual(aTS2), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//! Performs a simple test on accessing modification time for
//! Integer Parameters.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_TimeStamp::accessMTime(const int funcID, const bool)
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  /* ===========================
   *  Create Integer Parameters
   * =========================== */

  doc->NewCommand();
  Handle(ActAPI_IUserParameter) aParam1 = createParameter(doc, Parameter_Int);
  Handle(ActAPI_IUserParameter) aParam2 = createParameter(doc, Parameter_Int);
  doc->CommitCommand();

  Handle(ActData_IntParameter)
    anIntParam1 = Handle(ActData_IntParameter)::DownCast(aParam1);
  Handle(ActData_IntParameter)
    anIntParam2 = Handle(ActData_IntParameter)::DownCast(aParam2);

  /* ===============================================================
   *  Set some values and read the correspondent modification times
   * =============================================================== */

  doc->NewCommand();
  anIntParam1->SetValue(0);
  anIntParam2->SetValue(0);
  doc->CommitCommand();

  Handle(ActAux_TimeStamp) aTSFor1 = anIntParam1->GetMTime();
  Handle(ActAux_TimeStamp) aTSFor2 = anIntParam2->GetMTime();

  TEST_VERIFY( aTSFor1->IsLess(aTSFor2), DescriptionFn(), funcID )

  doc->NewCommand();
  anIntParam2->SetValue(0);
  anIntParam1->SetValue(0);
  doc->CommitCommand();

  aTSFor1 = anIntParam1->GetMTime();
  aTSFor2 = anIntParam2->GetMTime();

  TEST_VERIFY( aTSFor1->IsGreater(aTSFor2), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
