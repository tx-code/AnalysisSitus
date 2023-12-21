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
#include <ActTest_ExtTransactionEngine.h>

#pragma warning(disable: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(disable: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY

//-----------------------------------------------------------------------------
// Business logic
//-----------------------------------------------------------------------------

//! Performs several named commits and checks that the stored Undo collection
//! is well-formed, while Redo one is empty.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ExtTransactionEngine::namedEngineCommits(const int funcID, const bool)
{
  const Standard_Integer NbCommits = 4;
  TCollection_AsciiString Names[] = {"TR 1", "TR 2", "TR 3", "TR 4"};

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  Handle(ActData_ExtTransactionEngine) engine = new ActData_ExtTransactionEngine(doc);

  // Perform several named commits
  // ...
  for ( Standard_Integer i = 0; i < NbCommits; i++ )
  {
    TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )

    engine->OpenCommand();
    TEST_VERIFY( engine->HasOpenCommand(), DescriptionFn(), funcID )
    engine->CommitCommandExt( ActAPI_TxData() << Names[i] );
  }

  // Get the list of Undo & Redo data
  const ActAPI_TxDataSeq& UndoData = engine->GetUndoData();
  const ActAPI_TxDataSeq& RedoData = engine->GetRedoData();

  // No Redos should exist
  TEST_VERIFY( RedoData.IsEmpty(), DescriptionFn(), funcID)

  // Verify number of Undos
  TEST_VERIFY( UndoData.Length() == NbCommits, DescriptionFn(), funcID )

  // Verify Undos by iterating from the beginning to end
  Standard_Integer CommitIndex = NbCommits;
  for ( ActAPI_TxDataSeq::Iterator it(UndoData); it.More(); it.Next() )
  {
    TCollection_AsciiString aName;
    it.ChangeValue() >> aName;
    TEST_VERIFY( aName == Names[--CommitIndex], DescriptionFn(), funcID );
  }

  return outcome(DescriptionFn(), funcID).success();
}

//! After several commits, performs Undo operations and checks the UR
//! collections' contents.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ExtTransactionEngine::namedEngineUndos(const int funcID, const bool)
{
  const Standard_Integer NbCommits = 5,
                         NbUndos = 2;
  TCollection_AsciiString Names[] = {"TR 1", "TR 2", "TR 3", "TR 4", "TR 5"};

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  Handle(ActData_ExtTransactionEngine) engine = new ActData_ExtTransactionEngine(doc);

  // Perform several named commits
  // ...
  for ( Standard_Integer i = 0; i < NbCommits; i++ )
  {
    TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )

    engine->OpenCommand();
    TEST_VERIFY( engine->HasOpenCommand(), DescriptionFn(), funcID )
    engine->CommitCommandExt( ActAPI_TxData() << Names[i] );
  }

  // Couple of Undos
  // ...
  TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )
  engine->Undo(NbUndos);

  // Get the list of Undo & Redo data
  const ActAPI_TxDataSeq& UndoData = engine->GetUndoData();
  const ActAPI_TxDataSeq& RedoData = engine->GetRedoData();

  // Verify number of Undos
  TEST_VERIFY(UndoData.Length() == NbCommits - NbUndos, DescriptionFn(), funcID )

  // Verify number of Redos
  TEST_VERIFY(RedoData.Length() == NbUndos, DescriptionFn(), funcID )

  // Verify Undos by iterating from the beginning to end
  Standard_Integer UndoIndex = NbCommits - NbUndos;
  for ( ActAPI_TxDataSeq::Iterator it(UndoData); it.More(); it.Next() )
  {
    TCollection_AsciiString aName;
    it.ChangeValue() >> aName;
    TEST_VERIFY(aName == Names[--UndoIndex], DescriptionFn(), funcID )
  }

  // Verify Redos by iterating from the beginning to end
  Standard_Integer RedoIndex = NbCommits - NbUndos;
  for ( ActAPI_TxDataSeq::Iterator it(RedoData); it.More(); it.Next() )
  {
    TCollection_AsciiString aName;
    it.ChangeValue() >> aName;
    TEST_VERIFY(aName == Names[RedoIndex++], DescriptionFn(), funcID )
  }

  return outcome(DescriptionFn(), funcID).success();
}

