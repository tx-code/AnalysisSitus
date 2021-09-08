//-----------------------------------------------------------------------------
// Created on: February 2012
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
#include <ActData_BaseTreeFunction.h>

// Active Data (auxiliary) includes
#include <ActAux_SpyLog.h>

// ACT Framework includes
#include <ActData_BaseModel.h>
#include <ActData_LogBook.h>
#include <ActData_ParameterFactory.h>
#include <ActData_TreeFunctionParameter.h>
#include <ActData_TreeFunctionPriority.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_ReferenceList.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TFunction_Logbook.hxx>

#undef COUT_DEBUG

//-----------------------------------------------------------------------------
// Base Tree Function
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_BaseTreeFunction::ActData_BaseTreeFunction() : ActAPI_ITreeFunction()
{
  m_driver = new ActData_TreeFunctionDriver();
  m_driver->initFunction(this);
}

//! Returns human-readable name of Tree Function.
//! \return human-readable name.
Standard_CString ActData_BaseTreeFunction::GetName() const
{
  return this->DynamicType()->Name();
}

//! Implements basic execution scheme distinguishing such stages as
//! VALIDATION, actual EXECUTION, etc. If actual execution does not happen,
//! IDLE execution scheme is applied. Idle scheme consists in propagation
//! of VALIDITY and PENDING statuses.
//! \param theArgsIN [in] INPUT Parameters.
//! \param theArgsOUT [in] OUTPUT Parameters.
//! \return execution status.
Standard_Integer
  ActData_BaseTreeFunction::Execute(const Handle(ActAPI_HParameterList)& theArgsIN,
                                    const Handle(ActAPI_HParameterList)& theArgsOUT) const
{
  /* ==========================
   *  Pre-execution validation
   * ========================== */

  if ( !this->validate(theArgsIN, theArgsOUT) )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "TREE_FUNCTION_VALIDATION_FAILED" << this->GetName() );
    return 1; // VALIDATION ERROR
  }

  /* ==================
   *  Execution scheme
   * ================== */

  Standard_Integer aRes = 1;

  // ...
  // Forbid execution if there are any invalid INPUTs which are not enumerated
  // in OUTPUTs. Indeed, if for each invalid INPUT a Tree Function has the
  // correspondent OUTPUT, it means that Tree Function can theoretically
  // fix the initial invalidity (this is broadly used in evaluation mechanism).
  // If, however, Tree Function is executed without such a chance (some invalid
  // INPUT is not covered by itself in the OUTPUT list), we do not allow
  // algorithms to run as they have no possibility to eliminate data defects
  // ...

  Standard_Boolean isRecoValid   = !this->hasUnrecoverableParameters(theArgsIN, theArgsOUT, 1);
  Standard_Boolean isRecoPending =  this->hasUnrecoverableParameters(theArgsIN, theArgsOUT, 2);
  Standard_Boolean isBlocking    =  this->NoPropagation();

  if ( isRecoPending )
  {
    m_progress.SendLogMessage( LogNotice(Normal) << "TREE_FUNCTION_PENDING_INPUTS" << this->GetName() );

    if ( !isBlocking )
      this->propagatePending(theArgsOUT); // IDLE execution

    aRes = !isRecoValid; // We want INVALIDITY propagation for invalid inputs only
  }
  else
  {
    Standard_Boolean isHeavy = this->IsHeavy();
    Standard_Boolean canExecute;

    // Check if HEAVY Tree Function has DEPLOYMENT record in LogBook
    if ( isHeavy )
    {
      Standard_Boolean isUndefinedType;
      Handle(ActAPI_IUserParameter)
        TFuncParam = ActData_ParameterFactory::NewParameterSettle( m_driver->Label(), isUndefinedType );

      const Standard_Boolean isDeployed = ActData_LogBook::IsPendingCursor(TFuncParam);

      if ( isDeployed && !isRecoValid )
        m_progress.SendLogMessage( LogWarn(Normal) << "TREE_FUNCTION_INVALID_UNRECOVERABLE_INPUT" << this->GetName() );

      if ( !isDeployed )
        m_progress.SendLogMessage( LogNotice(Normal) << "TREE_FUNCTION_HEAVY_SKIPPED" << this->GetName() );

      canExecute = isDeployed && isRecoValid;
    }
    else
      canExecute = isRecoValid;

    // Proceed with the actual execution
    if ( canExecute )
    {
      // Accumulate statistics for heavy functions only as heavy functions
      // are called by user rather than by the application itself
      if ( this->IsHeavy() )
        ActAux_SpyLog::Instance()->CallCount( this->GetName() );

      // Execute
      aRes = this->execute(theArgsIN, theArgsOUT, m_UserData);
    }
    else
    {
      if ( !isBlocking )
        this->propagatePending(theArgsOUT); // IDLE execution

      aRes = !isRecoValid; // We want INVALIDITY propagation for invalid inputs only
    }
  }

  // Propagate invalidation wave. Notice that in order to keep invalidation
  // flags in the Data Model, you should not abort transaction in case of
  // Tree Function execution failure
  if ( aRes > 0 && !isBlocking )
    this->propagateInvalid(theArgsOUT); // IDLE execution

  return aRes;
}

