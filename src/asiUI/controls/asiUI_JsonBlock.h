//-----------------------------------------------------------------------------
// Created on: 16 June 2022
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

#ifndef asiUI_JsonBlock_h
#define asiUI_JsonBlock_h

// Qt includes
#pragma warning(push, 0)
#include <QRect>
#pragma warning(pop)

#include <map>
#include <list>

//! Structure of Json piece of code. This block is placed between open and closed brace/bracket.
//! It contains number block in the document of open and close.
class asiUI_JsonBlock
{
public:
  //! Constructor.
  asiUI_JsonBlock()
   : m_blockNumber(-1),
     m_isCollapsed(false),
     m_isBrace(false),
     m_blockNumberClose(-1),
     m_positionOpen(-1),
     m_positionClose(-1)
  {}

  //! Constructor.
  asiUI_JsonBlock(const int blockNumber, const bool collapsed)
   : m_blockNumber(blockNumber),
     m_isCollapsed(collapsed),
     m_isBrace(false),
     m_blockNumberClose(-1),
     m_positionOpen(-1),
     m_positionClose(-1),
     m_collapsedInRow("")
  {}

  //! Returns true if the instance is valid.
  bool isValid() const { return m_blockNumber >= 0; }

  bool      m_isCollapsed;      //!< flag whether the block is collapsed or not
  bool      m_isBrace;          //!< if true, it's the {} else it's []

  int       m_blockNumber;      //!< block number
  int       m_positionOpen;     //!< position in block number

  int       m_blockNumberClose; //!< block number end
  int       m_positionClose;    //!< position in block number close

  QString   m_collapsedInRow;   //!< collapsed text if blockNumber equals to blockNumberClose
};

typedef std::map<int, asiUI_JsonBlock> asiUI_JsonBlocks;
typedef std::map<int, QRect>           asiUI_MapIntToRect;

typedef std::list<int>                 asiUI_ListOfInt;
typedef std::map<int, asiUI_ListOfInt> asiUI_MapOfListOfInt;

typedef std::map<int, int>             asiUI_MapIntToInt;
typedef std::map<int, asiUI_MapIntToInt>  asiUI_MapOfMapToInt;

#endif
