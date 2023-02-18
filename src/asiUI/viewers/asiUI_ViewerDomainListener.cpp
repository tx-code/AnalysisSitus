//-----------------------------------------------------------------------------
// Created on: 07 November 2016 (99 years of October Revolution)
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

// Own include
#include <asiUI_ViewerDomainListener.h>

// asiUI includes
#include <asiUI_IV.h>

//-----------------------------------------------------------------------------

//! Constructor accepting all necessary facilities.
//! \param[in] wViewerPart   part viewer.
//! \param[in] wViewerDomain domain viewer.
//! \param[in] wViewerHost   host geometry viewer.
//! \param[in] model         Data Model instance.
//! \param[in] progress      progress notifier.
//! \param[in] plotter       imperative plotter.
asiUI_ViewerDomainListener::asiUI_ViewerDomainListener(asiUI_ViewerPart*              wViewerPart,
                                                       asiUI_ViewerDomain*            wViewerDomain,
                                                       asiUI_ViewerHost*              wViewerHost,
                                                       const Handle(asiEngine_Model)& model,
                                                       ActAPI_ProgressEntry           progress,
                                                       ActAPI_PlotterEntry            plotter)
: asiUI_Viewer3dListener (wViewerDomain, model, progress, plotter),
  m_wViewerPart          (wViewerPart),
  m_wViewerHost          (wViewerHost),
  m_pScaleU              (nullptr),
  m_pScaleV              (nullptr),
  m_pJoinEdges           (nullptr)
{}

//-----------------------------------------------------------------------------

//! Destructor.
asiUI_ViewerDomainListener::~asiUI_ViewerDomainListener()
{}

//-----------------------------------------------------------------------------

//! Connects this listener to the target widget.
void asiUI_ViewerDomainListener::Connect()
{
  asiUI_Viewer3dListener::Connect(); // Connect basic reactions.
}

//-----------------------------------------------------------------------------

void asiUI_ViewerDomainListener::populateMenu(QMenu& menu)
{
  asiUI_ViewerDomain*
    pDomainViewer = ::qobject_cast<asiUI_ViewerDomain*>(m_pViewer);

  m_pScaleU = menu.addAction("Scale up in U");
  m_pScaleV = menu.addAction("Scale up in V");

  std::map<int, TopoDS_Edge> edges;
  pDomainViewer->GetSelectedEdges(edges);

  if ( edges.size() == 2 )
  {
    menu.addSeparator();
    m_pJoinEdges = menu.addAction("Join p-curves (&J)");
  }
}

//-----------------------------------------------------------------------------

void asiUI_ViewerDomainListener::executeAction(QAction* action)
{
  asiUI_ViewerDomain*
    pDomainViewer = ::qobject_cast<asiUI_ViewerDomain*>(m_pViewer);

  /* ============
   *  Scale up U.
   * ============ */

  if ( action == m_pScaleU )
  {
    pDomainViewer->onScaleU();
  }

  /* ============
   *  Scale up V.
   * ============ */

  else if ( action == m_pScaleV )
  {
    pDomainViewer->onScaleV();
  }

  /* ==========================
   *  Join (concatenate) edges.
   * ========================== */

  else if ( action == m_pJoinEdges )
  {
    pDomainViewer->onJoinEdges();
  }
}
