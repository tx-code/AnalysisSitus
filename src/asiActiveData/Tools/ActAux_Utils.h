//-----------------------------------------------------------------------------
// Created on: May 2013
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

#ifndef ActAux_Utils_HeaderFile
#define ActAux_Utils_HeaderFile

// ACT algorithmic collection includes
#include <ActAux_Common.h>

// OCCT includes
#include <OSD_File.hxx>
#include <TCollection_AsciiString.hxx>

//! \ingroup AD_DF
//!
//! Function to filter strings.
typedef Standard_Boolean (*ActAux_StrFilter)(const TCollection_AsciiString& theString);

//! \ingroup AD_DF
//!
//! Function to compare strings.
typedef Standard_Boolean (*ActAux_StrComparator)(const TCollection_AsciiString& theString1,
                                                 const TCollection_AsciiString& theString2);

//! \ingroup AD_DF
//!
//! Auxiliary functionality.
namespace ActAux_Utils
{
  //! String utilities.
  namespace Str
  {
    //! Formats the passed value according to the passed format string.
    //! This is actually a standard sprintf functionality.
    //! \param theFormat [in] format string.
    //! \param theVal [in] value to format.
    //! \return formatted string.
    template<typename EType>
    TCollection_AsciiString Formatted(const Standard_CString theFormat,
                                      const EType theVal)
    {
      Standard_Character Buff[100];
      sprintf_s(Buff, theFormat, theVal);
      return Buff;
    }

    ActData_EXPORT TCollection_AsciiString
      Space(const Standard_Integer theNum);

    ActData_EXPORT TCollection_AsciiString
      BaseFilename(const TCollection_AsciiString& theFilename,
                   const Standard_Boolean doKeepExt = Standard_False);

    ActData_EXPORT TCollection_AsciiString
      BasePath(const TCollection_AsciiString& theFilename,
               const Standard_Boolean keepSlash = Standard_False);

    ActData_EXPORT Standard_Real
      RealAt(const TCollection_AsciiString& theStr,
             const Standard_Integer theStart,
             const Standard_Integer theEnd,
             Standard_Boolean& isOk);

    ActData_EXPORT Standard_Integer
      IntAt(const TCollection_AsciiString& theStr,
            const Standard_Integer theStart,
            const Standard_Integer theEnd,
            Standard_Boolean& isOk);

    ActData_EXPORT TCollection_AsciiString
      Trim(const TCollection_AsciiString& theStr);

    ActData_EXPORT Standard_Boolean
      FetchLine(OSD_File& FILE,
                TCollection_AsciiString& theLine,
                ActAux_StrFilter theFilter);

    ActData_EXPORT Standard_Boolean
      NextToken(TCollection_AsciiString& theLine,
                Standard_Integer& theIdx,
                TCollection_AsciiString& theToken,
                OSD_File& FILE,
                const TCollection_AsciiString& theSeparator,
                ActAux_StrFilter theFilter);

    ActData_EXPORT void
      Split(const TCollection_AsciiString& theLine,
            const TCollection_AsciiString& theSeparator,
            NCollection_Sequence<TCollection_AsciiString>& theChunks);

    ActData_EXPORT TCollection_AsciiString
      Slashed(const TCollection_AsciiString& theStr);

  }
}

#endif
