//-----------------------------------------------------------------------------
// Created on: June 2012
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
#include <ActData_CopyPasteEngine.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_NodeFactory.h>
#include <ActData_ParameterFactory.h>

// Active Data (API) includes
#include <ActAPI_IPartition.h>

// OCCT includes
#include <TDataStd_TreeNode.hxx>
#include <TDF_ClosureMode.hxx>
#include <TDF_ClosureTool.hxx>
#include <TDF_CopyLabel.hxx>
#include <TDF_CopyTool.hxx>
#include <TDF_DataMapIteratorOfLabelDataMap.hxx>
#include <TDF_DataSet.hxx>
#include <TDF_LabelDataMap.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>

#define COPY_SUFFIX_BUFF " [copy]"
#define COPY_SUFFIX "*"

#undef DUMP_CAF_DATA
#if defined DUMP_CAF_DATA
  #include <ActData_CAFDumper.h>

  #define FILE_DEBUG_DUMPING_PATH "D://"
#endif

// Initialize default suffix option.
ActData_CopyPasteEngine::SuffixOptions
  ActData_CopyPasteEngine::_suffixOption = ActData_CopyPasteEngine::Option_SuffixAll;

//! Default constructor.
ActData_CopyPasteEngine::ActData_CopyPasteEngine()
{
}

//! Creates a new instance of Copy/Paste Tool charged with the entire Model.
//! \param Model [in] entire Data Model.
ActData_CopyPasteEngine::ActData_CopyPasteEngine(const Handle(ActAPI_IModel)& Model)
{
  this->Init(Model);
}

//! Initializes the Copy/Paste Tool with the given Model.
//! \param Model [in] entire Data Model.
void ActData_CopyPasteEngine::Init(const Handle(ActAPI_IModel)& Model)
{
  m_model = Model;
}

//! Transfers data from the source Node into the buffering section.
//! \param Source [in] source Node to transfer to the buffering section.
//! \return copying result. True in case of success, false -- otherwise.
Standard_Boolean
  ActData_CopyPasteEngine::TransferToBuffer(const Handle(ActAPI_INode)& Source)
{
  m_status = Status_NoError; // Let further process override it

  // Clean up the buffering section
  Handle(ActData_BaseModel)::DownCast(m_model)->releaseCopyPasteBuffer();

  // Perform data transferring
  return this->transfer(Source, Direction_ToBuffer);
}

//! Dispatches the buffered user tree from buffering section distributing
//! the Nodal copies by proper Partitions. This process is fully analogical
//! to those performed by TransferToBuffer method.
//! \return root Node of copying result.
Handle(ActAPI_INode) ActData_CopyPasteEngine::RestoreFromBuffer()
{
  m_status = Status_NoError; // Let further process override it

  // Access the first buffered data Node as a source one
  Handle(ActAPI_INode) Source = this->GetRootBuffered();

  // Perform data transferring
  if ( !this->transfer(Source, Direction_FromBuffer) )
    return NULL;

  return ActData_NodeFactory::NodeSettle( m_relocTable[1].Find1( Source->RootLabel() ) );
}

//! Accessor for the working Model.
//! \return Model instance.
const Handle(ActAPI_IModel)& ActData_CopyPasteEngine::GetModel() const
{
  return m_model;
}

//! Sets working Model instance.
//! \param theModel [in] Model instance to set.
void ActData_CopyPasteEngine::SetModel(const Handle(ActAPI_IModel)& theModel)
{
  m_model = theModel;
}

//! Accessor for the buffering HEAD CAF Label.
//! \return buffering Label.
TDF_Label ActData_CopyPasteEngine::GetBufferHead() const
{
  Handle(ActData_BaseModel) aModelBase = Handle(ActData_BaseModel)::DownCast(m_model);
  return aModelBase->accessCopyPasteSection().FindChild(1);
}

//! Accessor for the root buffered Node.
//! \return root buffered Node.
Handle(ActAPI_INode) ActData_CopyPasteEngine::GetRootBuffered() const
{
  TDF_Label aNodeRoot = this->GetBufferHead().FindChild(1, Standard_False);
  if ( aNodeRoot.IsNull() )
    return NULL;

  return ActData_NodeFactory::NodeSettle(aNodeRoot);
}

