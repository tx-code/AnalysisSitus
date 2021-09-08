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
#include <ActData_CAFConversionAsset.h>

// Active Data includes
#include <ActData_BasePartition.h>

#undef COUT_DEBUG

//! Constructor accepting the Data Model instance being adjusted.
//! \param theModel [in] working Data Model.
ActData_CAFConversionAsset::ActData_CAFConversionAsset(const Handle(ActAPI_IModel)& theModel)
{
  m_modelBase = Handle(ActData_BaseModel)::DownCast(theModel);
}

//! Checks whether the Data Model contains any BAD-FORMED Data Nodes.
//! \return true/false.
Standard_Boolean ActData_CAFConversionAsset::HasBadFormedNodes() const
{
  Handle(ActAPI_HPartitionList) aPList = m_modelBase->Partitions();
  for ( ActAPI_PartitionList::Iterator it( *aPList.operator->() ); it.More(); it.Next() )
  {
    // Access base Partition
    const Handle(ActAPI_IPartition)& aPart = it.Value();
    Handle(ActData_BasePartition) aPartBase = Handle(ActData_BasePartition)::DownCast(aPart);

    if ( aPartBase.IsNull() )
      Standard_ProgramError::Raise("Non-standard Partitions prohibited");

    // Iterate over the Nodes in their persistent order
    ActData_BasePartition::Iterator aNodeIt(aPartBase);
    for ( ; aNodeIt.More(); aNodeIt.Next() )
    {
      Handle(ActAPI_INode) aNode = aNodeIt.Value();

      if ( aNode.IsNull() || !aNode->IsWellFormed() )
        return Standard_True;
    }
  }

  return Standard_False;
}

//! Iterates over the entire Data Model and checks if there are any Nodal
//! Parameters with missed MTime information. MTime property required
//! for Data <-> Presentation synchronization and must exist from the
//! very beginning in order for Undo/Redo functionality to work.
//! \param theParams [out] Parameters having no MTime property.
//! \return true/false.
Standard_Boolean
  ActData_CAFConversionAsset::HasUnMTimedParams(Handle(ActAPI_HParameterList)& theParams) const
{
  Handle(ActAPI_HParameterList) aResult = new ActAPI_HParameterList();
  Handle(ActAPI_HPartitionList) aPList = m_modelBase->Partitions();
  for ( ActAPI_PartitionList::Iterator it( *aPList.operator->() ); it.More(); it.Next() )
  {
    // Access base Partition
    const Handle(ActAPI_IPartition)& aPart = it.Value();
    Handle(ActData_BasePartition) aPartBase = Handle(ActData_BasePartition)::DownCast(aPart);

    if ( aPartBase.IsNull() )
      Standard_ProgramError::Raise("Non-standard Partitions prohibited");

    // Iterate over the Nodes in their persistent order
    ActData_BasePartition::Iterator aNodeIt(aPartBase);
    for ( ; aNodeIt.More(); aNodeIt.Next() )
    {
      // Iterate over the underlying Node CAF data. Notice, that we iterate
      // not over the Parameter slots registered in the Nodal Cursor, but
      // rather over the actual raw OCAF structures. This is because currently
      // existing Cursor might have been updated by Application and became
      // unsynchronized with the actual data so. Ergo, we use it only to get
      // its root TDF Label and continue iterating by OCAF Labels directly

      TDF_Label aNodeLab = aNodeIt.Value()->RootLabel();
      this->accessUnMTimedParams(aNodeLab, aResult);
    }
  }

  theParams = aResult;
  return !aResult->IsEmpty();
}

//! Changes the internal versions (FRAMEWORK & APPLICATION) of the Data Model
//! by setting them to the actual ones.
void ActData_CAFConversionAsset::ActualizeVersions()
{
  m_modelBase->bindVersionInfo();
}

//! Iterates over the entire Data Model and sets actual MTime for those Nodal
//! Parameters which do not have it yet.
void ActData_CAFConversionAsset::ModifyParams()
{
  Handle(ActAPI_HParameterList) aParamList;
  if ( !this->HasUnMTimedParams(aParamList) )
    return;

  for ( ActAPI_ParameterList::Iterator it( *aParamList.operator->() ); it.More(); it.Next() )
    it.ChangeValue()->SetModified();
}

//! Changes the type of the given (existing) Parameter to the requested one.
//! This method will clean up the initial Parameter data without a care of any
//! references which might exist in other Data Nodes or even in the same Node.
//! Ergo, use this method with care and do not forget to update the referrers
//! if any.
//! \param theNode [in] Data Node to adjust.
//! \param thePID [in] ID of the Parameter to change the type for.
//! \param theNewType [in] new Parameter type.
//! \return new Parameter instance of the desired type. This instance is a
//!         BAD-FORMED Data Cursor ATTACHED to the CAF source and ready to be
//!         populated with actual data so;
Handle(ActAPI_IUserParameter)
  ActData_CAFConversionAsset::ChangeParameterType(const Handle(ActAPI_INode)& theNode,
                                                  const Standard_Integer thePID,
                                                  const ActAPI_ParameterType theNewType)
{
  Handle(ActAPI_IUserParameter) P = this->settleParam(theNode, thePID);
  if ( P.IsNull() )
    return NULL;

#if defined COUT_DEBUG
  std::cout << "CAF Conversion> changing type of Parameter " << P->DynamicType()->Name() << std::endl;
#endif

  // Clean up CAF raw data
  P->RootLabel().ForgetAllAttributes();

  // Expand new Parameter of the desired type
  return this->expandParam(theNode, thePID, theNewType);
}

