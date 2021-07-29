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
#include <asiEngine_Triangulation.h>

// asiVisu includes
#include <asiVisu_TriangulationNodeInfo.h>
#include <asiVisu_TriangulationPrs.h>

// asiAlgo includes
#include <asiAlgo_CheckDeviations.h>
#include <asiAlgo_Timer.h>

#if defined USE_MOBIUS
  #include <mobius/cascade.h>

  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

Handle(asiData_TriangulationNode) asiEngine_Triangulation::CreateTriangulation()
{
  // Add Triangulation Node to Partition
  Handle(asiData_TriangulationNode)
    triangulation_n = Handle(asiData_TriangulationNode)::DownCast( asiData_TriangulationNode::Instance() );
  //
  m_model->GetTriangulationPartition()->AddNode(triangulation_n);

  // Initialize
  triangulation_n->Init();
  triangulation_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
  triangulation_n->SetName("Triangulation");

  // Return the just created Node
  return triangulation_n;
}

#if defined USE_MOBIUS

//-----------------------------------------------------------------------------

t_ptr<poly_Mesh> asiEngine_Triangulation::GetTriangulation()
{
  return m_model->GetTriangulationNode()->GetTriangulation();
}

#endif

//-----------------------------------------------------------------------------

Handle(asiAlgo_BVHFacets) asiEngine_Triangulation::BuildBVH(const bool store)
{
#if defined USE_MOBIUS
  // Get Triangulation Node
  Handle(asiData_TriangulationNode) tris_n = m_model->GetTriangulationNode();

  // Build BVH for facets
  Handle(asiAlgo_BVHFacets)
    bvh = new asiAlgo_BVHFacets(tris_n->GetTriangulation(),
                                asiAlgo_BVHFacets::Builder_Binned,
                                true,
                                m_progress,
                                m_plotter);

  if ( store) // Store in OCAF
    tris_n->SetBVH(bvh);

  return bvh;
#else
  (void) store;

  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return nullptr;
#endif
}

//-----------------------------------------------------------------------------

bool
  asiEngine_Triangulation::CheckDeviation(const Handle(asiData_IVPointSetNode)& pcNode)
{
  Handle(asiData_DeviationNode) devNode;
  return this->CheckDeviation(pcNode, devNode);
}

//-----------------------------------------------------------------------------

bool
  asiEngine_Triangulation::CheckDeviation(const Handle(asiData_IVPointSetNode)& pcNode,
                                          Handle(asiData_DeviationNode)&        devNode)
{
#if defined USE_MOBIUS
  // Get Triangulation Node.
  Handle(asiData_TriangulationNode) trisNode = m_model->GetTriangulationNode();

  // Check deviations.
  asiAlgo_CheckDeviations checkDeviations( pcNode->GetPoints(),
                                           m_progress,
                                           m_plotter );
  //
  if ( !checkDeviations.Perform( cascade::GetOpenCascadeMesh( trisNode->GetTriangulation() ) ) )
    return false;

  // Create Deviation Node.
  Handle(ActAPI_INode) devNodeBase = asiData_DeviationNode::Instance();
  m_model->GetDeviationPartition()->AddNode(devNodeBase);

  // Initialize.
  devNode = Handle(asiData_DeviationNode)::DownCast(devNodeBase);
  //
  devNode->Init();
  devNode->SetName("Deviation");
  devNode->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);

  // Store deviations.
  devNode->SetMeshWithScalars( checkDeviations.GetResult() );

  // Add Deviation Node as a child of the Triangulation Node.
  trisNode->AddChildNode(devNode);

  return true;
#else
  (void) pcNode;
  (void) devNode;

  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return false;
#endif
}

//-----------------------------------------------------------------------------

void asiEngine_Triangulation::HighlightFacets(const TColStd_PackedMapOfInteger& facetIndices)
{
  // Get Triangulatiom Node.
  Handle(asiData_TriangulationNode) N = m_model->GetTriangulationNode();

  // Get Presentation for the Triangulation Node.
  Handle(asiVisu_TriangulationPrs)
    prs = Handle(asiVisu_TriangulationPrs)::DownCast( m_prsMgr->GetPresentation(N) );
  //
  if ( prs.IsNull() )
    return;

  // Make sure to restore the previous selection mode.
  const int prevMode = m_prsMgr->GetCurrentSelection().GetSelectionModes();
  {
    m_prsMgr->Highlight(N, prs->MainActor(), facetIndices, SelectionMode_Face);
  }
  m_prsMgr->ChangeCurrentSelection().SetSelectionModes(prevMode);
}

//-----------------------------------------------------------------------------

void asiEngine_Triangulation::GetHighlightedFacets(TColStd_PackedMapOfInteger& facetIndices)
{
  // Get actual selection
  const asiVisu_ActualSelection&          sel     = m_prsMgr->GetCurrentSelection();
  const Handle(asiVisu_CellPickerResult)& pickRes = sel.GetCellPickerResult(SelectionNature_Persistent);
  //
  asiVisu_TriangulationNodeInfo*
    nodeInfo = asiVisu_TriangulationNodeInfo::Retrieve( pickRes->GetPickedActor() );
  //
  if ( !nodeInfo )
    return;

  facetIndices = pickRes->GetPickedElementIds();
}
