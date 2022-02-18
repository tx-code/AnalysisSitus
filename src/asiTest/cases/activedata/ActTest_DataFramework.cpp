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
#include <ActTest_DataFramework.h>

Standard_Integer ActTest_DataFramework::PARAM_TAG = 1;

//-----------------------------------------------------------------------------
// Test functions support
//-----------------------------------------------------------------------------

//! Saves the internal CAF document to file with the passed name.
//! \param doc [in] CAF document to save.
//! \param filename [in] filename.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActTest_DataFramework::saveDocument(const Handle(TDocStd_Document)& doc,
                                      const TCollection_AsciiString& filename)
{
  if ( doc.IsNull() )
    Standard_ProgramError::Raise("Cannot save NULL Document");

  PCDM_StoreStatus
    aStat = ActData_Application::Instance()->SaveAs(doc, filename);

  return aStat == PCDM_SS_OK;
}

//! Loads the CAF document from the file with the passed name.
//! \param filename [in] filename.
//! \param doc [out] document retrieved from file.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActTest_DataFramework::openDocument(const TCollection_AsciiString& filename,
                                      Handle(TDocStd_Document)& doc)
{
  doc.Nullify();
  PCDM_ReaderStatus
    aStat = ActData_Application::Instance()->Open(filename, doc);

  if ( aStat != PCDM_RS_OK )
    return Standard_False;

  doc->SetModificationMode(Standard_True);
  doc->SetUndoLimit(10);

  return Standard_True;
}

//! Creates new Parameter of the passed type.
//! \param doc [in] working CAF document.
//! \param type [in] type of the Parameter to allocate.
//! \return Parameter instance.
Handle(ActAPI_IUserParameter)
  ActTest_DataFramework::createParameter(const Handle(TDocStd_Document)& doc,
                                         const ActAPI_ParameterType type)
{
  Standard_Boolean isUndefinedType;
  TDF_Label aParamLab = nextParameterLabel(doc);
  Handle(ActAPI_IUserParameter)
    aParam = ActData_ParameterFactory::NewParameterExpand(type, aParamLab, isUndefinedType);

  TEST_PRINT_DECOR_L("    |")
  TEST_PRINT_DECOR  ("    +---> ")
  TEST_PRINT        ("Created Parameter with ID: ")
  TEST_PRINT_L      ( aParam->GetId().ToCString() )

  return aParam;
}

//! Creates new Parameter of the passed type returning the corresponding TDF
//! Label as an output argument.
//! \param doc [in] working CAF document.
//! \param type [in] type of the Parameter.
//! \param newLabel [out] just created TDF Label.
//! \return new Parameter.
Handle(ActAPI_IUserParameter)
  ActTest_DataFramework::createParameter(const Handle(TDocStd_Document)& doc,
                                         const ActAPI_ParameterType type,
                                         TDF_Label& newLabel)
{
  Standard_Boolean isUndefinedType;
  newLabel = nextParameterLabel(doc);
  Handle(ActAPI_IUserParameter)
    aParam = ActData_ParameterFactory::NewParameterExpand(type, newLabel, isUndefinedType);

  TEST_PRINT_DECOR_L("    |")
  TEST_PRINT_DECOR  ("    +---> ")
  TEST_PRINT        ("Created Parameter with ID: ")
  TEST_PRINT_L      ( aParam->GetId().ToCString() )

  return aParam;
}

//! Returns next TDF Label for settling down a Parameter.
//! \param doc [in] working CAF document.
//! \return TDF Label.
TDF_Label ActTest_DataFramework::nextParameterLabel(const Handle(TDocStd_Document)& doc)
{
  return doc->Main().Root().FindChild(nextParameterTag(), Standard_True);
}

//! Returns next Tag for Parameter Labels.
//! \return integer Tag.
Standard_Integer ActTest_DataFramework::nextParameterTag()
{
  return PARAM_TAG++;
}

//-----------------------------------------------------------------------------
// Methods for working with graphs and trees
//-----------------------------------------------------------------------------

//! Checks whether the passed Label entry is expected to be found on the passed
//! level of a graph or tree.
//! \param theLevel [in] actual level the Label is found on.
//! \param theEntry [in] entry of the Label.
//! \param theEntriesByLevels [in] reference collection to check the Label against.
//! \return true if the Label is in place, false -- otherwise.
Standard_Boolean
  ActTest_DataFramework::isLabelExpectedByLevel(const Standard_Integer theLevel,
                                                const TCollection_AsciiString& theEntry,
                                                const EntriesByLevels& theEntriesByLevels)
{
  Standard_Boolean isFound = Standard_False;
  const EntryList& theEntriesByLevel = theEntriesByLevels.Value(theLevel);
  EntryList::Iterator anEntryIt(theEntriesByLevel);
  for ( ; anEntryIt.More(); anEntryIt.Next() )
  {
    const TCollection_AsciiString& anEntry = anEntryIt.Value();
    if ( anEntry == theEntry )
    {
      isFound = Standard_True;
      break;
    }
  }
  return isFound;
}
