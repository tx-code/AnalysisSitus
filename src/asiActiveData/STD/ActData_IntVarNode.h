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

#ifndef ActData_IntVarNode_HeaderFile
#define ActData_IntVarNode_HeaderFile

// Active Data includes
#include <ActData_BaseVarNode.h>
#include <ActData_IntParameter.h>

DEFINE_STANDARD_HANDLE(ActData_IntVarNode, ActData_BaseVarNode)

//! \ingroup AD_DF
//!
//! Simple Node defining its only Integer Parameter as an expressible one.
//! This Node is shipped with Active Data to support Evaluation
//! mechanism. The idea behind this class is to represent Integer Variables
//! involved in Python expressions. Anyway, you are free to use it as
//! just a plain data chunk if you want.
class ActData_IntVarNode : public ActData_BaseVarNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_IntVarNode, ActData_BaseVarNode)

  // Automatic registration of Node instance in the Nodal Factory.
  DEFINE_NODE_FACTORY(ActData_IntVarNode, Instance)

public:

  ActData_EXPORT static Handle(ActAPI_INode) Instance();

// Initialization routines:
public:

  ActData_EXPORT void Init(const TCollection_AsciiString& theName,
                           const Standard_Integer theValue);

// Setters & Getters:
public:

  ActData_EXPORT void
    SetValue(const Standard_Integer theValue);

  ActData_EXPORT Standard_Integer
    GetValue() const;

protected:

  //! Allocation is allowed only via Instance method.
  ActData_EXPORT ActData_IntVarNode();

};

#endif
