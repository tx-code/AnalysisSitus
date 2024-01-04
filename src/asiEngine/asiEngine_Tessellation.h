//-----------------------------------------------------------------------------
// Created on: 05 October 2018
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

#ifndef asiEngine_Tessellation_h
#define asiEngine_Tessellation_h

// asiEngine includes
#include <asiEngine_Base.h>

// asiData includes
#include <asiData_MeshNormsNode.h>
#include <asiData_TessNode.h>

#if !defined BUILD_ALGO_ONLY
  // asiVisu includes
  #include <asiVisu_PrsManager.h>
#endif

//! Data Model API for tessellations.
class asiEngine_Tessellation : public asiEngine_Base
{
public:

  //! Ctor.
  //! \param[in] model    Data Model instance.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiEngine_Tessellation(const Handle(asiEngine_Model)& model,
                         ActAPI_ProgressEntry           progress = nullptr,
                         ActAPI_PlotterEntry            plotter  = nullptr)
  //
  : asiEngine_Base (model, progress, plotter)
#if !defined BUILD_ALGO_ONLY
  , m_prsMgr       (nullptr)
#endif
  {}

#if !defined BUILD_ALGO_ONLY
  //! Ctor.
  //! \param[in] model    Data Model instance.
  //! \param[in] prsMgr   Presentation Manager for visual commands.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  asiEngine_Tessellation(const Handle(asiEngine_Model)&             model,
                         const vtkSmartPointer<asiVisu_PrsManager>& prsMgr,
                         ActAPI_ProgressEntry                       progress = nullptr,
                         ActAPI_PlotterEntry                        plotter  = nullptr)
  //
  : asiEngine_Base (model, progress, plotter),
    m_prsMgr       (prsMgr)
  {}
#endif

public:

  //! Creates new Tessellation Node in the Data Model.
  //! \return newly created Tessellation Node.
  asiEngine_EXPORT Handle(asiData_TessNode)
    CreateTessellation();

  //! Creates new Tessellation Norms Node.
  //! \param[in] parent      parent Node.
  //! \param[in] name        name to set.
  //! \param[in] isElemental indicates whether the normal field to be stored
  //!                        in the Node is elemental (true) or nodal (false).
  //! \return newly created Node.
  asiEngine_EXPORT Handle(asiData_MeshNormsNode)
    CreateNorms(const Handle(ActAPI_INode)&       parent,
                const TCollection_ExtendedString& name,
                const bool                        isElemental);

  //! Computes normal field for the given Tessellation Node. This method
  //! creates a child Node under the passed one to store the computed
  //! normal vectors.
  //! \param[in] tessNode    Tessellation Node to compute the normal field for.
  //! \param[in] doElemNorms indicates whether to compute norms by
  //!                        elements. The alternative is to compute
  //!                        norms by nodes (averaging the elements' nodes).
  //! \return child Data Node representing the computed normal field.
  asiEngine_EXPORT Handle(asiData_MeshNormsNode)
    ComputeNorms(const Handle(asiData_TessNode)& tessNode,
                 const bool                      doElemNorms);

#if !defined BUILD_ALGO_ONLY
protected:

  vtkSmartPointer<asiVisu_PrsManager> m_prsMgr; //!< Presentation manager.
#endif

};

#endif
