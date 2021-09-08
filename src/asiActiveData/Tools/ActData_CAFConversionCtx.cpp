//-----------------------------------------------------------------------------
// Created on: March 2013
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
#include <ActData_CAFConversionCtx.h>

// Active Data (auxiliary) includes
#include <ActAux_Utils.h>

// Active Data includes
#include <ActData_CAFConversionAsset.h>
#include <ActData_CAFDumper.h>

// Active Data (auxiliary) includes
#include <ActAux_TimeStamp.h>

// OCCT includes
#include <Message_ProgressSentry.hxx>
#include <OSD_Directory.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TDF_Tool.hxx>

#define CARD_DUMP "dump"
#define CARD_TMP "tmp"
#define CARD_SLASH "\\"
#define CARD_UNDERSCORE "_"
#define CARD_DOT "."

#undef CTX_DEBUG

//! Complete constructor.
//! \param theDumpPath [in] directory for dumping of temporary Data Model
//!        copies.
ActData_CAFConversionCtx::ActData_CAFConversionCtx(const TCollection_AsciiString& theDumpPath)
: m_dumpPath(theDumpPath),
  m_sampler(new ActData_CAFConversionModel)
{
}

//! Cleans up all the registered modification records.
void ActData_CAFConversionCtx::Clear()
{
  m_oriModel.Nullify();
  m_resModel.Nullify();
  m_modif.Clear();
  m_sampler->Clear();
}

//! Records insertion modification for the given Parameter. This Parameter
//! is going to be inserted before those identified by the GID passed as a
//! second argument. If the second argument is omitted, insertion will
//! work as appending.
//! \param theParamDTO [in] Parameter data to insert.
//! \param theGIDBefore [in] GID of the Parameter to insert the new one before.
//! \return true if request has been registered, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::Insert(const Handle(ActData_ParameterDTO)& theParamDTO,
                                                  const ActAPI_ParameterGID& theGIDBefore)
{
  return m_modif.AddInsert( new HRecord(theGIDBefore, theParamDTO) );
}

//! Records updating modification for the given Parameter.
//! \param theGID [in] GID of the Parameter to affect.
//! \param theNewParamDTO [in] new data to apply.
//! \return true if request has been registered, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::Update(const ActAPI_ParameterGID& theGID,
                                                  const Handle(ActData_ParameterDTO)& theNewParamDTO)
{
  return m_modif.AddUpdate( new HRecord(theGID, theNewParamDTO) );
}

//! Records deletion modification.
//! \param theGID [in] GID of the Parameter to delete.
//! \return true if request has been registered, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::Delete(const ActAPI_ParameterGID& theGID)
{
  return m_modif.AddDelete( new HRecord(theGID) ); // NULL as data
}

//! Applies all recorded modifications.
//! \param theOldModel [in] Data Model instance to apply modifications on.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFConversionCtx::Apply(const Handle(ActAPI_IModel)& theModel,
                                  const Handle(Message_ProgressIndicator)& theProgress)
{
  Message_ProgressSentry PEntry(theProgress, "CAF_CONVERSION_CTX_APPLYING", 0, 2, 1);
  PEntry.Show();

  /* =================================
   *  Save & retrieve Model into copy
   * ================================= */

  m_tempModels.Append(m_oriModel);
  m_oriModel = theModel;
  m_tempModels.Append(m_resModel);
  m_resModel = theModel->Clone();

  if ( !this->saveAndRetrieve(m_oriModel, m_resModel) )
    return Standard_False;

  // Disable transactions
  m_resModel->DisableTransactions();

#if defined CTX_DEBUG
  this->dumpModel(m_resModel);
#endif

  /* =====================================
   *  Apply modifications & normalization
   * ===================================== */

  if ( !this->applyModifications(theProgress) )
  {
#if defined CTX_DEBUG
    this->dumpModel(m_resModel);
#endif
    return Standard_False;
  }

  // Step progress
  PEntry.Next();
  PEntry.Show();

  if ( !this->applyNormalization(theProgress) )
  {
#if defined CTX_DEBUG
    this->dumpModel(m_resModel);
#endif
    return Standard_False;
  }

  /* ==============
   *  Finalization
   * ============== */

  // ...
  // One more copy to assure data consistency
  // ...

  Handle(ActAPI_IModel) aNewModel = m_resModel->Clone();

  if ( !this->saveAndRetrieve(m_resModel, aNewModel) )
    return Standard_False;

  m_tempModels.Append(m_resModel);
  m_resModel = aNewModel;

  // ...
  // Clean up LogBook
  // ...

  // Disable transactions
  m_resModel->DisableTransactions();
  m_resModel->FuncReleaseLogBook();

#if defined CTX_DEBUG
  this->dumpModel(m_resModel);
#endif

  // Release all temporary Model instances
  for ( Standard_Integer i = 1; i <= m_tempModels.Length(); ++i )
  {
    const Handle(ActAPI_IModel)& M = m_tempModels.Value(i);
    if ( !M.IsNull() )
      M->Release();
  }
  m_tempModels.Clear();

  return Standard_True;
}