//! Read-only accessor for Relocation Table.
//! \param isFirstStage [in] indicates whether the Relocation Table is
//!        requested for the first (copying) stage. The alternative is
//!        the pasting stage.
//! \return Relocation Table.
const ActData_CopyPasteEngine::RelocationTable&
  ActData_CopyPasteEngine::GetRelocationTable(const Standard_Boolean isFirstStage) const
{
  return (isFirstStage ? m_relocTable[0] : m_relocTable[1]);
}

//! Read-only accessor for Sampler Tree.
//! \return Sampler Tree.
const ActData_SamplerTreeNode& ActData_CopyPasteEngine::GetSamplerTree() const
{
  return m_sTree;
}

//! Accessor for the Reference Filter.
//! \return Reference Filter instance.
ActData_CopyPasteEngine::ReferenceFilter& ActData_CopyPasteEngine::AccessReferenceFilter()
{
  return m_refFilter;
}

//! Disables string suffixes.
void ActData_CopyPasteEngine::SetSuffixOptionNone()
{
  _suffixOption = Option_SuffixNone;
}

//! Enables suffixes on root copied Nodes only.
void ActData_CopyPasteEngine::SetSuffixOptionForRoot()
{
  _suffixOption = Option_SuffixForRoot;
}

//! Enables suffixes on all Nodes including nested children.
void ActData_CopyPasteEngine::SetSuffixOptionAll()
{
  _suffixOption = Option_SuffixAll;
}

//! Kernel method performing actual data transferring and serving for both
//! copying and pasting procedures.
//! \param theSource    [in] Data Node to copy.
//! \param theDirection [in] copying direction.
Standard_Boolean
  ActData_CopyPasteEngine::transfer(const Handle(ActAPI_INode)& theSource,
                                    const CopyDirection         theDirection)
{
  // Check the source Data Node
  if ( theSource.IsNull() || !theSource->IsWellFormed() )
    return Standard_False;

  Standard_Integer aRTIndex = (theDirection == Direction_ToBuffer ? 0 : 1);

  // Prepare utilities
  m_sTree = ActData_SamplerTreeNode();
  m_relocTable[aRTIndex].Clear();

#if defined DUMP_CAF_DATA
  static Standard_Integer PASSB = 0; ++PASSB;
  TCollection_AsciiString aBaseDumpNameB("CAFDumper_transfer_before_"); aBaseDumpNameB += PASSB; aBaseDumpNameB += ".log";
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat(aBaseDumpNameB), m_model);
#endif

  // Build copy of the Nodal hierarchy in the buffer
  this->flattenTree(theSource, m_sTree, theDirection, _suffixOption != Option_SuffixNone);

#if defined DUMP_CAF_DATA
  static Standard_Integer PASSA = 0; ++PASSA;
  TCollection_AsciiString aBaseDumpNameA("CAFDumper_transfer_after_"); aBaseDumpNameA += PASSA; aBaseDumpNameA += ".log";
  ActData_CAFDumper::Dump(TCollection_AsciiString(FILE_DEBUG_DUMPING_PATH).Cat(aBaseDumpNameA), m_model);
#endif

  this->rebuildTreeLinks(theDirection);

  // Normalize references in the buffered tree
  Handle(ActAPI_INode)
    aRootCopy = ActData_NodeFactory::NodeSettle( m_relocTable[aRTIndex].Find1( theSource->RootLabel() ) );
  //
  this->normalizeBackReferences(aRootCopy, theDirection);
  this->normalizeDirectReferences(aRootCopy, theDirection);

  return Standard_True;
}

//! Copies the passed sub-tree starting from the given root Node. Internally,
//! this method recursively dispatches the user-tree into plain storage list.
//! \param theRoot      [in]  root Node of the sub-tree being copied.
//! \param theSTree     [out] Sampler Tree being accumulated.
//! \param theDirection [in]  copying direction (to/from buffer).
//! \param doUseSuffix  [in]  indicates whether to adjust the name of the copied
//!                           Nodes with a specific suffix.
void ActData_CopyPasteEngine::flattenTree(const Handle(ActAPI_INode)& theRoot,
                                          ActData_SamplerTreeNode&    theSTree,
                                          const CopyDirection         theDirection,
                                          const Standard_Boolean      doUseSuffix)
{
  this->copyNode(theRoot, theDirection, doUseSuffix);
  theSTree.ID = theRoot->GetId();

  Handle(ActAPI_IChildIterator) aChildIt = theRoot->GetChildIterator();
  for ( ; aChildIt->More(); aChildIt->Next() )
  {
    Handle(ActAPI_INode) aChildNode = aChildIt->Value();
    Standard_Boolean doChildSuffix = (_suffixOption == Option_SuffixAll);
    //
    this->flattenTree(aChildNode, theSTree.NewChild(), theDirection, doChildSuffix);
  }
}

