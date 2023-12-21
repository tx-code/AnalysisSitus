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
#include <ActTest_ReferenceParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Test function for Reference Parameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ReferenceParameter::testReference(const int funcID, const bool)
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new REFERENCE Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_ReferenceParameter)
    param = ActParamTool::AsReference( createParameter(doc, Parameter_Reference) );
  doc->CommitCommand();

  // Root label for Parameter
  TDF_Label label;

  /* ======================================
   *  Create another Parameter to refer to
   * ====================================== */

  doc->NewCommand();

  ActAPI_ParameterList PARAMS;
  PARAMS.Append( createParameter(doc, Parameter_Int,  label) );
  PARAMS.Append( createParameter(doc, Parameter_Real, label) );
  PARAMS.Append( createParameter(doc, Parameter_Bool, label) );

  doc->CommitCommand();

  /* =====================================================
   *  Set up the reference to the 1-st Parameter & verify
   * ===================================================== */

  doc->NewCommand();
  param->SetTarget( PARAMS(1) );
  doc->CommitCommand();

  TEST_VERIFY( param->IsTarget( PARAMS(1) ), DescriptionFn(), funcID )

  doc->NewCommand();
  param->RemoveTarget();
  doc->CommitCommand();

  TEST_VERIFY( param->GetTarget().IsNull(), DescriptionFn(), funcID )

  /* =====================================================
   *  Set up the reference to the 2-nd Parameter & verify
   * ===================================================== */

  doc->NewCommand();
  param->SetTarget( PARAMS(2) );
  doc->CommitCommand();

  TEST_VERIFY( param->IsTarget( PARAMS(2) ), DescriptionFn(), funcID )
  TEST_VERIFY( !param->GetTarget().IsNull(), DescriptionFn(), funcID )

  /* =====================================================
   *  Set up the reference to the 3-rd Parameter & verify
   * ===================================================== */

  doc->NewCommand();
  param->SetTarget( PARAMS(3) );
  doc->CommitCommand();

  TEST_VERIFY( param->IsTarget( PARAMS(3) ), DescriptionFn(), funcID )
  TEST_VERIFY( !param->GetTarget().IsNull(), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
