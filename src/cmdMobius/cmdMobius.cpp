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
    cf->ViewerPart->Repaint();
    cf->ViewerHost->Repaint();
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

  // Set as Tcl variable.
  std::string name;
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->SetVar( name, new cmdMobius_Mesh(mesh) );
  }

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles: %1."
                                                        << mesh->GetNumTriangles() );

  // Get triangulation node.
  Handle(asiData_TriangulationNode) tris_n = cmdMobius::model->GetTriangulationNode();

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
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
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(tris);
  }

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
    norms_n = asiEngine_Tessellation(cmdMobius::model).CreateNorms(tris_n,
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
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Check if there's any user selection to process.
  TColStd_PackedMapOfInteger facetIds;
  trisApi.GetHighlightedFacets(facetIds);
  //
  if ( facetIds.Extent() )
  {
    tris = asiAlgo_Utils::Mesh::ExtractRegion(tris, facetIds);
  }

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(tris);
  }

  TIMER_NEW
  TIMER_GO

  // Compute links.
  mesh->ComputeEdges();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Compute links")

  TIMER_RESET
  TIMER_GO

  // Flip edges.
  mesh->FlipEdges(1./180.*M_PI, 5./180.*M_PI);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Flip edges")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles: %1."
                                                        << mesh->GetNumTriangles() );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
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
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

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

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(tris);
  }

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
    const int fid = fit.Key() - 1; // Mobius indices are 0-based.
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
      foundIds.Add(_th.iIdx + 1); // OpenCascade triangles are 1-based.
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

int MOBIUS_POLY_RefineByMidedges(const Handle(asiTcl_Interp)& interp,
                                 int                          argc,
                                 const char**                 argv)
{
#if defined USE_MOBIUS
  // Get triangulation.
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(tris);
  }

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

    double areaThreshold = 1.;
    double lenThreshold  = 1.;
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
      int numTris = mesh->GetNumTriangles();

      // Compute areas.
      std::vector<poly_TriangleHandle> ths;
      std::vector<double>              tAreas;
      std::vector<int>                 tNums;
      //
      for ( int idx = 0; idx < numTris; ++idx )
      {
        poly_TriangleHandle th(idx);

        tAreas.push_back( mesh->ComputeArea(th) );
        tNums .push_back( idx );
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
        poly_TriangleHandle th(idx);
        poly_Triangle       t;

        // Get the next triangle to process.
        mesh->GetTriangle(th, t);
        //
        if ( t.IsDeleted() )
          continue;

        // Refine triangle based on its size.
        const double area = mesh->ComputeArea(th);
        const double len  = mesh->ComputeMaxLen(th);
        //
        if ( (area > areaThreshold) || (len > lenThreshold) )
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
      const int fid = fit.Key() - 1; // Mobius indices are 0-based.

      std::vector<poly_TriangleHandle> ths;
      if ( !mesh->RefineByMidedges(poly_TriangleHandle(fid), ths) )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot refine by midedges.");
        return TCL_ERROR;
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
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
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
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(tris);
  }

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

  poly_TriangleHandle th[2] = { poly_TriangleHandle(facetIds.GetMinimalMapped() - 1),
                                poly_TriangleHandle(facetIds.GetMaximalMapped() - 1) };

  poly_EdgeHandle he = mesh->FindEdge(th[0], th[1]);
  //
  if ( !he.IsValid() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The selected triangle have no common edge.");
    return TCL_ERROR;
  }

  const bool force = interp->HasKeyword(argc, argv, "force");
  bool       isOk  = false;

  // Collapse the common edge.
  if ( force )
    isOk = mesh->CollapseEdge(he);
  else
    isOk = mesh->CollapseEdge(he, true, true, 0.01);

  if ( !isOk )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot collapse the edge.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Collapse edge")

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles after edge collapse: %1."
                                                        << mesh->GetNumTriangles() );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
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
  //
  Handle(Poly_Triangulation)
    tris = tris_n->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }

  double maxLen = 1.;
  //
  interp->GetKeyValue(argc, argv, "maxlen", maxLen);

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(tris);
  }

  TIMER_NEW
  TIMER_GO

  mesh->ComputeEdges();

  const int maxIter = 10;
  bool      stop    = false;
  int       iter    = 0;

  // Refine.
  do
  {
    int  numEdges   = mesh->GetNumEdges();
    bool anyRefined = false;
    //
    for ( int idx = 0; idx < numEdges; ++idx )
    {
      poly_EdgeHandle eh(idx);
      poly_Edge       e;

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
        mesh->CollapseEdge(eh, true, true, 0.01);

        if ( !anyRefined ) anyRefined = true;
      }
    }

    if ( !anyRefined || (++iter >= maxIter) )
      stop = true;
  }
  while ( !stop );

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Collapse edges")

  ///

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. of triangles after edge collapse: %1."
                                                        << mesh->GetNumTriangles() );

  // Update data model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
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

