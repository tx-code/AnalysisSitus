//-----------------------------------------------------------------------------
// Created on: 25 May 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023, Julia Slyadneva
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

#pragma once

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Constructs tight bounding cylinder for a part.
class asiAlgo_BuildOptBoundingCyl : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_BuildOptBoundingCyl, ActAPI_IAlgorithm)

public:

  //! Auxiliary structure storing properties of calculated bounding volume
  struct t_optBnd
  {
    TopoDS_Shape shape;
    double       volume = 0.;
    double       radius = 0.;
    double       height = 0.;
    gp_Ax3       placement;   //!< Placement of the local axes.
    gp_Trsf      trsf;        //!< Transformation.

    t_optBnd& min(t_optBnd& other)
    {
      if (this->volume <= other.volume)
        return *this;

      return other;
    }

    void clear()
    {
      shape.Nullify();
      volume = 0.;
      radius = 0.;
      height = 0.;
      placement = gp_Ax3();
      trsf      = gp_Trsf();
    }
  };

public:

  //! Ctor.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_BuildOptBoundingCyl(ActAPI_ProgressEntry progress,
                                ActAPI_PlotterEntry  plotter);

  //! Runs calculation.
  //! \param[in] aag              the attributed adjacency graph.
  //! \param[in] forceTriangulate the Boolean flag indicating whether
  //!                             triangulation on a part is requested
  //!                             to be forcibly regenerated even if it
  //!                             already exists.
  asiAlgo_EXPORT bool
    Perform(const Handle(asiAlgo_AAG)& aag,
            const bool                 forceTriangulate = false);

public:

  //! \return the oriented minimal bounding cylinder.
  const t_optBnd& GetResult() const
  {
    return m_cylinder;
  }

private:

  t_optBnd m_cylinder; //!< The constructed cylinder.
};