//! After several commits and Undo operations, performs Redo operations and
//! checks the UR collections' contents.
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ExtTransactionEngine::namedEngineRedos(const int funcID, const bool)
{
  const Standard_Integer NbCommits = 5,
                         NbUndos = 4,
                         NbRedos = 3;
  TCollection_AsciiString Names[] = {"TR 1", "TR 2", "TR 3", "TR 4", "TR 5"};

  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  Handle(ActData_ExtTransactionEngine) engine = new ActData_ExtTransactionEngine(doc);

  // Perform several named commits
  // ...
  // TR 5 | TR 4 | TR 3 | TR 2 | TR 1 <-|||-> <empty>
  for ( Standard_Integer i = 0; i < NbCommits; i++ )
  {
    TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )

    engine->OpenCommand();
    TEST_VERIFY( engine->HasOpenCommand(), DescriptionFn(), funcID )
    engine->CommitCommandExt( ActAPI_TxData() << Names[i] );
  }

  // Couple of Undos
  // ...
  // TR 1 <-|||-> TR 2 | TR 3 | TR 4 | TR 5
  TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )
  engine->Undo(NbUndos);

  // Several Redos
  // ...
  // TR 4 | TR 3 | TR 2 | TR 1 <-|||-> TR 5
  TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )
  engine->Redo(NbRedos);

  // Get the list of Undo & Redo data
  const ActAPI_TxDataSeq& UndoData = engine->GetUndoData();
  const ActAPI_TxDataSeq& RedoData = engine->GetRedoData();

  // Verify number of Undos
  TEST_VERIFY(UndoData.Length() == NbCommits - NbUndos + NbRedos, DescriptionFn(), funcID )

  // Verify number of Redos
  TEST_VERIFY(RedoData.Length() == NbUndos - NbRedos, DescriptionFn(), funcID )

  // Verify Undos by iterating from the beginning to end
  Standard_Integer UndoIndex = NbCommits - NbUndos + NbRedos;
  for ( ActAPI_TxDataSeq::Iterator it(UndoData); it.More(); it.Next() )
  {
    TCollection_AsciiString aName;
    it.ChangeValue() >> aName;
    TEST_VERIFY(aName == Names[--UndoIndex], DescriptionFn(), funcID )
  }

  // Verify Redos by iterating from the beginning to end
  Standard_Integer RedoIndex = NbCommits - NbUndos + NbRedos;
  for ( ActAPI_TxDataSeq::Iterator it(RedoData); it.More(); it.Next() )
  {
    TCollection_AsciiString aName;
    it.ChangeValue() >> aName;
    TEST_VERIFY(aName == Names[RedoIndex++], DescriptionFn(), funcID )
  }

  return outcome(DescriptionFn(), funcID).success();
}

//! Checks if Undo Limit works ;)
//! \param funcID [in] ID of test function.
//! \return true if test is passed, false -- otherwise.
outcome ActTest_ExtTransactionEngine::namedEngineUndoLimit(const int funcID, const bool)
{
  ActTest_DocAlloc docAlloc;
  Handle(TDocStd_Document) doc = docAlloc.Doc;

  Handle(ActData_ExtTransactionEngine) engine = new ActData_ExtTransactionEngine(doc);

  const Standard_Integer NbCommits = DEFAULT_UNDO_LIMIT + 1; // One more than limit
  TCollection_AsciiString* Names = new TCollection_AsciiString[NbCommits];
  for ( Standard_Integer i = 0; i < NbCommits; ++i )
    Names[i] = TCollection_AsciiString("TR ").Cat(i + 1);

  // Perform several named commits
  // ...
  // TR 101 | TR 100 | TR 99 | TR 98 | TR 97 | ... | TR 5 | TR 4 | TR 3 | TR 2 <-|||-> <empty>
  for ( Standard_Integer i = 0; i < NbCommits; i++ )
  {
    TEST_VERIFY( !engine->HasOpenCommand(), DescriptionFn(), funcID )

    engine->OpenCommand();
    TEST_VERIFY( engine->HasOpenCommand(), DescriptionFn(), funcID )
    engine->CommitCommandExt( ActAPI_TxData() << Names[i] );
  }

  // Get the list of Undo & Redo data
  const ActAPI_TxDataSeq& UndoData = engine->GetUndoData();
  const ActAPI_TxDataSeq& RedoData = engine->GetRedoData();

  // Verify number of Undos
  TEST_VERIFY(UndoData.Length() == NbCommits - 1, DescriptionFn(), funcID ) // One disappeared

  // Verify number of Redos
  TEST_VERIFY(RedoData.Length() == 0, DescriptionFn(), funcID ) // No Redo records

  // Verify Undos by iterating from the beginning to end
  Standard_Integer UndoIndex = NbCommits;
  for ( ActAPI_TxDataSeq::Iterator it(UndoData); it.More(); it.Next() )
  {
    TCollection_AsciiString aName;
    it.ChangeValue() >> aName;
    TEST_VERIFY(aName == Names[--UndoIndex], DescriptionFn(), funcID )
  }
  delete[] Names;

  return outcome(DescriptionFn(), funcID).success();
}

#pragma warning(default: 4127) // "Conditional expression is constant" by TEST_VERIFY
#pragma warning(default: 4800) // "Standard_Boolean: forcing value to bool" by TEST_VERIFY
