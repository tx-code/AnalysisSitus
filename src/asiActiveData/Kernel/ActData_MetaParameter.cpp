//-----------------------------------------------------------------------------
// Created on: March 2016
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
#include <ActData_MetaParameter.h>

// Active Data includes
#include <ActData_NodeFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <TDataStd_AsciiString.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_ReferenceList.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_Tool.hxx>

//! Default constructor. Creates a DETACHED instance of Meta Parameter.
ActData_MetaParameter::ActData_MetaParameter() : ActAPI_IDataCursor()
{
  m_status = SS_Detached;

  // Initialize a collection for evaluation Tree Functions
  m_evaluators = new ActAPI_HIndexedParameterMap;
}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(ActData_MetaParameter) ActData_MetaParameter::Instance()
{
  return new ActData_MetaParameter();
}

//-----------------------------------------------------------------------------
// Data Cursor behavior
//-----------------------------------------------------------------------------

//! Returns true if this Parameter is ATTACHED to the CAF structure,
//! false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_MetaParameter::IsAttached()
{
  return m_status == SS_Attached;
}

//! Returns true if this Parameter is DETACHED from the CAF structure,
//! false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_MetaParameter::IsDetached()
{
  return m_status == SS_Detached;
}

//! Checks the underlying CAF Structure.
//! \return true if everything is OK, false -- otherwise.
Standard_Boolean ActData_MetaParameter::IsWellFormed()
{
  // Check if the object is attached to CAF
  if ( this->IsDetached() )
    return Standard_False;

  // Check if type of the owning Node is present
  if ( !ActData_Utils::CheckLabelAttr( m_label, -1,
                                       TDataStd_AsciiString::GetID() ) )
    return Standard_False;

  // Check if user flags are present
  if ( !ActData_Utils::CheckLabelAttr( m_label, -1,
                                       TDataStd_Integer::GetID() ) )
    return Standard_False;

  return Standard_True; // It seems we have passed the check
}

//! \return true always (since meta data is not involved into Tree Function
//!         execution mechanism).
Standard_Boolean ActData_MetaParameter::IsValidData()
{
  return Standard_True;
}

//! \return false always (since meta data is not involved into Tree Function
//!         execution mechanism).
Standard_Boolean ActData_MetaParameter::IsPendingData()
{
  return Standard_False;
}

//! \return ID of the Meta Parameter as entry of its root Label.
ActAPI_DataObjectId ActData_MetaParameter::GetId() const
{
  return ActData_Utils::GetEntry(m_label);
}

//! \return root Label of the Meta Parameter.
TDF_Label ActData_MetaParameter::RootLabel() const
{
  return m_label;
}

//-----------------------------------------------------------------------------
// Getters for the owner Node
//-----------------------------------------------------------------------------

//! Returns the Data Node the Parameter is associated to.
//! \return Data Node instance.
Handle(ActAPI_INode) ActData_MetaParameter::GetNode()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  /* ====================================================
   *  Access Node TDF Label & allocate the Node instance
   * ==================================================== */

  // Level up
  TDF_Label aNodeLabel = m_label.Father();

  return ActData_NodeFactory::NodeSettle(aNodeLabel);
}

//! Extract ID of the Node by cutting out the trailing tags of the
//! Parameter's ID.
ActAPI_DataObjectId ActData_MetaParameter::GetNodeId() const
{
  return ActData_Common::NodeIdByParameterId( this->GetId() );
}

//-----------------------------------------------------------------------------
// Getters & Setters [TYPE]
//-----------------------------------------------------------------------------

//! Sets type name.
//! \param theTypeName [in] type name to set.
void ActData_MetaParameter::SetTypeName(const TCollection_AsciiString& theTypeName)
{
  ActData_Utils::SetAsciiStringValue(m_label, -1, theTypeName);
}

//! \return Node's type name.
TCollection_AsciiString ActData_MetaParameter::GetTypeName() const
{
  return ActData_Utils::GetAsciiStringValue(m_label, -1);
}

//-----------------------------------------------------------------------------
// Getters & Setters [TREE NODE]
//-----------------------------------------------------------------------------

//! Appends one Node as a child to another. META Parameter of the child Node
//! is passed to access its internal Tree Node attribute.
//! \param theTreeNode [in] META playing as a Tree Node.
void ActData_MetaParameter::AppendChild(const Handle(ActData_MetaParameter)& theTreeNode)
{
  // Let this method recover possibly missing parent Tree Node attribute
  Handle(TDataStd_TreeNode) aParentTN = this->GetCAFTreeNode();
  if ( aParentTN.IsNull() )
  {
    aParentTN = TDataStd_TreeNode::Set( m_label,
                                        TDataStd_TreeNode::GetDefaultTreeID() );
  }

  // Let this method recover possibly missing child Tree Node attribute
  Handle(TDataStd_TreeNode) aChildTN = theTreeNode->GetCAFTreeNode();
  if ( aChildTN.IsNull() )
  {
    aChildTN = TDataStd_TreeNode::Set( theTreeNode->RootLabel(),
                                       TDataStd_TreeNode::GetDefaultTreeID() );
  }

  // Add child to parent
  ActData_Utils::AppendChild(aParentTN, aChildTN);
}

