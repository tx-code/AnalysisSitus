//-----------------------------------------------------------------------------
// Created on: April 2012
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
#include <ActData_ReferenceListParameter.h>

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_ParameterFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDF_LabelList.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Hidden constructor.
ActData_ReferenceListParameter::ActData_ReferenceListParameter() : ActData_UserParameter()
{}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(ActData_ReferenceListParameter) ActData_ReferenceListParameter::Instance()
{
  return new ActData_ReferenceListParameter();
}

//! Adds the passed target to the reference list.
//! \param theTarget       [in] target to add.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::AddTarget(const Handle(ActAPI_IDataCursor)& theTarget,
                                               const ActAPI_ModificationType     theModType,
                                               const Standard_Boolean            doResetValidity,
                                               const Standard_Boolean            doResetPending)
{
  this->AddTarget(theTarget->RootLabel(), theModType,
                  doResetValidity, doResetPending);
}

//! Adds the passed target to the reference list.
//! \param theTargetLab    [in] target Label to add.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::AddTarget(const TDF_Label&              theTargetLab,
                                               const ActAPI_ModificationType theModType,
                                               const Standard_Boolean        doResetValidity,
                                               const Standard_Boolean        doResetPending)
{
  ActData_Utils::AppendReference(m_label, DS_Targets, theTargetLab);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Adds the passed target to the beginning of the reference list.
//! \param theTarget       [in] target to prepend.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::PrependTarget(const Handle(ActAPI_IDataCursor)& theTarget,
                                                   const ActAPI_ModificationType     theModType,
                                                   const Standard_Boolean            doResetValidity,
                                                   const Standard_Boolean            doResetPending)
{
  this->PrependTarget(theTarget->RootLabel(), theModType,
                      doResetValidity, doResetPending);
}

//! Adds the passed target to the beginning of the reference list.
//! \param theTargetLab    [in] target Label to prepend.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::PrependTarget(const TDF_Label&              theTargetLab,
                                                   const ActAPI_ModificationType theModType,
                                                   const Standard_Boolean        doResetValidity,
                                                   const Standard_Boolean        doResetPending)
{
  ActData_Utils::PrependReference(m_label, DS_Targets, theTargetLab);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Inserts the passed target into the reference list after the given
//! position.
//! \param theIndex        [in] index to add the given target after.
//! \param theTarget       [in] target to insert.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::InsertTargetAfter(const Standard_Integer            theIndex,
                                                       const Handle(ActAPI_IDataCursor)& theTarget,
                                                       const ActAPI_ModificationType     theModType,
                                                       const Standard_Boolean            doResetValidity,
                                                       const Standard_Boolean            doResetPending)
{
  this->InsertTargetAfter(theIndex, theTarget->RootLabel(), theModType,
                          doResetValidity, doResetPending);
}

//! Inserts the passed target into the reference list after the given
//! position.
//! \param theIndex        [in] index to add the given target after.
//! \param theTargetLab    [in] target Label to insert.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::InsertTargetAfter(const Standard_Integer        theIndex,
                                                       const TDF_Label&              theTargetLab,
                                                       const ActAPI_ModificationType theModType,
                                                       const Standard_Boolean        doResetValidity,
                                                       const Standard_Boolean        doResetPending)
{
  TDF_LabelList aLabelList;
  this->getTargets(aLabelList);
  //
  Standard_Integer aCurrentIndex = 1;
  TDF_Label aCurrentLab;
  for ( TDF_ListIteratorOfLabelList anIt(aLabelList); anIt.More(); anIt.Next() )
  {
    if ( aCurrentIndex == theIndex )
    {
      aCurrentLab = anIt.Value();
      break;
    }
    else
      ++aCurrentIndex;
  }

  if ( aCurrentLab.IsNull() )
    Standard_ProgramError::Raise("No Label found with such index");

  Handle(TDataStd_ReferenceList) aRefList = this->AccessReferenceList();
  aRefList->InsertAfter(theTargetLab, aCurrentLab);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Checks whether the passed target is referenced by the underlying list
//! of Parameters.
//! \param theTarget [in] Data Cursor to check.
//! \return index of the target or 0 if nothing was found.
Standard_Integer
  ActData_ReferenceListParameter::HasTarget(const Handle(ActAPI_IDataCursor)& theTarget)
{
  return this->HasTarget( theTarget->RootLabel() );
}

//! Checks whether the passed target is referenced by the underlying list
//! of Parameters.
//! \param theTargetLab [in] target Label to check.
//! \return index of the target or 0 if nothing was found.
Standard_Integer
  ActData_ReferenceListParameter::HasTarget(const TDF_Label& theTargetLab)
{
  TDF_LabelList aLabelList;
  this->getTargets(aLabelList);
  //
  return ActData_Utils::HasTarget(aLabelList, theTargetLab);
}

//! Removes all occurrences of the passed target from the internal collection.
//! Does nothing if such target happens to be not found.
//! \param theTarget       [in] target Data Cursor to remove.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::RemoveTargetOccurrences(const Handle(ActAPI_IDataCursor)& theTarget,
                                                          const ActAPI_ModificationType     theModType,
                                                          const Standard_Boolean            doResetValidity,
                                                          const Standard_Boolean            doResetPending)
{
  Standard_Boolean isDone,
                   isAnyDone = Standard_False;
  do
  {
    isDone = this->removeTarget( theTarget->RootLabel() );
    if ( isDone && !isAnyDone )
      isAnyDone = Standard_True;
  }
  while ( isDone );

  if ( isAnyDone )
  {
    // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
    SPRING_INTO_FUNCTION(theModType)
    // Reset Parameter's validity flag if requested
    RESET_VALIDITY(doResetValidity)
    // Reset Parameter's PENDING property
    RESET_PENDING(doResetPending);
  }

  return isAnyDone;
}

