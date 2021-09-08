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

#ifndef ActData_NameParameter_HeaderFile
#define ActData_NameParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_NameDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Name Parameter without any OCAF connectivity.
class ActData_NameDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_NameDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_NameDTO(const ActAPI_ParameterGID& theGID) : ActData_ParameterDTO( theGID, Parameter_Name ) {}

public:

  TCollection_ExtendedString Value; //!< Actual string.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_NameParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Parameter representing a Node name.
class ActData_NameParameter : public ActData_UserParameter
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_NameParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_NameParameter) Instance();

public:

  ActData_EXPORT void
    SetValue(const TCollection_ExtendedString& theString,
             const ActAPI_ModificationType theModType = MT_Touched,
             const Standard_Boolean doResetValidity = Standard_True,
             const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT TCollection_ExtendedString
    GetValue();

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
               const ActAPI_ModificationType theModType = MT_Touched,
               const Standard_Boolean doResetValidity = Standard_True,
               const Standard_Boolean doResetPending = Standard_True);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& theGID);

protected:

  ActData_NameParameter();

private:

  virtual Standard_Boolean isWellFormed() const;
  virtual Standard_Integer parameterType() const;

};

#endif
