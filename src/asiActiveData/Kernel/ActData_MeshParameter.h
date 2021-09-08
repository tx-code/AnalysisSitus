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

#ifndef ActData_MeshParameter_HeaderFile
#define ActData_MeshParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

// Mesh includes
#include <ActData_Mesh.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_MeshDTO, ActData_ParameterDTO)

//! \ingroup AD_DF
//!
//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Mesh Parameter without any OCAF connectivity.
class ActData_MeshDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MeshDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param theGID [in] GID.
  ActData_MeshDTO(const ActAPI_ParameterGID& theGID) : ActData_ParameterDTO( theGID, Parameter_Mesh ) {}

public:

  Handle(ActData_Mesh) Mesh; //!< Actual mesh.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_MeshParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! Parameter class representing Mesh data.
class ActData_MeshParameter : public ActData_UserParameter
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_MeshParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_MeshParameter) Instance();

public:

  ActData_EXPORT void
    DeltaModeOn();

  ActData_EXPORT void
    DeltaModeOff();

  ActData_EXPORT void
    SetMesh(const Handle(ActData_Mesh)& theMesh,
            const ActAPI_ModificationType theModType = MT_Touched,
            const Standard_Boolean doResetValidity = Standard_True,
            const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT Standard_Integer
    AddNode(const Standard_Real theNodeX,
            const Standard_Real theNodeY,
            const Standard_Real theNodeZ,
            const ActAPI_ModificationType theModType = MT_Touched,
            const Standard_Boolean doResetValidity = Standard_True,
            const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT Standard_Integer
    AddElement(Standard_Address theNodes,
               const Standard_Integer theNbNodes,
               const ActAPI_ModificationType theModType = MT_Touched,
               const Standard_Boolean doResetValidity = Standard_True,
               const Standard_Boolean doResetPending = Standard_True);

  ActData_EXPORT Handle(ActData_Mesh)
    GetMesh();

private:

  ActData_MeshParameter();

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

  //! Tags range for storage mapping.
  enum Datum
  {
    DS_Mesh = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_Mesh + RESERVED_DATUM_RANGE
  };

};

#endif
