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

// Own include
#include <ActTest_DummyTreeFunction.h>

//-----------------------------------------------------------------------------
// Implementation of Tree Function for testing purposes
//-----------------------------------------------------------------------------

//! Default constructor.
ActTest_DummyTreeFunction::ActTest_DummyTreeFunction()
{}

//! Instantiation routine.
//! \return Tree Function instance.
Handle(ActTest_DummyTreeFunction) ActTest_DummyTreeFunction::Instance()
{
  return new ActTest_DummyTreeFunction();
}

//! Static accessor for the GUID associated with the Tree Function.
//! \return requested GUID.
Standard_CString ActTest_DummyTreeFunction::GUID()
{
  return "A7E0D6DA-AC44-44D8-B9A0-03F0AF6F814B";
}

//! Accessor for the GUID.
//! \return GUID.
Standard_CString ActTest_DummyTreeFunction::GetGUID() const
{
  return GUID();
}

//! Executes the Tree Function.
Standard_Integer
  ActTest_DummyTreeFunction::execute(const Handle(ActAPI_HParameterList)&,
                                     const Handle(ActAPI_HParameterList)&,
                                     const Handle(Standard_Transient)&) const
{
  // Do nothing...
  return 0; // SUCCESS
}

//! Returns signature for validation of INPUT Parameters.
//! \return signature.
ActAPI_ParameterTypeStream
  ActTest_DummyTreeFunction::inputSignature() const
{
  return ActAPI_ParameterTypeStream();
}

//! Returns signature for validation of OUTPUT Parameters.
//! \return signature.
ActAPI_ParameterTypeStream
  ActTest_DummyTreeFunction::outputSignature() const
{
  return ActAPI_ParameterTypeStream();
}
