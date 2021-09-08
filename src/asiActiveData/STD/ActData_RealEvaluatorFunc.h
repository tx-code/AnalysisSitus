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

#ifndef ActData_RealEvaluatorFunc_HeaderFile
#define ActData_RealEvaluatorFunc_HeaderFile

// Active Data includes
#include <ActData_BaseTreeFunction.h>
#include <ActData_Common.h>

DEFINE_STANDARD_HANDLE(ActData_RealEvaluatorFunc, ActData_BaseTreeFunction)

//! \ingroup AD_DF
//!
//! Tree Function performing evaluation of mathematical expressions associated with
//! Nodal Parameters of Real & Integer types. Notice, however, that in both
//! cases the Tree Function asks evaluator to return a Real value. If the input
//! Parameter is of an Integer type, the explicit conversion (without any
//! rounding) will be applied.
//!
//! This Tree Function is designed for the following parameterization scheme:
//! <pre>
//! INPUT Parameters:              ________________OPTIONAL_________________
//!                               |                                         |
//! +=======================+     +=============+             +=============+
//! | Target Real Parameter | --> | Var Param 1 | --> ... --> | Var Param N |
//! +=======================+     +=============+             +=============+
//!                               |                                         |
//!                               |__________DEFINED_BY_APPLICATION_________|
//!
//!                              ||
//!                              || 
//!                             _||_  EVALUATION OF PYTHON EXPRESSION
//!                             \  /
//!                              \/
//!
//!                    +=======================+
//! OUTPUT Parameters: | Target Real Parameter |
//!                    +=======================+
//! </pre>
//! This evaluator function is a ready-to-use custom implementation of
//! Tree Function concept dedicated for supporting of Variables mechanism
//! shipped with Active Data.
class ActData_RealEvaluatorFunc : public ActData_BaseTreeFunction
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_RealEvaluatorFunc, ActData_BaseTreeFunction)

public:

  ActData_EXPORT static Handle(ActData_RealEvaluatorFunc)
    Instance();

  ActData_EXPORT virtual Standard_CString
    GetName() const;

  ActData_EXPORT static Standard_CString
    GUID();

  ActData_EXPORT virtual Standard_CString
    GetGUID() const;

  ActData_EXPORT static Standard_Boolean
    IS_HEAVY();

  ActData_EXPORT virtual Standard_Boolean
    IsHeavy() const;

  ActData_EXPORT virtual Standard_Integer
    Priority() const;

private:

  virtual Standard_Integer
    execute(const Handle(ActAPI_HParameterList)& theArgsIN,
            const Handle(ActAPI_HParameterList)& theArgsOUT,
            const Handle(Standard_Transient)& theUserData = nullptr) const;

  virtual ActAPI_ParameterTypeStream
    inputSignature() const;

  virtual ActAPI_ParameterTypeStream
    outputSignature() const;

  virtual Standard_Boolean
    validateInput(const Handle(ActAPI_HParameterList)& theArgsIN) const;

private:

  ActData_RealEvaluatorFunc();

};

#endif
