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

// Own include
#include <asiEngine_Clearance.h>

// asiEngine includes
#include <asiEngine_CheckClearanceFunc.h>

// asiAlgo includes
#include <asiAlgo_MeshMerge.h>

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

Handle(asiData_ClearanceNode)
  asiEngine_Clearance::CreateClearance(const Handle(ActAPI_INode)& owner)
{
#if defined USE_MOBIUS
  t_ptr<poly_Mesh> mesh;

  // Resolve the owner.
  if ( owner->IsKind( STANDARD_TYPE(asiData_PartNode) ) )
  {
    // Merge facets.
    asiAlgo_MeshMerge meshMerge( Handle(asiData_PartNode)::DownCast(owner)->GetShape(),
                                 asiAlgo_MeshMerge::Mode_MobiusMesh );
    //
    mesh = meshMerge.GetMobiusMesh();
  }
  else if ( owner->IsKind( STANDARD_TYPE(asiData_IVTopoItemNode) ) )
  {
    // Merge facets.
    asiAlgo_MeshMerge meshMerge( Handle(asiData_IVTopoItemNode)::DownCast(owner)->GetShape(),
                                 asiAlgo_MeshMerge::Mode_MobiusMesh );
    //
    mesh = meshMerge.GetMobiusMesh();
  }
  else if ( owner->IsKind( STANDARD_TYPE(asiData_TriangulationNode) ) )
  {
    mesh = Handle(asiData_TriangulationNode)::DownCast(owner)->GetTriangulation();
  }
  else
  {
    return nullptr; // Unexpected type of owner.
  }

  // Create a new peristent Node.
  Handle(asiData_ClearanceNode)
    node = Handle(asiData_ClearanceNode)::DownCast( asiData_ClearanceNode::Instance() );
  //
  m_model->GetClearancePartition()->AddNode(node);

  // Prepare name.
  TCollection_AsciiString nodeName("Clearance");

  // Initialize.
  node->Init();
  node->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
  node->SetName(nodeName);
  node->SetMesh(mesh);

  // Attach tree function.
  node->ConnectTreeFunction( asiData_ClearanceNode::PID_CheckClearanceFunc,
                             asiEngine_CheckClearanceFunc::GUID(),
                             ActParamStream() << node->Parameter(asiData_ThicknessNode::PID_Mesh),
                             ActParamStream() << node->Parameter(asiData_ThicknessNode::PID_ScalarMin)
                                              << node->Parameter(asiData_ThicknessNode::PID_ScalarMax) );

  // Set as child for the owner Node.
  owner->AddChildNode(node);

  return node;
#else
  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return nullptr;
#endif
}