//! Gives Tree Function a chance to ask for execution even in case when
//! its inputs are untouched. This is a way of how IMPLICIT parameterization
//! works. Taking advantage of this method you can complexify the chaining
//! logic in unlimited way, so be careful. By default this option is disabled
//! -- method returns FALSE.
//! \param theArgsIN [in] input arguments. All them are guaranteed to be
//!        untouched (not referenced in the LogBook).
//! \param theUserData [in] user data.
//! \return true if your Tree Function asks for execution regardless of
//!         the fact that its arguments are untouched.
Standard_Boolean
  ActData_BaseTreeFunction::MustExecuteIntact(const Handle(ActAPI_HParameterList)& ActData_NotUsed(theArgsIN),
                                              const Handle(Standard_Transient)&    ActData_NotUsed(theUserData)) const
{
  return Standard_False;
}

//! Gives Tree Function a possibility to prevent propagation of
//! pending and invalidity states to its outputs. Use this option
//! with care as this way you will block notifications in execution
//! graph.
//! \return true/false.
Standard_Boolean
  ActData_BaseTreeFunction::NoPropagation() const
{
  return Standard_False;
}

//! Gives Tree Function a possibility to connect its inputs and outputs
//! before Execution Graph is built.
//! \param theOwnerNode [in] Data Node owning the Function.
void ActData_BaseTreeFunction::AutoConnect(const Handle(ActAPI_INode)& ActData_NotUsed(theOwnerNode)) const
{
}

//! Returns priority of the Tree Function. The bigger the returned value is,
//! the higher is the corresponding priority.
//! \return Tree Function priority ("normal" is the default).
Standard_Integer ActData_BaseTreeFunction::Priority() const
{
  return TreeFunctionPriority_Normal;
}

//! Performs validation of the INPUT and OUTPUT Parameters by custom
//! signatures.
//! \param theArgsIN [in] INPUT Parameters.
//! \param theArgsOUT [in] OUTPUT Parameters.
//! \return true if Parameters are OK, false -- otherwise.
Standard_Boolean
  ActData_BaseTreeFunction::validate(const Handle(ActAPI_HParameterList)& theArgsIN,
                                     const Handle(ActAPI_HParameterList)& theArgsOUT) const
{
  if ( this->validateInput(theArgsIN) && this->validateOutput(theArgsOUT) )
    return Standard_True;

  return Standard_False;
}

//! Performs validation of the INPUT Parameters by custom INPUT signature.
//! \param theArgsIN [in] INPUT Parameters.
//! \return true if Parameters are OK, false -- otherwise.
Standard_Boolean
  ActData_BaseTreeFunction::validateInput(const Handle(ActAPI_HParameterList)& theArgsIN) const
{
  Standard_Boolean isOk = this->validateBySignature( theArgsIN, this->inputSignature() );
  if ( !isOk )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "TREE_FUNCTION_INPUT_VALIDATION_FAILED" << this->GetName() );
  }
  return isOk;
}

//! Performs validation of the OUTPUT Parameters by custom OUTPUT signature.
//! \param theArgsOUT [in] OUTPUT Parameters.
//! \return true if Parameters are OK, false -- otherwise.
Standard_Boolean
  ActData_BaseTreeFunction::validateOutput(const Handle(ActAPI_HParameterList)& theArgsOUT) const
{
  Standard_Boolean isOk = this->validateBySignature( theArgsOUT, this->outputSignature() );
  if ( !isOk )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "TREE_FUNCTION_OUTPUT_VALIDATION_FAILED" << this->GetName() );
  }
  return isOk;
}

