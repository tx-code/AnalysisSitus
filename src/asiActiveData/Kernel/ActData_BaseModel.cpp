//-----------------------------------------------------------------------------
// Created on: February 2012
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
#include <ActData_BaseModel.h>

// Active Data (API) includes
#include <ActAPI_Version.h>

// Active Data includes
#include <ActData_Application.h>
#include <ActData_BasePartition.h>
#include <ActData_BaseTreeFunction.h>
#include <ActData_BaseVarNode.h>
#include <ActData_BoolVarNode.h>
#include <ActData_CAFConverter.h>
#include <ActData_CAFConverterFw.h>
#include <ActData_DependencyAnalyzer.h>
#include <ActData_ExtTransactionEngine.h>
#include <ActData_IntVarNode.h>
#include <ActData_RealEvaluatorFunc.h>
#include <ActData_RealVarNode.h>
#include <ActData_SequentialFuncIterator.h>
#include <ActData_TransactionEngine.h>
#include <ActData_TreeFunctionParameter.h>
#include <ActData_Utils.h>

// Detached tasks
#if defined ActiveData_USE_TBB
#include <ActData_FuncExecutionTask.h>
#endif

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDataStd_Integer.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_Tool.hxx>
#include <TFunction_DoubleMapIteratorOfDoubleMapOfIntegerLabel.hxx>
#include <TFunction_DriverTable.hxx>
#include <TFunction_GraphNode.hxx>
#include <TFunction_IFunction.hxx>
#include <TFunction_Iterator.hxx>
#include <TFunction_Scope.hxx>

#define ERR_UNDEFINED_TX_DATA "### Error: undefined"

#undef DUMP_CAF_DATA
#if defined DUMP_CAF_DATA
  #include <ActData_CAFDumper.h>
  #define FILE_DEBUG_DUMPING_PATH "F://"
  #pragma message("===== warning: DUMP_CAF_DATA is enabled")
#endif

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

Standard_Boolean ActData_BaseModel::MTime_On = 1;

//----------------------------------------------------------------------------
// Construction methods
//----------------------------------------------------------------------------

//! Default constructor. Created DETACHED Model instance.
//! \param useExtTransactions [in] indicates whether the Data Model should
//!        utilize Simple (false, default) or Extended Transaction Engine
//!        for deployment of Transactional Scopes.
ActData_BaseModel::ActData_BaseModel(const Standard_Boolean useExtTransactions)
: ActAPI_IModel()
{
  m_status = MS_Undefined;
  m_versionStatus = Version_Undefined;
  m_bSimpleTxMode = !useExtTransactions;
  m_iFuncExecutionFlags = ExecFlags_NoFlags;
}

//----------------------------------------------------------------------------
// Persistence basic methods
//----------------------------------------------------------------------------

//! Creates empty Data Model attached to the new CAF Document.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_BaseModel::NewEmpty()
{
  // Initialize consistent Data Model structure
  Handle(TDocStd_Document) aDoc = this->newDocument();
  this->init(aDoc);
  this->initPartitions();
  this->initFunctionDrivers();

  m_trEngine->DisableTransactions();

  // Bind Version information if not yet
  this->bindVersionInfo();

  m_trEngine->EnableTransactions();

  // Set version status to OK on empty Model
  m_versionStatus = Version_Ok;

  return Standard_True;
}

//! Initializes the Model instance with the CAF Document opened from
//! file with the given filename.
//! \param theFilename [in] name of the file containing CAF Document to open.
//! \param theNotifier [in] notifier.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_BaseModel::Open(const TCollection_AsciiString& theFilename,
                          ActAPI_ProgressEntry           theNotifier)
{
  /* ==========================
   *  Prepare the CAF Document
   * ========================== */

  if ( !m_doc.IsNull() )
    this->Release();

  const Handle(ActData_Application)& anApp = ActData_Application::Instance();
  Handle(TDocStd_Document) aDoc = this->newDocument();

  /* =======================
   *  Open the CAF Document
   * ======================= */

  PCDM_ReaderStatus readerStatus = PCDM_RS_OpenError;

  try
  {
    readerStatus = anApp->Open(theFilename, aDoc);
  }
  catch ( Standard_Failure exc )
  {
    std::cout << "OCCT exception:"         << std::endl;
    std::cout << exc.DynamicType()->Name() << std::endl;
    std::cout << exc.GetMessageString()    << std::endl;
    return Standard_False;
  }

  /* =======================
   *  Initialize Data Model
   * ======================= */

  // Check status
  if ( readerStatus != PCDM_RS_OK )
  {
    TCollection_AsciiString statusStr;

         if ( readerStatus == PCDM_RS_NoDriver )                    statusStr = "PCDM_RS_NoDriver";
    else if ( readerStatus == PCDM_RS_UnknownFileDriver )           statusStr = "PCDM_RS_UnknownFileDriver";
    else if ( readerStatus == PCDM_RS_OpenError )                   statusStr = "PCDM_RS_OpenError";
    else if ( readerStatus == PCDM_RS_NoVersion )                   statusStr = "PCDM_RS_NoVersion";
    else if ( readerStatus == PCDM_RS_NoSchema )                    statusStr = "PCDM_RS_NoSchema";
    else if ( readerStatus == PCDM_RS_NoDocument )                  statusStr = "PCDM_RS_NoDocument";
    else if ( readerStatus == PCDM_RS_ExtensionFailure )            statusStr = "PCDM_RS_ExtensionFailure";
    else if ( readerStatus == PCDM_RS_WrongStreamMode )             statusStr = "PCDM_RS_WrongStreamMode";
    else if ( readerStatus == PCDM_RS_FormatFailure )               statusStr = "PCDM_RS_FormatFailure";
    else if ( readerStatus == PCDM_RS_TypeFailure )                 statusStr = "PCDM_RS_TypeFailure";
    else if ( readerStatus == PCDM_RS_TypeNotFoundInSchema )        statusStr = "PCDM_RS_TypeNotFoundInSchema";
    else if ( readerStatus == PCDM_RS_UnrecognizedFileFormat )      statusStr = "PCDM_RS_UnrecognizedFileFormat";
    else if ( readerStatus == PCDM_RS_MakeFailure )                 statusStr = "PCDM_RS_MakeFailure";
    else if ( readerStatus == PCDM_RS_PermissionDenied )            statusStr = "PCDM_RS_PermissionDenied";
    else if ( readerStatus == PCDM_RS_DriverFailure )               statusStr = "PCDM_RS_DriverFailure";
    else if ( readerStatus == PCDM_RS_AlreadyRetrievedAndModified ) statusStr = "PCDM_RS_AlreadyRetrievedAndModified";
    else if ( readerStatus == PCDM_RS_AlreadyRetrieved )            statusStr = "PCDM_RS_AlreadyRetrieved";
    else if ( readerStatus == PCDM_RS_UnknownDocument )             statusStr = "PCDM_RS_UnknownDocument";
    else if ( readerStatus == PCDM_RS_WrongResource )               statusStr = "PCDM_RS_WrongResource";
    else if ( readerStatus == PCDM_RS_ReaderException )             statusStr = "PCDM_RS_ReaderException";
    else if ( readerStatus == PCDM_RS_NoModel )                     statusStr = "PCDM_RS_NoModel";

    theNotifier.SendLogMessage(LogErr(Normal) << "Status of reader is not OK. Error code: %1." << statusStr);

    return Standard_False;
  }

  this->init(aDoc);
  this->initPartitions();
  this->initFunctionDrivers();

  m_status |= MS_Saved;
  if ( this->IsModified() )
    m_status -= MS_Modified;

  return Standard_True;
}

