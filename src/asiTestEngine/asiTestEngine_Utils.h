//-----------------------------------------------------------------------------
// Created on: 09 December 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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

#ifndef asiTestEngine_Utils_h
#define asiTestEngine_Utils_h

// asiTestEngine includes
#include <asiTestEngine_ResultGroup.h>

// Standard includes
#include <string>

//-----------------------------------------------------------------------------

//! Auxiliary functions for testing.
namespace asiTestEngine_Utils
{
  //! Derives a group name from filename.
  asiTestEngine_EXPORT std::string
    GetGroupNameFromFile(const std::string& filename);

  //! Generates a unique directory name for dumping reports.
  //! \return full dir name.
  asiTestEngine_EXPORT std::string
    GetUniqueDirName();

  //! Creates index.html in the `outputDir` to reference all files collected in
  //! the passed `outFilenames` vector. This index page will serve as an entry
  //! point to the entire report.
  //!
  //! \param[in] outputDir    output directory for the testing session.
  //! \param[in] resultGroups named result groups.
  //! \param[in] numSucceeded number of succeeded test cases.
  //! \param[in] numFailed    number of failed test cases.
  //! \param[in] numBad       number of bad test cases.
  //! \param[in] numNoref     number of test cases without reference data.
  //! \param[in] numGenref    number of test cases for which the reference data was generated.
  //! \return true in case of success, false -- otherwise.
  asiTestEngine_EXPORT bool
    CreateIndex(const TCollection_AsciiString&    outputDir,
                const asiTestEngine_ResultGroups& resultGroups,
                const int                         numSucceeded,
                const int                         numFailed,
                const int                         numBad,
                const int                         numNoref,
                const int                         numGenref);

} // asiTestEngine_Utils namespace.

#endif
