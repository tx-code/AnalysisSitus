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

// Own include
#include <asiUI_MobiusProgressNotifier.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QCoreApplication>
//
#include <Standard_WarningsRestore.hxx>

using namespace mobius;

//-----------------------------------------------------------------------------

asiUI_MobiusProgressNotifier::asiUI_MobiusProgressNotifier(ActAPI_ProgressEntry progress,
                                                           QProgressBar*        pProgressBar)
: m_progress     (progress),
  m_pProgressBar (pProgressBar),
  m_bInfinite    (true),
  m_bCancel      (false)
{
  this->Reset();
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::Reset()
{
  m_bCancel = false;
  m_pProgressBar->reset();
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::Init(const int capacity)
{
  this->Reset();

  m_bInfinite = (capacity == INT_MAX);

  if ( !m_bInfinite )
    m_pProgressBar->setMaximum(capacity);

  QCoreApplication::processEvents();
}

//-----------------------------------------------------------------------------

int asiUI_MobiusProgressNotifier::GetCapacity() const
{
  return m_pProgressBar->maximum();
}

//-----------------------------------------------------------------------------

bool asiUI_MobiusProgressNotifier::IsInfinite() const
{
  return m_bInfinite;
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::SetMessageKey(const std::string&)
{
}

//-----------------------------------------------------------------------------

std::string asiUI_MobiusProgressNotifier::GetMessageKey() const
{
  return "";
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::SetProgressStatus(const core_ProgressStatus)
{
}

//-----------------------------------------------------------------------------

core_ProgressStatus asiUI_MobiusProgressNotifier::GetProgressStatus() const
{
  return core_ProgressStatus::Progress_Undefined;
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::AskCancel()
{
  m_bCancel = true;
}

//-----------------------------------------------------------------------------

bool asiUI_MobiusProgressNotifier::IsCancelling()
{
  return m_bCancel;
}

//-----------------------------------------------------------------------------

bool asiUI_MobiusProgressNotifier::IsRunning()
{
  return false;
}

//-----------------------------------------------------------------------------

bool asiUI_MobiusProgressNotifier::IsFailed()
{
  return false;
}

//-----------------------------------------------------------------------------

int asiUI_MobiusProgressNotifier::GetCurrentProgress() const
{
  return m_pProgressBar->value();
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::StepProgress(const int stepProgress)
{
  m_pProgressBar->setValue( m_pProgressBar->value() + stepProgress );

  QCoreApplication::processEvents();
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::SetProgress(const int progress)
{
  m_pProgressBar->setValue(progress);
}

//-----------------------------------------------------------------------------

void asiUI_MobiusProgressNotifier::SendLogMessage(const std::string&       message,
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
      _severity = ActAPI_LogMessageSeverity::Severity_Information;
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

void asiUI_MobiusProgressNotifier::SendLogMessage(const core_MsgStream& logStream)
{
  this->SendLogMessage( logStream.GetText(),
                        logStream.GetSeverity(),
                        logStream.GetPriority(),
                        logStream.GetArgs() );
}
