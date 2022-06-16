//-----------------------------------------------------------------------------
// Created on: 16 June 2022
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

// Own include
#include <asiAlgo_SuppressSoft.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_CheckInsertion.h>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAlgo_SuppressSoft::asiAlgo_SuppressSoft(const TopoDS_Shape&        masterCAD,
                                           const Handle(asiAlgo_AAG)& aag,
                                           ActAPI_ProgressEntry       progress,
                                           ActAPI_PlotterEntry        plotter)
//
: ActAPI_IAlgorithm(progress, plotter)
{
  m_input = masterCAD;

  if ( aag.IsNull() )
  {
#if defined COUT_DEBUG
    TIMER_NEW
    TIMER_GO
#endif

    m_aag = new asiAlgo_AAG(masterCAD);

#if defined COUT_DEBUG
    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Construct AAG")
#endif
  }
  else
    m_aag = aag;

  // Prepare topological killer.
  m_tool = new asiAlgo_SuppressFaces(m_aag);
}

//-----------------------------------------------------------------------------

bool asiAlgo_SuppressSoft::Perform(const asiAlgo_Feature& faceIndices)
{
  /* =======================================================
   *  Check if suppression is requested for soft insertions
   * ======================================================= */

  // Find bridge faces (those joining master faces with feature).
  NCollection_DataMap<int, asiAlgo_Feature> bridgeFaces;
  for ( asiAlgo_Feature::Iterator fit(faceIndices); fit.More(); fit.Next() )
  {
    const int face_id = fit.Key();
    //
    if ( !m_aag->HasFace(face_id) )
    {
      this->GetProgress().SendLogMessage( LogWarn(Normal) << "algoFeat_SuppressSoft.NoSuchFace"
                                                          << face_id );
      //
      this->AddStatusFlag(StatusCode_WarnMissingFaces);
      continue;
    }

    // We are looking for those faces having at least one neighbor not
    // scheduled for suppression.
    const asiAlgo_Feature& neighbors = m_aag->GetNeighbors(face_id);
    //
    for ( asiAlgo_Feature::Iterator nit(neighbors); nit.More(); nit.Next() )
    {
      const int neighbor_id = nit.Key();
      //
      if ( !faceIndices.Contains(neighbor_id) )
      {
        asiAlgo_Feature* mapPtr = bridgeFaces.ChangeSeek(face_id);
        if ( mapPtr == NULL )
        {
          this->GetPlotter().DRAW_SHAPE( m_aag->GetFace(face_id), Color_Violet, "Feature bridge" );
          //
          mapPtr = bridgeFaces.Bound( face_id, asiAlgo_Feature() );
        }

        // Add master neighbor.
        (*mapPtr).Add(neighbor_id);
      }
    }
  }

  // Now check insertions, so that to prohibit suppression of hard features.
  asiAlgo_CheckInsertion insertion( this->GetProgress(), this->GetPlotter() );
  asiAlgo_Feature hardFaces;
  //
  for ( NCollection_DataMap<int, asiAlgo_Feature>::Iterator fit(bridgeFaces); fit.More(); fit.Next() )
  {
    const int              feature_face_id = fit.Key();
    const TopoDS_Face&     feature_face    = m_aag->GetFace(feature_face_id);
    const asiAlgo_Feature& master_faces    = fit.Value();

    for ( asiAlgo_Feature::Iterator mit(master_faces); mit.More(); mit.Next() )
    {
      const int          master_face_id = mit.Key();
      const TopoDS_Face& master_face    = m_aag->GetFace(master_face_id);

      if ( insertion.Check(feature_face, master_face) != InsertionType_Soft )
      {
        this->GetPlotter().DRAW_SHAPE(feature_face, Color_Red, "Hard feature");
        hardFaces.Add(feature_face_id);
      }
    }
  }

  asiAlgo_Feature      faces2Suppress;
  asiAlgo_Feature      unsuppressedFaceIds;
  TopTools_ListOfShape unsuppressedFaces;
  //
  if ( hardFaces.Extent() )
  {
    asiAlgo_Feature hardFacesCompletion, visited;
    for ( asiAlgo_Feature::Iterator fit(hardFaces); fit.More(); fit.Next() )
    {
      const int hard_face_id = fit.Key();

      // Take the whole feature the current hard face belongs to.
      visited.Add(hard_face_id);
      //
      this->collectLocalFeature(hard_face_id, faceIndices, hardFacesCompletion, visited);
    }

    this->GetProgress().SendLogMessage( LogWarn(Normal) << "1% non-isolated face(s) detected."
                                                        << hardFacesCompletion.Extent() );
    //
    this->AddStatusFlag(StatusCode_WarnHardFacesFound);

    for ( asiAlgo_Feature::Iterator fit(faceIndices); fit.More(); fit.Next() )
    {
      const int feature_face_id = fit.Key();
      //
      if ( !hardFacesCompletion.Contains(feature_face_id) )
        faces2Suppress.Add(feature_face_id);
      else
        this->GetPlotter().DRAW_SHAPE( m_aag->GetFace(feature_face_id), Color_Red, "Hard feature completion" );
    }

    // Store faces which cannot be suppressed.
    unsuppressedFaceIds.Union(hardFaces, hardFacesCompletion);
  }
  else
    faces2Suppress = faceIndices;
  //
  m_nbOfOldUnsuppressed = unsuppressedFaceIds.Extent();
  //
  asiAlgo_Feature::Iterator itUIF(unsuppressedFaceIds);
  for ( ; itUIF.More(); itUIF.Next() )
  {
    unsuppressedFaces.Append(m_aag->GetFace(itUIF.Key()));
  }

  /* =====================
   *  Perform suppression
   * ===================== */

  // Check if there is anything to suppress after detection of hard faces.
  if ( faces2Suppress.IsEmpty() )
  {
    this->GetProgress().SendLogMessage( LogWarn(Normal) << "algoFeat_SuppressSoft.NoIsolatedFaces" );
    //
    this->AddStatusFlag(StatusCode_WarnNoFaces2Suppress);

    m_output       = m_input;
    m_unsuppressed = unsuppressedFaceIds;
    return true; // Return true as the algorithm succeeded though did nothing.
  }
  //
  // Remove faces with a basic tool for topological reduction.
  if ( !m_tool->Perform(faces2Suppress, false) )
  {
    this->AddStatusFlag(StatusCode_ErrFailed);
    m_unsuppressed = unsuppressedFaceIds;
    return false;
  }
  //
  const Handle(BRepTools_History) history = m_tool->GetHistory();
  const TopoDS_Shape& resShape = m_tool->GetResult();
  //
  TopTools_IndexedMapOfShape newFaces;
  if ( !resShape.IsNull() )
    TopExp::MapShapes(resShape, TopAbs_FACE, newFaces);
  //
  // Since the suppression was successful, the indices of the unsuppressed faces
  // need to be updated.
  TopTools_ListOfShape::Iterator itUF(unsuppressedFaces);
  for ( ; itUF.More(); itUF.Next() )
  {
    if ( !history.IsNull() && history->IsRemoved(itUF.Value()) )
      continue;
    //
    TopTools_ListOfShape modified;
    TopTools_ListOfShape generated;
    //
    if ( !history.IsNull() )
    {
      modified  = history->Modified(itUF.Value());
      generated = history->Generated(itUF.Value());
    }
    //
    if ( !modified.IsEmpty() || !generated.IsEmpty() )
    {
      TopTools_ListOfShape::Iterator itMod(modified);
      for ( ; itMod.More(); itMod.Next() )
      {
        m_unsuppressed.Add(newFaces.FindIndex(itMod.Value()));
      }
      //
      TopTools_ListOfShape::Iterator itGen(generated);
      for ( ; itGen.More(); itGen.Next() )
      {
        m_unsuppressed.Add(newFaces.FindIndex(itGen.Value()));
      }
    }
    else
    {
      m_unsuppressed.Add(newFaces.FindIndex(itUF.Value()));
    }
  }
  //
  m_output = resShape;
  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_SuppressSoft::collectLocalFeature(const int              seed_face_id,
                                               const asiAlgo_Feature& globalFeatureFaces,
                                               asiAlgo_Feature&       localFeatureFaces,
                                               asiAlgo_Feature&       visited) const
{
  localFeatureFaces.Add(seed_face_id);

  // Get direct neighbors
  const asiAlgo_Feature& neighbors = m_aag->GetNeighbors(seed_face_id);
  //
  for ( asiAlgo_Feature::Iterator nit(neighbors); nit.More(); nit.Next() )
  {
    const int neighbor_id = nit.Key();
    if ( visited.Contains(neighbor_id) )
      continue;

    visited.Add(neighbor_id);
    //
    if ( globalFeatureFaces.Contains(neighbor_id) )
      this->collectLocalFeature(neighbor_id, globalFeatureFaces, localFeatureFaces, visited);
  }
}