int MOBIUS_POLY_RefineMidpoints(const Handle(asiTcl_Interp)& interp,
                                int                          argc,
                                const char**                 argv)
{
#if defined USE_MOBIUS

  // Get mesh from the Triangulation Node.
  Handle(Poly_Triangulation)
    poly = cmdMobius::model->GetTriangulationNode()->GetTriangulation();
  //
  if ( poly.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is empty.");
    return TCL_ERROR;
  }

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(poly);
  }

  TIMER_NEW
  TIMER_GO

  const int numTris = mesh->GetNumTriangles();

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "%1 triangles to refine."
                                                       << numTris);

  double areaThreshold = 1.;
  double lenThreshold  = 1.;
  //
  interp->GetKeyValue(argc, argv, "minarea", areaThreshold);

  // Compute areas.
  std::vector<poly_TriangleHandle> ths;
  std::vector<double>              tAreas;
  std::vector<int>                 tNums;
  //
  for ( int idx = 0; idx < numTris; ++idx )
  {
    poly_TriangleHandle th(idx);

    tAreas.push_back( mesh->ComputeArea(th) );
    tNums .push_back( idx );
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
    poly_TriangleHandle th(idx);
    poly_Triangle       t;

    // Get the next triangle to process.
    mesh->GetTriangle(th, t);
    //
    if ( t.IsDeleted() )
      continue;

    // Refine triangle based on its size.
    const double area = mesh->ComputeArea(th);
    const double len  = mesh->ComputeMaxLen(th);
    //
    if ( (area > areaThreshold) && (len > lenThreshold) )
    {
      mesh->RefineByMidpoint( poly_TriangleHandle(idx) );
    }
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Midpoint refine")

  // Update Data Model.
  cmdMobius::model->OpenCommand();
  {
    cmdMobius::model->GetTriangulationNode()->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
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

int MOBIUS_POLY_Smooth(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  // Get mesh from the Triangulation Node.
  Handle(Poly_Triangulation) poly = tris_n->GetTriangulation();
  //
  if ( poly.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is empty.");
    return TCL_ERROR;
  }

  // Get the named mesh.
  t_ptr<t_mesh> mesh;
  std::string   name;
  //
  if ( interp->GetKeyValue(argc, argv, "model", name) )
  {
    // Get the named mesh.
    Handle(asiTcl_Variable) var     = interp->GetVar(name);
    Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
    //
    if ( meshVar.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                          << name);
      return TCL_ERROR;
    }
    //
    mesh = meshVar->GetMesh();
  }
  else
  {
    // Take from the node.
    mesh = cascade::GetMobiusMesh(poly);
  }

  // Compose the domain of planar faces only.
  std::unordered_set<int> planarDomain;
  Handle(asiAlgo_AAG)     aag = cmdMobius::model->GetPartNode()->GetAAG();
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

  TIMER_NEW
  TIMER_GO

  int iter = 1;
  interp->GetKeyValue(argc, argv, "iter", iter);

  const int numTris = poly->NbTriangles();

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "%1 triangles to smooth in %2 iterations."
                                                       << numTris << iter);

  mesh->ComputeEdges();
  mesh->Smooth(iter, planarDomain);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Smooth")

  // Update Data Model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
  }
  cmdMobius::model->CommitCommand();

  // Actualize.
  if ( cmdMobius::cf->ViewerPart )
    cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_RefineInc(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  /* =============
   *  Preparation.
   * ============= */

  Handle(asiData_TriangulationNode)
    tris_n = cmdMobius::model->GetTriangulationNode();

  // Get name of the mesh.
  std::string name;
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh name is not provided.");
    return TCL_ERROR;
  }

  // Get the named mesh.
  Handle(asiTcl_Variable) var     = interp->GetVar(name);
  Handle(cmdMobius_Mesh)  meshVar = Handle(cmdMobius_Mesh)::DownCast(var);
  //
  if ( meshVar.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no mesh named '%1'."
                                                        << name);
    return TCL_ERROR;
  }

  // Get mesh.
  const t_ptr<poly_Mesh>& mesh = meshVar->GetMesh();
  //
  if ( mesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The mesh named '%1' is null."
                                                        << name);
    return TCL_ERROR;
  }

  /* ============
   *  Refinement.
   * ============ */

  TIMER_NEW
  TIMER_GO

  // TODO: NYI

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Incremental refine")

  /* ==============
   *  Finalization.
   * ============== */

  // Update Data Model.
  cmdMobius::model->OpenCommand();
  {
    tris_n->SetTriangulation( cascade::GetOpenCascadeMesh(mesh) );
  }
  cmdMobius::model->CommitCommand();

  // Actualize.
  cmdMobius::cf->ViewerPart->PrsMgr()->Actualize(tris_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int MOBIUS_POLY_CheckJacobian(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
#if defined USE_MOBIUS

  // Get mesh from the Triangulation Node.
  Handle(Poly_Triangulation)
    tris = cmdMobius::model->GetTriangulationNode()->GetTriangulation();
  //
  if ( tris.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is empty.");
    return TCL_ERROR;
  }

  asiEngine_Triangulation trisApi( cmdMobius::model,
                                   cmdMobius::cf->ViewerPart->PrsMgr(),
                                   interp->GetProgress(),
                                   interp->GetPlotter() );

  // Convert to Mobius.
  t_ptr<t_mesh> mesh = cascade::GetMobiusMesh(tris);

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
    const double J = mesh->ComputeScaledJacobian(ht);

    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Scaled Jacobian for the facet %1 is %2."
                                                         << ht.GetIdx() << J);
  }

  return TCL_OK;