//! Releases the Data Model.
//! \param theVersionStatus [in] ultimate version status to set.
void ActData_BaseModel::Release(const VersionStatus theVersionStatus)
{
  if ( m_doc.IsNull() )
    return;

  /* ================
   *  Close Document
   * ================ */

  const Handle(ActData_Application)& anApp = ActData_Application::Instance();
  anApp->Close(m_doc);
  m_doc.Nullify();

  /* ===============
   *  Release Model
   * =============== */

  m_copyPasteEngine.Nullify();
  m_status = MS_Undefined;
  m_versionStatus = theVersionStatus;
  m_trEngine->Release();
  m_partitionMap->Clear();
  m_funcCtx->ReleaseUserData();
  m_rootLabel.Nullify();
}

//! Saves the model into the file with the given name.
//! \param theFilename [in] filename.
//! \param theNotifier [in] notifier.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_BaseModel::SaveAs(const TCollection_AsciiString& theFilename,
                            ActAPI_ProgressEntry           theNotifier)
{
  if ( m_doc.IsNull() )
    return Standard_False;

  // Start transaction
  m_doc->NewCommand();

  // Bind Version information if not yet
  this->bindVersionInfo();

  // TODO: Buffering section and LogBook must be ignored!!!

  // Commit transaction
  m_doc->CommitCommand();

  const Handle(ActData_Application)& anApp = ActData_Application::Instance();

  // Write
  PCDM_StoreStatus writerStatus = PCDM_SS_WriteFailure;
  //
  try
  {
    writerStatus = anApp->SaveAs(m_doc, theFilename);
  }
  catch ( Standard_Failure exc )
  {
    std::cout << "OCCT exception:"         << std::endl;
    std::cout << exc.DynamicType()->Name() << std::endl;
    std::cout << exc.GetMessageString()    << std::endl;
    return Standard_False;
  }
  //
  // Check status
  if ( writerStatus != PCDM_SS_OK )
  {
    TCollection_AsciiString statusStr;

         if ( writerStatus == PCDM_SS_DriverFailure )      statusStr = "PCDM_SS_DriverFailure";
    else if ( writerStatus == PCDM_SS_WriteFailure )       statusStr = "PCDM_SS_WriteFailure";
    else if ( writerStatus == PCDM_SS_Failure )            statusStr = "PCDM_SS_Failure";
    else if ( writerStatus == PCDM_SS_Doc_IsNull )         statusStr = "PCDM_SS_Doc_IsNull";
    else if ( writerStatus == PCDM_SS_No_Obj )             statusStr = "PCDM_SS_No_Obj";
    else if ( writerStatus == PCDM_SS_Info_Section_Error ) statusStr = "PCDM_SS_Info_Section_Error";

    theNotifier.SendLogMessage(LogErr(Normal) << "Status of writer is not OK. Error code: %1." << statusStr);

    return Standard_False;
  }

  m_status |= MS_Saved;
  if ( this->IsModified() )
    m_status -= MS_Modified;

  return Standard_True;
}

//! Returns true if the model has been modified, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_BaseModel::IsModified() const
{
  return (m_status & MS_Modified) > 0;
}

//! Returns true if the model has been saved, false - otherwise.
//! \return true/false.
Standard_Boolean ActData_BaseModel::IsSaved() const
{
  return (m_status & MS_Saved) > 0;
}

//! Accessor for the version status.
//! \return version status.
ActAPI_IModel::VersionStatus ActData_BaseModel::GetVersionStatus() const
{
  return m_versionStatus;
}

//! Checks whether the Data Model instance is initialized properly or not.
//! \return true/false.
Standard_Boolean ActData_BaseModel::IsInitialized() const
{
  return (m_status & MS_Initialized) > 0;
}

//! Checks whether the Data Model instance is in UNDEFINED status.
//! \return true/false.
Standard_Boolean ActData_BaseModel::IsUndefined() const
{
  return (m_status & MS_Undefined) > 0;
}

//----------------------------------------------------------------------------
// Transaction mechanism
//----------------------------------------------------------------------------

//! Disables transactional mode.
void ActData_BaseModel::DisableTransactions()
{
  m_trEngine->DisableTransactions();
}

//! Enables transactional mode (it is enabled by default).
void ActData_BaseModel::EnableTransactions()
{
  m_trEngine->EnableTransactions();
}

//! Starts new transactional scope.
void ActData_BaseModel::OpenCommand()
{
  if ( this->HasOpenCommand() )
    Standard_ProgramError::Raise("Nested transactions are prohibited");

  m_trEngine->OpenCommand();
}

//! Returns true if any command is opened, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_BaseModel::HasOpenCommand() const
{
  return m_trEngine->HasOpenCommand();
}

//! Rolls back current transaction.
void ActData_BaseModel::AbortCommand()
{
  m_trEngine->AbortCommand();
}

//! Commits current transaction.
//! \param theData [in] optional application-specific data to bind to
//!        transaction.
void ActData_BaseModel::CommitCommand(ActAPI_TxData theData)
{
  if ( m_trEngine->IsInstance( STANDARD_TYPE(ActData_ExtTransactionEngine) ) )
  {
    if ( theData.IsEmpty() )
      theData << ERR_UNDEFINED_TX_DATA;

    Handle(ActData_ExtTransactionEngine)::DownCast(m_trEngine)->CommitCommandExt(theData);
  }
  else
    m_trEngine->CommitCommand();

  // Set status to MODIFIED
  m_status |= MS_Modified;
}

//! Performs Undo operation.
//! \param theNbUndoes [in] number of Undo operations to perform one-by-one.
//! \return affected Parameters.
Handle(ActAPI_TxRes)
  ActData_BaseModel::Undo(const Standard_Integer theNbUndoes)
{
#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_UNDO_before.log"), this);
#endif

  Handle(ActAPI_TxRes) result = m_trEngine->Undo(theNbUndoes);

#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_UNDO_after.log"), this);
#endif

  return result;
}

//! Returns the number of available Undo deltas.
//! \return number of available Undo deltas.
Standard_Integer ActData_BaseModel::NbUndos() const
{
  return m_trEngine->NbUndos();
}

//! Performs Redo operation.
//! \param theNbRedoes [in] number of Redo operations to perform one-by-one.
//! \return affected Parameters.
Handle(ActAPI_TxRes)
  ActData_BaseModel::Redo(const Standard_Integer theNbRedoes)
{
#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_REDO_before.log"), this);
#endif

  Handle(ActAPI_TxRes) result = m_trEngine->Redo(theNbRedoes);

#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_REDO_after.log"), this);
#endif

  return result;
}

//! Returns the number of available Redo deltas.
//! \return number of available Redo deltas.
Standard_Integer ActData_BaseModel::NbRedos() const
{
  return m_trEngine->NbRedos();
}

//! \return map of Data Node IDs modified in the current transaction.
Handle(ActAPI_HNodeIdMap) ActData_BaseModel::GetModifiedNodes() const
{
  Handle(ActAPI_HNodeIdMap) res = new ActAPI_HNodeIdMap;

  // Iterate over the last delta.
  Handle(ActAPI_HDataObjectIdMap) ids = m_trEngine->entriesToUndo(1);
  //
  for ( ActAPI_HDataObjectIdMap::Iterator it(*ids); it.More(); it.Next() )
  {
    /*
     * A Data Node is, by design, referenced by an ID having a format like
     * A:B:C:D, i.e., the tuple of four integer numbers. Since the transaction
     * engine return the entries of labels which are nested hierarchically
     * to the Node's root (these labels have IDs like A:B:C:D:E:F...), to
     * get the modified Nodes it is necessary to trim these IDs by the initial
     * four positions. Doing so, we will likely obtain duplications, and that
     * is why the returned collection is a map (to preserve unique entities).
     */

#ifdef COUT_DEBUG
    std::cout << "\t>>> modified label: " << it.Value() << std::endl;
#endif

    // Access the modified Parameter.
    bool                       isOk    = true;
    const ActAPI_DataObjectId& id      = it.Value();
    ActAPI_ParameterId         paramId = ActData_Common::TrimToParameterId(id, isOk);
    //
    if ( !isOk )
      continue;

    // Compose a Node ID.
    ActAPI_NodeId nodeId  = ActData_Common::NodeIdByParameterId(paramId);

    // Add to result.
    res->Add(nodeId);
  }

  return res;
}

