//-----------------------------------------------------------------------------
// Created on: 14 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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

#ifndef asiAlgo_CheckClearance_h
#define asiAlgo_CheckClearance_h

// asiAlgo includes
#include <asiAlgo_BVHFacets.h>
#include <asiAlgo_MeshWithFields.h>
#include <asiAlgo_Optional.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

#if defined USE_MOBIUS
// Mobius includes
#include <mobius/poly_Mesh.h>
#endif

//-----------------------------------------------------------------------------

//! Utility to check feature clearances in a CAD part.
class asiAlgo_CheckClearance : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_CheckClearance, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] shape    B-rep shape of a CAD part to analyze.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_CheckClearance(const TopoDS_Shape&  shape,
                           ActAPI_ProgressEntry progress = nullptr,
                           ActAPI_PlotterEntry  plotter  = nullptr);

#if defined USE_MOBIUS
  //! Ctor.
  //! \param[in] tris     facets of a CAD part to analyze.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_CheckClearance(const mobius::t_ptr<mobius::poly_Mesh>& tris,
                           ActAPI_ProgressEntry                    progress = nullptr,
                           ActAPI_PlotterEntry                     plotter  = nullptr);
#endif

public:

  //! Performs ray casting for clearance analysis.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! \return result of the clearance check which is a faceted representation
  //!         of the CAD part with associated distance field. The scalar
  //!         values representing the distance field are bounded to the
  //!         mesh nodes.
  const asiAlgo_MeshWithFields& GetClearanceField() const
  {
    return m_resField;
  }

  //! \return the computed min clearance value.
  const tl::optional<double>& GetMinClearance() const
  {
    return m_fMinClr;
  }

  //! \return the computed max clearance value.
  const tl::optional<double>& GetMaxClearance() const
  {
    return m_fMaxClr;
  }

public:

  //! Sets the indices of the B-rep elements whose clearance should be analyzed.
  //! If a subdomain is not defined, the entire input model is exposed to the
  //! analysis.
  //!
  //! \param[in] subdomain the elements to cast rays from. This is an unordered
  //!                      set of 1-based indices as returned by `TopExp::MapShapes()`.
  asiAlgo_EXPORT void
    SetSubdomain(const TColStd_PackedMapOfInteger& subdomain);

  //! \return true if a subdomain is defined.
  asiAlgo_EXPORT bool
    HasSubdomain() const;

  //! Checks whether the passed index is a subdomain's element.
  //! \param[in] id the 1-based ID to check.
  //! \return true if the passed element is contained in a subdomain.
  asiAlgo_EXPORT bool
    IsInSubdomain(const int id) const;

protected:

  Handle(asiAlgo_BVHFacets)  m_bvh;       //!< BVH representation of a CAD part.
  TColStd_PackedMapOfInteger m_subdomain; //!< Optional subdomain to narrow down the zone of interest.
  asiAlgo_MeshWithFields     m_resField;  //!< Mesh with a scalar field.
  tl::optional<double>       m_fMinClr;   //!< Min clearance.
  tl::optional<double>       m_fMaxClr;   //!< Max clearance.

};

#endif
