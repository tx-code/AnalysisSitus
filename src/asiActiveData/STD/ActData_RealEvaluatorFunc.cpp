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

// Own include
#include <ActData_RealEvaluatorFunc.h>

// Active Data includes
#include <ActData_IntParameter.h>
#include <ActData_RealParameter.h>
#include <ActData_TreeFunctionPriority.h>

// Active Data (auxiliary) includes
#include <ActAux_ExprCalculator.h>

// OCCT includes
#include <Standard_ProgramError.hxx>

#undef COUT_DEBUG

//! Default constructor.
ActData_RealEvaluatorFunc::ActData_RealEvaluatorFunc() : ActData_BaseTreeFunction()
{
}

//! Instantiation routine.
//! \return Tree Function instance.
Handle(ActData_RealEvaluatorFunc) ActData_RealEvaluatorFunc::Instance()
{
  return new ActData_RealEvaluatorFunc();
}

//! Returns human-readable name for Evaluation Tree Function.
//! \return Function name.
Standard_CString ActData_RealEvaluatorFunc::GetName() const
{
  return "Real Eval";
}

//! Returns static GUID associated with this kind of Tree Functions.
//! \return requested GUID.
Standard_CString ActData_RealEvaluatorFunc::GUID()
{
  return "04F1DABC-B8A6-4283-990E-0F69BF86A844";
}

//! Non-static accessor for GUID (required by Data Framework).
//! \return requested GUID.
Standard_CString ActData_RealEvaluatorFunc::GetGUID() const
{
  return GUID();
}

//! Returns static heaviness property for Real Evaluation Tree Function.
//! \return always FALSE.
Standard_Boolean ActData_RealEvaluatorFunc::IS_HEAVY()
{
  return Standard_False;
}

//! Returns true if this Tree Function is HEAVY, false -- otherwise.
//! \return always false.
Standard_Boolean ActData_RealEvaluatorFunc::IsHeavy() const
{
  return IS_HEAVY();
}

//! Returns priority of the Tree Function. The bigger the returned value is,
//! the higher is the corresponding priority.
//! \return Tree Function priority ("normal" is the default).
Standard_Integer ActData_RealEvaluatorFunc::Priority() const
{
  return TreeFunctionPriority_High;
}