//! Accessor for the resulting Model instance.
//! \return resulting Data Model instance.
const Handle(ActAPI_IModel)& ActData_CAFConversionCtx::Result() const
{
  m_resModel->EnableTransactions();
  return m_resModel;
}

//-----------------------------------------------------------------------------
// Internals for APPLYING
//-----------------------------------------------------------------------------

//! Applies normalization process.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFConversionCtx::applyNormalization(const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Construct relocation map
  m_sampler->BuildRelocationMap();

  // Iterate over all registered Partitions
  Handle(ActData_BaseModel) MBase = Handle(ActData_BaseModel)::DownCast(m_resModel);
  for ( PartitionMap::Iterator pit( *MBase->m_partitionMap.operator->() );
        pit.More(); pit.Next() )
  {
    // Access Partition
    Handle(ActData_BasePartition) Part = Handle(ActData_BasePartition)::DownCast( pit.Value() );

    // Iterate over the Nodes in their persistent order
    for ( ActData_BasePartition::Iterator nit(Part); nit.More(); nit.Next() )
    {
      Handle(ActData_BaseNode) N = Handle(ActData_BaseNode)::DownCast( nit.Value() );

      // Access section root Labels
      TDF_Label NRoot                   = N->RootLabel();
      TDF_Label NRootUser               = NRoot     .FindChild(ActData_BaseNode::TagUser,               Standard_False);
      TDF_Label NRootMeta               = NRoot     .FindChild(ActData_BaseNode::TagInternal,           Standard_False);
      TDF_Label NRootMeta_InputReaders  = NRootMeta .FindChild(ActData_MetaParameter::DS_InputReaders,  Standard_False);
      TDF_Label NRootMeta_OutputWriters = NRootMeta .FindChild(ActData_MetaParameter::DS_OutputWriters, Standard_False);
      TDF_Label NRootMeta_Referrers     = NRootMeta .FindChild(ActData_MetaParameter::DS_Referrers,     Standard_False);
      TDF_Label NRootMeta_Evaluators    = NRootMeta .FindChild(ActData_MetaParameter::DS_Evaluators,    Standard_False);

      // ...
      // Perform normalization in sections
      // ...

      // Back-references (input readers)
      if ( !NRootMeta_InputReaders.IsNull() )
        if ( !this->normalizeReferenceList(NRootMeta_InputReaders, Standard_True) )
          return Standard_False;

      // Back-references (output writers)
      if ( !NRootMeta_OutputWriters.IsNull() )
        if ( !this->normalizeReferenceList(NRootMeta_OutputWriters, Standard_True) )
          return Standard_False;

      // Back-references (referrers)
      if ( !NRootMeta_Referrers.IsNull() )
        if ( !this->normalizeReferenceList(NRootMeta_Referrers, Standard_True) )
          return Standard_False;

      // Evaluators
      if ( !NRootMeta_Evaluators.IsNull() )
        if ( !this->normalizeNodalSection(NRootMeta_Evaluators, Standard_True) )
          return Standard_False;

      // USER section with any reference types
      if ( !NRootUser.IsNull() )
        if ( !this->normalizeNodalSection(NRootUser, Standard_False) )
          return Standard_False;
    }
  }
  return Standard_True;
}

