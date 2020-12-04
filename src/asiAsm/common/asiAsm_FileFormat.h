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

#ifndef asiAsm_XdeFileFormat_h
#define asiAsm_XdeFileFormat_h

// asiAsm includes
#include <asiAsm.h>

// OCCT includes
#include <NCollection_IndexedMap.hxx>
#include <TCollection_AsciiString.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Supported file types.
enum FileFormat
{
  FileFormat_Unknown = 0, //!< Unknown file format.

  //---------------------------------------------------------------------------
  // Native
  //---------------------------------------------------------------------------
  FileFormat_NATIVE, //!< Native project format.

  //---------------------------------------------------------------------------
  // CAD
  //---------------------------------------------------------------------------
  FileFormat_BREP,
  FileFormat_STEP,
  FileFormat_IGES,

  //---------------------------------------------------------------------------
  // Mesh
  //---------------------------------------------------------------------------
  FileFormat_STL,
  FileFormat_PLY,

  //---------------------------------------------------------------------------
  // Others
  //---------------------------------------------------------------------------
  FileFormat_XML,

  //---------------------------------------------------------------------------
  // Last item
  //---------------------------------------------------------------------------
  FileFormat_Last //!< Last item for convenient iteration over enum.
};

//! \ingroup ASIASM
//!
//! Utility for recognizing CAD data formats and filtering out
//! unavailable converters.
class asiAsm_XdeFileFormatTool
{
public:

  //! The list of names for all supported CAD formats.
  asiAsm_EXPORT static const char* const
    NAMED_FORMATS[FileFormat_Last];

public:

  //! Ctor accepting the application-specific extension for native formats.
  //! \param[in] nativeExt extension for native (project) files.
  FileFormatTool(const char* nativeExt) : m_nativeExt(nativeExt) {}

public:

  //! Converts format type enum format name.
  //! \param[in] formatType format type in question.
  //! \return format name.
  static const char* NameFromFormat(const FileFormat formatType)
  {
    return formatType >= 0 && formatType < FileFormat_Last
        ? NAMED_FORMATS[formatType]
        : NAMED_FORMATS[FileFormat_Unknown];
  }

  //! Converts format string to format type enum.
  //! \param[in] formatName format name in question.
  //! \return format type enum.
  static FileFormat FormatFromName(const char* formatName)
  {
    for ( int it = 0; it < FileFormat_Last; ++it )
    {
      if ( !std::strcmp(formatName, NAMED_FORMATS[it]) )
        return (FileFormat) it;
    }
    return FileFormat_Unknown;
  }

public:

  //! Returns file extension from the name in lower case.
  //! \param[in] path full filename.
  //! \return file extension.
  asiAsm_EXPORT static TCollection_AsciiString
    GetFileExtension(const TCollection_AsciiString& path);

public:

  //! Returns file format analyzing the file extension.
  //! \param[in] path full filename.
  //! \return file format enum.
  asiAsm_EXPORT FileFormat
    FormatFromFileExtension(const TCollection_AsciiString& path);

  //! Returns file format analyzing the file contents.
  //! \param[in] path full filename.
  //! \return file format enum.
  asiAsm_EXPORT FileFormat
    FormatFromFileContent(const TCollection_AsciiString& path);

public:

  //! Returns true if the passed CAD data format can be exported.
  //! \param[in] format data format in question.
  //! \return true/false.
  asiAsm_EXPORT virtual bool
    IsExportSupported(const FileFormat format);

  //! Returns true if the passed CAD data format can be imported.
  //! \param[in] format data format in question.
  //! \return true/false.
  asiAsm_EXPORT virtual bool
    IsImportSupported(const FileFormat format);

protected:

  const char* m_nativeExt; //!< Extension for files in native format.

};

#endif
