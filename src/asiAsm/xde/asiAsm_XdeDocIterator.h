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

#ifndef asiAsm_XdeDocIterator_h
#define asiAsm_XdeDocIterator_h

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// Standard includes
#include <stack>

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! \ingroup ASIASM
//!
//! Depth-first iterator for assembly tree. Depth-first manner of iteration
//! answers the user needs to observe the assembly contents.
class DocIterator
{
public:

  //! Initializes iterator with Assembly Document.
  //! \param[in] asmDoc Assembly Document to iterate.
  //! \param[in] level  max level of hierarchy to reach (INT_MAX is for no limit).
  asiAsm_EXPORT
    DocIterator(const Handle(Doc)& asmDoc,
                const int          level = INT_MAX);

  //! Initializes iterator with Data Model and a chosen item to iterate.
  //! \param[in] root   assembly item to start iterating from.
  //! \param[in] asmDoc Assembly Document to iterate.
  //! \param[in] level  max level of hierarchy to reach (INT_MAX is for no limit).
  asiAsm_EXPORT
    DocIterator(const AssemblyItemId& root,
                const Handle(Doc)&    asmDoc,
                const int             level = INT_MAX);

public:

  //! \return true if there is still something to iterate, false -- otherwise.
  asiAsm_EXPORT bool
    More() const;

  //! Moves depth-first iterator to the next position.
  asiAsm_EXPORT void
    Next();

  //! \return current item.
  asiAsm_EXPORT AssemblyItemId
    Current() const;

protected:

  //! Creates an item and fills it with data.
  //! \param[in] L OCAF label for item.
  //! \return just created item.
  asiAsm_EXPORT AssemblyItemId
    createItem(const TDF_Label& L) const;

protected:

  Handle(Doc)                m_asmDoc;    //!< Assembly Document to iterate.
  std::stack<AssemblyItemId> m_fringe;    //!< Items pending for iteration.
  int                        m_iMaxLevel; //!< Limit on max depth of iteration.

};

} // xde
} // asiAsm

#endif
