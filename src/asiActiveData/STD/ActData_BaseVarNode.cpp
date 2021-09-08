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

// Own include
#include <ActData_BaseVarNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------
// Class: ActData_BaseVarNode
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_BaseVarNode::ActData_BaseVarNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name, Param_Name);
}

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString ActData_BaseVarNode::GetName()
{
  return ActData_ParameterFactory::AsName( this->Parameter(Param_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param theName [in] name to set.
void ActData_BaseVarNode::SetName(const TCollection_ExtendedString& theName)
{
  ActData_ParameterFactory::AsName( this->Parameter(Param_Name) )->SetValue(theName);
}

//! Sets variable name.
//! \param theName [in] variable name.
void ActData_BaseVarNode::SetVariableName(const TCollection_AsciiString& Name)
{
  this->Parameter(Param_Value)->SetName(Name);
}

//! Accessor for the variable name.
//! \return variable name.
TCollection_AsciiString ActData_BaseVarNode::GetVariableName() const
{
  return TCollection_AsciiString( this->Parameter(Param_Value)->GetName() );
}

//! Renames the variable affecting all dependent Nodes by modifying their
//! evaluation strings. Notice that this method is designed to propagate
//! changes on already connected Parameters only. Those Parameters which
//! refer to the variable with the NewName but not yet connected to it (i.e.
//! those having "dead" evaluation strings) are not affected.
//! \param NewName [in] new name to set.
void ActData_BaseVarNode::RenameConnected(const TCollection_AsciiString& NewName)
{
  TCollection_AsciiString anOldName = this->GetVariableName();

  /* ===========================================================
   *  Make a replacement in evaluation strings of Input Readers
   * =========================================================== */

  this->replaceInEvalStrings(anOldName, NewName);

  /* ================================================
   *  Change actual name of the underlying Parameter
   * ================================================ */

  this->SetVariableName(NewName);
}

//! Cleans up the evaluation strings of the dependent Variable Nodes.
void ActData_BaseVarNode::beforeRemove()
{
  TCollection_AsciiString aName = this->GetVariableName();
  this->replaceInEvalStrings(aName, "", Standard_True);
}

//! Performs string substitutions in the dependent Parameters.
//! \param What [in] what to find & replace.
//! \param With [in] replacement string.
//! \param isCompleteErase [in] forces erasing of the evaluation strings of
//!        the dependent Parameters.
void ActData_BaseVarNode::replaceInEvalStrings(const TCollection_AsciiString& What,
                                               const TCollection_AsciiString& With,
                                               const Standard_Boolean isCompleteErase)
{
  Handle(ActAPI_HParameterList) anInputReaders = this->GetInputReaders();
  ActAPI_ParameterList::Iterator anIt( *anInputReaders.operator->() );
  for ( ; anIt.More(); anIt.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aParam = anIt.Value();
    Handle(ActData_TreeFunctionParameter)
      aTreeFuncParam = Handle(ActData_TreeFunctionParameter)::DownCast(aParam);

    if ( aTreeFuncParam.IsNull() )
      continue;

    Handle(ActAPI_HParameterList) anArgParams = aTreeFuncParam->Arguments();
    ActAPI_ParameterList::Iterator anArgIt( *anArgParams.operator->() );
    for ( ; anArgIt.More(); anArgIt.Next() )
    {
      const Handle(ActAPI_IUserParameter)& anArgParam = anArgIt.Value();
      ActData_Utils::ReplaceEvaluationString(anArgParam, What, With, isCompleteErase);
    }
  }
}
