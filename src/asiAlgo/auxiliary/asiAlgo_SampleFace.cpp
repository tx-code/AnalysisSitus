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
#include <asiAlgo_DiscrBuilder.h>
#include <asiAlgo_PointInPoly.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <Geom2dAdaptor_Curve.hxx>

#define TOT_VERTS 10000

static double pgon[TOT_VERTS][2];

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

using namespace asiAlgo;

namespace
{
  //! Discretizes the passed parametric curve `c2d`.
  bool DiscretizePCurve(const Handle(Geom2d_Curve)& c2d,
                        const double                f,
                        const double                l,
                        std::vector<gp_XY>&         points)
  {
    // Discretize with different methods (if one fails, try another).
    int nPts = 0;
    //
    try
    {
      const bool revOrder = (f > l ? true : false);
      //
      Geom2dAdaptor_Curve gac(c2d, revOrder ? l : f, revOrder ? f : l);
      GCPnts_QuasiUniformDeflection QUDefl(gac, 1e-4);
      //
      if ( !QUDefl.IsDone() )
      {
        GCPnts_TangentialDeflection TDefl(gac, 1.0, 1e-4);
        //
        if ( !TDefl.NbPoints() )
        {
          return false;
        }
        else
        {
          nPts = TDefl.NbPoints();

          for ( int p = 1; p <= nPts; ++p )
          {
            gp_XY point = gac.Value( TDefl.Parameter(p) ).XY();
            points.push_back(point);
          }
        }
      }
      else
      {
        nPts = QUDefl.NbPoints();

        if ( revOrder )
          for ( int p = nPts; p >= 1; --p )
            points.push_back( gac.Value( QUDefl.Parameter(p) ).XY() );
        else
          for ( int p = 1; p <= nPts; ++p )
            points.push_back( gac.Value( QUDefl.Parameter(p) ).XY() );

      }
    }
    catch ( ... )
    {
      return false;
    }

    return nPts > 0;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_SampleFace::Wire2Polygon(const TopoDS_Wire&  wire,
                                      const TopoDS_Face&  face,
                                      std::vector<gp_XY>& polygon)
{
  TopTools_IndexedMapOfShape edges;
  TopExp::MapShapes(wire, TopAbs_EDGE, edges);

  for ( BRepTools_WireExplorer wexp(wire); wexp.More(); wexp.Next() )
  {
    const TopoDS_Edge& edge = wexp.Current();

    if ( (edge.Orientation() == TopAbs_INTERNAL) || (edge.Orientation() == TopAbs_EXTERNAL) )
      continue;

    // Access p-curve.
    double f, l;
    Handle(Geom2d_Curve) c2d = BRep_Tool::CurveOnSurface(edge, face, f, l);
    //
    if ( c2d.IsNull() )
    {
      return false;
    }

    std::vector<gp_XY> pts;

    if ( !::DiscretizePCurve(c2d, f, l, pts) )
    {
      return false;
    }

    if ( edge.Orientation() == TopAbs_FORWARD )
    {
      for ( auto pit = pts.cbegin(); pit != pts.cend(); ++pit )
        polygon.push_back(*pit);
    }
    else
    {
      for ( auto pit = pts.rbegin(); pit != pts.rend(); ++pit )
        polygon.push_back(*pit);
    }
  }

#if defined COUT_DEBUG
  std::cout << "--" << std::endl;
  for ( const auto& pt : polygon )
  {
    std::cout << "Next UV point: " << pt.X() << ", " << pt.Y() << std::endl;
  }
#endif

  return true;
}

//-----------------------------------------------------------------------------

asiAlgo_SampleFace::asiAlgo_SampleFace(const TopoDS_Face&   face,
                                       ActAPI_ProgressEntry progress,
                                       ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_bSquare         (false),
  m_algo            (PmcAlgo_Precise),
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

void asiAlgo_SampleFace::SetSquare(const bool square)
{
  m_bSquare = square;
}

//-----------------------------------------------------------------------------

void asiAlgo_SampleFace::SetPmcAlgo(const PmcAlgo algo)
{
  m_algo = algo;
}

//-----------------------------------------------------------------------------

bool asiAlgo_SampleFace::Perform(const int numBins)
{
  if ( m_face.IsNull() )
    return false;

  if ( numBins < 2 )
    return false;

  // Prepare for Haines.
  if ( (m_algo == PmcAlgo_Haines) && m_polygon.empty() )
  {
    TopoDS_Wire wire = asiAlgo_Utils::OuterWire(m_face);

    m_polygon.clear();
    Wire2Polygon(wire, m_face, m_polygon);
  }

  // Prepare for a discrete classifier.
  if ( (m_algo == PmcAlgo_Discrete) && m_discrClass.IsNull() )
  {
    const double discr   = 42;
    const double uGrain  = (m_fUmax - m_fUmin)/discr;
    const double vGrain  = (m_fVmax - m_fVmin)/discr;
    const double uvGrain = Min(uGrain, vGrain);

    // Discretization parameters.
    discr::Params params;
    params.SetMinElemSize(uvGrain);
    params.SetMaxElemSize(uvGrain*10);
    params.SetDeviationAngle(1.*M_PI/180.); // 1 degree.

    // Tessellator.
    discr::Builder discretizer(m_face);
    discretizer.SetParams(params);
    discretizer.Tessellate();

    // Access the discrete model.
    m_discrModel = discretizer.GetModel();
    //
    if (m_discrModel.IsNull() )
      return false;

    // Get the discretized face.
    const discr::Face& dFace = m_discrModel->GetFace(1);

    // Prepare the classifier.
    m_discrClass = new discr::Classifier2d(dFace);
  }

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
  double xStep, yStep, step;
  int nx, ny;
  //
  if ( m_bSquare )
  {
    xStep = (xMax - xMin) / numBins;
    yStep = (yMax - yMin) / numBins;
    step  = Max(xStep, yStep);

    const int _nx = int( (xMax - xMin) / step );
    const int _ny = int( (yMax - yMin) / step );
    const int n   = Max(_nx, _ny);

    nx = ny = n;
  }
  else
  {
    xStep = (numBins + 1) * (xMax - xMin) / (numBins*numBins);
    yStep = (numBins + 1) * (yMax - yMin) / (numBins*numBins);
    step  = Min(xStep, yStep);

    nx = int( (xMax - xMin) / step ) + 1;
    ny = int( (yMax - yMin) / step ) + 1;
  }

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
    x = xMin + step*i;
    for ( int j = 0; j <= ny; ++j )
    {
      y = yMin + step*j;

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

Handle(asiAlgo_BaseCloud<double>) asiAlgo_SampleFace::GetResult3d() const
{
  BRepAdaptor_Surface bas(m_face);

  Handle(asiAlgo_BaseCloud<double>) sampledPts = new asiAlgo_BaseCloud<double>;
  //
  for ( int i = 0; i <= m_grid->Nx; ++i )
  {
    const double x = m_grid->XMin + m_grid->CellSize*i;
    //
    for ( int j = 0; j <= m_grid->Ny; ++j )
    {
      const double y = m_grid->YMin + m_grid->CellSize*j;

      if ( m_grid->pArray[i][j][0] )
        sampledPts->AddElement( bas.Value(x, y) );
    }
  }

  return sampledPts;
}

//-----------------------------------------------------------------------------

asiAlgo_Membership
  asiAlgo_SampleFace::classify(const gp_Pnt2d& PonS)
{
  /* Haines */
  if ( m_algo == PmcAlgo_Haines )
  {
    if ( m_polygon.empty() || (m_polygon.size() >= TOT_VERTS) )
      return Membership_Unknown;

    int idx = 0;
    for ( const auto& uv : m_polygon )
    {
      pgon[idx][0] = uv.X();
      pgon[idx][1] = uv.Y();
      idx++;
    }

    double point[2] = { PonS.X(), PonS.Y() };
    const int state = CrossingsTest(pgon, int( m_polygon.size() ), point);

    if ( state == 1 )
      return Membership_In;
    if ( state == 0 )
      return Membership_Out;
  }

  /* Discrete */
  else if ( m_algo == PmcAlgo_Discrete )
  {
    const discr::Location dLoc = m_discrClass->Locate(PonS);

    if ( dLoc == discr::Location_IN )
      return Membership_In;
    if ( dLoc == discr::Location_ON )
      return Membership_On;
    if ( dLoc == discr::Location_OUT )
      return Membership_Out;
  }

  /* Precise */
  else
  {
    const TopAbs_State state = m_class.Perform(PonS);

    if ( state == TopAbs_IN )
      return Membership_In;
    if ( state == TopAbs_ON )
      return Membership_On;
    if ( state == TopAbs_OUT )
      return Membership_Out;
  }

  return Membership_Unknown;
}

//-----------------------------------------------------------------------------

const Handle(asiAlgo::discr::Model)& asiAlgo_SampleFace::GetDiscrModel() const
{
  return m_discrModel;
}
