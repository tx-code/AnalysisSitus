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
#include <asiAlgo_SuppressHard.h>

// OpenCascade includes
#include <BRepAlgoAPI_Defeaturing.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAlgo_SuppressHard::asiAlgo_SuppressHard(const TopoDS_Shape&        masterCAD,
                                           const Handle(asiAlgo_AAG)& aag,
                                           ActAPI_ProgressEntry       progress,
                                           ActAPI_PlotterEntry        plotter)
: ActAPI_IAlgorithm(progress, plotter)
{
  m_input = masterCAD;

  if ( aag.IsNull() )
    m_aag = new asiAlgo_AAG(masterCAD, true);
  else
    m_aag = aag;
}

//-----------------------------------------------------------------------------

bool asiAlgo_SuppressHard::Perform(const asiAlgo_Feature& faceIndices)
{
  // Repack faces.
  TopTools_ListOfShape faces2Kill;
  for ( asiAlgo_Feature::Iterator fit(faceIndices); fit.More(); fit.Next() )
  {
    const TopoDS_Shape& faceSh = m_aag->GetMapOfFaces()( fit.Key() );
    faces2Kill.Append(faceSh);
  }

  // Prepare tool.
  BRepAlgoAPI_Defeaturing API;
  //
  API.SetShape         ( m_aag->GetMasterShape() );
  API.AddFacesToRemove ( faces2Kill );
  API.SetRunParallel   ( false );
  API.SetToFillHistory ( true );

  // Perform.
  try
  {
    API.Build();
  }
  catch ( ... )
  {
    if ( !GetProgress().IsCancelling() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "An exception was raised during face removal.");
      return false;
    }
  }

  if ( GetProgress().IsCancelling() )
    return false;

  // Check result.
  if ( !API.IsDone() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Smart face removal was not completed.");
    return false;
  }
  //
  if ( API.HasWarnings() )
  {
    std::ostringstream out;
    API.DumpWarnings(out);

    m_progress.SendLogMessage(LogWarn(Normal) << "Smart face removal finished with warnings:\n\t %1"
                                              << out.str().c_str() );
    return false;
  }

  // Set result.
  m_output  = API.Shape();
  m_history = API.History();
  return true;
}