//! Builds user-tree connectivity between the buffered Nodes according to the
//! passed sample tree.
//! \param theDirection [in] copying direction.
void ActData_CopyPasteEngine::rebuildTreeLinks(const CopyDirection theDirection)
{
  for ( ActData_SamplerTreeNode::Iterator anIt(m_sTree, Standard_True); anIt.More(); anIt.Next() )
  {
    /* ===========================================================
     *  Access buffered copy for the currently iterated source ID
     * =========================================================== */

    const ActData_SamplerTreeNode* aSampleIDPtr = anIt.Current();
    Handle(ActAPI_INode)
      aCurrentCopy = this->getCopyBySampleID( (*aSampleIDPtr).ID, theDirection );

    /* ==============================================
     *  Build Tree Node connectivity between current
     *  copy and its buffered children
     * ============================================== */

    const NCollection_Sequence<ActData_SamplerTreeNode>& aChildIDs = aSampleIDPtr->Children;
    if ( aChildIDs.IsEmpty() )
      continue;

    // Iterate over the child IDs to add their corresponding buffered Nodes
    // as children to the currently iterated Data Node
    for ( NCollection_Sequence<ActData_SamplerTreeNode>::Iterator anIt1(aChildIDs); anIt1.More(); anIt1.Next() )
    {
      const ActData_SamplerTreeNode& aChildSampleID = anIt1.Value();
      Handle(ActAPI_INode)
        aCurrentChild = this->getCopyBySampleID(aChildSampleID.ID, theDirection);
      aCurrentCopy->AddChildNode(aCurrentChild);
    }
  }
}

//! Adjusts back-references for the copied hierarchy of Nodes so that to keep
//! only in-scoped ones. The out-scoped back references are removed as they
//! cannot be consistent ("external world" Nodes are not affected).
//! \param theRoot      [in] root Node to start normalization process from.
//! \param theDirection [in] copying direction (to/from buffer).
void ActData_CopyPasteEngine::normalizeBackReferences(const Handle(ActAPI_INode)& theRoot,
                                                      const CopyDirection         theDirection)
{
  Handle(ActData_BaseNode) aNodeBase = Handle(ActData_BaseNode)::DownCast(theRoot);

  /* =====================
   *  Normalize OBSERVERS
   * ===================== */

  this->normalizeObservers(aNodeBase, ActData_BaseNode::Observer_InputReaders,  theDirection);
  this->normalizeObservers(aNodeBase, ActData_BaseNode::Observer_OutputWriters, theDirection);
  this->normalizeObservers(aNodeBase, ActData_BaseNode::Observer_Referrers,     theDirection);

  /* ============================================
   *  Perform normalization process for children
   * ============================================ */

  Handle(ActAPI_IChildIterator) aChildIt = theRoot->GetChildIterator();
  for ( ; aChildIt->More(); aChildIt->Next() )
  {
    Handle(ActAPI_INode) aChildNode = aChildIt->Value();
    this->normalizeBackReferences(aChildNode, theDirection);
  }
}

