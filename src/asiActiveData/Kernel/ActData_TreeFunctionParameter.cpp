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
#include <ActData_TreeFunctionParameter.h>

// Active Data includes
#include <ActData_BaseTreeFunction.h>
#include <ActData_ParameterFactory.h>
#include <ActData_Utils.h>

// OCCT includes
#include <Standard_ProgramError.hxx>
#include <TDF_ListIteratorOfLabelList.hxx>
#include <TFunction_Driver.hxx>
#include <TFunction_GraphNode.hxx>
#include <TFunction_Function.hxx>
#include <TFunction_IFunction.hxx>
#include <TFunction_Scope.hxx>

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Hidden constructor.
ActData_TreeFunctionParameter::ActData_TreeFunctionParameter() : ActData_UserParameter()
{
}

//! Ensures correct construction of the Parameter object, e.g. prevents
//! allocating it in stack memory.
//! \return Parameter instance.
Handle(ActData_TreeFunctionParameter) ActData_TreeFunctionParameter::Instance()
{
  return new ActData_TreeFunctionParameter();
}

//! Sets OCCT TFunction Driver GUID.
//! \param theGUID [in] TFunction Driver GUID to set.
void ActData_TreeFunctionParameter::SetDriverGUID(const Standard_GUID& theGUID)
{
  // Add global OCAF Function Scope by simple accessing it
  Handle(TFunction_Scope) aFuncScope = TFunction_Scope::Set(m_label);

  // Set TFunction_Function attribute initialized with the passed GUID.
  // Also set TFunction_GraphNode for dependency graph (the graph itself is
  // not rebuilt here) and registers the function in the global scope
  // for this document
  TFunction_IFunction::NewFunction(m_label, theGUID);
}

//! Accessor for the Driver GUID.
//! \param theGUID [out] requested GUID if any.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_TreeFunctionParameter::GetDriverGUID(Standard_GUID& theGUID) const
{
  Handle(TFunction_Function) aFuncAttr;
  if ( !m_label.FindAttribute(TFunction_Function::GetID(), aFuncAttr) )
    return Standard_False;

  theGUID = aFuncAttr->GetDriverGUID();
  return Standard_True;
}

//! Adds the passed Parameter as an argument for Tree Function.
//! \param theParam [in] Parameter to add.
void ActData_TreeFunctionParameter::AddArgument(const Handle(ActAPI_IUserParameter)& theParam)
{
  Handle(ActData_UserParameter) aBaseParam =
    Handle(ActData_UserParameter)::DownCast(theParam);

  TDF_Label anArgumentsLab = m_label.FindChild(DS_Arguments);

  Handle(TDataStd_ReferenceList) aRefsAttr;
  if ( !anArgumentsLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefsAttr) )
    aRefsAttr = TDataStd_ReferenceList::Set(anArgumentsLab);

  aRefsAttr->Append(aBaseParam->m_label);
}

//! Checks whether the passed Parameter's Label is enumerated among declared Tree
//! Function arguments.
//! \param theParamLab [in] Parameter's root to check.
//! \return true/false.
Standard_Boolean
  ActData_TreeFunctionParameter::HasArgument(const TDF_Label& theParamLab) const
{
  TDF_LabelList anArgLabels;
  this->getArguments(anArgLabels);
  for ( TDF_ListIteratorOfLabelList aLabIt(anArgLabels); aLabIt.More(); aLabIt.Next() )
  {
    TDF_Label& aLab = aLabIt.Value();
    if ( ActData_Utils::GetEntry(aLab) == ActData_Utils::GetEntry(theParamLab) )
      return Standard_True;
  }
  return Standard_False;
}

//! Checks whether the passed Parameter is enumerated among declared Tree
//! Function arguments.
//! \param theParam [in] Parameter to check.
//! \return true/false.
Standard_Boolean
  ActData_TreeFunctionParameter::HasArgument(const Handle(ActAPI_IUserParameter)& theParam) const
{
  return this->HasArgument( theParam->RootLabel() );
}

