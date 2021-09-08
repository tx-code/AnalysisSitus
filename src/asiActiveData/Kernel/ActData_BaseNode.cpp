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
#include <ActData_BaseNode.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_GraphFrozenException.h>
#include <ActData_ParameterFactory.h>
#include <ActData_RealEvaluatorFunc.h>
#include <ActData_Utils.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_Tool.hxx>

//-----------------------------------------------------------------------------

#define ERR_EXGRAPH_FROZEN "Execution Graph cannot be affected as it is frozen"

//-----------------------------------------------------------------------------

//! Constructor for Parameters Iterator accepting the Node to iterate.
//! \param theNode [in] Node to iterate over.
ActData_BaseParamIterator::ActData_BaseParamIterator(const Handle(ActAPI_INode)& theNode)
{
  this->Init(theNode);
}

//! Initializes Iterator with the Node to iterate over.
//! \param theNode [in] Node to iterate over.
void ActData_BaseParamIterator::Init(const Handle(ActAPI_INode)& theNode)
{
  m_node = Handle(ActData_BaseNode)::DownCast(theNode);
  m_it = m_node->m_paramScope.User->cbegin();
}

//! \return true if there is something to iterate, false -- otherwise.
Standard_Boolean ActData_BaseParamIterator::More() const
{
  return m_it != m_node->m_paramScope.User->cend();
}

//! Puts iterator to the next position.
void ActData_BaseParamIterator::Next()
{
  ++m_it;
}

//! \return internal ID of the current Parameter.
Standard_Integer ActData_BaseParamIterator::Key() const
{
  return m_it->first;
}

//! \return current Parameter.
const Handle(ActAPI_IUserParameter)& ActData_BaseParamIterator::Value() const
{
  return m_it->second;
}

//------------------------------------------------------------------------------

//! Creates tree iterator for the TREE NODE hierarchy.
//! \param theNode     [in] root Node to start iteration from.
//! \param isAllLevels [in] indicates whether to iterate all levels in depth.
ActData_BaseChildIterator::ActData_BaseChildIterator(const Handle(ActAPI_INode)& theNode,
                                                     const Standard_Boolean      isAllLevels)
{
  this->Init(theNode, isAllLevels);
}

//! Initializes tree iterator.
//! \param theNode     [in] root Node to start iteration from.
//! \param isAllLevels [in] indicates whether to iterate all levels in depth.
void ActData_BaseChildIterator::Init(const Handle(ActAPI_INode)& theNode,
                                     const Standard_Boolean      isAllLevels)
{
  m_node = Handle(ActData_BaseNode)::DownCast(theNode);

  // Access Tree Node attribute
  Handle(TDataStd_TreeNode) aTreeNode = m_node->m_paramScope.Meta->GetCAFTreeNode();

  if ( !aTreeNode.IsNull() )
    m_it.Initialize(aTreeNode, isAllLevels);
}

//! \return true if there is something to iterate, false -- otherwise.
Standard_Boolean ActData_BaseChildIterator::More() const
{
  return m_it.More();
}

//! Puts iterator to the next position.
void ActData_BaseChildIterator::Next()
{
  m_it.Next();
}

//! \return current Node.
Handle(ActAPI_INode) ActData_BaseChildIterator::Value() const
{
  return ActData_NodeFactory::NodeSettle( this->ValueLabel() );
}


//! \return root label of the current Node.
TDF_Label ActData_BaseChildIterator::ValueLabel() const
{
  Handle(TDataStd_TreeNode) aChildTreeNode = m_it.Value();

  // Use the convention that Tree Node Parameter is nested into Internal
  // scope of the Node:
  //  * Label  <-> owning TDF Label for Tree Node attribute (META)
  //  * Father <-> Label for Node
  TDF_Label aNodeLab = aChildTreeNode->Label().Father();

  return aNodeLab;
}

//-----------------------------------------------------------------------------
// Construction
//-----------------------------------------------------------------------------

ActData_BaseNode::ActData_BaseNode() : ActAPI_INode()
{
  // Initialize META Parameter
  m_paramScope.Meta = ActData_MetaParameter::Instance();

  // Initialize a collection for user Parameters
  m_paramScope.User = new ActAPI_HIndexedParameterMap;
}

//-----------------------------------------------------------------------------
// Data Cursor behavior
//-----------------------------------------------------------------------------

//! \return true if this Data Node is attached to OCAF, false -- otherwise.
Standard_Boolean ActData_BaseNode::IsAttached()
{
  Handle(ActAPI_HNodalParameterList) aParamList = this->accessAllParameters();
  ActAPI_NodalParameterList::Iterator anIt( *aParamList.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    Handle(ActAPI_IUserParameter) aNextParam = anIt.Value().Parameter;
    if ( !aNextParam->IsAttached() )
      return Standard_False;
  }

  return Standard_True;
}

//! \return true if this Data Node is detached from OCAF, false -- otherwise.
Standard_Boolean ActData_BaseNode::IsDetached()
{
  return !this->IsAttached();
}

//! \return true if this Data Node is well-formed, false -- otherwise.
Standard_Boolean ActData_BaseNode::IsWellFormed()
{
  Handle(ActAPI_HNodalParameterList) aParamList = this->accessAllParameters();
  ActAPI_NodalParameterList::Iterator anIt( *aParamList.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    Handle(ActAPI_IUserParameter) aNextParam = anIt.Value().Parameter;
    if ( !aNextParam->IsWellFormed() )
      return Standard_False;
  }

  return Standard_True;
}

//! \return true if the stored data is attached and valid, false -- otherwise.
Standard_Boolean ActData_BaseNode::IsValidData()
{
  if ( !this->IsWellFormed() )
    return Standard_False;

  Handle(ActAPI_HNodalParameterList) aParamList = this->accessAllParameters();
  ActAPI_NodalParameterList::Iterator anIt( *aParamList.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    Handle(ActAPI_IUserParameter) aNextParam = anIt.Value().Parameter;
    if ( !aNextParam->IsValidData() )
      return Standard_False;
  }

  return Standard_True;
}

//! Checks validity of the Node together with its children (via TREE NODE).
//! \param forAllLevels [in] indicates whether all level of hierarchy are
//!                          to be checked.
//! \return true in case of validity, false -- otherwise.
Standard_Boolean ActData_BaseNode::IsValidDataWithChildren(const Standard_Boolean forAllLevels)
{
  if ( !this->IsValidData() )
    return Standard_False;

  Handle(ActAPI_IChildIterator) cit = this->GetChildIterator(forAllLevels);
  for ( ; cit->More(); cit->Next() )
  {
    Handle(ActAPI_INode) aChild = cit->Value();
    if ( !aChild->IsValidData() )
      return Standard_False;
  }

  return Standard_True;
}

//! Checks whether data is in PENDING state.
//! \return true/false.
Standard_Boolean ActData_BaseNode::IsPendingData()
{
  if ( !this->IsWellFormed() )
    return Standard_False;

  Handle(ActAPI_HNodalParameterList) aParamList = this->accessAllParameters();
  ActAPI_NodalParameterList::Iterator anIt( *aParamList.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    Handle(ActAPI_IUserParameter) aNextParam = anIt.Value().Parameter;
    if ( aNextParam->IsPendingData() )
      return Standard_True;
  }

  return Standard_False;
}

//! \return ID of the Data Node.
ActAPI_DataObjectId ActData_BaseNode::GetId() const
{
  return ActData_Utils::GetEntry(m_label);
}

//-----------------------------------------------------------------------------
// Accessors to Parameters
//-----------------------------------------------------------------------------

//! \return Parameter Iterator.
Handle(ActAPI_IParamIterator) ActData_BaseNode::GetParamIterator() const
{
  return new ActData_BaseParamIterator(this);
}

//! \return collection of Parameters.
Handle(ActAPI_HIndexedParameterMap) ActData_BaseNode::Parameters() const
{
  return m_paramScope.User;
}