//! Establishes consistent connectivity for Tree Function and Reference
//! Parameters on the copy of Nodal hierarchy.
//! \param theRoot      [in] root Node to start normalization process.
//! \param theDirection [in] copying direction (to/from buffer).
void ActData_CopyPasteEngine::normalizeDirectReferences(const Handle(ActAPI_INode)& theRoot,
                                                        const CopyDirection         theDirection)
{
  Handle(ActData_BaseNode) aNodeBase = Handle(ActData_BaseNode)::DownCast(theRoot);

  /* =======================================
   *  Normalize TREE FUNCTIONS & REFERENCES
   * ======================================= */

  Handle(ActAPI_HNodalParameterList) aParams = aNodeBase->accessAllParameters();
  for ( ActAPI_NodalParameterList::Iterator aParamIt( *aParams.operator->() );
        aParamIt.More(); aParamIt.Next() )
  {
    ActAPI_NodalParameter aNodalParam = aParamIt.Value();
    Handle(ActAPI_IUserParameter) aParam = aNodalParam.Parameter;

    if ( aParam->IsKind( STANDARD_TYPE(ActData_TreeFunctionParameter) ) )
    {
      Handle(ActData_TreeFunctionParameter)
        aTFuncParam = ActData_ParameterFactory::AsTreeFunction(aParam);
      //
      this->normalizeTreeFunction(aNodeBase, aNodalParam.RelativeId, aNodalParam.IsInternal, theDirection);
    }
    else if ( aParam->IsKind( STANDARD_TYPE(ActData_ReferenceParameter) ) )
    {
      Handle(ActData_ReferenceParameter)
        aRefParam = ActData_ParameterFactory::AsReference(aParam);
      //
      this->normalizeReference(aNodeBase, aNodalParam.RelativeId, theDirection);
    }
    else if ( aParam->IsKind( STANDARD_TYPE(ActData_ReferenceListParameter) ) &&
              !aNodalParam.IsInternal ) // Skip internals (they are back-references)
    {
      Handle(ActData_ReferenceListParameter)
        aRefParam = ActData_ParameterFactory::AsReferenceList(aParam);
      //
      this->normalizeReferenceList(aNodeBase, aNodalParam.RelativeId, theDirection);
    }
  }

  /* ============================================
   *  Perform normalization process for children
   * ============================================ */

  Handle(ActAPI_IChildIterator) aChildIt = theRoot->GetChildIterator();
  for ( ; aChildIt->More(); aChildIt->Next() )
  {
    Handle(ActAPI_INode) aChildNode = aChildIt->Value();
    //
    this->normalizeDirectReferences(aChildNode, theDirection);
  }
}

