//-----------------------------------------------------------------------------
// Created on: May 2012
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

#ifndef ActData_BaseVarNode_HeaderFile
#define ActData_BaseVarNode_HeaderFile

// Active Data includes
#include <ActData_BaseNode.h>
#include <ActData_Common.h>
#include <ActData_Utils.h>

DEFINE_STANDARD_HANDLE(ActData_BaseVarNode, ActData_BaseNode)

//! \ingroup AD_DF
//!
//! Base class for Data Nodes playing as Variables in embedded ACT Data
//! Framework evaluation mechanism.
class ActData_BaseVarNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BaseVarNode, ActData_BaseNode)

public:

  //! Contained Parameters.
  enum ParamId
  {
    Param_Name = ActData_BaseNode::UserParam_Last, //!< Name of the Node, not variable (!)
    Param_Value, //!< Variable's value.
    Param_Last = Param_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

// Generic naming support:
public:

  ActData_EXPORT virtual TCollection_ExtendedString
    GetName();

  ActData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& theName);

// Setters & Getters:
public:

  ActData_EXPORT void
    SetVariableName(const TCollection_AsciiString& Name);

  ActData_EXPORT TCollection_AsciiString
    GetVariableName() const;

// Service methods:
public:

  ActData_EXPORT virtual void
    RenameConnected(const TCollection_AsciiString& NewName);

protected:

  //! Allocation of base class prohibited.
  ActData_EXPORT ActData_BaseVarNode();

protected:

  ActData_EXPORT virtual void
    beforeRemove();

private:

  void replaceInEvalStrings(const TCollection_AsciiString& What,
                            const TCollection_AsciiString& With = TCollection_AsciiString(),
                            const Standard_Boolean isCompleteErase = Standard_False);

  void replaceRecursive(TCollection_AsciiString& Source,
                        const TCollection_AsciiString& What,
                        const TCollection_AsciiString& With);

};

#endif
