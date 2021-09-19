//-----------------------------------------------------------------------------
// Created on: 17 September 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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

// cmdMobius includes
#include <cmdMobius_Mesh.h>

// asiEngine includes
#include <asiEngine_IV.h>
#include <asiEngine_Part.h>
#include <asiEngine_Tessellation.h>
#include <asiEngine_Triangulation.h>

// asiAlgo includes
#include <asiAlgo_MeshComputeNorms.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_MeshSmooth.h>
#include <asiAlgo_Timer.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

#ifdef USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/cascade_Triangulation.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

// OpenCascade includes
#include <gp_Pln.hxx>
#include <Intf_InterferencePolygon2d.hxx>
#include <Intf_Polygon2d.hxx>

typedef std::unordered_set<int> t_domain;

namespace
{
  //! The derived polygon class.
  class SimplePolygon : public Intf_Polygon2d
  {
  public:

    //! Ctor with initializer list.
    SimplePolygon(const std::initializer_list<std::pair<double, double>>& poles)
    {
      for ( const auto& P : poles )
      {
        gp_Pnt2d P2d(P.first, P.second);

        m_poles.push_back( gp_Pnt2d(P.first, P.second) );

        // One thing which is pretty inconvenient is the necessity to
        // update the AABB of a polygon manually. If you forget doing that,
        // the intersection check will return nothing.
        myBox.Add(P2d);
      }
    }

  public:

    //! Returns the tolerance of the polygon.
    virtual double DeflectionOverEstimation() const
    {
      return Precision::Confusion();
    }

    //! Returns the number of segments in the polyline.
    virtual int NbSegments() const
    {
      return int( m_poles.size() - 1 );
    }

    //! Returns the points of the segment <index> in the Polygon.
    virtual void Segment(const int index, gp_Pnt2d& beg, gp_Pnt2d& end) const
    {
      beg = m_poles[index - 1];
      end = m_poles[index];
    }

  protected:
 
    std::vector<gp_Pnt2d> m_poles;

  };

  //! Composes the index domain for all planar faces in the active part.
  //! \return planar domain.
  t_domain ComposePlanarDomain()
  {
    // Compose the domain of planar faces only.
    t_domain            planarDomain;
    Handle(asiAlgo_AAG) aag = cmdMobius::model->GetPartNode()->GetAAG();
    //
    if ( !aag.IsNull() )
    {
      const TopTools_IndexedMapOfShape& allFaces = aag->GetMapOfFaces();
      //
      for ( int fid = 1; fid <= allFaces.Extent(); ++fid )
      {
        if ( asiAlgo_Utils::IsPlanar( aag->GetFace(fid) ) )
          planarDomain.insert(fid);
      }
    }

    return planarDomain;
  }

#if defined USE_MOBIUS
  //! Returns the mesh to work with. It can be either the named mesh from a
  //! Tcl variable or the persistent mesh from the data model.
  //! \param[in] interp the Tcl interpreter.
  //! \param[in] argc   the number of arguments.
  //! \param[in] argv   the argument string.
  //! \return the mesh to work with.
  t_ptr<poly_Mesh> GetActiveMesh(const Handle(asiTcl_Interp)& interp,
                                 int                          argc,
                                 const char**                 argv)
  {
    // Make a copy to assure that we do not modify the persistent data
    // directly as otherwise undo/redo will be buggy.
    t_ptr<t_mesh>
      mesh = cmdMobius::model->GetTriangulationNode()->GetTriangulation()->DeepCopy();

    return mesh;
  }

  void DrawLinks(const std::vector<poly_EdgeHandle>& innerEdges,
                 const std::vector<poly_EdgeHandle>& bndEdges,
                 const t_ptr<t_mesh>&                mesh,
                 const t_ptr<t_plane>&               pln,
                 ActAPI_PlotterEntry                 plotter)
  {
    for ( auto eh : innerEdges )
    {
      poly_Edge edge;
      if ( !mesh->GetEdge(eh, edge) ) continue;

      t_xyz edgeVertices[2];
      if ( !mesh->GetVertex(edge.hVertices[0], edgeVertices[0]) ) continue;
      if ( !mesh->GetVertex(edge.hVertices[1], edgeVertices[1]) ) continue;

      t_uv edgeUVs[2];
      pln->InvertPoint(edgeVertices[0], edgeUVs[0]);
      pln->InvertPoint(edgeVertices[1], edgeUVs[1]);

      plotter.DRAW_LINK( cascade::GetOpenCascadePnt2d(edgeUVs[0]), cascade::GetOpenCascadePnt2d(edgeUVs[1]), Color_Yellow, "link" );
    }

    for ( auto eh : bndEdges )
    {
      poly_Edge edge;
      if ( !mesh->GetEdge(eh, edge) ) continue;

      t_xyz edgeVertices[2];
      if ( !mesh->GetVertex(edge.hVertices[0], edgeVertices[0]) ) continue;
      if ( !mesh->GetVertex(edge.hVertices[1], edgeVertices[1]) ) continue;

      t_uv edgeUVs[2];
      pln->InvertPoint(edgeVertices[0], edgeUVs[0]);
      pln->InvertPoint(edgeVertices[1], edgeUVs[1]);

      plotter.DRAW_LINK( cascade::GetOpenCascadePnt2d(edgeUVs[0]), cascade::GetOpenCascadePnt2d(edgeUVs[1]), Color_Green, "link" );
    }
  }