//! Applies modification requests on the resulting Data Model.
//! \param theProgress [in] Progress Indicator.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFConversionCtx::applyModifications(const Handle(Message_ProgressIndicator)& ActData_NotUsed(theProgress))
{
  // Get modification lists
  NCollection_Sequence<Handle(HRecord)>& aListInsert = m_modif.ListInsert();
  NCollection_Sequence<Handle(HRecord)>& aListDelete = m_modif.ListDelete();
  NCollection_Sequence<Handle(HRecord)>& aListUpdate = m_modif.ListUpdate();

  /* ===========================================
   *  Apply modifications on Conversion Sampler
   * =========================================== */

  // Clean up Conversion Model (sampler)
  m_sampler->Clear();

  // Apply INSERTION requests
  for ( NCollection_Sequence<Handle(HRecord)>::Iterator mit(aListInsert); mit.More(); mit.Next() )
  {
    const Handle(HRecord)& aRec = mit.Value();
    if ( !this->applyInsert(aRec) )
      return Standard_False;
  }

  // Apply DELETION requests
  for ( NCollection_Sequence<Handle(HRecord)>::Iterator mit(aListDelete); mit.More(); mit.Next() )
  {
    const Handle(HRecord)& aRec = mit.Value();
    if ( !this->applyRemove(aRec) )
      return Standard_False;
  }

  // Apply UPDATING requests
  for ( NCollection_Sequence<Handle(HRecord)>::Iterator mit(aListUpdate); mit.More(); mit.Next() )
  {
    const Handle(HRecord)& aRec = mit.Value();
    if ( !this->applyUpdate(aRec) )
      return Standard_False;
  }

  /* ===========================
   *  Propagate Sampler to OCAF
   * =========================== */

  if ( !this->samplerToOCAF() )
    return Standard_False;

  return Standard_True;
}

//! Applies INSERTION record.
//! \param theRec [in] modification record to apply.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::applyInsert(const Handle(HRecord)& theRec)
{
  ActAPI_DataObjectId NID = theRec->Data->GID().NID;

  // Register Conversion Node with set of original Parameters if not yet
  if ( !m_sampler->ContainsNode(NID) )
  {
    if ( !m_sampler->AddOriginNode(NID, m_resModel) )
      return Standard_False; // Seems like data framework has been changed, so we can do nothing
  }

  // Propagate modification on Sampler
  return m_sampler->GetNode(NID)->Insert(theRec->Data, theRec->GID);
}

//! Applies UPDATING record.
//! \param theRec [in] modification record to apply.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::applyUpdate(const Handle(HRecord)& theRec)
{
  ActAPI_DataObjectId NID = theRec->GID.NID;

  // Register Conversion Node with original set Parameter if not yet
  if ( !m_sampler->ContainsNode(NID) )
  {
    if ( !m_sampler->AddOriginNode(NID, m_resModel) )
      return Standard_False; // Seems like data framework has been changed, so we can do nothing
  }

  // Propagate modification on Sampler
  return m_sampler->GetNode(NID)->Update(theRec->GID, theRec->Data);
}

//! Applies DELETION record.
//! \param theRec [in] modification record to apply.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::applyRemove(const Handle(HRecord)& theRec)
{
  ActAPI_DataObjectId NID = theRec->GID.NID;

  // Register Conversion Node with original set Parameter if not yet
  if ( !m_sampler->ContainsNode(NID) )
  {
    if ( !m_sampler->AddOriginNode(NID, m_resModel) )
      return Standard_False; // Seems like data framework has been changed, so we can do nothing
  }

  // Propagate modification on Sampler
  return m_sampler->GetNode(NID)->Remove(theRec->GID);
}

//-----------------------------------------------------------------------------
// Internals
//-----------------------------------------------------------------------------

