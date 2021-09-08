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

#ifndef ActData_TransactionEngine_HeaderFile
#define ActData_TransactionEngine_HeaderFile

// Active Data includes
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_TxRes.h>

// OCCT includes
#include <TDocStd_Document.hxx>

#define DEFAULT_UNDO_LIMIT 100

//! \ingroup AD_DF
//!
//! Base class for auxiliary Data Model co-worker tool providing a set of
//! services for deployment and management of CAF transactional scopes. Think
//! of Transaction Engine as a wrapper under the standard OCAF transaction
//! mechanism plus additional services which can be provided by the descendant
//! classes.
//!
//! From the very general point of view, Transaction Engine is normally
//! utilized by the Data Model architectural facade in order to channel all
//! transactional stuff into one dedicated place for customization.
class ActData_TransactionEngine : public Standard_Transient
{
friend class ActData_BaseModel;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TransactionEngine, Standard_Transient)

public:

  ActData_EXPORT
    ActData_TransactionEngine(const Handle(TDocStd_Document)& theDoc,
                              const Standard_Integer          theUndoLimit = DEFAULT_UNDO_LIMIT);

public:

  ActData_EXPORT virtual void
    Release();

// Kernel functionality:
public:

  ActData_EXPORT virtual void
    DisableTransactions();

  ActData_EXPORT virtual void
    EnableTransactions();

  ActData_EXPORT virtual void
    OpenCommand();

  ActData_EXPORT virtual void
    CommitCommand();

  ActData_EXPORT virtual Standard_Boolean
    HasOpenCommand() const;

  ActData_EXPORT virtual void
    AbortCommand();

  ActData_EXPORT virtual Handle(ActAPI_TxRes)
    Undo(const Standard_Integer theNbUndoes = 1);

  ActData_EXPORT virtual Standard_Integer
    NbUndos() const;

  ActData_EXPORT virtual Handle(ActAPI_TxRes)
    Redo(const Standard_Integer theNbRedoes = 1);

  ActData_EXPORT virtual Standard_Integer
    NbRedos() const;

// Construction & initialization is hidden:
protected:

  void
    init(const Handle(TDocStd_Document)& Doc);

private:

  Handle(ActAPI_HDataObjectIdMap)
    entriesToUndo(const Standard_Integer theNbUndoes) const;

  Handle(ActAPI_HDataObjectIdMap)
    entriesToRedo(const Standard_Integer theNbRedoes) const;

  void
    touchAffectedParameters(const Handle(ActAPI_TxRes)& theParams);

  void
    addEntriesByDelta(const Handle(TDF_Delta)&         theDelta,
                      Handle(ActAPI_HDataObjectIdMap)& theMap) const;

  Standard_Boolean
    isTransactionModeOn() const;

  Standard_Boolean
    isTransactionModeOff() const;

  Handle(ActAPI_IDataCursor)
    parameterById(const ActAPI_ParameterId& pid,
                  Standard_Boolean&         isParam,
                  Standard_Boolean&         isUndefinedType) const;

  Handle(ActAPI_TxRes)
    extractTxRes(const Handle(ActAPI_HDataObjectIdMap)& pids) const;

protected:

  //! Undo Limit.
  Standard_Integer m_iUndoLimit;

  //! CAF Document instance.
  Handle(TDocStd_Document) m_doc;

  //! Indicates whether some transaction is currently active.
  Standard_Boolean m_bIsActiveTransaction;

};

#endif
