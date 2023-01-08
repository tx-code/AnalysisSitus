//-----------------------------------------------------------------------------
// Created on: 31 March 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiAlgo_Logger.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// Active Data includes
#include <ActAux_TimeStamp.h>

// OCCT includes
#include <Standard_ProgramError.hxx>

// Useful to print logged messages also to the standard output.
#undef COUT_DEBUG

//-----------------------------------------------------------------------------

namespace
{
  template<typename T>
  std::string toString(const Handle(Standard_Transient)& theValue)
  {
    Handle(T) aValue = Handle(T)::DownCast(theValue);
    if (aValue.IsNull())
      return "";

    std::string res = asiAlgo_Utils::Str::ToString(aValue->Value);
    return res;
  }

  std::string getString(const Handle(Standard_Transient)& theValue)
  {
    std::string aStandInteger = toString<ActAPI_VariableInt>(theValue);
    if (!aStandInteger.empty())
      return aStandInteger;

    std::string aStandReal = toString<ActAPI_VariableReal>(theValue);
    if (!aStandReal.empty())
      return aStandReal;

    std::string aStandString = toString<ActAPI_VariableString>(theValue);
    if (!aStandString.empty())
      return aStandString;

    return "<empty arg>";
  }

  std::string getFormatted(const std::string&         message,
                           const ActAPI_LogArguments& arguments)
  {
    // Try to treat the passed message as a key
    std::string formatted = message;

    for ( int i = 1; i <= arguments.Length(); ++i )
    {
      std::string iarg = "%"; iarg += asiAlgo_Utils::Str::ToString(i);
      size_t parg = formatted.find(iarg);
      std::string sarg = getString(arguments.Value(i));

      if ( parg != std::string::npos )
      {
        formatted.erase(parg, parg + iarg.size());
        formatted.insert(parg, sarg);
      }
      else
      {
        formatted += " ";
        formatted += sarg;
      }
    }

    return formatted;
  }

  std::string
    formatMessage(const std::string&              message,
                  const ActAPI_LogMessageSeverity severity,
                  const ActAPI_LogMessagePriority,
                  const ActAPI_LogArguments&      arguments,
                  const Handle(Standard_Transient)&)
  {
    if ( message.empty() )
      return "";

    // Apply arguments.
    std::string msg = getFormatted(message, arguments);

    // Generate severity-dependent prefix,
    std::string prefix;
    //
    if ( severity == Severity_Information )
    {
      prefix = "[INFO] ";
    }
    else if ( severity == Severity_Notice )
    {
      prefix = "[NOTICE] ";
    }
    else if ( severity == Severity_Warning )
    {
      prefix = "[WARNING] ";
    }
    else if ( severity == Severity_Error )
    {
      prefix = "[ERROR] ";
    }

    return prefix + msg;
  }
}

//-----------------------------------------------------------------------------
// THREAD-UNSAFE methods
//-----------------------------------------------------------------------------

//! Default constructor.
asiAlgo_Logger::asiAlgo_Logger() : ActAPI_ILogger()
{}

//! Returns a copy of the list of messages in OCCT form and cleans up the
//! internal thread-safe TBB collection.
//! \return list of logging messages.
ActAPI_LogMessageList asiAlgo_Logger::PopMessageList()
{
  ActAPI_LogMessageList resultList;
  ActAPI_LogMessage msg;

#ifdef USE_TBB
  while ( m_messageQueue.unsafe_size() > 0 )
  {
    m_messageQueue.try_pop(msg);
    resultList.Append(msg);
  }
#else
  while ( m_messageQueue.size() > 0 )
  {
    msg = m_messageQueue.front();

    m_messageQueue.pop();
    resultList.Append(msg);
  }
#endif

  return resultList;
}

//! Checks whether the logger contains any error messages.
//! \return true/false.
unsigned int asiAlgo_Logger::HasErrors()
{
  ActAPI_LogMessageList aResultList = this->PopMessageList();
  for ( ActAPI_LogMessageList::Iterator it(aResultList); it.More(); it.Next() )
  {
    if ( it.Value().Severity == Severity_Error )
      return 1;
  }

  return 0;
}

//! Cleans up the internal collection of messages.
void asiAlgo_Logger::Clear()
{
#ifdef USE_TBB
  m_messageQueue.clear();
#else
  _MessageQueue empty;
  std::swap(m_messageQueue, empty);
#endif
}

//-----------------------------------------------------------------------------
// THREAD-SAFE logging kernel methods
//-----------------------------------------------------------------------------

