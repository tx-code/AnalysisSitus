//-----------------------------------------------------------------------------
// Created on: 17 November 2022
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
#include <asiAlgo_OrientCnc.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>
#include <asiAlgo_FeatureAttrArea.h>

// asiAlgo includes
#include <asiAlgo_AAGIterator.h>

// occt includes
#include <BRepBuilderAPI_Transform.hxx>

// Standard includes
#include <algorithm>
#include <vector>

const double MINIMUM_RELATIVE_AREA = 0.1;

//-----------------------------------------------------------------------------

asiAlgo_OrientCnc::asiAlgo_OrientCnc(const Handle(asiAlgo_AAG)& aag,
                                     ActAPI_ProgressEntry       progress,
                                     ActAPI_PlotterEntry        plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_aag             (aag),
  m_fExtents        (0.)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_OrientCnc::Perform()
{
  if ( m_aag.IsNull() )
    return false; // Contract check.

  const TopoDS_Shape& partShape = m_aag->GetMasterShape();

  // Calculate geometric mean of AABB face areas.
  const double basisArea = Pow(asiAlgo_Utils::ComputeAABBVolume(partShape), 2.0 / 3.0);
  //
  m_fExtents = Sqrt(basisArea);

  std::vector<int>    candidates;
  std::vector<gp_Dir> cylAxes;

  for( asiAlgo_AAGRandomIterator it(m_aag); it.More(); it.Next() )
  {
    const int fid           = it.GetFaceId();
    const TopoDS_Face& face = m_aag->GetFace(fid);

    gp_Ax1 cylAxis;

    if ( asiAlgo_Utils::IsPlanar(face) )
    {
      // check area
      const double faceArea = asiAlgo_Utils::CacheFaceArea(fid, m_aag);
      if ( faceArea / basisArea < MINIMUM_RELATIVE_AREA )
      {
        continue;
      }

      candidates.push_back(fid);
    }
    else if ( asiAlgo_Utils::IsCylindrical(face, cylAxis) )
    {
      cylAxes.push_back(cylAxis.Direction());
    }
  }

  std::sort(candidates.begin(), candidates.end(), [&](int a, int b)
  {
    return asiAlgo_Utils::CacheFaceArea(a, m_aag) > asiAlgo_Utils::CacheFaceArea(b, m_aag);
  });

  tl::optional<gp_Ax3> result;

  for ( const auto& candidateId: candidates )
  {
    const TopoDS_Face& candidate = m_aag->GetFace(candidateId);

    const Handle(Geom_Plane)
      candidatePlane = Handle(Geom_Plane)::DownCast( BRep_Tool::Surface(candidate) );

    gp_Ax3 candidateAx3 = candidatePlane->Position(); // This should be XY equal to the UV of the plane and Z the normal

    // Take the opposite of the normal.
    if ( candidate.Orientation() != TopAbs_REVERSED )
    {
      // Flipping X and Z axes means rotating 180 degrees around the Y axis.
      candidateAx3.XReverse();
      candidateAx3.ZReverse();
    }

    if ( !result.has_value() )
    {
      // Default to largest.
      result = candidateAx3;
    }

    if ( std::none_of( cylAxes.begin(), cylAxes.end(),
                      [candidateAx3](const gp_Dir& ax) {
                        return ax.IsParallel(candidateAx3.Axis().Direction(), Precision::Angular());
                      }
                     ) )
    {
      continue;
    }

    result = candidateAx3; // found largest that has parallel cylinder
    break;
  }

  if ( !result.has_value() )
  {
    // there are no base faces
    // take the most common cylinder axis
    int    bestCount = -1;
    gp_Dir bestAxis;

    for ( const auto& cylAxis: cylAxes )
    {
      int curCount = int( std::count_if( cylAxes.begin(), cylAxes.end(),
                                        [cylAxis](const gp_Dir& other) { return cylAxis.IsParallel( other, Precision::Angular() ); }
                                       ) );

      if ( curCount > bestCount )
      {
        bestCount = curCount;
        bestAxis = cylAxis;
      }
    }

    if ( bestCount >= 0 )
    {
      result = gp_Ax3(gp::Origin(), bestAxis);
    }
  }

  if ( !result.has_value() )
  {
    // Just use the pre-existing OZ axis.
    return false;
  }

  m_ax = result;

  gp_Dir OZ = gp::DZ();
  gp_Ax3 XOY( gp::Origin(), OZ );

  if ( result->Direction().IsEqual( OZ, Precision::Angular() ) )
  {
    m_progress.SendLogMessage(LogInfo(Normal) << "Part is already oriented to detected machining axis.");
    return true;
  }

  // calculate the transform to align machining axis to OZ
  m_T = asiAlgo_Utils::GetAlignmentTrsf(XOY, result.value());

  return true;
}