//! Allocates new CAF Label for the Nodal Parameter with the given PID in the
//! USER scope of the Node.
//! \param theNode [in] Data Node to allocate the Parameter raw source for.
//! \param thePID [in] Parameter ID used as OCAF tag.
//! \param theType [in] type of the Parameter to expand on the raw source.
//! \return Parameter instance (BAD-FORMED).
Handle(ActAPI_IUserParameter)
  ActData_CAFConversionAsset::expandParam(const Handle(ActAPI_INode)& theNode,
                                          const Standard_Integer thePID,
                                          const ActAPI_ParameterType theType) const
{
  TDF_Label aParamRoot = this->uScopePRoot(theNode, thePID, Standard_True);

  Standard_Boolean isUndefinedType;
  return ActData_ParameterFactory::NewParameterExpand(theType, aParamRoot, isUndefinedType);
}

//! Settles down a Parameter for the given PID on the Nodal CAF source for the
//! given Data Node. The type of the Parameter is defined automatically.
//! \param theNode [in] Data Node to access Parameter data for.
//! \param thePID [in] ID of the Parameter to access.
//! \return Parameter instance.
Handle(ActAPI_IUserParameter)
  ActData_CAFConversionAsset::settleParam(const Handle(ActAPI_INode)& theNode,
                                          const Standard_Integer thePID) const
{
  TDF_Label aParamRoot = this->uScopePRoot(theNode, thePID, Standard_False);
  if ( aParamRoot.IsNull() )
    return NULL;

  Standard_Boolean isUndefinedType;
  return ActData_ParameterFactory::NewParameterSettle(aParamRoot, isUndefinedType);
}

//! Accessor for the root TDF Label corresponding to the given Parameter ID in
//! passed Data Node.
//! \param theNode [in] Data Node to access data for.
//! \param thePID [in] Parameter local ID.
//! \param toCreate [in] indicates whether to create the root TDF Label for the
//!        Nodal Parameter if it does not exist yet.
//! \return root TDF Label for the Parameter.
TDF_Label
  ActData_CAFConversionAsset::uScopePRoot(const Handle(ActAPI_INode)& theNode,
                                          const Standard_Integer thePID,
                                          const Standard_Boolean toCreate) const
{
  TDF_Label aUScope = theNode->RootLabel().FindChild(ActData_BaseNode::TagUser, Standard_False);
  return aUScope.FindChild(thePID, toCreate);
}

//! Given the root Nodal Label, this method traverses all its underlying
//! Parameters and checks whether they have MTime attribute set or not. If not,
//! such Parameters are pushed to the output collection for further processing.
//! \param theNodeLab [in] root Nodal Label.
//! \param theParams [out] collection of Nodal Parameters having no MTime
//!        attribute associated.
void ActData_CAFConversionAsset::accessUnMTimedParams(const TDF_Label& theNodeLab,
                                                      Handle(ActAPI_HParameterList)& theParams) const
{
  Handle(ActAPI_HParameterList)
    anInternalParams = this->accessUnMTimedParamsBySection( theNodeLab.FindChild(ActData_BaseNode::TagInternal) );
  Handle(ActAPI_HParameterList)
    aUserParams = this->accessUnMTimedParamsBySection( theNodeLab.FindChild(ActData_BaseNode::TagUser) );

  // Push Parameter collections to output
  for ( ActAPI_ParameterList::Iterator it( *anInternalParams.operator->() ); it.More(); it.Next() )
    theParams->Append( it.Value() );
  for ( ActAPI_ParameterList::Iterator it( *aUserParams.operator->() ); it.More(); it.Next() )
    theParams->Append( it.Value() );
}

//! Given the root TDF Label of a particular Nodal section, this method finds
//! all Parameters having no MTime attribute associated.
//! \param theSectionLab [in] root Label of a particular Nodal section.
//! \return collection of Parameters having no MTime attribute associated.
Handle(ActAPI_HParameterList)
  ActData_CAFConversionAsset::accessUnMTimedParamsBySection(const TDF_Label& theSectionLab) const
{
  Handle(ActAPI_HParameterList) aResult = new ActAPI_HParameterList();
  for ( TDF_ChildIterator it(theSectionLab, Standard_False); it.More(); it.Next() )
  {
    TDF_Label aParamRootLab = it.Value();

    if ( !ActData_ParameterFactory::IsUserParameter(aParamRootLab) )
      Standard_ProgramError::Raise("Not valid CAF Document structure");

    Standard_Boolean isUndefinedType;
    Handle(ActData_UserParameter)
      aParamBase = Handle(ActData_UserParameter)::DownCast( ActData_ParameterFactory::NewParameterSettle(aParamRootLab,
                                                                                                         isUndefinedType) );

    Handle(ActAux_TimeStamp) aTS = aParamBase->GetMTime();
    if ( aTS->IsOrigin() )
      aResult->Append(aParamBase);
  }
  return aResult;
}
