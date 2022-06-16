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
#include <asiAlgo_SuppressFeatures.h>

// asiAlgo includes
#include <asiAlgo_SuppressHard.h>
#include <asiAlgo_SuppressSoft.h>

// OCCT includes
#include <BRepTools.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>

#undef FILE_DEBUG
#if defined FILE_DEBUG
  #pragma message("===== warning: FILE_DEBUG is enabled")
#endif

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAlgo_SuppressFeatures::asiAlgo_SuppressFeatures(ActAPI_ProgressEntry notifier,
                                                   ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm(notifier, plotter)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_SuppressFeatures::operator()(const TopoDS_Shape&        shape,
                                          const asiAlgo_Feature&     feature,
                                          const bool                 tryHard,
                                          TopoDS_Shape&              resShape,
                                          asiAlgo_Feature&           unsuppressed,
                                          Handle(BRepTools_History)& history)
{
  TColStd_PackedMapOfInteger stillThere;

  // Try to suppress soft faces.
  asiAlgo_SuppressSoft softEraser(shape, nullptr, m_progress, m_plotter);
  //
  const bool isSoftOk     = softEraser.Perform(feature);
  bool       isSuppressed = false;

  // Check the result of the efficient algorithm to decide whether to launch
  // more tricky one.
  if ( !isSoftOk || softEraser.HasStatusFlag(asiAlgo_SuppressSoft::StatusCode_WarnNoFaces2Suppress) )
  {
    if ( tryHard )
    {
      m_progress.SendLogMessage( LogNotice(Normal) << "Can not remove isolated faces: falling back to non-isolated suppression." );

      // Suppress hard faces reusing the same AAG. Here we take advantage of
      // the fact that if the soft eraser returns false, it means that the
      // model was not affected, so its AAG is still valid.
      asiAlgo_SuppressHard hardEraser(shape, softEraser.GetAAG(), m_progress, m_plotter);
      //
      if ( !hardEraser.Perform(feature) || hardEraser.GetResult().IsNull() )
      {
        isSuppressed = false;
        stillThere   = feature;
      }
      else
      {
        isSuppressed = true;
        resShape     = hardEraser.GetResult();
        history      = hardEraser.GetHistory();
      }
    }
    else
    {
      isSuppressed = false;
      stillThere   = feature;
    }
  }
  else
  {
    isSuppressed = true;

    m_progress.SendLogMessage( LogNotice(Normal) << "Isolated feature faces were removed." );

    // Check if any unsuppressed features remain.
    stillThere = softEraser.GetUnsuppressed();

    // If any unsuppressed faces remain, inform the user.
    if ( !stillThere.IsEmpty() )
      m_progress.SendLogMessage( LogWarn(Normal) << "Some faces were not suppressed. You may try again selecting only these faces." );

    resShape = softEraser.GetResult();
    history  = softEraser.GetHistory();
  }

  // Fill collection of hard faces.
  if ( !stillThere.IsEmpty() )
  {
    unsuppressed.Unite(stillThere);
  }

  return true;
}