//! Disconnects out-scoped Tree Function and adjusts in-scoped one.
//! \param theNode      [in] Data Node to perform normalization.
//! \param theTFuncID   [in] ID of the Tree Function Parameter.
//! \param isInternal   [in] indicates whether the Parameter requested for
//!                          normalization is internal one.
//! \param theDirection [in] copying direction (to/from buffer).
void ActData_CopyPasteEngine::normalizeTreeFunction(const Handle(ActData_BaseNode)& theNode,
                                                    const Standard_Integer          theTFuncID,
                                                    const Standard_Boolean          isInternal,
                                                    const CopyDirection             theDirection)
{
  Handle(ActData_TreeFunctionParameter) aTFuncParam = theNode->accessFuncParameter(theTFuncID, isInternal);
  Standard_Boolean isTFuncInScoped = Standard_True;

  // Prepare collections for out-scoped Parameters
  Handle(ActAPI_HParameterList) OutScopedArguments = new ActAPI_HParameterList();
  Handle(ActAPI_HParameterList) OutScopedResults = new ActAPI_HParameterList();

  Standard_Integer aRTIndex = (theDirection == Direction_ToBuffer ? 0 : 1);

  /* =============================
   *  Check & normalize ARGUMENTS
   * ============================= */

  Handle(ActAPI_HParameterList) anArgsList = aTFuncParam->Arguments();
  if ( !anArgsList.IsNull() )
  {
    Handle(TDataStd_ReferenceList) anArgsRefsAttr = aTFuncParam->getArgumentsAttr();
    anArgsRefsAttr->Clear();

    for ( ActAPI_ParameterList::Iterator anArgsIt( *anArgsList.operator->() );
          anArgsIt.More(); anArgsIt.Next() )
    {
      Handle(ActAPI_IUserParameter) anArg = anArgsIt.Value();

      // Invalid argument exists. This means that Tree Function's fundamental
      // data is corrupted and cannot be used for execution flow. This can
      // happen, for instance, with out-scoped arguments/results removed
      // between Copy & Paste invocations. The only thing we can do here
      // is to disable such Tree Function and inform user that data
      // consistency has been broken
      if ( anArg.IsNull() )
      {
        theNode->disconnectTreeFunction(theTFuncID, isInternal, Standard_True);
        m_status = Status_WarnNullFuncArgument;
        return;
      }

      TDF_Label aRefLabel = anArg->RootLabel();

      // If the passed reference is not an original one, we are trying to
      // get the original by Relocation Table. This addendum allows us
      // to get rid of "smart" relocation mechanism for "self-contained"
      // Labels embedded to OCAF. Having this mechanism mandatory working,
      // we are forced to complexify our classification routine with such
      // stuff...
      TDF_Label aRefSrc;
      if ( m_relocTable[aRTIndex].IsBound2(aRefLabel) )
        aRefSrc = m_relocTable[aRTIndex].Find2(aRefLabel);
      else
        aRefSrc = aRefLabel;

      ActData_RefClassifier::RefScope
        aRefScope = ActData_RefClassifier::Classify(aRefSrc, m_sTree);

      if ( aRefScope == ActData_RefClassifier::RefScope_OUT )
      {
        Standard_Boolean isUndefinedType;
        isTFuncInScoped = Standard_False;
        anArgsRefsAttr->Append(aRefSrc); // Keep it as-is
        OutScopedArguments->Append( ActData_ParameterFactory::NewParameterSettle(aRefSrc, isUndefinedType) );
      }
      else
      {
        TDF_Label aCopyRefLabel = m_relocTable[aRTIndex].Find1(aRefSrc);
        anArgsRefsAttr->Append(aCopyRefLabel);
      }
    }
  }

  /* ===========================
   *  Check & normalize RESULTS
   * =========================== */

  Handle(ActAPI_HParameterList) aResList = aTFuncParam->Results();
  if ( !aResList.IsNull() )
  {
    Handle(TDataStd_ReferenceList) aResRefsAttr = aTFuncParam->getResultsAttr();
    aResRefsAttr->Clear();

    for ( ActAPI_ParameterList::Iterator aResIt( *aResList.operator->() );
          aResIt.More(); aResIt.Next() )
    {
      Handle(ActAPI_IUserParameter) aRes = aResIt.Value();

      // Invalid argument exists. This means that Tree Function's fundamental
      // data is corrupted and cannot be used for execution flow. This can
      // happen, for instance, with out-scoped arguments/results removed
      // between Copy & Paste invocations. The only thing we can do here
      // is to disable such Tree Function and inform user that data
      // consistency has been broken
      if ( aRes.IsNull() )
      {
        theNode->disconnectTreeFunction(theTFuncID, isInternal, Standard_True);
        m_status = Status_WarnNullFuncResult;
        return;
      }

      TDF_Label aRefLabel = aRes->RootLabel();

      // If the passed reference is not an original one, we are trying to
      // get the original by Relocation Table. This addendum allows us
      // to get rid of "smart" relocation mechanism for "self-contained"
      // Labels embedded to OCAF. Having this mechanism mandatory working,
      // we are forced to complexify our classification routine with such
      // stuff...
      TDF_Label aRefSrc;
      if ( m_relocTable[aRTIndex].IsBound2(aRefLabel) )
        aRefSrc = m_relocTable[aRTIndex].Find2(aRefLabel);
      else
        aRefSrc = aRefLabel;

      ActData_RefClassifier::RefScope
        aRefScope = ActData_RefClassifier::Classify(aRefSrc, m_sTree);

      if ( aRefScope == ActData_RefClassifier::RefScope_OUT )
      {
        Standard_Boolean isUndefinedType;
        isTFuncInScoped = Standard_False;
        aResRefsAttr->Append(aRefSrc); // Keep it as-is
        OutScopedResults->Append( ActData_ParameterFactory::NewParameterSettle(aRefSrc, isUndefinedType) );
      }
      else
      {
        TDF_Label aCopyRefLabel = m_relocTable[aRTIndex].Find1(aRefSrc);
        aResRefsAttr->Append(aCopyRefLabel);
      }
    }
  }

  /* =========================================
   *  Treat somehow out-scoped Tree Functions
   * ========================================= */

  Standard_GUID aFuncGUID;

  // Little trick to have the copy of the Function in the global Function Scope
  if ( aTFuncParam->GetDriverGUID(aFuncGUID) && (theDirection == Direction_FromBuffer) )
    aTFuncParam->SetDriverGUID(aFuncGUID);

  // Check if the out-scoped Tree Function is requested to be passed through
  // the normalization process nevertheless
  if ( !isTFuncInScoped && m_refFilter.AccessTreeFunctionFilter().IsPassingOutScoped(aFuncGUID) )
  {
    // Iterate over out-scoped references and register current Tree Function
    // as INPUT Reader and OUTPUT Writer in the correspondent out-scoped
    // Data Nodes
    for ( ActAPI_ParameterList::Iterator osIt( *OutScopedArguments.operator->() ); osIt.More(); osIt.Next() )
    {
      Handle(ActAPI_INode) anExternalNode = osIt.Value()->GetNode();

      // Do not allow back-reference to itself
      // Do not allow external referring to buffer
      if ( !ActAPI_IDataCursor::IsEqual(anExternalNode, theNode) && (theDirection == Direction_FromBuffer) )
        Handle(ActData_BaseNode)::DownCast(anExternalNode)->connectReader(aTFuncParam);
    }
    for ( ActAPI_ParameterList::Iterator osIt( *OutScopedResults.operator->() ); osIt.More(); osIt.Next() )
    {
      Handle(ActAPI_INode) anExternalNode = osIt.Value()->GetNode();

      // Do not allow back-reference to itself
      // Do not allow external referring to buffer
      if ( !ActAPI_IDataCursor::IsEqual(anExternalNode, theNode) && (theDirection == Direction_FromBuffer) )
        Handle(ActData_BaseNode)::DownCast(anExternalNode)->connectWriter(aTFuncParam);
    }
  }
  else if ( !isTFuncInScoped )
  {
    // Notice that we use "smart" disconnection here in order to remove the
    // back references produced by this Function as well. It is safe only in
    // case when entire set of back references has been already normalized
    // before this method is called
    theNode->disconnectTreeFunction(theTFuncID, isInternal, Standard_True);
  }
}

