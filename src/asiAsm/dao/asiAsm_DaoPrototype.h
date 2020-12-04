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

#ifndef asiAsm_DaoPrototype_h
#define asiAsm_DaoPrototype_h

// dao includes
#include <asiAsm_DaoBase.h>

// entity includes
#include <asiAsm_EntPrototype.h>

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! DAO for prototypes.
class asiAsm_DaoPrototype : public asiAsm_DaoBase
{
public:

  // Default ctor.
  asiAsm_DaoPrototype() : asiAsm_DaoBase() {}

  //! Ctor.
  //! \param[in] dbName   database name.
  //! \param[in] progress progress notifier.
  asiAsm_DaoPrototype(const std::string&   dbName,
                      ActAPI_ProgressEntry progress)
  //
  : asiAsm_DaoBase(dbName, progress) {}

public:

  /** @name CRUD
   *  Basic services for Create, Read, Update, Delete (CRUD).
   */
  //@{

  //! Creates a prototype.
  //! \param[in] ent entity to store.
  //! \return ID of the created prototype record or -1 if the request is
  //!         declined for whatever reason.
  asiAsm_EXPORT int
    Create(const asiAsm_EntPrototype& ent) const;

  //! Reads prototype with the passed ID.
  //! \param[in]  id  ID of the prototype to request.
  //! \param[out] ent retrieved prototype.
  //! \return false if the data cannot be retrieved for whatever reason.
  asiAsm_EXPORT bool
    Read(const int            id,
         asiAsm_EntPrototype& ent) const;

  //! Updates the prototype with the passed ID.
  //! \param[in] id  ID of the prototype of interest.
  //! \param[in] ent new prototype data to set.
  //! \return false if the data cannot be updated for whatever reason.
  asiAsm_EXPORT bool
    Update(const int                  id,
           const asiAsm_EntPrototype& ent) const;

  //! Deletes the prototype record having the passed ID.
  //! \param[in] id ID of the prototype to delete.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT bool
    Delete(const int id) const;

  //@}

private:

  //! \return table name.
  virtual const char* tableName() const
  {
    return asiAsm_Macro_Table_Prototype;
  }

  //! Creates table.
  //! \return true in case of success, false -- otherwise.
  asiAsm_EXPORT virtual bool
    createTable() const;

};

#endif
