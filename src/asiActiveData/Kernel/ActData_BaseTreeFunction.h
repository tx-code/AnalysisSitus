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

#ifndef ActData_BaseTreeFunction_HeaderFile
#define ActData_BaseTreeFunction_HeaderFile

// Active Data includes
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>
#include <ActAPI_ITreeFunction.h>

// OCCT includes
#include <TFunction_Driver.hxx>
#include <TFunction_Logbook.hxx>

DEFINE_STANDARD_HANDLE(ActData_BaseTreeFunction, ActAPI_ITreeFunction)
DEFINE_STANDARD_HANDLE(ActData_TreeFunctionDriver, TFunction_Driver)

//! \ingroup AD_DF
//!
//! Base class for Tree Functions. Conceptually, Tree Function is a wrapper
//! under some algorithm allowing its execution against the dependency graph
//! defined by the user of Active Data.
class ActData_BaseTreeFunction : public ActAPI_ITreeFunction
{
friend class ActData_BaseModel;
friend class ActData_TreeFunctionDriver;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_BaseTreeFunction, ActAPI_ITreeFunction)

public:

  ActData_EXPORT virtual Standard_CString
    GetName() const;

  ActData_EXPORT virtual Standard_Integer
    Execute(const Handle(ActAPI_HParameterList)& theArgsIN,
            const Handle(ActAPI_HParameterList)& theArgsOUT) const;

  ActData_EXPORT virtual Standard_Boolean
    MustExecuteIntact(const Handle(ActAPI_HParameterList)& theArgsIN,
                      const Handle(Standard_Transient)&    theUserData = nullptr) const;

  ActData_EXPORT virtual Standard_Boolean
    NoPropagation() const;

  ActData_EXPORT virtual Standard_Integer
    Priority() const;

  ActData_EXPORT virtual void
    AutoConnect(const Handle(ActAPI_INode)& theOwnerNode) const;

public:

  //! Initializes shared user data.
  //! \param theUserData [in] user data container to set.
  void SetUserData(const Handle(Standard_Transient)& theUserData)
  {
    m_UserData = theUserData;
  }

  //! Accessor for the shared user data.
  //! \return requested container.
  const Handle(Standard_Transient)& GetUserData() const
  {
    return m_UserData;
  }

  //! Initializes shared Progress Notifier.
  //! \param thePNotifier [in] Progress Notifier instance to set.
  void SetProgressNotifier(const Handle(ActAPI_IProgressNotifier)& thePNotifier)
  {
    m_progress = thePNotifier;
  }

  //! Accessor for the shared Progress Notifier.
  //! \return requested Progress Notifier instance.
  Handle(ActAPI_IProgressNotifier) GetProgressNotifier() const
  {
    return m_progress.Access();
  }

  //! Initializes shared Progress Notifier.
  //! \param thePlotter [in] Imperative Plotter to set.
  void SetPlotter(const Handle(ActAPI_IPlotter)& thePlotter)
  {
    m_plotter = thePlotter;
  }

  //! \return plotter.
  const Handle(ActAPI_IPlotter)& GetPlotter() const
  {
    return m_plotter.Access();
  }

  //! Returns Tree Function Driver.
  //! \return Function Driver.
  const Handle(ActData_TreeFunctionDriver)& GetDriver() const
  {
    return m_driver;
  }

protected:

  ActData_EXPORT
    ActData_BaseTreeFunction();

protected:

  ActData_EXPORT virtual Standard_Boolean
    validate(const Handle(ActAPI_HParameterList)& theArgsIN,
             const Handle(ActAPI_HParameterList)& theArgsOUT) const;

  ActData_EXPORT virtual Standard_Boolean
    validateInput(const Handle(ActAPI_HParameterList)& theArgsIN) const;

  ActData_EXPORT virtual Standard_Boolean
    validateOutput(const Handle(ActAPI_HParameterList)& theArgsOUT) const;

private:

  virtual Standard_Integer
    execute(const Handle(ActAPI_HParameterList)& theArgsIN,
            const Handle(ActAPI_HParameterList)& theArgsOUT,
            const Handle(Standard_Transient)& theUserData = nullptr) const = 0;

  virtual ActAPI_ParameterTypeStream
    inputSignature() const = 0;

  virtual ActAPI_ParameterTypeStream
    outputSignature() const = 0;

protected:

  ActData_EXPORT Standard_Boolean
    validateBySignature(const Handle(ActAPI_HParameterList)& theArgs,
                        const ActAPI_ParameterTypeStream& theSignature) const;

  ActData_EXPORT Standard_Boolean
    hasUnrecoverableParameters(const Handle(ActAPI_HParameterList)& theArgsIN,
                               const Handle(ActAPI_HParameterList)& theArgsOUT,
                               const Standard_Integer theCheckType) const;

  ActData_EXPORT void
    propagateInvalid(const Handle(ActAPI_HParameterList)& theArgs) const;

  ActData_EXPORT void
    propagatePending(const Handle(ActAPI_HParameterList)& theArgs) const;

protected:

  //! Shared Progress Notifier.
  mutable ActAPI_ProgressEntry m_progress;

  //! Shared Plotter.
  mutable ActAPI_PlotterEntry m_plotter;

private:

  //! Internal OCCT TFunction Driver.
  Handle(ActData_TreeFunctionDriver) m_driver;

  //! Shared user data.
  Handle(Standard_Transient) m_UserData;

};

//! \ingroup AD_DF
//!
//! Function Driver implementing necessary OCCT TFunction Driver mechanism
//! with respect to Active Data concepts. This class describes a kernel
//! object which is used by OCCT TFunction engine in order to build a
//! dependency graph. Besides that, this class provides an entry point to
//! execution routine which delegates the actual execution of the particular
//! algorithm to the domain-specific Tree Function.
class ActData_TreeFunctionDriver : public TFunction_Driver
{
friend class ActData_BaseModel;
friend class ActData_BaseTreeFunction;
friend class ActData_FuncExecutionCtx;

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_TreeFunctionDriver, TFunction_Driver)

public:

  ActData_EXPORT ActData_TreeFunctionDriver();

  //! Accessor for the corresponding domain-specific Tree Function.
  //! \return the corresponding Tree Function.
  inline const Handle(ActAPI_ITreeFunction)& GetFunction() const
  {
    return m_func;
  }

private:

  ActData_EXPORT virtual Standard_Boolean
    MustExecute(const Handle(TFunction_Logbook)&) const;

  ActData_EXPORT virtual Standard_Integer
    Execute(Handle(TFunction_Logbook)&) const;

  ActData_EXPORT virtual void
    Arguments(TDF_LabelList& theArguments) const;

  ActData_EXPORT virtual void
    Results(TDF_LabelList& theResults) const;

private:

  Handle(ActAPI_HParameterList)
    parametersByLabels(const TDF_LabelList& theLabels) const;

  void
    initFunction(const Handle(ActAPI_ITreeFunction)& theFunc);

private:

  //! Domain-specific Tree Function associated with the TFunction Driver.
  Handle(ActAPI_ITreeFunction) m_func;

};

#endif
