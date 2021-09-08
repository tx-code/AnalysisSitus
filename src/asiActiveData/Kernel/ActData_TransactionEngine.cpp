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
#include <ActData_TransactionEngine.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_BaseNode.h>
#include <ActData_ParameterFactory.h>

// OCCT includes
#include <TDF_Delta.hxx>
#include <TDF_LabelList.hxx>
#include <TDF_ListIteratorOfAttributeDeltaList.hxx>
#include <TDF_ListIteratorOfDeltaList.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_Tool.hxx>

#define ERR_TRANSACTION_DEPLOYMENT_OFF "Transactions are OFF"
#define ERR_NULL_DOC "Document is NULL"
#define ERR_TR_ALREADY_OPENED "Command is already opened"

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------
// Construction & initialization
//-----------------------------------------------------------------------------

//! Creates a new instance of Transaction Engine charged with the CAF Document.
//! \param Doc [in] CAF Document corresponding to the actual Data Model.
//! \param UndoLimit [in] Undo limit to set.
ActData_TransactionEngine::ActData_TransactionEngine(const Handle(TDocStd_Document)& Doc,
                                                     const Standard_Integer UndoLimit)
: Standard_Transient()
{
  m_iUndoLimit = UndoLimit;
  m_bIsActiveTransaction = Standard_False;
  this->init(Doc);
}

//! Initializes the Transaction Engine with the given CAF Document.
//! \param Doc [in] CAF Document corresponding to the actual Data Model.
void ActData_TransactionEngine::init(const Handle(TDocStd_Document)& Doc)
{
  m_doc = Doc;
  m_doc->SetUndoLimit(m_iUndoLimit);

  this->EnableTransactions();
}

//-----------------------------------------------------------------------------
// Service functionality:
//-----------------------------------------------------------------------------

void ActData_TransactionEngine::Release()
{
  if ( !m_doc.IsNull() )
    m_doc.Nullify();

  m_bIsActiveTransaction = Standard_False;
}

//-----------------------------------------------------------------------------
// Kernel methods
//-----------------------------------------------------------------------------

//! Disables the fundamental requirement to perform any modification in a
//! transactional scope.
void ActData_TransactionEngine::DisableTransactions()
{
  m_doc->SetModificationMode(Standard_False);
}

//! Enables the fundamental requirement to perform any modification in a
//! transactional scope.
void ActData_TransactionEngine::EnableTransactions()
{
  m_doc->SetModificationMode(Standard_True);
}

//! Starts new transactional scope.
void ActData_TransactionEngine::OpenCommand()
{
  if ( this->isTransactionModeOff() )
    return;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  if ( m_bIsActiveTransaction )
    Standard_ProgramError::Raise(ERR_TR_ALREADY_OPENED);

  m_doc->OpenCommand();
  m_bIsActiveTransaction = Standard_True;
}

//! Commits current transaction.
void ActData_TransactionEngine::CommitCommand()
{
  if ( this->isTransactionModeOff() )
    return;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  m_doc->CommitCommand();
  m_bIsActiveTransaction = Standard_False;
}

//! Returns true if any command is opened, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_TransactionEngine::HasOpenCommand() const
{
  if ( this->isTransactionModeOff() )
    return Standard_False;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  return m_bIsActiveTransaction;
}

//! Rolls back current transaction.
void ActData_TransactionEngine::AbortCommand()
{
  if ( this->isTransactionModeOff() )
    return;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  m_doc->AbortCommand();
  m_bIsActiveTransaction = Standard_False;
}

