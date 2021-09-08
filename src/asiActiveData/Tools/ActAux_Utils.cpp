//-----------------------------------------------------------------------------
// Created on: 2012-2015
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
#include <ActAux_Utils.h>

#define BUFLEN 65536

//! Returns as many spaces as specified in the passed argument.
//! \param theNum [in] desired number of spaces.
//! \return string composed of spaces.
TCollection_AsciiString ActAux_Utils::Str::Space(const Standard_Integer theNum)
{
  TCollection_AsciiString aRes;
  for ( Standard_Integer c = 1; c <= theNum; ++c )
    aRes += " ";

  return aRes;
}

//! Extracts base filename from the given string representing path.
//! E.g. if the input string is "D:\dir1\dir2\hello.ext", the result is
//! "hello". Extension string in the input filename is not mandatory.
//! \param theFilename [in] input filename.
//! \param doKeepExt [in] indicates whether to keep extension.
//! \return extracted base filename.
TCollection_AsciiString
  ActAux_Utils::Str::BaseFilename(const TCollection_AsciiString& theFilename,
                                  const Standard_Boolean doKeepExt)
{
  TCollection_AsciiString Result = theFilename;
  TCollection_AsciiString RSeparator(ActAux_Macro_RSLASH_STR);
  TCollection_AsciiString DSeparator(ActAux_Macro_DSLASH_STR);

  TCollection_AsciiString Token;
  Standard_Integer t;

  /* ================================
    *  Explode with reverse separator
    * ================================ */

  // Traverse all tokens with reverse separator to access the last one
  t = 1;
  do
  {
    Token = Result.Token(RSeparator.ToCString(), t++);
  }
  while ( !Token.IsEmpty() );

  // Set result to be the last token
  Result = Result.Token(RSeparator.ToCString(), t - 2);

  /* ===============================
   *  Explode with direct separator
   * =============================== */

  // Traverse all tokens with direct separator to access the last one
  t = 1;
  do
  {
    Token = Result.Token(DSeparator.ToCString(), t++);
  }
  while ( !Token.IsEmpty() );

  // Set result to be the last token
  Result = Result.Token(DSeparator.ToCString(), t - 2);;

  /* ====================================
    *  Get base name before extension dot
    * ==================================== */

  if ( !doKeepExt )
    Result = Result.Token(ActAux_Macro_DOT_STR, 1);

  return Result;
}

//! Extracts base path from the given string representing filename.
//! E.g. if the input string is "D:\dir1\dir2\hello.ext", the result is
//! "D:\dir1\dir2" or "D:\dir1\dir2\" depending on 'keepSlash' option.
//! \param theFilename [in] input filename.
//! \param keepSlash [in] indicates whether trailing slash must be
//!        contained in the resulting path.
//! \return extracted base path.
TCollection_AsciiString
  ActAux_Utils::Str::BasePath(const TCollection_AsciiString& theFilename,
                              const Standard_Boolean keepSlash)
{
  TCollection_AsciiString Result = theFilename;
  TCollection_AsciiString RSeparator(ActAux_Macro_RSLASH);
  TCollection_AsciiString DSeparator(ActAux_Macro_DSLASH);

  Standard_Integer TrimPos = theFilename.SearchFromEnd(RSeparator);
  if ( TrimPos == -1 )
  {
    TrimPos = theFilename.SearchFromEnd(DSeparator);
    if ( TrimPos == -1 )
      return Result;
  }

  Result = theFilename.SubString(1, TrimPos);

  // Proceed with trailing slash
  Standard_Character aLastChar = Result.Value( Result.Length() );
  if ( keepSlash )
  {
    Result = Slashed(Result);
  }
  else
  {
    if ( aLastChar == ActAux_Macro_RSLASH || aLastChar == ActAux_Macro_DSLASH )
      Result = Result.SubString(1, Result.Length() - 1);
  }

  return Result;
}

//! Reads real value recorded between the passed indices inclusively in
//! the given string.
//! \param theStr [in] source string.
//! \param theStart [in] 1-based start index.
//! \param theEnd [in] 1-based end index.
//! \param isOk [in] success/failure indicator.
Standard_Real ActAux_Utils::Str::RealAt(const TCollection_AsciiString& theStr,
                                        const Standard_Integer theStart,
                                        const Standard_Integer theEnd,
                                        Standard_Boolean& isOk)
{
  TCollection_AsciiString aResStr = theStr.SubString(theStart, theEnd);
  if ( !aResStr.IsRealValue() )
  {
    isOk = Standard_False;
    return DBL_MAX;
  }
  isOk = Standard_True;
  return aResStr.RealValue();
}