//! Performs normalization starting from the given Label representing root
//! of Nodal sub-section. All children of this root are suggested as root
//! Labels for Parameters.
//! \param theNodeRoot [in] root Label to start normalization process from.
//! \param isInternal  [in] true if the target section is internal for Node.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFConversionCtx::normalizeNodalSection(const TDF_Label&       theNodeRoot,
                                                  const Standard_Boolean isInternal)
{
  Handle(ActData_BaseNode)
    PRefOwner = Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeSettle( theNodeRoot.Father() ) );

  // Iterate over Nodal Parameters
  for ( TDF_ChildIterator lit(theNodeRoot); lit.More(); lit.Next() )
  {
    const TDF_Label PRoot = lit.Value();

    // Check if we can settle down Parameter Cursor on the currently iterated
    // Label. It can happen that this Label is a "ghost" one if it corresponds
    // to removed Parameter with any remaining references
    Handle(ActAPI_IUserParameter) P;
    if ( ActParamTool::IsUserParameter(PRoot) )
    {
      Standard_Boolean isUndefinedType;
      P = ActParamTool::NewParameterSettle(PRoot, isUndefinedType);
    }

    // If we face "ghost" Parameter, we just skip it. Subsequently all
    // references to this Parameter will be normalized, so this ghost will
    // eventually find peace ;)
    if ( P.IsNull() )
      continue;

    if ( P->GetParamType() == Parameter_Reference ) // DIRECT REFERENCE
    {
      Handle(ActData_ReferenceParameter) PRef = ActParamTool::AsReference(P);
      TDF_Label TargetLab = PRef->GetTargetLabel();

      // All the following cannot be done for Nodes
      if ( ActData_NodeFactory::IsNode(TargetLab) )
        continue;

      // We proceed with Parameters only
      if ( !TargetLab.IsNull() )
      {
        ActAPI_ParameterGID OldGID = ActData_Utils::ConvertToGID(TargetLab, isInternal);
        ActAPI_ParameterGID NewGID = m_sampler->Converted(OldGID);

        if ( NewGID.IsNull() ) // Target has been removed (PID == -1)
        {
          PRef->RemoveTarget(); // Nullify target

          // Get target Node to disconnect the nullified listener
          TDF_Label NTargetRoot = TargetLab.Father()  // Section
                                           .Father(); // Root Nodal Label
          Handle(ActData_BaseNode)
            NTarget = Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeSettle(NTargetRoot) );

          // Disconnect referrer
          if ( !NTarget.IsNull() )
            NTarget->disconnectReferrer(PRef);
        }
        else // Target is Ok, just convert it
        {
          Handle(ActAPI_IUserParameter) PNew = this->paramByGID(NewGID);
          if ( PNew.IsNull() )
            return Standard_False;

          PRef->SetTarget(PNew);
        }
      }
    }
    else if ( P->GetParamType() == Parameter_ReferenceList ) // REFERENCE LIST
    {
      // Convert to Reference List Parameter
      Handle(ActData_ReferenceListParameter) PRef = ActParamTool::AsReferenceList(P);

      // Access owner Label for Reference List attribute
      TDF_Label refListOwner = ActData_Utils::ChooseLabelByTag(PRef->RootLabel(),
                               ActData_ReferenceListParameter::DS_Targets);

      // Normalize reference list passing 'false' value as we cannot have
      // internal Reference List Parameters
      if ( !this->normalizeReferenceList(refListOwner, Standard_False) )
        return Standard_False;
    }
    else if ( P->GetParamType() == Parameter_TreeFunction ) // TREE FUNCTION
    {
      Handle(ActData_TreeFunctionParameter) PFunc = ActParamTool::AsTreeFunction(P);

      // Get ARGUMENTS and RESULTS
      TDF_LabelList ArgLabelList; PFunc->getArguments(ArgLabelList);
      TDF_LabelList ResLabelList; PFunc->getResults(ResLabelList);

      // Get OCAF attributes containing actual references
      Handle(TDataStd_ReferenceList) ArgListAttr = PFunc->getArgumentsAttr();
      Handle(TDataStd_ReferenceList) ResListAttr = PFunc->getResultsAttr();

      // Boolean flag indicating whether some argument or result Parameter
      // is removed during the conversion process. If so, Tree Function will
      // be disconnected
      Standard_Boolean isAnyRemoved = Standard_False;

      // ...
      // Normalize ARGUMENTS
      // ...

      if ( !ArgListAttr.IsNull() )
      {
        // Clean up OCAF attribute
        ArgListAttr->Clear();

        for ( TDF_ListIteratorOfLabelList lit1(ArgLabelList); lit1.More(); lit1.Next() )
        {
          TDF_Label& ArgLab = lit1.Value();
          ActAPI_ParameterGID OldGID = ActData_Utils::ConvertToGID(ArgLab, isInternal);
          ActAPI_ParameterGID NewGID = m_sampler->Converted(OldGID);

          if ( !NewGID.IsNull() )
          {
            Handle(ActAPI_IUserParameter) PNew = this->paramByGID(NewGID);

            TDF_Label NewArgLab;
            if ( PNew.IsNull() )
              NewArgLab = ArgLab;
            else
              NewArgLab = PNew->RootLabel();

            ArgListAttr->Append( PNew->RootLabel() ); // Adjust target
          }
          else if ( !isAnyRemoved )
            isAnyRemoved = Standard_True;
        }
      }

      // ...
      // Normalize RESULTS
      // ...

      if ( !ResListAttr.IsNull() )
      {
        // Clean up OCAF attribute
        ResListAttr->Clear();

        for ( TDF_ListIteratorOfLabelList lit1(ResLabelList); lit1.More(); lit1.Next() )
        {
          TDF_Label& ResLab = lit1.Value();
          ActAPI_ParameterGID OldGID = ActData_Utils::ConvertToGID(ResLab, isInternal);
          ActAPI_ParameterGID NewGID = m_sampler->Converted(OldGID);

          if ( !NewGID.IsNull() )
          {
            Handle(ActAPI_IUserParameter) PNew = this->paramByGID(NewGID);

            TDF_Label NewResLab;
            if ( PNew.IsNull() )
              NewResLab = ResLab;
            else
              NewResLab = PNew->RootLabel();

            ResListAttr->Append( PNew->RootLabel() ); // Adjust target
          }
          else if ( !isAnyRemoved )
            isAnyRemoved = Standard_True;
        }
      }

      // ...
      // Perform disconnection if needed
      // ...

      if ( isAnyRemoved )
        PRefOwner->disconnectTreeFunction(PFunc, Standard_True);
    }
  }

  return Standard_True;
}

