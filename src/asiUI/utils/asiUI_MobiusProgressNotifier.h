//-----------------------------------------------------------------------------
// Created on: 24 March 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
//    * Neither the name of the copyright holder(s) nor the
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
//-----------------------------------------------------------------------------

#ifndef asiUI_MobiusProgressNotifier_h
#define asiUI_MobiusProgressNotifier_h

// asiUI includes
#include <asiUI.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>

// Mobius includes
#include <mobius/core_IProgressNotifier.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QProgressBar>
//
#include <Standard_WarningsRestore.hxx>

//-----------------------------------------------------------------------------

//! Progress notification tool for Mobius.
class asiUI_MobiusProgressNotifier : public mobius::core_IProgressNotifier
{
public:

  //! Ctor with native progress notifier and a progress bar.
  asiUI_EXPORT
    asiUI_MobiusProgressNotifier(ActAPI_ProgressEntry progress,
                                 QProgressBar*        pProgressBar);

public:

  //! Cleans up the internal state of the Progress Notifier, so that it
  //! becomes ready to track another job.
  asiUI_EXPORT virtual void
    Reset();

  //! Initializes the Progress Notifier with the deterministic capacity
  //! value. Capacity represents the unitless overall progress value which
  //! can be ever collected by all running tasks.
  //!
  //! Please note, that by default the progress scale is declared with
  //! infinite capacity. Practically, it means that algorithm is not able
  //! to foresee the number of steps it will need to complete. Make sure that
  //! in such a case your interface reacts adequately (e.g. no percentage is
  //! shown to the user).
  //!
  //! \param[in] capacity capacity score to set (infinite by default).
  asiUI_EXPORT virtual void
    Init(const int capacity = INT_MAX);

  //! Returns the capacity value.
  //! \return requested capacity value.
  asiUI_EXPORT virtual int
    GetCapacity() const;

  //! Returns true if the capacity value is infinite.
  //! \return true/false.
  asiUI_EXPORT virtual bool
    IsInfinite() const;

  //! Sets message (localization) key.
  //! \param[in] msgKey message key to set.
  asiUI_EXPORT virtual void
    SetMessageKey(const std::string& msgKey);

  //! Returns message localization key.
  //! \return localization key.
  asiUI_EXPORT virtual std::string
    GetMessageKey() const;

  //! Sets the job status.
  //! \param[in] status progress status to set.
  asiUI_EXPORT virtual void
    SetProgressStatus(const mobius::core_ProgressStatus status);

  //! Returns current progress status.
  //! \return the ultimate progress status.
  asiUI_EXPORT virtual mobius::core_ProgressStatus
    GetProgressStatus() const;

  //! Requests job cancellation.
  asiUI_EXPORT virtual void
    AskCancel();

  //! Checks whether the job is being canceled.
  //! \return true/false.
  asiUI_EXPORT virtual bool
    IsCancelling();

  //! Checks whether the job is in running state.
  //! \return true/false.
  asiUI_EXPORT virtual bool
    IsRunning();

  //! Checks whether the job is in failed state.
  //! \return true/false.
  asiUI_EXPORT virtual bool
    IsFailed();

  //! Returns the currently cumulated progress value.
  //! \return current cumulative progress.
  asiUI_EXPORT virtual int
    GetCurrentProgress() const;

  //! This method is used to increment the progress value by the passed step.
  //! \param[in] incr progress value to increment by.
  asiUI_EXPORT virtual void
    StepProgress(const int incr);

  //! This method is used to set the progress value.
  //! \param[in] progress progress value to set.
  asiUI_EXPORT virtual void
    SetProgress(const int progress);

  //! This method is used to send a logging message.
  //! \param[in] message   message string (normally it is i18n key).
  //! \param[in] severity  message severity (info, notice, warning, error).
  //! \param[in] priority  message priority (normal, high).
  //! \param[in] arguments message arguments (if any).
  asiUI_EXPORT virtual void
    SendLogMessage(const std::string&               message,
                   const mobius::core_MsgSeverity   severity,
                   const mobius::core_MsgPriority   priority  = mobius::MsgPriority_Normal,
                   const mobius::core_MsgArguments& arguments = mobius::core_MsgArguments());

  //! This method is used to send a logging message in a stream form.
  //! \param[in] logStream logging stream.
  asiUI_EXPORT virtual void
    SendLogMessage(const mobius::core_MsgStream& logStream);

protected:

  ActAPI_ProgressEntry m_progress;     //!< Native progress entry.
  QProgressBar*        m_pProgressBar; //!< Progress bar.
  bool                 m_bInfinite;    //!< Is infinite or not.
  bool                 m_bCancel;      //!< Cancel request.

};

#endif