//! Removes the passed target from the internal collection. Does nothing
//! if such target happens to be not found.
//! \param theTarget       [in] target Data Cursor to remove.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::RemoveTarget(const Handle(ActAPI_IDataCursor)& theTarget,
                                               const ActAPI_ModificationType     theModType,
                                               const Standard_Boolean            doResetValidity,
                                               const Standard_Boolean            doResetPending)
{
  Standard_Boolean isDone = this->removeTarget( theTarget->RootLabel() );
  //
  if ( isDone )
  {
    // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
    SPRING_INTO_FUNCTION(theModType)
    // Reset Parameter's validity flag if requested
    RESET_VALIDITY(doResetValidity)
    // Reset Parameter's PENDING property
    RESET_PENDING(doResetPending);
  }
  //
  return isDone;
}

//! Removes the target with the given index from the internal collection. Does
//! nothing if such target happens to be not found.
//! \param theTargetIndex  [in] index of the target to remove.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::RemoveTarget(const Standard_Integer        theTargetIndex,
                                               const ActAPI_ModificationType theModType,
                                               const Standard_Boolean        doResetValidity,
                                               const Standard_Boolean        doResetPending)
{
  // Get list of target Labels
  TDF_LabelList aTargetLabs;
  this->getTargets(aTargetLabs);

  // Fins Label with the given index
  TDF_Label aTargetLab;
  Standard_Integer aCurrentIndex = 1;
  for ( TDF_ListIteratorOfLabelList it(aTargetLabs); it.More(); it.Next() )
  {
    if ( aCurrentIndex == theTargetIndex )
    {
      aTargetLab = it.Value();
      break;
    }
    else
      ++aCurrentIndex;
  }

  // Remove target if any
  Standard_Boolean isDone = ( aTargetLab.IsNull() ? Standard_False : this->removeTarget(aTargetLab) );
  //
  if ( isDone )
  {
    // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
    SPRING_INTO_FUNCTION(theModType)
    // Reset Parameter's validity flag if requested
    RESET_VALIDITY(doResetValidity)
    // Reset Parameter's PENDING property
    RESET_PENDING(doResetPending);
  }
  //
  return isDone;
}

//! Removes all targets.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::RemoveTargets(const ActAPI_ModificationType theModType,
                                                const Standard_Boolean        doResetValidity,
                                                const Standard_Boolean        doResetPending)
{
  Handle(TDataStd_ReferenceList) aRefs = this->AccessReferenceList();
  //
  Standard_Boolean isDone;
  if ( aRefs.IsNull() || aRefs->IsEmpty() )
    isDone = Standard_False;
  else
  {
    isDone = Standard_True;
    aRefs->Clear();
  }
  //
  if ( isDone )
  {
    // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
    SPRING_INTO_FUNCTION(theModType)
    // Reset Parameter's validity flag if requested
    RESET_VALIDITY(doResetValidity)
    // Reset Parameter's PENDING property
    RESET_PENDING(doResetPending);
  }
  //
  return isDone;
}

