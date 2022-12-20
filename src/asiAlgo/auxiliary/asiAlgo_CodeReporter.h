//-----------------------------------------------------------------------------
// Created on: 19 December 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

#ifndef asiAlgo_CodeReporter_HeaderFile
#define asiAlgo_CodeReporter_HeaderFile

// asiAlgo includes
#include <asiAlgo_SemanticCode.h>

// Standard includes
#include <unordered_set>

//-----------------------------------------------------------------------------

//! Base class for all classes accumulating diagnostic and semantic
//! codes.
class asiAlgo_CodeReporter
{
/* Status codes */
public:

  //! Sets status codes as a set of integer values.
  //! \param[in] codes status codes to set.
  void SetStatusCodes(const std::unordered_set<int>& codes)
  {
    m_statusCodes = codes;
  }

  //! Checks whether the passed status code is registered or not.
  //! \param[in] code the code to check.
  //! \return true/false.
  bool HasStatusCode(const int code) const
  {
    return ( m_statusCodes.find(code) != m_statusCodes.end() );
  }

  //! \return all collected status codes.
  const std::unordered_set<int>& GetStatusCodes() const
  {
    return m_statusCodes;
  }

  //! Adds the passed code to the stored collection of status codes.
  //! \param[in] code status code to add to the current collection of codes.
  void AddStatusCode(const int code)
  {
    m_statusCodes.insert(code);
  }

  //! Adds the passed codes to the stored collection of status codes.
  //! \param[in] codes status codes to add to the current collection of codes.
  void AddStatusCodes(const std::unordered_set<int>& codes)
  {
    for ( auto it = codes.cbegin(); it != codes.cend(); ++it )
      this->AddStatusCode(*it);
  }

/* Semantic codes */
public:

  //! Sets the collection of semantic codes to store.
  //! \param[in] codes the codes to set.
  void SetSemanticCodes(const asiAlgo_SemanticCodes& codes)
  {
    m_semanticCodes = codes;
  }

  //! Returns all registered semantic codes.
  //! \return const reference to the collection of semantic codes.
  const asiAlgo_SemanticCodes& GetSemanticCodes() const
  {
    return m_semanticCodes;
  }

  //! Adds the passed semantic code to the stored collection. If the passed
  //! `featId` index and `code` number are identical, the corresponding
  //! face IDs will be grouped into a single diagnostic code. Such a technique
  //! allows merging the diagnostic codes not only by their types, but also
  //! by the involved features.
  //!
  //! \param[in] featId the feature of interest.
  //! \param[in] code   the code to add.
  //! \param[in] fid    the sole face ID to associate with the
  //!                   passed semantic code.
  //! \param[in] type   the type of semantic code to report.
  void AddSemanticCode(const int                      featId,
                       const int                      code,
                       const int                      fid,
                       const asiAlgo_SemanticCodeType type)
  {
    m_semanticCodes.Add( asiAlgo_SemanticCode(featId, code, fid, type) );
  }

  //! Adds the passed semantic code to the stored collection. If the passed
  //! `featId` index and `code` number are identical, the corresponding
  //! face IDs will be grouped into a single diagnostic code. Such a technique
  //! allows merging the diagnostic codes not only by their types, but also
  //! by the involved features.
  //!
  //! \param[in] featId the feature of interest.
  //! \param[in] code   the code to add.
  //! \param[in] fids   the face IDs to associate with the
  //!                   passed semantic code.
  //! \param[in] type   the type of semantic code to report.
  void AddSemanticCode(const int                      featId,
                       const int                      code,
                       const asiAlgo_Feature&         fids,
                       const asiAlgo_SemanticCodeType type)
  {
    m_semanticCodes.Add( asiAlgo_SemanticCode(featId, code, fids, type) );
  }

  //! Adds the passed semantic code to the stored collection.
  //! \param[in] code the code to add.
  void AddSemanticCode(const asiAlgo_SemanticCode& code)
  {
    m_semanticCodes.Add(code);
  }

  //! Adds all semantic codes from the passed collection to the
  //! stored one.
  //! \param[in] codes the codes to add.
  void AddSemanticCodes(const asiAlgo_SemanticCodes& codes)
  {
    for ( asiAlgo_SemanticCodes::Iterator cit(codes); cit.More(); cit.Next() )
      this->AddSemanticCode( cit.Value() );
  }

protected:

  //! This ctor is protected to prohibit instantiation
  //! of the base class.
  asiAlgo_CodeReporter() = default;

protected:

  //! Status codes which can be error codes, warning codes or any other
  //! statuses to give more details on the algorithm's execution state.
  //! We use an unordered set as we want to avoid any duplications in
  //! the collection.
  std::unordered_set<int> m_statusCodes;

  //! Semantic codes. Unlike the status codes, these semantic codes are
  //! not simply integers, but containers that associate the diagnostic
  //! codes with the B-rep features.
  asiAlgo_SemanticCodes m_semanticCodes;

};

#endif
