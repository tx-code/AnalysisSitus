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

#ifndef asiAsm_ServiceModel_h
#define asiAsm_ServiceModel_h

// service includes
#include <asiAsm.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Data model facade.
class asiAsm_ServiceModel : public Standard_Transient
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAsm_ServiceModel, Standard_Transient)

public:

  //! Ctor.
  //! \param[in] progress progress entry.
  //! \param[in] plotter  imperative plotter.
  asiAsm_EXPORT
    asiAsm_ServiceModel(ActAPI_ProgressEntry progress = nullptr,
                        ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! \return database name.
  const std::string& GetDbName() const
  {
    return m_dbName;
  }

public:

  //! Creates new empty Data Model.
  //! \param[in] dbName database name.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    NewEmpty(const std::string& dbName);

  //! Loads Data Model by initialization of a database connection.
  //! \param[in] dbName database name.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    Load(const std::string& dbName);

  //! Releases data model.
  asiAsm_EXPORT void
    Release();

protected:

  //! Populates database with empty tables.
  asiAsm_EXPORT virtual bool
    initSchema();

protected:

  //! Database name.
  std::string m_dbName;

  ActAPI_ProgressEntry m_progress; //!< Progress entry.
  ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

#endif