//! Appends a logging message with INFORMATION severity to the Logger queue.
//! \param theMessage   [in] message to add.
//! \param thePriority  [in] priority of the message.
//! \param theArguments [in] message arguments.
//! \param theTimeStamp [in] application-specific timestamp. Current timestamp
//!                          is used in case of nullptr value passed.
void asiAlgo_Logger::Info(const std::string&                theMessage,
                          const ActAPI_LogMessagePriority   thePriority,
                          const ActAPI_LogArguments&        theArguments,
                          const Handle(Standard_Transient)& theTimeStamp)
{
  this->appendMessage(theMessage,
                      Severity_Information,
                      thePriority,
                      theArguments,
                      theTimeStamp.IsNull() ? ActAux_TimeStampTool::Generate().get() : theTimeStamp);
}

//! Appends a logging message with NOTICE severity to the Logger queue.
//! \param theMessage   [in] message to add.
//! \param thePriority  [in] priority of the message.
//! \param theArguments [in] message arguments.
//! \param theTimeStamp [in] application-specific timestamp. Current timestamp
//!                          is used in case of nullptr value passed.
void asiAlgo_Logger::Notice(const std::string&                theMessage,
                            const ActAPI_LogMessagePriority   thePriority,
                            const ActAPI_LogArguments&        theArguments,
                            const Handle(Standard_Transient)& theTimeStamp)
{
  this->appendMessage(theMessage,
                      Severity_Notice,
                      thePriority,
                      theArguments,
                      theTimeStamp.IsNull() ? ActAux_TimeStampTool::Generate().get() : theTimeStamp);
}

//! Appends a logging message with WARNING severity to the Logger queue.
//! \param theMessage   [in] message to add.
//! \param thePriority  [in] priority of the message.
//! \param theArguments [in] message arguments.
//! \param theTimeStamp [in] application-specific timestamp. Current timestamp
//!                          is used in case of nullptr value passed.
void asiAlgo_Logger::Warn(const std::string&                theMessage,
                          const ActAPI_LogMessagePriority   thePriority,
                          const ActAPI_LogArguments&        theArguments,
                          const Handle(Standard_Transient)& theTimeStamp)
{
  this->appendMessage(theMessage,
                      Severity_Warning,
                      thePriority,
                      theArguments,
                      theTimeStamp.IsNull() ? ActAux_TimeStampTool::Generate().get() : theTimeStamp);
}

//! Appends a logging message with ERROR severity to the Logger queue.
//! \param theMessage   [in] message to add.
//! \param thePriority  [in] priority of the message.
//! \param theArguments [in] message arguments.
//! \param theTimeStamp [in] application-specific timestamp. Current timestamp
//!                          is used in case of nullptr value passed.
void asiAlgo_Logger::Error(const std::string&                theMessage,
                           const ActAPI_LogMessagePriority   thePriority,
                           const ActAPI_LogArguments&        theArguments,
                           const Handle(Standard_Transient)& theTimeStamp)
{
  this->appendMessage(theMessage,
                      Severity_Error,
                      thePriority,
                      theArguments,
                      theTimeStamp.IsNull() ? ActAux_TimeStampTool::Generate().get() : theTimeStamp);
}

//! Appends a logging message with the passed severity to the Logger queue.
//! \param theMessage   [in] message to add.
//! \param theSeverity  [in] severity of the message.
//! \param thePriority  [in] priority of the message.
//! \param theArguments [in] message arguments.
//! \param theTimeStamp [in] application-specific timestamp.
void asiAlgo_Logger::appendMessage(const std::string&                theMessage,
                                   const ActAPI_LogMessageSeverity   theSeverity,
                                   const ActAPI_LogMessagePriority   thePriority,
                                   const ActAPI_LogArguments&        theArguments,
                                   const Handle(Standard_Transient)& theTimeStamp)
{
#if defined COUT_DEBUG
  std::cout << "\tLOGGER: " << getFormatted(theMessage, theArguments).ToCString() << std::endl;
#endif

  m_messageQueue.push( ActAPI_LogMessage(thePriority,
                                         theSeverity,
                                         theMessage,
                                         theArguments,
                                         theTimeStamp) );

  if ( !m_appenders.IsEmpty() )
  {
    std::string msg = formatMessage(theMessage,
                                    theSeverity,
                                    thePriority,
                                    theArguments,
                                    theTimeStamp);
    // Put in all appender streams.
    for ( int k = 0; k < m_appenders.Length(); ++k )
    {
      *m_appenders[k] << msg << "\n";
      *m_appenders[k] << std::flush;
    }
  }
}