//! Allows to access a single user Parameter by its internal ID.
//! \param theId [in] relative ID of the user Parameter to access.
//! \return instance of the user Parameter.
Handle(ActAPI_IUserParameter)
  ActData_BaseNode::Parameter(const Standard_Integer theId) const
{
  auto pit = m_paramScope.User->find(theId);
  //
  if ( pit == m_paramScope.User->cend() )
    return nullptr;

  return pit->second;
}

//! Initializes user Parameter.
//! \param theId         [in] internal ID of the user Parameter.
//! \param theName       [in] Unicode name.
//! \param theSemanticId [in] semantic ID.
//! \param theUFlags     [in] user flags.
//! \param isSilent      [in] silent / logged modification.
void ActData_BaseNode::InitParameter(const Standard_Integer            theId,
                                     const TCollection_ExtendedString& theName,
                                     const TCollection_AsciiString&    theSemanticId,
                                     const Standard_Integer            theUFlags,
                                     const Standard_Boolean            isSilent)
{
  const Handle(ActAPI_IUserParameter)& P = this->Parameter(theId);

  // Set Parameter name
  P->SetName(theName, isSilent ? MT_Silent : MT_Touched);

  // Set Parameter semantic key (the actual meaning is defined by client)
  if ( !theSemanticId.IsEmpty() )
  {
    P->SetSemanticId(theSemanticId, isSilent ? MT_Silent : MT_Touched);
  }

  // Set accessibility flag
  P->SetUserFlags(theUFlags, isSilent ? MT_Silent : MT_Touched);
}

//-----------------------------------------------------------------------------
// Accessors to immanent persistent properties
//-----------------------------------------------------------------------------

//! \return type name of the Node.
TCollection_AsciiString ActData_BaseNode::GetTypeName()
{
  return m_paramScope.Meta->GetTypeName();
}

//! Accessor for the store application-specific flags.
//! \return requested flags.
Standard_Integer ActData_BaseNode::GetUserFlags()
{
  return m_paramScope.Meta->GetUserFlags();
}

//! Checks if this Node has the passed user flags enabled.
//! \param theUFlags [in] flags to check.
//! \return true/false.
Standard_Boolean ActData_BaseNode::HasUserFlags(const Standard_Integer theUFlags)
{
  return (m_paramScope.Meta->GetUserFlags() & theUFlags) > 0;
}

//! Sets application-specific flags for the Data Node.
//! \param theUFlags [in] application-specific flags to set.
void ActData_BaseNode::SetUserFlags(const Standard_Integer theUFlags)
{
  m_paramScope.Meta->SetUserFlags(theUFlags);
}

//! Adds application-specific flags for the Data Node.
//! \param theUFlags [in] application-specific flags to add.
void ActData_BaseNode::AddUserFlags(const Standard_Integer theUFlags)
{
  m_paramScope.Meta->SetUserFlags(this->GetUserFlags() | theUFlags);
}

//! Removes application-specific user flags for the Data Node.
//! \param theUFlags [in] application-specific flags to remove.
void ActData_BaseNode::RemoveUserFlags(const Standard_Integer theUFlags)
{
  m_paramScope.Meta->SetUserFlags( this->GetUserFlags() & (~theUFlags) );
}

//-----------------------------------------------------------------------------
// Accessors parent/child Data Nodes
//-----------------------------------------------------------------------------

//! Creates Child Iterator for traversing the hierarchy of Node.
//! \param isAllLevels [in] indicates whether all levels have to be traversed.
//! \return Child Iterator.
Handle(ActAPI_IChildIterator)
  ActData_BaseNode::GetChildIterator(const Standard_Boolean isAllLevels) const
{
  return new ActData_BaseChildIterator(this, isAllLevels);
}

//! Provides "direct" access to the child Node by its one-based index.
//! This method is actually based on iterator, so do not expect too much
//! from its performance.
//! \param oneBased_idx [in] 1-based index of the child Node to access.
//! \return child Node.
Handle(ActAPI_INode)
  ActData_BaseNode::GetChildNode(const Standard_Integer oneBased_idx) const
{
  Handle(ActAPI_IChildIterator) cit = this->GetChildIterator();
  //
  int                  current_idx = 1;
  Handle(ActAPI_INode) current_n;
  //
  while ( current_idx < oneBased_idx )
  {
    ++current_idx;
    cit->Next();
  }
  current_n = cit->Value();

  return current_n;
}

//! Collects all children of this Node.
//! \param[out] theChildren children.
//! \return number of children.
int ActData_BaseNode::GetChildren(Handle(ActAPI_HNodeList)& theChildren) const
{
  int res = 0;
  theChildren = new ActAPI_HNodeList;

  for ( Handle(ActAPI_IChildIterator) cit = this->GetChildIterator();
        cit->More(); cit->Next() )
  {
    theChildren->Append( cit->Value() );
    ++res;
  }

  return res;
}

//! Adds a child Node to this one.
//! \param theNode [in] child Node to add.
void ActData_BaseNode::AddChildNode(const Handle(ActAPI_INode)& theNode)
{
  // Convert to base Node
  Handle(ActData_BaseNode) BN = Handle(ActData_BaseNode)::DownCast(theNode);

  // Access meta for the child
  Handle(ActData_MetaParameter) anOtherTreeNode = BN->m_paramScope.Meta;

  // Add child to this one using meta interface
  m_paramScope.Meta->AppendChild(anOtherTreeNode);

  //-------------------------
  // Put record into LogBook
  //-------------------------

  // Prepare LogBook Data Cursor
  TDF_Label aLogBookSection =
    m_label.Root().FindChild(ActData_BaseModel::StructureTag_LogBook);

  // IMPACT record is placed today
  ActData_LogBook(aLogBookSection).Impact(m_label);
}

//! Attempts to remove the passed Data Node as child one.
//! \param theNode [in] child Node to remove.
//! \return true if a Node with the given ID was find and removed from the
//!         children collection, false -- otherwise.
Standard_Boolean ActData_BaseNode::RemoveChildNode(const Handle(ActAPI_INode)& theNode)
{
  // Convert to base Node
  Handle(ActData_BaseNode) BN = Handle(ActData_BaseNode)::DownCast(theNode);

  // Access meta for the child (if it is really a child...)
  Handle(ActData_MetaParameter) anOtherTreeNode = BN->m_paramScope.Meta;

  // Remove child using meta interface
  const Standard_Boolean isOk = m_paramScope.Meta->RemoveChild(anOtherTreeNode);

  //-------------------------
  // Put record into LogBook
  //-------------------------

  if ( isOk )
  {
    // Prepare LogBook Data Cursor
    TDF_Label aLogBookSection =
      m_label.Root().FindChild(ActData_BaseModel::StructureTag_LogBook);

    // IMPACT record is placed today
    ActData_LogBook(aLogBookSection).Impact(m_label);
  }

  return isOk;
}

//! Returns parent Data Node for this one. If a parent does not exists, a NULL
//! handle is returned.
//! \return parent Data Node.
Handle(ActAPI_INode) ActData_BaseNode::GetParentNode()
{
  // Access the internal Tree Node attribute
  Handle(TDataStd_TreeNode) aNodeAttr = m_paramScope.Meta->GetCAFTreeNode();
  //
  if ( aNodeAttr.IsNull() || !aNodeAttr->HasFather() )
    return NULL;

  // Access parent Tree Node attribute
  Handle(TDataStd_TreeNode) aParentNodeAttr = aNodeAttr->Father();
  //
  if ( aParentNodeAttr->Label().Father().IsNull() )
    return NULL;

  // Tree Node attribute lives in the META section which is one level down
  // with respect to the root Label of the Node. Therefore, we have to move
  // one level up to access the root
  return ActData_NodeFactory::NodeSettle( aParentNodeAttr->Label().Father() );
}

//-----------------------------------------------------------------------------
// OCAF internals
//-----------------------------------------------------------------------------