  bool HasIntersections(const poly_EdgeHandle eh0,
                        const poly_EdgeHandle eh1,
                        const t_ptr<t_mesh>&  mesh,
                        const t_ptr<t_plane>& pln,
                        ActAPI_ProgressEntry  progress,
                        ActAPI_PlotterEntry   plotter)
  {
    poly_Edge edges[2];
    if ( !mesh->GetEdge(eh0, edges[0]) ) return false;
    if ( !mesh->GetEdge(eh1, edges[1]) ) return false;

    t_xyz edge0Vertices[2], edge1Vertices[2];
    if ( !mesh->GetVertex(edges[0].hVertices[0], edge0Vertices[0]) ) return false;
    if ( !mesh->GetVertex(edges[0].hVertices[1], edge0Vertices[1]) ) return false;
    if ( !mesh->GetVertex(edges[1].hVertices[0], edge1Vertices[0]) ) return false;
    if ( !mesh->GetVertex(edges[1].hVertices[1], edge1Vertices[1]) ) return false;

    t_uv edge0UVs[2], edge1UVs[2];
    pln->InvertPoint(edge0Vertices[0], edge0UVs[0]);
    pln->InvertPoint(edge0Vertices[1], edge0UVs[1]);
    pln->InvertPoint(edge1Vertices[0], edge1UVs[0]);
    pln->InvertPoint(edge1Vertices[1], edge1UVs[1]);

    SimplePolygon poly0 = { {edge0UVs[0].U(), edge0UVs[0].V()}, {edge0UVs[1].U(), edge0UVs[1].V()} };
    SimplePolygon poly1 = { {edge1UVs[0].U(), edge1UVs[0].V()}, {edge1UVs[1].U(), edge1UVs[1].V()} };

    Intf_InterferencePolygon2d algo(poly0, poly1);
    const int numPts = algo.NbSectionPoints();

    int numInters = 0;
    for ( int isol = 1; isol <= numPts; ++isol )
    {
      const double p[2] = { algo.PntValue(isol).ParamOnFirst(),
                            algo.PntValue(isol).ParamOnSecond() };

      if ( ( (p[0] > 0) && (p[0] < 1) ) || ( (p[1] > 0) && (p[1] < 1) ) )
      {
        gp_Pnt P = algo.PntValue(isol).Pnt();
        progress.SendLogMessage(LogNotice(Normal) << "Intersection p[0] = %1." << p[0]);
        progress.SendLogMessage(LogNotice(Normal) << "Intersection p[1] = %1." << p[1]);
        plotter.DRAW_POINT( gp_Pnt2d( P.X(), P.Y() ), Color_Red, "intersection" );
        numInters++;
      }
    }

    return numInters > 0;
  }
#endif
}

//-----------------------------------------------------------------------------

Handle(asiEngine_Model)        cmdMobius::model = nullptr;
Handle(asiUI_CommonFacilities) cmdMobius::cf    = nullptr;

//-----------------------------------------------------------------------------