//! Attempts to remove child Node by its META Parameter.
//! \param theTreeNode [in] META playing as a Tree Node.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MetaParameter::RemoveChild(const Handle(ActData_MetaParameter)& theTreeNode)
{
  return ActData_Utils::RemoveChild( this->GetCAFTreeNode(),
                                     theTreeNode->GetCAFTreeNode() );
}

//! \return Tree Node attribute.
Handle(TDataStd_TreeNode) ActData_MetaParameter::GetCAFTreeNode()
{
  return ActData_Utils::AccessTreeNode(m_label, Standard_False);
}

//-----------------------------------------------------------------------------
// Getters & Setters [USER FLAGS]
//-----------------------------------------------------------------------------

//! Sets user flags.
//! \param theUserFlags [in] user flags to set.
void ActData_MetaParameter::SetUserFlags(const Standard_Integer theUserFlags)
{
  ActData_Utils::SetIntegerValue(m_label, -1, theUserFlags);
}

//! \return user flags.
Standard_Integer ActData_MetaParameter::GetUserFlags() const
{
  Standard_Integer result;
  if ( !ActData_Utils::GetIntegerValue(m_label, -1, result) )
    Standard_ProgramError::Raise("No attribute available for user flags");

  return result;
}

//-----------------------------------------------------------------------------
// Getters & Setters [REFERENCES]
//-----------------------------------------------------------------------------

//! \return input reader cursors.
Handle(ActAPI_HDataCursorList) ActData_MetaParameter::GetInputReaderCursors()
{
  return ActData_Utils::ConvertToCursors( this->GetInputReaders() );
}

//! \return output writer cursors.
Handle(ActAPI_HDataCursorList) ActData_MetaParameter::GetOutputWriterCursors()
{
  return ActData_Utils::ConvertToCursors( this->GetOutputWriters() );
}

//! \return referrer cursors.
Handle(ActAPI_HDataCursorList) ActData_MetaParameter::GetReferrerCursors()
{
  return ActData_Utils::ConvertToCursors( this->GetReferrers() );
}

//! \return input reader Labels.
TDF_LabelList ActData_MetaParameter::GetInputReaders() const
{
  Handle(TDataStd_ReferenceList) ref = this->GetInputReadersAttr();
  if ( ref.IsNull() )
    return TDF_LabelList();

  return ref->List();
}

//! \return output writer Labels.
TDF_LabelList ActData_MetaParameter::GetOutputWriters() const
{
  Handle(TDataStd_ReferenceList) ref = this->GetOutputWritersAttr();
  if ( ref.IsNull() )
    return TDF_LabelList();

  return ref->List();
}

//! \return referrer Labels.
TDF_LabelList ActData_MetaParameter::GetReferrers() const
{
  Handle(TDataStd_ReferenceList) ref = this->GetReferrersAttr();
  if ( ref.IsNull() )
    return TDF_LabelList();

  return ref->List();
}

//! \return input readers attribute.
Handle(TDataStd_ReferenceList) ActData_MetaParameter::GetInputReadersAttr() const
{
  return ActData_Utils::GetReferenceList(m_label, DS_InputReaders);
}

//! \return output writers attribute.
Handle(TDataStd_ReferenceList) ActData_MetaParameter::GetOutputWritersAttr() const
{
  return ActData_Utils::GetReferenceList(m_label, DS_OutputWriters);
}

//! \return referrers attribute.
Handle(TDataStd_ReferenceList) ActData_MetaParameter::GetReferrersAttr() const
{
  return ActData_Utils::GetReferenceList(m_label, DS_Referrers);
}

//! Checks whether the given Label is contained in the list of input readers.
//! \param theLab [in] OCAF Label to check.
//! \return true/false.
Standard_Boolean ActData_MetaParameter::HasInputReader(const TDF_Label& theLab) const
{
  return ActData_Utils::HasTarget(this->GetInputReaders(), theLab) > 0;
}

//! Checks whether the given Label is contained in the list of output writers.
//! \param theLab [in] OCAF Label to check.
//! \return true/false.
Standard_Boolean ActData_MetaParameter::HasOutputWriter(const TDF_Label& theLab) const
{
  return ActData_Utils::HasTarget(this->GetOutputWriters(), theLab) > 0;
}