//! Accessor to the raw Label the Node Cursor is settled to.
//! \return root Label for the Data Node.
TDF_Label ActData_BaseNode::RootLabel() const
{
  return m_label;
}

//-----------------------------------------------------------------------------
// Tree Function mechanism support
//-----------------------------------------------------------------------------

//! Connects Tree Function to the Tree Function Parameter with the given ID.
//! \param theId      [in] ID of the Tree Function Parameter.
//! \param theGUID    [in] GUID of the Tree Function driver.
//! \param theArgsIN  [in] input Parameters.
//! \param theArgsOUT [in] output Parameters.
void ActData_BaseNode::ConnectTreeFunction(const Standard_Integer               theId,
                                           const Standard_GUID&                 theGUID,
                                           const Handle(ActAPI_HParameterList)& theArgsIN,
                                           const Handle(ActAPI_HParameterList)& theArgsOUT)
{
  if ( theArgsIN.IsNull() || theArgsIN->IsEmpty() ) // Only OUTPUT can be empty (for implicitly parameterized leafs)
    return;

  // It is necessary to disconnect reference before connecting a new one in
  // order not to loose back-reference (preventing back-reference leak)
  if ( this->HasConnectedTreeFunction(theId) )
    this->DisconnectTreeFunction(theId);

  /* ==================================
   *  Set reference to Function Driver
   * ================================== */

  Handle(ActData_TreeFunctionParameter)
    aFuncParam = this->accessFuncParameter(theId, Standard_False);
  aFuncParam->SetDriverGUID(theGUID);

  /* ==============================
   *  Bind Tree Function arguments
   * ============================== */

  // Add argument TDF Labels to OCAF data
  for ( Standard_Integer i = 1; i <= theArgsIN->Length(); i++ )
  {
    Handle(ActAPI_IUserParameter) aNextArg = theArgsIN->Value(i);
    aFuncParam->AddArgument(aNextArg);

    // Register just added Function as an INPUT Reader
    Handle(ActData_BaseNode)
      aNextNode = Handle(ActData_BaseNode)::DownCast( aNextArg->GetNode() );

    if ( ActAPI_IDataCursor::IsEqual(this, aNextNode) )
      continue; // Do not refer to itself

    aNextNode->connectReader(aFuncParam);
  }

  // Add result TDF Labels to OCAF data
  if ( !theArgsOUT.IsNull() )
  {
    for ( Standard_Integer i = 1; i <= theArgsOUT->Length(); i++ )
    {
      Handle(ActAPI_IUserParameter) aNextArg = theArgsOUT->Value(i);
      aFuncParam->AddResult(aNextArg);

      // Register just added Function as an OUTPUT Writer
      Handle(ActData_BaseNode)
        aNextNode = Handle(ActData_BaseNode)::DownCast( aNextArg->GetNode() );

      if ( ActAPI_IDataCursor::IsEqual(this, aNextNode) )
        continue; // Do not refer to itself

      aNextNode->connectWriter(aFuncParam);
    }
  }
}

//! Checks whether this Node has a Tree Function connected to the Parameter
//! with the given ID.
//! \param theId [in] ID of the Parameter to check.
//! \return true/false.
Standard_Boolean
  ActData_BaseNode::HasConnectedTreeFunction(const Standard_Integer theId)
{
  return ActData_ParameterFactory::AsTreeFunction( this->Parameter(theId) )->IsConnected();
}

//! Disconnects Tree Function from the given Tree Function Parameter.
//! \param theId [in] ID of the target Tree Function Parameter.
void ActData_BaseNode::DisconnectTreeFunction(const Standard_Integer theId)
{
  // Notice that last argument is FALSE: unbinding is "soft"
  this->disconnectTreeFunction(theId, Standard_False, Standard_False);
}

//-----------------------------------------------------------------------------
// Evaluation mechanism
//-----------------------------------------------------------------------------

//! Connects expression evaluation Tree Function to the integer or real
//! Parameter with the given ID.
//! \param theId     [in] ID of the evaluation Tree Function Parameter.
//! \param theVarsIN [in] input arguments.
void ActData_BaseNode::ConnectEvaluator(const Standard_Integer               theId,
                                        const Handle(ActAPI_HParameterList)& theVarsIN)
{
  // It is necessary to Disconnect reference before connecting new one in
  // order not to loose back-reference (preventing back-reference leak)
  if ( this->HasConnectedEvaluator(theId) )
    this->DisconnectEvaluator(theId);

  /* =========================
   *  Access target Parameter
   * ========================= */

  Handle(ActAPI_IUserParameter) aParam = this->Parameter(theId);
  //
  if ( aParam->GetParamType() != Parameter_Real &&
       aParam->GetParamType() != Parameter_Int )
    Standard_ProgramError::Raise("Not implemented for others than REAL & INT");

  /* ==================================
   *  Set reference to Function Driver
   * ================================== */

  if ( !this->IsEvaluable(theId) )
    Standard_ProgramError::Raise("Cannot connect non-expressible Parameter");

  Standard_Integer aEvalFuncTag = m_paramScope.ExpressibleParams.Find(theId);
  Handle(ActData_TreeFunctionParameter)
    aFuncParam = this->accessFuncParameter(aEvalFuncTag, Standard_True);

  // Notice that Real Evaluator is also used for integer values
  aFuncParam->SetDriverGUID( ActData_RealEvaluatorFunc::GUID() );

  /* ==============================
   *  Bind Tree Function arguments
   * ============================== */

  // Add target Parameter as the first INPUT argument
  aFuncParam->AddArgument(aParam);

  // Add variable Parameters as the rest INPUT arguments
  if ( !theVarsIN.IsNull() )
  {
    for ( Standard_Integer i = 1; i <= theVarsIN->Length(); i++ )
    {
      Handle(ActAPI_IUserParameter) aNextArg = theVarsIN->Value(i);
      aFuncParam->AddArgument(aNextArg);

      // Register just added Function as an INPUT reader
      Handle(ActData_BaseNode)
        aNextNode = Handle(ActData_BaseNode)::DownCast( aNextArg->GetNode() );

      if ( ActAPI_IDataCursor::IsEqual(this, aNextNode) )
        continue; // Do not refer to itself

      aNextNode->connectReader(aFuncParam);
    }
  }

  // Add target Parameter as a sole OUTPUT argument
  aFuncParam->AddResult(aParam);
}

//! Checks whether evaluation Tree Function is connected to the integer or
//! real Parameter with the given ID.
//! \param theId [in] ID of the scalar Parameter to check the connected
//!                   evaluator for.
//! \return true/false.
Standard_Boolean
  ActData_BaseNode::HasConnectedEvaluator(const Standard_Integer theId)
{
  if ( !this->IsEvaluable(theId) )
    return Standard_False;

  // Get evaluator Tree Function Parameter
  Handle(ActData_TreeFunctionParameter)
    aFuncParam = Handle(ActData_TreeFunctionParameter)::DownCast( this->Evaluator(theId) );

  if ( aFuncParam.IsNull() )
    return Standard_False;

  return aFuncParam->IsConnected();
}

//! Returns evaluator associated with the given scalar (integer or real)
//! Parameter.
//! \param theId [in] ID of the scalar Parameter to access the evaluator for.
//! \return evaluation Tree Function.
Handle(ActAPI_IUserParameter) ActData_BaseNode::Evaluator(const Standard_Integer theId)
{
  if ( !this->IsEvaluable(theId) )
    return NULL;

  // Get evaluator Tree Function Parameter
  Standard_Integer aEvalFuncTag = m_paramScope.ExpressibleParams.Find(theId);
  return this->accessFuncParameter(aEvalFuncTag, Standard_True);
}

//! Checks whether the scalar Parameter with the given ID is evaluable or
//! not. "Evaluable" means that there is a hidden evaluation Tree Function
//! which is used to calculate the associated scalar value automatically.
//! \param theId [in] ID of the scalar (integer or real) Parameter to check.
//! \return true / false.
Standard_Boolean
  ActData_BaseNode::IsEvaluable(const Standard_Integer theId)
{
  return m_paramScope.ExpressibleParams.IsBound(theId);
}

