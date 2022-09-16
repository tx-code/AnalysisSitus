//-----------------------------------------------------------------------------
// Created on: 11 September 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Natalia Ermolaeva
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

#ifndef asiUI_JsonSearchThread_h
#define asiUI_JsonSearchThread_h

// Qt includes
#pragma warning(push, 0)
#include <QMutex>
#include <QTextCursor>
#include <QThread>
#pragma warning(pop)

class QTextDocument;

//! Prepares structure of found text key in json document.
class asiUI_JsonSearchThread : public QThread
{
  Q_OBJECT

public:
  asiUI_JsonSearchThread() : m_document(nullptr) {}
  ~asiUI_JsonSearchThread() {}

  //! Sets the value to search
  //! \param[in] value the text key
  void setSearchValue(const QString& value) { m_search = value; }

  //! Returns value of search
  QString searchValue() const { return m_search; }

  //! Sets the document for search
  //! \param[in] doc the document
  void setDocument(QTextDocument* doc) { m_document = doc; }

  //!< Returns found indices.
  std::list<QTextCursor> matchedIndices() const { return m_matchedIndices; }

  virtual void run();

private:
  QTextDocument*         m_document;       //! document to search
  QMutex                 m_mutex;          //! locking thread
  QString                m_search;         //!< value to search
  std::list<QTextCursor> m_matchedIndices; //!< found indices
};

#endif
