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
#include <ActTest_CAFConversionCtx.h>

// asiTestEngine includes
#include <asiTestEngine_FileComparator.h>
#include <asiTestEngine_Launcher.h>

// Active Data unit tests
#include <ActTest_DummyTreeFunction.h>
#include <ActTest_StubANode.h>
#include <ActTest_StubBNode.h>
#include <ActTest_StubBNodeConv.h>
#include <ActTest_StubCNodeConv.h>

// Active Data includes
#include <ActData_CAFDumper.h>

// ACT algorithmic collection includes
#include <ActAux_ShapeFactory.h>

#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

#define INSERT_001_REF  "insert_001_after"
#define INSERT_002A_REF "insert_002a_after"
#define INSERT_002B_REF "insert_002b_after"
#define INSERT_002C_REF "insert_002c_after"
#define INSERT_002D_REF "insert_002d_after"
#define INSERT_002E_REF "insert_002e_after"
#define INSERT_002F_REF "insert_002f_after"
#define INSERT_002G_REF "insert_002g_after"
#define INSERT_002H_REF "insert_002h_after"
#define INSERT_002I_REF "insert_002i_after"
#define INSERT_002J_REF "insert_002j_after"
#define INSERT_003_REF  "insert_003_after"
#define INSERT_004_REF  "insert_004_after"
#define MODIFY_001_REF  "modify_001_after"
#define REMOVE_001_REF  "remove_001_after"
#define REMOVE_002_REF  "remove_002_after"
#define REMOVE_003_REF  "remove_003_after"
#define REMOVE_004_REF  "remove_004_after"
#define REMOVE_005_REF  "remove_005_after"
#define CMPLX_001_REF   "complex_001_after"

#define DUMP_EXT "dump"
#define FILTER_NAMEDSHAPE "[TNaming_NamedShape]"
#define FILTER_MTIME "{MTime}"

//! Auxiliary functions.
namespace Aux
{
  //! Returns true if the given line can be processed, false -- otherwise.
  //! This function is used to skip MTime records, Named Shaped addresses
  //! and other useless stuff which may vary from build to build (of course,
  //! here we mean the builds of OCCT, not the Framework).
  //! The mentioned properties are always different for dumped CAF files.
  //! \param theString [in] string to filter.
  //! \return true if string should be passed for further comparison,
  //!         false -- otherwise.
  Standard_Boolean DiffLineFilter(const TCollection_AsciiString& theString)
  {
    if ( theString.Search(FILTER_NAMEDSHAPE) != -1 )
      return Standard_False;

    if ( theString.Search(FILTER_MTIME) != -1 )
      return Standard_False;

    return Standard_True;
  }

  //! Types of Parameters being tested.
  ActAPI_ParameterType ParamTypes[] =
    { Parameter_Int,
      Parameter_Real,
      Parameter_Bool,
      Parameter_Shape,
      Parameter_RealArray,
      Parameter_AsciiString,
      Parameter_Name,
      Parameter_BoolArray,
      Parameter_StringArray,
      Parameter_ComplexArray,
      Parameter_IntArray,
      Parameter_ReferenceList,
      Parameter_Group,
      Parameter_Mesh,
      Parameter_Reference,
      Parameter_Selection,
      Parameter_TimeStamp };
};

//-----------------------------------------------------------------------------
// Test functions
//-----------------------------------------------------------------------------