//! Disconnects evaluation Tree Function for the given Parameter.
//! \param theId [in] ID of the scalar (integer or real) Parameter.
void ActData_BaseNode::DisconnectEvaluator(const Standard_Integer theId)
{
  if ( !m_paramScope.ExpressibleParams.IsBound(theId) )
    Standard_ProgramError::Raise("Cannot connect non-expressible Parameter");

  // Get evaluator Tree Function Parameter
  const Standard_Integer aEvalFuncTag = m_paramScope.ExpressibleParams.Find(theId);

  // Unbinding is "soft"
  this->disconnectTreeFunction(aEvalFuncTag, Standard_True, Standard_False);
}

//-----------------------------------------------------------------------------
// Plain references
//-----------------------------------------------------------------------------

//! Sets reference and back-reference for the given target.
//! \param theId     [in] ID of the reference holder Parameter.
//! \param theTarget [in] data object to refer to (Parameter or Node).
void ActData_BaseNode::ConnectReference(const Standard_Integer            theId,
                                        const Handle(ActAPI_IDataCursor)& theTarget)
{
  // It is necessary to Disconnect reference before connecting new one in
  // order not to loose back-reference (preventing back-reference leak)
  if ( this->HasConnectedReference(theId) )
    this->DisconnectReference(theId);

  /* ======================================================
   *  Access Reference Parameter and bind DIRECT reference
   * ====================================================== */

  // Access Reference Parameter
  Handle(ActData_ReferenceParameter) aRefParam = this->accessRefParameter(theId);

  // Bind reference
  aRefParam->SetTarget(theTarget);

  /* =============================================
   *  Bind BACK reference for the referenced Node
   * ============================================= */

  Handle(ActData_BaseNode) aReferredNode;
  if ( theTarget->IsKind( STANDARD_TYPE(ActAPI_INode) ) )
  {
    aReferredNode = Handle(ActData_BaseNode)::DownCast(theTarget);
  }
  else if ( theTarget->IsKind( STANDARD_TYPE(ActAPI_IUserParameter) ) )
  {
    Handle(ActAPI_IUserParameter) aReferredParam = Handle(ActAPI_IUserParameter)::DownCast(theTarget);
    aReferredNode = Handle(ActData_BaseNode)::DownCast( aReferredParam->GetNode() );
  }

  if ( !aReferredNode.IsNull() && !ActAPI_IDataCursor::IsEqual(this, aReferredNode) )
    aReferredNode->connectReferrer(aRefParam);
}

//! Checks whether the Parameter with the given ID is a reference holder with
//! some reference stored.
//! \param theId [in] ID of the reference holder to check.
//! \return true / false.
Standard_Boolean ActData_BaseNode::HasConnectedReference(const Standard_Integer theId)
{
  Handle(ActData_ReferenceParameter) aRefParam = this->accessRefParameter(theId);
  //
  return !aRefParam->GetTarget().IsNull();
}

//! Disconnects all references from the given reference holder.
//! \param theId [in] ID of the reference holder.
void ActData_BaseNode::DisconnectReference(const Standard_Integer theId)
{
  // Access Reference Parameter
  Handle(ActData_ReferenceParameter) aRefParam = this->accessRefParameter(theId);

  // Access target Data Cursor to remove a back-reference
  Handle(ActAPI_IDataCursor) aTargetDC = aRefParam->GetTarget();

  if ( aTargetDC.IsNull() )
    return; // Nothing to disconnect

  Handle(ActData_BaseNode) aReferredNode;
  if ( aTargetDC->IsKind( STANDARD_TYPE(ActAPI_INode) ) )
  {
    aReferredNode = Handle(ActData_BaseNode)::DownCast(aTargetDC);
  }
  else if ( aTargetDC->IsKind( STANDARD_TYPE(ActAPI_IUserParameter) ) )
  {
    Handle(ActAPI_IUserParameter) aReferredParam = Handle(ActAPI_IUserParameter)::DownCast(aTargetDC);
    aReferredNode = Handle(ActData_BaseNode)::DownCast( aReferredParam->GetNode() );
  }

  // Clean up reference
  aRefParam->RemoveTarget();

  // Remove back-reference
  if ( !aReferredNode.IsNull() )
    aReferredNode->disconnectReferrer(aRefParam);
}

//-----------------------------------------------------------------------------
// Multiple references
//-----------------------------------------------------------------------------

//! Adds the passed Data Cursor to the Reference List Parameter addressed
//! by the given ID. The back-reference is established as well.
//! \param theId         [in] PID for the target Reference List Parameter.
//! \param theTarget     [in] Data Cursor to add as a target.
//! \param theAfterIndex [in] index to add the new target after. Pass zero
//!                           for prepending (default) and number of existing
//!                           references for appending.
void ActData_BaseNode::ConnectReferenceToList(const Standard_Integer            theId,
                                              const Handle(ActAPI_IDataCursor)& theTarget,
                                              const Standard_Integer            theAfterIndex)
{
  /* ===========================================================
   *  Access Reference List Parameter and bind DIRECT reference
   * =========================================================== */

  // Access Reference List Parameter
  Handle(ActData_ReferenceListParameter) aRefParam = this->accessRefListParameter(theId);

  // Bind reference
  if ( theAfterIndex == aRefParam->NbTargets() )
    aRefParam->AddTarget(theTarget);
  else if ( theAfterIndex == 0 )
    aRefParam->PrependTarget(theTarget);
  else
    aRefParam->InsertTargetAfter(theAfterIndex, theTarget);

  /* =============================================
   *  Bind BACK reference for the referenced Node
   * ============================================= */

  Handle(ActData_BaseNode) aReferredNode;
  if ( theTarget->IsKind( STANDARD_TYPE(ActAPI_INode) ) )
    aReferredNode = Handle(ActData_BaseNode)::DownCast(theTarget);
  else if ( theTarget->IsKind( STANDARD_TYPE(ActAPI_IUserParameter) ) )
  {
    Handle(ActAPI_IUserParameter) aReferredParam = Handle(ActAPI_IUserParameter)::DownCast(theTarget);
    aReferredNode = Handle(ActData_BaseNode)::DownCast( aReferredParam->GetNode() );
  }

  if ( !aReferredNode.IsNull() && !ActAPI_IDataCursor::IsEqual(this, aReferredNode) )
    aReferredNode->connectReferrer(aRefParam);
}

//! Checks whether this Node contains non-empty Reference List Parameter
//! addressed by the given ID.
//! \param theId [in] PID for the Reference List Parameter to check.
//! \return true in case if non-dummy Reference List Parameter exists,
//!         false -- otherwise.
Standard_Boolean ActData_BaseNode::HasConnectedReferenceList(const Standard_Integer theId)
{
  Handle(ActData_ReferenceListParameter) aRefParam = this->accessRefListParameter(theId);
  //
  return !aRefParam->AccessReferenceList().IsNull() &&
         !aRefParam->AccessReferenceList()->IsEmpty();
}

//! Disconnects all references in the Reference List Parameter addressed by
//! the given ID.
//! \param theId [in] PID for the Reference List Parameter to release.
void ActData_BaseNode::DisconnectReferenceList(const Standard_Integer theId)
{
  // Access Reference List Parameter
  Handle(ActData_ReferenceListParameter) aRefParam = this->accessRefListParameter(theId);

  // Iterate over entire collection of references, disconnecting them one-by-one
  Standard_Integer aNbRefs = aRefParam->NbTargets();
  for ( Standard_Integer i = aNbRefs; i > 0; --i )
    DisconnectReferenceFromList(theId, i);
}

