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

#ifndef asiAlgo_ShapeSerializer_h
#define asiAlgo_ShapeSerializer_h

// asiAlgo includes
#include <asiAlgo.h>

// Standard includes
#include <string>

class TopoDS_Shape;

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Serializes and deserializes B-rep shapes to and from binary buffer,
//! optionally using the base64 binary-to-text encoding scheme.
//!
//! The B-rep data serialized with base64 encoding can be embedded into text
//! files, e.g., XML or JSON.
class asiAlgo_ShapeSerializer
{
public:

  //! Converts shape to base64-encoded binary buffer.
  //! \param[in]  shape    the shape to serialize.
  //! \param[out] strShape the shape in a serialized form.
  //! \param[in]  isBin    the Boolean flag indicating whether the string
  //!                      representation of the shape should be prepared
  //!                      as a raw binary buffer without base64 encoding.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    Serialize(const TopoDS_Shape& shape,
              std::string&        strShape,
              const bool          isBin = false);

  //! Restores B-rep shape from its serialized representation.
  //! \param[in]  strShape the shape in a serialized form.
  //! \param[out] shape    the output OpenCascade's shape.
  //! \param[in]  isBin    the Boolean flag indicating whether the string
  //!                      representation of the shape is given as a raw binary
  //!                      buffer, i.e., without base64 encoding. If so, the
  //!                      passed buffer will not undergo the decoding phase.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    Deserialize(const std::string& strShape,
                TopoDS_Shape&      shape,
                const bool         isBin = false);

protected:

  //! Checks if a character is base64-encoded.
  //! \param[in] character the char to check.
  //! \return true if character is base64-encoded, false -- otherwise.
  asiAlgo_EXPORT static bool
    isBase64(const unsigned char character);

  //! Encodes the passed buffer as base64-encoded string.
  //! \param[in] buffer    the input buffer to encode.
  //! \param[in] bufferLen the input buffer's length.
  //! \return base64-encoded representation of the buffer.
  asiAlgo_EXPORT static std::string
    encodeBase64(const unsigned char* buffer,
                 const unsigned int   bufferLen);

  //! Decodes a string from base64-encoded buffer.
  //! \param[in] encodedBuffer the buffer to decode.
  //! \return decoded buffer as string.
  asiAlgo_EXPORT static std::string
    decodeBase64(const std::string& encodedBuffer);

  //! Converts shape from a binary buffer to the topological representation.
  //! \param[in]  binStrShape the shape as a binary buffer.
  //! \param[out] shape       the output OpenCascade's shape.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    convertBinToShape(const std::string& binStrShape,
                      TopoDS_Shape&      shape);

  //! Converts the passed shape to a raw binary buffer.
  //! \param[in]  shape       the shape to convert.
  //! \param[out] binStrShape the output binary buffer.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    convertShapeToBin(const TopoDS_Shape& shape,
                      std::string&        binStrShape);

private:

  //! Ctor is not for use.
  asiAlgo_ShapeSerializer() = delete;

};

#endif