//! Checks whether the given Label is contained in the list of referrers.
//! \param theLab [in] OCAF Label to check.
//! \return true/false.
Standard_Boolean ActData_MetaParameter::HasReferrer(const TDF_Label& theLab) const
{
  return ActData_Utils::HasTarget(this->GetReferrers(), theLab) > 0;
}

//! Adds the given OCAF Label to the beginning of the list of input readers.
//! \param theLab [in] OCAF Label to add.
void ActData_MetaParameter::PrependInputReader(const TDF_Label& theLab)
{
  ActData_Utils::PrependReference(m_label, DS_InputReaders, theLab);
}

//! Adds the given OCAF Label to the beginning of the list of output writers.
//! \param theLab [in] OCAF Label to add.
void ActData_MetaParameter::PrependOutputWriter(const TDF_Label& theLab)
{
  ActData_Utils::PrependReference(m_label, DS_OutputWriters, theLab);
}

//! Adds the given OCAF Label to the beginning of the list of referrers.
//! \param theLab [in] OCAF Label to add.
void ActData_MetaParameter::PrependReferrer(const TDF_Label& theLab)
{
  ActData_Utils::PrependReference(m_label, DS_Referrers, theLab);
}

//! Appends the given OCAF Label to the tail of the list of input readers.
//! \param theLab [in] OCAF Label to add.
void ActData_MetaParameter::AppendInputReader(const TDF_Label& theLab)
{
  ActData_Utils::AppendReference(m_label, DS_InputReaders, theLab);
}

//! Appends the given OCAF Label to the tail of the list of output writers.
//! \param theLab [in] OCAF Label to add.
void ActData_MetaParameter::AppendOutputWriter(const TDF_Label& theLab)
{
  ActData_Utils::AppendReference(m_label, DS_OutputWriters, theLab);
}

//! Appends the given OCAF Label to the tail of the list of referrers
//! \param theLab [in] OCAF Label to add.
void ActData_MetaParameter::AppendReferrer(const TDF_Label& theLab)
{
  ActData_Utils::AppendReference(m_label, DS_Referrers, theLab);
}

//! Removes the given OCAF Label from the list of input readers.
//! \param theLab [in] OCAF Label to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MetaParameter::RemoveInputReader(const TDF_Label& theLab)
{
  Handle(TDataStd_ReferenceList) aRefList = this->GetInputReadersAttr();
  if ( aRefList.IsNull() )
    return Standard_False;
  //
  return aRefList->Remove(theLab);
}

//! Removes the given OCAF Label from the list of output writers.
//! \param theLab [in] OCAF Label to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MetaParameter::RemoveOutputWriter(const TDF_Label& theLab)
{
  Handle(TDataStd_ReferenceList) aRefList = this->GetOutputWritersAttr();
  if ( aRefList.IsNull() )
    return Standard_False;
  //
  return aRefList->Remove(theLab);
}

//! Removes the given OCAF Label from the list of referrers.
//! \param theLab [in] OCAF Label to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MetaParameter::RemoveReferrer(const TDF_Label& theLab)
{
  Handle(TDataStd_ReferenceList) aRefList = this->GetReferrersAttr();
  if ( aRefList.IsNull() )
    return Standard_False;
  //
  return aRefList->Remove(theLab);
}

//-----------------------------------------------------------------------------
// Manipulating with DTO
//-----------------------------------------------------------------------------

//! Initializes Parameter with the passed DTO.
//! \param theDTO [in] data for initialization.
void ActData_MetaParameter::SetFromDTO(const Handle(ActData_MetaDTO)& theDTO)
{
  // Set type name
  this->SetTypeName(theDTO->TypeName);

  // Set parent
  TDF_Label aParentLab;
  TDF_Tool::Label(m_label.Data(), theDTO->TreeNodeParent, aParentLab);
  //
  if ( !aParentLab.IsNull() && ActData_NodeFactory::IsNode(aParentLab) )
  {
    // Access parent Data Node
    Handle(ActData_BaseNode)
      aParentNode = Handle(ActData_BaseNode)::DownCast( ActData_NodeFactory::NodeSettle(aParentLab) );

    // Add this as child
    aParentNode->m_paramScope.Meta->AppendChild(this);
  }

  // Set user flags
  this->SetUserFlags(theDTO->UserFlags);

  // Set input readers
  for ( Standard_Integer i = 1; i <= theDTO->InputReaders->Length(); ++i )
    this->AppendInputReader( theDTO->InputReaders->Value(i)->RootLabel() );

  // Set output writers
  for ( Standard_Integer i = 1; i <= theDTO->OutputWriters->Length(); ++i )
    this->AppendOutputWriter( theDTO->OutputWriters->Value(i)->RootLabel() );

  // Set referrers
  for ( Standard_Integer i = 1; i <= theDTO->Referrers->Length(); ++i )
    this->AppendReferrer( theDTO->Referrers->Value(i)->RootLabel() );
}

