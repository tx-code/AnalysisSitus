//-----------------------------------------------------------------------------
// Created on: 05 January 2023
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

// Own include
#include <asiAlgo_MobiusProgressNotifier.h>

using namespace mobius;

//-----------------------------------------------------------------------------

asiAlgo_MobiusProgressNotifier::asiAlgo_MobiusProgressNotifier(ActAPI_ProgressEntry progress)
: m_progress(progress)
{
  this->Reset();
}

/* =========================================================================
 *  Section: Methods to use in a single-threaded context
 *  Purpose: E.g. having a single GUI thread responsive to user events,
 *           you can call these methods in that thread, while it is
 *           prohibited to call them in the background working thread
 *           containing several tasks
 * ========================================================================= */

void asiAlgo_MobiusProgressNotifier::Reset()
{
  m_progress.Reset();
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::Init(const int capacity)
{
  m_progress.Init(capacity);
}

//-----------------------------------------------------------------------------

int asiAlgo_MobiusProgressNotifier::GetCapacity() const
{
  return m_progress.Capacity();
}

//-----------------------------------------------------------------------------

bool asiAlgo_MobiusProgressNotifier::IsInfinite() const
{
  return m_progress.IsInfinite();
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::SetMessageKey(const std::string& msgKey)
{
  m_progress.SetMessageKey( msgKey.c_str() );
}

//-----------------------------------------------------------------------------

std::string asiAlgo_MobiusProgressNotifier::GetMessageKey() const
{
  return m_progress.MessageKey().ToCString();
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::SetProgressStatus(const core_ProgressStatus status)
{
  switch ( status )
  {
    case core_ProgressStatus::Progress_Canceled:
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
      break;
    case core_ProgressStatus::Progress_Failed:
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
      break;
    case core_ProgressStatus::Progress_Running:
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Running);
      break;
    case core_ProgressStatus::Progress_Succeeded:
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);
      break;
    case core_ProgressStatus::Progress_Undefined:
    default:
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Undefined);
      break;
  }
}

//-----------------------------------------------------------------------------

core_ProgressStatus asiAlgo_MobiusProgressNotifier::GetProgressStatus() const
{
  switch ( m_progress.ProgressStatus() )
  {
    case ActAPI_ProgressStatus::Progress_Canceled:
      return core_ProgressStatus::Progress_Canceled;

    case ActAPI_ProgressStatus::Progress_Failed:
      return core_ProgressStatus::Progress_Failed;

    case ActAPI_ProgressStatus::Progress_Running:
      return core_ProgressStatus::Progress_Running;

    case ActAPI_ProgressStatus::Progress_Succeeded:
      return core_ProgressStatus::Progress_Succeeded;

    case ActAPI_ProgressStatus::Progress_Undefined:
    default:
      break;
  }

  return core_ProgressStatus::Progress_Undefined;
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::AskCancel()
{
  m_progress.Cancel();
}

//-----------------------------------------------------------------------------

bool asiAlgo_MobiusProgressNotifier::IsCancelling()
{
  return m_progress.IsCancelling();
}

//-----------------------------------------------------------------------------

bool asiAlgo_MobiusProgressNotifier::IsRunning()
{
  return m_progress.IsRunning();
}

//-----------------------------------------------------------------------------

bool asiAlgo_MobiusProgressNotifier::IsFailed()
{
  return m_progress.IsFailed();
}

/* =========================================================================
 *  Section: Thread-safe methods
 * ========================================================================= */

int asiAlgo_MobiusProgressNotifier::GetCurrentProgress() const
{
  return m_progress.CurrentProgress();
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::StepProgress(const int stepProgress)
{
  m_progress.StepProgress(stepProgress);
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::SetProgress(const int progress)
{
  m_progress.SetProgress(progress);
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::SendLogMessage(const std::string&       message,
                                                    const core_MsgSeverity   severity,
                                                    const core_MsgPriority   priority,
                                                    const core_MsgArguments& arguments)
{
  // Assimilate severity.
  ActAPI_LogMessageSeverity _severity;
  //
  switch ( severity )
  {
    case core_MsgSeverity::MsgSeverity_Error:
      _severity = ActAPI_LogMessageSeverity::Severity_Error;
      break;
    case core_MsgSeverity::MsgSeverity_Information:
      _severity = ActAPI_LogMessageSeverity::Severity_Information;
      break;
    case core_MsgSeverity::MsgSeverity_Notice:
      _severity = ActAPI_LogMessageSeverity::Severity_Notice;
      break;
    case core_MsgSeverity::MsgSeverity_Warning:
      _severity = ActAPI_LogMessageSeverity::Severity_Warning;
      break;
    default:
      break;
  };

  // Assimilate priority.
  ActAPI_LogMessagePriority _priority;
  //
  switch ( priority )
  {
    case core_MsgPriority::MsgPriority_High:
      _priority = ActAPI_LogMessagePriority::Priority_High;
      break;
    case core_MsgPriority::MsgPriority_Normal:
    default:
      _priority = ActAPI_LogMessagePriority::Priority_Normal;
      break;
  };

  // Convert args.
  ActAPI_LogArguments _args;
  //
  for ( const auto& arg : arguments )
  {
    t_ptr<core_VarBool>       vBool    = t_ptr<core_VarBool>       ::DownCast(arg);
    t_ptr<core_VarInt>        vInt     = t_ptr<core_VarInt>        ::DownCast(arg);
    t_ptr<core_VarReal>       vReal    = t_ptr<core_VarReal>       ::DownCast(arg);
    t_ptr<core_VarRealVector> vRealVec = t_ptr<core_VarRealVector> ::DownCast(arg);
    t_ptr<core_VarString>     vString  = t_ptr<core_VarString>     ::DownCast(arg);

    // Convert.
    Handle(ActAPI_VariableBase) _var;
    //
    if ( !vBool.IsNull() )
    {
      _var = new ActAPI_VariableBool(vBool->Name.c_str(), vBool->Value);
    }
    else if ( !vInt.IsNull() )
    {
      _var = new ActAPI_VariableInt(vInt->Name.c_str(), vInt->Value);
    }
    else if ( !vReal.IsNull() )
    {
      _var = new ActAPI_VariableReal(vReal->Name.c_str(), vReal->Value);
    }
    else if ( !vRealVec.IsNull() )
    {
      // TODO: NYI
    }
    else if ( !vString.IsNull() )
    {
      _var = new ActAPI_VariableString(vString->Name.c_str(), vString->Value.c_str());
    }

    if ( !_var.IsNull() )
      _args.Append(_var);
  }

  m_progress.SendLogMessage(message.c_str(), _severity, _priority, _args);
}

//-----------------------------------------------------------------------------

void asiAlgo_MobiusProgressNotifier::SendLogMessage(const core_MsgStream& logStream)
{
  this->SendLogMessage( logStream.GetText(),
                        logStream.GetSeverity(),
                        logStream.GetPriority(),
                        logStream.GetArgs() );
}