//! Reads integer value recorded between the passed indices inclusively in
//! the given string.
//! \param theStr [in] source string.
//! \param theStart [in] 1-based start index.
//! \param theEnd [in] 1-based end index.
//! \param isOk [in] success/failure indicator.
Standard_Integer ActAux_Utils::Str::IntAt(const TCollection_AsciiString& theStr,
                                          const Standard_Integer theStart,
                                          const Standard_Integer theEnd,
                                          Standard_Boolean& isOk)
{
  TCollection_AsciiString aResStr = theStr.SubString(theStart, theEnd);
  if ( !aResStr.IsIntegerValue() )
  {
    isOk = Standard_False;
    return INT_MAX;
  }
  isOk = Standard_True;
  return aResStr.IntegerValue();
}

//! Trims leading and trailing spaces on the given string.
//! \param theStr [in] string to adjust.
//! \return modified string.
TCollection_AsciiString ActAux_Utils::Str::Trim(const TCollection_AsciiString& theStr)
{
  TCollection_AsciiString aResult(theStr);
  aResult.LeftAdjust();
  aResult.RightAdjust();
  return aResult;
}

//! Fetches the next meaningful line from the passed file opened for reading.
//! Comment lines are skipped.
//! \param FILE [in] file handler.
//! \param theLine [out] fetched line.
//! \param theFilter [in] line filter.
//! \return meaningful (non-comment) line.
Standard_Boolean ActAux_Utils::Str::FetchLine(OSD_File& FILE,
                                              TCollection_AsciiString& theLine,
                                              ActAux_StrFilter theFilter)
{
  if ( FILE.IsAtEnd() )
    return Standard_False;

  // Stuff for streaming
  Standard_Integer NbChar;
  Standard_Boolean isOk = Standard_False;

  // Fetch first meaningful line
  do
  {
    FILE.ReadLine(theLine, BUFLEN, NbChar);

    if ( theLine.IsEmpty() || ( theFilter && !theFilter(theLine) ) )
      continue;

    isOk = Standard_True;
  }
  while ( !isOk && !FILE.IsAtEnd() );

  return isOk;
}

//! Extracts token with the given index from the passed line. If token is
//! empty, fetches another line. The main advantage of this function is that
//! you should not care about fetching new line and re-initializing integer
//! token counter when parse your target file.
//! \param theLine [in/out] line to get token from. Can be changed to the
//!        newly fetched one.
//! \param theIdx [in/out] index of the next token to access. Can be reset
//!        in case when new line is fetched.
//! \param theToken [out] next token.
//! \param FILE [in] file handler.
//! \param theSeparator [in] separator.
//! \param theFilter [in] line filter.
//! \return true in case of success (next token exists), false -- otherwise.
Standard_Boolean ActAux_Utils::Str::NextToken(TCollection_AsciiString& theLine,
                                              Standard_Integer& theIdx,
                                              TCollection_AsciiString& theToken,
                                              OSD_File& FILE,
                                              const TCollection_AsciiString& theSeparator,
                                              ActAux_StrFilter theFilter)
{
  theToken = Trim( theLine.Token(theSeparator.ToCString(), theIdx++) );
  if ( theToken.IsEmpty() )
  {
    if ( !FetchLine(FILE, theLine, theFilter) )
      return Standard_False;

    theIdx = 1;
    theToken = Trim( theLine.Token(theSeparator.ToCString(), theIdx++) );
  }
  return Standard_True;
}

//! Splits the passed string by the given delimiter.
//! \param theLine [in] line to split.
//! \param theSeparator [in] delimiter.
//! \param theChunks [out] splitting result.
void ActAux_Utils::Str::Split(const TCollection_AsciiString& theLine,
                              const TCollection_AsciiString& theSeparator,
                              NCollection_Sequence<TCollection_AsciiString>& theChunks)
{
  Standard_Integer t = 1;
  Standard_Boolean stop = Standard_False;
  do
  {
    TCollection_AsciiString T = Trim( theLine.Token(theSeparator.ToCString(), t++) );
    if ( T.IsEmpty() )
      stop = Standard_True;
    else
      theChunks.Append(T);
  }
  while ( !stop );
}

//! Adds trailing slash to the passed string if necessary.
//! \param theStr [in] input string.
//! \return resulting string.
TCollection_AsciiString
  ActAux_Utils::Str::Slashed(const TCollection_AsciiString& theStr)
{
  if ( theStr.IsEmpty() )
    return theStr;

  TCollection_AsciiString Result = theStr;
  Standard_Character lastChar = Result.Value( Result.Length() );
  if ( lastChar != ActAux_Macro_RSLASH && lastChar != ActAux_Macro_DSLASH )
    Result += ActAux_Macro_RSLASH;

  return Result;
}
