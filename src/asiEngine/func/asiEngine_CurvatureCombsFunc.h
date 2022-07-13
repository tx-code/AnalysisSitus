//-----------------------------------------------------------------------------
// Created on: 13 July 2022
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

#ifndef asiEngine_CurvatureCombsFunc_HeaderFile
#define asiEngine_CurvatureCombsFunc_HeaderFile

// asiEngine includes
#include <asiEngine.h>

// Active Data includes
#include <ActData_BaseTreeFunction.h>

// Forward declarations.
class asiEngine_Model;

//-----------------------------------------------------------------------------

//! Tree function for curvature analysis
class asiEngine_CurvatureCombsFunc : public ActData_BaseTreeFunction
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiEngine_CurvatureCombsFunc, ActData_BaseTreeFunction)

public:

  //! Instantiation routine.
  //! \return Tree Function instance.
  static Handle(asiEngine_CurvatureCombsFunc) Instance()
  {
    return new asiEngine_CurvatureCombsFunc();
  }

  //! Static accessor for the GUID associated with the Tree Function.
  //! \return requested GUID.
  static const char* GUID()
  {
    return "F2E1CD60-55C2-4AA6-80FF-3A8008AD0F93";
  }

  //! Accessor for the GUID.
  //! \return GUID.
  virtual const char* GetGUID() const
  {
    return GUID();
  }

public:

  //! Returns true if this Tree Function is HEAVY, false -- otherwise.
  //! \return always false.
  virtual bool IsHeavy() const
  {
    return false;
  }

  //! \return human-friendly function name.
  virtual const char* GetName() const
  {
    return "Check curvature";
  }

private:

  //! Executes Tree Function on the given input and output arguments.
  //! \param[in]      inputs   input Parameters.
  //! \param[in, out] outputs  output Parameters.
  //! \param[in]      userData custom user data.
  //! \return execution status.
  virtual int
    execute(const Handle(ActAPI_HParameterList)& inputs,
            const Handle(ActAPI_HParameterList)& outputs,
            const Handle(Standard_Transient)&    userData) const;

  //! \return expected input signature.
  virtual ActAPI_ParameterTypeStream
    inputSignature() const;

  //! \return expected output signature.
  virtual ActAPI_ParameterTypeStream
    outputSignature() const;

private:

  asiEngine_CurvatureCombsFunc() = default; //!< Default ctor.

};

#endif