//! Disconnects reference with the given index from the Reference List
//! Parameter.
//! \param theId [in] PID for the Reference List Parameter.
//! \param theIndex [in] index of the reference item to disconnect.
Standard_Boolean
  ActData_BaseNode::DisconnectReferenceFromList(const Standard_Integer theId,
                                                const Standard_Integer theIndex)
{
  // Access Reference List Parameter
  Handle(ActData_ReferenceListParameter) aRefParam = this->accessRefListParameter(theId);

  // Access target
  Handle(ActAPI_IDataCursor) aTargetDC = aRefParam->GetTarget(theIndex);
  if ( aTargetDC.IsNull() )
    return Standard_False;

  // Resolve target's owning Node
  Handle(ActData_BaseNode) aReferredNode;
  if ( aTargetDC->IsKind( STANDARD_TYPE(ActAPI_INode) ) )
    aReferredNode = Handle(ActData_BaseNode)::DownCast(aTargetDC);
  else if ( aTargetDC->IsKind( STANDARD_TYPE(ActAPI_IUserParameter) ) )
  {
    Handle(ActAPI_IUserParameter) aReferredParam = Handle(ActAPI_IUserParameter)::DownCast(aTargetDC);
    aReferredNode = Handle(ActData_BaseNode)::DownCast( aReferredParam->GetNode() );
  }

  // Remove reference
  if ( !aRefParam->RemoveTarget(aTargetDC) )
    return Standard_False;

  // Remove (or not) back reference
  if ( !aReferredNode.IsNull() )
    this->disconnectReferrerSmart(aRefParam, aReferredNode);

  return Standard_True;
}

//-----------------------------------------------------------------------------
// Back-references (observers)
//-----------------------------------------------------------------------------

//! \return external input reader Parameters.
Handle(ActAPI_HParameterList) ActData_BaseNode::GetInputReaders()
{
  return this->accessObservers(Observer_InputReaders);
}

//! \return external output writer Parameters.
Handle(ActAPI_HParameterList) ActData_BaseNode::GetOutputWriters()
{
  return this->accessObservers(Observer_OutputWriters);
}

//! \return external referrer Parameters.
Handle(ActAPI_HParameterList) ActData_BaseNode::GetReferrers()
{
  return this->accessObservers(Observer_Referrers);
}

//-----------------------------------------------------------------------------
// Construction internals
//-----------------------------------------------------------------------------

//! This is the very basic function for Active Data Nodes. It makes the Node
//! Cursor aware of the available user Parameters and their optional evaluation
//! Tree Functions which are normally hidden from the user.
//! \param theId         [in] ID of the user Parameter to register.
//! \param theParam      [in] Data Cursor for the user Parameter.
//! \param isExpressible [in] indicates whether a hidden Tree Function
//!                           Parameter should be created in the META section.
void ActData_BaseNode::registerParameter(const Standard_Integer               theId,
                                         const Handle(ActAPI_IUserParameter)& theParam,
                                         const Standard_Boolean               isExpressible)
{
  // Store detached Data Cursor in the internal map
  (*m_paramScope.User)[theId] = theParam;

  if ( isExpressible )
  {
    // Get the next free tag for the new TDF Label the internal Parameter
    // will be settled down to
    Standard_Integer aLastOccupiedTag = 0;
    for ( auto pit = m_paramScope.Meta->Evaluators()->cbegin(); pit != m_paramScope.Meta->Evaluators()->cend(); ++pit )
      aLastOccupiedTag = pit->first;

    // Construct new DETACHED Tree Function Parameter serving evaluation
    // of the user Parameter. This Tree Function Parameter is hidden for
    // Data Framework clients
    Standard_Integer aEvalFuncTag = aLastOccupiedTag + 1;
    ( *m_paramScope.Meta->Evaluators() )[aEvalFuncTag] = ActData_TreeFunctionParameter::Instance();

    // Add the corresponding relation to the internal transient map
    m_paramScope.ExpressibleParams.Bind(theId, aEvalFuncTag);
  }
}

//! Node removal routine. This method deletes the Node with all existing
//! references gracefully. It should be noted that as a result of this
//! method the Execution Graph can happen to be modified.
//! \param canAffectExGraph [in] indicates whether this method is allowed
//!                              to modify Execution Graph or not. If not,
//!                              an exception is thrown as we do not want to
//!                              allow inconsistent data in any case.
void ActData_BaseNode::remove(const Standard_Boolean canAffectExGraph)
{
  this->beforeRemove();

  /* ==========================================================
   *  Remove observers of all three types: INPUT Readers,
   *  OUTPUT Writers and Referrers. The following kind
   *  of connectivity is treated here:
   *
   *  NODE_A --[listens to]--> THIS     NODE_A --x--> THIS
   *  ...                           >>> ...
   *  NODE_X --[listens to]--> THIS     NODE_X --x--> THIS
   *
   *  E.g: Hypothetical "Mesh Properties" Node is referencing
   *       "Mesh" Node. "Mesh" Node is removed --> "Mesh
   *       Properties" must stop listen to the Mesh Node
   * ========================================================== */

  // Iterate over the INPUT readers disconnecting them one by one
  Handle(ActAPI_HParameterList) anInputReaders = this->GetInputReaders();
  ActAPI_ParameterList::Iterator aParamIt( *anInputReaders.operator->() );
  for ( ; aParamIt.More(); aParamIt.Next() )
  {
    ActData_GraphFrozenException_Raise_if( !canAffectExGraph, ERR_EXGRAPH_FROZEN );
    Handle(ActData_TreeFunctionParameter)::DownCast( aParamIt.Value() )->DisconnectSoft();
  }

  // Iterate over the OUTPUT writers disconnecting them one by one
  Handle(ActAPI_HParameterList) anOutputWriters = this->GetOutputWriters();
  aParamIt.Init( *anOutputWriters.operator->() );
  for ( ; aParamIt.More(); aParamIt.Next() )
  {
    ActData_GraphFrozenException_Raise_if(!canAffectExGraph, ERR_EXGRAPH_FROZEN);
    Handle(ActData_TreeFunctionParameter)::DownCast( aParamIt.Value() )->DisconnectSoft();
  }

  // Iterate over the Referrers cleaning up them one by one
  Handle(ActAPI_HParameterList) aReferrers = this->GetReferrers();
  aParamIt.Init( *aReferrers.operator->() );
  for ( ; aParamIt.More(); aParamIt.Next() )
  {
    const Handle(ActAPI_IUserParameter)& anObserver = aParamIt.Value();

    if ( anObserver->GetParamType() == Parameter_Reference )
    {
      Handle(ActData_ReferenceParameter) aRefObserver = ActParamTool::AsReference(anObserver);
      Handle(ActData_BaseNode)
        aUserNode = Handle(ActData_BaseNode)::DownCast( aRefObserver->GetNode() );

      aUserNode->beforeRemoveMyReference(aRefObserver); // Notify about intervention
      aRefObserver->RemoveTarget();
    }
    else if ( anObserver->GetParamType() == Parameter_ReferenceList )
    {
      Handle(ActData_ReferenceListParameter)
        aRefListObserver = ActParamTool::AsReferenceList(anObserver);
      Handle(ActData_BaseNode)
        aUserNode = Handle(ActData_BaseNode)::DownCast( aRefListObserver->GetNode() );

      /* ========================================================
       *  Observer might have been listening to the Node or some
       *  of its Parameters. We cannot know this exactly, so we
       *  try to detach everything
       * ======================================================== */

      // First try to remove the Node itself
      aUserNode->beforeRemoveMyReference(aRefListObserver, this); // Notify about intervention
      aRefListObserver->RemoveTargetOccurrences(this);

      // Then iterate over the entire Parameters trying to remove any
      // back-references to them
      Handle(ActAPI_HNodalParameterList) aParamList = this->accessAllParameters();
      ActAPI_NodalParameterList::Iterator anIt( *aParamList.operator->() );
      for ( ; anIt.More(); anIt.Next() )
      {
        const ActAPI_NodalParameter& aThisParam = anIt.Value();
        aUserNode->beforeRemoveMyReference(aRefListObserver, aThisParam.Parameter); // Notify about intervention
        aRefListObserver->RemoveTargetOccurrences(aThisParam.Parameter);
      }
    }
    else
      Standard_ProgramError::Raise("Unexpected type of referrer");
  }

  /* ===============================================================
   *  Clean up back references to the underlying Tree Functions
   *  which are being also deleted. Such references could exist
   *  in a global LogBook, Function Scope, etc. The following
   *  kind of connectivity is treated here:
   *
   *  THIS --[listens to]--> NODE_A     THIS --x--> NODE_A
   *  ...                           >>> 
   *  THIS --[listens to]--> NODE_X     THIS --x--> NODE_X
   *
   *  E.g: "Mesh" Node is referencing to "Shape" Node. "Mesh" Node
   *       is removed --> back reference must be released from
   *       "Shape" Node
   * =============================================================== */

  Handle(ActAPI_HNodalParameterList) aParamList = this->accessAllParameters();
  ActAPI_NodalParameterList::Iterator anIt( *aParamList.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    ActAPI_NodalParameter aNodalParam = anIt.Value();
    Handle(ActAPI_IUserParameter) aNextParam = aNodalParam.Parameter;

    // Smart disconnecting with cleaning up back references
    if ( aNextParam->IsKind( STANDARD_TYPE(ActData_TreeFunctionParameter) ) )
    {
      ActData_GraphFrozenException_Raise_if(!canAffectExGraph, ERR_EXGRAPH_FROZEN);
      this->disconnectTreeFunction(aNodalParam.RelativeId, aNodalParam.IsInternal, Standard_True);
    }
    else if ( aNextParam->IsKind( STANDARD_TYPE(ActData_ReferenceParameter) ) )
    {
      // -------------------------------------------------------------------
      // Reference Parameter is being removed. Thus we have to clean up all
      // back references established by this Parameter in other Nodes
      // -------------------------------------------------------------------

      Handle(ActData_ReferenceParameter) aRefP = ActData_ParameterFactory::AsReference(aNextParam);
      Handle(ActAPI_IDataCursor) aTargetCsr = aRefP->GetTarget();
      this->disconnectReferrerFor(aRefP, aTargetCsr);
    }
    else if ( aNextParam->IsKind( STANDARD_TYPE(ActData_ReferenceListParameter) ) )
    {
      // -------------------------------------------------------------------
      // Reference List Parameter is being removed. Thus we have to clean
      // up all back references established by this Parameter in other
      // Nodes
      // -------------------------------------------------------------------

      Handle(ActData_ReferenceListParameter) aRefP = ActData_ParameterFactory::AsReferenceList(aNextParam);
      for ( Standard_Integer i = 1; i <= aRefP->NbTargets(); ++i )
      {
        Handle(ActAPI_IDataCursor) aTargetCsr = aRefP->GetTarget(i);
        this->disconnectReferrerFor(aRefP, aTargetCsr);
      }
    }
  }

  /* ==========================
   *  Clean up the Node itself
   * ========================== */

  ActData_Utils::RemoveWithReferences(m_label);
}

