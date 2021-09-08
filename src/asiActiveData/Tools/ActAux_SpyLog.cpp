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
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActAux_SpyLog.h>

// Active Data (auxiliary) includes
#include <ActAux_Utils.h>

// OCCT includes
#include <OSD_Environment.hxx>
#include <OSD_File.hxx>
#include <OSD_Protection.hxx>

// STD includes
#include <iostream>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#endif

#define SpyLog_Filename "statistics.txt"

Handle(ActAux_SpyLog) ActAux_SpyLog::__instance;

//! Returns instance of the spy logger.
//! \return instance of spy logger.
Handle(ActAux_SpyLog) ActAux_SpyLog::Instance()
{
  if ( __instance.IsNull() )
    __instance = new ActAux_SpyLog();

  return __instance;
}

//! Constructor.
ActAux_SpyLog::ActAux_SpyLog() : Standard_Transient()
{
  this->Start();
}

//! Destructor.
ActAux_SpyLog::~ActAux_SpyLog()
{
  this->Stop();
}

//! Starts logging.
void ActAux_SpyLog::Start()
{
#ifdef _WIN32
  // Get user name
  CHAR username[UNLEN+1];
  DWORD username_len = UNLEN+1;
  GetUserNameA(username, &username_len);
  m_username = username;

  // Get computer name
  CHAR compname[UNLEN+1];
  DWORD compname_len = UNLEN+1;
  GetComputerNameA(compname, &compname_len);
  m_compname = compname;
#else
  // Get user name
  const size_t UNLEN = 256;
  char username[UNLEN];
  getlogin_r(username, UNLEN);
  m_username = username;

  // Get computer name
  char compname[UNLEN];
  gethostname(compname, UNLEN);
  m_compname = compname;
#endif
  // Session start timestamp
  m_session_start = ActAux_TimeStampTool::Generate();
}

//! Counts the call of analysis of the given name.
//! \param name [in] name of the analysis.
void ActAux_SpyLog::CallCount(const std::string& name)
{
  std::map<std::string, int>::iterator it = m_call_count.find(name);

  if ( it == m_call_count.end() )
    m_call_count.insert( std::pair<std::string, int>(name, 1) );
  else
    it->second += 1;
}

//! Stop logging.
void ActAux_SpyLog::Stop()
{
  TCollection_AsciiString testDumpPath = OSD_Environment("AD_TEST_DUMP").Value();
  TCollection_AsciiString dumpPath     = testDumpPath.ToCString();
  std::string             filename     = ActAux_Utils::Str::Slashed(dumpPath).ToCString() + std::string(SpyLog_Filename);

  // Loop until the file is accessible
  int limit = 1000, attempt = 0;
  std::ofstream FILE;
  FILE.open(filename, std::ios_base::app);
  while ( !FILE.is_open() && attempt < limit )
  {
#ifdef _WIN32
    Sleep(100); // msec
#else
    usleep(100000); // mksec
#endif
    FILE.open(filename, std::ios_base::app);
    ++attempt;
  }

  // Dump header
  std::string dump;
  dump += ( "---\nUser " + m_username
                         + " on " + m_compname
                         + " : started "
                         + m_session_start->ToString(Standard_False, Standard_False).ToCString()
                         + " : ended "
                         + ActAux_TimeStampTool::Generate()->ToString(Standard_False, Standard_False).ToCString()
                         + "\n" );
  for ( std::map<std::string, int>::const_iterator cit = m_call_count.cbegin(); cit != m_call_count.cend(); cit++ )
  {
    std::ostringstream os;
    os << cit->second;
    dump += ("    " + cit->first + " was called " + os.str() + " times\n");
  }

  // Dump results to file
  FILE.write( dump.c_str(), (Standard_Integer) dump.size() );

  // Close file
  FILE.close();
}