//! Performs normalization in Reference List attribute owned by the passed
//! OCAF Label.
//! \param theRefListOwner [in] owning Label.
//! \param isBackRef       [in] indicates whether the owning Label stores
//!                             back-references.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_CAFConversionCtx::normalizeReferenceList(const TDF_Label&       theRefListOwner,
                                                   const Standard_Boolean isBackRef)
{
  if ( theRefListOwner.IsNull() )
    return Standard_True;

  // Recover owner Data Cursors
  Handle(ActAPI_INode)          ownerNode;
  Handle(ActAPI_IUserParameter) ownerParam;
  //
  if ( !isBackRef )
  {
    // Node
    ownerNode = ActData_NodeFactory::NodeSettle( theRefListOwner.Father() // Parameter's root
                                                                .Father() // USER section
                                                                .Father() // Node's root
                                                );
    if ( ownerNode.IsNull() )
      return Standard_False;

    // Parameter
    Standard_Boolean isUndefinedType;
    ownerParam = ActData_ParameterFactory::NewParameterSettle( theRefListOwner.Father(),
                                                               isUndefinedType );
    //
    if ( ownerParam.IsNull() )
      return Standard_False;
  }

  // Get Reference List attribute
  Handle(TDataStd_ReferenceList) refAttr;
  if ( !theRefListOwner.FindAttribute(TDataStd_ReferenceList::GetID(), refAttr) )
    return Standard_False;

  // Collection of new Labels
  TDF_LabelList NewRefTargets;

  // ...
  // Disconnect all references initially and prepare new list of targets
  // ...

  TDF_LabelList oldTargetLabels = refAttr->List();
  //
  for ( TDF_ListIteratorOfLabelList lit(oldTargetLabels); lit.More(); lit.Next() )
  {
    TDF_Label& TargetLab = lit.Value();

    // All the following cannot be done for Nodes
    if ( ActData_NodeFactory::IsNode(TargetLab) )
      continue;

    // We proceed with Parameters only
    if ( !TargetLab.IsNull() )
    {
      refAttr->Remove(TargetLab); // Clean up corresponding target from attribute

      // The following IF is used to kill a back-reference from the target
      // Node. This code is only useful if the current reference list does
      // not represent back-references. If it does, then we don't have to
      // clean up anything externally
      if ( !isBackRef )
      {
        // Get target Node to disconnect the nullified listener
        TDF_Label NTargetRoot = TargetLab.Father()  // Section
                                         .Father(); // Root Nodal Label
        Handle(ActData_BaseNode)
          NTarget = Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeSettle(NTargetRoot) );

        // Disconnect referrer
        if ( !NTarget.IsNull() )
          NTarget->disconnectReferrerSmart(ActParamTool::AsReferenceList(ownerParam), NTarget);
      }

      // Get converted Label for the target. Notice that 'false' is passed to
      // GID construction function. It means that the target Parameter is
      // always assumed here as non-internal. Indeed, we cannot have any
      // references to internal Parameters (at least we cannot deal with such
      // exotic cases here)
      ActAPI_ParameterGID OldGID = ActData_Utils::ConvertToGID(TargetLab, Standard_False);
      ActAPI_ParameterGID NewGID = m_sampler->Converted(OldGID);

      if ( !NewGID.IsNull() ) // Target has been removed (PID == -1)
      {
        Handle(ActAPI_IUserParameter) PNew = this->paramByGID(NewGID);

        TDF_Label NewTargetLab;
        if ( PNew.IsNull() )
          NewTargetLab = TargetLab; // Cannot convert
        else
          NewTargetLab = PNew->RootLabel();

        NewRefTargets.Prepend(NewTargetLab);
      }
    }
  }

  // ...
  // Connect new targets
  // ...

  // Process target Labels one-by-one
  for ( TDF_ListIteratorOfLabelList lit(NewRefTargets); lit.More(); lit.Next() )
  {
    TDF_Label& NewTargetLab = lit.Value();

    // Before adding the new target as a back-reference, let's check if
    // the direct referrer (either Tree Function or Reference [List]) is still
    // valid. E.g. the original Tree Function may happen to be disconnected
    // at this moment
    Standard_Boolean isAlive = Standard_True;
    //
    if ( isBackRef )
    {
      Standard_Boolean isUndefinedType;
      Handle(ActAPI_IUserParameter)
        targetParam = ActData_ParameterFactory::NewParameterSettle(NewTargetLab,
                                                                   isUndefinedType);
      //
      if ( targetParam.IsNull() )
        continue;

      // Check if Tree Function still references this Label
      if ( targetParam->IsKind( STANDARD_TYPE(ActData_TreeFunctionParameter) ) )
      {
        Handle(ActData_TreeFunctionParameter)
          targetTF = Handle(ActData_TreeFunctionParameter)::DownCast(targetParam);

        // TODO: the following check is not enough. We should check that
        //       referrer really contains a reference to the Node being
        //       converted
        if ( targetTF->Arguments().IsNull() && targetTF->Results().IsNull() )
          isAlive = Standard_False;
      }
    }
    //
    if ( !isAlive )
      continue;

    // Add new target Label (if not yet)
    if ( !ActData_Utils::HasTarget(refAttr->List(), NewTargetLab) )
      ActData_Utils::PrependReference(refAttr->Label(), -1, NewTargetLab);

    // Recover back-reference in the target
    if ( !isBackRef )
    {
      // Get target Node to connect referrer
      TDF_Label NTargetRoot = NewTargetLab.Father()  // Section
                                          .Father(); // Root Nodal Label
      Handle(ActData_BaseNode)
        NTarget = Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeSettle(NTargetRoot) );

      // Connect referrer
      if ( !ActAPI_IDataCursor::IsEqual(ownerNode, NTarget) )
        NTarget->connectReferrer(ownerParam, Standard_True);
    }
  }

  return Standard_True;
}

