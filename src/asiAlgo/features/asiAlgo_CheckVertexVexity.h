//-----------------------------------------------------------------------------
// Created on: 04 February 2022
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

#ifndef asiAlgo_CheckVertexVexity_h
#define asiAlgo_CheckVertexVexity_h

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_FeatureAngleType.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! Utility to analyze vertex vexity.
class asiAlgo_CheckVertexVexity : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_CheckVertexVexity, ActAPI_IAlgorithm)

public:

  //! Auxiliary edge info.
  struct t_edgeInfo
  {
    TopoDS_Edge             edge;
    bool                    isStraight;
    bool                    isCircular;
    asiAlgo_BorderTrihedron axes;

    //! Default ctor.
    t_edgeInfo()
    : isStraight (false),
      isCircular (false)
    {}

    //! Conversion ctor.
    t_edgeInfo(const TopoDS_Edge& E)
    : edge       (E),
      isStraight (false),
      isCircular (false)
    {}
  };

  typedef NCollection_IndexedDataMap<TopoDS_Edge, t_edgeInfo, TopTools_ShapeMapHasher> t_edgeInfoMap;

  typedef NCollection_DataMap<TopoDS_Vertex, asiAlgo_FeatureAngleType> t_vexityMap;

public:

  //! Constructor.
  //! \param[in] aag      the AAG instance.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_CheckVertexVexity(const Handle(asiAlgo_AAG)& aag,
                              ActAPI_ProgressEntry       progress = nullptr,
                              ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Collects all edges where vertex vexity is to be checked.
  asiAlgo_EXPORT void
    CollectEdges(const int      fid,
                 t_edgeInfoMap& edges) const;

  //! Checks convexity of a vertex common for the passed two edges.
  asiAlgo_EXPORT asiAlgo_FeatureAngleType
    CheckConvexity(const TopoDS_Edge& E1,
                   const TopoDS_Edge& E2,
                   const int          fid,
                   TopoDS_Vertex&     V) const;

  //! Checks all contours in the given face.
  asiAlgo_EXPORT void
    CheckContours(const int    fid,
                  t_vexityMap& vexity) const;

protected:

  Handle(asiAlgo_AAG) m_aag; //!< AAG instance.

};

#endif