#else
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
    "poly-init [-model <M>]\n"
    "\n"
    "\t Initializes a mesh from a CAD part and optionally stores it is a Tcl variable\n"
    "\t named <M>. If the mesh is to stored, it keeps associativity with the originating\n"
    "\t CAD model.",
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
    "poly-flip-edges\n"
    "\n"
    "\t Flips triangulation edges.",
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
  interp->AddCommand("poly-refine-midedges",
    //
    "poly-refine-midedges [-minarea <minarea>] [-minlen <minlen>]\n"
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
    "poly-collapse-edges -maxlen <maxlen>\n"
    "\n"
    "\t Collapses tiny edges incrementally.",
    //
    __FILE__, group, MOBIUS_POLY_CollapseEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-refine-midpoints",
    //
    "poly-refine-midpoints [-minarea <minarea>]\n"
    "\t Applies midpoint refinement to each triangle.",
    //
    __FILE__, group, MOBIUS_POLY_RefineMidpoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-smooth",
    //
    "poly-smooth [-model <name>] [-iter <iter>]\n"
    "\t Smooths triangulation.",
    //
    __FILE__, group, MOBIUS_POLY_Smooth);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-refine-inc",
    //
    "poly-refine-inc -model M\n"
    "\t Incrementally refines the named triangulation.",
    //
    __FILE__, group, MOBIUS_POLY_RefineInc);

  //-------------------------------------------------------------------------//
  interp->AddCommand("poly-check-jacobian",
    //
    "poly-check-jacobian\n"
    "\t Checks the scaled Jacobian for the selected triangles.",
    //
    __FILE__, group, MOBIUS_POLY_CheckJacobian);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdMobius)