//! Accessor for a single target addressed by the passed index.
//! \param theIndex [in] 1-based index of the target to access.
//! \return requested target.
Handle(ActAPI_IDataCursor)
  ActData_ReferenceListParameter::GetTarget(const Standard_Integer theIndex) const
{
  TDF_Label aTargetLab = this->GetTargetLabel(theIndex);

  if ( !aTargetLab.IsNull() )
  {
    if ( ActData_ParameterFactory::IsUserParameter(aTargetLab) )
    {
      Standard_Boolean isUndefinedType;
      return ActData_ParameterFactory::NewParameterSettle(aTargetLab, isUndefinedType);
    }
    else if ( ActData_NodeFactory::IsNode(aTargetLab) )
    {
      return ActData_NodeFactory::NodeSettle(aTargetLab);
    }
  }

  return NULL;
}

//! Accessor for a single target Label addressed by the passed index.
//! \param theIndex [in] 1-based index of the target Label to access.
//! \return requested target Label.
TDF_Label
  ActData_ReferenceListParameter::GetTargetLabel(const Standard_Integer theIndex) const
{
  TDF_LabelList aLabelList;
  this->getTargets(aLabelList);
  Standard_Integer aTargetIdx = 0;
  for ( TDF_ListIteratorOfLabelList anIt(aLabelList); anIt.More(); anIt.Next() )
  {
    aTargetIdx++;
    TDF_Label& aLab = anIt.Value();
    if ( aTargetIdx == theIndex )
      return aLab;
  }

  return TDF_Label();
}

//! Returns the referenced Data Cursors.
//! \return referenced targets.
Handle(ActAPI_HDataCursorList) ActData_ReferenceListParameter::GetTargets() const
{
  TDF_LabelList aLabelList;
  this->getTargets(aLabelList);
  //
  return ActData_Utils::ConvertToCursors(aLabelList);
}

//! Sets new collection of targets to the Parameter. Old ones (if any)
//! are lost.
//! \param theTargets      [in] new targets to set.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void ActData_ReferenceListParameter::SetTargets(const Handle(ActAPI_HDataCursorList)& theTargets,
                                                const ActAPI_ModificationType         theModType,
                                                const Standard_Boolean                doResetValidity,
                                                const Standard_Boolean                doResetPending)
{
  Handle(TDataStd_ReferenceList) aRefList = this->AccessReferenceList();
  if ( !aRefList.IsNull() )
    aRefList->Clear();

  for ( ActAPI_DataCursorList::Iterator it( *theTargets.operator->() ); it.More(); it.Next() )
    this->AddTarget(it.Value(), MT_Silent, Standard_False);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Removes the old target (passed as the first argument) and inserts the new
//! one (passed as the second argument) to its place.
//! \param theTargetOld    [in] target DC to remove.
//! \param theTargetNew    [in] target DC to insert instead of the old one.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::ExchangeTarget(const Handle(ActAPI_IDataCursor)& theTargetOld,
                                                 const Handle(ActAPI_IDataCursor)& theTargetNew,
                                                 const ActAPI_ModificationType     theModType,
                                                 const Standard_Boolean            doResetValidity,
                                                 const Standard_Boolean            doResetPending)
{
  return this->ExchangeTarget(theTargetOld->RootLabel(), theTargetNew->RootLabel(),
                              theModType, doResetValidity, doResetPending);
}

//! Removes the old target (passed as the first argument) and inserts the new
//! one (passed as the second argument) to its place.
//! \param theTargetOldLab [in] target Label to remove.
//! \param theTargetNewLab [in] target Label to insert instead of the old one.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::ExchangeTarget(const TDF_Label&              theTargetOldLab,
                                                 const TDF_Label&              theTargetNewLab,
                                                 const ActAPI_ModificationType theModType,
                                                 const Standard_Boolean        doResetValidity,
                                                 const Standard_Boolean        doResetPending)
{
  Standard_Integer aTargetIdx = this->HasTarget(theTargetOldLab);
  if ( !aTargetIdx )
    return Standard_False;

  if ( !this->RemoveTarget(aTargetIdx, MT_Silent, Standard_False) )
    return Standard_False;

  if ( aTargetIdx == 1 )
    this->PrependTarget(theTargetNewLab, MT_Silent, Standard_False);
  else
    this->InsertTargetAfter(aTargetIdx - 1, theTargetNewLab, MT_Silent, Standard_False);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);

  return Standard_True;
}