//----------------------------------------------------------------------------
// Services for working with Data Model structure
//----------------------------------------------------------------------------

//! Accessor for the Variable Partition instance.
//! \param theVarType [in] Variable type to return the dedicated Partition for.
//! \return requested Partition.
Handle(ActAPI_IPartition)
  ActData_BaseModel::VariablePartition(const VariableType& theVarType) const
{
  return this->getVariablePartition(theVarType);
}

//! Returns the list of registered Partitions in their storage order.
//! \return list of registered Partitions.
Handle(ActAPI_HPartitionList) ActData_BaseModel::Partitions() const
{
  Handle(ActAPI_HPartitionList) aResultList = new ActAPI_HPartitionList();

  // Iterate over all registered Partitions
  for ( PartitionMap::Iterator it( *m_partitionMap.operator->() ); it.More(); it.Next() )
    aResultList->Append( it.Value() );

  return aResultList;
}

//! Accessor for the Partition instance by the passed ID. If such Partition
//! not found, throws an exception.
//! \param theTypeId [in] type of the Partition to access.
//! \return requested Partition.
Handle(ActAPI_IPartition)
  ActData_BaseModel::Partition(const Standard_Integer theTypeId) const
{
  if ( !m_partitionMap->IsBound(theTypeId) )
    Standard_ProgramError::Raise("No partition registered for this type");

  return m_partitionMap->Find(theTypeId);
}

//! Accessor for the Partition instance by the passed Node type. If such
//! Partition not found, throws an exception.
//! \param theNodeType [in] type of the Node to get Partition for.
//! \return requested Partition.
Handle(ActAPI_IPartition)
  ActData_BaseModel::Partition(const TCollection_AsciiString& theNodeType) const
{
  PartitionMap::Iterator anIt( *m_partitionMap.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    const Handle(ActAPI_IPartition)& aNextPartition = anIt.Value();
    TCollection_AsciiString aNodeType = aNextPartition->GetNodeType()->Name();

    if ( aNodeType.IsEqual(theNodeType) )
      return aNextPartition;
  }

  Standard_ProgramError::Raise("No partition registered for this Node type");
  return NULL;
}

//! Returns a Data Node by the passed Node ID. Notice that normally Node
//! instances are not preserved anyhow in the Model instance as they are
//! created by demand. This routine actually creates a new Node instance
//! playing as an interface to the underlying CAF data. If the passed ID
//! does not correspond to any CAF Label representing ACT Nodal structure,
//! NULL reference is returned.
//! \param theNodeId [in] Node ID to find a Node.
//! \return the requested Node or NULL reference if nothing was found.
Handle(ActAPI_INode) ActData_BaseModel::FindNode(const ActAPI_DataObjectId& theNodeId) const
{
  /* =========================
   *  Find TDF Label by entry
   * ========================= */

  TDF_Label aResLabel;
  TDF_Tool::Label(m_doc->GetData(), theNodeId, aResLabel, Standard_False);

  if ( aResLabel.IsNull() )
    return NULL;

  return ActData_NodeFactory::NodeSettle(aResLabel);
}

//! Finds Data Node by its name. This function simply iterates the registered
//! Partitions, so it can be quite slow for large models. It should be noted
//! that Node names are not unique, so this function will return the first
//! found Node. Another peculiarity is that this function iterates the
//! persistent structure of OCAF, so the result does not depend on parent-child
//! or any other relations between your Nodes.
//!
//! \param theNodeName [in] name of the Node to find.
//! \return found Node or NULL if nothing was found.
Handle(ActAPI_INode)
  ActData_BaseModel::FindNodeByName(const TCollection_ExtendedString& theNodeName) const
{
  Handle(ActAPI_HNodeList) aFoundNodes = this->FindNodesByName(theNodeName);

  if ( aFoundNodes->IsEmpty() )
    return NULL;

  return aFoundNodes->First();
}

//! Finds all Data Nodes which have the specified name.
//!
//! \param theNodeName [in] name of the Nodes to find.
//! \return found Nodes or empty list if nothing was found.
Handle(ActAPI_HNodeList)
  ActData_BaseModel::FindNodesByName(const TCollection_ExtendedString& theNodeName) const
{
  Handle(ActAPI_HNodeList) aResult = new ActAPI_HNodeList;

  // Iterate over all registered Partitions
  PartitionMap::Iterator aPartIt( *m_partitionMap.operator->() );
  for ( ; aPartIt.More(); aPartIt.Next() )
  {
    const Handle(ActAPI_IPartition)& aPart = aPartIt.Value();
    Handle(ActData_BasePartition) aBasePart = Handle(ActData_BasePartition)::DownCast(aPart);
    
    // Iterate over all Nodes contained in the Partition
    ActData_BasePartition::Iterator aNodeIt(aBasePart);
    for ( ; aNodeIt.More(); aNodeIt.Next() )
    {
      Handle(ActAPI_INode) aNode = aNodeIt.Value();

      if ( aNode.IsNull() || !aNode->IsWellFormed() )
        continue;

      if ( aNode->GetName() == theNodeName )
        aResult->Append(aNode);
    }
  }

  return aResult;
}

//! Finds Data Node by its name and the names of its direct parents.
//!
//! \param theNodeNames [in] names of the parent Nodes and the target
//!                          Node (the last item in the sequence).
//! \return found Node or NULL if nothing was found.
Handle(ActAPI_INode)
  ActData_BaseModel::FindNodeByNames(const std::vector<TCollection_ExtendedString>& theNodeNames) const
{
  if ( theNodeNames.size() < 1 )
    return NULL;

  // The last item in the passed sequence is the Node name
  const TCollection_ExtendedString& aNodeName = theNodeNames[theNodeNames.size() - 1];

  // In case if there is a single item, just find any Node
  if ( theNodeNames.size() == 1 )
    return this->FindNodeByName(aNodeName);

  // Find all Nodes with the given Name
  Handle(ActAPI_HNodeList) aHeadNodes = this->FindNodesByName(aNodeName);
  //
  if ( aHeadNodes->Length() < 1 )
    return NULL;

  // Loop over the parent Nodes checking their names.
  //
  // name 1 / name 2 / name 3
  // -------------------------
  //   N1  ->  N2   ->  N3
  //   N4  ->  N5   ->  N6
  //   N7  ->  N8   ->  N9
  //   N10 ->  N11  ->  N12
  // ...

  // Loop over the head Nodes. For each head Node, we will traverse its
  // parents to match their names against the input list of names.
  for ( ActAPI_HNodeList::Iterator nit(*aHeadNodes); nit.More(); nit.Next() )
  {
    Handle(ActAPI_INode) aHead = nit.Value();

    // Loop over the parents
    Standard_Boolean     areNamesMatching = Standard_True;
    Handle(ActAPI_INode) aCurrentNode     = aHead;
    //
    for ( int k = (int) theNodeNames.size() - 2; k >= 0; --k )
    {
      Handle(ActAPI_INode) aParent = aCurrentNode->GetParentNode();
      //
      if ( aParent.IsNull() || !aParent->IsWellFormed() )
      {
        areNamesMatching = Standard_False;
        break;
      }

      if ( aParent->GetName() != theNodeNames[k] )
      {
        areNamesMatching = Standard_False;
        break;
      }

      aCurrentNode = aParent;
    }

    if ( areNamesMatching )
      return aHead;
  }

  return NULL;
}