//! Normalizes plain Reference Parameters.
//! \param theNode      [in] Node to normalize a Reference Parameter in.
//! \param theRefID     [in] relative ID of the Reference Parameter to normalize.
//! \param theDirection [in] copying direction (to/from buffer).
void ActData_CopyPasteEngine::normalizeReference(const Handle(ActData_BaseNode)& theNode,
                                                 const Standard_Integer          theRefID,
                                                 const CopyDirection             theDirection)
{
  Handle(ActData_ReferenceParameter)
    RefParam = ActData_ParameterFactory::AsReference( theNode->Parameter(theRefID) );
  //
  if ( RefParam->GetTarget().IsNull() )
    return;

  Standard_Integer aRTIndex = (theDirection == Direction_ToBuffer ? 0 : 1);

  TCollection_AsciiString NodeType = theNode->GetTypeName();
  Handle(ActAPI_IDataCursor) aTargetDC = RefParam->GetTarget();
  TDF_Label aRefLabel = aTargetDC->RootLabel();
  ActData_RefClassifier::RefScope aRefScope = ActData_RefClassifier::Classify(aRefLabel, m_sTree);

  if ( aRefScope == ActData_RefClassifier::RefScope_OUT )
  {
    Standard_Boolean isPassOutScoped = m_refFilter.AccessRefParamFilter().IsPassingOutScoped(NodeType, theRefID);

    // Add new back-reference (observer) for the pointed Data Cursor. Now not
    // only the source Node is an observer, but its copy -- as well
    if ( isPassOutScoped && (theDirection == Direction_FromBuffer) )
    {
      Handle(ActAPI_INode)
        aTargetNode = ( aTargetDC->IsKind( STANDARD_TYPE(ActAPI_INode) ) ? Handle(ActAPI_INode)::DownCast(aTargetDC) :
                                                                           Handle(ActAPI_IUserParameter)::DownCast(aTargetDC)->GetNode() );
      Handle(ActData_BaseNode)::DownCast(aTargetNode)->connectReferrer(RefParam);
    }

    if ( !isPassOutScoped )
      RefParam->RemoveTarget();
  }
  else
  {
    TDF_Label aCopyRefLabel = m_relocTable[aRTIndex].Find1(aRefLabel);
    RefParam->SetTarget(aCopyRefLabel);
  }
}

