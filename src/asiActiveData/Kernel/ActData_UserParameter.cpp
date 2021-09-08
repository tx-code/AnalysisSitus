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
#include <ActData_UserParameter.h>

// Active Data (API) includes
#include <ActAPI_INode.h>

// Active Data includes
#include <ActData_BaseModel.h>
#include <ActData_LogBook.h>
#include <ActData_NodeFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDataStd_Integer.hxx>
#include <TFunction_Scope.hxx>

#undef COUT_DEBUG

//! Default constructor. Creates a DETACHED Nodal Parameter.
ActData_UserParameter::ActData_UserParameter() : ActAPI_IUserParameter()
{
  m_status = SS_Detached;
}

//-----------------------------------------------------------------------------
// Data Cursor behavior
//-----------------------------------------------------------------------------

//! Returns true if this Parameter is ATTACHED to the CAF structure,
//! false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_UserParameter::IsAttached()
{
  return m_status == SS_Attached;
}

//! Returns true if this Parameter is DETACHED from the CAF structure,
//! false -- otherwise.
//! \return true/false.
Standard_Boolean ActData_UserParameter::IsDetached()
{
  return m_status == SS_Detached;
}

//! Checks the underlying CAF Structure.
//! \return true if everything is OK, false -- otherwise.
Standard_Boolean ActData_UserParameter::IsWellFormed()
{
  /* ========================================
   *  Check if the object is attached to CAF
   * ======================================== */

  if ( this->IsDetached() )
    return Standard_False;

  /* ======================
   *  Check Parameter type
   * ====================== */

  if ( !ActData_Utils::CheckLabelAttr( m_label, DS_ParamType,
                                       TDataStd_Integer::GetID() ) )
    return Standard_False;

  Standard_Integer paramType;
  if ( !ActData_Utils::GetIntegerValue(m_label, DS_ParamType, paramType) )
    return Standard_False;

  if ( paramType != this->parameterType() )
    return Standard_False;

  /* =========================================
   *  Check other basic mandatory data chunks
   * ========================================= */

  if ( !ActData_Utils::CheckLabelAttr( m_label, DS_IsValid,
                                       TDataStd_Integer::GetID() ) )
    return Standard_False;

  /* ============================
   *  Check client-specific data
   * ============================ */

  return this->isWellFormed();
}

//-----------------------------------------------------------------------------
// Accessors to the persistent properties
//-----------------------------------------------------------------------------

//! Accessor for the type of the Parameter.
//! \return type ID.
Standard_Integer ActData_UserParameter::GetParamType()
{
  return this->parameterType();
}

//! Sets evaluation string for the Parameter.
//! \param theEvalString [in] evaluation string to set.
void ActData_UserParameter::SetEvalString(const TCollection_AsciiString& theEvalString,
                                          const ActAPI_ModificationType theModType)
{
  ActData_Utils::SetAsciiStringValue(m_label, DS_EvalString, theEvalString);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
}

//! Accessor for the evaluation string associated with the Parameter.
//! \return evaluation string.
TCollection_AsciiString ActData_UserParameter::GetEvalString()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  return ActData_Utils::GetAsciiStringValue(m_label, DS_EvalString);
}

//! Sets the name for the Parameter.
//! \param theString [in] name to set.
void ActData_UserParameter::SetName(const TCollection_ExtendedString& theString,
                                    const ActAPI_ModificationType theModType)
{
  ActData_Utils::SetExtStringValue(m_label, DS_Name, theString);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
}

//! Accessor for the name of the Parameter.
//! \return name of the Parameter.
TCollection_ExtendedString ActData_UserParameter::GetName()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  return ActData_Utils::GetExtStringValue(m_label, DS_Name);
}

//! Returns the Data Node the Parameter is associated to.
//! \return Data Node instance.
Handle(ActAPI_INode) ActData_UserParameter::GetNode()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  /* ====================================================
   *  Access Node TDF Label & allocate the Node instance
   * ==================================================== */

  // Two levels up: check if we have reached META root
  TDF_Label aNodeLabel = m_label.Father().Father();
  //
  if ( !ActData_NodeFactory::IsNode(aNodeLabel) )
  {
    // NOTICE: User Parameters may be used in META section. If it happens,
    //         then Father() should be called three times
    Handle(TDataStd_TreeNode) TN;
    if ( aNodeLabel.FindAttribute(TDataStd_TreeNode::GetDefaultTreeID(), TN) )
      aNodeLabel = aNodeLabel.Father();
  }

  return ActData_NodeFactory::NodeSettle(aNodeLabel);
}

//! Extract ID of the Node by cutting out the trailing tags of the
//! Parameter's ID.
ActAPI_DataObjectId ActData_UserParameter::GetNodeId()
{
  return ActData_Common::NodeIdByParameterId( this->GetId() );
}