//! Returns the root Data Node defined by custom implementation.
//! \return root Data Node.
Handle(ActAPI_INode) ActData_BaseModel::GetRootNode() const
{
  return this->getRootNode();
}

//! Deletes the Data Node with the passed ID from the Data Model. The
//! deletion rules are the followings:
//! - Data Node is deleted with underlying child Nodes recursively;
//! - If Data Node A (those being deleted) is a source for Data Node B, the
//!   corresponding Tree Function Parameter is removed from B;
//! Notice that this method does not perform any validation, so make sure that
//! you checked deletion business rules beforehand.
//! \param theNodeId [in] ID of the Data Node to delete.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_BaseModel::DeleteNode(const ActAPI_DataObjectId& theNodeId)
{
  // Run recursive deletion routine
  this->doDeleteRecursive(theNodeId);

  return Standard_True;
}

//! Another form of deletion method.
//! \param theNode [in] Data Node to delete.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_BaseModel::DeleteNode(const Handle(ActAPI_INode)& theNode)
{
  return this->DeleteNode( theNode->GetId() );
}

//! Copies the Data Node with the given ID to the internal buffer.
//! \param theNodeId [in] ID of the Node to copy.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_BaseModel::CopyNode(const ActAPI_DataObjectId& theNodeId)
{
  return this->CopyNode( this->FindNode(theNodeId) );
}

//! Another form of copying method.
//! \param theNode [in] Node to copy.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_BaseModel::CopyNode(const Handle(ActAPI_INode)& theNode)
{
#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_COPY_before.log"), this);
#endif

  // Get reference types to pass out-scope filtering
  ActAPI_FuncGUIDStream FuncGUIDsToPass;
  ActAPI_ParameterLocatorStream RefsToPass;
  this->getInvariantCopyRefs(FuncGUIDsToPass, RefsToPass);

  // Charge out-scope Reference Filter
  m_copyPasteEngine->AccessReferenceFilter().Load(FuncGUIDsToPass, RefsToPass);

  // Transfer data to the buffering section
  Standard_Boolean aRes = m_copyPasteEngine->TransferToBuffer(theNode);

#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_COPY_after.log"), this);
#endif

  return aRes;
}

//! Pastes the buffered Node as a child of the given one.
//! \param theParent [in] parent for the pasted copy.
//! \return true in case of success, false -- otherwise.
Handle(ActAPI_INode) ActData_BaseModel::PasteAsChild(const Handle(ActAPI_INode)& theParent)
{
#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_PASTE_before.log"), this);
#endif

  // Access the buffered Node
  Handle(ActAPI_INode) aBufRoot = m_copyPasteEngine->GetRootBuffered();
  if ( aBufRoot.IsNull() || !aBufRoot->IsWellFormed() )
    return Handle(ActAPI_INode)();

  // Pasting is prohibited to children of the source Node
  if ( m_copyPasteEngine->GetRelocationTable(Standard_True).IsBound1( theParent->RootLabel() ) )
    return Handle(ActAPI_INode)();

  // Restore the Data Node from the buffering section distributing itself
  // with all the children by proper Partitions
  Handle(ActAPI_INode) aResNode = m_copyPasteEngine->RestoreFromBuffer();
  theParent->AddChildNode(aResNode);

#if defined DUMP_CAF_DATA
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat("CAFDumper_PASTE_after.log"), this);
#endif

  return aResNode;
}

//! Renames the Data Node with the given ID populating it with the passed
//! new name.
//! \param theNodeId [in] ID of the Data Node to rename.
//! \param theNewName [in] new name to set.
void ActData_BaseModel::RenameNode(const ActAPI_DataObjectId& theNodeId,
                                   const TCollection_AsciiString& theNewName)
{
  this->FindNode(theNodeId)->SetName(theNewName);
}

//! Renames Variable Data Node with the given ID populating it with the
//! passed new name.
//! \param theNodeId [in] ID of the Variable Node to rename.
//! \param theNewName [in] new name to set.
//! \param doFullSynchronization [in] indicates whether to perform full
//!        traversal over the entire model in order to find all references to
//!        the given Variable in the currently existing evaluation strings
//!        and replace them with the new name of the Variable. The latter
//!        scheme also brings a necessity to provide auto-connection of the
//!        existing Evaluator Tree Functions with the target Variable Node.
void ActData_BaseModel::RenameVariable(const ActAPI_DataObjectId& theNodeId,
                                       const TCollection_AsciiString& theNewName,
                                       const Standard_Boolean doFullSynchronization)
{
  /* ================================================
   *  Fiurst check that Variable is of expected type
   * ================================================ */

  Handle(ActAPI_INode) aNode = this->FindNode(theNodeId);
  if ( !aNode->IsKind( STANDARD_TYPE(ActData_BaseVarNode) ) )
    return; // Improper type of Data Node: only embedded Variables are accepted

  Handle(ActData_BaseVarNode) aVarNode = Handle(ActData_BaseVarNode)::DownCast(aNode);

  TCollection_AsciiString anOldName = aVarNode->GetVariableName();

  /* ===================================================================
   *  Then ask Variable to rename itself in a way it can do so. It will
   *  lead to updating of all connected Parameters
   * =================================================================== */

  aVarNode->RenameConnected(theNewName);

  if ( !doFullSynchronization )
    return;

  /* =======================================================
   *  Affect occurrences of old variable name in Copy/Paste
   *  buffering section
   * ======================================================= */

  TDF_Label aCPRoot = this->accessCopyPasteSection(Standard_False);
  if ( !aCPRoot.IsNull() )
  {
    TDF_Label aBuffRoot = aCPRoot.FindChild(1, Standard_False);
    if ( !aBuffRoot.IsNull() )
    {
      for ( TDF_ChildIterator it(aBuffRoot, Standard_False); it.More(); it.Next() )
      {
        TDF_Label aNodeRoot = it.Value();
        Handle(ActAPI_INode) aNode1 = ActData_NodeFactory::NodeSettle(aNodeRoot);

        if ( aNode1.IsNull() )
          continue;

        Handle(ActData_BaseNode) aNodeBase = Handle(ActData_BaseNode)::DownCast(aNode1);
        Handle(ActAPI_HNodalParameterList) aNParams = aNodeBase->accessAllParameters();

        for ( ActAPI_NodalParameterList::Iterator pit( *aNParams.operator->() ); pit.More(); pit.Next() )
        {
          Handle(ActAPI_IUserParameter) aParam = pit.Value().Parameter;
          ActData_Utils::ReplaceEvaluationString(aParam, anOldName, theNewName, Standard_False);
        }
      }
    }
  }

  /* ==============================================================
   *  Now take care of all remaining "dead" references which might
   *  exist in the Data Model
   * ============================================================== */

  this->chargeEvaluatorsWithVar(aVarNode);
}