//! Returns Parameter instance by GID.
//! \param theGID [in] Parameter GID.
//! \return Parameter instance.
Handle(ActAPI_IUserParameter)
  ActData_CAFConversionCtx::paramByGID(const ActAPI_ParameterGID& theGID) const
{
  // Access OCAF Node
  Handle(ActAPI_INode) N = m_resModel->FindNode(theGID.NID);

  if ( N.IsNull() )
    return NULL; // Protect against inconsistent GIDs

  // Access next Persistent Parameter
  TDF_Label PLab =
    N->RootLabel().FindChild(ActData_BaseNode::TagUser, Standard_False)
                  .FindChild(theGID.PID, Standard_False);

  if ( !ActParamTool::IsUserParameter(PLab) )
    return NULL;

  Standard_Boolean isUndefinedType;
  return ActParamTool::NewParameterSettle(PLab, isUndefinedType);
}

//! Transfers data cumulated by Sampler to OCAF Document of
//! the resulting Model.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_CAFConversionCtx::samplerToOCAF()
{
  /* ==============================================================
   *  First pass: normalize Nodes & remove existing CAF structures
   * ============================================================== */

  for ( ActData_CAFConversionModel::Iterator sit(m_sampler); sit.More(); sit.Next() )
  {
    // Access currently iterated Conversion Node
    const ActAPI_DataObjectId& NID = sit.Key();
    const Handle(ActData_CAFConversionNode)& aConvNode = sit.Value();

    // Re-enumerate PIDs
    aConvNode->NormalizePIDs();

    // Access USER section of Nodal Parameters
    TDF_Label aUGroupLab = this->uSectionRoot(NID, m_resModel, Standard_False);

    if ( aUGroupLab.IsNull() )
      return Standard_False; // No USER group -> data inconsistent

    for ( TDF_ChildIterator cit(aUGroupLab); cit.More(); cit.Next() )
    {
      const TDF_Label aParamRootLab = cit.Value();
      ActData_Utils::RemoveWithReferences(aParamRootLab);
    }
  }

  // Clean up LogBook
  Handle(ActData_BaseModel) aResModelBase = Handle(ActData_BaseModel)::DownCast(m_resModel);
  aResModelBase->LogBook().Label().ForgetAllAttributes();

  // Prepare new Model
  Handle(ActAPI_IModel) aNewModel = m_resModel->Clone();

  // Complete removal by re-loading into copy, so that all "ghost" Labels
  // will disappear
  if ( !this->saveAndRetrieve(m_resModel, aNewModel) )
    return Standard_False;

  // Re-initialize result and disable transactions
  m_tempModels.Append(m_resModel);
  m_resModel = aNewModel;
  m_resModel->DisableTransactions();

  /* ===========================================================
   *  Second pass: populate USER sections from Conversion Nodes
   * =========================================================== */

  for ( ActData_CAFConversionModel::Iterator sit(m_sampler); sit.More(); sit.Next() )
  {
    // Access currently iterated Conversion Node
    const ActAPI_DataObjectId& NID = sit.Key();
    const Handle(ActData_CAFConversionNode)& aConvNode = sit.Value();

    // Access USER section of Nodal Parameters
    TDF_Label aUGroupLab = this->uSectionRoot(NID, m_resModel, Standard_True);

    // Populate OCAF USER section with DTO values
    for ( ActData_CAFConversionNode::Iterator pit(aConvNode); pit.More(); pit.Next() )
    {
      // Access next Conversion Parameter instance
      const Handle(ActData_CAFConversionParameter)& aConvParam = pit.Value();
      Handle(ActData_ParameterDTO)& aDTO = aConvParam->ChangeData();
      Standard_Integer aNewPID = aDTO->GID().PID;

      if ( aConvParam->GetHistory().Evolution == ActData_CAFConversionParameter::Evolution_Deleted )
        continue; // Deletion case

      // Expand Parameter data in OCAF
      Standard_Boolean isUndefinedType;
      TDF_Label aParamRoot = aUGroupLab.FindChild(aNewPID, Standard_True);
      //
      Handle(ActAPI_IUserParameter)
        aParam = ActParamTool::NewParameterExpand(aDTO->ParamType(), aParamRoot, isUndefinedType);

      // Populate Parameter with data
      Handle(ActData_UserParameter) aParamBase = Handle(ActData_UserParameter)::DownCast(aParam);
      aParamBase->SetFromDTO(aDTO, MT_Silent, Standard_False, Standard_False);
    }
  }

  return Standard_True;
}

