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

#ifndef ActData_FuncExecutionCtx_HeaderFile
#define ActData_FuncExecutionCtx_HeaderFile

// Active Data includes
#include <ActData_Common.h>
#include <ActData_TreeFunctionParameter.h>

// Active Data (API) includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>
#include <ActAPI_TxData.h>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <NCollection_Handle.hxx>
#include <NCollection_Map.hxx>
#include <NCollection_List.hxx>
#include <TFunction_GraphNode.hxx>

// Active Data forward declarations
class ActData_BaseModel;
class ActData_TreeFunctionParameter;

DEFINE_STANDARD_HANDLE(ActData_FuncExecutionCtx, Standard_Transient)

//! \ingroup AD_DF
//!
//! Class representing execution context for Tree Function mechanism.
class ActData_FuncExecutionCtx : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_FuncExecutionCtx, Standard_Transient)

public:

  //! Custom data associated with each function type. Tree Function concept
  //! presumes that the internal workflow uses input and output Parameters
  //! to perform a dedicated algorithm. It means that the arguments for the
  //! algorithm are always data chunks coming from data model. However, in
  //! many cases algorithm might need some service-layer stuff to proceed, e.g.
  //! progress indicators & collectors, etc. Such data is passed to
  //! Tree Functions by means of a specialized data map. Note that unlike
  //! input & output Parameters, this kind of data is always optional and
  //! exists in a single instance for each type of Tree Function.
  typedef NCollection_DataMap<Standard_GUID,
                              Handle(Standard_Transient),
                              ActiveData::GuidHasher> FunctionDataMap;
  //
  typedef NCollection_Shared<FunctionDataMap> HFunctionDataMap;

// Construction:
public:

  ActData_EXPORT
    ActData_FuncExecutionCtx();

// User data:
public:

  ActData_EXPORT void
    BindUserData(const Standard_GUID& theFuncGUID,
                 const Handle(Standard_Transient)& theUserData);

  ActData_EXPORT void
    UnBindUserData(const Standard_GUID& theFuncGUID);

  ActData_EXPORT Handle(Standard_Transient)
    AccessUserData(const Standard_GUID& theFuncGUID);

  ActData_EXPORT void
    ReleaseUserData();

// Transaction data:
public:

  ActData_EXPORT void
    SetTxData(const ActAPI_TxData& theData);

  ActData_EXPORT const ActAPI_TxData&
    AccessTxData() const;

  ActData_EXPORT void
    ReleaseTxData();

// Managing dependency graph:
public:

  ActData_EXPORT static void
    UpdateDependencies(const Handle(ActData_BaseModel)& theModel);

  ActData_EXPORT static Standard_Integer
    CheckDependencyGraph(const Handle(ActData_BaseModel)& theModel);

  ActData_EXPORT static Standard_Integer
    CheckDependencyGraph(const Handle(ActData_BaseModel)& theModel,
                         Handle(ActAPI_HParameterList)& theFaultyParams);

// Progress notification:
public:

  ActData_EXPORT void
    SetProgressNotifier(const Handle(ActAPI_IProgressNotifier)& thePNotifier);

  ActData_EXPORT void
    SetPlotter(const Handle(ActAPI_IPlotter)& thePlotter);

  ActData_EXPORT const Handle(ActAPI_IProgressNotifier)&
    ProgressNotifier() const;

  ActData_EXPORT const Handle(ActAPI_IPlotter)&
    Plotter() const;

  ActData_EXPORT Standard_Boolean
    IsProgressNotifierOn() const;

  ActData_EXPORT Standard_Boolean
    IsPlotterOn() const;

  ActData_EXPORT void
    ProgressNotifierOn();

  ActData_EXPORT void
    PlotterOn();

  ActData_EXPORT void
    ProgressNotifierOff();

  ActData_EXPORT void
    PlotterOff();

  ActData_EXPORT void
    FreezeGraph();

  ActData_EXPORT Standard_Boolean
    IsGraphFrozen() const;

  ActData_EXPORT void
    UnFreezeGraph();

// Forced execution:
public:

  ActData_EXPORT void
    Force(const Handle(ActData_TreeFunctionParameter)& theParam);

// Heavy deployment:
public:

  ActData_EXPORT void
    Deploy(const Handle(ActData_TreeFunctionParameter)& theParam);

// Services:
public:

  ActData_EXPORT void
    ForceDeployPropagation(const Handle(ActData_BaseModel)& theModel);

public:

  //! Sets a collection of Tree Function Parameters requiring "heavy"
  //! execution.
  //! \param theRoots [in] list to set.
  void SetFunctions2Deploy(const Handle(ActAPI_HParameterList)& theRoots)
  {
    m_functions2Deploy = theRoots;
  }

  //! Cleans up the collection of Tree Function Parameters registered for
  //! "heavy" execution.
  void CleanFunctions2Deploy()
  {
    m_functions2Deploy.Nullify();
  }

// Transient data:
private:

  //! Data map of user-specific data associated with Tree Functions.
  Handle(HFunctionDataMap) m_funcDataMap;

  //! Global Progress Notifier for all Tree Functions.
  Handle(ActAPI_IProgressNotifier) m_progress;

  //! Global Plotter for all Tree Functions.
  Handle(ActAPI_IPlotter) m_plotter;

  //! Indicates whether Progress Notification is enabled or not.
  Standard_Boolean m_bProgressNotifierOn;

  //! Indicates whether Imperative Plotter is enabled or not.
  Standard_Boolean m_bPlotterOn;

  //! Indicates whether Execution Graph is frozen or not.
  Standard_Boolean m_bIsGraphFrozen;

  //! Transaction user data.
  ActAPI_TxData m_txData;

  //! List of Tree Function Parameters to deploy for "heavy" execution.
  Handle(ActAPI_HParameterList) m_functions2Deploy;

};

#endif