//! Validates the passed collection of Parameters against the given signature.
//! \param theArgs [in] Parameters to validate.
//! \param theSignature [in] validation scheme.
//! \return true if the Parameter are OK, false -- otherwise.
Standard_Boolean
  ActData_BaseTreeFunction::validateBySignature(const Handle(ActAPI_HParameterList)& theArgs,
                                                const ActAPI_ParameterTypeStream& theSignature) const
{
  if ( theArgs.IsNull() && theSignature.List.IsNull() )
    return Standard_True;

  if ( theArgs->Length() != theSignature.List->Length() )
    return Standard_False; // BAD NUMBER OF OUTPUT PARAMETERS

  for ( Standard_Integer i = 1; i <= theArgs->Length(); ++i )
  {
    const Handle(ActAPI_IUserParameter)& aNextParam = theArgs->Value(i);
    //
    if ( aNextParam.IsNull() )
      return Standard_False; // BAD PARAMETER.

    Standard_Integer anActualType = aNextParam->GetParamType(),
                     anExpectedType = theSignature.List->Value(i);

    if ( anActualType != anExpectedType )
      return Standard_False; // BAD TYPE OF OUTPUT PARAMETER
  }

  return Standard_True;
}

//! Checks whether the given collection of OUTPUT Parameters contains all
//! INVALID or PENDING Parameters from INPUTs. In such a case we can allow Tree
//! Function execution as it has a chance to fix invalid (pending) INPUTs.
//! Otherwise we will forbid execution and propagate invalidation (pending)
//! wave.
//! \param theArgsIN [in] collection of INPUT Parameters to check.
//! \param theArgsOUT [in] collection of OUTPUT Parameters to check.
//! \param theCheckType [in] 1 for validity check, 2 -- for pending.
//! \return true/false.
Standard_Boolean
  ActData_BaseTreeFunction::hasUnrecoverableParameters(const Handle(ActAPI_HParameterList)& theArgsIN,
                                                       const Handle(ActAPI_HParameterList)& theArgsOUT,
                                                       const Standard_Integer theCheckType) const
{
  for ( ActAPI_ParameterList::Iterator it( *theArgsIN.operator->() ); it.More(); it.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aParamIN = it.Value();
    Standard_Boolean isParamOk = Standard_False;

    if ( theCheckType == 1 )
      isParamOk = aParamIN->IsValidData();
    else if ( theCheckType == 2 )
      isParamOk = !aParamIN->IsPendingData();

    if ( !isParamOk )
    {
      // Attempt to find it in OUTPUTs
      Standard_Boolean isRecoverable = Standard_False;
      if ( !theArgsOUT.IsNull() )
      {
        for ( ActAPI_ParameterList::Iterator oit( *theArgsOUT.operator->() ); oit.More(); oit.Next() )
        {
          const Handle(ActAPI_IUserParameter)& aParamOUT = oit.Value();
          if ( aParamIN->GetId() == aParamOUT->GetId() )
          {
            isRecoverable = Standard_True;
            break;
          }
        }
      }

      if ( !isRecoverable )
        return Standard_True;
    }
  }

  return Standard_False;
}

//! Invalidates the given set of Parameters.
//! \param theArgs [in] collection of Parameters to invalidate.
void ActData_BaseTreeFunction::propagateInvalid(const Handle(ActAPI_HParameterList)& theArgs) const
{
  if ( theArgs.IsNull() )
    return;

  for ( ActAPI_ParameterList::Iterator it( *theArgs.operator->() ); it.More(); it.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aParam = it.Value();
    aParam->SetValidity(Standard_False, MT_Impacted);
  }
}

//! Sets the passed Parameters as PENDING.
//! \param theArgs [in] collection of Parameters to set as PENDING.
void ActData_BaseTreeFunction::propagatePending(const Handle(ActAPI_HParameterList)& theArgs) const
{
  if ( theArgs.IsNull() )
    return;

  for ( ActAPI_ParameterList::Iterator it( *theArgs.operator->() ); it.More(); it.Next() )
  {
    const Handle(ActAPI_IUserParameter)& aParam = it.Value();
    aParam->SetPending(Standard_True, MT_Impacted);
  }
}

//-----------------------------------------------------------------------------
// Tree Function Driver
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_TreeFunctionDriver::ActData_TreeFunctionDriver() : TFunction_Driver()
{
}