//! Performs test on atomic insertion No. 001.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_001(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_001_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );

  // Perform insertion for each Parameter type
  for ( Standard_Integer i = 0; i < sizeof(Aux::ParamTypes) / sizeof(ActAPI_ParameterType); ++i )
  {
    Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[i], GID);

    // Register only one insertion request
    Ctx.Clear();
    Ctx.Insert(DTO);

    // Apply Modification
    TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

    // Get resulting Model
    const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

    // Prepare filenames for results and dump the resulting Model
    TCollection_AsciiString aFilenameAct = filenameActualDump("insert_001", i);
    TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_001_REF, i);

    ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                            ActData_CAFDumper::Content_Plain,
                            ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

    // Perform comparison
    TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  }
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (a).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002a(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002a_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_TimeStamp );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID );

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002a", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002A_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (b).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002b(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002b_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_Shape );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002b", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002B_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (c).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002c(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002c_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_AStr );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002c", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002C_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (d).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002d(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002d_before.dump";

  // ...
  // Initialize Reference Parameter with target out from Node C
  // ...

  N->ConnectReference( ActTest_StubCNode::PID_Ref,
                       M->GetRootNode()->Parameter(ActTest_StubANode::PID_Real) );

  // ...
  // Dump the Model before modifications
  // ...


  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_Ref );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002d", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002D_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (e).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002e(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002e_before.dump";

  // ...
  // Initialize Reference List Parameter with targets out from Node C
  // ...

  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             M->GetRootNode()->Parameter(ActTest_StubANode::PID_Real) );

  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             M->GetRootNode()->Parameter(ActTest_StubANode::PID_DummyShapeA) );

  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             M->GetRootNode()->Parameter(ActTest_StubANode::PID_DummyShapeB) );

  // ...
  // Dump the Model before modifications
  // ...

  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_RefList );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002e", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002E_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (f).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002f(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002f_before.dump";

  // ...
  // Initialize Tree Function Parameter with targets out from Node C
  // ...

  Handle(ActAPI_INode) aRoot = M->GetRootNode();

  N->ConnectTreeFunction( ActTest_StubCNode::PID_TFunc,
                          ActTest_DummyTreeFunction::GUID(),
                          ActAPI_ParameterStream() << aRoot->Parameter(ActTest_StubANode::PID_DummyShapeA)
                                                   << aRoot->Parameter(ActTest_StubANode::PID_DummyShapeB),
                          ActAPI_ParameterStream() << aRoot->Parameter(ActTest_StubANode::PID_Real) );

  // ...
  // Dump the Model before modifications
  // ...

  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_TFunc );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002f", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002F_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (g).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002g(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002g_before.dump";

  // ...
  // Initialize Reference Parameter with target from Node C affected by
  // conversion process
  // ...

  N->ConnectReference( ActTest_StubCNode::PID_Ref,
                       N->Parameter(ActTest_StubCNode::PID_Selection) );

  // ...
  // Dump the Model before modifications
  // ...

  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_Ref );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002g", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002G_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (h).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002h(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002h_before.dump";

  // ...
  // Initialize Reference List Parameter with targets from Node C affected by
  // conversion process
  // ...

  // In-scoped, not modified
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             N->Parameter(ActTest_StubCNode::PID_Shape) );

  // In-scoped, modified
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             N->Parameter(ActTest_StubCNode::PID_Selection) );

  // Out-scoped
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             M->GetRootNode()->Parameter(ActTest_StubANode::PID_DummyShapeB) );

  // ...
  // Dump the Model before modifications
  // ...

  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_AStr );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002h", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002H_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (i).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002i(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002i_before.dump";

  // ...
  // Initialize Tree Function Parameter with arguments and results from
  // both Node C (affected by conversion process) and external Node A
  // ...

  Handle(ActAPI_INode) aRoot = M->GetRootNode();

  N->ConnectTreeFunction( ActTest_StubCNode::PID_TFunc,
                          ActTest_DummyTreeFunction::GUID(),
                          ActAPI_ParameterStream() << aRoot->Parameter(ActTest_StubANode::PID_DummyShapeA)
                                                   << N->Parameter(ActTest_StubCNode::PID_Shape),
                          ActAPI_ParameterStream() << aRoot->Parameter(ActTest_StubANode::PID_DummyShapeB)
                                                   << N->Parameter(ActTest_StubCNode::PID_Mesh) );

  // ...
  // Dump the Model before modifications
  // ...

  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_TFunc );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002i", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002I_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (j).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002j(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002j_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );

  Handle(ActData_ParameterDTO) DTO1 = dtoByType(Aux::ParamTypes[0], GID);
  Handle(ActData_ParameterDTO) DTO2 = dtoByType(Aux::ParamTypes[1], GID);

  // Register two insertion requests
  Ctx.Clear();
  Ctx.Insert(DTO1);
  Ctx.Insert(DTO2);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_002j", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_002J_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 002 (k).
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_002k(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_002k_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );

  Handle(ActData_ParameterDTO) DTO1 = dtoByType(Aux::ParamTypes[0], GID);
  Handle(ActData_ParameterDTO) DTO2 = dtoByType(Aux::ParamTypes[1], GID);

  // Register two insertion requests
  TEST_VERIFY( Ctx.Insert(DTO1), DescriptionFn(), funcID )
  TEST_VERIFY( Ctx.Insert(DTO2), DescriptionFn(), funcID )

  ActAPI_ParameterGID GID100( N->GetId(), 100 );

  // Check invalid Update-Delete pair
  Ctx.Clear();
  TEST_VERIFY( Ctx.Update(GID100, DTO1), DescriptionFn(), funcID )
  TEST_VERIFY( !Ctx.Delete(GID100),      DescriptionFn(), funcID )

  // Check invalid Delete-Delete pair
  Ctx.Clear();
  TEST_VERIFY( Ctx.Delete(GID100),  DescriptionFn(), funcID )
  TEST_VERIFY( !Ctx.Delete(GID100), DescriptionFn(), funcID )

  // Check invalid Update-Update pair
  Ctx.Clear();
  TEST_VERIFY( Ctx.Update(GID100, DTO1),  DescriptionFn(), funcID )
  TEST_VERIFY( !Ctx.Update(GID100, DTO2), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 003.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_003(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_003_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ===================
   *  Perform insertion
   * =================== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID( N->GetId() );
  ActAPI_ParameterGID GIDBefore( N->GetId(), ActTest_StubCNode::PID_Int );

  // Perform insertion for Integer Parameter type
  Handle(ActData_ParameterDTO) DTO = dtoByType(Aux::ParamTypes[0], GID);

  // Register only one insertion request
  Ctx.Clear();
  Ctx.Insert(DTO, GIDBefore);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_003", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_003_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic insertion No. 004.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::insert_004(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "insert_004_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // ...
  // Get Node B
  // ...

  Handle(ActAPI_INode) A = M->GetRootNode();
  Handle(ActAPI_IChildIterator) AIt = A->GetChildIterator();
  AIt->Next();
  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( AIt->Value() );

  TEST_VERIFY( !B.IsNull(), DescriptionFn(), funcID )
  TEST_VERIFY( B->IsWellFormed(), DescriptionFn(), funcID )

  // ...
  // Prepare Conversion Context
  // ...

  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ==============
   *  Insertion #1
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID1( N->GetId() );
  ActAPI_ParameterGID GID1Before( N->GetId(), ActTest_StubCNode::PID_RealArray );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO1 = dtoByType(Aux::ParamTypes[0], GID1);

  // Register insertion request
  Ctx.Insert(DTO1, GID1Before);

  /* ==============
   *  Insertion #2
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID2( N->GetId() );
  ActAPI_ParameterGID GID2Before( N->GetId(), ActTest_StubCNode::PID_Shape );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO2 = dtoByType(Aux::ParamTypes[0], GID2);

  // Register insertion request
  Ctx.Insert(DTO2, GID2Before);

  /* ==============
   *  Insertion #3
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID3( N->GetId() );
  ActAPI_ParameterGID GID3Before( N->GetId(), ActTest_StubCNode::PID_Ref );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO3 = dtoByType(Aux::ParamTypes[0], GID3);

  // Register insertion request
  Ctx.Insert(DTO3, GID3Before);

  /* ==============
   *  Insertion #4
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID4( N->GetId() );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO4 = dtoByType(Aux::ParamTypes[0], GID4);

  // Register insertion request
  Ctx.Insert(DTO4);

  /* ==============
   *  Insertion #5
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID5( B->GetId() );
  ActAPI_ParameterGID GID5Before( B->GetId(), ActTest_StubBNode::PID_Real );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO5 = dtoByType(Aux::ParamTypes[0], GID5);

  // Register insertion request
  Ctx.Insert(DTO5, GID5Before);

  /* ==============
   *  Insertion #6
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID6( B->GetId() );
  ActAPI_ParameterGID GID6Before( B->GetId(), ActTest_StubBNode::PID_RefList );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO6 = dtoByType(Aux::ParamTypes[0], GID6);

  // Register insertion request
  Ctx.Insert(DTO6, GID6Before);

  /* ===============
   *  Apply changes
   * =============== */

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("insert_004", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(INSERT_004_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic modification No. 001.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::modify_001(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "modify_001_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ======================
   *  Perform modification
   * ====================== */

  //! Types of Parameters being tested.
  Standard_Integer PIDs[] =
    { ActTest_StubCNode::PID_Int,          // 0
      ActTest_StubCNode::PID_Real,         // 1
      ActTest_StubCNode::PID_Bool,         // 2
      ActTest_StubCNode::PID_Shape,        // 3
      ActTest_StubCNode::PID_RealArray,    // 4
      ActTest_StubCNode::PID_AStr,         // 5
      ActTest_StubCNode::PID_UStr,         // 6
      ActTest_StubCNode::PID_BoolArray,    // 7
      ActTest_StubCNode::PID_StrArray,     // 8
      ActTest_StubCNode::PID_ComplexArray, // 9
      ActTest_StubCNode::PID_IntArray,     // 10
      ActTest_StubCNode::PID_RefList,      // 11
      ActTest_StubCNode::PID_Group,        // 12
      ActTest_StubCNode::PID_Mesh,         // 13
      ActTest_StubCNode::PID_Ref,          // 14
      ActTest_StubCNode::PID_Selection,    // 15
      ActTest_StubCNode::PID_TimeStamp };  // 16

  // Perform insertion for each Parameter type
  for ( Standard_Integer i = 0; i < sizeof(Aux::ParamTypes) / sizeof(ActAPI_ParameterType); ++i )
  {
    if ( PIDs[i] == -1 )
      continue; // We do not have any Parameter of such type (i) in Node C

    // Prepare target Parameter's DTO
    ActAPI_ParameterGID GID( N->GetId(), PIDs[i] );
    Handle(ActData_ParameterDTO) DTO = dtoByType( Aux::ParamTypes[i], ActAPI_ParameterGID() );

    // Register only one updating request
    Ctx.Clear();
    Ctx.Update(GID, DTO);

    // Apply Modification
    TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

    // Get resulting Model
    const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

    // Prepare filenames for results and dump the resulting Model
    TCollection_AsciiString aFilenameAct = filenameActualDump("modify_001", i);
    TCollection_AsciiString aFilenameRef = filenameRefDump(MODIFY_001_REF, i);

    ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                            ActData_CAFDumper::Content_Plain,
                            ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

    // Perform comparison
    TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  }
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic modification No. 002.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::modify_002(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "modify_002_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ======================
   *  Perform modification
   * ====================== */

  // Prepare target Parameter's DTO
  ActAPI_ParameterGID GID( N->GetId(), ActData_BaseNode::RESERVED_PARAM_RANGE ); // INTEGER
  Handle(ActData_ParameterDTO) DTO = dtoByType( Parameter_Real, ActAPI_ParameterGID() ); // REAL

  // Register only one updating request
  Ctx.Clear();
  Ctx.Update(GID, DTO);

  // Apply Modification: failure due to type inconsistency is expected
  TEST_VERIFY( !Ctx.Apply(M), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic removal No. 001.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::remove_001(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "remove_001_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* =================
   *  Perform removal
   * ================= */

  // Prepare target Parameter's GID
  ActAPI_ParameterGID GID(N->GetId(), ActTest_StubCNode::PID_Selection);

  // Register only one deletion request
  Ctx.Clear();
  Ctx.Delete(GID);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("remove_001", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(REMOVE_001_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic removal No. 002.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::remove_002(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "remove_002_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* =================
   *  Perform removal
   * ================= */

  // Prepare target Parameter's GID
  ActAPI_ParameterGID GID(N->GetId(), ActTest_StubCNode::PID_TimeStamp);

  // Register only one deletion request
  Ctx.Clear();
  Ctx.Delete(GID);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("remove_002", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(REMOVE_002_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic removal No. 003.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::remove_003(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "remove_003_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* =================
   *  Perform removal
   * ================= */

  // Prepare target GIDs
  ActAPI_ParameterGID GID1(N->GetId(), ActTest_StubCNode::PID_UStr);
  ActAPI_ParameterGID GID2(N->GetId(), ActTest_StubCNode::PID_StrArray);

  // Register two deletion requests
  Ctx.Clear();
  Ctx.Delete(GID1);
  Ctx.Delete(GID2);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("remove_003", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(REMOVE_003_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic removal No. 004.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::remove_004(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "remove_004_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* =================
   *  Perform removal
   * ================= */

  // Prepare target GID
  ActAPI_ParameterGID GID(N->GetId(), ActTest_StubCNode::PID_Real);

  // Register only one deletion request
  Ctx.Clear();
  Ctx.Delete(GID);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("remove_004", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(REMOVE_004_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on atomic removal No. 005.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::remove_005(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "remove_005_before.dump";

  // ...
  // Get Node B
  // ...

  Handle(ActAPI_INode) A = M->GetRootNode();
  Handle(ActAPI_IChildIterator) AIt = A->GetChildIterator();
  AIt->Next();
  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( AIt->Value() );

  TEST_VERIFY( !B.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( B->IsWellFormed(), DescriptionFn(), funcID );

  // ...
  // Initialize Reference List Parameter with targets from Node C affected by
  // conversion process
  // ...

  // In-scoped
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             N->Parameter(ActTest_StubCNode::PID_Shape) );

  // In-scoped
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             N->Parameter(ActTest_StubCNode::PID_Mesh) );

  // In-scoped
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             N->Parameter(ActTest_StubCNode::PID_Selection) );

  // Out-scoped
  N->ConnectReferenceToList( ActTest_StubCNode::PID_RefList,
                             A->Parameter(ActTest_StubANode::PID_DummyShapeB) );

  // Reference to Reference List
  B->ConnectReference( ActTest_StubBNode::PID_Ref,
                       N->Parameter(ActTest_StubCNode::PID_RefList) );

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Prepare Conversion Context
  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* =================
   *  Perform removal
   * ================= */

  // Prepare target GID
  ActAPI_ParameterGID GID(N->GetId(), ActTest_StubCNode::PID_RefList);

  // Register only one deletion request
  Ctx.Clear();
  Ctx.Delete(GID);

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("remove_005", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(REMOVE_005_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )
  return outcome(DescriptionFn(), funcID).success();
}

//! Performs test on complex modification No. 001.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_CAFConversionCtx::complex_001(const int funcID, const bool)
{
  /* =======================
   *  Create ABC-01 Project
   * ======================= */

  Handle(ActTest_DummyModel) M;
  Handle(ActTest_StubCNode) N;
  init_ABC01(M, N);

  // Prepare filename
  TCollection_AsciiString aFilenameBefore = dumpPath() + "complex_001_before.dump";

  // Dump the Model before modifications
  M->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aFilenameBefore, M,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // ...
  // Get Node B
  // ...

  Handle(ActAPI_INode) A = M->GetRootNode();
  Handle(ActAPI_IChildIterator) AIt = A->GetChildIterator();
  AIt->Next();
  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( AIt->Value() );

  TEST_VERIFY( !B.IsNull(), DescriptionFn(), funcID );
  TEST_VERIFY( B->IsWellFormed(), DescriptionFn(), funcID );

  // ...
  // Prepare Conversion Context
  // ...

  ActData_CAFConversionCtx Ctx( dumpPath() );

  /* ==============
   *  Insertion #1
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID1( N->GetId() );
  ActAPI_ParameterGID GID1Before( N->GetId(), ActTest_StubCNode::PID_Shape );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO1 = dtoByType(Aux::ParamTypes[0], GID1);

  // Register insertion request
  Ctx.Insert(DTO1, GID1Before);

  /* ==============
   *  Insertion #2
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID2( N->GetId() );
  ActAPI_ParameterGID GID2Before( N->GetId(), ActTest_StubCNode::PID_RefList );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO2 = dtoByType(Aux::ParamTypes[0], GID2);

  // Register insertion request
  Ctx.Insert(DTO2, GID2Before);

  /* ==============
   *  Insertion #3
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID3( N->GetId() );
  ActAPI_ParameterGID GID3Before( N->GetId(), ActTest_StubCNode::PID_Ref );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO3 = dtoByType(Aux::ParamTypes[0], GID3);

  // Register insertion request
  Ctx.Insert(DTO3, GID3Before);

  /* ==============
   *  Insertion #4
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID4( N->GetId() );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO4 = dtoByType(Aux::ParamTypes[0], GID4);

  // Register insertion request
  Ctx.Insert(DTO4);

  /* ==============
   *  Insertion #5
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID5( B->GetId() );
  ActAPI_ParameterGID GID5Before( B->GetId(), ActTest_StubBNode::PID_Real );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO5 = dtoByType(Aux::ParamTypes[0], GID5);

  // Register insertion request
  Ctx.Insert(DTO5, GID5Before);

  /* ==============
   *  Insertion #6
   * ============== */

  // Prepare new Parameter's GID
  ActAPI_ParameterGID GID6( B->GetId() );
  ActAPI_ParameterGID GID6Before( B->GetId(), ActTest_StubBNode::PID_RefList );

  // Prepare DTO
  Handle(ActData_ParameterDTO) DTO6 = dtoByType(Aux::ParamTypes[0], GID6);

  // Register insertion request
  Ctx.Insert(DTO6, GID6Before);

  /* =============
   *  Deletion #7
   * ============= */

  // Prepare target GID
  ActAPI_ParameterGID GID7(N->GetId(), ActTest_StubCNode::PID_Real);

  // Register removal request
  Ctx.Delete(GID7);

  /* =============
   *  Deletion #8
   * ============= */

  // Prepare target GID
  ActAPI_ParameterGID GID8(N->GetId(), ActTest_StubCNode::PID_UStr);

  // Register removal request
  Ctx.Delete(GID8);

  /* =============
   *  Deletion #9
   * ============= */

  // Prepare target GID
  ActAPI_ParameterGID GID9(B->GetId(), ActTest_StubBNode::PID_Ref);

  // Register removal request
  Ctx.Delete(GID9);

  /* ==============
   *  Updating #10
   * ============== */

  // Prepare target Parameter's DTO
  ActAPI_ParameterGID GID10( N->GetId(), ActTest_StubCNode::PID_IntArray );
  Handle(ActData_ParameterDTO) DTO10 = dtoByType( Parameter_IntArray, ActAPI_ParameterGID() );

  // Register updating request
  Ctx.Update(GID10, DTO10);

  /* ===============
   *  Apply changes
   * =============== */

  // Apply Modification
  TEST_VERIFY( Ctx.Apply(M), DescriptionFn(), funcID )

  // Get resulting Model
  const Handle(ActAPI_IModel)& ResModel = Ctx.Result();

  // Prepare filenames for results and dump the resulting Model
  TCollection_AsciiString aFilenameAct = filenameActualDump("complex_001", 0);
  TCollection_AsciiString aFilenameRef = filenameRefDump(CMPLX_001_REF, 0);

  ActData_CAFDumper::Dump(aFilenameAct, ResModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);

  // Perform comparison
  TEST_VERIFY( verifyResults(aFilenameAct, aFilenameRef), DescriptionFn(), funcID )

  /* ===============================================================
   *  Settle down converted Nodes to check their WELL-FORMED states
   * =============================================================== */

  TDF_Label RootModel = ResModel->RootLabel();
  TDF_Label RootPart = RootModel.FindChild(ActData_BaseModel::StructureTag_Partitions,
                                           Standard_False);
  TDF_Label RootPartB = RootPart.FindChild(2, Standard_False);
  TDF_Label RootPartC = RootPart.FindChild(3, Standard_False);
  TDF_Label RootNodeB = RootPartB.FindChild(1, Standard_False);
  TDF_Label RootNodeC = RootPartC.FindChild(1, Standard_False);

  Handle(ActData_BaseNode) CNotConv = Handle(ActData_BaseNode)::DownCast( ActTest_StubCNode::Instance() );
  ActData_NodeFactory::NodeSettle(CNotConv, RootNodeC);

  Handle(ActData_BaseNode) CConv = Handle(ActData_BaseNode)::DownCast( ActTest_StubCNodeConv::Instance() );
  ActData_NodeFactory::NodeSettle(CConv, RootNodeC);

  Handle(ActData_BaseNode) BNotConv = Handle(ActData_BaseNode)::DownCast( ActTest_StubBNode::Instance() );
  ActData_NodeFactory::NodeSettle(BNotConv, RootNodeB);

  Handle(ActData_BaseNode) BConv = Handle(ActData_BaseNode)::DownCast( ActTest_StubBNodeConv::Instance() );
  ActData_NodeFactory::NodeSettle(BConv, RootNodeB);

  TEST_VERIFY( !CNotConv->IsWellFormed(), DescriptionFn(), funcID )
  TEST_VERIFY( CConv->IsWellFormed(), DescriptionFn(), funcID )

  TEST_VERIFY( !BNotConv->IsWellFormed(), DescriptionFn(), funcID )
  TEST_VERIFY( BConv->IsWellFormed(), DescriptionFn(), funcID )

  return outcome(DescriptionFn(), funcID).success();
}

//-----------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------

//! Initializes ABC-01 Project (refer to documentation for details).
//! \param M [out] sample Data Model.
//! \param N [out] sample Node.
void ActTest_CAFConversionCtx::init_ABC01(Handle(ActTest_DummyModel)& M,
                                          Handle(ActTest_StubCNode)&  N)
{
  /* ============================
   *  Create Data Model instance
   * ============================ */

  M = new ActTest_DummyModel();
  M->NewEmpty();

  /* ======================
   *  Disable transactions
   * ====================== */

  M->DisableTransactions();

  /* ============================
   *  Create root Node of type A
   * ============================ */

  Handle(ActTest_StubANode) A = Handle(ActTest_StubANode)::DownCast( ActTest_StubANode::Instance() );
  M->StubAPartition()->AddNode(A);
  A->Init(TopoDS_Shape(), TopoDS_Shape(), 0.0);

  /* ===============================
   *  Create working Node of type C
   * =============================== */

  N = Handle(ActTest_StubCNode)::DownCast( ActTest_StubCNode::Instance() );
  M->StubCPartition()->AddNode(N);
  N->Init();

  /* ================================
   *  Create referrer Node of type B
   * ================================ */

  Handle(ActTest_StubBNode) B = Handle(ActTest_StubBNode)::DownCast( ActTest_StubBNode::Instance() );
  M->StubBPartition()->AddNode(B);
  B->Init(10, 3.14);
  B->SetName("Test Node B");

  /* ==========================
   *  Initialization of Node C
   * ========================== */

  Standard_Integer  Int = 99;
  Standard_Real    Real = 99.99;
  Standard_Boolean Bool = Standard_True;
  TopoDS_Shape    Shape = ActAux_ShapeFactory::Sphere(10);

  // Prepare collections
  Handle(HRealArray)    RealArray    = testRealArray(1.0);
  Handle(HBoolArray)    BoolArray    = testBoolArray();
  Handle(HStringArray)  StrArray     = testStringArray(" [init]");
  Handle(HComplexArray) ComplexArray = testComplexArray(1.0);
  Handle(HIntArray)     IntArray     = testIntArray(1);

  // Prepare strings
  TCollection_AsciiString    AStr("Test ASCII string");
  TCollection_ExtendedString UStr("Test UNICODE string");

  // Integer mask
  TColStd_PackedMapOfInteger Mask;
  Mask.Add(1);
  Mask.Add(-10);
  Mask.Add(10);

  // Initialize Data Node
  ActParamTool::AsInt          ( N->Parameter(ActTest_StubCNode::PID_Int) )         ->SetValue ( Int );
  ActParamTool::AsReal         ( N->Parameter(ActTest_StubCNode::PID_Real) )        ->SetValue ( Real );
  ActParamTool::AsBool         ( N->Parameter(ActTest_StubCNode::PID_Bool) )        ->SetValue ( Bool );
  ActParamTool::AsShape        ( N->Parameter(ActTest_StubCNode::PID_Shape) )       ->SetShape ( Shape );
  ActParamTool::AsRealArray    ( N->Parameter(ActTest_StubCNode::PID_RealArray) )   ->SetArray ( RealArray );
  ActParamTool::AsAsciiString  ( N->Parameter(ActTest_StubCNode::PID_AStr) )        ->SetValue ( AStr );
  ActParamTool::AsName         ( N->Parameter(ActTest_StubCNode::PID_UStr) )        ->SetValue ( UStr );
  ActParamTool::AsBoolArray    ( N->Parameter(ActTest_StubCNode::PID_BoolArray) )   ->SetArray ( BoolArray );
  ActParamTool::AsStringArray  ( N->Parameter(ActTest_StubCNode::PID_StrArray) )    ->SetArray ( StrArray );
  ActParamTool::AsComplexArray ( N->Parameter(ActTest_StubCNode::PID_ComplexArray) )->SetArray ( ComplexArray );
  ActParamTool::AsIntArray     ( N->Parameter(ActTest_StubCNode::PID_IntArray) )    ->SetArray ( IntArray );
  ActParamTool::AsMesh         ( N->Parameter(ActTest_StubCNode::PID_Mesh) )        ->SetMesh  ( new ActData_Mesh );
  ActParamTool::AsSelection    ( N->Parameter(ActTest_StubCNode::PID_Selection) )   ->SetMask  ( new TColStd_HPackedMapOfInteger(Mask) );
  ActParamTool::AsTimeStamp    ( N->Parameter(ActTest_StubCNode::PID_TimeStamp) )   ->SetValue ( new ActAux_TimeStamp );

  // Set C as a child for root Node
  A->AddChildNode(N);

  /* ==========================
   *  Initialization of Node B
   * ========================== */

  // Establish Tree Function
  B->ConnectTreeFunction( ActTest_StubBNode::PID_TFunc,
                          ActTest_DummyTreeFunction::GUID(),
                          ActParamStream() << N->Parameter(ActTest_StubCNode::PID_Int)
                                           << N->Parameter(ActTest_StubCNode::PID_Real),
                          ActParamStream() << N->Parameter(ActTest_StubCNode::PID_Bool) );

  // Connect Reference List
  B->ConnectReferenceToList( ActTest_StubBNode::PID_RefList, N->Parameter(ActTest_StubCNode::PID_AStr) );
  B->ConnectReferenceToList( ActTest_StubBNode::PID_RefList, N->Parameter(ActTest_StubCNode::PID_UStr) );
  B->ConnectReferenceToList( ActTest_StubBNode::PID_RefList, N->Parameter(ActTest_StubCNode::PID_BoolArray) );
  B->ConnectReferenceToList( ActTest_StubBNode::PID_RefList, N->Parameter(ActTest_StubCNode::PID_StrArray) );
  B->ConnectReferenceToList( ActTest_StubBNode::PID_RefList, N->Parameter(ActTest_StubCNode::PID_ComplexArray) );

  // Connect single Reference
  B->ConnectReference( ActTest_StubBNode::PID_Ref, N->Parameter(ActTest_StubCNode::PID_TimeStamp) );

  // Set B as a child for root Node
  A->AddChildNode(B);
}

//! Returns path where temporary files are dumped.
//! \return dumping path.
TCollection_AsciiString ActTest_CAFConversionCtx::dumpPath()
{
  TCollection_AsciiString
    path = ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_files() ).c_str();
  return path;
}

//! Returns path where source files are located.
//! \return source path.
TCollection_AsciiString ActTest_CAFConversionCtx::sourcePath()
{
  TCollection_AsciiString
    path = ActAux::slashed(ActAux::slashed( asiTestEngine_Launcher::current_temp_dir_source() ) + "conversion_ctx").c_str();
  return path;
}

//! Returns filename for actual results number {theIdx}.
//! \param theBaseName [in] base filename.
//! \param theIdx [in] index of results.
//! \return requested filename.
TCollection_AsciiString
  ActTest_CAFConversionCtx::filenameActualDump(const TCollection_AsciiString& theBaseName,
                                               const Standard_Integer theIdx)
{
  return dumpPath().Cat( theBaseName )
                   .Cat("_after").Cat("-").Cat(theIdx).Cat(".").Cat(DUMP_EXT);
}

//! Returns filename for referential results number {theIdx}.
//! \param theBaseName [in] base name for results file (grid name).
//! \param theIdx [in] index of results.
//! \return requested filename.
TCollection_AsciiString
  ActTest_CAFConversionCtx::filenameRefDump(const TCollection_AsciiString& theBaseName,
                                            const Standard_Integer theIdx)
{
  return sourcePath().Cat(theBaseName).Cat("-").Cat(theIdx).Cat(".").Cat(DUMP_EXT);
}

//! Performs verification of results comparing actual CAD dump files with
//! the referential ones.
//! \param theActualFn [in] filename for actual results.
//! \param theRefFn [in] filename for referential results.
//! \return true if verification succeeds, false -- otherwise.
Standard_Boolean
  ActTest_CAFConversionCtx::verifyResults(const TCollection_AsciiString& theActualFn,
                                          const TCollection_AsciiString& theRefFn)
{
  asiTestEngine_FileComparator aFileCmp(theActualFn, theRefFn, Aux::DiffLineFilter);
  aFileCmp.StartFromLine(6); // Skip irrelevant (and unstable) header.
  aFileCmp.Perform();

  if ( !aFileCmp.IsDone() )
    return Standard_False;

  Standard_Boolean areSame = aFileCmp.AreSame();
  if ( !areSame )
  {
    TCollection_AsciiString aMsg("Results are different. Line No. ");
    aMsg += aFileCmp.LineWithDiffs();
    TEST_PRINT_DECOR_L( aMsg.ToCString() );
  }
  else
  {
    TCollection_AsciiString aMsg("Files are the same");
    TEST_PRINT_DECOR_L( aMsg.ToCString() );
  }

  return areSame;
}

//! Prepares DTO for the given Parameter type.
//! \param theType [in] Parameter type to construct DTO for.
//! \param theGID  [in] GID of the Parameter.
//! \return DTO instance.
Handle(ActData_ParameterDTO)
  ActTest_CAFConversionCtx::dtoByType(const ActAPI_ParameterType theType,
                                      const ActAPI_ParameterGID& theGID)
{
  Handle(ActData_ParameterDTO) aResult;
  switch ( theType )
  {
    case Parameter_Int:
      aResult = new ActData_IntDTO(theGID);
      Handle(ActData_IntDTO)::DownCast(aResult)->Value = 2013;
      break;
    case Parameter_Real:
      aResult = new ActData_RealDTO(theGID);
      Handle(ActData_RealDTO)::DownCast(aResult)->Value = 3.1415;
      break;
    case Parameter_Bool:
      aResult = new ActData_BoolDTO(theGID);
      Handle(ActData_BoolDTO)::DownCast(aResult)->Value = Standard_False;
      break;
    case Parameter_Shape:
      aResult = new ActData_ShapeDTO(theGID);
      Handle(ActData_ShapeDTO)::DownCast(aResult)->Shape = ActAux_ShapeFactory::Sphere(1);
      break;
    case Parameter_RealArray:
      aResult = new ActData_RealArrayDTO(theGID);
      Handle(ActData_RealArrayDTO)::DownCast(aResult)->Array = testRealArray(2);
      break;
    case Parameter_AsciiString:
      aResult = new ActData_AsciiStringDTO(theGID);
      Handle(ActData_AsciiStringDTO)::DownCast(aResult)->Value = "Test string";
      break;
    case Parameter_Name:
      aResult = new ActData_NameDTO(theGID);
      Handle(ActData_NameDTO)::DownCast(aResult)->Value = "Test name";
      break;
    case Parameter_BoolArray:
      aResult = new ActData_BoolArrayDTO(theGID);
      Handle(ActData_BoolArrayDTO)::DownCast(aResult)->Array = testBoolArray();
      Handle(ActData_BoolArrayDTO)::DownCast(aResult)->Array->SetValue(0, 0);
      break;
    case Parameter_StringArray:
      aResult = new ActData_StringArrayDTO(theGID);
      Handle(ActData_StringArrayDTO)::DownCast(aResult)->Array = testStringArray(" [dtoByType]");
      break;
    case Parameter_ComplexArray:
      aResult = new ActData_ComplexArrayDTO(theGID);
      Handle(ActData_ComplexArrayDTO)::DownCast(aResult)->Array = testComplexArray(2.0);
      break;
    case Parameter_IntArray:
      aResult = new ActData_IntArrayDTO(theGID);
      Handle(ActData_IntArrayDTO)::DownCast(aResult)->Array = testIntArray(2);
      break;
    case Parameter_Mesh:
      aResult = new ActData_MeshDTO(theGID);
      Handle(ActData_MeshDTO)::DownCast(aResult)->Mesh = new ActData_Mesh;
      break;
    case Parameter_Selection:
      aResult = new ActData_SelectionDTO(theGID);
      Handle(ActData_SelectionDTO)::DownCast(aResult)->Mask = new TColStd_HPackedMapOfInteger;
      break;
    case Parameter_TimeStamp:
      aResult = new ActData_TimeStampDTO(theGID);
      Handle(ActData_TimeStampDTO)::DownCast(aResult)->TimeStamp = new ActAux_TimeStamp;
      break;
    case Parameter_Reference:
      aResult = new ActData_ReferenceDTO(theGID);
      break;
    case Parameter_ReferenceList:
      aResult = new ActData_ReferenceListDTO(theGID);
      break;
    case Parameter_TreeFunction:
      aResult = new ActData_TreeFunctionDTO(theGID);
      break;
    case Parameter_TreeNode:
      aResult = new ActData_TreeNodeDTO(theGID);
      break;
    default:
      aResult = new ActData_ParameterDTO(theGID, theType);
      break;
  }
  return aResult;
}

//! Returns real array for testing.
//! \param theMultCoeff [in] multiplication coefficient for values.
//! \return real array.
Handle(HRealArray)
  ActTest_CAFConversionCtx::testRealArray(const Standard_Real theMultCoeff)
{
  // Real array
  Handle(HRealArray) RealArray = new HRealArray(0, 2);
  RealArray->SetValue(0, -theMultCoeff*10.1);
  RealArray->SetValue(1, -theMultCoeff*20.2);
  RealArray->SetValue(2, -theMultCoeff*30.3);
  return RealArray;
}

//! Returns integer array for testing.
//! \param theMultCoeff [in] multiplication coefficient for values.
//! \return integer array.
Handle(HIntArray)
  ActTest_CAFConversionCtx::testIntArray(const Standard_Integer theMultCoeff)
{
  // Integer array
  Handle(HIntArray) IntArray = new HIntArray(0, 2);
  IntArray->SetValue(0, theMultCoeff*1000);
  IntArray->SetValue(1, theMultCoeff*2000);
  IntArray->SetValue(2, theMultCoeff*3000);
  return IntArray;
}

//! Returns Boolean array for testing.
//! \return array for testing.
Handle(HBoolArray) ActTest_CAFConversionCtx::testBoolArray()
{
  // Boolean array
  Handle(HBoolArray) BoolArray = new HBoolArray(0, 2);
  BoolArray->SetValue(0, Standard_True);
  BoolArray->SetValue(1, Standard_False);
  BoolArray->SetValue(2, Standard_True);
  return BoolArray;
}

//! Returns string array for testing.
//! \param theSuffix [in] string suffix to apply.
//! \return array for testing.
Handle(HStringArray) ActTest_CAFConversionCtx::testStringArray(const TCollection_AsciiString& theSuffix)
{
  // String array
  Handle(HStringArray) StrArray = new HStringArray(0, 2);
  StrArray->SetValue( 0, TCollection_ExtendedString("Test string 0").Cat(theSuffix) );
  StrArray->SetValue( 1, TCollection_ExtendedString("Test string 1").Cat(theSuffix) );
  StrArray->SetValue( 2, TCollection_ExtendedString("Test string 2").Cat(theSuffix) );
  return StrArray;
}

//! Returns complex array for testing.
//! \param theMultCoeff [in] multiplication coefficient for values.
//! \return array for testing.
Handle(HComplexArray)
  ActTest_CAFConversionCtx::testComplexArray(const Standard_Real theMultCoeff)
{
  // Complex array
  Handle(HComplexArray) ComplexArray = new HComplexArray(0, 2);
  ComplexArray->SetValue( 0, ComplexNumber(theMultCoeff*1.1, -theMultCoeff*1.1) );
  ComplexArray->SetValue( 1, ComplexNumber(theMultCoeff*2.2, -theMultCoeff*2.2) );
  ComplexArray->SetValue( 2, ComplexNumber(theMultCoeff*3.3, -theMultCoeff*3.3) );
  return ComplexArray;
}

#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
