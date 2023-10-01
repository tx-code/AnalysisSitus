//-----------------------------------------------------------------------------
// Created on: 03 August 2021
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
#include <asiAlgo_RecognizeConvexHull.h>

// Analysis Situs includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_AttrFaceUniformGrid.h>
#include <asiAlgo_BuildConvexHull.h>
#include <asiAlgo_BVHFacets.h>
#include <asiAlgo_FeatureAttrConvexHull.h>
#include <asiAlgo_ProjectPointOnMesh.h>
#include <asiAlgo_SampleFace.h>

// OpenCascade includes
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <TopExp_Explorer.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

asiAlgo_RecognizeConvexHull::asiAlgo_RecognizeConvexHull(const TopoDS_Shape&  shape,
                                                         ActAPI_ProgressEntry progress,
                                                         ActAPI_PlotterEntry  plotter)
//
: asiAlgo_Recognizer ( shape, nullptr, progress, plotter ),
  m_iGridPts         ( 20 ),
  m_fToler           ( 0.1 ), // mm
  m_bHaines          ( true ),
  m_bCacheSampl      ( false )
{}

//-----------------------------------------------------------------------------

asiAlgo_RecognizeConvexHull::asiAlgo_RecognizeConvexHull(const Handle(asiAlgo_AAG)& aag,
                                                         ActAPI_ProgressEntry       progress,
                                                         ActAPI_PlotterEntry        plotter)
//
: asiAlgo_Recognizer ( aag, progress, plotter ),
  m_iGridPts         ( 20 ),
  m_fToler           ( 0.1 ), // mm
  m_bHaines          ( true ),
  m_bCacheSampl      ( false )
{}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeConvexHull::SetGridResolution(const int numSlices)
{
  m_iGridPts = numSlices;
}

//-----------------------------------------------------------------------------