//! Adds the passed Variable Node into the Variable Partition of a given type.
//! Comparing to the low-level approach of population of a Partition via
//! AddNode method, this one also performs additional work attempting to
//! connect all possibly existing evaluators with the given Variable by
//! its name. Practically it means additional iteration over the entire
//! Model along with parsing of evaluation strings associated with each
//! existing Expressible Parameter. If you're confident about disutility
//! of such operation on your custom Data Model, prefer adding Data Nodes
//! via Partitions directly in order to achieve better performance.
//! \param theVarType [in] type of the Variable being added.
//! \param theVarName [in] name of the Variable being added.
//! \return ID of the just added Data Node.
ActAPI_DataObjectId
  ActData_BaseModel::AddVariable(const ActAPI_IModel::VariableType theVarType,
                                 const TCollection_AsciiString& theVarName)
{
  /* =================================================
   *  Resolve variable type and prepare a Data Cursor
   * ================================================= */

  Handle(ActAPI_INode) aNode;
  switch ( theVarType )
  {
  case ActAPI_IModel::Variable_Bool:
    aNode = ActData_BoolVarNode::Instance();
    break;
  case ActAPI_IModel::Variable_Int:
    aNode = ActData_IntVarNode::Instance();
    break;
  case ActAPI_IModel::Variable_Real:
    aNode = ActData_RealVarNode::Instance();
    break;
  default:
    Standard_ProgramError::Raise("Unexpected type of Variable Node");
  }

  /* =======================================================
   *  Populate appropriate Partition with new Variable Node
   * ======================================================= */

  Handle(ActData_BaseVarNode) aVarNode = Handle(ActData_BaseVarNode)::DownCast(aNode);
  ActAPI_DataObjectId
    aResult = this->VariablePartition(theVarType)->AddNode(aVarNode);

  /* =======================================================================
   *  Set Variable name. Notice that Variable Node here is still BAD-FORMED
   *  as we do not pass any value there
   * ======================================================================= */

  aVarNode->SetVariableName(theVarName);

  /* ==========================================
   *  Update evaluation connections and return
   * ========================================== */

  this->chargeEvaluatorsWithVar(aVarNode);
  return aResult;
}

//----------------------------------------------------------------------------
// Accessors to raw CAF data & model sections
//----------------------------------------------------------------------------

//! Accessor for the underlying CAF TDF Label.
//! \return CAF Label.
TDF_Label ActData_BaseModel::RootLabel() const
{
  return m_rootLabel;
}

//! Returns LogBook Data Cursor.
//! \return LogBook Data Cursor.
ActData_LogBook ActData_BaseModel::LogBook() const
{
  TDF_Label aLogBookSection =
    this->RootLabel().FindChild(ActData_BaseModel::StructureTag_LogBook);

  return ActData_LogBook(aLogBookSection);
}

//----------------------------------------------------------------------------
// Accessors for Model engines
//----------------------------------------------------------------------------

//! Accessor for the Transaction Engine instance.
//! \return Transaction Engine instance.
const Handle(ActData_TransactionEngine)& ActData_BaseModel::TransactionEngine() const
{
  return m_trEngine;
}

//! Accessor for the Copy/Paste Engine instance.
//! \return Copy/Paste Engine instance.
const Handle(ActData_CopyPasteEngine)& ActData_BaseModel::CopyPasteEngine() const
{
  return m_copyPasteEngine;
}

//! Accessor for the Execution Context instance.
//! \return Execution Context instance.
const Handle(ActData_FuncExecutionCtx)& ActData_BaseModel::FuncExecutionCtx() const
{
  return m_funcCtx;
}

//----------------------------------------------------------------------------
// Tree Function mechanism
//----------------------------------------------------------------------------

//! Executes the entire dependency graph of Tree Functions.
//! \param doDetach [in] indicates whether execution routine must be
//!        performed in the detached (working) thread.
//! \param theData [in] optional TxData structure which allows to pass
//!        transactional information to the working thread. Use this in
//!        order to customize your threading pattern. E.g. you can specify
//!        a callback to invoke once data is committed.
//! \return execution status.
Standard_Integer ActData_BaseModel::FuncExecuteAll(const Standard_Boolean doDetach,
                                                   ActAPI_TxData theData)
{
  // Proceed with request on detached execution
  if ( doDetach )
  {
#if defined ActiveData_USE_TBB
    ActData_FuncExecutionTask::Launch(this->FuncProgressNotifier(), this, theData);
    return MS_Undefined;
#else
    Standard_ProgramError::Raise("Cannot detach without TBB 3-rd party enabled.");
#endif
  }

  Standard_Integer aResult = Execution_Undefined;

  /* =========================================================
   *  Give Tree Functions a chance to auto-connect themselves
   * ========================================================= */

  Handle(TFunction_Scope) aFuncScope = TFunction_Scope::Set(m_rootLabel);

  // Iterate over the entire collection of Labels involved in Tree Function
  // mechanism in order to set the initial execution statuses for them
  TFunction_DoubleMapIteratorOfDoubleMapOfIntegerLabel aScopeIt( aFuncScope->GetFunctions() );
  NCollection_Map<Handle(ActAPI_IUserParameter), ActAPI_IDataCursor::Hasher> UniqueParams;
  for ( ; aScopeIt.More(); aScopeIt.Next() )
  {
    const TDF_Label& aCurrentLab = aScopeIt.Key2();
    if ( aCurrentLab.IsNull() || !aCurrentLab.NbAttributes() )
      continue;

    // Build function interface under the label containing TFunction_Function attribute
    TFunction_IFunction aFuncInterface(aCurrentLab);

    Handle(ActData_TreeFunctionDriver) aFuncDriver =
      Handle(ActData_TreeFunctionDriver)::DownCast( aFuncInterface.GetDriver() );
    Handle(ActData_BaseTreeFunction) aTreeFuncBase =
      Handle(ActData_BaseTreeFunction)::DownCast( aFuncDriver->GetFunction() );

    Standard_Boolean isUndefinedType;
    //
    Handle(ActAPI_IUserParameter)
      Parameter = ActParamTool::NewParameterSettle( aFuncDriver->Label(), isUndefinedType );

    if ( UniqueParams.Contains(Parameter) )
      continue;
    else
      UniqueParams.Add(Parameter);

    // Let Function connect itself
    aTreeFuncBase->AutoConnect( Parameter->GetNode() );
  }

  /* =======================================================
   *  Now build a graph marking all functions registered to
   *  "heavy" execution for LogBook
   * ======================================================= */

  // Build Execution Graph
  ActData_FuncExecutionCtx::UpdateDependencies(this);

  // Prepare graph for deployment
  m_funcCtx->ForceDeployPropagation(this);

  /* ============================================================
   *  Build a dependency graph for Tree Functions. Then check it
   *  on connectivity anomalies and halt execution if any
   * ============================================================ */

  Handle(ActAPI_HParameterList) aFaultyParams;
  Standard_Integer aGraphState =
    ActData_FuncExecutionCtx::CheckDependencyGraph(this, aFaultyParams);

  if ( aGraphState & ActData_DependencyAnalyzer::GraphState_HasLoops )
    aResult |= Execution_LoopsDetected;
  else if ( aGraphState & ActData_DependencyAnalyzer::GraphState_NoGraph )
  {
    aResult = Execution_NoFunctions;
    return aResult;
  }
  else if ( !(aGraphState & ActData_DependencyAnalyzer::GraphState_Ok) )
  {
    aResult |= Execution_Failed;
    return aResult;
  }

  /* ====================================
   *  Bind transaction data if requested
   * ==================================== */

  if ( !theData.IsEmpty() )
    m_funcCtx->SetTxData(theData);
  else
    m_funcCtx->ReleaseTxData();

  /* ==========================================================
   *  Freeze Execution Graph to prevent it from being modified
   *  by Tree Functions
   * ========================================================== */

  m_funcCtx->FreezeGraph();

  /* ============================================================
   *  Iterate over the dependency graph preparing its Labels for
   *  execution workflow. Here we still have a possibility to
   *  exclude some Labels from the execution "waterfall" if we
   *  found these Labels faulty (e.g. forming a loop in a
   *  dependency graph)
   * ============================================================ */

  // Iterate over the entire collection of Labels involved in Tree Function
  // mechanism in order to set the initial execution statuses for them
  for ( aScopeIt.Initialize( aFuncScope->GetFunctions() ); aScopeIt.More(); aScopeIt.Next() )
  {
    const TDF_Label& aNextLab = aScopeIt.Key2();

    // Find out whether the current Label belongs to a chain of FAULTY
    // Labels reported by dependency graph topological checker. If so,
    // such Labels will be excluded from the execution flow. Thus we make
    // Tree Function engine as robust as possible
    Standard_Boolean isLabelToSkip = Standard_False;
    ActAPI_ParameterList::Iterator aFaultyParamIt( *aFaultyParams.operator->() );
    for ( ; aFaultyParamIt.More(); aFaultyParamIt.Next() )
    {
      const Handle(ActAPI_IUserParameter)& aParam = aFaultyParamIt.Value();
      Handle(ActData_UserParameter) aBaseParam = Handle(ActData_UserParameter)::DownCast(aParam);
      if ( aBaseParam->m_label == aNextLab )
        isLabelToSkip = Standard_True;
    }

    Handle(TFunction_GraphNode) aNextGraphNode;
    if ( aNextLab.FindAttribute(TFunction_GraphNode::GetID(), aNextGraphNode) )
    {
      aNextGraphNode->SetStatus(isLabelToSkip ? TFunction_ES_WrongDefinition : // FAULTY Label
                                                TFunction_ES_NotExecuted);     // GOOD Label
    }
  }

  // Process started
  ActAPI_ProgressEntry aPEntry( m_funcCtx->IsProgressNotifierOn() ? m_funcCtx->ProgressNotifier() : NULL );
  aPEntry.Reset();

  // Plotter
  ActAPI_PlotterEntry aPlotter( m_funcCtx->IsPlotterOn() ? m_funcCtx->Plotter() : NULL );

  // Iterate over the Dependency Graph with sequential (!!!) iterator
  ActData_SequentialFuncIterator aFuncIt(m_rootLabel);

#if defined COUT_DEBUG
  TCollection_AsciiString dumpPrefix(">>>");
#endif

  // NOTICE: it is not possible to iterate independent Tree Functions in a
  //         parallel manner as they still have possibility to be prioritized.
  //         Indeed, Real Evaluation Tree Functions are normally of higher
  //         priority than other ones as they need to be executed first in
  //         order to support implicit parameterization schemes (see
  //         MustExecuteIntact method in BaseTreeFunction class)
  Standard_Integer aCumulRes = 0; // No errors initially
  for ( ; aFuncIt.More(); aFuncIt.Next() )
  {
    const TDF_LabelList& aCurrentFunctions = aFuncIt.Current();
    if ( aCurrentFunctions.IsEmpty() )
      break;

#if defined COUT_DEBUG
    dumpPrefix += "\t";
#endif

    TDF_ListIteratorOfLabelList aCurrentIt(aCurrentFunctions);
    for ( ; aCurrentIt.More(); aCurrentIt.Next() )
    {
      TDF_Label aCurrentLab = aCurrentIt.Value();

      // Build function interface under the label containing TFunction_Function attribute
      TFunction_IFunction aFuncInterface(aCurrentLab);

      /* ===================================================
       *  Access Tree Function to set custom transient data
       * =================================================== */

      Handle(ActData_TreeFunctionDriver) funcDriver =
        Handle(ActData_TreeFunctionDriver)::DownCast( aFuncInterface.GetDriver() );
      Handle(ActData_BaseTreeFunction) funcBase =
        Handle(ActData_BaseTreeFunction)::DownCast( funcDriver->GetFunction() );

      funcBase->SetUserData( m_funcCtx->AccessUserData( funcBase->GetGUID() ) );
      funcBase->SetProgressNotifier( aPEntry.Access() );
      funcBase->SetPlotter( aPlotter.Access() );

#if defined COUT_DEBUG
      std::cout << dumpPrefix.ToCString() << "F: " << funcBase->DynamicType()->Name() << std::endl;
#endif

      /* ====================================
       *  Perform workflow for modified data
       * ==================================== */

      Handle(TFunction_Logbook) log; // TODO: dummy (because OCCT wants it)
      if ( funcDriver->MustExecute(log) ) // Check our custom LogBook
      {
        const Standard_Integer aRes = funcDriver->Execute(log);
        if ( aRes > 0 )
          aCumulRes++; // Cumulate errors
      }

      // Ok, succeeded, so we set internal flag utilized by CAF engine
      aFuncIt.SetStatus(aCurrentLab, TFunction_ES_Succeeded);
    }
  }

  /* ======================================================
   *  Unfreeze Execution Graph and release other resources
   * ====================================================== */

  m_funcCtx->UnFreezeGraph();
  m_funcCtx->CleanFunctions2Deploy();

  /* ==============
   *  Finalization
   * ============== */

  // Prepare final result of execution
  if ( aCumulRes == 0 )
    aResult |= Execution_Done;
  else
    aResult |= Execution_Failed;

  // Clean up LogBook
  this->FuncReleaseLogBook();

  return aResult;
}

