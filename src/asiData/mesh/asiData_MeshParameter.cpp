//-----------------------------------------------------------------------------
// Created on: 06 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiData_MeshParameter.h>

// Active Data includes
#include <ActData_Utils.h>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Default constructor.
asiData_MeshParameter::asiData_MeshParameter() : ActData_UserParameter()
{}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(asiData_MeshParameter) asiData_MeshParameter::Instance()
{
  return new asiData_MeshParameter();
}

//! Sets the mesh to store.
//! \param[in] mesh            mesh to set.
//! \param[in] MType           modification type.
//! \param[in] doResetValidity indicates whether to reset validity flag.
//! \param[in] doResetPending  indicates whether this Parameter must lose its
//!                            PENDING (or out-dated) property.
void asiData_MeshParameter::SetMesh(void*                         mesh,
                                    const ActAPI_ModificationType MType,
                                    const bool                    doResetValidity,
                                    const bool                    doResetPending)
{
  if ( this->IsDetached() )
    Standard_ProgramError::Raise("Cannot access detached data");

  // Settle down an attribute an populate it with data
  TDF_Label                dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_Mesh, true);
  Handle(asiData_MeshAttr) attr    = asiData_MeshAttr::Set(dataLab);
  //
  attr->SetMesh(mesh);

  // Mark root label of the Parameter as modified (Touched, Impacted or Silent)
  SPRING_INTO_FUNCTION(MType)
  // Reset Parameter's validity flag if requested
  RESET_VALIDITY(doResetValidity)
  // Reset Parameter's PENDING property
  RESET_PENDING(doResetPending)
}

//! Accessor for the stored mesh.
//! \return stored mesh.
void* asiData_MeshParameter::GetMesh()
{
  if ( !this->IsWellFormed() )
    Standard_ProgramError::Raise("Data inconsistent");

  // Choose a data label ensuring not to create it
  TDF_Label dataLab = ActData_Utils::ChooseLabelByTag(m_label, DS_Mesh, false);
  //
  if ( dataLab.IsNull() )
    return nullptr;

  // Get mesh attribute
  Handle(asiData_MeshAttr) attr;
  dataLab.FindAttribute(asiData_MeshAttr::GUID(), attr);
  //
  if ( attr.IsNull() )
    return nullptr;

  return attr->GetMesh();
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
bool asiData_MeshParameter::isWellFormed() const
{
  // Mesh may not be present, that's Ok for such sort of transient attributes.

  return true;
}

//! Returns Parameter type.
//! \return Parameter type.
int asiData_MeshParameter::parameterType() const
{
  return Parameter_PolyMesh;
}

//-----------------------------------------------------------------------------
// DTO construction
//-----------------------------------------------------------------------------

//! Populates Parameter from the passed DTO.
//! \param[in] DTO             DTO to source data from.
//! \param[in] MType           modification type.
//! \param[in] doResetValidity indicates whether validity flag must be
//!                            reset or not.
//! \param[in] doResetPending  indicates whether pending flag must be reset
//!                            or not.
void asiData_MeshParameter::setFromDTO(const Handle(ActData_ParameterDTO)& DTO,
                                       const ActAPI_ModificationType       MType,
                                       const bool                          doResetValidity,
                                       const bool                          doResetPending)
{
  Handle(asiData_MeshDTO) MyDTO = Handle(asiData_MeshDTO)::DownCast(DTO);
  this->SetMesh(MyDTO->pMesh, MType, doResetValidity, doResetPending);
}

//! Creates and populates DTO.
//! \param[in] GID ready-to-use GID for DTO.
//! \return constructed DTO instance.
Handle(ActData_ParameterDTO)
  asiData_MeshParameter::createDTO(const ActAPI_ParameterGID& GID)
{
  Handle(asiData_MeshDTO) res = new asiData_MeshDTO(GID);
  res->pMesh = this->GetMesh();
  return res;
}