//! Adds the passed Parameter as a resulting one for Tree Function.
//! \param theParam [in] Parameter to add.
void ActData_TreeFunctionParameter::AddResult(const Handle(ActAPI_IUserParameter)& theParam)
{
  Handle(ActData_UserParameter) aBaseParam =
    Handle(ActData_UserParameter)::DownCast(theParam);

  TDF_Label aResultsLab = m_label.FindChild(DS_Results);

  Handle(TDataStd_ReferenceList) aRefsAttr;
  if ( !aResultsLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefsAttr) )
    aRefsAttr = TDataStd_ReferenceList::Set(aResultsLab);

  aRefsAttr->Append(aBaseParam->m_label);
}

//! Checks whether the passed Parameter's Label is enumerated among declared Tree
//! Function results.
//! \param theParamLab [in] Parameter's root to check.
//! \return true/false.
Standard_Boolean
  ActData_TreeFunctionParameter::HasResult(const TDF_Label& theParamLab) const
{
  TDF_LabelList aResLabels;
  this->getResults(aResLabels);
  for ( TDF_ListIteratorOfLabelList aLabIt(aResLabels); aLabIt.More(); aLabIt.Next() )
  {
    TDF_Label& aLab = aLabIt.Value();
    if ( ActData_Utils::GetEntry(aLab) == ActData_Utils::GetEntry(theParamLab) )
      return Standard_True;
  }
  return Standard_False;
}

//! Checks whether the passed Parameter is enumerated among declared Tree
//! Function results.
//! \param theParam [in] Parameter to check.
//! \return true/false.
Standard_Boolean
  ActData_TreeFunctionParameter::HasResult(const Handle(ActAPI_IUserParameter)& theParam) const
{
  return this->HasResult( theParam->RootLabel() );
}

//! Disconnects Tree Function releasing all data hooked onto the corresponding
//! OCAF sub-tree.
//! \param toKillCompletely [in] if true, the Tree Function will be erased
//!        from Execution Graph. Otherwise, it will just loose its Parameters.
void ActData_TreeFunctionParameter::Disconnect(const Standard_Boolean toKillCompletely)
{
  if ( toKillCompletely )
    ActData_Utils::RemoveWithReferences( m_label, Standard_False );

  ActData_Utils::RemoveWithReferences( this->getArgumentsLabel() );
  ActData_Utils::RemoveWithReferences( this->getResultsLabel() );
}

//! Equivalent to Disconnect(false).
void ActData_TreeFunctionParameter::DisconnectSoft()
{
  this->Disconnect(Standard_False);
}

//! Checks whether Tree Function is connected or not.
//! \return true/false.
Standard_Boolean ActData_TreeFunctionParameter::IsConnected() const
{
  if ( !ActData_Utils::CheckLabelAttr( m_label, -1,
                                       TFunction_Function::GetID() ) )
    return Standard_False;

  if ( !ActData_Utils::CheckLabelAttr( m_label, -1,
                                       TFunction_GraphNode::GetID() ) )
    return Standard_False;

  if ( !ActData_Utils::CheckLabelAttr( m_label, DS_Arguments,
                                       TDataStd_ReferenceList::GetID() ) )
    return Standard_False;

  // NOTICE: we do not check RESULTS as they might be empty by Data
  //         Framework convention. This can be useful for IMPLICITLY
  //         parameterized LEAF Tree Functions

  return Standard_True;
}

//! Returns INPUT Parameters.
//! \return INPUT Parameters.
Handle(ActAPI_HParameterList) ActData_TreeFunctionParameter::Arguments() const
{
  TDF_LabelList anArgLabels;
  this->getArguments(anArgLabels);
  return ActData_ParameterFactory::ParamsByLabelsSettle(anArgLabels);
}

//! Returns OUTPUT Parameters.
//! \return OUTPUT Parameters.
Handle(ActAPI_HParameterList) ActData_TreeFunctionParameter::Results() const
{
  TDF_LabelList aResultLabels;
  this->getResults(aResultLabels);
  return ActData_ParameterFactory::ParamsByLabelsSettle(aResultLabels);
}

