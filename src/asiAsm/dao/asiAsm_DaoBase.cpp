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

// Own includes
#include <asiAsm_DaoBase.h>

// asiAsm includes
#include <asiAsm_Utils.h>

// Qt includes
#pragma warning(push, 0)
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#pragma warning(pop)

//-----------------------------------------------------------------------------

asiAsm_DaoBase::t_status asiAsm_DaoBase::GetStatus() const
{
  QSqlDatabase db = QSqlDatabase::database( m_dbName.c_str() );

  if ( !db.isValid() )
    return asiAsm_DaoDetached;

  return asiAsm_DaoAttached;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::ExecuteQuery(QSqlQuery& preparedQuery) const
{
  // Perform contract check.
  if ( !this->attach() )
    return false;

  m_progress.SendLogMessage( LogInfo(Normal) << "Executing prepared query.");

  if ( !preparedQuery.exec() )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "Error (%1) executing SQL query\n\t%2."
                                              << asiAsm_Utils::Str::ToAsciiString( preparedQuery.lastError().text() )
                                              << asiAsm_Utils::Str::ToAsciiString( preparedQuery.executedQuery() ) );
    return false;
  }

  m_progress.SendLogMessage( LogNotice(Normal) << "Executed prepared SQL query\n\t%2."
                                               << asiAsm_Utils::Str::ToAsciiString( preparedQuery.executedQuery() ) );
  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::ExecuteQuery(const std::string& queryStr,
                                  QSqlQuery&         query) const
{
  return this->ExecuteQuery( QString( queryStr.c_str() ), query );
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::ExecuteQuery(const std::string& queryStr) const
{
  QSqlQuery query;
  return this->ExecuteQuery(queryStr, query);
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::ExecuteQuery(const QString& queryStr,
                                  QSqlQuery&     query) const
{
  // Perform contract check.
  if ( !this->attach() )
    return false;

  m_progress.SendLogMessage( LogNotice(Normal) << "Executing query\n\t%1."
                                               << asiAsm_Utils::Str::ToAsciiString(queryStr) );

  QSqlDatabase db = QSqlDatabase::database( m_dbName.c_str() );

  // Execute query.
  query = QSqlQuery(db);
  //
  if ( !query.exec(queryStr) )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "Error executing SQL query: %1."
                                              << asiAsm_Utils::Str::ToAsciiString( query.lastError().text() ) );
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::ExecuteQuery(const QString& queryStr) const
{
  QSqlQuery query;
  return this->ExecuteQuery(queryStr, query);
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::CheckTableExists(const std::string& name) const
{
  // Prepare query.
  QSqlQuery query( QSqlDatabase::database( m_dbName.c_str() ) );
  //
  query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=':name'");
  //
  query.bindValue( ":name", name.c_str() );
  //
  if ( !this->ExecuteQuery(query) )
    return false;

  query.last();
  const int rowCount = query.at() + 1;
  return rowCount > 0;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoBase::attach() const
{
  QSqlDatabase db = QSqlDatabase::database( m_dbName.c_str() );

  if ( !db.isValid() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Database connection '%1' is not valid."
                                             << m_dbName);
    return false;
  }

  if ( db.isOpen() )
    return true;

  if ( !db.open() )
  {
    m_progress.SendLogMessage( LogErr(Normal) << "Cannot open database '%1': %2."
                                              << m_dbName
                                              << asiAsm_Utils::Str::ToAsciiString( db.lastError().text() ) );
    return false;
  }

  return true;
}
