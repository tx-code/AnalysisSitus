//-----------------------------------------------------------------------------
// Created on: 21 May 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

#ifndef asiData_MetadataParameter_h
#define asiData_MetadataParameter_h

// asiData includes
#include <asiData_MetadataAttr.h>

// Active Data includes
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>
#include <ActData_UserParameter.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

//! Data Transfer Object (DTO) corresponding to data wrapped with
//! Metadata Parameter without any OCAF connectivity.
class asiData_MetadataDTO : public ActData_ParameterDTO
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_MetadataDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param GID [in] GID.
  asiData_MetadataDTO(const ActAPI_ParameterGID& GID) : ActData_ParameterDTO(GID, Parameter_UNDEFINED) {}

public:

  asiData_MetadataAttr::t_shapeColorMap ShapeColorMap; //!< Shape-Color map.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Node Parameter representing Metadata.
class asiData_MetadataParameter : public ActData_UserParameter
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_MetadataParameter, ActData_UserParameter)

public:

  asiData_EXPORT static Handle(asiData_MetadataParameter)
    Instance();

public:

  asiData_EXPORT void
    SetColor(const TopoDS_Shape&           shape,
             const int                     icolor,
             const ActAPI_ModificationType MType           = MT_Silent,
             const bool                    doResetValidity = false,
             const bool                    doResetPending  = false);

  asiData_EXPORT int
    GetColor(const TopoDS_Shape& shape);

  asiData_EXPORT void
    SetShapeColorMap(const asiData_MetadataAttr::t_shapeColorMap& map,
                     const ActAPI_ModificationType                MType           = MT_Touched,
                     const bool                                   doResetValidity = true,
                     const bool                                   doResetPending  = true);

  asiData_EXPORT void
    GetShapeColorMap(asiData_MetadataAttr::t_shapeColorMap& map);

protected:

  asiData_EXPORT
    asiData_MetadataParameter();

private:

  virtual bool isWellFormed() const;
  virtual int parameterType() const;

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& DTO,
               const ActAPI_ModificationType       MType = MT_Touched,
               const bool                          doResetValidity = true,
               const bool                          doResetPending = true);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& GID);

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_ShapeColorMap = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_ShapeColorMap + RESERVED_DATUM_RANGE
  };

};

#endif