//! Returns the heaviness property of the wrapped Tree Function.
//! \return true/false.
Standard_Boolean ActData_TreeFunctionParameter::IsHeavyFunction() const
{
  // Build function interface under the label containing TFunction_Function attribute
  TFunction_IFunction aFuncInterface(m_label);

  if ( this->IsConnected() )
  {
    // Access Tree Function Driver
    Handle(ActData_TreeFunctionDriver) aFuncDriver =
      Handle(ActData_TreeFunctionDriver)::DownCast( aFuncInterface.GetDriver() );

    return aFuncDriver->GetFunction()->IsHeavy();
  }

  return Standard_False;
}

//! Checks whether the Tree Function instance wrapped by this Parameter has
//! any inputs in status PENDING.
//! \return true/false.
Standard_Boolean ActData_TreeFunctionParameter::HasPendingArguments() const
{
  return this->hasPending( this->Arguments() );
}

//! Checks whether the Tree Function instance wrapped by this Parameter has
//! any outputs in status PENDING.
//! \return true/false.
Standard_Boolean ActData_TreeFunctionParameter::HasPendingResults() const
{
  return this->hasPending( this->Results() );
}

//! Returns internal OCAF Reference List Attribute used to store Tree Function
//! arguments.
//! \return requested OCAF Attribute.
Handle(TDataStd_ReferenceList) ActData_TreeFunctionParameter::getArgumentsAttr() const
{
  TDF_Label anArgsLab = this->getArgumentsLabel();
  if ( anArgsLab.IsNull() )
    return NULL;

  Handle(TDataStd_ReferenceList) aRefsAttr;
  anArgsLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefsAttr);

  return aRefsAttr;
}

//! Returns internal OCAF Reference List Attribute used to store Tree Function
//! results.
//! \return requested OCAF Attribute.
Handle(TDataStd_ReferenceList) ActData_TreeFunctionParameter::getResultsAttr() const
{
  TDF_Label aResLab = this->getResultsLabel();
  if ( aResLab.IsNull() )
    return NULL;

  Handle(TDataStd_ReferenceList) aRefsAttr;
  aResLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefsAttr);

  return aRefsAttr;
}

//! Returns internal OCAF Label used to store Tree Function arguments.
//! \return requested OCAF Label.
TDF_Label ActData_TreeFunctionParameter::getArgumentsLabel() const
{
  return m_label.FindChild(DS_Arguments, Standard_False);
}

//! Returns internal OCAF Label used to store Tree Function results.
//! \return requested OCAF Label.
TDF_Label ActData_TreeFunctionParameter::getResultsLabel() const
{
  return m_label.FindChild(DS_Results, Standard_False);
}

//! Returns OCAF Labels playing as references to Tree Function arguments.
//! \param theLabelList [out] resulting Labels.
void ActData_TreeFunctionParameter::getArguments(TDF_LabelList& theLabelList) const
{
  TDF_Label aArgsLab = m_label.FindChild(DS_Arguments, Standard_False);

  if ( aArgsLab.IsNull() )
    return;

  Handle(TDataStd_ReferenceList) aRefList;
  if ( aArgsLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefList) )
    theLabelList = aRefList->List();
}

//! Returns OCAF Labels playing as references to Tree Function results.
//! \param theLabelList [out] resulting Labels.
void ActData_TreeFunctionParameter::getResults(TDF_LabelList& theLabelList) const
{
  TDF_Label aResultsLab = m_label.FindChild(DS_Results, Standard_False);

  if ( aResultsLab.IsNull() )
    return;

  Handle(TDataStd_ReferenceList) aRefList;
  if ( aResultsLab.FindAttribute(TDataStd_ReferenceList::GetID(), aRefList) )
    theLabelList = aRefList->List();
}

