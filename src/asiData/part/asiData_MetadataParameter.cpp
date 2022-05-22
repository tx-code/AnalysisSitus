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

// Own include
#include <asiData_MetadataParameter.h>

// Active Data includes
#include <ActData_Utils.h>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Default constructor.
asiData_MetadataParameter::asiData_MetadataParameter() : ActData_UserParameter()
{}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(asiData_MetadataParameter)
  asiData_MetadataParameter::Instance()
{
  return new asiData_MetadataParameter();
}

void
  asiData_MetadataParameter::SetColor(const TopoDS_Shape&           shape,
                                      const int                     icolor,
                                      const ActAPI_ModificationType MType,
                                      const bool                    doResetValidity,
                                      const bool                    doResetPending)
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  // Settle down an attribute an populate it with data
  TDF_Label                    dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_ShapeColorMap, true);
  Handle(asiData_MetadataAttr) attr    = asiData_MetadataAttr::Set(dataLab);
  //
  attr->SetColor(shape, icolor);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(MType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending)
}

int
  asiData_MetadataParameter::GetColor(const TopoDS_Shape& shape)
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  // Choose a data label ensuring not to create it
  TDF_Label dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_ShapeColorMap, false);
  //
  if ( dataLab.IsNull() )
    return 0;

  // Get metadata attribute
  Handle(asiData_MetadataAttr) attr;
  dataLab.FindAttribute(asiData_MetadataAttr::GUID(), attr);
  //
  if ( attr.IsNull() )
    return 0;

  return attr->GetColor(shape);
}

//! Sets shape-color map.
//! \param map             [in] shape-color map to set.
//! \param MType           [in] modification type.
//! \param doResetValidity [in] indicates whether to reset validity flag.
//! \param doResetPending  [in] indicates whether this Parameter must lose its
//!                             PENDING (or out-dated) property.
void asiData_MetadataParameter::SetShapeColorMap(const asiData_MetadataAttr::t_shapeColorMap& map,
                                                 const ActAPI_ModificationType                MType,
                                                 const bool                                   doResetValidity,
                                                 const bool                                   doResetPending)
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  // Settle down an attribute an populate it with data
  TDF_Label                    dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_ShapeColorMap, true);
  Handle(asiData_MetadataAttr) attr    = asiData_MetadataAttr::Set(dataLab);
  //
  attr->SetShapeColorMap(map);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(MType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending)
}

//! Accessor for the stored shape-color map.
//! \param[out] map the stored shape-color map.
void
  asiData_MetadataParameter::GetShapeColorMap(asiData_MetadataAttr::t_shapeColorMap& map)
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  // Choose a data label ensuring not to create it
  TDF_Label dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_ShapeColorMap, false);
  //
  if ( dataLab.IsNull() )
    return;

  // Get metadata attribute
  Handle(asiData_MetadataAttr) attr;
  dataLab.FindAttribute(asiData_MetadataAttr::GUID(), attr);
  //
  if ( attr.IsNull() )
    return;

  map = attr->GetShapeColorMap();
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
bool asiData_MetadataParameter::isWellFormed() const
{
  // Always fine
  return true;
}

//! Returns Parameter type.
//! \return Parameter type.
int asiData_MetadataParameter::parameterType() const
{
  return Parameter_Metadata;
}

//-----------------------------------------------------------------------------
// DTO construction
//-----------------------------------------------------------------------------

//! Populates Parameter from the passed DTO.
//! \param DTO             [in] DTO to source data from.
//! \param MType           [in] modification type.
//! \param doResetValidity [in] indicates whether validity flag must be
//!                             reset or not.
//! \param doResetPending  [in] indicates whether pending flag must be reset
//!                             or not.
void asiData_MetadataParameter::setFromDTO(const Handle(ActData_ParameterDTO)& DTO,
                                           const ActAPI_ModificationType       MType,
                                           const bool                          doResetValidity,
                                           const bool                          doResetPending)
{
  Handle(asiData_MetadataDTO)
    MyDTO = Handle(asiData_MetadataDTO)::DownCast(DTO);

  this->SetShapeColorMap(MyDTO->ShapeColorMap, MType, doResetValidity, doResetPending);
}

//! Creates and populates DTO.
//! \param GID [in] ready-to-use GID for DTO.
//! \return constructed DTO instance.
Handle(ActData_ParameterDTO)
  asiData_MetadataParameter::createDTO(const ActAPI_ParameterGID& GID)
{
  Handle(asiData_MetadataDTO)
    res = new asiData_MetadataDTO(GID);

  this->GetShapeColorMap(res->ShapeColorMap);
  return res;
}