void cmdMobius::ClearViewers(const bool repaint)
{
  if ( cf.IsNull() )
    return;

  // Get all presentation managers
  const vtkSmartPointer<asiVisu_PrsManager>& partPM   = cf->ViewerPart->PrsMgr();
  const vtkSmartPointer<asiVisu_PrsManager>& hostPM   = cf->ViewerHost->PrsMgr();
  const vtkSmartPointer<asiVisu_PrsManager>& domainPM = cf->ViewerDomain->PrsMgr();

  // Update viewers
  partPM  ->DeleteAllPresentations();
  hostPM  ->DeleteAllPresentations();
  domainPM->DeleteAllPresentations();

  if ( repaint )
  {
    cf->ViewerPart  ->Repaint();
    cf->ViewerHost  ->Repaint();
    cf->ViewerDomain->Repaint();
  }
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_Init(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
#if defined USE_MOBIUS
  // Get shape.
  Handle(asiData_PartNode) part_n = cmdMobius::model->GetPartNode();
  TopoDS_Shape             shape  = part_n->GetShape();
  //
  if ( shape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part shape is null.");
    return TCL_ERROR;
  }

  asiAlgo_MeshMerge meshMerge(shape, asiAlgo_MeshMerge::Mode_MobiusMesh);
  const t_ptr<poly_Mesh>& mesh = meshMerge.GetMobiusMesh();

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles: %1."
                                                        << mesh->GetNumTriangles() );

  // Get triangulation node.
  Handle(asiData_TriangulationNode) tris_n = cmdMobius::model->GetTriangulationNode();

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_ComputeNorms(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
#if defined USE_MOBIUS
  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Compute norm of each triangle.
  Handle(HIntArray)                elemIds;
  Handle(HRealArray)               elemNorms;
  NCollection_DataMap<int, gp_Vec> norms;
  int                              tidx = 0;
  //
  for ( t_mesh::TriangleIterator tit(mesh); tit.More(); tit.Next(), ++tidx )
  {
    poly_TriangleHandle ht = tit.Current();

    // Compute norm.
    t_xyz N;
    if ( !mesh->ComputeNormal(ht, N) )
      continue;

    norms.Bind( tidx, cascade::GetOpenCascadeVec(N) );
  }

  // Convert to plain arrays.
  asiAlgo_MeshComputeNorms::GetResultArrays(norms, elemIds, elemNorms);

  // Create Data Node for the norms.
  Handle(asiData_MeshNormsNode) norms_n;
  //
  cmdMobius::model->OpenCommand();
  {
    norms_n = asiEngine_Tessellation(cmdMobius::model).CreateNorms(cmdMobius::model->GetTriangulationNode(),
                                                                  "Normal field",
                                                                   true); // Elemental.
    //
    norms_n->SetIDs(elemIds);
    norms_n->SetVectors(elemNorms);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(norms_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_FlipEdges(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Get the desired number of flipping iterations.
  int iter = 1;
  interp->GetKeyValue(argc, argv, "iter", iter);

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles: %1. To flip edges in %2 iteration(s)."
                                                        << mesh->GetNumTriangles() << iter );

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( !facetIds.Extent() ) // Entire mesh.
  {
    TIMER_NEW
    TIMER_GO

    // Flip edges.
    for ( int i = 0; i < iter; ++i )
    {
      mesh->ComputeEdges();
      mesh->FlipEdges(1./180.*M_PI, 5./180.*M_PI);
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Flip edges")
  }
  else if ( facetIds.Extent() == 2 )
  {
    poly_TriangleHandle th[2] = { poly_TriangleHandle( facetIds.GetMinimalMapped() ),
                                  poly_TriangleHandle( facetIds.GetMaximalMapped() ) };

    mesh->ComputeEdges();

    poly_EdgeHandle he = mesh->FindEdge(th[0], th[1]);
    //
    if ( !he.IsValid() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "The selected triangles have no common edge.");
      return TCL_ERROR;
    }

    const bool force = interp->HasKeyword(argc, argv, "force");
    bool       isOk  = false;

    TIMER_NEW
    TIMER_GO

    mesh->ComputeEdges();

    // Flip the common edge.
    if ( force )
      isOk = mesh->FlipEdge(he, 1./180.*M_PI, 15./180.*M_PI, false, false);
    else
      isOk = mesh->FlipEdge(he);

    if ( !isOk )
    {
      if ( !force )
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot flip the edge (try '-force').");
      else
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot forcibly flip the edge.");

      return TCL_ERROR;
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Flip edge")
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, select two adjacent triangles.");
    return TCL_ERROR;
  }

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_FindAdjacent(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
#if defined USE_MOBIUS
  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( !facetIds.Extent() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No facets are selected.");
    return TCL_ERROR;
  }

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Compute links")

  TIMER_RESET
  TIMER_GO

  // Find triangles.
  TColStd_PackedMapOfInteger foundIds;
  //
  for ( TColStd_PackedMapOfInteger::Iterator fit(facetIds); fit.More(); fit.Next() )
  {
    const int fid = fit.Key(); // Mobius indices are 0-based.
    const poly_TriangleHandle th(fid);

    std::vector<poly_TriangleHandle> ths;

    if ( interp->HasKeyword(argc, argv, "v") )
    {
      std::unordered_set<poly_TriangleHandle> thsSet;
      mesh->FindAdjacentByVertices(th, thsSet);
      //
      for ( const auto& ath : thsSet )
        ths.push_back(ath);
    }
    else
    {
      if ( !mesh->FindAdjacentByEdges(th, ths) )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find adjacent triangles.");
        return TCL_ERROR;
      }
    }

    for ( const auto& _th : ths )
      foundIds.Add(_th.iIdx); // OpenCascade triangles are 1-based.
  }
  //
  foundIds.Subtract(facetIds); // Do not pass the initially selected facets.

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Find adjacent triangles")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles found: %1."
                                                        << foundIds.Extent() );

  trisApi.HighlightFacets(foundIds);

  *interp << foundIds;

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_FindBoundary(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
#if defined USE_MOBIUS
  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Compute links")

  TIMER_RESET
  TIMER_GO

  // Find boundary.
  std::vector<poly_EdgeHandle>     bndEdges;
  std::vector<poly_TriangleHandle> bndTris;
  //
  mesh->FindBoundaryEdges(bndEdges, bndTris);

  if ( interp->HasKeyword(argc, argv, "draw") )
  {
    for ( const auto he : bndEdges )
    {
      poly_Edge e;
      mesh->GetEdge(he, e);

      t_xyz V[2];
      mesh->GetVertex(e.hVertices[0], V[0]);
      mesh->GetVertex(e.hVertices[1], V[1]);

      gp_XYZ P[2] = { gp_XYZ( V[0].X(), V[0].Y(), V[0].Z() ),
                      gp_XYZ( V[1].X(), V[1].Y(), V[1].Z() ) };

      interp->GetPlotter().DRAW_LINK(P[0], P[1], Color_Red, "bnd");
    }
  }

  TColStd_PackedMapOfInteger foundIds;
  for ( const auto ht : bndTris )
    foundIds.Add(ht.iIdx);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Find boundary triangles")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of boundary triangles found: %1."
                                                        << foundIds.Extent() );

  trisApi.HighlightFacets(foundIds);

  *interp << foundIds;

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_RefineMidpoints(const Handle(asiTcl_Interp)& interp,
                                int                          argc,
                                const char**                 argv)
{
#if defined USE_MOBIUS
  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);
  //
  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "%1 triangles in the active mesh."
                                                        << mesh->GetNumTriangles() );

  // Compose the domain of interest.
  t_domain domain;
  if ( interp->HasKeyword(argc, argv, "planar") )
    domain = ::ComposePlanarDomain();

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( !facetIds.Extent() )
  {
    TIMER_NEW
    TIMER_GO

    double areaThreshold = 0.01;
    double lenThreshold  = 0.01;
    //
    interp->GetKeyValue(argc, argv, "minarea", areaThreshold);

    // Compute areas.
    std::vector<poly_TriangleHandle> ths;
    std::vector<double>              tAreas;
    std::vector<int>                 tIds;
    std::vector<int>                 tNums;
    //
    for ( poly_Mesh::TriangleIterator tit(mesh); tit.More(); tit.Next() )
    {
      poly_TriangleHandle th( tit.Current() );
      poly_Triangle       t;
      //
      if ( !mesh->GetTriangle(th, t) || t.IsDeleted() )
        continue;

      tAreas .push_back( mesh->ComputeArea(th) );
      tIds   .push_back( tit.Current().GetIdx() );
      tNums  .push_back( int( tNums.size() ) );
    }

    // Sort facets by descending areas.
    std::sort( tNums.begin(), tNums.end(),
               [&](const int a, const int b)
               {
                 return tAreas[a] > tAreas[b];
               } );

    // Refine. Notice that we do not use triangle iterator here as more triangles
    // are added as long as we refine.
    for ( int idx : tNums )
    {
      poly_TriangleHandle th(tIds[idx]);
      poly_Triangle       t;

      // Get the next triangle to process.
      mesh->GetTriangle(th, t);
      //
      if ( t.IsDeleted() )
        continue;

      // Check that this triangle is in the domain.
      if ( !domain.empty() && !domain.count( t.GetFaceRef() ) )
        continue;

      // Refine triangle based on its size.
      const double area = mesh->ComputeArea(th);
      const double len  = mesh->ComputeMaxLen(th);

      /*std::cout << "triangle " << idx << ": (area, len) = ("
                << area << ", " << len << ")" << std::endl;*/

      if ( (area > areaThreshold) || (len > lenThreshold) )
      {
        mesh->RefineByMidpoint(th);
      }
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Midpoint refine")
  }
  else
  {
    // Refine triangles.
    for ( TColStd_PackedMapOfInteger::Iterator fit(facetIds); fit.More(); fit.Next() )
    {
      const int fid = fit.Key();

      interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Refining facet #%1." << fid);

      if ( !mesh->RefineByMidpoint( poly_TriangleHandle(fid) ) )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot refine by midpoint.");
        return TCL_ERROR;
      }
    }
  }

  // Update Data Model.
  cmdMobius::model->OpenCommand();
  {
    cmdMobius::model->GetTriangulationNode()->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Actualize.
  if ( cmdMobius::cf->ViewerPart )
    cmdMobius::cf->ViewerPart->PrsMgr()->Actualize( cmdMobius::model->GetTriangulationNode() );

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_RefineByMidedges(const Handle(asiTcl_Interp)& interp,
                                 int                          argc,
                                 const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Compose the domain of interest.
  t_domain domain;
  if ( interp->HasKeyword(argc, argv, "planar") )
    domain = ::ComposePlanarDomain();

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( !facetIds.Extent() )
  {
    TIMER_NEW
    TIMER_GO

    // Compute links.
    mesh->ComputeEdges();

    double areaThreshold = 0.01;
    double lenThreshold  = 0.01;
    //
    interp->GetKeyValue(argc, argv, "minarea", areaThreshold);
    interp->GetKeyValue(argc, argv, "minlen",  lenThreshold);

    const int maxIter = 1;
    bool      stop    = false;
    int       iter    = 0;

    // Refine. Notice that we do not use triangle iterator here as more triangles
    // are added as long as we refine.
    do
    {
      // Compute areas.
      std::vector<poly_TriangleHandle> ths;
      std::vector<double>              tAreas;
      std::vector<int>                 tIds;
      std::vector<int>                 tNums;
      //
      for ( poly_Mesh::TriangleIterator tit(mesh); tit.More(); tit.Next() )
      {
        poly_TriangleHandle th( tit.Current() );
        poly_Triangle       t;
        //
        if ( !mesh->GetTriangle(th, t) || t.IsDeleted() )
          continue;

        tAreas .push_back( mesh->ComputeArea(th) );
        tIds   .push_back( tit.Current().GetIdx() );
        tNums  .push_back( int( tNums.size() ) );
      }

      // Sort facets by descending areas.
      std::sort( tNums.begin(), tNums.end(),
                 [&](const int a, const int b)
                 {
                   return tAreas[a] > tAreas[b];
                 } );

      bool anyRefined = false;
      //
      for ( int idx : tNums )
      {
        poly_TriangleHandle th(tIds[idx]);
        poly_Triangle       t;

        // Get the next triangle to process.
        mesh->GetTriangle(th, t);
        //
        if ( t.IsDeleted() )
          continue;

        // Check that this triangle is in the domain.
        if ( !domain.empty() && !domain.count( t.GetFaceRef() ) )
          continue;

        // Refine triangle based on its size.
        const double area = mesh->ComputeArea(th);
        const double len  = mesh->ComputeMaxLen(th);

        /*std::cout << "triangle " << idx << ": (area, len) = ("
                  << area << ", " << len << ")" << std::endl;*/

        if ( (area > areaThreshold) && (len > lenThreshold) )
        {
          mesh->RefineByMidedges(th);

          if ( !anyRefined ) anyRefined = true;
        }
      }

      if ( !anyRefined || (++iter >= maxIter) )
        stop = true;
    }
    while ( !stop );

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Refine entire mesh by midedges")
  }
  else
  {
    TIMER_NEW
    TIMER_GO

    // Compute links.
    mesh->ComputeEdges();

    // Refine triangles.
    for ( TColStd_PackedMapOfInteger::Iterator fit(facetIds); fit.More(); fit.Next() )
    {
      const int fid = fit.Key(); // Mobius indices are 0-based.

      interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Refining facet #%1." << fid);

      std::vector<poly_TriangleHandle> ths;
      if ( !mesh->RefineByMidedges(poly_TriangleHandle(fid), ths) )
      {
        interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Cannot refine facet #%1 by midedges."
                                                             << fid);
        continue;
      }
    }

    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Refine selected facets by midedges")
  }

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles after refine: %1."
                                                        << mesh->GetNumTriangles() );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_CollapseEdge(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( facetIds.Extent() != 2 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, select two adjacent triangles.");
    return TCL_ERROR;
  }

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  poly_TriangleHandle th[2] = { poly_TriangleHandle( facetIds.GetMinimalMapped() ),
                                poly_TriangleHandle( facetIds.GetMaximalMapped() ) };

  poly_EdgeHandle he = mesh->FindEdge(th[0], th[1]);
  //
  if ( !he.IsValid() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The selected triangles have no common edge.");
    return TCL_ERROR;
  }

  const bool force = interp->HasKeyword(argc, argv, "force");
  bool       isOk  = false;

  // Collapse the common edge.
  if ( force )
    isOk = mesh->CollapseEdge(he, false, false);
  else
    isOk = mesh->CollapseEdge(he, true, true, 0.01);

  if ( !isOk )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot collapse the edge (try '-force').");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Collapse edge")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles after edge collapse: %1."
                                                        << mesh->GetNumTriangles() );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_CollapseEdges(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  // Read max edge length.
  double maxLen = 1.;
  interp->GetKeyValue(argc, argv, "maxlen", maxLen);

  // Compose the domain of interest.
  t_domain domain;
  if ( interp->HasKeyword(argc, argv, "planar") )
    domain = ::ComposePlanarDomain();

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  const int maxIter = 1;//10;
  bool      stop    = false;
  int       iter    = 0;
  int       nbDone  = 0;

  mesh->ComputeEdges();

  // Refine.
  do
  {
    bool anyRefined = false;
    //
    for ( poly_Mesh::EdgeIterator eit(mesh); eit.More(); eit.Next() )
    {
      const poly_EdgeHandle eh = eit.Current();
      poly_Edge             e;

      // Get the next edge.
      mesh->GetEdge(eh, e);
      //
      if ( e.IsDeleted() )
        continue;

      t_xyz V[2];
      mesh->GetVertex(e.hVertices[0], V[0]);
      mesh->GetVertex(e.hVertices[1], V[1]);

      // Refine triangle based on its size.
      const double len = (V[1] - V[0]).Modulus();
      //
      if ( len < maxLen )
      {
        if ( mesh->CollapseEdge(eh, true, true, 0.01, domain) )
        {
          nbDone++;

          //mesh->ComputeEdges();
        }

        if ( !anyRefined ) anyRefined = true;
      }
    }

    if ( !anyRefined || (++iter >= maxIter) )
      stop = true;
  }
  while ( !stop );

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Collapse edges")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of edges collapsed: %1."
                                                        << nbDone );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_Smooth(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
#if defined USE_MOBIUS
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  int iter = 1;
  interp->GetKeyValue(argc, argv, "iter", iter);

  // Compose the domain of interest.
  t_domain domain;
  if ( interp->HasKeyword(argc, argv, "planar") )
    domain = ::ComposePlanarDomain();

  const int numTris = mesh->GetNumTriangles();

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "%1 triangles to smooth in %2 iteration(s)."
                                                       << numTris << iter);

  mesh->ComputeEdges();
  mesh->Smooth(iter, domain);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Smooth")

  // Update Data Model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Actualize.
  if ( cmdMobius::cf->ViewerPart )
    cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_RefineInc(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
#if defined USE_MOBIUS
  /* =============
   *  Preparation.
   * ============= */

  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Compose the domain of interest.
  t_domain domain = ::ComposePlanarDomain();

  double coeff = 0.33;
  //
  if ( interp->HasKeyword(argc, argv, "fine") )
    coeff = 0.05;

  /* ============
   *  Refinement.
   * ============ */

  TIMER_NEW
  TIMER_GO

  // Get bounds to use for the fineness control.
  double xMin, xMax, yMin, yMax, zMin, zMax;
  mesh->GetBounds(xMin, xMax, yMin, yMax, zMin, zMax);

  // Derive the min element size.
  const double minDim  = Min(Abs(xMax - xMin), Min(Abs(yMax - yMin), Abs(zMax - zMin)));
  const double minLen  = minDim*coeff;
  const double minArea = minLen*minLen;

  interp->GetProgress().SendLogMessage(LogNotice(Normal) << "Min edge length: %1." << minLen);
  interp->GetProgress().SendLogMessage(LogNotice(Normal) << "Min triangle area: %1." << minArea);

  /* Next stage: refine midedges */

  const int maxIter = 4;
  bool      stop    = false;
  int       iter    = 0;

  mesh->ComputeEdges();

  // Refine. Notice that we do not use triangle iterator here as more triangles
  // are added as long as we refine.
  do
  {
    // Compute areas.
    std::vector<poly_TriangleHandle> ths;
    std::vector<double>              tAreas;
    std::vector<int>                 tIds;
    std::vector<int>                 tNums;
    //
    for ( poly_Mesh::TriangleIterator tit(mesh); tit.More(); tit.Next() )
    {
      poly_TriangleHandle th( tit.Current() );
      poly_Triangle       t;
      //
      if ( !mesh->GetTriangle(th, t) || t.IsDeleted() )
        continue;

      tAreas .push_back( mesh->ComputeArea(th) );
      tIds   .push_back( tit.Current().GetIdx() );
      tNums  .push_back( int( tNums.size() ) );
    }

    // Sort facets by descending areas.
    std::sort( tNums.begin(), tNums.end(),
                [&](const int a, const int b)
                {
                  return tAreas[a] > tAreas[b];
                } );

    bool anyRefined = false;
    //
    for ( int idx : tNums )
    {
      poly_TriangleHandle th(tIds[idx]);
      poly_Triangle       t;

      // Get the next triangle to process.
      mesh->GetTriangle(th, t);
      //
      if ( t.IsDeleted() )
        continue;

      // Check that this triangle is in the domain.
      if ( !domain.empty() && !domain.count( t.GetFaceRef() ) )
        continue;

      // Refine triangle based on its size.
      const double area = mesh->ComputeArea(th);
      const double len  = mesh->ComputeMaxLen(th);

      /*std::cout << "triangle " << idx << ": (area, len) = ("
                << area << ", " << len << ")" << std::endl;*/

      if ( (area > minArea) || (len > minLen) )
      {
        mesh->RefineByMidedges(th);

        if ( !anyRefined ) anyRefined = true;
      }
    }

    if ( !anyRefined || (++iter >= maxIter) )
      stop = true;

    if ( !stop )
    {
      mesh->FlipEdges(1./180.*M_PI, 5./180.*M_PI);
      mesh->ComputeEdges();
    }
  }
  while ( !stop );

  /* Next stage: refine my midpoints */

  {
    // Compute areas.
    std::vector<poly_TriangleHandle> ths;
    std::vector<double>              tAreas;
    std::vector<int>                 tIds;
    std::vector<int>                 tNums;
    //
    for ( poly_Mesh::TriangleIterator tit(mesh); tit.More(); tit.Next() )
    {
      poly_TriangleHandle th( tit.Current() );
      poly_Triangle       t;
      //
      if ( !mesh->GetTriangle(th, t) || t.IsDeleted() )
        continue;

      tAreas .push_back( mesh->ComputeArea(th) );
      tIds   .push_back( tit.Current().GetIdx() );
      tNums  .push_back( int( tNums.size() ) );
    }

    // Sort facets by descending areas.
    std::sort( tNums.begin(), tNums.end(),
               [&](const int a, const int b)
               {
                 return tAreas[a] > tAreas[b];
               } );

    // Refine. Notice that we do not use triangle iterator here as more triangles
    // are added as long as we refine.
    for ( int idx : tNums )
    {
      poly_TriangleHandle th(tIds[idx]);
      poly_Triangle       t;

      // Get the next triangle to process.
      mesh->GetTriangle(th, t);
      //
      if ( t.IsDeleted() )
        continue;

      // Check that this triangle is in the domain.
      if ( !domain.empty() && !domain.count( t.GetFaceRef() ) )
        continue;

      // Refine triangle based on its size.
      const double area = mesh->ComputeArea(th);
      const double len  = mesh->ComputeMaxLen(th);

      /*std::cout << "triangle " << idx << ": (area, len) = ("
                << area << ", " << len << ")" << std::endl;*/

      if ( (area > minArea) && (len > minLen) )
      {
        mesh->RefineByMidpoint(th);
      }
    }
  }

  /* Next stage: split boundary edges */

  // Find boundary.
  //std::vector<poly_EdgeHandle>     bndEdges;
  //std::vector<poly_TriangleHandle> bndTris;
  ////
  //mesh->FindBoundaryEdges(bndEdges, bndTris);

  //for ( const auto he : bndEdges )
  //{
  //  mesh->SplitEdge(he);
  //}

  /* Stage 03: flip edges */

  // Flip edges.
  for ( int i = 0; i < maxIter; ++i )
  {
    mesh->ComputeEdges();
    mesh->FlipEdges(1./180.*M_PI, 5./180.*M_PI);
  }

  /* Next stage: smooth mesh */

  mesh->ComputeEdges();
  mesh->Smooth(1, domain);

  /* Next stage: flip edges */

  // Flip edges.
  for ( int i = 0; i < maxIter; ++i )
  {
    mesh->ComputeEdges();
    mesh->FlipEdges(1./180.*M_PI, 5./180.*M_PI);
  }

  /* Next stage: smooth mesh */

  mesh->ComputeEdges();
  mesh->Smooth(10, domain);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Incremental refine")

  /* ==============
   *  Finalization.
   * ============== */

  // Update Data Model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Actualize.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_Check(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
#if defined USE_MOBIUS
  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( facetIds.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, select facet(s).");
    return TCL_ERROR;
  }

  for ( TColStd_PackedMapOfInteger::Iterator fit(facetIds); fit.More(); fit.Next() )
  {
    const poly_TriangleHandle ht( fit.Key() - 1 );

    const double L = mesh->ComputeMaxLen(ht);
    const double A = mesh->ComputeArea(ht);
    const double J = mesh->ComputeScaledJacobian(ht);

    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Max length of the facet %1 is %2."
                                                         << ht.GetIdx() << L);

    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Area of the facet %1 is %2."
                                                         << ht.GetIdx() << A);

    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Scaled Jacobian of the facet %1 is %2."
                                                         << ht.GetIdx() << J);
  }

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_SplitEdge(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( facetIds.Extent() != 2 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, select two adjacent triangles.");
    return TCL_ERROR;
  }

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  poly_TriangleHandle th[2] = { poly_TriangleHandle( facetIds.GetMinimalMapped() ),
                                poly_TriangleHandle( facetIds.GetMaximalMapped() ) };

  poly_EdgeHandle he = mesh->FindEdge(th[0], th[1]);
  //
  if ( !he.IsValid() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The selected triangles have no common edge.");
    return TCL_ERROR;
  }

  // Split the common edge.
  if ( !mesh->SplitEdge(he) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot split the edge.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Split edge")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles after edge split: %1."
                                                        << mesh->GetNumTriangles() );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_SplitBoundary(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Compute links")

  TIMER_RESET
  TIMER_GO

  // Find boundary.
  std::vector<poly_EdgeHandle>     bndEdges;
  std::vector<poly_TriangleHandle> bndTris;
  //
  mesh->FindBoundaryEdges(bndEdges, bndTris);

  if ( interp->HasKeyword(argc, argv, "draw") )
  {
    for ( const auto he : bndEdges )
    {
      poly_Edge e;
      mesh->GetEdge(he, e);

      t_xyz V[2];
      mesh->GetVertex(e.hVertices[0], V[0]);
      mesh->GetVertex(e.hVertices[1], V[1]);

      gp_XYZ P[2] = { gp_XYZ( V[0].X(), V[0].Y(), V[0].Z() ),
                      gp_XYZ( V[1].X(), V[1].Y(), V[1].Z() ) };

      interp->GetPlotter().DRAW_LINK(P[0], P[1], Color_Red, "bnd");
    }
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Find boundary edges")

  TIMER_RESET
  TIMER_GO

  for ( const auto he : bndEdges )
  {
    mesh->SplitEdge(he);
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Split boundary edges")

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation(mesh);
  }
  cmdMobius::model->CommitCommand();

  // Update UI.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);
  cmdMobius::cf->ObjectBrowser->Populate();

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_FindDomainEdges(const Handle(asiTcl_Interp)& interp,
                                int                          argc,
                                const char**                 argv)
{
#if defined USE_MOBIUS
  int domainId = -1;
  if ( !interp->GetKeyValue(argc, argv, "domain", domainId) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The domain ID is not specified.");
    return TCL_ERROR;
  }

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Compute links")

  TIMER_RESET
  TIMER_GO

  // Find boundary.
  std::vector<poly_EdgeHandle> innerEdges, bndEdges;
  //
  mesh->FindDomainEdges(domainId, innerEdges, bndEdges);

  if ( interp->HasKeyword(argc, argv, "draw") )
  {
    for ( const auto he : innerEdges )
    {
      poly_Edge e;
      mesh->GetEdge(he, e);

      t_xyz V[2];
      mesh->GetVertex(e.hVertices[0], V[0]);
      mesh->GetVertex(e.hVertices[1], V[1]);

      gp_XYZ P[2] = { gp_XYZ( V[0].X(), V[0].Y(), V[0].Z() ),
                      gp_XYZ( V[1].X(), V[1].Y(), V[1].Z() ) };

      interp->GetPlotter().DRAW_LINK(P[0], P[1], Color_Blue, "innerEdge");
    }

    for ( const auto he : bndEdges )
    {
      poly_Edge e;
      mesh->GetEdge(he, e);

      t_xyz V[2];
      mesh->GetVertex(e.hVertices[0], V[0]);
      mesh->GetVertex(e.hVertices[1], V[1]);

      gp_XYZ P[2] = { gp_XYZ( V[0].X(), V[0].Y(), V[0].Z() ),
                      gp_XYZ( V[1].X(), V[1].Y(), V[1].Z() ) };

      interp->GetPlotter().DRAW_LINK(P[0], P[1], Color_Magenta, "bndEdge");
    }
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Find domain edges")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of inner edges found: %1."
                                                        << int( innerEdges.size() ) );
  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of boundary edges found: %1."
                                                        << int( bndEdges.size() ) );

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_CheckDomainInter(const Handle(asiTcl_Interp)& interp,
                                 int                          argc,
                                 const char**                 argv)
{
#if defined USE_MOBIUS
  int domainId = -1;
  if ( !interp->GetKeyValue(argc, argv, "domain", domainId) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The domain ID is not specified.");
    return TCL_ERROR;
  }

  Handle(asiAlgo_AAG) aag = cmdMobius::model->GetPartNode()->GetAAG();
  //
  if ( aag.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "AAG is null, so we cannot access the host surface for a domain.");
    return TCL_ERROR;
  }
  //
  if ( !aag->HasFace(domainId) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a face with the ID %1." << domainId);
    return TCL_ERROR;
  }

  Handle(Geom_Plane) occPlane;
  t_ptr<t_plane>     pln;
  //
  if ( asiAlgo_Utils::IsPlanar(aag->GetFace(domainId), occPlane) )
  {
    pln = cascade::GetMobiusPlane(occPlane);
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The face with the ID %1 is not planar." << domainId);
    return TCL_ERROR;
  }

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the active mesh.
  t_ptr<t_mesh> mesh = ::GetActiveMesh(interp, argc, argv);

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Compute links")

  // Find edges.
  std::vector<poly_EdgeHandle> innerEdges, bndEdges;
  //
  mesh->FindDomainEdges(domainId, innerEdges, bndEdges);

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of inner edges found: %1."
                                                        << int( innerEdges.size() ) );
  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of boundary edges found: %1."
                                                        << int( bndEdges.size() ) );

  if ( interp->HasKeyword(argc, argv, "draw") )
    ::DrawLinks( innerEdges, bndEdges, mesh, pln, interp->GetPlotter() );

  TIMER_RESET
  TIMER_GO

  bool hasInters = false;
  for ( const auto he_inner : innerEdges )
  {
    for ( const auto he_bnd : bndEdges )
    {
      if ( ::HasIntersections( he_inner, he_bnd, mesh, pln, interp->GetProgress(), interp->GetPlotter() ) )
      {
        hasInters = true;
        break;
      }
    }

    if ( hasInters )
      break;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Find domain intersections")

  if ( hasInters )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Intersections detected.");
  else
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "No intersections detected.");

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

void cmdMobius::Factory(const Handle(asiTcl_Interp)&      interp,
                        const Handle(Standard_Transient)& data)
{
  static const char* group = "cmdMobius";

  /* ==========================
   *  Initialize UI facilities.
   * ========================== */

  // Get common facilities.
  Handle(asiUI_CommonFacilities)
    passedCF = Handle(asiUI_CommonFacilities)::DownCast(data);
  //
  if ( passedCF.IsNull() )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "[cmdMobius] UI facilities are not available. GUI may not be updated.");
  else
    cf = passedCF;

  /* ================================
   *  Initialize Data Model instance.
   * ================================ */

  model = Handle(asiEngine_Model)::DownCast( interp->GetModel() );
  //
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "[cmdMobius] Data Model instance is null or not of asiEngine_Model kind.");
    return;
  }

  /* ==================
   *  Add Tcl commands.
   * ================== */

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-init",
    //
    "poly-init\n"
    "\n"
    "\t Initializes a mesh from a CAD part.",
    //
    __FILE__, group, MOBIUS_POLY_Init);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-compute-norms",
    //
    "poly-compute-norms\n"
    "\n"
    "\t Computes normal field.",
    //
    __FILE__, group, MOBIUS_POLY_ComputeNorms);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-flip-edges",
    //
    "poly-flip-edges [-force] [-iter <num>]\n"
    "\n"
    "\t Flips triangulation edges for entire model if no facets are selected.\n"
    "\t If two facets with a common edge are selected, the shared edge will be\n"
    "\t flipped unless it's impossible to do. If so, you can still use the '-force'\n"
    "\t flag to relax the angular distortion criteria applied by the flipping operator.\n"
    "\t You might also want to perfom edge flipping iteratively. For that, pass the\n"
    "\t '-iter' flag followed by the number of iterations.",
    //
    __FILE__, group, MOBIUS_POLY_FlipEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-find-adjacent",
    //
    "poly-find-adjacent [-v]\n"
    "\n"
    "\t Finds adjacent triangles for the given one.",
    //
    __FILE__, group, MOBIUS_POLY_FindAdjacent);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-find-boundary",
    //
    "poly-find-boundary [-draw]\n"
    "\n"
    "\t Finds boundary edges and triangles.",
    //
    __FILE__, group, MOBIUS_POLY_FindBoundary);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-refine-midpoints",
    //
    "poly-refine-midpoints [-minarea <minarea>] [-planar]\n"
    "\t Applies midpoint refinement to each triangle.",
    //
    __FILE__, group, MOBIUS_POLY_RefineMidpoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-refine-midedges",
    //
    "poly-refine-midedges [-minarea <minarea>] [-minlen <minlen>] [-planar]\n"
    "\n"
    "\t Refines the input triangles by midedges.",
    //
    __FILE__, group, MOBIUS_POLY_RefineByMidedges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-collapse-edge",
    //
    "poly-collapse-edge [-force]\n"
    "\n"
    "\t Collapses the edge between the two selected triangles. If the '-force' flag\n"
    "\t is passed, no validity checks are performed on edge collapse.",
    //
    __FILE__, group, MOBIUS_POLY_CollapseEdge);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-collapse-edges",
    //
    "poly-collapse-edges -maxlen <maxlen> [-planar]\n"
    "\n"
    "\t Collapses tiny edges incrementally.",
    //
    __FILE__, group, MOBIUS_POLY_CollapseEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-smooth",
    //
    "poly-smooth [-iter <iter>]\n"
    "\t Smooths triangulation.",
    //
    __FILE__, group, MOBIUS_POLY_Smooth);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-refine-inc",
    //
    "poly-refine-inc [-fine]\n"
    "\t Incrementally refines the named triangulation.",
    //
    __FILE__, group, MOBIUS_POLY_RefineInc);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-check",
    //
    "poly-check\n"
    "\t Checks the basic metrics on the selected triangles, such as area, scaled Jacobian, etc.",
    //
    __FILE__, group, MOBIUS_POLY_Check);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-split-edge",
    //
    "poly-split-edge\n"
    "\n"
    "\t Splits the edge between the two selected triangles.",
    //
    __FILE__, group, MOBIUS_POLY_SplitEdge);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-split-boundary",
    //
    "poly-split-boundary [-draw]\n"
    "\n"
    "\t Splits boundary triangles.",
    //
    __FILE__, group, MOBIUS_POLY_SplitBoundary);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-find-domain-edges",
    //
    "poly-domain-edges -domain <id> [-draw]\n"
    "\n"
    "\t Finds edges belonging to the given domain.",
    //
    __FILE__, group, MOBIUS_POLY_FindDomainEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-check-domain-inter",
    //
    "poly-check-domain-inter -domain <id>\n"
    "\n"
    "\t Checks self-intersections in the given domain.",
    //
    __FILE__, group, MOBIUS_POLY_CheckDomainInter);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdMobius)
