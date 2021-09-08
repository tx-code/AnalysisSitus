//-----------------------------------------------------------------------------
// Created on: July 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_ExtTransactionEngine.h>

//! Creates a new instance of Transaction Engine charged with the CAF Document.
//! \param Doc [in] CAF Document corresponding to the actual Data Model.
//! \param UndoLimit [in] Undo Limit to set.
ActData_ExtTransactionEngine::ActData_ExtTransactionEngine(const Handle(TDocStd_Document)& Doc,
                                                           const Standard_Integer UndoLimit)
: ActData_TransactionEngine(Doc, UndoLimit)
{
}

//! Customized Undo method for extended transactions.
//! \param theNbUndoes [in] number of Undo operations to perform one-by-one.
//! \return affected Parameters.
Handle(ActAPI_TxRes)
  ActData_ExtTransactionEngine::Undo(const Standard_Integer theNbUndoes)
{
  if ( m_undoData.IsEmpty() )
    return NULL; // There is nothing to Undo

  // Perform actual Undo
  Handle(ActAPI_TxRes)
    result = ActData_TransactionEngine::Undo(theNbUndoes);

  // Juggle naming stack
  for ( Standard_Integer NbDone = 0; NbDone < theNbUndoes; NbDone++ )
  {
    m_redoData.Prepend( m_undoData.First() );
    m_undoData.Remove(1);
  }

  return result;
}

//! Customized Redo method for extended transactions.
//! \param theNbRedoes [in] number of Redo operations to perform one-by-one.
//! \return affected Parameters.
Handle(ActAPI_TxRes)
  ActData_ExtTransactionEngine::Redo(const Standard_Integer theNbRedoes)
{
  if ( m_redoData.IsEmpty() )
    return NULL; // There is nothing to Redo

  // Perform actual Redo
  Handle(ActAPI_TxRes)
    result = ActData_TransactionEngine::Redo(theNbRedoes);

  // Juggle naming stack
  for ( Standard_Integer NbDone = 0; NbDone < theNbRedoes; NbDone++ )
  {
    m_undoData.Prepend( m_redoData.First() );
    m_redoData.Remove(1);
  }

  return result;
}

//! Commits current transaction.
//! \param theData [in] user-data to bind to transaction being committed.
void ActData_ExtTransactionEngine::CommitCommandExt(const ActAPI_TxData& theData)
{
  // Perform general commit first, so that Modification Delta is passed to
  // OCCT native stack
  ActData_TransactionEngine::CommitCommand();

  // Juggle TxData stack
  m_undoData.Prepend(theData);
  m_redoData.Clear();

  // Proceed with Undo Limit
  Standard_Integer UndoLimit = m_doc->GetUndoLimit();
  if ( m_undoData.Length() > UndoLimit )
    m_undoData.Remove( m_undoData.Length() );
}

//! Accessor for the sequence of user data associated with Undo Modification
//! Deltas. This method returns only those Data containers which lie within
//! the given depth.
//! \param theDepth [in] depth to limit the collection of data items
//!        being accessed.
//! \return collection of user data for Undo Modification Deltas.
Handle(ActAPI_HTxDataSeq)
  ActData_ExtTransactionEngine::GetUndoData(const Standard_Integer theDepth) const
{
  Handle(ActAPI_HTxDataSeq) aResult = new ActAPI_HTxDataSeq();
  for ( Standard_Integer i = 1; i <= Min(m_undoData.Length(), theDepth); ++i )
    aResult->Append( m_undoData.Value(i) );

  return aResult;
}

//! Accessor for the sequence of user data associated with Redo Modification
//! Deltas. This method returns only those Data containers which lie within
//! the given depth.
//! \param theDepth [in] depth to limit the collection of data items
//!        being accessed.
//! \return collection of user data for Undo Modification Deltas.
Handle(ActAPI_HTxDataSeq)
  ActData_ExtTransactionEngine::GetRedoData(const Standard_Integer theDepth) const
{
  Handle(ActAPI_HTxDataSeq) aResult = new ActAPI_HTxDataSeq();
  for ( Standard_Integer i = 1; i <= theDepth; ++i )
    aResult->Append( m_redoData.Value(i) );

  return aResult;
}
