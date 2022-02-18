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


#ifndef ActTest_StubANode_HeaderFile
#define ActTest_StubANode_HeaderFile

// Active Data includes
#include <ActData_BaseNode.h>

//! \ingroup AD_TEST
//!
//! Implementation of Data Node for unit tests.
class ActTest_StubANode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActTest_StubANode, ActData_BaseNode)

  // Automatic registration of Node instance in the Nodal Factory.
  DEFINE_NODE_FACTORY(ActTest_StubANode, Instance)

public:

  //! IDs of the underlying Parameters.
  enum ParamId
  {
    PID_Name = ActData_BaseNode::UserParam_Last,
    PID_DummyShapeA, //!< Let it be some topological shape here...
    PID_DummyShapeB, //!< Let's have another shape...
    PID_Real,        //!< And some dummy real value...
    PID_TFunc,       //!< Plus sole Tree Function to test alternative dependencies...
    PID_Ref          //!< Plus plain reference to other Parameters...
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

   void
    Init(const TopoDS_Shape& theShapeA,
         const TopoDS_Shape& theShapeB,
         const Standard_Real theVal);

   TopoDS_Shape
    GetShapeA() const;

   TopoDS_Shape
    GetShapeB() const;

   Standard_Real
    GetValue() const;

   Standard_Boolean
    HasConnectedFunction() const;

protected:

  //! Allocation is allowed only via Instance method.
  ActTest_StubANode();

};

#endif