//! Callback allowing Data Node to perform some application-specific logic
//! before the actual removal continues.
void ActData_BaseNode::beforeRemove()
{
  // Empty in basic implementation
}

//! If this Data Node references some other one, then a back-reference is
//! stored in the target Node in order to keep track of its observers. If the
//! target Node is then deleted, the observer must be adjusted so that to
//! nullify the corresponding reference. This action is a kind of intervention
//! to user-specific logic, however, it is really necessary for data
//! consistency. Actually, this situation is quite similar to FOREIGN-key
//! protection in Data Bases where you normally cannot delete the referenced
//! object until any external FOREIGN key constraint exists.
//!
//! In Active Data Model you can still use such protection, but we do allow
//! cascading deletion as well. In the latter case we notify the affected Node
//! that its reference is going to be dropped. This is a honest way of
//! semi-automated user-specific data modification, allowing you to prepare
//! your Data Node for such external collision ;)
//!
//! The use case:
//! 1. You have two coherent arrays in your Data Node;
//! 2. One array is a list of references to external Nodes;
//! 3. Second array is array of integer indices corresponding to some data
//!    portions in the referenced external Nodes;
//! 4. The arrays must be kept synchronized, so that when reference is
//!    removed from the list, the corresponding integer index is removed
//!    from the array and vice versa;
//! 5. It is not a problem to maintain one-direction synchronization between
//!    integer array and reference list as we always control any modifications
//!    on our custom Parameters;
//! 6. BUT: reference list can be affected without our explicit intention.
//!    This can happen if we allow cascading deletion and some target Node
//!    is removed;
//! 7. In that case we can use this callback in order to drop the
//!    corresponding record in the integer array as well. Thus we will keep
//!    our arrays synchronized.
void ActData_BaseNode::beforeRemoveMyReference(const Handle(ActData_ReferenceParameter)&)
{
  // Empty in basic implementation
}

//! Idea is the same as for beforeRemoveMyReferrer() for Reference Parameter.
void ActData_BaseNode::beforeRemoveMyReference(const Handle(ActData_ReferenceListParameter)&,
                                               const Handle(ActAPI_IDataCursor)&)
{
  // Empty in basic implementation
}

//-----------------------------------------------------------------------------
// Internal accessors to Cursor's transient structure
//-----------------------------------------------------------------------------

//! \return all Parameters registered in the Node: either user ones or
//!         internal ones. For the internal ones the only candidates to
//!         be pushed into this list are the evaluation Tree Functions from
//!         the META section.
Handle(ActAPI_HNodalParameterList) ActData_BaseNode::accessAllParameters()
{
  Handle(ActAPI_HNodalParameterList) result = new ActAPI_HNodalParameterList();

  // Add USER Parameters
  for ( auto pit = m_paramScope.User->cbegin(); pit != m_paramScope.User->cend(); ++pit )
  {
    result->Append( ActAPI_NodalParameter(pit->second, pit->first, Standard_False) );
  }

  // Add INTERNAL evaluation Tree Function Parameters
  for ( auto pit = m_paramScope.Meta->Evaluators()->cbegin(); pit != m_paramScope.Meta->Evaluators()->cend(); ++pit )
  {
    result->Append( ActAPI_NodalParameter(pit->second, pit->first, Standard_True) );
  }

  return result;
}

//! Gives access to the Tree Function Parameter identified by the given ID.
//! \param theId      [in] ID of the Parameter.
//! \param isInternal [in] indicates whether this Parameter is internal
//!                        or the user's one.
//! \return Parameter's Data Cursor.
Handle(ActData_TreeFunctionParameter)
  ActData_BaseNode::accessFuncParameter(const Standard_Integer theId,
                                        const Standard_Boolean isInternal)
{
  Handle(ActData_TreeFunctionParameter)
    aFuncParam = m_paramScope.SafeCast<Handle(ActData_TreeFunctionParameter)>(theId, isInternal);

  if ( aFuncParam.IsNull() ) // Bad type of expected Tree Function Parameter
    Standard_ProgramError::Raise("Tree Function Parameter expected");

  return aFuncParam;
}

//! Gives access to the Reference Parameter identified by the given ID.
//! \param theId [in] ID of the Parameter.
//! \return Parameter's Data Cursor.
Handle(ActData_ReferenceParameter)
  ActData_BaseNode::accessRefParameter(const Standard_Integer theId)
{
  Handle(ActData_ReferenceParameter)
    aRefParam = m_paramScope.SafeCast<Handle(ActData_ReferenceParameter)>(theId, Standard_False);

  if ( aRefParam.IsNull() ) // Bad type of expected Reference Parameter
    Standard_ProgramError::Raise("Reference Parameter expected");

  return aRefParam;
}

