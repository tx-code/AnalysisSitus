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

#ifndef ActData_ExtTransactionEngine_HeaderFile
#define ActData_ExtTransactionEngine_HeaderFile

// Active Data includes
#include <ActData_TransactionEngine.h>

// Active Data (API) includes
#include <ActAPI_TxData.h>

// OCCT includes
#include <NCollection_Sequence.hxx>
#include <TCollection_ExtendedString.hxx>

DEFINE_STANDARD_HANDLE(ActData_ExtTransactionEngine, ActData_TransactionEngine)

//! \ingroup AD_DF
//!
//! Transaction manager for transactions extended by custom user data.
class ActData_ExtTransactionEngine : public ActData_TransactionEngine
{
public:

  DEFINE_STANDARD_RTTI_INLINE(ActData_ExtTransactionEngine, ActData_TransactionEngine)

// Construction:
public:

  ActData_EXPORT
    ActData_ExtTransactionEngine(const Handle(TDocStd_Document)& Doc,
                                 const Standard_Integer UndoLimit = DEFAULT_UNDO_LIMIT);

// Kernel methods:
public:

  ActData_EXPORT virtual Handle(ActAPI_TxRes)
    Undo(const Standard_Integer theNbUndoes);

  ActData_EXPORT virtual Handle(ActAPI_TxRes)
    Redo(const Standard_Integer theNbRedoes);

  ActData_EXPORT virtual void
    CommitCommandExt(const ActAPI_TxData& theData);

// Auxiliary methods:
public:

  //! Accessor for the sequence of user data associated with Undo Modification
  //! Deltas. This collection is ordered from the most fresh transaction to
  //! the most long-standing one from the left to the right.
  //! \return collection of user data for Undo Modification Deltas.
  const ActAPI_TxDataSeq& GetUndoData() const
  {
    return m_undoData;
  }

  //! Accessor for the sequence of user data associated with Redo Modification
  //! Deltas. This collection is ordered from the most fresh transaction to
  //! the most long-standing one from the left to the right.
  //! \return collection of user data for Redo Modification Deltas.
  const ActAPI_TxDataSeq& GetRedoData() const
  {
    return m_redoData;
  }

public:

  ActData_EXPORT Handle(ActAPI_HTxDataSeq)
    GetUndoData(const Standard_Integer theDepth) const;

  ActData_EXPORT Handle(ActAPI_HTxDataSeq)
    GetRedoData(const Standard_Integer theDepth) const;

private:

  //! Collection of user data extending the managed Modification Deltas
  //! for Undoes.
  ActAPI_TxDataSeq m_undoData;

  //! Collection of user data extending the managed Modification Deltas
  //! for Redoes.
  ActAPI_TxDataSeq m_redoData;

};

#endif
