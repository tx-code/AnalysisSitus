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

#ifndef asiAsm_DaoBase_h
#define asiAsm_DaoBase_h

// asiAsm includes
#include <asiAsm_Dict.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>

class QSqlQuery;
class QString;

//-----------------------------------------------------------------------------

//! \ingroup ASIASM
//!
//! Base class for all Data Access Objects. DAO is a communicator between
//! C++ and database which manages all SQL requests. We use these manually
//! crafted DAOs instead of any sort of dirty automatic ORM approaches.
class asiAsm_DaoBase
{
public:

  //! Status of DAO.
  enum t_status
  {
    asiAsm_DaoDetached = 0, //!< DAO is detached from DB and cannot query data.
    asiAsm_DaoAttached      //!< DAO is attached and ready for querying data.
  };

public:

  //! \return status of DAO w.r.t. database connection.
  asiAsm_EXPORT t_status
    GetStatus() const;

public:

  //! Initializes DAO with the database name and the progress notifier.
  //! \param[in] dbName   database name.
  //! \param[in] progress progress entry.
  void Init(const std::string&   dbName,
            ActAPI_ProgressEntry progress = nullptr)
  {
    this->SetDbName(dbName);
    m_progress = progress;
  }

  //! Sets name of the database used for data access.
  //! \param[in] dbName database name.
  void SetDbName(const std::string& dbName)
  {
    m_dbName = dbName;
  }

  //! \return name of database used for data access.
  const std::string& GetDbName() const
  {
    return m_dbName;
  }

  //! Sets progress notifier.
  //! \param[in] progress progress notifier to set.
  virtual void SetProgress(ActAPI_ProgressEntry progress)
  {
    m_progress = progress;
  }

public:

  //! Creates table for the specific data type represented by this DAO.
  //! \return true in case of success, false -- otherwise.
  virtual bool CreateTable() const
  {
    if ( !this->attach() )
      return false;

    if ( this->CheckTableExists( this->tableName() ) )
    {
      m_progress.SendLogMessage( LogWarn(Normal) << "Table `%1` already exists."
                                                 << this->tableName() );
      return false;
    }

    return this->createTable();
  }

public:

  //! Executes SQL query.
  //! \param[out] query prepared query.
  //! \return false in case of failure.
  asiAsm_EXPORT bool
    ExecuteQuery(QSqlQuery& preparedQuery) const;

  //! Executes SQL query passed as a plain string.
  //! \param[in]  queryStr query to execute.
  //! \param[out] query    executed query.
  //! \return false in case of failure.
  asiAsm_EXPORT bool
    ExecuteQuery(const std::string& queryStr,
                 QSqlQuery&         query) const;

  //! Executes SQL query passed as a plain string.
  //! \param[in] queryStr query to execute.
  //! \return false in case of failure.
  asiAsm_EXPORT bool
    ExecuteQuery(const std::string& queryStr) const;

  //! Executes SQL query passed as a plain string.
  //! \param[in]  queryStr query to execute.
  //! \param[out] query    executed query.
  //! \return false in case of failure.
  asiAsm_EXPORT bool
    ExecuteQuery(const QString& queryStr,
                 QSqlQuery&     query) const;

  //! Executes SQL query passed as a plain string.
  //! \param[in] queryStr query to execute.
  //! \return false in case of failure.
  asiAsm_EXPORT bool
    ExecuteQuery(const QString& queryStr) const;

  //! Checks whether a table with the passed name exists or not.
  //! \param[in] name table name.
  //! \return true if the table exists, false -- otherwise.
  asiAsm_EXPORT bool
    CheckTableExists(const std::string& name) const;

protected:

  //! Default ctor.
  asiAsm_DaoBase() {}

  //! Ctor.
  //! \param[in] dbName   database name.
  //! \param[in] progress progress entry.
  asiAsm_DaoBase(const std::string&   dbName,
                 ActAPI_ProgressEntry progress = nullptr)
  //
  : m_dbName   (dbName),
    m_progress (progress)
  {}

protected:

  //! Attempts to attach DAO to the database. This method first checks that
  //! the database is valid. If so, it checks if the connection is opened.
  //! If the connection is not opened, this method attempts to open it.
  //!
  //! \return false is the DAO fails to attach to the database.
  asiAsm_EXPORT bool
    attach() const;

private:

  //! \return table name corresponding to the specific data type.
  virtual const char* tableName() const = 0;

  //! The subclasses have to implement this method to provide the logic how
  //! to create the data type's corresponding table.
  virtual bool createTable() const = 0;

protected:

  //! Database to connect.
  std::string m_dbName;

  //! Progress notifier.
  ActAPI_ProgressEntry m_progress;

};

#endif