//! Gives access to the Reference List Parameter identified by the given ID.
//! \param theId [in] ID of the Parameter.
//! \return Parameter's Data Cursor.
Handle(ActData_ReferenceListParameter)
  ActData_BaseNode::accessRefListParameter(const Standard_Integer theId)
{
  Handle(ActData_ReferenceListParameter)
    aRefParam = m_paramScope.SafeCast<Handle(ActData_ReferenceListParameter)>(theId, Standard_False);

  if ( aRefParam.IsNull() ) // Bad type of expected Reference Parameter
    Standard_ProgramError::Raise("Reference List Parameter expected");

  return aRefParam;
}

//! Gives access to the Tree Node Parameter identified by the given ID.
//! \param theId [in] ID of the Parameter.
//! \return Parameter's Data Cursor.
Handle(ActData_TreeNodeParameter) ActData_BaseNode::accessTreeNodeParameter(const Standard_Integer theId)
{
  return m_paramScope.SafeCast<Handle(ActData_TreeNodeParameter)>(theId, Standard_False);
}

//! Returns all observers of the given type.
//! \param theObsType [in] observer type.
//! \return observer Parameters.
Handle(ActAPI_HParameterList)
  ActData_BaseNode::accessObservers(const ObserverType theObsType)
{
  Handle(ActAPI_HParameterList) aResult = new ActAPI_HParameterList();
  TDF_LabelList aRefLabs;
  switch ( theObsType )
  {
    case Observer_InputReaders:
      aRefLabs = m_paramScope.Meta->GetInputReaders();
      break;
    case Observer_OutputWriters:
      aRefLabs = m_paramScope.Meta->GetOutputWriters();
      break;
    case Observer_Referrers:
      aRefLabs = m_paramScope.Meta->GetReferrers();
      break;
    default:
      Standard_ProgramError::Raise("Unexpected observer type");
  }

  // Loop over the clients
  TDF_ListIteratorOfLabelList aRefLabsIt(aRefLabs);
  for ( ; aRefLabsIt.More(); aRefLabsIt.Next() )
  {
    TDF_Label& aNextRefLab = aRefLabsIt.Value();

    Standard_Boolean isUndefinedType;
    //
    Handle(ActAPI_IUserParameter)
      aNextRefParam = ActData_ParameterFactory::NewParameterSettle(aNextRefLab, isUndefinedType);

    if ( aNextRefParam.IsNull() )
      continue; // Observer might have been removed or something...

    if ( aNextRefParam->GetParamType() != Parameter_TreeFunction &&
         aNextRefParam->GetParamType() != Parameter_Reference &&
         aNextRefParam->GetParamType() != Parameter_ReferenceList )
      Standard_ProgramError::Raise("Unexpected type of observer Parameter");

    aResult->Append(aNextRefParam);
  }

  // Return client Parameters
  return aResult;
}

//-----------------------------------------------------------------------------
// Data Cursor behavior internals
//-----------------------------------------------------------------------------

//! Attaches the Data Cursor to the OCAF Label.
//! \param theLabel    [in] target OCAF Label.
//! \param isExpanding [in] indicates whether to fill OCAF with data or to use
//!                         the existing OCAF structure (must be compatible
//!                         with the format of Node).
void ActData_BaseNode::attach(const TDF_Label&       theLabel,
                              const Standard_Boolean isExpanding)
{
  // Settle Node to the passed TDF Label
  m_label = theLabel;

  /* ==============================================================
   *  Settle down framework-specific plain & relational Parameters
   *  to sub-Labels of META sub-container
   * ============================================================== */

  // Allow construction of sub-Labels in EXPANDING mode ONLY
  TDF_Label aMetaLab = m_label.FindChild(TagInternal, isExpanding);
  //
  isExpanding ? m_paramScope.Meta->expandOn(aMetaLab) :
                m_paramScope.Meta->settleOn(aMetaLab);

  if ( isExpanding )
  {
    m_paramScope.Meta->SetTypeName ( this->DynamicType()->Name() );
  }

  /* ===========================================================
   *  Settle down domain-specific plain & relational Parameters
   *  to sub-Labels of USER sub-container
   * =========================================================== */

  for ( auto pid = m_paramScope.User->cbegin(); pid != m_paramScope.User->cend(); ++pid )
  {
    const Handle(ActData_UserParameter)& aBaseParam =
      Handle(ActData_UserParameter)::DownCast(pid->second);

    Standard_Integer aNewTag = pid->first;

    // Allow construction of sub-Labels in EXPANDING mode ONLY
    TDF_Label aParamLabRoot = m_label.FindChild(TagUser, isExpanding);
    TDF_Label aParamLab = aParamLabRoot.FindChild(aNewTag, isExpanding);

    isExpanding ? aBaseParam->expandOn(aParamLab) : aBaseParam->settleOn(aParamLab);
  }
}

//! Checks whether a Node instance can be safely settled down onto the
//! passed OCAF Label.
//! \param theLabel [in] Label to check.
//! \return true/false.
Standard_Boolean ActData_BaseNode::canSettleOn(const TDF_Label& theLabel)
{
  /* =====================================================
   *  Check if internal plain & relational Parameters can
   *  be settled
   * ===================================================== */

  TDF_Label aMetaLab = theLabel.FindChild(TagInternal, Standard_False);
  //
  if ( aMetaLab.IsNull() )
    return Standard_False;

  /* ============================================================
   *  Check if domain-specific plain & relational Parameters can
   *  be settled
   * ============================================================ */

  for ( auto pit = m_paramScope.User->cbegin(); pit != m_paramScope.User->cend(); ++pit )
  {
    Standard_Integer aNewTag       = pit->first;
    TDF_Label        aParamLabRoot = theLabel.FindChild(TagUser, Standard_False);

    if ( aParamLabRoot.IsNull() )
      return Standard_False;

    TDF_Label aParamLab = aParamLabRoot.FindChild(aNewTag, Standard_False);

    if ( aParamLab.IsNull() )
      return Standard_False;
  }

  return Standard_True;
}

//! Expands the Node to OCAF tree.
//! \param theLabel [in] target root Label.
void ActData_BaseNode::expandOn(const TDF_Label& theLabel)
{
  this->attach(theLabel, Standard_True);
}

//! Settles down the Node to OCAF tree.
//! \param theLabel [in] target root Label.
void ActData_BaseNode::settleOn(const TDF_Label& theLabel)
{
  this->attach(theLabel, Standard_False);
}

//-----------------------------------------------------------------------------
// Tree Function mechanism support internals
//-----------------------------------------------------------------------------

//! Disconnects Tree Function.
//! \param theId            [in] ID of the Tree Function Parameter.
//! \param isInternal       [in] true for META section.
//! \param doKillCompletely [in] indicates whether Tree Function must
//!                              be completely killed (even from the list
//!                              of available Functions).
void ActData_BaseNode::disconnectTreeFunction(const Standard_Integer theId,
                                              const Standard_Boolean isInternal,
                                              const Standard_Boolean doKillCompletely)
{
  Handle(ActData_TreeFunctionParameter)
    aFuncParam = this->accessFuncParameter(theId, isInternal);

  if ( aFuncParam.IsNull() )
    return;

  this->disconnectTreeFunction(aFuncParam, doKillCompletely);
}

