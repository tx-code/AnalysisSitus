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
#include <service_Model.h>

// dao includes
#include <asiAsm_DaoPrototype.h>

// Qt includes
#pragma warning(push, 0)
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#pragma warning(pop)

using namespace asiAsm;

//-----------------------------------------------------------------------------

service::Model::Model(ActAPI_ProgressEntry progress,
                      ActAPI_PlotterEntry  plotter)
//
: Standard_Transient (),
  m_progress         (progress),
  m_plotter          (plotter)
{}

//-----------------------------------------------------------------------------

bool service::Model::NewEmpty(const std::string& dbName)
{
  m_dbName = dbName;

  // Prepare database.
  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", m_dbName.c_str() );
  db.setDatabaseName( m_dbName.c_str() );
  //
  if ( !db.isValid() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Database connection '%1' is invalid."
                                             << m_dbName);
    return false;
  }
  //
  if ( !db.open() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Connection with '%1' failed to open."
                                             << m_dbName);
    return false;
  }

  // Initialize schema.
  if ( !this->initSchema() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to initialize database schema.");
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool service::Model::Load(const std::string& dbName)
{
  m_dbName = dbName;

  QSqlDatabase db = QSqlDatabase::database( m_dbName.c_str() );
  //
  if ( !db.isValid() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Connection to '%1' failed: invalid db."
                                             << m_dbName);
    return false;
  }
  //
  if ( !db.open() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Connection with '%1' failed to open."
                                             << m_dbName);
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

void service::Model::Release()
{
  QSqlDatabase db = QSqlDatabase::database( m_dbName.c_str() );

  db.close();
}

//-----------------------------------------------------------------------------

bool service::Model::initSchema()
{
  // Prototype.
  if ( !dao::Prototype(m_dbName, m_progress).CreateTable() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot create table for prototypes.");
    return false;
  }

  return true;
}
