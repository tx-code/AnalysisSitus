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
#include <ActTest_NameParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs test on accessing value of NameParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_NameParameter::accessValue(const int asiTestEngine_NotUsed(funcID))
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new NAME Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_NameParameter)
    param = ActParamTool::AsName( createParameter(doc, Parameter_Name) );
  doc->CommitCommand();

  TCollection_ExtendedString VALUE = "TEST_TEST";

  // Parameter is always well-formed
  TEST_VERIFY( param->IsWellFormed() )

  /* =====================
   *  Set Parameter value
   * ===================== */

  TCollection_ExtendedString
    aMsg = TCollection_ExtendedString("Setting a test value: ").Cat(VALUE);

  doc->NewCommand();
  TEST_PRINT_DECOR_L( TCollection_AsciiString(aMsg).ToCString() );
  param->SetValue(VALUE);
  doc->CommitCommand();

  // Parameter is always well-formed
  TEST_VERIFY( param->IsWellFormed() )

  /* ========================
   *  Access Parameter value
   * ======================== */

  TCollection_ExtendedString aString = param->GetValue();

  // Finally, we check if the output is equal to the input to ensure
  // that no corruption has happened during data transferring to/from CAF
  TEST_VERIFY( ActAux::are_equal( TCollection_AsciiString(aString).ToCString(),
                                 TCollection_AsciiString(VALUE).ToCString() ) )

  return outcome().success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