//! Disconnects the passed Tree Function Parameter.
//! \param thePFunc         [in] Tree Function Parameter to disconnect.
//! \param doKillCompletely [in] indicates whether Tree Function must
//!                              be completely killed (even from the list
//!                              of available Functions).
void ActData_BaseNode::disconnectTreeFunction(const Handle(ActData_TreeFunctionParameter)& thePFunc,
                                              const Standard_Boolean doKillCompletely)
{
  /* ===================================================================
   *  Remove back-references from the Function's input and output Nodes
   * =================================================================== */

  TDF_LabelList anArgList, aResList;

  // Disconnect as reader
  thePFunc->getArguments(anArgList);
  TDF_ListIteratorOfLabelList aParIt(anArgList);
  for ( ; aParIt.More(); aParIt.Next() )
  {
    const TDF_Label& aCurrentRoot = aParIt.Value();
    Handle(ActData_BaseNode) anArgNode =
      Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeByParamSettle(aCurrentRoot) );
    //
    if ( !anArgNode.IsNull() )
      anArgNode->disconnectReader(thePFunc);
  }

  // Disconnect as writer
  thePFunc->getResults(aResList);
  aParIt.Initialize(aResList);
  for ( ; aParIt.More(); aParIt.Next() )
  {
    const TDF_Label& aCurrentRoot = aParIt.Value();
    Handle(ActData_BaseNode) aResNode =
      Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeByParamSettle(aCurrentRoot) );
    //
    if ( !aResNode.IsNull() )
      aResNode->disconnectWriter(thePFunc);
  }

  /* ==========================
   *  Disconnect Tree Function
   * ========================== */

  // Here we have an option not to unbind the transient correspondence record
  // as Tree Function Parameter might be still alive.
  // Tree Function will be removed from Execution Graph. This is done in
  // order not to keep "dead" references after manual deletion of Nodes.
  thePFunc->Disconnect(doKillCompletely);
}

//! Registers the passed Tree Function as an input reader for the
//! Data Node. Conceptually, it means that the passed Tree Function declares
//! some Parameters of this Node as inputs.
//! \param theReader [in] Parameter to make a reference to.
//! \param isPrepend [in] indicates whether target should be prepended rather
//!                       than appended.
void ActData_BaseNode::connectReader(const Handle(ActData_TreeFunctionParameter)& theReader,
                                     const Standard_Boolean                       isPrepend)
{
  if ( m_paramScope.Meta->HasInputReader( theReader->RootLabel() ) )
    return; // Avoid setting the same back-reference

  if ( isPrepend )
    m_paramScope.Meta->PrependInputReader( theReader->RootLabel() );
  else
    m_paramScope.Meta->AppendInputReader( theReader->RootLabel() );
}

//! Registers the passed Tree Function as an output writer for the
//! Data Node. Conceptually, it means that the passed Tree Function declares
//! some Parameters of this Node as outputs.
//! \param theWriter [in] Parameter to make a reference to.
//! \param isPrepend [in] indicates whether target should be prepended rather
//!                       than appended.
void ActData_BaseNode::connectWriter(const Handle(ActData_TreeFunctionParameter)& theWriter,
                                     const Standard_Boolean                       isPrepend)
{
  if ( m_paramScope.Meta->HasOutputWriter( theWriter->RootLabel() ) )
    return; // Avoid setting the same back-reference

  if ( isPrepend )
    m_paramScope.Meta->PrependOutputWriter( theWriter->RootLabel() );
  else
    m_paramScope.Meta->AppendOutputWriter( theWriter->RootLabel() );
}

//! Registers the passed Reference (List) Parameter as a referrer for the
//! Data Node. Conceptually, it means that the passed Parameter contains
//! a link to one of this Node's Parameters.
//! \param theReferrer [in] Reference (or Reference List) Parameter to make
//!                         a back-pointer to.
//! \param isPrepend   [in] indicates whether target should be prepended rather
//!                         than appended.
void ActData_BaseNode::connectReferrer(const Handle(ActAPI_IUserParameter)& theReferrer,
                                       const Standard_Boolean               isPrepend)
{
  if ( m_paramScope.Meta->HasReferrer( theReferrer->RootLabel() ) )
    return; // Avoid setting the same back-reference

  if ( isPrepend )
    m_paramScope.Meta->PrependReferrer( theReferrer->RootLabel() );
  else
    m_paramScope.Meta->AppendReferrer( theReferrer->RootLabel() );
}

//! Removes a reference to the passed observer.
//! \param theObsType  [in] type of observer.
//! \param theObserver [in] Parameter to remove a reference for.
void ActData_BaseNode::disconnectObserver(const ObserverType                   theObsType,
                                          const Handle(ActAPI_IUserParameter)& theObserver)
{
  switch ( theObsType )
  {
    case Observer_InputReaders:
      this->disconnectReader( ActData_ParameterFactory::AsTreeFunction(theObserver) );
      break;
    case Observer_OutputWriters:
      this->disconnectWriter( ActData_ParameterFactory::AsTreeFunction(theObserver) );
      break;
    case Observer_Referrers:
      this->disconnectReferrer(theObserver);
      break;
    default:
      Standard_ProgramError::Raise("Unexpected observer type");
  }
}

//! Removes a reference to the passed Tree Function if it has been referenced
//! as a reader Parameter for the Data Node. Otherwise -- does nothing.
//! \param theReader [in] Parameter to remove a reference for.
void ActData_BaseNode::disconnectReader(const Handle(ActData_TreeFunctionParameter)& theReader)
{
  m_paramScope.Meta->RemoveInputReader( theReader->RootLabel() );
}

//! Removes a reference to the passed Tree Function if it has been referenced
//! as a writer Parameter for the Data Node. Otherwise -- does nothing.
//! \param theWriter [in] Parameter to remove a reference for.
void ActData_BaseNode::disconnectWriter(const Handle(ActData_TreeFunctionParameter)& theWriter)
{
  m_paramScope.Meta->RemoveOutputWriter( theWriter->RootLabel() );
}

//! Removes a back reference corresponding to the passed Reference Parameter.
//! \param theReference [in] Reference (or Reference List) Parameter to remove
//!                          a back reference for.
void ActData_BaseNode::disconnectReferrer(const Handle(ActAPI_IUserParameter)& theReference)
{
  m_paramScope.Meta->RemoveReferrer( theReference->RootLabel() );
}

//! Disconnects referrer (passed Parameter) for the given Data Cursor. If the
//! passed Data Cursor is a Parameter, then its owning Node will be used as
//! a back-reference holder.
//! \param theReference [in] referrer to disconnect.
//! \param theDC        [in] Data Cursor to disconnect referrer for.
void ActData_BaseNode::disconnectReferrerFor(const Handle(ActAPI_IUserParameter)& theReference,
                                             const Handle(ActAPI_IDataCursor)&    theDC)
{
  if ( theDC.IsNull() )
    return;

  Handle(ActData_BaseNode) aTargetNode;
  if ( theDC->IsKind( STANDARD_TYPE(ActAPI_INode) ) )
  {
    aTargetNode = Handle(ActData_BaseNode)::DownCast(theDC);
  }
  else if ( theDC->IsKind( STANDARD_TYPE(ActAPI_IUserParameter) ) )
  {
    Handle(ActAPI_IUserParameter) aTargetP = Handle(ActAPI_IUserParameter)::DownCast(theDC);
    aTargetNode = Handle(ActData_BaseNode)::DownCast( aTargetP->GetNode() );
  }

  if ( !aTargetNode.IsNull() )
    aTargetNode->disconnectReferrer(theReference);
}

//! Disconnects referrer for the given Node only if Parameters of this Node
//! are no more referenced by the passed Reference List.
//! \param theReferrer     [in] Reference List to disconnect or not.
//! \param theReferredNode [in] Node to disconnect or not observer for.
void ActData_BaseNode::disconnectReferrerSmart(const Handle(ActData_ReferenceListParameter)& theReferrer,
                                               const Handle(ActAPI_INode)&                   theReferredNode)
{
  // ...
  // Check whether the Reference List still contains any references
  // to the target Node. If so, we cannot disconnect the corresponding
  // back reference as we will loose back-trace for some other
  // remaining references
  // ...

  Standard_Boolean              stillSomeRefs = Standard_False;
  Handle(ActAPI_IParamIterator) PIt           = theReferredNode->GetParamIterator();
  //
  for ( ; PIt->More(); PIt->Next() )
  {
    const Handle(ActAPI_IUserParameter)& PFromRefNode = PIt->Value();
    if ( theReferrer->HasTarget(PFromRefNode) )
    {
      stillSomeRefs = Standard_True;
      break;
    }
  }

  // Remove back-reference
  if ( !stillSomeRefs )
    Handle(ActData_BaseNode)::DownCast(theReferredNode)->disconnectReferrer(theReferrer);
}
