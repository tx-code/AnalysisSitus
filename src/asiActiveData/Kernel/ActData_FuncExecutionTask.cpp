//-----------------------------------------------------------------------------
// Created on: July 2012
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
#include <ActData_FuncExecutionTask.h>

#if defined ActiveData_USE_TBB

// OCCT includes
#include <Standard_ProgramError.hxx>

//! Initializing constructor.
//! \param thePNotifier [in] Progress Notifier instance.
//! \param theModel [in] Data Model instance.
//! \param theData [in] optional transaction data.
ActData_FuncExecutionTask::ActData_FuncExecutionTask(const Handle(ActAPI_IProgressNotifier)& thePNotifier,
                                                     const Handle(ActAPI_IModel)& theModel,
                                                     const ActAPI_TxData& theData)
  : m_PNotifier(thePNotifier),
    m_model(theModel),
    m_txData(theData)
{}

//! Forks the new TBB non-waitable task for multiplication algorithm.
//! \param thePNotifier [in] Progress Notifier instance.
//! \param theModel [in] Data Model instance.
//! \param theData [in] optional transaction data.
ActData_FuncExecutionTask*
  ActData_FuncExecutionTask::Launch(const Handle(ActAPI_IProgressNotifier)& thePNotifier,
                                    const Handle(ActAPI_IModel)& theModel,
                                    const ActAPI_TxData& theData)
{
  ActAPI_ProgressEntry aPEntry(thePNotifier);
  aPEntry.Reset();

  // It is necessary to indicate that the background job
  // is about to start to ensure correct GUI state transitions.
  // This should be done still by the main thread, otherwise
  // GUI receives corresponding signal with fatally long delay.
  aPEntry.SetProgressStatus(Progress_Running);

  ActData_FuncExecutionTask*
    aTask = new( tbb::task::allocate_root() ) ActData_FuncExecutionTask(thePNotifier, theModel, theData);

  if ( theModel->FuncExecutionFlags() & ActAPI_IModel::ExecFlags_ForceNoDetach )
  {
    tbb::task::spawn_root_and_wait(*aTask);
  }
  else
  {
    tbb::task::enqueue(*aTask); // Non-blocking fork
  }

  return aTask;
}

//! Performs actual executing routine. Note that the code of this
//! method is executed in a distinct thread. Moreover, this code creates
//! additional TBB tasks (threads).
//! \return next task to execute -- NULL in our case.
tbb::task* ActData_FuncExecutionTask::execute()
{
  // Execute dependency graph
  Standard_Integer aRes = ActAPI_IModel::Execution_Undefined;
  try
  {
    m_model->FuncProgressNotifierOn();
    m_model->FuncExecuteAll(Standard_False, m_txData);
    m_model->FuncProgressNotifierOff();
  }
  catch ( ... )
  {
    aRes = ActAPI_IModel::Execution_Failed;
  }

  // Set resulting status of the parallel job and notify implicitly.
  // NOTE: Progress_Running status should be forced by the main thread
  // before the parallel job is started, to ensure that GUI prepares
  // for background job execution.
  // See also Launch() method.
  ActAPI_ProgressEntry aPEntry(m_PNotifier);
  if ( aPEntry.IsCancelling() ) // Cancellation requested
    aPEntry.SetProgressStatus(Progress_Canceled);
  else if ( ActAPI_IModel::IsExecutionFailed(aRes) )
    aPEntry.SetProgressStatus(Progress_Failed);
  else
    aPEntry.SetProgressStatus(Progress_Succeeded);

  return NULL; // No next task to execute
}

#endif // ActiveData_USE_TBB
