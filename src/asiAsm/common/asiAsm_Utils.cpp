//-----------------------------------------------------------------------------
// Created on: 22 September 2020 (*)
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <asiAsm_Utils.h>

using namespace sqlc::common;

//-----------------------------------------------------------------------------

void Utils::Str::Split(const std::string&        source_str,
                       const std::string&        delim_str,
                       std::vector<std::string>& result)
{
  // Initialize collection of strings to split.
  std::vector<std::string> chunks;
  chunks.push_back(source_str);

  // Split by each delimiter consequently.
  for ( size_t delim_idx = 0; delim_idx < delim_str.length(); ++delim_idx )
  {
    std::vector<std::string> new_chunks;
    const char delim = delim_str[delim_idx];

    // Split each chunk
    for ( size_t chunk_idx = 0; chunk_idx < chunks.size(); ++chunk_idx )
    {
      const std::string& source = chunks[chunk_idx];
      std::string::size_type currPos = 0, prevPos = 0;
      while ( (currPos = source.find(delim, prevPos) ) != std::string::npos )
      {
        std::string item = source.substr(prevPos, currPos - prevPos);
        if ( item.size() > 0 )
        {
          new_chunks.push_back(item);
        }
        prevPos = currPos + 1;
      }
      new_chunks.push_back( source.substr(prevPos) );
    }

    // Set new collection of chunks for splitting by the next delimiter.
    chunks = new_chunks;
  }

  // Set result
  result = chunks;
}

//-----------------------------------------------------------------------------

void Utils::Str::Split(const TCollection_AsciiString&        source_str,
                       const char*                           delim,
                       std::vector<TCollection_AsciiString>& result)
{
  int tt = 1;
  TCollection_AsciiString chunk;
  bool stop = false;

  do
  {
    chunk = source_str.Token(delim, tt++);
    //
    if ( !chunk.IsEmpty() )
      result.push_back(chunk);
    else
      stop = true;
  }
  while ( !stop );
}

//-----------------------------------------------------------------------------

void Utils::Str::ReplaceAll(std::string&       str,
                            const std::string& what,
                            const std::string& with)
{
  for ( size_t pos = 0; ; pos += with.length() )
  {
    pos = str.find(what, pos); // Locate the substring to replace.
    if ( pos == std::string::npos )
      break; // Not found.

    // Replace by erasing and inserting.
    str.erase( pos, what.length() );
    str.insert(pos, with);
  }
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  Utils::Str::Slashed(const TCollection_AsciiString& strIN)
{
  std::string str( strIN.ToCString() );
  //
  if ( !str.length() )
    return str.c_str();

  char c = str.at(str.length() - 1);
  if ( c == *sqlc_SlashStr )
    return str.c_str();

  std::string strOUT(str);
  strOUT.append(sqlc_SlashStr);
  return strOUT.c_str();
}
