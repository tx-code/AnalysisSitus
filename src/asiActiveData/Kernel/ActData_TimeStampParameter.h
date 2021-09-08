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

#ifndef ActData_TimeStampParameter_HeaderFile
#define ActData_TimeStampParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>

// Active Data (auxiliary) layer includes
#include <ActAux_TimeStamp.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_TimeStampDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Timestamp Parameter without any OCAF connectivity.
class ActData_TimeStampDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TimeStampDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_TimeStampDTO(const ActAPI_ParameterGID& theGID) : ActData_ParameterDTO( theGID, Parameter_TimeStamp ) {}

public:

  Handle(ActAux_TimeStamp) TimeStamp; //!< Actual timestamp.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_TimeStampParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Node Parameter representing timestamp.
class ActData_TimeStampParameter : public ActData_UserParameter
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TimeStampParameter, ActData_UserParameter)

public:

  //! Item comprised in the stored timestamp. Corresponds to tm structure
  //! from wchar.h.
  enum Item
  {
    Item_Seconds = 1, //!< Seconds after the minute: 0 - 59.
    Item_Minutes,     //!< Minutes after the hour: 0 - 59.
    Item_Hours,       //!< Hours after the midnight: 0 - 23.
    Item_MDays,       //!< Day in the month: 0 - 31.
    Item_Month,       //!< Months after January: 0 - 11.
    Item_Years,       //!< Years after 1900.
    Item_WDays,       //!< Days after Sunday: 0 - 6.
    Item_YDays,       //!< Days after January 1: 0 - 365.
    Item_IsDST        //!< Indicates whether daylight savings time is set.
  };

public:

  ActData_EXPORT static Handle(ActData_TimeStampParameter) Instance();

public:

  ActData_EXPORT void
    SetValue(const Handle(ActAux_TimeStamp)& theVal,
             const ActAPI_ModificationType theModType = MT_Touched,
             const Standard_Boolean doResetValidity = Standard_True,
             const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT void
    SetValue(const Standard_Integer theItemVal,
             const Item theItemType,
             const ActAPI_ModificationType theModType = MT_Touched,
             const Standard_Boolean doResetValidity = Standard_True,
             const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT Handle(ActAux_TimeStamp)
    GetValue();

  ActData_EXPORT Standard_Integer
    GetValue(const Item theItemType);

private:

  ActData_TimeStampParameter();

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

private:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_Value = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_Value + RESERVED_DATUM_RANGE
  };

};

#endif
