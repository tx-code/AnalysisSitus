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

#ifndef ActData_ReferenceParameter_HeaderFile
#define ActData_ReferenceParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>

// OCCT includes
#include <TDF_LabelList.hxx>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_ReferenceDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Reference Parameter without any OCAF connectivity.
class ActData_ReferenceDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_ReferenceDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_ReferenceDTO(const ActAPI_ParameterGID& theGID)
  : ActData_ParameterDTO( theGID, Parameter_Reference ),
    Target(NULL)
  {}

public:

  Handle(ActAPI_IDataCursor) Target; //!< Reference target.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_ReferenceParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Parameter representing a reference onto CAF TDF Label corresponding to the
//! root of some Data Object (e.g. Node or Parameter).
class ActData_ReferenceParameter : public ActData_UserParameter
{
friend class ActData_BaseNode;
friend class ActData_BaseModel;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_ReferenceParameter, ActData_UserParameter)

public:

  //! Managed reference types.
  enum RefType
  {
    Ref_Undefined = 1,
    Ref_Parameter,
    Ref_Node
  };

public:

  ActData_EXPORT static Handle(ActData_ReferenceParameter)
    Instance();

  ActData_EXPORT void
    SetTarget(const TDF_Label& theLabel,
              const ActAPI_ModificationType theModType = MT_Touched,
              const Standard_Boolean doResetValidity = Standard_True,
              const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT void
    SetTarget(const Handle(ActAPI_IDataCursor)& theDC,
              const ActAPI_ModificationType theModType = MT_Touched,
              const Standard_Boolean doResetValidity = Standard_True,
              const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT Handle(ActAPI_IDataCursor)
    GetTarget(const RefType theRefType) const;

  ActData_EXPORT Handle(ActAPI_IDataCursor)
    GetTarget() const;

  ActData_EXPORT TDF_Label
    GetTargetLabel() const;

  ActData_EXPORT Standard_Boolean
    IsTarget(const Handle(ActAPI_IDataCursor)& theDC);

  ActData_EXPORT void
    RemoveTarget(const ActAPI_ModificationType theModType = MT_Touched,
                 const Standard_Boolean doResetValidity = Standard_True,
                 const Standard_Boolean doResetPending = Standard_True);

protected:

  ActData_ReferenceParameter();

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

  //! Tags for the stored CAF data chunks.
  enum Datum
  {
    DS_Ref = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_Ref + RESERVED_DATUM_RANGE
  };

};

#endif
