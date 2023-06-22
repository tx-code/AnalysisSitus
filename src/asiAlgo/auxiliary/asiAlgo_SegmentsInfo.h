//-----------------------------------------------------------------------------
// Created on: 24 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Kiselev
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

#ifndef asiAlgo_SegmentsInfo_h
#define asiAlgo_SegmentsInfo_h

// asiAlgo include
#include <asiAlgo.h>
#include <asiAlgo_Optional.h>

// STL includes
#include <string>
#include <vector>

//-----------------------------------------------------------------------------

class asiAlgo_SegmentsInfo;
typedef std::vector< asiAlgo_SegmentsInfo > asiAlgo_SegmentsInfoVec;

//-----------------------------------------------------------------------------

//! Segments information.
class asiAlgo_SegmentsInfo
{
  public:

    double      id;
    std::string type;
    double      cuttingLength;

    tl::optional< int >    nextSegment;
    tl::optional< double > angleToNextSegment;

    tl::optional< double > radius;
    tl::optional< double > angle;

  public:

    //! Constructor.
    asiAlgo_EXPORT
      asiAlgo_SegmentsInfo();

    //! Constructor.
    asiAlgo_EXPORT
      asiAlgo_SegmentsInfo(const double       _id,
                           const std::string& _type,
                           const double       _cuttingLength);

  public:

    //! Checks if the passed segments info data structure equals this one.
    //! \param[in] info        the other data structure.
    //! \param[in] linToler    the linear tolerance.
    //! \param[in] angTolerDeg the angular tolerance in degrees.
    //! \return true in the case of equality, false -- otherwise.
    asiAlgo_EXPORT bool
      IsEqual(const asiAlgo_SegmentsInfo& info,
              const double                linToler,
              const double                angTolerDeg) const;

  public:

    //! Checks if the passed vector of segments info data structure equals this another one.
    //! \param[in] v1          the first vector of data structures.
    //! \param[in] v2          the second vector of data structures.
    //! \param[in] linToler    the linear tolerance.
    //! \param[in] angTolerDeg the angular tolerance in degrees.
    //! \return true in the case of equality, false -- otherwise.
    asiAlgo_EXPORT static bool
      AreEqual(const asiAlgo_SegmentsInfoVec& v1,
               const asiAlgo_SegmentsInfoVec& v2,
               const double                   linToler,
               const double                   angTolerDeg);

    //! Constructs segments info data structure from a JSON object.
    //! \param[in]  pJsonGenericObj the JSON object to populate the data structure from.
    //! \param[out] info            the outcome data structure.
    asiAlgo_EXPORT static void
      FromJSON(void*                 pJsonGenericObj,
               asiAlgo_SegmentsInfo& info);

    //! Constructs vector of segments info data structure from a JSON object.
    //! \param[in]  pJsonGenericObj the JSON object to populate the vector of data structure from.
    //! \param[out] infoVec         the outcome vector of data structures.
    asiAlgo_EXPORT static void
      FromJSON(void*                    pJsonGenericObj,
               asiAlgo_SegmentsInfoVec& infoVec);

    //! Converts the passed segments info data structure to JSON (the passed `out` stream).
    //! \param[in]     info     the data structur to serialize.
    //! \param[in]     indent   the pretty indentation shift.
    //! \param[in,out] out      the output JSON string stream.
    //! \param[in]     pureJSON the flag to convert info to pure JSON (without escaping symbols).
    asiAlgo_EXPORT static void
      ToJSON(const asiAlgo_SegmentsInfo& info,
             const int                   indent,
             std::ostream&               out,
             const bool                  pureJSON = false);
};

#endif
