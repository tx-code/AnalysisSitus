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
#include <ActTest_IntParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//! Performs test on accessing value of IntParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_IntParameter::accessValue(const int funcID)
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new INT Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_IntParameter)
    param = ActParamTool::AsInt( createParameter(doc, Parameter_Int) );
  doc->CommitCommand();

  const Standard_Integer VALUE = 10;

  // Parameter is not well-formed as it does not have value Attribute yet
  TEST_VERIFY( !param->IsWellFormed(), DescriptionFn(), funcID )

  /* =====================
   *  Set Parameter value
   * ===================== */

  TCollection_AsciiString
    aMsg = TCollection_AsciiString("Setting a test value: ")
                              .Cat(VALUE);

  doc->NewCommand();
  TEST_PRINT_DECOR_L( aMsg.ToCString() );
  param->SetValue(VALUE);
  doc->CommitCommand();

  // Now we expect the Parameter to become well-formed
  TEST_VERIFY( param->IsWellFormed(), DescriptionFn(), funcID )

  /* ==========================
   *  Validate Parameter value
   * ========================== */

  // Validate
  TEST_VERIFY( param->GetValue() == VALUE, DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
