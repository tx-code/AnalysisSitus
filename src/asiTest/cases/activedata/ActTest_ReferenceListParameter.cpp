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
#include <ActTest_ReferenceListParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Test function for Reference List Parameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ReferenceListParameter::testReferences(const int asiTestEngine_NotUsed(funcID))
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new REFERENCE LIST Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_ReferenceListParameter)
    param = ActParamTool::AsReferenceList( createParameter(doc, Parameter_ReferenceList) );
  doc->CommitCommand();

  // Root label for Parameter
  TDF_Label label;

  /* ==========================================
   *  Create some other Parameters to refer to
   * ========================================== */

  doc->NewCommand();

  ActAPI_ParameterList PARAMS;
  PARAMS.Append( createParameter(doc, Parameter_Int,         label) );
  PARAMS.Append( createParameter(doc, Parameter_Real,        label) );
  PARAMS.Append( createParameter(doc, Parameter_Bool,        label) );
  PARAMS.Append( createParameter(doc, Parameter_AsciiString, label) );
  PARAMS.Append( createParameter(doc, Parameter_BoolArray,   label) );

  doc->CommitCommand();

  /* =======================
   *  Set up the references
   * ======================= */

  doc->NewCommand();

  param->AddTarget( PARAMS(1) );
  param->AddTarget( PARAMS(2) );
  param->AddTarget( PARAMS(3) );
  param->AddTarget( PARAMS(4) );

  doc->CommitCommand();

  /* =======================
   *  Verify the references
   * ======================= */

  TEST_VERIFY(  param->HasTarget( PARAMS(1) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(2) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(3) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(4) ) )
  TEST_VERIFY( !param->HasTarget( PARAMS(5) ) )

  /* ========================
   *  Remove some references
   * ======================== */

  doc->NewCommand();

  TEST_VERIFY(  param->RemoveTarget( PARAMS(1) ) )
  TEST_VERIFY(  param->RemoveTarget( PARAMS(3) ) )
  TEST_VERIFY( !param->RemoveTarget( PARAMS(5) ) )

  doc->CommitCommand();

  /* ===============================
   *  Verify the rest of references
   * =============================== */

  TEST_VERIFY( !param->HasTarget( PARAMS(1) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(2) ) )
  TEST_VERIFY( !param->HasTarget( PARAMS(3) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(4) ) )
  TEST_VERIFY( !param->HasTarget( PARAMS(5) ) )

  return outcome().success();
}

//! Test function for Reference List Parameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ReferenceListParameter::testReferenceListParameter_SwapTargets(const int asiTestEngine_NotUsed(funcID))
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new REFERENCE LIST Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_ReferenceListParameter)
    param = ActParamTool::AsReferenceList( createParameter(doc, Parameter_ReferenceList) );
  doc->CommitCommand();

  // Root label for Parameter
  TDF_Label label;

  /* ==========================================
   *  Create some other Parameters to refer to
   * ========================================== */

  doc->NewCommand();

  ActAPI_ParameterList PARAMS;
  PARAMS.Append( createParameter(doc, Parameter_Int,         label) );
  PARAMS.Append( createParameter(doc, Parameter_Real,        label) );
  PARAMS.Append( createParameter(doc, Parameter_Bool,        label) );
  PARAMS.Append( createParameter(doc, Parameter_AsciiString, label) );
  PARAMS.Append( createParameter(doc, Parameter_BoolArray,   label) );

  doc->CommitCommand();

  /* =========================================
   *  Add targets to Reference List Parameter
   * ========================================= */

  doc->NewCommand();

  param->AddTarget( PARAMS(1) );
  param->AddTarget( PARAMS(2) );
  param->AddTarget( PARAMS(3) );
  param->AddTarget( PARAMS(4) );

  doc->CommitCommand();

  TEST_VERIFY(  param->HasTarget( PARAMS(1) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(2) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(3) ) )
  TEST_VERIFY(  param->HasTarget( PARAMS(4) ) )
  TEST_VERIFY( !param->HasTarget( PARAMS(5) ) )

  /* =====================
   *  Try to swap targets
   * ===================== */

  doc->NewCommand();

  TEST_VERIFY( param->SwapTargets( 1, 3 ) );
  TEST_VERIFY( param->SwapTargets( 4, 2 ) );

  doc->CommitCommand();

  // check swap (1,3)
  TEST_VERIFY( param->HasTarget( PARAMS(1) ) == 3 )
  TEST_VERIFY( param->HasTarget( PARAMS(3) ) == 1 )

  // check swap (4,2)
  TEST_VERIFY( param->HasTarget( PARAMS(4) ) == 2 )
  TEST_VERIFY( param->HasTarget( PARAMS(2) ) == 4 )

  doc->NewCommand();

  // check swap with unreferenced target
  TEST_VERIFY( !param->SwapTargets( 5, 3 ) )

  doc->CommitCommand();

  return outcome().success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
