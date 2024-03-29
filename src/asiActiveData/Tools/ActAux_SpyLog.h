//-----------------------------------------------------------------------------
// Created on: February 2015
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
// Web: http://analysissitus.org
//-----------------------------------------------------------------------------

#ifndef ActAux_SpyLog_HeaderFile
#define ActAux_SpyLog_HeaderFile

#ifdef _WIN32
// Win API and services
#include <windows.h>
#include <lmcons.h>
#endif

// Active Data (auxiliary) includes
#include <ActAux_TimeStamp.h>

// OCCT includes
#include <Standard_Type.hxx>

// STD includes
#include <map>

DEFINE_STANDARD_HANDLE(ActAux_SpyLog, Standard_Transient)

//! \ingroup AD_DF
//!
//! Journal for logging of usage statistics.
class ActAux_SpyLog : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAux_SpyLog, Standard_Transient)

public:

  ActData_EXPORT static Handle(ActAux_SpyLog)
    Instance();

public:

  ActData_EXPORT void
    Start();

  ActData_EXPORT void
    CallCount(const std::string& name);

  ActData_EXPORT void
    Stop();

protected:

  ActData_EXPORT
    ActAux_SpyLog();

  ActData_EXPORT
    ~ActAux_SpyLog();

private:

  ActAux_SpyLog(const ActAux_SpyLog&) {}
  void operator=(const ActAux_SpyLog&) {}

private:

  static Handle(ActAux_SpyLog) __instance; //!< Instance.

private:

  std::string                m_username;      //!< User name.
  std::string                m_compname;      //!< Computer name.
  Handle(ActAux_TimeStamp)   m_session_start; //!< Timestamp of session start.
  std::map<std::string, int> m_call_count;    //!< Call counter for analyses.

};


#endif
