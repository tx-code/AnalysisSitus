//-----------------------------------------------------------------------------
// Created on: March 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, OPEN CASCADE SAS
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

// Own include
#include <ActData_UserExtParameter.h>

// OpenCascade includes
#include <TDataStd_Integer.hxx>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_UserExtParameter::ActData_UserExtParameter() : ActData_UserParameter()
{
}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(ActData_UserExtParameter) ActData_UserExtParameter::Instance()
{
  return new ActData_UserExtParameter();
}

//! \return local Parameter ID.
Standard_Integer ActData_UserExtParameter::GetParamId() const
{
  if ( m_status == SS_Detached )
    return -1;

  return m_label.Tag();
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
Standard_Boolean ActData_UserExtParameter::isWellFormed() const
{
  return Standard_True;
}

//! Returns Parameter type.
//! \return Parameter type.
Standard_Integer ActData_UserExtParameter::parameterType() const
{
  if ( m_status == SS_Detached )
    return Parameter_UNDEFINED; // Non-standard and cannot be queries.

  // Get label which stores the Parameter type.
  TDF_Label aTypeLab = m_label.FindChild(ActData_UserParameter::DS_ParamType, Standard_False);
  //
  if ( aTypeLab.IsNull() )
    return -1;

  Handle(TDataStd_Integer) aTypeAttr;
  if ( !aTypeLab.FindAttribute(TDataStd_Integer::GetID(), aTypeAttr) )
    return -1; // Type Attribute not found

  return aTypeAttr->Get(); // Get the real type.
}

//-----------------------------------------------------------------------------
// DTO construction
//-----------------------------------------------------------------------------

void ActData_UserExtParameter::setFromDTO(const Handle(ActData_ParameterDTO)&,
                                                const ActAPI_ModificationType,
                                                const Standard_Boolean,
                                                const Standard_Boolean)
{}

//-----------------------------------------------------------------------------

Handle(ActData_ParameterDTO)
  ActData_UserExtParameter::createDTO(const ActAPI_ParameterGID&)
{
  return nullptr;
}