//! Execution routine. Wraps the actual expression evaluation algorithm for
//! Tree Function mechanism.
//! \param theArgsIN [in] input arguments. The first one is expected to be
//!        a target Real Parameter, while the rest ones are the involved
//!        Variable Parameters.
//! \param theArgsOUT [in] output arguments. Expected a single target Real
//!        Parameter to put the evaluation result into.
//! \param theUserData [in] user data.
//! \param PEntry [in] Progress Entry.
//! \return execution status.
Standard_Integer
  ActData_RealEvaluatorFunc::execute(const Handle(ActAPI_HParameterList)& theArgsIN,
                                     const Handle(ActAPI_HParameterList)& theArgsOUT,
                                     const Handle(Standard_Transient)& theUserData) const
{
  m_progress.SetMessageKey("EVAL_FUNC_STARTING");

  /* ============================
   *  Interpret INPUT parameters
   * ============================ */

  Handle(ActAPI_IUserParameter) aTargetParam_IN = theArgsIN->Value(1);

  // Check and re-pack the variable Parameters
  ActAPI_ParameterList aVarList_IN;
  ActAPI_ParameterList::Iterator anInputIt( *theArgsIN.operator->() );
  anInputIt.Next(); // Skip the first one as it is a target Parameter
  for ( ; anInputIt.More(); anInputIt.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aNextVar = anInputIt.Value();
    aVarList_IN.Append(aNextVar);
  }

  /* =============================
   *  Interpret OUTPUT parameters
   * ============================= */
  
  Handle(ActAPI_IUserParameter) aTargetParam_OUT = theArgsOUT->Value(1);

  /* ==============================================================
   *  Check on self-loops. While it might be Ok to have self-loops
   *  in general case of Tree Function, it is really a defect of
   *  parameterization for Variables. So we filter such cases out
   * ============================================================== */

  for ( Standard_Integer i = 1; i <= aVarList_IN.Length(); i++ )
  {
    Handle(ActAPI_IUserParameter) aNextArg = aVarList_IN.Value(i);
    if ( aNextArg->GetId() == aTargetParam_IN->GetId() )
    {
      return 1; // self-dependent
    }
  }

  /* ============================================================
   *  Assemble a collection of involved variables for calculator
   * ============================================================ */

  ActAPI_VariableList aVars;
  ActAPI_ParameterList::Iterator aVarsIt(aVarList_IN);
  for ( ; aVarsIt.More(); aVarsIt.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aVarParam = aVarsIt.Value();

    // Implicit conversion to ASCII string
    TCollection_AsciiString aVarName = aVarParam->GetName();
    Standard_Integer aVarType = aVarParam->GetParamType();

    Handle(ActAPI_VariableBase) aVar = nullptr;

    if ( aVarType == Parameter_Real )
    {
      Handle(ActData_RealParameter)
        aRealParam = Handle(ActData_RealParameter)::DownCast(aVarParam);
      aVar = ActAPI_VariableReal::Instance( aVarName, aRealParam->GetValue() );
    }
    else if ( aVarType == Parameter_Int )
    {
      Handle(ActData_IntParameter)
        anIntParam = Handle(ActData_IntParameter)::DownCast(aVarParam);
      aVar = ActAPI_VariableInt::Instance( aVarName, anIntParam->GetValue() );
    }
    else
      continue; // Should never happen

    aVars.push_back(aVar);
  }

  /* ===============================
   *  Perform expression evaluation
   * =============================== */

  TCollection_AsciiString anExpr = aTargetParam_IN->GetEvalString();

  TCollection_AsciiString anErr, anErrArg;
  Standard_Integer anErrOffset;
  Handle(ActAux_ExprCalculator)
    aCalculator = Handle(ActAux_ExprCalculator)::DownCast(theUserData);

  Standard_Real aResult = aCalculator->CalcAsReal(anExpr, aVars, anErr,
                                                  anErrArg, anErrOffset);

  if ( !anErr.IsEmpty() )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "EVAL_FUNC_FAILED_WITH_ERROR" << anErr);
    return 1;
  }

  if ( Abs(aResult) == DBL_MAX )
  {
    return 1;
  }

  /* =======================================
   *  Dispatch result into OUTPUT Parameter
   * ======================================= */

  // Transfer result
  if ( aTargetParam_OUT->GetParamType() == Parameter_Real )
    Handle(ActData_RealParameter)::DownCast(aTargetParam_OUT)->SetValue(aResult,
                                                                        MT_Impacted);
  else if ( aTargetParam_OUT->GetParamType() == Parameter_Int )
    Handle(ActData_IntParameter)::DownCast(aTargetParam_OUT)->SetValue( (Standard_Integer) aResult,
                                                                         MT_Impacted );

  m_progress.SendLogMessage(LogInfo(Normal) << "EVAL_FUNC_SUCCESS" << anErr);

  return 0; // OK
}

//! Overrides default validation scheme so that to allow infinite number
//! of INPUT Parameters.
Standard_Boolean
  ActData_RealEvaluatorFunc::validateInput(const Handle(ActAPI_HParameterList)& theArgsIN) const
{
  if ( theArgsIN->Length() == 0 )
    return Standard_False;

  ActAPI_ParameterList::Iterator anInputIt( *theArgsIN.operator->() );
  for ( ; anInputIt.More(); anInputIt.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aNextVar = anInputIt.Value();
    Standard_Integer aNextVarType = aNextVar->GetParamType();

    if ( aNextVarType != Parameter_Int && aNextVarType != Parameter_Real )
      return Standard_False;
  }

  return Standard_True;
}

//! Returns accepted INPUT signature for VALIDATION.
//! \return expected INPUT signature.
ActAPI_ParameterTypeStream ActData_RealEvaluatorFunc::inputSignature() const
{
  return ActAPI_ParameterTypeStream(); // NOT USED
}

//! Returns accepted OUTPUT signature for VALIDATION.
//! \return expected OUTPUT signature.
ActAPI_ParameterTypeStream ActData_RealEvaluatorFunc::outputSignature() const
{
  return ActAPI_ParameterTypeStream() << Parameter_Real << Parameter_Int;
}