//! Returns Tree Function bound to the passed ID and registered in the Model.
//! \param theFuncGUID [in] GUID for Tree Function (unique per Function type).
//! \return requested Tree Function or NULL if not found.
Handle(ActAPI_ITreeFunction) ActData_BaseModel::Function(const Standard_GUID& theFuncGUID) const
{
  if ( !m_treeFunctionMap->IsBound(theFuncGUID) )
    return NULL;

  return m_treeFunctionMap->Find(theFuncGUID);
}

//! Returns all registered Tree Functions for consultation purposes only.
//! \return map of registered Tree Functions.
const Handle(HTreeFunctionMap)& ActData_BaseModel::Functions() const
{
  return m_treeFunctionMap;
}

//! Sets Progress Notifier for execution context.
//! \param thePNotifier [in] Progress Notifier to set.
void ActData_BaseModel::FuncSetProgressNotifier(const Handle(ActAPI_IProgressNotifier)& thePNotifier)
{
  m_funcCtx->SetProgressNotifier(thePNotifier);
}

//! Sets Plotter for execution context.
//! \param thePlotter [in] Plotter to set.
void ActData_BaseModel::FuncSetPlotter(const Handle(ActAPI_IPlotter)& thePlotter)
{
  m_funcCtx->SetPlotter(thePlotter);
}

//! Returns global Progress Notifier.
//! \return Progress Notifier instance.
const Handle(ActAPI_IProgressNotifier)& ActData_BaseModel::FuncProgressNotifier() const
{
  return m_funcCtx->ProgressNotifier();
}

//! Returns global Imperative Plotter.
//! \return Imperative Plotter instance.
const Handle(ActAPI_IPlotter)& ActData_BaseModel::FuncPlotter() const
{
  return m_funcCtx->Plotter();
}

//! Returns true if Progress Notifier is ENABLED, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_BaseModel::FuncIsProgressNotifierOn() const
{
  return m_funcCtx->IsProgressNotifierOn();
}

//! Returns true if Plotter is ENABLED, false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_BaseModel::FuncIsPlotterOn() const
{
  return m_funcCtx->IsPlotterOn();
}

//! Sets Progress Notifier ENABLED.
void ActData_BaseModel::FuncProgressNotifierOn()
{
  m_funcCtx->ProgressNotifierOn();
}

//! Sets Plotter ENABLED.
void ActData_BaseModel::FuncPlotterOn()
{
  m_funcCtx->PlotterOn();
}

//! Sets Progress Notifier DISABLED.
void ActData_BaseModel::FuncProgressNotifierOff()
{
  m_funcCtx->ProgressNotifierOff();
}

//! Sets Plotter DISABLED.
void ActData_BaseModel::FuncPlotterOff()
{
  m_funcCtx->PlotterOff();
}

//! Cleans up the LogBook section.
void ActData_BaseModel::FuncReleaseLogBook()
{
  TDF_Label aLogBookLab = this->accessLogBookSection(Standard_False);

  if ( !aLogBookLab.IsNull() )
    aLogBookLab.ForgetAllAttributes(); // All child Labels will be also cleaned up
}

