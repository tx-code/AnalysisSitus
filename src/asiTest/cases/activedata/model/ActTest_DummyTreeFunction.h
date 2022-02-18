//-----------------------------------------------------------------------------
// Created on: 17 February 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

#ifndef ActTest_DummyTreeFunction_HeaderFile
#define ActTest_DummyTreeFunction_HeaderFile

// Active Data includes
#include <ActData_BaseTreeFunction.h>

DEFINE_STANDARD_HANDLE(ActTest_DummyTreeFunction, ActData_BaseTreeFunction)

//! \ingroup AD_TEST
//!
//! Dummy Tree Function doing nothing with data.
class ActTest_DummyTreeFunction : public ActData_BaseTreeFunction
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActTest_DummyTreeFunction, ActData_BaseTreeFunction)

public:

  static Handle(ActTest_DummyTreeFunction)
    Instance();

  static Standard_CString
    GUID();

  virtual Standard_CString
    GetGUID() const;

  //! Returns true if this Tree Function is HEAVY, false -- otherwise.
  //! \return always false.
  inline virtual Standard_Boolean IsHeavy() const
  {
    return Standard_False;
  }

private:

  virtual Standard_Integer
    execute(const Handle(ActAPI_HParameterList)&,
            const Handle(ActAPI_HParameterList)&,
            const Handle(Standard_Transient)&) const;


  virtual ActAPI_ParameterTypeStream
    inputSignature() const;

  virtual ActAPI_ParameterTypeStream
    outputSignature() const;

private:

  ActTest_DummyTreeFunction();

};

#endif
