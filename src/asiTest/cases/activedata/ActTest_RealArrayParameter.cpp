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
#include <ActTest_RealArrayParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs test on accessing value of RealArrayParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_RealArrayParameter::accessValue(const int asiTestEngine_NotUsed(funcID))
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new REAL ARRAY Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_RealArrayParameter)
    param = ActParamTool::AsRealArray( createParameter(doc, Parameter_RealArray) );
  doc->CommitCommand();

  // Parameter is not well-formed as it does not have value Attribute yet
  TEST_VERIFY( !param->IsWellFormed() )

  /* =======================================
   *  Initialize input array with test data
   * ======================================= */

  const Standard_Integer NB_ELEMS = 1000;
  Handle(HRealArray) anArray_TO = new HRealArray(0, NB_ELEMS - 1);

  for ( Standard_Integer i = 0; i < NB_ELEMS; i++ )
    anArray_TO->SetValue(i, (NB_ELEMS - i) * 0.5);

  /* =====================
   *  Set Parameter value
   * ===================== */

  TCollection_AsciiString
    aMsg = TCollection_AsciiString("Setting a test array of ")
                              .Cat(NB_ELEMS)
                              .Cat(" elements");

  doc->NewCommand();
  TEST_PRINT_DECOR_L( aMsg.ToCString() );
  param->SetArray(anArray_TO);
  doc->CommitCommand();

  // Now we expect the Parameter to become well-formed
  TEST_VERIFY( param->IsWellFormed() )

  /* ==========================================================
   *  Access array FROM CAF structure. Perform validations on:
   *  - the number of elements;
   *  - the entire array;
   * ========================================================== */

  // Access number of elements
  Standard_Integer aNbElems_FROM = param->NbElements();

  // Check if the number of elements is as expected
  TEST_VERIFY(NB_ELEMS == aNbElems_FROM)

  // Access array
  Handle(HRealArray) anArray_FROM = param->GetArray();

  // Check array element-by-element
  for ( Standard_Integer i = anArray_FROM->Lower(); i <= anArray_FROM->Upper(); i++ )
  {
    const Standard_Real aValue_TO = anArray_TO->Value(i);
    const Standard_Real aValue_FROM = anArray_FROM->Value(i);

    // Exact equality is expected
    TEST_VERIFY(aValue_TO == aValue_FROM)
  }

  return outcome().success();
}

//! Performs test on accessing particular elements of RealArrayParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_RealArrayParameter::accessElements(const int asiTestEngine_NotUsed(funcID))
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new REAL ARRAY Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_RealArrayParameter)
    param = ActParamTool::AsRealArray( createParameter(doc, Parameter_RealArray) );
  doc->CommitCommand();

  // Parameter is not well-formed as it does not have value Attribute yet
  TEST_VERIFY( !param->IsWellFormed() )

  /* =======================================
   *  Initialize input array with test data
   * ======================================= */

  const Standard_Integer NB_ELEMS = 2;
  Handle(HRealArray) anArray_TO = new HRealArray(0, NB_ELEMS - 1);

  anArray_TO->SetValue(0, 0.0);
  anArray_TO->SetValue(1, 0.0);
  
  /* =====================
   *  Set Parameter value
   * ===================== */

  TCollection_AsciiString
    aMsg = TCollection_AsciiString("Setting a test array of ")
                              .Cat(NB_ELEMS)
                              .Cat(" elements");

  doc->NewCommand();
  TEST_PRINT_DECOR_L( aMsg.ToCString() );
  param->SetArray(anArray_TO);
  doc->CommitCommand();

  // Now we expect the Parameter to become well-formed
  TEST_VERIFY( param->IsWellFormed() )

  /* ========================
   *  Update Parameter value
   * ======================== */

  doc->NewCommand();
  param->SetElement(0, 5.5);
  param->SetElement(1, 11.5);
  doc->CommitCommand();

  /* =====================
   *  Perform validations
   * ===================== */

  TEST_VERIFY(param->NbElements()  == NB_ELEMS)
  TEST_VERIFY(param->GetElement(0) == 5.5)
  TEST_VERIFY(param->GetElement(1) == 11.5)

  return outcome().success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
