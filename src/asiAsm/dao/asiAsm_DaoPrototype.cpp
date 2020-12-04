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

// dao includes
#include <asiAsm_DaoPrototype.h>

// Qt includes
#pragma warning(push, 0)
#include <QSqlQuery>
#include <QVariant>
#pragma warning(pop)

using namespace asiAsm;

//-----------------------------------------------------------------------------

int asiAsm_DaoPrototype::Create(const entity::Prototype& ent) const
{
  QSqlDatabase db = QSqlDatabase::database( m_dbName.c_str() );

  // Prepare and execute query.
  QSqlQuery query(db);
  //
  query.prepare("INSERT INTO t_prototype "
                "(name, is_assembly) "
                "VALUES "
                "(:name, :is_assembly)");
  //
  query.bindValue( ":name",        ent.name.c_str() );
  query.bindValue( ":is_assembly", ent.is_assembly ? 1 : 0 );
  //query.bindValue( ":metadata_id", ent.metadata_id );
  //
  const bool isOk = this->ExecuteQuery(query);

  if ( !isOk )
    return -1;

  const int id = query.lastInsertId().toInt();
  return id;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoPrototype::Read(const int          id,
                               entity::Prototype& ent) const
{
  // TODO: NYI
  return false;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoPrototype::Update(const int                id,
                                 const entity::Prototype& ent) const
{
  // TODO: NYI
  return false;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoPrototype::Delete(const int id) const
{
  // TODO: NYI
  return false;
}

//-----------------------------------------------------------------------------

bool asiAsm_DaoPrototype::createTable() const
{
  // Prepare and execute query.
  QString
    str = QString("CREATE TABLE `%1` ("
                  "`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                  "`name` TEXT, "
                  "`is_assembly` INTEGER DEFAULT 0, "
                  "`metadata_id` INTEGER, "
                  "FOREIGN KEY(`metadata_id`) REFERENCES `%2`(`id`)"
                  ")").arg( this->tableName() ).arg(asiAsm_Macro_Table_Metadata);
  //
  return this->ExecuteQuery(str);
}