//! Sets client-specific flags in form of a single integer. Normally such
//! integers are treated as masks by the client code. E.g. it might be
//! necessary to bind some GUI information with each Parameter like
//! Visibility, Accessibility, Read-Only qualificators, etc. As all such
//! information is essentially application-specific, Active Data
//! does not try to guess all possible variations of these addendum flags.
//! Instead, it gives user a possibility to do it at the application level.
//! \param theUFlags [in] value to set.
//! \param theModType [in] modification type.
void ActData_UserParameter::SetUserFlags(const Standard_Integer theUFlags,
                                         const ActAPI_ModificationType theModType)
{
  ActData_Utils::SetIntegerValue(m_label, DS_UserFlags, theUFlags);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
}

//! Checks whether this Parameter has the passed user flags.
//! \param theUFlags [in] user flags to check.
//! \return true/false.
Standard_Boolean
  ActData_UserParameter::HasUserFlags(const Standard_Integer theUFlags)
{
  return (this->GetUserFlags() & theUFlags) > 0;
}

//! Adds the passed user flags to the existing ones.
//! \param theUFlags [in] flags to add.
//! \param theModType [in] modification type.
void ActData_UserParameter::AddUserFlags(const Standard_Integer theUFlags,
                                         const ActAPI_ModificationType theModType)
{
  if ( this->HasUserFlags(theUFlags) )
    return;

  Standard_Integer aNewFlags = this->GetUserFlags() | theUFlags;
  this->SetUserFlags(aNewFlags, theModType);
}

//! Removes the passed user flags from the existing ones.
//! \param theUFlags [in] flags to remove.
//! \param theModType [in] modification type.
void ActData_UserParameter::RemoveUserFlags(const Standard_Integer theUFlags,
                                            const ActAPI_ModificationType theModType)
{
  Standard_Integer aFlags = this->GetUserFlags();
  if ( this->HasUserFlags(theUFlags) )
    aFlags -= theUFlags;

  this->SetUserFlags(aFlags, theModType);
}

//! Accessor for application-specific flags packed into a single integer value.
//! \return application-specific flags.
Standard_Integer ActData_UserParameter::GetUserFlags()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  Standard_Integer aValue;
  if ( !ActData_Utils::GetIntegerValue(m_label, DS_UserFlags, aValue) )
    return 0;

  return aValue;
}

//! Sets the semantic ID associated with this Parameter.
//! \param theId [in] semantic ID to set.
void ActData_UserParameter::SetSemanticId(const TCollection_AsciiString& theId,
                                          const ActAPI_ModificationType theModType)
{
  ActData_Utils::SetAsciiStringValue(m_label, DS_SemanticId, theId);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
}

//! Accessor for the semantic ID associated with this Parameter.
//! \return requested semantic ID.
TCollection_AsciiString ActData_UserParameter::GetSemanticId()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  return ActData_Utils::GetAsciiStringValue(m_label, DS_SemanticId);
}

//! Sets the modification timestamp for the Parameter to the current one.
void ActData_UserParameter::SetModified()
{
  if ( ActData_BaseModel::MTime_On )
    ActData_Utils::SetTimeStampValue(m_label, DS_MTime);
}

//! Accessor for the modification timestamp associated with the Parameter.
//! \return modification timestamp.
Handle(ActAux_TimeStamp) ActData_UserParameter::GetMTime()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  return ActData_Utils::GetTimeStampValue(m_label, DS_MTime);
}

//! Returns validity flag associated with the Parameter.
//! \return data validity flag.
Standard_Boolean ActData_UserParameter::IsValidData()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  Standard_Integer aValue;
  if ( !ActData_Utils::GetIntegerValue(m_label, DS_IsValid, aValue) )
    return Standard_True;

  return (aValue > 0);
}

//! Sets data validity flag.
//! \param isValid [in] validity flag to set.
//! \param theModType [in] modification type.
void ActData_UserParameter::SetValidity(const Standard_Boolean isValid,
                                        const ActAPI_ModificationType theModType)
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  ActData_Utils::SetIntegerValue( m_label, DS_IsValid, (isValid ? 1 : 0) );

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
}

//! Returns PENDING flag associated with the Parameter.
//! \return PENDING flag.
Standard_Boolean ActData_UserParameter::IsPendingData()
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  Standard_Integer aValue;
  if ( !ActData_Utils::GetIntegerValue(m_label, DS_IsPending, aValue) )
    return Standard_False;

  return (aValue > 0);
}

//! Sets PENDING flag for data.
//! \param isPending [in] PENDING flag to set.
//! \param theModType [in] modification type.
void ActData_UserParameter::SetPending(const Standard_Boolean isPending,
                                       const ActAPI_ModificationType theModType)
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  ActData_Utils::SetIntegerValue( m_label, DS_IsPending, (isPending ? 1 : 0) );

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
}

ActAPI_DataObjectId ActData_UserParameter::GetId() const
{
  return ActData_Utils::GetEntry(m_label);
}

//-----------------------------------------------------------------------------
// Manipulating with DTO
//-----------------------------------------------------------------------------

