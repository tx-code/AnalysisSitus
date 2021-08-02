//-----------------------------------------------------------------------------
// Created on: 01 August 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiAlgo_SampleFace.h>

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_Timer.h>

// OpenCascade includes
#include <BRepTools.hxx>

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAlgo_SampleFace::asiAlgo_SampleFace(const TopoDS_Face&   face,
                                       ActAPI_ProgressEntry progress,
                                       ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_face            (face),
  m_fUmin           (0.),
  m_fUmax           (0.),
  m_fVmin           (0.),
  m_fVmax           (0.)
{
  m_class.Init( face, asiAlgo_CheckValidity().MaxTolerance(face) );

  if ( !m_face.IsNull() )
    BRepTools::UVBounds(m_face, m_fUmin, m_fUmax, m_fVmin, m_fVmax);
}

//-----------------------------------------------------------------------------

bool asiAlgo_SampleFace::Perform(const float step)
{
  if ( m_face.IsNull() )
    return false;

  // Domain.
  const double xMin = m_fUmin;
  const double yMin = m_fVmin;
  const double xMax = m_fUmax;
  const double yMax = m_fVmax;

#if defined DRAW_DEBUG
  m_plotter.REDRAW_POINT("Pmin", gp_Pnt2d(xMin, yMin), Color_Red);
  m_plotter.REDRAW_POINT("Pmax", gp_Pnt2d(xMax, yMax), Color_Blue);

  Handle(asiAlgo_BaseCloud<double>)
    innerPts = new asiAlgo_BaseCloud<double>; // inner points

  Handle(asiAlgo_BaseCloud<double>)
    outerPts = new asiAlgo_BaseCloud<double>; // inner points
#endif

  // Steps along axes.
  const float xStep = step;
  const float yStep = step;

  // Number of cells in each dimension.
  const int nx = int( (xMax - xMin) / xStep ) + 1;
  const int ny = int( (yMax - yMin) / yStep ) + 1;

  // Prepare the output grid.
  m_grid = new asiAlgo_UniformGrid<float>( (float) xMin,
                                           (float) yMin,
                                            0.f,
                                            nx, ny, 0,
                                           (float) step );

  // Process the block of data.
  double x, y;
  for ( int i = 0; i <= nx; ++i )
  {
    x = xMin + xStep*i;
    for ( int j = 0; j <= ny; ++j )
    {
      y = yMin + yStep*j;

      // Evaluate scalar.
      const int s = (int) this->classify( gp_Pnt2d(x, y) );

#if defined DRAW_DEBUG
      m_plotter.REDRAW_POINT("probe", gp_Pnt2d(xMin, yMin), Color_Red);
#endif

      // Put scalar into the grid.
      m_grid->pArray[i][j][0] = (float) ( (s & Membership_In) ? 1. : 0. );
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

const Handle(asiAlgo_UniformGrid<float>)& asiAlgo_SampleFace::GetResult() const
{
  return m_grid;
}

//-----------------------------------------------------------------------------

asiAlgo_Membership
  asiAlgo_SampleFace::classify(const gp_Pnt2d& PonS)
{
  const TopAbs_State state = m_class.Perform(PonS);

  if ( state == TopAbs_IN )
    return Membership_In;
  if ( state == TopAbs_ON )
    return Membership_On;
  if ( state == TopAbs_OUT )
    return Membership_Out;

  return Membership_Unknown;
}
