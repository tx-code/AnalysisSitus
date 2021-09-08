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

#ifndef ActData_TreeFunctionParameter_HeaderFile
#define ActData_TreeFunctionParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_ITreeFunction.h>

// OCCT includes
#include <TDataStd_ReferenceList.hxx>
#include <TDF_LabelList.hxx>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_TreeFunctionDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Tree Function Parameter without any OCAF connectivity.
class ActData_TreeFunctionDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TreeFunctionDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_TreeFunctionDTO(const ActAPI_ParameterGID& theGID)
  : ActData_ParameterDTO( theGID, Parameter_TreeFunction )
  {}

public:

  Standard_GUID                DriverGUID; //!< Tree Function GUID.
  Handle(ActAPI_HParameterList) Arguments;  //!< Tree Function arguments.
  Handle(ActAPI_HParameterList) Results;    //!< Tree Function results.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_TreeFunctionParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! \todo provide comment here
class ActData_TreeFunctionParameter : public ActData_UserParameter
{
friend class ActData_BaseNode;
friend class ActData_BaseModel;
friend class ActData_CopyPasteEngine;
friend class ActData_TreeFunctionDriver;
friend class ActData_CAFConversionCtx;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TreeFunctionParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_TreeFunctionParameter)
    Instance();

  ActData_EXPORT void
    SetDriverGUID(const Standard_GUID& theGUID);

  ActData_EXPORT Standard_Boolean
    GetDriverGUID(Standard_GUID& theGUID) const;

  ActData_EXPORT void
    AddArgument(const Handle(ActAPI_IUserParameter)& theParam);

  ActData_EXPORT Standard_Boolean
    HasArgument(const TDF_Label& theParamLab) const;

  ActData_EXPORT Standard_Boolean
    HasArgument(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT void
    AddResult(const Handle(ActAPI_IUserParameter)& theParam);

  ActData_EXPORT Standard_Boolean
    HasResult(const TDF_Label& theParamLab) const;

  ActData_EXPORT Standard_Boolean
    HasResult(const Handle(ActAPI_IUserParameter)& theParam) const;

  ActData_EXPORT void
    Disconnect(const Standard_Boolean toKillCompletely = Standard_True);

  ActData_EXPORT void
    DisconnectSoft();

  ActData_EXPORT Standard_Boolean
    IsConnected() const;

  ActData_EXPORT Handle(ActAPI_HParameterList)
    Arguments() const;

  ActData_EXPORT Handle(ActAPI_HParameterList)
    Results() const;

  ActData_EXPORT Standard_Boolean
    IsHeavyFunction() const;

  ActData_EXPORT Standard_Boolean
    HasPendingArguments() const;

  ActData_EXPORT Standard_Boolean
    HasPendingResults() const;

protected:

  ActData_TreeFunctionParameter();

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

  Handle(TDataStd_ReferenceList)
    getArgumentsAttr() const;

  Handle(TDataStd_ReferenceList)
    getResultsAttr() const;

  TDF_Label
    getArgumentsLabel() const;

  TDF_Label
    getResultsLabel() const;

  void
    getArguments(TDF_LabelList& theLabelList) const;

  void
    getResults(TDF_LabelList& theLabelList) const;

  Standard_Boolean
    hasPending(const Handle(ActAPI_HParameterList)& theParams) const;

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_Arguments = ActData_UserParameter::DS_DatumLast,
    DS_Results,
    DS_DatumLast = DS_Arguments + RESERVED_DATUM_RANGE
  };

};

#endif
