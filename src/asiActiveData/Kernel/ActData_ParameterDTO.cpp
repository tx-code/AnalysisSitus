//-----------------------------------------------------------------------------
// Created on: March 2013
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

#include <ActData_ParameterDTO.h>

#define NIL ""

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             NIL, NIL, NIL,
             Standard_True, 0, Standard_False);
}

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theName [in] name.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType,
                                           const TCollection_ExtendedString& theName)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             theName, NIL, NIL,
             Standard_True, 0, Standard_False);
}

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theSID [in] semantic ID.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType,
                                           const TCollection_AsciiString& theSID)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             NIL, theSID, NIL,
             Standard_True, 0, Standard_False);
}

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theName [in] name.
//! \param theSID [in] semantic ID.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType,
                                           const TCollection_ExtendedString& theName,
                                           const TCollection_AsciiString& theSID)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             theName, theSID, NIL,
             Standard_True, 0, Standard_False);
}

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theName [in] name.
//! \param theSID [in] semantic ID.
//! \param theEvalStr [in] evaluation string.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType,
                                           const TCollection_ExtendedString& theName,
                                           const TCollection_AsciiString& theSID,
                                           const TCollection_AsciiString& theEvalStr)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             theName, theSID, theEvalStr,
             Standard_True, 0, Standard_False);
}

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theUFlags [in] user flags.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType,
                                           const Standard_Integer theUFlags)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             NIL, NIL, NIL,
             Standard_True, theUFlags, Standard_False);
}

//! Constructor.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theName [in] name.
//! \param theSID [in] semantic ID.
//! \param theEvalStr [in] evaluation string.
//! \param theUFlags [in] user flags.
ActData_ParameterDTO::ActData_ParameterDTO(const ActAPI_ParameterGID& theGID,
                                           const ActAPI_ParameterType theParamType,
                                           const TCollection_ExtendedString& theName,
                                           const TCollection_AsciiString& theSID,
                                           const TCollection_AsciiString& theEvalStr,
                                           const Standard_Integer theUFlags)
: Standard_Transient()
{
  this->Init(theGID, theParamType,
             theName, theSID, theEvalStr,
             Standard_True, theUFlags, Standard_False);
}

//! Initialization routine.
//! \param theGID [in] global ID of the Parameter.
//! \param theParamType [in] Parameter type.
//! \param theName [in] name.
//! \param theSID [in] semantic ID.
//! \param theEvalStr [in] evaluation string.
//! \param isValid [in] validity flag.
//! \param theUFlags [in] user flags.
//! \param isPending [in] pending flag.
void ActData_ParameterDTO::Init(const ActAPI_ParameterGID& theGID,
                                const ActAPI_ParameterType theParamType,
                                const TCollection_ExtendedString& theName,
                                const TCollection_AsciiString& theSID,
                                const TCollection_AsciiString& theEvalStr,
                                const Standard_Boolean isValid,
                                const Standard_Integer theUFlags,
                                const Standard_Boolean isPending)
{
  m_GID        = theGID;
  m_type       = theParamType;
  m_name       = theName;
  m_SID        = theSID;
  m_evalStr    = theEvalStr;
  m_bIsValid   = isValid;
  m_iUFlags    = theUFlags;
  m_bIsPending = isPending;
}