//! Get execution flags.
//! \return bitwise summed execution flags.
//! \sa ExecutionStatus.
Standard_Integer ActData_BaseModel::FuncExecutionFlags() const
{
  return m_iFuncExecutionFlags;
}

//! Set flags to configure Execution process for Tree Functions.
//! \param theFlags [in] bitwise summed execution flags.
//! \sa ExecutionStatus.
void ActData_BaseModel::FuncSetExecutionFlags(const Standard_Integer theFlags)
{
  m_iFuncExecutionFlags = theFlags;
}

//----------------------------------------------------------------------------
// Construction internals
//----------------------------------------------------------------------------

//! Internal method to be used by descendant classes for implementation of
//! custom Data Models. Normally, descendant class should register all
//! involved Partitions in its constructor. This mechanism is not intended
//! to allow dynamic manipulation with Partitions in runtime.
//! \param theTypeId [in] Partition type ID.
//! \param thePartition [in] Partition instance to register in Data Model.
void ActData_BaseModel::registerPartition(const Standard_Integer theTypeId,
                                          const Handle(ActAPI_IPartition)& thePartition)
{
  // Allocate Partition map if needed
  if ( m_partitionMap.IsNull() )
    m_partitionMap = new HPartitionMap();

  if ( m_partitionMap->IsBound(theTypeId) )
    m_partitionMap->UnBind(theTypeId);

  // Bind the passed Partition to the passed type
  m_partitionMap->Bind(theTypeId, thePartition);

  Handle(ActData_BasePartition) aBasePartition =
    Handle(ActData_BasePartition)::DownCast(thePartition);

  TDF_Label aPartitionLab = m_rootLabel.FindChild(StructureTag_Partitions);
  aBasePartition->settleOn( aPartitionLab.FindChild(theTypeId) );
}

//! Internal method to be used by descendant classes for implementation of
//! custom Data Models. Normally, descendant class should register all
//! involved Tree Functions in its constructor. This mechanism is not intended
//! to allow dynamic manipulation with Tree Functions in runtime.
//! \param thePartition [in] Tree Function to register in Data Model.
void ActData_BaseModel::registerTreeFunction(const Handle(ActAPI_ITreeFunction)& theTreeFunction)
{
  // Allocate Tree Function map if needed
  if ( m_treeFunctionMap.IsNull() )
    m_treeFunctionMap = new HTreeFunctionMap();

  Standard_GUID aFuncGUID( theTreeFunction->GetGUID() );
  if ( m_treeFunctionMap->IsBound(aFuncGUID) )
    return;

  // Bind the passed Tree Function into internal map
  m_treeFunctionMap->Bind(aFuncGUID, theTreeFunction);

  // Register the corresponding Function Driver for OCCT TFunction mechanism
  this->registerFunctionDriver( aFuncGUID,
                                Handle(ActData_BaseTreeFunction)::DownCast(theTreeFunction) );
}

//! Initializes Data Model with the passed CAF Document and prepares integral
//! Data Model Engines.
//! \param theDoc [in] CAF Document to initialize the Model with.
void ActData_BaseModel::init(const Handle(TDocStd_Document)& theDoc)
{
  // Initialize data containers
  m_doc = theDoc;
  m_rootLabel = m_doc->Main().Root();

  // Initialize status
  m_status = MS_Initialized;

  // Initialize Engines
  m_funcCtx = new ActData_FuncExecutionCtx();
  m_copyPasteEngine = new ActData_CopyPasteEngine(this);
  if ( m_bSimpleTxMode )
    m_trEngine = new ActData_TransactionEngine(m_doc);
  else
    m_trEngine = new ActData_ExtTransactionEngine(m_doc);
}

//! Creates new CAF Document.
//! \return just allocated CAF Document.
Handle(TDocStd_Document) ActData_BaseModel::newDocument()
{
  Handle(TDocStd_Document) aResDoc;
  ActData_Application::Instance()->NewDocument(ACTBinFormat, aResDoc);
  return aResDoc;
}

//----------------------------------------------------------------------------
// Tree Function internals
//----------------------------------------------------------------------------

//! Registers Tree Function with the given GUID in the Data Model.
//! \param theGUID [in] GUID of Tree Function type.
//! \param theFunc [in] Tree Function to register.
void ActData_BaseModel::registerFunctionDriver
  (const Standard_GUID& theGUID, const Handle(ActData_BaseTreeFunction)& theFunc)
{
  TFunction_DriverTable::Get()->AddDriver(theGUID, theFunc->m_driver);
}

//----------------------------------------------------------------------------
// Structure management internals
//----------------------------------------------------------------------------

void ActData_BaseModel::getInvariantCopyRefs(ActAPI_FuncGUIDStream& FuncGUIDs,
                                             ActAPI_ParameterLocatorStream& Refs) const
{
  FuncGUIDs << ActData_RealEvaluatorFunc::GUID();
  this->invariantCopyRefs(FuncGUIDs, Refs);
}

void ActData_BaseModel::doDeleteRecursive(const ActAPI_DataObjectId& theNodeId)
{
  /* =============================================
   *  Detach target Node from its parent (if any)
   * ============================================= */

  // Access the target Data Node
  Handle(ActAPI_INode) aNode = this->FindNode(theNodeId);
  if ( !aNode->IsWellFormed() )
    Standard_ProgramError::Raise("Inconsistent CAF data");

  // Access the parent Data Node
  Handle(ActAPI_INode) aParentNode = aNode->GetParentNode();

  // Delete the target Node from the parent one
  if ( !aParentNode.IsNull() )
    aParentNode->RemoveChildNode(aNode);

  /* ====================================
   *  Collect child Data Nodes to delete
   * ==================================== */

  Handle(ActAPI_HNodeList) aChildrenToDelete = new ActAPI_HNodeList();

  Handle(ActAPI_IChildIterator) aChildIt = aNode->GetChildIterator();
  for ( ; aChildIt->More(); aChildIt->Next() )
  {
    Handle(ActAPI_INode) aNextChild = aChildIt->Value();
    aChildrenToDelete->Append(aNextChild);
  }

  /* =====================================================================
   *  Ask the Node to remove itself. The Node will take care of releasing
   *  all Tree Function relations (input readers, output writers,
   *  self-references in LogBook, etc)
   * ===================================================================== */

  Handle(ActData_BaseNode)::DownCast(aNode)->remove( !m_funcCtx->IsGraphFrozen() );

  /* =============================================
   *  Repeat the process for each child Data Node
   * ============================================= */

  ActAPI_NodeList::Iterator aChildListIt( *aChildrenToDelete.operator->() );
  for ( ; aChildListIt.More(); aChildListIt.Next() )
  {
    Handle(ActAPI_INode) aNextChild = aChildListIt.Value();
    this->doDeleteRecursive( aNextChild->GetId() );
  }
}

