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

#ifndef service_Prototype_h
#define service_Prototype_h

// service includes
#include <service_Base.h>

// entity includes
#include <asiAsm_EntPrototype.h>

// dao includes
#include <asiAsm_DaoPrototype.h>

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace service {

//! \ingroup ASIASM
//!
//! Service for manipulating with the prototypes.
class Prototype : public Base
{
public:

  //! Default ctor.
  asiAsm_EXPORT
    Prototype();

  //! Ctor.
  //! \param[in] dbName   database name.
  //! \param[in] progress progress notifier.
  asiAsm_EXPORT
    Prototype(const std::string&   dbName,
              ActAPI_ProgressEntry progress = NULL);

public:

  //! Creates new prototype.
  //! \param[in] ent entity with the data to create a database record.
  //! \return ID of the new prototype.
  asiAsm_EXPORT int
    Create(const entity::Prototype& ent) const;

public:

  //! \return reference to DAO.
  virtual dao::Base& GetDAO()
  {
    return m_dao;
  }

protected:

  //! Prototype DAO.
  dao::Prototype m_dao;

};

} // service
} // asiAsm

#endif
