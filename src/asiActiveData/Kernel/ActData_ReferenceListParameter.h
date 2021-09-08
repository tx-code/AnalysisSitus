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

#ifndef ActData_ReferenceListParameter_HeaderFile
#define ActData_ReferenceListParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

// OCCT includes
#include <TDataStd_ReferenceList.hxx>
#include <TDF_LabelList.hxx>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_ReferenceListDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Reference List Parameter without any OCAF connectivity.
class ActData_ReferenceListDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_ReferenceListDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_ReferenceListDTO(const ActAPI_ParameterGID& theGID)
  : ActData_ParameterDTO( theGID, Parameter_ReferenceList )
  {}

public:

  Handle(ActAPI_HDataCursorList) Targets; //!< Reference targets.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_ReferenceListParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Parameter representing a list of references onto CAF TDF Labels
//! corresponding to the roots of Parameter entities.
//! NOTICE: NULL references are not supported.
//! \todo it is necessary to introduce NULL references. This could not be
//!       just TDF_Label() as it will be lost after persistence routines
//!       perform.
//! \todo take care of issue #23465 is OCCT (about standard lists).
class ActData_ReferenceListParameter : public ActData_UserParameter
{
friend class ActData_BaseNode;
friend class ActData_BaseModel;
friend class ActData_CAFConversionCtx;
friend class ActData_CopyPasteEngine;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_ReferenceListParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_ReferenceListParameter) Instance();

public:

  ActData_EXPORT void
    AddTarget(const Handle(ActAPI_IDataCursor)& theTarget,
              const ActAPI_ModificationType     theModType      = MT_Touched,
              const Standard_Boolean            doResetValidity = Standard_True,
              const Standard_Boolean            doResetPending  = Standard_True);

  ActData_EXPORT void
    AddTarget(const TDF_Label&              theTargetLab,
              const ActAPI_ModificationType theModType      = MT_Touched,
              const Standard_Boolean        doResetValidity = Standard_True,
              const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT void
    PrependTarget(const Handle(ActAPI_IDataCursor)& theTarget,
                  const ActAPI_ModificationType     theModType      = MT_Touched,
                  const Standard_Boolean            doResetValidity = Standard_True,
                  const Standard_Boolean            doResetPending  = Standard_True);

  ActData_EXPORT void
    PrependTarget(const TDF_Label&              theTargetLab,
                  const ActAPI_ModificationType theModType      = MT_Touched,
                  const Standard_Boolean        doResetValidity = Standard_True,
                  const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT void
    InsertTargetAfter(const Standard_Integer            theIndex,
                      const Handle(ActAPI_IDataCursor)& theTarget,
                      const ActAPI_ModificationType     theModType      = MT_Touched,
                      const Standard_Boolean            doResetValidity = Standard_True,
                      const Standard_Boolean            doResetPending  = Standard_True);

  ActData_EXPORT void
    InsertTargetAfter(const Standard_Integer        theIndex,
                      const TDF_Label&              theTargetLab,
                      const ActAPI_ModificationType theModType      = MT_Touched,
                      const Standard_Boolean        doResetValidity = Standard_True,
                      const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT Standard_Integer
    HasTarget(const Handle(ActAPI_IDataCursor)& theTarget);

  ActData_EXPORT Standard_Integer
    HasTarget(const TDF_Label& theTargetLab);

  ActData_EXPORT Standard_Boolean
    RemoveTargetOccurrences(const Handle(ActAPI_IDataCursor)& theTarget,
                            const ActAPI_ModificationType     theModType      = MT_Touched,
                            const Standard_Boolean            doResetValidity = Standard_True,
                            const Standard_Boolean            doResetPending  = Standard_True);

  ActData_EXPORT Standard_Boolean
    RemoveTarget(const Handle(ActAPI_IDataCursor)& theTarget,
                 const ActAPI_ModificationType     theModType      = MT_Touched,
                 const Standard_Boolean            doResetValidity = Standard_True,
                 const Standard_Boolean            doResetPending  = Standard_True);

  ActData_EXPORT Standard_Boolean
    RemoveTarget(const Standard_Integer        theTargetIndex,
                 const ActAPI_ModificationType theModType      = MT_Touched,
                 const Standard_Boolean        doResetValidity = Standard_True,
                 const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT Standard_Boolean
    RemoveTargets(const ActAPI_ModificationType theModType      = MT_Touched,
                  const Standard_Boolean        doResetValidity = Standard_True,
                  const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT Handle(ActAPI_IDataCursor)
    GetTarget(const Standard_Integer theIndex) const;

  ActData_EXPORT TDF_Label
    GetTargetLabel(const Standard_Integer theIndex) const;

  ActData_EXPORT Handle(ActAPI_HDataCursorList)
    GetTargets() const;

  ActData_EXPORT void
    SetTargets(const Handle(ActAPI_HDataCursorList)& theTargets,
               const ActAPI_ModificationType         theModType      = MT_Touched,
               const Standard_Boolean                doResetValidity = Standard_True,
               const Standard_Boolean                doResetPending  = Standard_True);

  ActData_EXPORT Standard_Boolean
    ExchangeTarget(const TDF_Label&              theTargetOldLab,
                   const TDF_Label&              theTargetNewLab,
                   const ActAPI_ModificationType theModType      = MT_Touched,
                   const Standard_Boolean        doResetValidity = Standard_True,
                   const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT Standard_Boolean
    ExchangeTarget(const Handle(ActAPI_IDataCursor)& theTargetOld,
                   const Handle(ActAPI_IDataCursor)& theTargetNew,
                   const ActAPI_ModificationType     theModType      = MT_Touched,
                   const Standard_Boolean            doResetValidity = Standard_True,
                   const Standard_Boolean            doResetPending  = Standard_True);

  ActData_EXPORT Standard_Boolean
    SwapTargets(const Standard_Integer        theFirstIndex,
                const Standard_Integer        theSecondIndex,
                const ActAPI_ModificationType theModType      = MT_Touched,
                const Standard_Boolean        doResetValidity = Standard_True,
                const Standard_Boolean        doResetPending  = Standard_True);

  ActData_EXPORT Standard_Integer
    NbTargets();

  ActData_EXPORT Handle(TDataStd_ReferenceList)
    AccessReferenceList() const;

protected:

  ActData_ReferenceListParameter();

private:

  void getTargets(TDF_LabelList& theLabelList) const;

  Standard_Boolean removeTarget(const TDF_Label& theTargetLab);

private:

  virtual Standard_Boolean isWellFormed() const;
  virtual Standard_Integer parameterType() const;

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
               const ActAPI_ModificationType       theModType      = MT_Touched,
               const Standard_Boolean              doResetValidity = Standard_True,
               const Standard_Boolean              doResetPending  = Standard_True);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& theGID);

protected:

  //! Tags for the underlying sub-Labels.
  enum Datum
  {
    DS_Targets = ActData_UserParameter::DS_DatumLast,
    //
    DS_DatumLast = ActData_UserParameter::DS_DatumLast + RESERVED_DATUM_RANGE
  };

};

#endif
