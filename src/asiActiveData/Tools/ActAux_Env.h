//-----------------------------------------------------------------------------
// Created on: April 2012
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

#ifndef ActAux_Env_HeaderFile
#define ActAux_Env_HeaderFile

// Win includes
#ifdef _WIN32
#include <windows.h>
#endif

// ActAux includes
#include <ActAux.h>

#define BUFSIZE 1000

//! Auxiliary functions.
class ActAux_Env
{
public:
  //---------------------------------------------------------------------------
  // Environment

  //! Returns value of ACT_DATA environment variable. This variable is used to
  //! refer to the directory containing all data files playing as inputs for
  //! unit tests.
  //! \return value of ACT_DATA variable.
  static std::string ActData()
  {
    return GetVariable("AD_DATA");
  }

  //! Returns value of ACT_TEST_DUMPING environment variable. This variable is
  //! used to refer to the directory containing all temporary files utilized by
  //! unit tests.
  //! \return value of ACT_TEST_DUMPING variable.
  static std::string ActDumping()
  {
    return GetVariable("AD_TEST_DUMP");
  }

  //! Returns value of ACT_TEST_DESCR environment variable. This variable is used
  //! to refer to the directory containing all temporary files utilized by unit
  //! tests.
  //! \return value of ACT_TEST_DESCR variable.
  static std::string ActDescr()
  {
    return GetVariable("AD_TEST_DESCR");
  }

  //! Returns value of the environment variable with the passed name.
  //! \param VarName [in] variable name.
  //! \return value of the variable.
  static std::string GetVariable(const char* VarName)
  {
#ifdef _WIN32
    TCHAR chNewEnv[BUFSIZE];
    GetEnvironmentVariable(VarName, chNewEnv, BUFSIZE);
#else
    char* chNewEnv = getenv(VarName);
#endif
    return chNewEnv;
  }

};

#endif
