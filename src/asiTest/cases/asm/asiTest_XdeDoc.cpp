//-----------------------------------------------------------------------------
// Created on: 05 January 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <asiTest_XdeDoc.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

#undef FILE_DEBUG
#if defined FILE_DEBUG
  #pragma message("===== warning: FILE_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

// Filenames are specified relatively to ASI_TEST_DATA environment variable.
#define filename_asm_001 "cad/asm/asm-simplified-connectors.stp"

//-----------------------------------------------------------------------------

bool asiTest_XdeDoc::loadDocument(const char*               shortFilename,
                                  Handle(asiAsm::xde::Doc)& doc)
{
  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Prepare filename.
  std::string
    filename = asiAlgo_Utils::Str::Slashed( asiAlgo_Utils::Env::AsiTestData() )
             + shortFilename;

  // Create a new empty XDE document.
  doc = new asiAsm::xde::Doc;

  // Load data from file.
  if ( !doc->Load( filename.c_str() ) )
  {
    cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load XDE document from file '%1'."
                                               << filename);
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiTest_XdeDoc::areEqual(const asiAsm::xde::PartIds& pids1,
                              const asiAsm::xde::PartIds& pids2)
{
  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  if ( pids1.Length() != pids2.Length() )
  {
    cf->Progress.SendLogMessage(LogErr(Normal) << "Unexpected number of part IDs.");
    return false;
  }

  for ( asiAsm::xde::PartIds::Iterator pit1(pids1); pit1.More(); pit1.Next() )
  {
    const asiAsm::xde::PartId& pid1 = pit1.Value();
    bool isFound = false;

    for ( asiAsm::xde::PartIds::Iterator pit2(pids2); pit2.More(); pit2.Next() )
    {
      const asiAsm::xde::PartId& pid2 = pit2.Value();
      //
      if ( pid1.IsEqual(pid2) )
      {
        isFound = true;
        break;
      }
    }

    if ( !isFound )
    {
      cf->Progress.SendLogMessage(LogErr(Normal) << "Part ID is not found.");
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

outcome asiTest_XdeDoc::testFindItems(const int funcID)
{
  outcome res(DescriptionFn(), funcID);

  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Load XDE document.
  Handle(asiAsm::xde::Doc) doc;
  //
  if ( !loadDocument(filename_asm_001, doc) )
    return res.failure();

  // Find items.
  {
    Handle(asiAsm::xde::HAssemblyItemIdsMap) items;
    //
    if ( !doc->FindItems("X473", items) )
    {
      cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot find assembly items.");
      return res.failure();
    }

    if ( items->Extent() != 1 )
    {
      cf->Progress.SendLogMessage(LogErr(Normal) << "Unexpected number of assembly items.");
      return res.failure();
    }

    if ( items->FindKey(1) != asiAsm::xde::AssemblyItemId("0:1:1:1/0:1:1:1:1/0:1:1:2:5") )
    {
      cf->Progress.SendLogMessage(LogErr(Normal) << "Unexpected assembly item ID.");
      return res.failure();
    }
  }

  return res.success();
}

//-----------------------------------------------------------------------------

outcome asiTest_XdeDoc::testAddPart(const int funcID)
{
  outcome res(DescriptionFn(), funcID);

  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Create a new empty XDE document.
  Handle(asiAsm::xde::Doc) doc = new asiAsm::xde::Doc;

  // Add parts.
  asiAsm::xde::PartIds pidsAdded;
  {
    pidsAdded.Append( doc->AddPart("Part 1") );
    pidsAdded.Append( doc->AddPart("Part 2") );
    pidsAdded.Append( doc->AddPart("Part 3") );
  }

  // Verify.
  asiAsm::xde::PartIds pidsGot;
  doc->GetParts(pidsGot);
  //
  if ( !areEqual(pidsAdded, pidsGot) )
    return res.failure();

  return res.success();
}