//! Performs Undo operation.
//! \param theNbUndoes [in] number of Undo operations to perform one-by-one.
//! \return affected Parameters (including META).
Handle(ActAPI_TxRes)
  ActData_TransactionEngine::Undo(const Standard_Integer theNbUndoes)
{
  if ( this->isTransactionModeOff() )
    return NULL;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  // Get Parameters which are going to be affected by Undo operation with
  // the given depth
  Handle(ActAPI_HDataObjectIdMap)
    anAffectedObjectIds = this->entriesToUndo(theNbUndoes);

  // Perform Undoes one-by-one
  for ( Standard_Integer NbDone = 0; NbDone < theNbUndoes; NbDone++ )
  {
    m_doc->Undo();
  }

  // Get Parameters after Data Model modification by Undo()
  Handle(ActAPI_TxRes)
    aTxRes = this->extractTxRes(anAffectedObjectIds);

  // Now touch the affected Parameters so that actualizing their MTime
  this->touchAffectedParameters(aTxRes);

  m_bIsActiveTransaction = Standard_False;

  return aTxRes;
}

//! Returns the number of available Undo deltas.
//! \return number of available Undo deltas.
Standard_Integer ActData_TransactionEngine::NbUndos() const
{
  if ( this->isTransactionModeOff() )
    return 0;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  return m_doc->GetAvailableUndos();
}

//! Performs Redo operation.
//! \param theNbRedoes [in] number of Redo operations to perform one-by-one.
//! \return affected Parameters (including META).
Handle(ActAPI_TxRes)
  ActData_TransactionEngine::Redo(const Standard_Integer theNbRedoes)
{
  if ( this->isTransactionModeOff() )
    return NULL;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  // Get Parameters which are going to be affected by Redo operation with
  // the given depth
  Handle(ActAPI_HDataObjectIdMap)
    anAffectedObjectIds = this->entriesToRedo(theNbRedoes);

  // Perform Redoes one-by-one
  for ( Standard_Integer NbDone = 0; NbDone < theNbRedoes; NbDone++ )
  {
    m_doc->Redo();
  }

  // Get Parameters after Data Model modification by Redo()
  Handle(ActAPI_TxRes)
    aTxRes = this->extractTxRes(anAffectedObjectIds);

  // Now touch the affected Parameters so that actualizing their MTime
  this->touchAffectedParameters(aTxRes);

  m_bIsActiveTransaction = Standard_False;

  return aTxRes;
}

//! Returns the number of available Redo deltas.
//! \return number of available Redo deltas.
Standard_Integer ActData_TransactionEngine::NbRedos() const
{
  if ( this->isTransactionModeOff() )
    return 0;

  if ( m_doc.IsNull() )
    Standard_ProgramError::Raise(ERR_NULL_DOC);

  return m_doc->GetAvailableRedos();
}

//-----------------------------------------------------------------------------
// Services for internal & friend usage only
//-----------------------------------------------------------------------------

//! Collects IDs of the data objects which are going to affected by Undo
//! operation with the given depth. This method must be invoked BEFORE
//! actual Undo is launched.
//! \param theNbUndoes [in] Undo depth.
//! \return collection of affected data object IDs.
Handle(ActAPI_HDataObjectIdMap)
  ActData_TransactionEngine::entriesToUndo(const Standard_Integer theNbUndoes) const
{
  Handle(ActAPI_HDataObjectIdMap) aMap = new ActAPI_HDataObjectIdMap();

  const TDF_DeltaList& aDeltaList       = m_doc->GetUndos();
  Standard_Integer     aNbDeltas        = aDeltaList.Extent();
  Standard_Integer     aDeltaIndex      = 0;
  Standard_Integer     aFirstDeltaIndex = aNbDeltas - theNbUndoes + 1;

#if defined COUT_DEBUG
  std::cout << "Undo..." << std::endl;
#endif

  for ( TDF_ListIteratorOfDeltaList it(aDeltaList); it.More(); it.Next() )
  {
    aDeltaIndex++;
    if ( aDeltaIndex < aFirstDeltaIndex )
      continue; // Skip the oldest non-requested Deltas

    this->addEntriesByDelta(it.Value(), aMap);
  }

  return aMap;
}

