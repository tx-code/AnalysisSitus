//-----------------------------------------------------------------------------
// Created on: 21 January 2021
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

// Own include
#include <asiAlgo_ShapeSerializer.h>

// OpenCascade includes
#include <BinTools.hxx>
#include <TopoDS_Shape.hxx>

static std::string __base64 =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";

//-----------------------------------------------------------------------------
// Base64 encoding and decoding is provided by Rene Nyffenegger
// rene.nyffenegger@adp-gmbh.ch.
//
// The modified versions are isBase64, encodeBase64, decodeBase64.
//
// Copyright (C) 2004-2008 Rene Nyffenegger
//-----------------------------------------------------------------------------

bool asiAlgo_ShapeSerializer::Serialize(const TopoDS_Shape& shape,
                                        std::string&        strShape,
                                        const bool          isBin)
{
  if ( shape.IsNull() )
    return false;

  std::string str;
  //
  if ( !convertShapeToBin(shape, str) )
    return false;

  if ( !isBin )
  {
    strShape = encodeBase64( (const unsigned char*) str.c_str(),
                             (unsigned int) str.size());
  }
  else
  {
    strShape = str;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ShapeSerializer::Deserialize(const std::string& strShape,
                                          TopoDS_Shape&      shape,
                                          const bool         isBin)
{
  if ( strShape.size() == 0 )
    return false;

  // Decode from base64 if necessary (normally, yes).
  std::string str;
  if ( !isBin )
    str = decodeBase64( strShape.c_str() );
  else
    str = strShape;

  // Construct shape.
  return convertBinToShape(str, shape);
}

//-----------------------------------------------------------------------------

bool asiAlgo_ShapeSerializer::isBase64(const unsigned char character)
{
  return ( isalnum(character) || (character == '+') || (character == '/') );
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_ShapeSerializer::encodeBase64(const unsigned char* buffer,
                                        const unsigned int   bufferLen)
{
  std::string result;
  unsigned char buffer3[3];
  unsigned char buffer4[4];
  //
  int len = bufferLen;
  int i   = 0;
  int j   = 0;

  while ( len-- )
  {
    buffer3[i++] = *(buffer++);
    if ( i == 3 )
    {
      buffer4[0] =   (buffer3[0] & 0xfc) >> 2;
      buffer4[1] = ( (buffer3[0] & 0x03) << 4 ) + ( (buffer3[1] & 0xf0) >> 4 );
      buffer4[2] = ( (buffer3[1] & 0x0f) << 2 ) + ( (buffer3[2] & 0xc0) >> 6 );
      buffer4[3] =    buffer3[2] & 0x3f;

      for ( i = 0; i < 4; ++i )
        result += __base64[buffer4[i]];

      i = 0;
    }
  }

  if ( i )
  {
    for ( j = i; j < 3; ++j )
      buffer3[j] = '\0';

    buffer4[0] =   (buffer3[0] & 0xfc) >> 2;
    buffer4[1] = ( (buffer3[0] & 0x03) << 4) + ( (buffer3[1] & 0xf0) >> 4 );
    buffer4[2] = ( (buffer3[1] & 0x0f) << 2) + ( (buffer3[2] & 0xc0) >> 6 );
    buffer4[3] =    buffer3[2] & 0x3f;

    for ( j = 0; j < i + 1; ++j )
      result += __base64[buffer4[j]];

    while ( i++ < 3 )
      result += '=';
  }

  return result;
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_ShapeSerializer::decodeBase64(const std::string& encodedBuffer)
{
  int in_len = (int) encodedBuffer.size();
  int i      = 0;
  int j      = 0;
  int in     = 0;
  unsigned char buffer4[4], buffer3[3];
  std::string result;

  while ( in_len-- &&
          ( encodedBuffer[in] != '=') &&
            isBase64(encodedBuffer[in] ) )
  {
    buffer4[i++] = encodedBuffer[in]; in++;

    if ( i == 4 )
    {
      for ( i = 0; i < 4; ++i )
        buffer4[i] = (unsigned char) __base64.find(buffer4[i]);

      buffer3[0] =   (buffer4[0]        << 2) + ( (buffer4[1] & 0x30) >> 4 );
      buffer3[1] = ( (buffer4[1] & 0xf) << 4) + ( (buffer4[2] & 0x3c) >> 2 );
      buffer3[2] = ( (buffer4[2] & 0x3) << 6) +    buffer4[3];

      for ( i = 0; i < 3; ++i )
        result += buffer3[i];

      i = 0;
    }
  }

  if ( i )
  {
    for ( j = i; j < 4; ++j )
      buffer4[j] = 0;

    for ( j = 0; j < 4; ++j )
      buffer4[j] = (unsigned char) __base64.find(buffer4[j]);

    buffer3[0] =   (buffer4[0]        << 2 ) + ( (buffer4[1] & 0x30) >> 4 );
    buffer3[1] = ( (buffer4[1] & 0xf) << 4 ) + ( (buffer4[2] & 0x3c) >> 2 );
    buffer3[2] = ( (buffer4[2] & 0x3) << 6 ) +    buffer4[3];

    for ( j = 0; j < i - 1; ++j )
      result += buffer3[j];
  }

  return result;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ShapeSerializer::convertBinToShape(const std::string& binStrShape,
                                                TopoDS_Shape&      shape)
{
  if ( binStrShape.size() == 0 )
    return false;

  // Use OpenCascade tool to recover the shape.
  std::stringbuf buffer(binStrShape, std::ios::in | std::ios::binary);
  std::istream stream(&buffer);
  //
  try
  {
    BinTools::Read(shape, stream);
  }
  catch ( ... )
  {
    return false; // Bad buffer.
  }

  return stream.good();
}

//-----------------------------------------------------------------------------

bool asiAlgo_ShapeSerializer::convertShapeToBin(const TopoDS_Shape& shape,
                                                std::string&        binStrShape)
{
  if ( shape.IsNull() )
    return false;

  std::stringbuf buffer(std::ios::out | std::ios::binary);
  std::ostream stream(&buffer);
  //
  BinTools::Write(shape, stream);
  binStrShape = buffer.str();
  //
  return stream.good();
}
