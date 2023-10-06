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
#include <ActTest_SelectionParameter.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs test on accessing map of integer values stored in
//! SelectionParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_SelectionParameter::accessSelectionMask(const int funcID, const bool)
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new SELECTION Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_SelectionParameter)
    param = ActParamTool::AsSelection( createParameter(doc, Parameter_Selection) );
  doc->CommitCommand();

  // Not yet initialized -> BAD-FORMED
  TEST_VERIFY( !param->IsWellFormed(), DescriptionFn(), funcID )

  /* =====================
   *  Prepare a test mask
   * ===================== */

  Standard_Integer MASKED_IDs[] = {10, 20, 30, 40, 15, 25, 35, 45};

  TColStd_PackedMapOfInteger aTestMap;
  for ( Standard_Integer i = 0; i < sizeof(MASKED_IDs)/sizeof(Standard_Integer); i++ )
    aTestMap.Add(MASKED_IDs[i]);
  
  /* =====================
   *  Store the test mask
   * ===================== */

  doc->OpenCommand();
  param->SetMask( new TColStd_HPackedMapOfInteger(aTestMap) );
  doc->CommitCommand();

  /* ================
   *  Verify results
   * ================ */

  // Initialized -> WELL-FORMED
  TEST_VERIFY( param->IsWellFormed(), DescriptionFn(), funcID )

  // Access stored mask
  Handle(TColStd_HPackedMapOfInteger) aMask_FROM = param->GetMask();

  TEST_VERIFY( !aMask_FROM.IsNull(), DescriptionFn(), funcID )

  // Compare the contents of both maps: they should be identical
  TEST_VERIFY( aMask_FROM->Map().IsEqual(aTestMap), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on accessing individual masked IDs of SelectionParameter.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_SelectionParameter::accessSelectionMaskIDs(const int funcID, const bool)
{
  /* ====================================
   *  Initialize underlying CAF document
   * ==================================== */

  TEST_PRINT_DECOR_L("Create new SELECTION Parameter");

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  doc->NewCommand();
  Handle(ActData_SelectionParameter)
    param = ActParamTool::AsSelection( createParameter(doc, Parameter_Selection) );
  doc->CommitCommand();

  TColStd_PackedMapOfInteger aTestMap;

  /* ========================================
   *  Store the test mask -- initially empty
   * ======================================== */

  doc->OpenCommand();
  param->SetMask( new TColStd_HPackedMapOfInteger(aTestMap) );
  doc->CommitCommand();

  TEST_VERIFY( param->Size() == 0, DescriptionFn(), funcID );

  /* ========================
   *  Try per-element access
   * ======================== */

  Standard_Integer MASKED_IDs[] = {10, 40, 15};

  doc->OpenCommand();
  param->Add(MASKED_IDs[0]);
  doc->CommitCommand();

  TEST_VERIFY(  param->Size() == 1 ,            DescriptionFn(), funcID )
  TEST_VERIFY(  param->Contains(MASKED_IDs[0]), DescriptionFn(), funcID )
  TEST_VERIFY( !param->Contains(MASKED_IDs[1]), DescriptionFn(), funcID )
  TEST_VERIFY( !param->Contains(MASKED_IDs[2]), DescriptionFn(), funcID )

  // Try again just the sampe
  doc->OpenCommand();
  param->Add(MASKED_IDs[0]);
  doc->CommitCommand();

  // Verify again
  TEST_VERIFY(  param->Size() == 1,             DescriptionFn(), funcID )
  TEST_VERIFY(  param->Contains(MASKED_IDs[0]), DescriptionFn(), funcID )
  TEST_VERIFY( !param->Contains(MASKED_IDs[1]), DescriptionFn(), funcID )
  TEST_VERIFY( !param->Contains(MASKED_IDs[2]), DescriptionFn(), funcID )

  doc->OpenCommand();
  param->Add(MASKED_IDs[1]);
  doc->CommitCommand();

  TEST_VERIFY(  param->Size() == 2,             DescriptionFn(), funcID )
  TEST_VERIFY(  param->Contains(MASKED_IDs[0]), DescriptionFn(), funcID )
  TEST_VERIFY(  param->Contains(MASKED_IDs[1]), DescriptionFn(), funcID )
  TEST_VERIFY( !param->Contains(MASKED_IDs[2]), DescriptionFn(), funcID )

  doc->OpenCommand();
  param->Add(MASKED_IDs[2]);
  doc->CommitCommand();

  TEST_VERIFY( param->Size() == 3,             DescriptionFn(), funcID )
  TEST_VERIFY( param->Contains(MASKED_IDs[0]), DescriptionFn(), funcID )
  TEST_VERIFY( param->Contains(MASKED_IDs[1]), DescriptionFn(), funcID )
  TEST_VERIFY( param->Contains(MASKED_IDs[2]), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