//! Collects IDs of the data objects which are going to affected by Redo
//! operation with the given depth. This method must be invoked BEFORE
//! actual Redo is launched.
//! \param theNbRedoes [in] Redo depth.
//! \return collection of affected data object IDs.
Handle(ActAPI_HDataObjectIdMap)
  ActData_TransactionEngine::entriesToRedo(const Standard_Integer theNbRedoes) const
{
  Handle(ActAPI_HDataObjectIdMap) aMap = new ActAPI_HDataObjectIdMap();

  const TDF_DeltaList& aDeltaList      = m_doc->GetRedos();
  Standard_Integer     aNbDeltas       = aDeltaList.Extent();
  Standard_Integer     aDeltaIndex     = 0;
  Standard_Integer     aLastDeltaIndex = aNbDeltas - theNbRedoes + 1;

#if defined COUT_DEBUG
  std::cout << "Redo..." << std::endl;
#endif

  for ( TDF_ListIteratorOfDeltaList it(aDeltaList); it.More(); it.Next() )
  {
    aDeltaIndex++;
    if ( aDeltaIndex > aLastDeltaIndex )
      break; // Skip the oldest non-requested Deltas

    this->addEntriesByDelta(it.Value(), aMap);
  }

  return aMap;
}

//! Iterates over the passed collection of Parameters attempting to touch
//! those of them which are still WELL-FORMED, i.e. were not removed or
//! damaged anyhow.
//! \param theParam [in] Parameters to touch.
void ActData_TransactionEngine::touchAffectedParameters(const Handle(ActAPI_TxRes)& theRes)
{
  // Now touch the affected Parameters so that actualizing their MTime
  this->DisableTransactions();
  for ( int k = 1; k <= theRes->parameterRefs.Extent(); ++k )
  {
    const ActAPI_TxRes::t_parameterRef& paramRef = theRes->parameterRefs(k);

    // For undefined types, we update MTime without Data Cursor interfaces
    // as they could not be created by Factory.
    if ( paramRef.isUndefined )
    {
      TDF_Label paramLab;
      TDF_Tool::Label( m_doc->GetData(), paramRef.id, paramLab);

      // Update MTime at low level.
      if ( ActData_BaseModel::MTime_On )
        ActData_Utils::SetTimeStampValue(paramLab, ActData_UserParameter::DS_MTime);
    }
    else
    {
      const Handle(ActAPI_IDataCursor)& aDC = theRes->parameterRefs(k).dc;
      //
      if ( aDC.IsNull() || !aDC->IsKind( STANDARD_TYPE(ActAPI_IUserParameter) ) )
        continue;

      const Handle(ActAPI_IUserParameter)&
        aUserParam = Handle(ActAPI_IUserParameter)::DownCast(aDC);

#if defined COUT_DEBUG
      std::cout << "UNDO: touching Parameter " << aUserParam->DynamicType()->Name() << "... ";
#endif

      if ( aUserParam->IsWellFormed() )
      {
        aUserParam->SetModified();

#if defined COUT_DEBUG
        std::cout << "WELL-FORMED ["
                  << aUserParam->GetNode()->DynamicType()->Name() << "] --> Modified" << std::endl;
#endif
      }
#if defined COUT_DEBUG
      else
        std::cout << "BAD-FORMED <-- Deleted?" << std::endl;
#endif
    }
  }

  this->EnableTransactions();
}

//! Retrieves Nodal Parameters affected by the given Delta and pushes them
//! into the passed collection. META Parameters are also added.
//! \param theDelta [in]  Delta to get Parameters for.
//! \param theMap   [out] resulting cumulative map of Parameters. It is not
//!                       cleaned up before usage.
void ActData_TransactionEngine::addEntriesByDelta(const Handle(TDF_Delta)&         theDelta,
                                                  Handle(ActAPI_HDataObjectIdMap)& theMap) const
{
  const TDF_AttributeDeltaList& attrDeltas = theDelta->AttributeDeltas();
  for ( TDF_ListIteratorOfAttributeDeltaList it(attrDeltas); it.More(); it.Next() )
  {
    Handle(TDF_AttributeDelta)& attrDelta = it.Value();
    if ( attrDelta.IsNull() )
      continue;

    TDF_Label aLab = attrDelta->Label();
    //
    TCollection_AsciiString entry;
    TDF_Tool::Entry(aLab, entry);

#if defined COUT_DEBUG
    std::cout << "\tEntry of affected label: " << entry.ToCString() << std::endl;
#endif

    // Add entry.
    theMap->Add(entry);
  }
}

