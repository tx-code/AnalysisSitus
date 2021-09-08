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

#ifndef ActData_UserExtParameter_HeaderFile
#define ActData_UserExtParameter_HeaderFile

// Active Data includes
#include <ActData_UserParameter.h>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

DEFINE_STANDARD_HANDLE(ActData_UserExtParameter, ActData_UserParameter)

//! \ingroup AD_DF
//!
//! External User Parameter. The data framework will construct this kind of
//! Parameters in cases when the Parameter Factory is incapable of creating
//! some specific Parameter types. This external Parameter is actually a
//! mockup to represent the custom Parameters which are not native to
//! Active Data. Alternatively, we could return null pointers but this would
//! be an unsafe and limited approach. An example of a use case where you
//! might face this External Parameter type is Tree Function execution.
//! If your Tree Function uses any non-standard Parameters in its inputs
//! or outputs, then such external Parameters will be passed to the execution
//! method. On execution, the external Parameters can be interpreted by the
//! client code which is well-aware of them.
//!
//! The real type of the external Parameter is available if the Parameter
//! is attached (so the type can be queried from the data storage). Also,
//! in the attached state, the external Parameter can return the local
//! Parameter ID which will help to access the real type in the invoking
//! code.
class ActData_UserExtParameter : public ActData_UserParameter
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_UserExtParameter, ActData_UserParameter)

public:

  ActData_EXPORT static Handle(ActData_UserExtParameter) Instance();

public:

  ActData_EXPORT Standard_Integer
    GetParamId() const;

private:

  ActData_UserExtParameter();

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

};

#endif