//! Initializes Parameter with the passed DTO.
//! \param theDTO [in] data for initialization.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] flag to reset validity state.
//! \param doResetPending [in] flag to reset pending state.
void ActData_UserParameter::SetFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
                                       const ActAPI_ModificationType theModType,
                                       const Standard_Boolean doResetValidity,
                                       const Standard_Boolean doResetPending)
{
  TCollection_ExtendedString NewName      = theDTO->Name();
  TCollection_AsciiString    NewSID       = theDTO->SID();
  TCollection_AsciiString    NewEvalStr   = theDTO->EvalStr();
  Standard_Integer           NewUFlags    = theDTO->UFlags();
  Standard_Boolean           NewIsValid   = theDTO->IsValid();
  Standard_Boolean           NewIsPending = theDTO->IsPending();

  if ( this->GetName() != NewName )
    this->SetName(NewName);

  if ( this->GetSemanticId() != NewSID )
    this->SetSemanticId(NewSID);

  if ( this->GetEvalString() != NewEvalStr )
    this->SetEvalString(NewEvalStr);

  if ( this->GetUserFlags() != NewUFlags )
    this->SetUserFlags(NewUFlags);

  if ( !doResetValidity )
    if ( this->IsValidData() != NewIsValid )
      this->SetValidity(NewIsValid);

  if ( !doResetPending )
    if ( this->IsPendingData() != NewIsPending )
      this->SetPending(NewIsPending);

  // Ask descendant to do initialize custom data
  this->setFromDTO(theDTO, theModType, doResetValidity, doResetPending);
}

//! Returns Parameter value in generic form of DTO. This method queries
//! data from OCAF and packs it into pure transient structure which is detached
//! from OCAF. In order to query data you have to ensure that Parameter is
//! in ATTACHED and WELL-FORMED state.
//! \return DTO containing the stored data.
Handle(ActData_ParameterDTO) ActData_UserParameter::GetAsDTO()
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  // Prepare GID for DTO
  ActAPI_ParameterGID GID;
  GID.NID = ActData_Utils::GetEntry( m_label.Father().Father() );
  GID.PID = m_label.Tag();

  // Ask descendant to create DTO instance
  Handle(ActData_ParameterDTO) aResDTO = this->createDTO(GID);

  // Initialize basic props
  aResDTO->ChangeName()      = this->GetName();
  aResDTO->ChangeSID()       = this->GetSemanticId();
  aResDTO->ChangeEvalStr()   = this->GetEvalString();
  aResDTO->ChangeIsValid()   = this->IsValidData();
  aResDTO->ChangeUFlags()    = this->GetUserFlags();
  aResDTO->ChangeIsPending() = this->IsPendingData();

  return aResDTO;
}

//-----------------------------------------------------------------------------
// OCAF internals
//-----------------------------------------------------------------------------

//! Return root TDF Label of the Parameter.
//! \return root Label.
TDF_Label ActData_UserParameter::RootLabel() const
{
  return m_label;
}

//-----------------------------------------------------------------------------
// Modification support
//-----------------------------------------------------------------------------

//! Marks this Parameter as TOUCHED (affected by user) in the global
//! modification LogBook.
void ActData_UserParameter::SetTouched()
{
  // Prepare LogBook Data Cursor
  TDF_Label aLogBookSection =
    m_label.Root().FindChild(ActData_BaseModel::StructureTag_LogBook);

  ActData_LogBook(aLogBookSection).Touch(m_label);
}

//! Marks this Parameter as IMPACTED (affected by Tree Function mechanism) in
//! the global modification LogBook.
void ActData_UserParameter::SetImpacted()
{
  // Prepare LogBook Data Cursor
  TDF_Label aLogBookSection =
    m_label.Root().FindChild(ActData_BaseModel::StructureTag_LogBook);

  ActData_LogBook(aLogBookSection).Impact(m_label);
}

//-----------------------------------------------------------------------------
// Data Cursor behavior internals
//-----------------------------------------------------------------------------

//! Attaches the transient Data Cursor to the CAF Label.
//! \param theLabel [in] Label to attach the cursor to.
void ActData_UserParameter::attach(const TDF_Label& theLabel)
{
#if defined COUT_DEBUG
  if ( theLabel.IsNull() )
    std::cout << "WARN: settling down Parameter on NULL Label" << std::endl;
#endif

  m_status = SS_Attached;
  m_label = theLabel;
}

//! Expands the Parameter Cursor on the passed TDF Label.
//! \param theLabel [in] root TDF Label for the Parameter to expand on.
void ActData_UserParameter::expandOn(const TDF_Label& theLabel)
{
  // Attach transient Cursor properties to the CAF Label
  this->attach(theLabel);

  // Set Parameter type
  ActData_Utils::SetIntegerValue( m_label, DS_ParamType, this->parameterType() );

  // Set initial validity as TRUE
  ActData_Utils::SetIntegerValue( m_label, DS_IsValid, 1 );

  // Set initial pending as FALSE
  ActData_Utils::SetIntegerValue( m_label, DS_IsPending, 0 );

  // Start Parameter's modification time history
  this->SetModified();
}

//! Settles the Parameter Cursor on the passed TDF Label.
//! \param theLabel [in] root TDF Label for the Parameter to settle on.
void ActData_UserParameter::settleOn(const TDF_Label& theLabel)
{
  // Attach transient Cursor properties to the CAF Label
  this->attach(theLabel);
}