//! Checks whether the transaction deployment requirement is currently
//! enabled.
//! \return true/false.
Standard_Boolean ActData_TransactionEngine::isTransactionModeOn() const
{
  return m_doc->ModificationMode();
}

//! Checks whether the transaction deployment requirement is currently
//! disbled.
//! \return true/false.
Standard_Boolean ActData_TransactionEngine::isTransactionModeOff() const
{
  return !this->isTransactionModeOn();
}

//! Creates a Data Cursor for a Parameter by its global ID.
//! \param[in]  pid         persistent ID.
//! \param[out] isParam     indicates whether the passed persistent ID is the ID
//!                         of a User or Meta Parameter.
//! \param[out] isUndefined indicates whether the Parameter of the requested
//!                         type is undefined in the Factory. This normally
//!                         happens if the Parameter is of a non-standard
//!                         type (i.e., its type is declared externally to
//!                         Active Data).
//! \return Data Cursor instance.
Handle(ActAPI_IDataCursor)
  ActData_TransactionEngine::parameterById(const ActAPI_ParameterId& pid,
                                           Standard_Boolean&         isParam,
                                           Standard_Boolean&         isUndefinedType) const
{
  isUndefinedType = Standard_False; // Can be corrected for the User Parameters.

  // Check the number of tags.
  std::vector<ActAPI_DataObjectId> tags;
  ActData_Common::SplitTags(pid, tags);
  //
  const Standard_Integer nTags = Standard_Integer( tags.size() );
  //
  if ( nTags < ActData_NumTags_MetaParameterId )
  {
    isParam = Standard_False;
    return NULL; // Not a Parameter simply because its ID has not enough capacity.
  }
  //
  isParam = Standard_True;

  TDF_Label aLab;
  TDF_Tool::Label(m_doc->GetData(), pid, aLab);

  // Try to get User Parameter.
  Handle(ActAPI_IDataCursor) aParamByLabel;
  //
  if ( nTags == ActData_NumTags_UserParameterId )
  {
    if ( aLab.Father().Tag() != ActData_BaseNode::TagUser )
    {
      isParam = Standard_False;
      return NULL; // Not a Parameter because it is not under the USER (`TagUser`) section.
    }

    aParamByLabel = ActData_ParameterFactory::ParamByChildLabelSettle(aLab, isUndefinedType);
  }
  else
    aParamByLabel = ActData_ParameterFactory::MetaParamByLabelSettle(aLab);

  return aParamByLabel;
}

//! Extracts the transaction result for the passed collection of persistent IDs.
//! \param[in] pids collection of persistent IDs.
Handle(ActAPI_TxRes)
  ActData_TransactionEngine::extractTxRes(const Handle(ActAPI_HDataObjectIdMap)& pids) const
{
  Handle(ActAPI_TxRes) result = new ActAPI_TxRes;

  for ( Standard_Integer k = 1; k <= pids->Extent(); ++k )
  {
    Standard_Boolean           isOk         = Standard_True;
    Standard_Boolean           isParam      = Standard_False;
    Standard_Boolean           isParamUndef = Standard_False;
    ActAPI_DataObjectId        id           = ActData_Common::TrimToParameterId( pids->FindKey(k), isOk );
    Handle(ActAPI_IDataCursor) dc           = this->parameterById(id, isParam, isParamUndef);
    //
    if ( !isParam )
      continue; // Skip persistent items which are not Parameters.

    // Check whether the Parameter in question is alive or not.
    Standard_Boolean isAlive;
    if ( isParamUndef )
      isAlive = Standard_True; // We consider unknown Parameters alive.
    else
      isAlive = ( !dc.IsNull() && dc->IsWellFormed() );
    //
    result->Add(id, dc, isAlive, isParamUndef);
  }

  return result;
}