//! Iterates over the entire Data Model finding all Expressible (Evaluable)
//! Parameters which can be theoretically connected via the evaluation
//! mechanism with the given Variable Node. If their evaluation strings
//! contain references to the given Variable, such connection is done.
//! Otherwise, such Parameters are skipped.
//! \param theVarNode [in] Variable Node to connect to suitable evaluators.
void ActData_BaseModel::chargeEvaluatorsWithVar(const Handle(ActData_BaseVarNode)& theVarNode)
{
  Handle(ActAPI_IUserParameter) aVarParam = theVarNode->Parameter(ActData_BaseVarNode::Param_Value);
  TCollection_AsciiString aVarName = aVarParam->GetName();

  // Iterate over all registered Partitions
  PartitionMap::Iterator aPartIt( *m_partitionMap.operator->() );
  for ( ; aPartIt.More(); aPartIt.Next() )
  {
    const Handle(ActAPI_IPartition)& aPart = aPartIt.Value();
    Handle(ActData_BasePartition) aBasePart = Handle(ActData_BasePartition)::DownCast(aPart);
    
    // Iterate over all Nodes contained in the Partition
    ActData_BasePartition::Iterator aNodeIt(aBasePart);
    for ( ; aNodeIt.More(); aNodeIt.Next() )
    {
      Handle(ActAPI_INode) aNode = aNodeIt.Value();

      if ( aNode.IsNull() )
        continue; // Skip bad Nodes (those which might have been removed)

      if ( ActAPI_IDataCursor::IsEqual(aNode, theVarNode) )
        continue; // Skip the Variable Node itself

      // Iterate over all USER Parameters registered in the Node
      Handle(ActAPI_HIndexedParameterMap) params = aNode->Parameters();
      //
      for ( auto pit = params->cbegin(); pit != params->cend(); ++pit )
      {
        Standard_Integer aRelParamID = pit->first;
        //
        if ( !aNode->IsEvaluable(aRelParamID) )
          continue; // Not evaluable Parameters are not interesting...

        // Check if eval string contains the reference
        Standard_Integer aStart = -1, aEnd = -1;
        TCollection_AsciiString aEvalStr = pit->second->GetEvalString();
        //
        if ( !ActData_StringAux::IsLexeme(aEvalStr, aVarName, aStart, aEnd) )
          continue; // Variable is not referenced here

        /* =========================================
         *  Now connect Evaluator Function properly
         * ========================================= */

        if ( !aNode->HasConnectedEvaluator(aRelParamID) )
          aNode->ConnectEvaluator(aRelParamID);

        Handle(ActData_TreeFunctionParameter) aFuncParam =
          Handle(ActData_TreeFunctionParameter)::DownCast( aNode->Evaluator(aRelParamID) );

        if ( !aFuncParam->HasArgument(aVarParam) )
        {
          Handle(ActAPI_HParameterList) aNewArguments = aFuncParam->Arguments();

          // Remove itself from its arguments in order to re-connect
          // correctly
          aNewArguments->Remove(1);

          // Append newborn variable
          aNewArguments->Append(aVarParam);

          // Reconnect (if already connected, it will automatically disconnect first)
          aNode->ConnectEvaluator(aRelParamID, aNewArguments);

          // We "touch" the affected Parameters as we want to queue evaluation
          // mechanism. The following lines of code make a big deal of
          // registering current Expressible Parameter in a LogBook for
          // consequent Tree Function execution
          const Handle(ActData_UserParameter)&
            aBaseParam = Handle(ActData_UserParameter)::DownCast(pit->second);
          //
          aBaseParam->SetTouched();
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
// Versioning internals
//----------------------------------------------------------------------------

//! Associates a version number with the root TDF Label of the Data Model.
void ActData_BaseModel::bindVersionInfo()
{
  TDF_Label aVersionsLab = m_rootLabel.FindChild(StructureTag_Version);

  // Store version of the Data Framework
  TDataStd_Integer::Set( aVersionsLab.FindChild(VersionTag_Framework),
                         this->actualVersionFramework() );

  // Ask actual implementations for version number
  TDataStd_Integer::Set( aVersionsLab.FindChild(VersionTag_Application),
                         this->actualVersionApp() );
}

//! Returns persistent version number of the Data Framework or -1 if the
//! version number is not bound.
//! \return version number.
Standard_Integer ActData_BaseModel::storedVersionFramework()
{
  TDF_Label aVersionsLab = m_rootLabel.FindChild(StructureTag_Version);
  TDF_Label aFwVersionLab = aVersionsLab.FindChild(VersionTag_Framework, Standard_False);

  if ( aFwVersionLab.IsNull() )
    return -1;

  Handle(TDataStd_Integer) aVersionAttr;
  if ( !aFwVersionLab.FindAttribute(TDataStd_Integer::GetID(), aVersionAttr) )
    return -1;

  return aVersionAttr->Get();
}

//! Returns persistent version number of the custom Application or -1 if the
//! version number is not bound.
//! \return version number.
Standard_Integer ActData_BaseModel::storedVersionApp()
{
  TDF_Label aVersionsLab = m_rootLabel.FindChild(StructureTag_Version);
  TDF_Label anAppVersionLab = aVersionsLab.FindChild(VersionTag_Application, Standard_False);

  if ( anAppVersionLab.IsNull() )
    return -1;

  Handle(TDataStd_Integer) aVersionAttr;
  if ( !anAppVersionLab.FindAttribute(TDataStd_Integer::GetID(), aVersionAttr) )
    return -1;

  return aVersionAttr->Get();
}

//! Return the actual version number of the Data Framework.
//! \return version number.
Standard_Integer ActData_BaseModel::actualVersionFramework()
{
  return ACT_VERSION_HEX;
}

//! Callback function supplying CAF converter required to perform
//! framework-specific conversion of Data Model from older version of
//! the framework to the recent one.
//! \return properly initialized CAF converter.
Handle(ActData_CAFConverter) ActData_BaseModel::converterFw()
{
  return new ActData_CAFConverter( ActData_ConversionStream() << ActData_ConversionTuple(VersionLog_Lot1Iteration4,
                                                                                         VersionLog_Lot2Iteration1,
                                                                                         ActData_ConversionLibrary::v040_to_v050)
                                                              << ActData_ConversionTuple(VersionLog_Lot2Iteration1,
                                                                                         VersionLog_Lot2Iteration2,
                                                                                         ActData_ConversionLibrary::v050_to_v060)
                                                              << ActData_ConversionTuple(VersionLog_Lot2Iteration2,
                                                                                         VersionLog_Lot2Iteration3,
                                                                                         ActData_ConversionLibrary::v060_to_v070)
                                                              << ActData_ConversionTuple(VersionLog_Lot2Iteration3,
                                                                                         VersionLog_Production1,
                                                                                         ActData_ConversionLibrary::v070_to_v080)
                                                              << ActData_ConversionTuple(VersionLog_Production1,
                                                                                         VersionLog_Production2,
                                                                                         ActData_ConversionLibrary::v080_to_v100) );
}

//----------------------------------------------------------------------------
// Copy & Paste internals
//----------------------------------------------------------------------------

//! Returns the CAF Label representing the Copy/Paste Buffering section in
//! the working CAF Document.
//! \param toCreate [in] indicates whether the Copy/Paste Buffering section
//!        Label must be created if it does not exist yet.
//! \return Label representing the Copy/Paste Buffering section.
TDF_Label ActData_BaseModel::accessCopyPasteSection(const Standard_Boolean toCreate)
{
  return m_rootLabel.FindChild(StructureTag_CopyPasteBuffer, toCreate);
}

//! Cleans up the internal buffer for Copy & Paste functionality.
void ActData_BaseModel::releaseCopyPasteBuffer()
{
  TDF_Label aBufferLab = this->accessCopyPasteSection(Standard_False);

  if ( !aBufferLab.IsNull() )
    aBufferLab.ForgetAllAttributes(); // All child Labels will be also cleaned up
}

//----------------------------------------------------------------------------
// Tree Function logging internals
//----------------------------------------------------------------------------

//! Returns the CAF Label representing the Execution LogBook section in
//! the working CAF Document.
//! \param toCreate [in] indicates whether the requested section Label must be
//!        created in case it does not exist yet.
//! \return Label representing the Execution LogBook section.
TDF_Label ActData_BaseModel::accessLogBookSection(const Standard_Boolean toCreate)
{
  return m_rootLabel.FindChild(StructureTag_LogBook, toCreate);
}
