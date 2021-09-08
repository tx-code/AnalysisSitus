//-----------------------------------------------------------------------------
// Created on: March 2012
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
#include <ActData_StringArrayParameter.h>

// Active Data includes
#include <ActData_Utils.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_ExtStringArray.hxx>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_StringArrayParameter::ActData_StringArrayParameter() : ActData_UserParameter()
{
}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(ActData_StringArrayParameter) ActData_StringArrayParameter::Instance()
{
  return new ActData_StringArrayParameter();
}

//! Initializes Parameter with empty array. Actually, this means that the
//! internal OCAF structures are prepared as usual. However,  no data is
//! recorded there (array is just NULL), so the Attribute is kept dummy.
void ActData_StringArrayParameter::InitEmpty()
{
  // Clean up array
  ActData_Utils::ChooseLabelByTag(m_label, DS_Array, Standard_True).ForgetAllAttributes();

  // Store Array dimension
  ActData_Utils::SetIntegerValue(m_label, DS_ElemNum, 0);
}

//! Back-ups the underlying array data for transactional usage.
void ActData_StringArrayParameter::BackupArray()
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  ActData_Utils::BackupStringArray(m_label, DS_Array);
}

//! Sets array element.
//! \param theIndex [in] index of the element to set.
//! \param theValue [in] string to set.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending [in] indicates whether this Parameter must lose its
//!        PENDING (or out-dated) property.
void ActData_StringArrayParameter::SetElement(const Standard_Integer theIndex,
                                              const TCollection_ExtendedString& theValue,
                                              const ActAPI_ModificationType theModType,
                                              const Standard_Boolean doResetValidity,
                                              const Standard_Boolean doResetPending)
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  if ( theIndex > this->NbElements() )
    Standard_Failure::Raise("SetElement -- out of range");

  ActData_Utils::SetStringArrayElem(m_label, DS_Array, theIndex, theValue);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Accessor for the array elements.
//! \param theIndex [in] index of the element to access.
//! \return string addressed by the given element index.
TCollection_ExtendedString
  ActData_StringArrayParameter::GetElement(const Standard_Integer theIndex)
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  if ( theIndex > this->NbElements() )
    Standard_Failure::Raise("GetElement -- out of range");

  return ActData_Utils::GetStringArrayElem(m_label, DS_Array, theIndex);
}

//! Sets entire array.
//! \param theArray [in] array to set.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending [in] indicates whether this Parameter must lose its
//!        PENDING (or out-dated) property.
void ActData_StringArrayParameter::SetArray(const Handle(HStringArray)& theArray,
                                            const ActAPI_ModificationType theModType,
                                            const Standard_Boolean doResetValidity,
                                            const Standard_Boolean doResetPending)
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  if ( theArray.IsNull() )
    this->InitEmpty();
  else
  {
    // Store Array data
    ActData_Utils::InitStringArray(m_label, DS_Array, theArray);

    // Store Array dimension
    ActData_Utils::SetIntegerValue( m_label, DS_ElemNum, theArray->Length() );
  }

  // Record modification in LogBook:
  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(theModType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending);
}

//! Accessor for the stored array.
//! \return stored array.
Handle(HStringArray) ActData_StringArrayParameter::GetArray()
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  return ActData_Utils::GetStringArray(m_label, DS_Array);
}

//! Returns number of elements.
//! \return number of elements.
Standard_Integer ActData_StringArrayParameter::NbElements()
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  Standard_Integer aValue;
  ActData_Utils::GetIntegerValue(m_label, DS_ElemNum, aValue);
  return aValue;
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
Standard_Boolean ActData_StringArrayParameter::isWellFormed() const
{
  // Missing DS_Array Attribute means EMPTY array for us. We do not use
  // empty Attributes just in order not to loose them on storage/retrieval

  if ( !ActData_Utils::CheckLabelAttr( m_label, DS_ElemNum,
                                       TDataStd_Integer::GetID() ) )
    return Standard_False;

  return Standard_True;
}

//! Returns Parameter type.
//! \return Parameter type.
Standard_Integer ActData_StringArrayParameter::parameterType() const
{
  return Parameter_StringArray;
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
void ActData_StringArrayParameter::setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
                                              const ActAPI_ModificationType theModType,
                                              const Standard_Boolean doResetValidity,
                                              const Standard_Boolean doResetPending)
{
  Handle(ActData_StringArrayDTO) MyDTO = Handle(ActData_StringArrayDTO)::DownCast(theDTO);
  this->SetArray(MyDTO->Array, theModType, doResetValidity, doResetPending);
}

//! Creates and populates DTO.
//! \param theGID [in] ready-to-use GID for DTO.
//! \return constructed DTO instance.
Handle(ActData_ParameterDTO)
  ActData_StringArrayParameter::createDTO(const ActAPI_ParameterGID& theGID)
{
  Handle(ActData_StringArrayDTO) aRes = new ActData_StringArrayDTO(theGID);
  aRes->Array = this->GetArray();
  return aRes;
}