//! Swap two targets with the given indexes in the internal collection. Does
//! nothing if such target happens to be not found.
//! \param theFirstIndex   [in] first target index.
//! \param theSecondIndex  [in] second target index.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::SwapTargets(const Standard_Integer        theFirstIndex,
                                              const Standard_Integer        theSecondIndex,
                                              const ActAPI_ModificationType theModType,
                                              const Standard_Boolean        doResetValidity,
                                              const Standard_Boolean        doResetPending)
{
  if ( !this->IsWellFormed() )
    return Standard_False;

  TDF_Label aFirstLabel  = this->GetTargetLabel(theFirstIndex);
  TDF_Label aSecondLabel = this->GetTargetLabel(theSecondIndex);

  if ( aFirstLabel.IsNull() || aSecondLabel.IsNull() )
    return Standard_False;

  // Redirect first label
  if ( !this->removeTarget(aFirstLabel) )
    return Standard_False;

  if ( theSecondIndex == 1 )
    this->PrependTarget(aFirstLabel, MT_Silent, Standard_False);
  else
    this->InsertTargetAfter(theSecondIndex - 1, aFirstLabel, MT_Silent, Standard_False);

  // Redirect second label
  if ( !this->removeTarget(aSecondLabel) )
    return Standard_False;

  if ( theFirstIndex == 1 )
    this->PrependTarget(aSecondLabel, MT_Silent, Standard_False);
  else
    this->InsertTargetAfter(theFirstIndex - 1, aSecondLabel, MT_Silent, Standard_False);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);

  return Standard_True;
}

//! Returns the number of registered targets.
//! \return number of targets.
Standard_Integer ActData_ReferenceListParameter::NbTargets()
{
  Handle(TDataStd_ReferenceList) aRefListAttr = this->AccessReferenceList();
  return aRefListAttr.IsNull() ? 0 : aRefListAttr->Extent();
}

//! Accessor for the internal OCAF Reference List Attribute.
//! \return Reference List Attribute.
Handle(TDataStd_ReferenceList)
  ActData_ReferenceListParameter::AccessReferenceList() const
{
  return ActData_Utils::GetReferenceList(m_label, DS_Targets);
}

//! Internal method returning the list of TDF Labels referenced by the
//! underlying list.
//! \param theLabelList [out] list of referenced TDF Labels.
void ActData_ReferenceListParameter::getTargets(TDF_LabelList& theLabelList) const
{
  Handle(TDataStd_ReferenceList) aRefList = this->AccessReferenceList();
  if ( !aRefList.IsNull() )
    theLabelList = aRefList->List();
}

//! Removes the referenced target from the collection.
//! \param theTargetLab [in] root Label of the target to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_ReferenceListParameter::removeTarget(const TDF_Label& theTargetLab)
{
  Handle(TDataStd_ReferenceList) aRefList = this->AccessReferenceList();
  if ( aRefList.IsNull() )
    return Standard_False;

  return aRefList->Remove(theTargetLab);
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
Standard_Boolean ActData_ReferenceListParameter::isWellFormed() const
{
  // No additional checks as there could be an empty list of references
  return Standard_True;
}

//! Returns Parameter type.
//! \return Parameter type.
Standard_Integer ActData_ReferenceListParameter::parameterType() const
{
  return Parameter_ReferenceList;
}

//-----------------------------------------------------------------------------
// DTO construction
//-----------------------------------------------------------------------------

//! Populates Parameter from the passed DTO.
//! \param theDTO          [in] DTO to source data from.
//! \param theModType      [in] modification type.
//! \param doResetValidity [in] indicates whether validity flag must be
//!                             reset or not.
//! \param doResetPending  [in] indicates whether pending flag must be reset
//!                             or not.
void ActData_ReferenceListParameter::setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
                                                const ActAPI_ModificationType       theModType,
                                                const Standard_Boolean              doResetValidity,
                                                const Standard_Boolean              doResetPending)
{
  Handle(ActData_ReferenceListDTO) MyDTO = Handle(ActData_ReferenceListDTO)::DownCast(theDTO);
  if ( MyDTO->Targets.IsNull() || MyDTO->Targets->IsEmpty() )
    return;

  for ( Standard_Integer i = 1; i <= MyDTO->Targets->Length(); ++i )
    this->AddTarget(MyDTO->Targets->Value(i), theModType, doResetValidity, doResetPending);
}

//! Creates and populates DTO.
//! \param theGID [in] ready-to-use GID for DTO.
//! \return constructed DTO instance.
Handle(ActData_ParameterDTO)
  ActData_ReferenceListParameter::createDTO(const ActAPI_ParameterGID& theGID)
{
  Handle(ActData_ReferenceListDTO) aRes = new ActData_ReferenceListDTO(theGID);

  Standard_Integer aNbTargets = this->NbTargets();
  if ( aNbTargets > 0 )
  {
    aRes->Targets = new ActAPI_HDataCursorList;
    for ( Standard_Integer i = 1; i <= aNbTargets; ++i )
      aRes->Targets->Append( this->GetTarget(i) );
  }
  return aRes;
}