//! Returns Parameter value in generic form of DTO. This method queries
//! data from OCAF and packs it into pure transient structure which is detached
//! from OCAF. In order to query data you have to ensure that Parameter is
//! in ATTACHED and WELL-FORMED state.
//! \return DTO containing the stored data.
Handle(ActData_MetaDTO) ActData_MetaParameter::GetAsDTO()
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  // Create DTO instance
  Handle(ActData_MetaDTO) aResDTO = new ActData_MetaDTO;

  // Set type name
  aResDTO->TypeName = this->GetTypeName();

  // Set parent
  Handle(TDataStd_TreeNode) aChildTN  = this->GetCAFTreeNode();
  Handle(TDataStd_TreeNode) aParentTN = aChildTN->Father();
  //
  Handle(ActAPI_INode) aParentNode;
  if ( !aParentTN.IsNull() )
    aParentNode = ActData_NodeFactory::NodeSettle( aParentTN->Label().Father() );
  //
  aResDTO->TreeNodeParent = aParentNode->GetId();

  // Set user flags
  aResDTO->UserFlags = this->GetUserFlags();

  // Set references
  aResDTO->InputReaders  = this->GetInputReaderCursors();
  aResDTO->OutputWriters = this->GetOutputWriterCursors();
  aResDTO->Referrers     = this->GetReferrerCursors();

  return aResDTO;
}

//-----------------------------------------------------------------------------
// Data Cursor behavior internals
//-----------------------------------------------------------------------------

//! Attaches the transient Data Cursor to the CAF Label.
//! \param theLabel [in] Label to attach the cursor to.
void ActData_MetaParameter::attach(const TDF_Label& theLabel)
{
#if defined COUT_DEBUG
  if ( theLabel.IsNull() )
    std::cout << "WARN: settling down META Parameter on NULL Label" << std::endl;
#endif

  m_status = SS_Attached;
  m_label  = theLabel;
}

//! Expands the Parameter Cursor on the passed TDF Label.
//! \param theLabel [in] root TDF Label for the Parameter to expand on.
void ActData_MetaParameter::expandOn(const TDF_Label& theLabel)
{
  // Attach transient Cursor properties to the CAF Label
  this->attach(theLabel);

  // Create attributes
  TDataStd_AsciiString ::Set ( theLabel, "" );
  TDataStd_TreeNode    ::Set ( theLabel, TDataStd_TreeNode::GetDefaultTreeID() );
  TDataStd_Integer     ::Set ( theLabel, 0 );

  // Create sub-Labels for back-references
  TDataStd_ReferenceList::Set( m_label.FindChild(DS_InputReaders) );
  TDataStd_ReferenceList::Set( m_label.FindChild(DS_OutputWriters) );
  TDataStd_ReferenceList::Set( m_label.FindChild(DS_Referrers) );

  // Expand Tree Function Parameters for evaluators
  for ( auto pit = m_evaluators->cbegin(); pit != m_evaluators->cend(); ++pit )
  {
    const Handle(ActData_UserParameter)&
      aBaseParam = Handle(ActData_UserParameter)::DownCast(pit->second);
    //
    Standard_Integer aNewTag = pit->first;

    // Allow construction of sub-Labels in EXPANDING mode ONLY
    TDF_Label aParamLabRoot = m_label.FindChild(DS_Evaluators, Standard_True);
    TDF_Label aParamLab     = aParamLabRoot.FindChild(aNewTag, Standard_True);

    // Expand Tree Function Parameter
    aBaseParam->expandOn(aParamLab);
  }
}

//! Settles the Parameter Cursor on the passed TDF Label.
//! \param theLabel [in] root TDF Label for the Parameter to settle on.
void ActData_MetaParameter::settleOn(const TDF_Label& theLabel)
{
  // Attach transient Cursor properties to the CAF Label
  this->attach(theLabel);

  // Settle Tree Function Parameters for evaluators
  for ( auto pit = m_evaluators->cbegin(); pit != m_evaluators->cend(); ++pit )
  {
     const Handle(ActData_UserParameter)&
      aBaseParam = Handle(ActData_UserParameter)::DownCast(pit->second);
    //
    Standard_Integer aNewTag = pit->first;

    // Allow construction of sub-Labels in EXPANDING mode ONLY
    TDF_Label aParamLabRoot = m_label.FindChild(DS_Evaluators, Standard_False);
    TDF_Label aParamLab     = aParamLabRoot.FindChild(aNewTag, Standard_False);

    // Settle down Tree Function Parameter
    aBaseParam->settleOn(aParamLab);
  }
}
