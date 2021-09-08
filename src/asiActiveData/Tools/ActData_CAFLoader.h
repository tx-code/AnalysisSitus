//-----------------------------------------------------------------------------
// Created on: March 2013
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

#ifndef ActData_CAFLoader_HeaderFile
#define ActData_CAFLoader_HeaderFile

// Active Data includes
#include <ActData_BaseModel.h>

// OCCT includes
#include <Message_ProgressIndicator.hxx>

DEFINE_STANDARD_HANDLE(ActData_CAFLoader, Standard_Transient)

//! \ingroup AD_DF
//!
//! Loader class for Data Model instances. This class enriches default
//! loading functionality with compatibility conversion mechanism.
class ActData_CAFLoader : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActData_CAFLoader, Standard_Transient)

public:

  ActData_EXPORT static Standard_Boolean
    LoadAndConvert(const TCollection_AsciiString& theFilename,
                   Handle(ActAPI_IModel)& theModel);

  ActData_EXPORT static Standard_Boolean
    LoadWithVersions(const TCollection_AsciiString& theFilename,
                     Handle(ActAPI_IModel)& theModel,
                     Standard_Integer& theFwVerStored,
                     Standard_Integer& theAppVerStored,
                     Standard_Integer& theFwVerActual,
                     Standard_Integer& theAppVerActual);

  ActData_EXPORT static Standard_Boolean
    Convert(Handle(ActAPI_IModel)& theModel,
            const Standard_Integer theFwVerStored,
            const Standard_Integer theAppVerStored,
            const Standard_Integer theFwVerActual,
            const Standard_Integer theAppVerActual,
            const TCollection_AsciiString& theFnBefore = TCollection_AsciiString(),
            const TCollection_AsciiString& theFnAfter = TCollection_AsciiString(),
            const Handle(Message_ProgressIndicator)& theProgress = nullptr);

  ActData_EXPORT static Standard_Boolean
    CheckWellFormed(const Handle(ActAPI_IModel)& theModel);

  ActData_EXPORT static Standard_Boolean
    ReconnectTreeFunctions(const Handle(ActAPI_IModel)& theModel);

public:

  ActData_EXPORT
    ActData_CAFLoader(const Handle(ActAPI_IModel)& theModel);

public:

  ActData_EXPORT Standard_Boolean
    Load(const TCollection_AsciiString& theFilename);

  ActData_EXPORT Standard_Boolean
    NeedsConversion() const;

  ActData_EXPORT Standard_Boolean
    Convert(const TCollection_AsciiString& theFnBefore = TCollection_AsciiString(),
            const TCollection_AsciiString& theFnAfter = TCollection_AsciiString(),
            const Handle(Message_ProgressIndicator)& theProgress = nullptr);

  ActData_EXPORT Standard_Boolean
    CheckWellFormed() const;

  ActData_EXPORT Standard_Boolean
    ReconnectTreeFunctions();

public:

  //! Returns converted Data Model.
  //! \return converted Data Model instance.
  inline const Handle(ActAPI_IModel)& ModifiedModel() const
  {
    return m_model;
  }

  //! Returns stored version of Framework.
  //! \return requested version string.
  inline TCollection_AsciiString FwStoredVersion() const
  {
    return versionToString(m_fwVerStored);
  }

  //! Returns actual version of Framework.
  //! \return requested version string.
  inline TCollection_AsciiString FwActualVersion() const
  {
    return versionToString(m_fwVerActual);
  }

  //! Returns stored version of Application.
  //! \return requested version string.
  inline TCollection_AsciiString AppStoredVersion() const
  {
    return versionToString(m_appVerStored);
  }

  //! Returns actual version of Application.
  //! \return requested version string.
  inline TCollection_AsciiString AppActualVersion() const
  {
    return versionToString(m_appVerActual);
  }

  //! Returns filename of the currently loaded Data Model.
  //! \return requested filename.
  inline const TCollection_AsciiString& Filename() const
  {
    return m_filename;
  }

private:

  ActData_EXPORT TCollection_AsciiString
    versionToString(const Standard_Integer theVerNum) const;

private:

  //! Default ctor.
  ActData_CAFLoader() : m_fwVerStored(-1), m_appVerStored(-1), m_fwVerActual(-1), m_appVerActual(-1) {}

private:

  //! Loaded and optionally converted Data Model.
  Handle(ActAPI_IModel) m_model;

  //! Loaded version of Framework.
  Standard_Integer m_fwVerStored;

  //! Loaded version of Application.
  Standard_Integer m_appVerStored;

  //! Actual version of Framework.
  Standard_Integer m_fwVerActual;

  //! Actual version of Application.
  Standard_Integer m_appVerActual;

  //! Filename of the currently loaded Data Model.
  TCollection_AsciiString m_filename;

};

#endif
