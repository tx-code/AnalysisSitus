//-----------------------------------------------------------------------------
// Created on: 27 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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
//    * Neither the name of the copyright holder(s) nor the
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
//-----------------------------------------------------------------------------

#ifndef asiData_DiscrModelParameter_h
#define asiData_DiscrModelParameter_h

// asiData includes
#include <asiData_DiscrModelAttr.h>

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

//! Data Transfer Object (DTO) corresponding to data wrapped with
//! the discrete model Parameter without any OCAF connectivity.
class asiData_DiscrModelDTO : public ActData_ParameterDTO
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_DiscrModelDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param[in] GID GID.
  asiData_DiscrModelDTO(const ActAPI_ParameterGID& GID)
  : ActData_ParameterDTO(GID, Parameter_UNDEFINED)
  {}

public:

  Handle(asiAlgo::discr::Model) DiscrModel; //!< Discrete model.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Node Parameter representing discrete model.
class asiData_DiscrModelParameter : public ActData_UserParameter
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_DiscrModelParameter, ActData_UserParameter)

public:

  asiData_EXPORT static Handle(asiData_DiscrModelParameter)
    Instance();

// API:
public:

  asiData_EXPORT void
    SetDiscrModel(const Handle(asiAlgo::discr::Model)& model,
                  const ActAPI_ModificationType        MType           = MT_Touched,
                  const bool                           doResetValidity = true,
                  const bool                           doResetPending  = true);

  asiData_EXPORT Handle(asiAlgo::discr::Model)
    GetDiscrModel();

protected:

  asiData_EXPORT
    asiData_DiscrModelParameter();

private:

  virtual bool isWellFormed() const;
  virtual int parameterType() const;

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& DTO,
               const ActAPI_ModificationType       MType = MT_Touched,
               const bool                          doResetValidity = true,
               const bool                          doResetPending  = true);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& GID);

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_DiscrModel = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast  = DS_DiscrModel + RESERVED_DATUM_RANGE
  };

};

#endif
