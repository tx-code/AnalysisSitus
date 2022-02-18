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

#ifndef ActTest_DataFramework_HeaderFile
#define ActTest_DataFramework_HeaderFile

// asiTest includes
#include "asiTest_CaseIDs.h"

// asiTestEngine includes
#include "asiTestEngine_TestCase.h"

// Active Data includes
#include <ActData_Application.h>
#include <ActData_BinDrivers.h>
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <BinDrivers.hxx>
#include <TDocStd_Document.hxx>

//! RAII interface for document. The purpose of this class is to ensure that
//! the document is closed at the end of each test function that allocates it.
class ActTest_DocAlloc
{
public:

  //! Ctor.
  ActTest_DocAlloc()
  {
    Handle(TDocStd_Application) app = ActData_Application::Instance();

    ActData_BinDrivers::DefineFormat(app);

    app->NewDocument(ACTBinFormat, Doc);

    Doc->SetModificationMode(Standard_True);
    Doc->SetUndoLimit(10);
  }

  //! Dtor.
  ~ActTest_DocAlloc()
  {
    if ( !Doc.IsNull() && Doc->IsOpened() )
      ActData_Application::Instance()->Close(Doc);
  }

public:

  Handle(TDocStd_Document) Doc; //!< Document instance.

};

//! \ingroup AD_TEST
//!
//! Convenient base class for Active Data test suites. Provides common
//! functionality and data sources.
class ActTest_DataFramework : public asiTestEngine_TestCase
{
protected:

  static Standard_Boolean
    saveDocument(const Handle(TDocStd_Document)& doc,
                 const TCollection_AsciiString& filename);

  static Standard_Boolean
    openDocument(const TCollection_AsciiString& filename,
                 Handle(TDocStd_Document)& doc);

  static Handle(ActAPI_IUserParameter)
    createParameter(const Handle(TDocStd_Document)& doc,
                    const ActAPI_ParameterType type);

  static Handle(ActAPI_IUserParameter)
    createParameter(const Handle(TDocStd_Document)& doc,
                    const ActAPI_ParameterType type,
                    TDF_Label& newLabel);

  static TDF_Label
    nextParameterLabel(const Handle(TDocStd_Document)& doc);

  static Standard_Integer
    nextParameterTag();

// Working with graphs and trees:
protected:

  //! Short-cut for a collection of Entries.
  typedef NCollection_Sequence<TCollection_AsciiString> EntryList;

  //! Short-cut for collection of TDF Label entries expected on each level of
  //! the dependency graph being executed.
  typedef NCollection_Sequence<EntryList> EntriesByLevels;

  static Standard_Boolean
    isLabelExpectedByLevel(const Standard_Integer theLevel,
                           const TCollection_AsciiString& theEntry,
                           const EntriesByLevels& theEntriesByLevels);

protected:

  //! CAF Tag for root TDF Labels of Data Parameters.
  static Standard_Integer PARAM_TAG;

};

#endif
