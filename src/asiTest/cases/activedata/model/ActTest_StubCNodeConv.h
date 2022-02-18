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

#ifndef ActTest_StubCNodeConv_HeaderFile
#define ActTest_StubCNodeConv_HeaderFile

// Active Data includes
#include <ActData_BaseNode.h>

DEFINE_STANDARD_HANDLE(ActTest_StubCNodeConv, ActData_BaseNode)

//! \ingroup AD_TEST
//!
//! C test Node representing expected data format after conversion.
class ActTest_StubCNodeConv : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActTest_StubCNodeConv, ActData_BaseNode)

  // Automatic registration of Node instance in the Nodal Factory.
  DEFINE_NODE_FACTORY(ActTest_StubCNodeConv, Instance)

public:

  //! IDs of the underlying Parameters.
  enum ParamId
  {
    /* Int            */ PID_Int = 100,
    /* Bool           */ PID_Bool,
    /* Int            */ PID_NEWInt1,
    /* Shape          */ PID_Shape,
    /* Real Array     */ PID_RealArray,
    /* Tree Function  */ PID_TFunc,
    /* ASCII String   */ PID_AStr,
    /* Bool Array     */ PID_BoolArray,
    /* String Array   */ PID_StrArray,
    /* Complex Array  */ PID_ComplexArray,
    /* Int Array      */ PID_IntArray,
    /* Int            */ PID_NEWInt2,
    /* Reference List */ PID_RefList,
    /* Group          */ PID_Group,
    /* Mesh           */ PID_Mesh,
    /* Int            */ PID_NEWInt3,
    /* Reference      */ PID_Ref,
    /* Selection      */ PID_Selection,
    /* Timestamp      */ PID_TimeStamp,
    /* Int            */ PID_NEWInt4
  };

public:

  static Handle(ActAPI_INode) Instance();

// Generic accessors:
public:

  virtual TCollection_ExtendedString
    GetName();
  
  virtual void
    SetName(const TCollection_ExtendedString& theName);

// Initialization and accessors:
public:

  void Init();

protected:

  //! Allocation is allowed only via Instance method.
  ActTest_StubCNodeConv();

};

#endif