//! Saves the passed Data Model into temporary files and then retrieves it
//! back into another (resulting) Model. The temporary file is removed
//! automatically.
//! \param theOldModel [in] Data Model to save & retrieve.
//! \param theNewModel [in/out] new (retrieved) Data Model instance.
Standard_Boolean
  ActData_CAFConversionCtx::saveAndRetrieve(const Handle(ActAPI_IModel)& theOldModel,
                                            const Handle(ActAPI_IModel)& theNewModel)
{
  Handle(ActData_BaseModel) anOldModelBase = Handle(ActData_BaseModel)::DownCast(theOldModel);
  Handle(ActData_BaseModel) aNewModelBase = Handle(ActData_BaseModel)::DownCast(theNewModel);

  TCollection_AsciiString aTempFilename = this->temporaryFilename( anOldModelBase->Document() );
  TCollection_AsciiString aTempFilenameRen = this->temporaryFilename( aNewModelBase->Document(),
                                                                      Standard_True );

  // Extract filename components for additional existence checks
  TCollection_AsciiString aBasePathTmp = ActAux_Utils::Str::BasePath(aTempFilename);
  TCollection_AsciiString aBasePathRen = ActAux_Utils::Str::BasePath(aTempFilenameRen);
  TCollection_AsciiString aBaseFnTmp = ActAux_Utils::Str::BaseFilename(aTempFilename, Standard_True);
  TCollection_AsciiString aBaseFnRen = ActAux_Utils::Str::BaseFilename(aTempFilenameRen, Standard_True);

  // Check if directories exist. If not, prepare use current directory
  OSD_Directory aDirTmp(aBasePathTmp), aDirRen(aBasePathRen);
  if ( !aDirTmp.Exists() || !aDirRen.Exists() )
  {
    aTempFilename = aBaseFnTmp;
    aTempFilenameRen = aBaseFnRen;
  }

  // Save Model to temporary file
  if ( !theOldModel->SaveAs(aTempFilename) )
    return Standard_False;

  // Rename temporary file in order to deceive OCAF Application and
  // avoid "Already Opened" retrieval status so
  OSD_Path aPathTmp(aTempFilename), aPathRen(aTempFilenameRen);
  OSD_File aFile(aPathTmp);
  aFile.Move(aTempFilenameRen);
  aFile.SetPath(aPathRen);

  // Recover temporary file to another Model
  Standard_Boolean isOk = theNewModel->Open(aTempFilenameRen);

  // Remove temporary file
  aFile.Remove();

  return isOk;
}

