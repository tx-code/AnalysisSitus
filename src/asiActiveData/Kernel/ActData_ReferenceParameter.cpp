//-----------------------------------------------------------------------------
// Created on: May 2012
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
#include <ActData_ReferenceParameter.h>

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_NodeFactory.h>
#include <ActData_ParameterFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDF_Reference.hxx>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_ReferenceParameter::ActData_ReferenceParameter() : ActData_UserParameter()
{
}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(ActData_ReferenceParameter) ActData_ReferenceParameter::Instance()
{
  return new ActData_ReferenceParameter();
}

//! Sets the passed CAF Label as a target one.
//! \param theLabel [in] Label to refer to.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending [in] indicates whether this Parameter must lose its
//!        PENDING (or out-dated) property.
void ActData_ReferenceParameter::SetTarget(const TDF_Label& theLabel,
                                           const ActAPI_ModificationType theModType,
                                           const Standard_Boolean doResetValidity,
                                           const Standard_Boolean doResetPending)
{
  TDF_Label aReferenceLab = m_label.FindChild(DS_Ref);

  Handle(TDF_Reference) aRefAttr;
  if ( !aReferenceLab.FindAttribute(TDF_Reference::GetID(), aRefAttr) )
    aRefAttr = TDF_Reference::Set(aReferenceLab, theLabel);
  else
    aRefAttr->Set(theLabel);

  // Record modification in LogBook:
  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Sets the passed Data Cursor as a target one.
//! \param theDC [in] Data Cursor to refer to.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending [in] indicates whether this Parameter must lose its
//!        PENDING (or out-dated) property.
void ActData_ReferenceParameter::SetTarget(const Handle(ActAPI_IDataCursor)& theDC,
                                           const ActAPI_ModificationType theModType,
                                           const Standard_Boolean doResetValidity,
                                           const Standard_Boolean doResetPending)
{
  if ( theDC.IsNull() ) // Empty reference is simply not initialized
    return;

  this->SetTarget(theDC->RootLabel(), theModType, doResetValidity, doResetPending);
}

//! Accessor for the referenced Data Cursor.
//! \param theRefType [in] requested type of reference.
//! \return referenced Data Cursor.
Handle(ActAPI_IDataCursor)
  ActData_ReferenceParameter::GetTarget(const RefType theRefType) const
{
  TDF_Label aTargetLab = this->GetTargetLabel();

  if ( aTargetLab.IsNull() )
    return NULL;

  if ( theRefType == Ref_Parameter )
  {
    Standard_Boolean isUndefinedType;
    return ActData_ParameterFactory::NewParameterSettle(aTargetLab, isUndefinedType);
  }
  else if ( theRefType == Ref_Node )
  {
    if ( !ActData_NodeFactory::IsNode(aTargetLab) )
      return NULL;

    return ActData_NodeFactory::NodeSettle(aTargetLab);
  }

  return NULL;
}

//! Accessor for the referenced Data Cursor.
//! \return referenced Data Cursor.
Handle(ActAPI_IDataCursor) ActData_ReferenceParameter::GetTarget() const
{
  Handle(ActAPI_IDataCursor) aResult = this->GetTarget(Ref_Node);
  if ( !aResult.IsNull() )
    return aResult;

  return this->GetTarget(Ref_Parameter);
}

//! Accessor for the referenced TDF Label.
//! \return referenced OCAF Label.
TDF_Label ActData_ReferenceParameter::GetTargetLabel() const
{
  TDF_Label aReferenceLab = m_label.FindChild(DS_Ref, Standard_False);
  if ( aReferenceLab.IsNull() )
    return TDF_Label();

  Handle(TDF_Reference) aRefAttr;
  if ( !aReferenceLab.FindAttribute(TDF_Reference::GetID(), aRefAttr) )
    return TDF_Label();

  TDF_Label aTargetLab = aRefAttr->Get();
  return aTargetLab;
}

//! Checks whether the passed Data Cursor is referenced by this one.
//! \param theDC [in] Data Cursor to check.
//! \return true if the given Data Cursor is referenced, false -- otherwise.
Standard_Boolean
  ActData_ReferenceParameter::IsTarget(const Handle(ActAPI_IDataCursor)& theDC)
{
  Handle(ActAPI_IDataCursor) aRefDC = this->GetTarget();
  if ( aRefDC.IsNull() )
    return Standard_False;

  return ActAPI_IDataCursor::IsEqual(aRefDC, theDC);
}

//! Cleans up the reference.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending [in] indicates whether this Parameter must lose its
//!        PENDING (or out-dated) property.
void ActData_ReferenceParameter::RemoveTarget(const ActAPI_ModificationType theModType,
                                              const Standard_Boolean doResetValidity,
                                              const Standard_Boolean doResetPending)
{
  TDF_Label aReferenceLab = m_label.FindChild(DS_Ref, Standard_False);
  if ( aReferenceLab.IsNull() )
    return;

  Handle(TDF_Reference) aRefAttr;
  if ( !aReferenceLab.FindAttribute(TDF_Reference::GetID(), aRefAttr) )
    return;

  aReferenceLab.ForgetAttribute( TDF_Reference::GetID() );

  // Record modification in LogBook:
  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
Standard_Boolean ActData_ReferenceParameter::isWellFormed() const
{
  // No additional checks as there could be an empty reference
  return Standard_True;
}

//! Returns Parameter type.
//! \return Parameter type.
Standard_Integer ActData_ReferenceParameter::parameterType() const
{
  return Parameter_Reference;
}

//-----------------------------------------------------------------------------
// DTO construction
//-----------------------------------------------------------------------------

//! Populates Parameter from the passed DTO.
//! \param theDTO [in] DTO to source data from.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether validity flag must be
//!        reset or not.
//! \param doResetPending [in] indicates whether pending flag must be reset
//!        or not.
void ActData_ReferenceParameter::setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
                                            const ActAPI_ModificationType theModType,
                                            const Standard_Boolean doResetValidity,
                                            const Standard_Boolean doResetPending)
{
  Handle(ActData_ReferenceDTO) MyDTO = Handle(ActData_ReferenceDTO)::DownCast(theDTO);
  this->SetTarget(MyDTO->Target, theModType, doResetValidity, doResetPending);
}

//! Creates and populates DTO.
//! \param theGID [in] ready-to-use GID for DTO.
//! \return constructed DTO instance.
Handle(ActData_ParameterDTO)
  ActData_ReferenceParameter::createDTO(const ActAPI_ParameterGID& theGID)
{
  Handle(ActData_ReferenceDTO) aRes = new ActData_ReferenceDTO(theGID);
  aRes->Target = this->GetTarget();
  return aRes;
}
