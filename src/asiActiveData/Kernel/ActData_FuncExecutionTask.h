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

#ifndef ActData_FuncExecutionTask_HeaderFile
#define ActData_FuncExecutionTask_HeaderFile

#if defined ActiveData_USE_TBB

// Active Data includes
#include <ActData_Common.h>

// Active Data (API) includes
#include <ActAPI_IModel.h>
#include <ActAPI_IProgressNotifier.h>

// Intel TBB includes
#include <tbb/task.h>

//! \ingroup AD_DF
//!
//! Specific invocation mechanism used to execute the dependency graph
//! in a distinct enqueued TBB task. Normally, you use this mechanism
//! in order to fork the algorithm's working thread from the GUI one.
class ActData_FuncExecutionTask : public tbb::task
{
public:

  ActData_EXPORT static ActData_FuncExecutionTask*
    Launch(const Handle(ActAPI_IProgressNotifier)& thePNotifier,
           const Handle(ActAPI_IModel)& theModel,
           const ActAPI_TxData& theData);

private:

  ActData_FuncExecutionTask(const Handle(ActAPI_IProgressNotifier)& thePNotifier,
                            const Handle(ActAPI_IModel)& theModel,
                            const ActAPI_TxData& theData);

  virtual tbb::task* execute();

// Input data for task:
private:

  //! Progress Notifier.
  Handle(ActAPI_IProgressNotifier) m_PNotifier;

  //! Data Model instance.
  Handle(ActAPI_IModel) m_model;

  //! Transaction data.
  ActAPI_TxData m_txData;

};

#else
  #pragma message("===== warning: TBB is disabled. You cannot use detached tasks for functions.")
#endif // ActiveData_USE_TBB

#endif