//! Normalizes Reference List Parameters.
//! \param theNode      [in] Node to normalize a Reference List Parameter in.
//! \param theRefID     [in] relative ID of the Reference List Parameter
//!                          to normalize.
//! \param theDirection [in] copying direction (to/from buffer).
void ActData_CopyPasteEngine::normalizeReferenceList(const Handle(ActData_BaseNode)& theNode,
                                                     const Standard_Integer          theRefID,
                                                     const CopyDirection             theDirection)
{
  Handle(ActData_ReferenceListParameter)
    RefParam = ActData_ParameterFactory::AsReferenceList( theNode->Parameter(theRefID) );
  //
  if ( RefParam->AccessReferenceList().IsNull() ||
       RefParam->AccessReferenceList()->IsEmpty() )
    return;

  Standard_Integer aRTIndex = (theDirection == Direction_ToBuffer ? 0 : 1);
  TCollection_AsciiString NodeType = theNode->GetTypeName();
  Standard_Boolean isPassOutScoped = m_refFilter.AccessRefParamFilter().IsPassingOutScoped(NodeType, theRefID);

  // Iterate over the referenced items to check their in-scoped/out-scoped
  // properties
  Handle(ActAPI_HDataCursorList) aDCList = RefParam->GetTargets();
  for ( ActAPI_DataCursorList::Iterator it( *aDCList.operator->() ); it.More(); it.Next() )
  {
    const Handle(ActAPI_IDataCursor)& aTargetDC = it.Value();

    // Classify target against scope filter
    TDF_Label aRefLabel = aTargetDC->RootLabel();
    ActData_RefClassifier::RefScope aRefScope = ActData_RefClassifier::Classify(aRefLabel, m_sTree);

    if ( aRefScope == ActData_RefClassifier::RefScope_OUT )
    {
      // Add new back-reference (observer) for the pointed Data Cursor. Now not
      // only the source Node is an observer, but its copy -- as well
      if ( isPassOutScoped && (theDirection == Direction_FromBuffer) )
      {
        Handle(ActAPI_INode)
          aTargetNode = ( aTargetDC->IsKind( STANDARD_TYPE(ActAPI_INode) ) ? Handle(ActAPI_INode)::DownCast(aTargetDC) :
                                                                             Handle(ActAPI_IUserParameter)::DownCast(aTargetDC)->GetNode() );
        Handle(ActData_BaseNode)::DownCast(aTargetNode)->connectReferrer(RefParam);
      }

      if ( !isPassOutScoped )
        RefParam->RemoveTarget(aTargetDC);
    }
    else
    {
      TDF_Label aCopyRefLabel = m_relocTable[aRTIndex].Find1(aRefLabel);
      RefParam->ExchangeTarget(aRefLabel, aCopyRefLabel, MT_Silent);
    }
  }
}

//! Removes out-scoped observer back-references and adjusts in-scoped
//! observer references.
//! \param theNode         [in] Data Node to adjust observers for.
//! \param theObserverType [in] type of observers to normalize.
//! \param theDirection    [in] copying direction (to/from buffer).
void ActData_CopyPasteEngine::normalizeObservers(const Handle(ActData_BaseNode)&      theNode,
                                                 const ActData_BaseNode::ObserverType theObserverType,
                                                 const CopyDirection                  theDirection)
{
  Standard_Integer               aRTIndex = (theDirection == Direction_ToBuffer ? 0 : 1);
  Handle(TDataStd_ReferenceList) aRefsAttr;
  Handle(ActAPI_HDataCursorList) anObserverList;
  //
  switch ( theObserverType )
  {
    case ActData_BaseNode::Observer_InputReaders:
      aRefsAttr      = theNode->m_paramScope.Meta->GetInputReadersAttr();
      anObserverList = theNode->m_paramScope.Meta->GetInputReaderCursors();
      break;
    case ActData_BaseNode::Observer_OutputWriters:
      aRefsAttr      = theNode->m_paramScope.Meta->GetOutputWritersAttr();
      anObserverList = theNode->m_paramScope.Meta->GetOutputWriterCursors();
      break;
    case ActData_BaseNode::Observer_Referrers:
      aRefsAttr      = theNode->m_paramScope.Meta->GetReferrersAttr();
      anObserverList = theNode->m_paramScope.Meta->GetReferrerCursors();
      break;
    default:
      Standard_ProgramError::Raise("Unexpected observer type");
  }

  // We start with cleaning up the attribute containing back-references.
  // This attribute is going to be re-filled according to the results of
  // in / out scope classification
  if ( !aRefsAttr.IsNull() )
    aRefsAttr->Clear(); // Clean up all observers

  // Iterate over the observers
  for ( ActAPI_DataCursorList::Iterator aParamIt( *anObserverList.operator->() );
        aParamIt.More(); aParamIt.Next() )
  {
    Handle(ActAPI_IUserParameter)   anObserver = Handle(ActAPI_IUserParameter)::DownCast( aParamIt.Value() );
    TDF_Label                       aRefLabel  = anObserver->RootLabel();
    ActData_RefClassifier::RefScope aRefScope  = ActData_RefClassifier::Classify(aRefLabel, m_sTree);

    if ( aRefScope == ActData_RefClassifier::RefScope_IN )
    {
      TDF_Label aCopyRefLabel = m_relocTable[aRTIndex].Find1(aRefLabel);
      aRefsAttr->Append(aCopyRefLabel);
    }
  }
}