//! Checks whether some of the Argument Labels is marked as MODIFIED or
//! FORCED for execution.
//! \todo since OCCT 7 Logbook is TDF Attribute, so we can start using it.
//! \return true/false.
Standard_Boolean
  ActData_TreeFunctionDriver::MustExecute(const Handle(TFunction_Logbook)&) const
{
  // Prepare LogBook Data Cursor
  TDF_Label aLogBookSection =
    this->Label().Root().FindChild(ActData_BaseModel::StructureTag_LogBook);
  ActData_LogBook LogBook(aLogBookSection);

  // Tree Function
  if ( LogBook.IsForced( this->Label() ) )
    return Standard_True;

  // Get Arguments
  TDF_LabelList anArgLabels;
  this->Arguments(anArgLabels);

  // Check if the Arguments are touched
  for ( TDF_ListIteratorOfLabelList it(anArgLabels); it.More(); it.Next() )
  {
    TDF_Label anArgLab = it.Value();

    if ( LogBook.IsModified(anArgLab) )
      return Standard_True;
  }

  Handle(ActAPI_HParameterList) aInputParams =
    ActData_ParameterFactory::ParamsByLabelsSettle(anArgLabels);
  return m_func->MustExecuteIntact( aInputParams, m_func->GetUserData() );
}

//! Entry point to Tree Function execution routine.
//! Passed LogBook is not used here as we utilize the global one in any case.
//! \todo since OCCT 7 Logbook is TDF Attribute, so we can start using it.
//! \return execution status.
Standard_Integer
  ActData_TreeFunctionDriver::Execute(Handle(TFunction_Logbook)&) const
{
  // Assemble Parameters
  TDF_LabelList anInputArgLabels; this->Arguments(anInputArgLabels);
  TDF_LabelList anOutputArgLabels; this->Results(anOutputArgLabels);

  Handle(ActAPI_HParameterList) aInputParams =
    ActData_ParameterFactory::ParamsByLabelsSettle(anInputArgLabels);
  Handle(ActAPI_HParameterList) aOutputParams =
    ActData_ParameterFactory::ParamsByLabelsSettle(anOutputArgLabels);

#ifdef COUT_DEBUG
  std::cout << "\nExecuting: number of INPUT parameters: " << aInputParams->Length() << std::endl;
  for ( Standard_Integer i = 1; i <= aInputParams->Length(); i++ )
    std::cout << "         +->>> " << aInputParams->Value(i)->DynamicType()->Name() << std::endl;

  if ( !aOutputParams.IsNull() )
  {
    std::cout << "         | number of OUTPUT parameters: " << aOutputParams->Length() << std::endl;
    for ( Standard_Integer i = 1; i <= aOutputParams->Length(); i++ )
      std::cout << "         +->>> " << aOutputParams->Value(i)->DynamicType()->Name() << std::endl;
    std::cout << "\n";
  }
#endif

  // Call implementer's method
  return m_func->Execute(aInputParams, aOutputParams);
}

//! Declares all argument TDF Labels for this TFunction Driver. This method is
//! used by OCCT TFunction mechanism in order to build the dependency graph.
//! \param theArguments [out] output collection of input arguments.
void ActData_TreeFunctionDriver::Arguments(TDF_LabelList& theArguments) const
{
  Standard_Boolean isUndefinedType;
  Handle(ActAPI_IUserParameter)
    aParam = ActData_ParameterFactory::NewParameterSettle( Parameter_TreeFunction,
                                                           this->Label(),
                                                           isUndefinedType );
  //
  Handle(ActData_TreeFunctionParameter)
    aTreeFuncParam = Handle(ActData_TreeFunctionParameter)::DownCast(aParam);

  if ( aTreeFuncParam.IsNull() )
    Standard_ProgramError::Raise("Invalid Label passed into Function Driver");

  aTreeFuncParam->getArguments(theArguments);
}

//! Declares all resulting TDF Labels for this TFunction Driver. This method is
//! used by OCCT TFunction mechanism in order to build the dependency graph.
//! \param theResults [out] output collection of output arguments.
void ActData_TreeFunctionDriver::Results(TDF_LabelList& theResults) const
{
  Standard_Boolean isUndefinedType;
  Handle(ActAPI_IUserParameter)
    aParam = ActData_ParameterFactory::NewParameterSettle( Parameter_TreeFunction,
                                                           this->Label(),
                                                           isUndefinedType);
  //
  Handle(ActData_TreeFunctionParameter)
    aTreeFuncParam = Handle(ActData_TreeFunctionParameter)::DownCast(aParam);

  if ( aTreeFuncParam.IsNull() )
    Standard_ProgramError::Raise("Invalid Label passed into Function Driver");

  aTreeFuncParam->getResults(theResults);
}

//! Initializes the internal Tree Function.
//! \param theFunc [in] Tree Function instance to set.
void ActData_TreeFunctionDriver::initFunction(const Handle(ActAPI_ITreeFunction)& theFunc)
{
  m_func = theFunc;
}
