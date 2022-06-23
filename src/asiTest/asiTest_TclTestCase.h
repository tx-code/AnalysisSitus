//-----------------------------------------------------------------------------
// Created on: 25 June 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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

#ifndef asiTest_TclTestCase_HeaderFile
#define asiTest_TclTestCase_HeaderFile

// asiTest includes
#include <asiTest.h>

// asiTestEngine includes
#include <asiTestEngine_TestCase.h>

//! Base class for all test cases which employ Tcl scripting.
class asiTest_TclTestCase : public asiTestEngine_TestCase
{
protected:

  //! Evaluates Tcl script using the available Tcl interpretor.
  //! \param[in] scriptFilename the full script's filename.
  //! \param[in] name           function name.
  //! \param[in] funcID         function ID.
  //! \return true if Tcl returns TCL_OK, false -- otherwise.
  static outcome evaluate(const TCollection_AsciiString& scriptFilename,
                          const std::string&             name,
                          const int                      funcID);

  //! Evaluates all Tcl scripts in the passed directory.
  //! \param[in] dir  the directory containing Tcl test scripts.
  //! \param[in] name function name.
  //! \return true if Tcl returns TCL_OK, false -- otherwise.
  static outcome evaluateAll(const TCollection_AsciiString& dir,
                             const std::string&             name);

};

#endif