//! Prepares a name for temporary file to store intermediate Data Model
//! instance.
//! \param theDoc [in] OCAF Document.
//! \param isCopy [in] indicates whether some kind of copy marker should
//!        be used to distinguish the generated filename
//! \return requested filename.
TCollection_AsciiString
  ActData_CAFConversionCtx::temporaryFilename(const Handle(TDocStd_Document)& theDoc,
                                              const Standard_Boolean isCopy) const
{
  // Get string representation of current timestamp
  TCollection_AsciiString
    aTSName = ActAux_TimeStampTool::Generate()->ToString(Standard_True,
                                                          Standard_True);

  // ...
  // Prepare filename
  // ...

  TCollection_AsciiString aResult = m_dumpPath;

  // Add trailing slash if needed
  if ( aResult.Value( aResult.Length() ) != '\\' )
    aResult += CARD_SLASH;

  // Compose filename
  aResult += CARD_TMP;
  aResult += CARD_UNDERSCORE;
  aResult += aTSName;
  if ( isCopy ) aResult += CARD_UNDERSCORE;
  aResult += CARD_DOT;
  aResult += theDoc->FileExtension();

  return aResult;
}

//! Accessor for TDF Label which is root for USER section of the Data Node
//! addressed by the given PID.
//! \param theNID [in] ID of the Node.
//! \param theModel [in] Data Model instance.
//! \param canCreate [in] indicates whether this method is allowed to create
//!        USER section in case if it does not exist.
//! \return requested Label.
TDF_Label
  ActData_CAFConversionCtx::uSectionRoot(const ActAPI_DataObjectId& theNID,
                                         const Handle(ActAPI_IModel)& theModel,
                                         const Standard_Boolean canCreate) const
{
  Handle(ActData_BaseModel) aModelBase = Handle(ActData_BaseModel)::DownCast(theModel);

  TDF_Label aRoot;
  TDF_Tool::Label(aModelBase->Document()->GetData(), theNID, aRoot, Standard_False);

  if ( aRoot.IsNull() )
    return TDF_Label();

  return aRoot.FindChild(ActData_BaseNode::TagUser, canCreate);
}

//! Dumps model into ASCII file using CAF Dumper tool.
//! \param theModel [in] Data Model to dump.
void ActData_CAFConversionCtx::dumpModel(const Handle(ActAPI_IModel)& theModel) const
{
  Handle(ActData_BaseModel) aModelBase = Handle(ActData_BaseModel)::DownCast(theModel);

  TCollection_AsciiString aDumpFn( this->temporaryFilename( aModelBase->Document() ) );
  aDumpFn += CARD_DOT;
  aDumpFn += CARD_DUMP;

  theModel->FuncReleaseLogBook();
  ActData_CAFDumper::Dump(aDumpFn, theModel,
                          ActData_CAFDumper::Content_Plain,
                          ActData_CAFDumper::Verbosity_DetailsSkipUnstable);
}
