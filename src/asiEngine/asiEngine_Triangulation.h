//-----------------------------------------------------------------------------
// Created on: 14 August 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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
//    * Neither the name of Sergey Slyadnev nor the
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

#ifndef asiEngine_Triangulation_h
#define asiEngine_Triangulation_h

// asiEngine includes
#include <asiEngine_Model.h>

// asiData includes
#include <asiData_TriangulationNode.h>

// asiVisu includes
#include <asiVisu_PrsManager.h>

//! Data Model API for mesh.
class asiEngine_Triangulation
{
public:

  asiEngine_Triangulation(const Handle(asiEngine_Model)&             model,
                          const vtkSmartPointer<asiVisu_PrsManager>& prsMgr,
                          ActAPI_ProgressEntry                       progress = NULL,
                          ActAPI_PlotterEntry                        plotter  = NULL)
  : m_model    (model),
    m_prsMgr   (prsMgr),
    m_progress (progress),
    m_plotter  (plotter)
  {}

public:

  //! Creates new Triangulation Node in the Data Model.
  //! \return newly created Triangulation Node.
  asiEngine_EXPORT Handle(asiData_TriangulationNode)
    CreateTriangulation();

  //! Constructs BVH structure for the facets stored in the triangulation Node.
  asiEngine_EXPORT void
    BuildBVH();

  //! Returns highlighted facets.
  //! \param facetIndices [out] indices of highlighted facets.
  asiEngine_EXPORT void
    GetHighlightedFacets(TColStd_PackedMapOfInteger& facetIndices);

protected:

  Handle(asiEngine_Model)             m_model;    //!< Data Model instance.
  vtkSmartPointer<asiVisu_PrsManager> m_prsMgr;   //!< Presentation manager.
  ActAPI_ProgressEntry                m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry                 m_plotter;  //!< Plotter.

};

#endif