//! Checks whether the passed list of Parameters contains any PENDING ones.
//! \param theParams [in] list of Parameters to check.
//! \return true/false.
Standard_Boolean ActData_TreeFunctionParameter::hasPending(const Handle(ActAPI_HParameterList)& theParams) const
{
  if ( theParams.IsNull() )
    return Standard_False;

  for ( ActAPI_ParameterList::Iterator pit( *theParams.operator->() ); pit.More(); pit.Next() )
  {
    const Handle(ActAPI_IUserParameter)& P = pit.Value();

    if ( P.IsNull() || !P->IsWellFormed() )
      continue;

    if ( P->IsPendingData() )
      return Standard_True;
  }
  return Standard_False;
}

//! Checks if this Parameter object is mapped onto CAF data structure in a
//! correct way.
//! \return true if the object is well-formed, false -- otherwise.
Standard_Boolean ActData_TreeFunctionParameter::isWellFormed() const
{
  // No additional checks as Tree Function can exist in disconnected state
  return Standard_True;
}

//! Returns Parameter type.
//! \return Parameter type.
Standard_Integer ActData_TreeFunctionParameter::parameterType() const
{
  return Parameter_TreeFunction;
}

//-----------------------------------------------------------------------------
// DTO construction
//-----------------------------------------------------------------------------

//! Populates Parameter from the passed DTO.
//! \param theDTO [in] DTO to source data from.
//! \param theModType [in] modification type.
//! \param doResetValidity [in] indicates whether validity flag must be
//!        reset or not.
//! \param doResetPending [in] indicates whether pending flag must be reset
//!        or not.
void ActData_TreeFunctionParameter::setFromDTO(const Handle(ActData_ParameterDTO)& theDTO,
                                               const ActAPI_ModificationType       ActData_NotUsed(theModType),
                                               const Standard_Boolean              ActData_NotUsed(doResetValidity),
                                               const Standard_Boolean              ActData_NotUsed(doResetPending))
{
  Handle(ActData_TreeFunctionDTO) MyDTO = Handle(ActData_TreeFunctionDTO)::DownCast(theDTO);

  // Disconnect if already connected
  if ( this->IsConnected() )
    this->Disconnect();

  // Populate arguments
  if ( !MyDTO->Arguments.IsNull() )
    for ( Standard_Integer i = 1; i <= MyDTO->Arguments->Length(); ++i )
      this->AddArgument( MyDTO->Arguments->Value(i) );

  // Populate results
  if ( !MyDTO->Results.IsNull() )
    for ( Standard_Integer i = 1; i <= MyDTO->Results->Length(); ++i )
      this->AddResult( MyDTO->Results->Value(i) );

  // Populate GUID
  Standard_UUID UUID = MyDTO->DriverGUID.ToUUID();
  if ( UUID.Data1 && UUID.Data2 && UUID.Data3 && UUID.Data4 )
    this->SetDriverGUID(MyDTO->DriverGUID);
}

//! Creates and populates DTO.
//! \param theGID [in] ready-to-use GID for DTO.
//! \return constructed DTO instance.
Handle(ActData_ParameterDTO)
  ActData_TreeFunctionParameter::createDTO(const ActAPI_ParameterGID& theGID)
{
  Handle(ActData_TreeFunctionDTO) aRes = new ActData_TreeFunctionDTO(theGID);

  // ...
  // Transfer data from OCAF structures to DTO
  // ...

  // Pass arguments
  Handle(ActAPI_HParameterList) ArgList = this->Arguments();
  if ( !ArgList.IsNull() )
  {
    aRes->Arguments = new ActAPI_HParameterList;
    for ( Standard_Integer i = 1; i <= ArgList->Length(); ++i )
      aRes->Arguments->Append( ArgList->Value(i) );
  }

  // Pass results
  Handle(ActAPI_HParameterList) ResList = this->Results();
  if ( !ResList.IsNull() )
  {
    aRes->Results = new ActAPI_HParameterList;
    for ( Standard_Integer i = 1; i <= ResList->Length(); ++i )
      aRes->Results->Append( ResList->Value(i) );
  }

  // Pass Driver GUID
  Standard_GUID GUID;
  if ( this->GetDriverGUID(GUID) )
    aRes->DriverGUID = GUID;

  return aRes;
}