//! Return the Nodal buffered copy by the sample source ID.
//! \param theSampleID [in] sample source ID.
//! \param theDirection [in] copying direction.
Handle(ActAPI_INode)
  ActData_CopyPasteEngine::getCopyBySampleID(const ActAPI_DataObjectId& theSampleID,
                                             const CopyDirection        theDirection) const
{
  Standard_Integer aRTIndex = (theDirection == Direction_ToBuffer ? 0 : 1);

  // Access the root Label for the copy of the Node
  TDF_Label aRootOfSource;
  TDF_Tool::Label(m_model->RootLabel().Data(), theSampleID, aRootOfSource, Standard_False);
  TDF_Label aRootOfCopy = m_relocTable[aRTIndex].Find1(aRootOfSource);

  // Settle down the Nodal instance & return
  return ActData_NodeFactory::NodeSettle(aRootOfCopy);
}

//! Copies the passed Node to the buffer or to the proper Partition depending
//! on the given Direction qualificator.
//! \param theNode      [in] Node to copy.
//! \param theDirection [in] copying direction.
//! \param doUseSuffix  [in] indicates whether to use suffix for adjusting the
//!                          name of the copied Node.
//! \return new Label corresponding to the root of the Nodal copy.
TDF_Label ActData_CopyPasteEngine::copyNode(const Handle(ActAPI_INode)& theNode,
                                            const CopyDirection         theDirection,
                                            const Standard_Boolean      doUseSuffix)
{
  // Choose root Label for the prepared copy
  TDF_Label aSectionRoot;
  if ( theDirection == Direction_ToBuffer )
    aSectionRoot = this->GetBufferHead();
  else
  {
    TCollection_AsciiString aNodeType = theNode->GetTypeName();
    Handle(ActAPI_IPartition) aPartition = m_model->Partition(aNodeType);
    aSectionRoot = aPartition->RootLabel();

    // Update MTime for Nodal Parameter
    Handle(ActAPI_HNodalParameterList) aParams = Handle(ActData_BaseNode)::DownCast(theNode)->accessAllParameters();
    for ( ActAPI_NodalParameterList::Iterator it( *aParams.operator->() ); it.More(); it.Next() )
      it.Value().Parameter->SetModified();
  }

  /* ==================
   *  Raw OCAF copying
   * ================== */

  // Prepare source and target Labels
  TDF_Label aSrcLab = theNode->RootLabel();
  TDF_Label aNodeRoot = TDF_TagSource::NewChild(aSectionRoot);

  // Prepare filter for Attributes
  TDF_IDFilter anAttrFilter(Standard_False); // Set filter in IGNORE mode
  anAttrFilter.Ignore( TDataStd_TreeNode::GetDefaultTreeID() );
  anAttrFilter.Ignore( TFunction_GraphNode::GetID() );

  // Prepare CAF relocation table and Closure Mode
  Handle(TDF_RelocationTable) aNativeRelocTable = new TDF_RelocationTable(Standard_True);
  Handle(TDF_DataSet) aDataSet = new TDF_DataSet();
  TDF_ClosureMode aClosureMode(Standard_True);
  aClosureMode.References(Standard_False); // Exclude references as we treat them ourselves
  aDataSet->AddLabel(aSrcLab);
  aNativeRelocTable->SetRelocation(aSrcLab, aNodeRoot);

  // Perform CAF closure without deep traversing over TDF references as
  // we are going to normalize them ourselves
  TDF_ClosureTool::Closure(aDataSet, anAttrFilter, aClosureMode);

  // Perform actual raw OCAF copying
  TDF_CopyTool::Copy(aDataSet, aNativeRelocTable);

  /* ==============
   *  Finalization
   * ============== */

  // Fill relocation table
  TDF_LabelDataMap& aRelocDataMap = aNativeRelocTable->LabelTable(); 
  for ( TDF_DataMapIteratorOfLabelDataMap mapIt(aRelocDataMap); mapIt.More(); mapIt.Next() )
    if ( theDirection == Direction_ToBuffer )
      m_relocTable[0].Bind( mapIt.Key(), mapIt.Value() );
    else
      m_relocTable[1].Bind( mapIt.Key(), mapIt.Value() );

  if ( doUseSuffix )
  {
    // Adjust name of copy
    TCollection_ExtendedString aNewName;
    if ( theDirection == Direction_ToBuffer )
      aNewName = theNode->GetName().Cat(COPY_SUFFIX_BUFF);
    else
      aNewName = theNode->GetName().Cat(COPY_SUFFIX);
    ActData_NodeFactory::NodeSettle(aNodeRoot)->SetName(aNewName);
  }

  return aNodeRoot;
}
