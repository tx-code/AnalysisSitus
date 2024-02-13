//-----------------------------------------------------------------------------
// Created on: 09 January 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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

#ifndef asiAlgo_FacetQuality_h
#define asiAlgo_FacetQuality_h

// Standard includes
#include <unordered_map>

#define asiAlgo_LINDEFL_MIN 1.0e-6
#define asiAlgo_ANGDEFL_MIN 0.1

#define FQ_Name_VeryRough "very-rough"
#define FQ_Name_Rough     "rough"
#define FQ_Name_Normal    "normal"
#define FQ_Name_Fine      "fine"
#define FQ_Name_VeryFine  "very-fine"

//! \ingroup ASI_MODELING
//!
//! This enumeration defines levels of detail to use in a faceter. Each item in
//! this enumeration corresponds to a pair of angular and linear deflection
//! values. Those values are chosen algorithmically, depending on the extent
//! of a shape and the selected detail level.
enum class asiAlgo_FacetQuality
{
  UNDEFINED = -1,
  //
  VeryRough = 0,
  Rough,
  Normal,
  Fine,
  VeryFine
};

//! \ingroup ASI_MODELING
//!
//! Quality names.
static std::unordered_map<std::string, asiAlgo_FacetQuality>
  asiAlgo_FacetQualityNames = { {FQ_Name_VeryRough, asiAlgo_FacetQuality::VeryRough},
                                {FQ_Name_Rough,     asiAlgo_FacetQuality::Rough},
                                {FQ_Name_Normal,    asiAlgo_FacetQuality::Normal},
                                {FQ_Name_Fine,      asiAlgo_FacetQuality::Fine},
                                {FQ_Name_VeryFine,  asiAlgo_FacetQuality::VeryFine} };

//! \ingroup ASI_MODELING
//!
//! Returns facet quality by its predefined string identifier.
//! \param[in] name the name to match with a facet quality level.
//! \return facet quality enum.
inline asiAlgo_FacetQuality asiAlgo_FacetQualityFromString(const char* name)
{
  auto fq = asiAlgo_FacetQualityNames.find(name);
  //
  if ( fq == asiAlgo_FacetQualityNames.cend() )
    return asiAlgo_FacetQuality::UNDEFINED;

  return (*fq).second;
}

//! \ingroup ASI_MODELING
//!
//! This function returns a pair of linear and angular deflection values for the
//! requested quality level. The linear deflection is derived from the passed
//! `minLinDefl` value by scaling it by a prescribed coefficient. The value of
//! the angular deflection is fixed for each quality level.
//!
//! \param[in]  facetQuality the requested quality level.
//! \param[in]  minLinDefl   the minimal allowed linear deflection in model units.
//! \param[out] linDefl      the computed linear deflection in model units.
//! \param[out] angDeflDeg   the computed angular deflection in degrees.
inline void asiAlgo_SelectFaceterOptions(const asiAlgo_FacetQuality facetQuality,
                                         const double               minLinDefl,
                                         double&                    linDefl,
                                         double&                    angDeflDeg)
{
  switch ( facetQuality )
  {
    case asiAlgo_FacetQuality::VeryRough:
    {
      linDefl    = minLinDefl * 10;
      angDeflDeg = 10;
    }
    break;

    case asiAlgo_FacetQuality::Rough:
    {
      linDefl    = minLinDefl * 2.5;
      angDeflDeg = 5.0;
    }
    break;

    case asiAlgo_FacetQuality::Normal:
    {
      linDefl    = minLinDefl;
      angDeflDeg = 1.0;
    }
    break;

    case asiAlgo_FacetQuality::Fine:
    {
      linDefl    = minLinDefl * 0.1;
      angDeflDeg = 1.0;
    }
    break;

    case asiAlgo_FacetQuality::VeryFine:
    {
      linDefl    = minLinDefl * 0.01;
      angDeflDeg = 0.1;
    }
    break;
  }
}

#endif
