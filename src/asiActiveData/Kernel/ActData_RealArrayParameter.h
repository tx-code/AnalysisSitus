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

#ifndef ActData_RealArrayParameter_HeaderFile
#define ActData_RealArrayParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_RealArrayDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Real Array Parameter without any OCAF connectivity.
class ActData_RealArrayDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_RealArrayDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_RealArrayDTO(const ActAPI_ParameterGID& theGID) : ActData_ParameterDTO( theGID, Parameter_RealArray ) {}

public:

  Handle(HRealArray) Array; //!< Actual array.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_RealArrayParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Node Parameter representing an array of real values.
class ActData_RealArrayParameter : public ActData_UserParameter
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_RealArrayParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_RealArrayParameter) Instance();

public:

  ActData_EXPORT void
    InitEmpty();

  ActData_EXPORT void
    BackupArray();

  ActData_EXPORT void
    SetElement(const Standard_Integer theIndex,
               const Standard_Real theValue,
               const ActAPI_ModificationType theModType = MT_Silent,
               const Standard_Boolean doResetValidity = Standard_False,
               const Standard_Boolean doResetPending = Standard_False);

  ActData_EXPORT Standard_Real
    GetElement(const Standard_Integer theIndex);

  ActData_EXPORT void
    SetArray(const Handle(HRealArray)& theArray,
             const ActAPI_ModificationType theModType = MT_Touched,
             const Standard_Boolean doResetValidity = Standard_True,
             const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT Handle(HRealArray)
    GetArray();

  ActData_EXPORT Standard_Integer
    NbElements();

protected:

  ActData_RealArrayParameter();

private:

  virtual Standard_Boolean isWellFormed() const;
  virtual Standard_Integer parameterType() const;

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
               const ActAPI_ModificationType theModType = MT_Touched,
               const Standard_Boolean doResetValidity = Standard_True,
               const Standard_Boolean doResetPending = Standard_True);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& theGID);

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_Array = ActData_UserParameter::DS_DatumLast,
    DS_ElemNum,
    DS_DatumLast = DS_Array + RESERVED_DATUM_RANGE
  };

};

#endif