int asiAlgo_RecognizeConvexHull::GetGridResolution() const
{
  return m_iGridPts;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeConvexHull::SetTolerance(const double tol)
{
  m_fToler = tol;
}

//-----------------------------------------------------------------------------

double asiAlgo_RecognizeConvexHull::GetTolerance() const
{
  return m_fToler;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeConvexHull::SetUseHaines(const bool on)
{
  m_bHaines = on;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeConvexHull::GetUseHaines() const
{
  return m_bHaines;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeConvexHull::SetCacheSampling(const bool on)
{
  m_bCacheSampl = on;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeConvexHull::GetCacheSampling() const
{
  return m_bCacheSampl;
}

//-----------------------------------------------------------------------------

const Handle(Poly_Triangulation)& asiAlgo_RecognizeConvexHull::GetHullMesh() const
{
  return m_hullMesh;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeConvexHull::GetHullPlanes(std::vector<RTCD::Plane>& planes) const
{
  for ( int i = 1; i <= m_hullMesh->NbTriangles(); ++i )
  {
    int N[3];
    m_hullMesh->Triangle(i).Get(N[0], N[1], N[2]);

    RTCD::Point a = m_hullMesh->Node(N[0]);
    RTCD::Point b = m_hullMesh->Node(N[1]);
    RTCD::Point c = m_hullMesh->Node(N[2]);

    RTCD::Plane plane = RTCD::ComputePlane(a, b, c);

    // Set optional anchor point.
    RTCD::Point mid = ( RTCD::Vector(a) + RTCD::Vector(b) + RTCD::Vector(c) )*(1./3.);
    //
    plane.anchor = mid;

    planes.push_back(plane);
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeConvexHull::Perform()
{
  // Clean up the result.
  m_result.faces.Clear();
  m_result.ids.Clear();

  const bool toDraw = !m_plotter.Access().IsNull();

  /* =============
   *  Prepare AAG.
   * ============= */

  // Build AAG if not available.
  if ( m_aag.IsNull() )
  {
#if defined COUT_DEBUG
    TIMER_NEW
    TIMER_GO
#endif

    m_aag = new asiAlgo_AAG(m_master, false);

#if defined COUT_DEBUG
    TIMER_FINISH
    TIMER_COUT_RESULT_MSG("Construct AAG")
#endif
  }

  /* ===================
   *  Build convex hull.
   * =================== */

  const TopoDS_Shape& shape = m_aag->GetMasterShape();

#if defined COUT_DEBUG
  TIMER_NEW
  TIMER_GO
#endif

  Handle(Poly_Triangulation) hull;

  asiAlgo_BuildConvexHull buildHull(m_progress, m_plotter);
  //
  if ( !buildHull.Perform(shape, hull) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to build convex hull.");
    return false;
  }
  //
  m_hullMesh = hull;

#if defined COUT_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_progress, "Build convex hull")
#endif

  /* ===================================================
   *  Sample faces and project their points to the hull.
   * =================================================== */

  asiAlgo_Feature convexHullFaces;

  // Build BVH for the convex hull.
  Handle(asiAlgo_BVHFacets) hullBvh = new asiAlgo_BVHFacets(hull);
  //
  if ( toDraw )
    hullBvh->Dump(m_plotter);

  // Projection algorithm.
  asiAlgo_ProjectPointOnMesh pointOnMesh(hullBvh);

#if defined COUT_DEBUG
  TIMER_RESET
  TIMER_GO
#endif

  Handle(asiAlgo_BaseCloud<double>) projPts3d = new asiAlgo_BaseCloud<double>;

  int numPointsProcessed = 0;

  // Get the faces to iterate over.
  const TopTools_IndexedMapOfShape& allFaces = m_aag->GetMapOfFaces();
  //
  for ( int f = 1; f <= allFaces.Extent(); ++f )
  {
    const TopoDS_Face& face = TopoDS::Face( allFaces(f) );

    // Sample face in its UV domain.
    asiAlgo_SampleFace sampleFace(face);
    //
    sampleFace.SetPmcAlgo (m_bHaines ? asiAlgo_SampleFace::PmcAlgo_Haines
                                     : asiAlgo_SampleFace::PmcAlgo_Precise);
    sampleFace.SetSquare  (true);
    //
    if ( !sampleFace.Perform(m_iGridPts) )
      continue;

    Handle(asiAlgo_BaseCloud<double>) pts3d = sampleFace.GetResult3d();

    // Add feature points complementary to the overlay grid.
    this->addFeaturePts(face, pts3d);

    const int numProbes = pts3d->GetNumberOfElements();
    numPointsProcessed += numProbes;

    if ( toDraw )
    {
      TCollection_AsciiString ptsName("probes_"); ptsName += f;
      TCollection_AsciiString faceName("face_"); faceName += f;

      m_plotter.DRAW_SHAPE(face, Color_Blue, faceName);
      m_plotter.DRAW_POINTS(pts3d->GetCoordsArray(), Color_Red, ptsName);
    }

    // Project points.
    int numOk = 0;
    //
    for ( int p = 0; p < numProbes; ++p )
    {
      gp_Pnt P      = pts3d->GetElement(p);
      gp_Pnt P_proj = pointOnMesh.Perform(P);

      const double dist = P.Distance(P_proj);

      if ( dist < m_fToler )
      {
        numOk++;
        if ( toDraw )
          projPts3d->AddElement(P_proj);
      }
    }

    if ( numOk == numProbes )
      convexHullFaces.Add(f);

    if ( m_bCacheSampl )
    {
      Handle(asiAlgo_AttrFaceUniformGrid) 
        ug = new asiAlgo_AttrFaceUniformGrid(sampleFace.GetResult());
      //
      m_aag->SetNodeAttribute(f, ug);
    }
  }

#if defined COUT_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("Convex-hull face recognition")
#endif

  if ( toDraw )
  {
    m_plotter.REDRAW_POINTS("proj", projPts3d->GetCoordsArray(), Color_Violet);
  }

  m_progress.SendLogMessage(LogInfo(Normal) << "Total number of points processed: %1."
                                            << numPointsProcessed);

  /* ============================
   *  Settle down AAG attributes.
   * ============================ */

  m_result.ids = convexHullFaces;

  for ( asiAlgo_Feature::Iterator fit(m_result.ids); fit.More(); fit.Next() )
  {
    const int fid = fit.Key();

    Handle(asiAlgo_FeatureAttrConvexHull)
      chAttr = new asiAlgo_FeatureAttrConvexHull;
    //
    m_aag->SetNodeAttribute(fid, chAttr);
  }

  return true; // Success.
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeConvexHull::addFeaturePts(const TopoDS_Face&                       face,
                                                const Handle(asiAlgo_BaseCloud<double>)& pts) const
{
  BRepAdaptor_Surface bas(face);

  TopTools_IndexedMapOfShape verts;
  TopExp::MapShapes(face, TopAbs_VERTEX, verts);

  // Add vertices.
  for ( int vidx = 1; vidx <= verts.Extent(); ++vidx )
  {
    pts->AddElement( BRep_Tool::Pnt( TopoDS::Vertex( verts(vidx) ) ) );
  }

  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);

  // Add midpoint.
  gp_Pnt2d midPt( (uMin + uMax)*0.5, (vMin + vMax)*0.5 );
  pts->AddElement( bas.Value( midPt.X(), midPt.Y() ) );
}
