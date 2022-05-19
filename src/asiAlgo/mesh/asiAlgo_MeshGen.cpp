//-----------------------------------------------------------------------------
// Created on: 15 February 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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
#include <asiAlgo_MeshGen.h>

// asiAlgo includes
#include <asiAlgo_FacetQuality.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <OSD.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Standard_ErrorHandler.hxx>

#undef FILE_DEBUG
#if defined FILE_DEBUG
  #pragma message("===== warning: FILE_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

bool asiAlgo_MeshGen::AutoSelectLinearDeflection(const TopoDS_Shape& shape,
                                                 double&             defl,
                                                 const double        linPrec)
{
  bool isOk = true;
  double xMin, yMix, zMin, xMax, yMax, zMax;
  //
  if ( !asiAlgo_Utils::Bounds(shape, xMin, yMix, zMin, xMax, yMax, zMax, 0.0001, false) )
  {
    xMin = yMix = zMin = 0.0;
    xMax = yMax = zMax = 1.0;
    isOk = false;
  }

  // Use a fraction of a bounding diagonal.
  const double diag = ( gp_XYZ(xMin, yMix, zMin) - gp_XYZ(xMax, yMax, zMax) ).Modulus();
  defl = linPrec*diag;
  return isOk;
}

//-----------------------------------------------------------------------------

double asiAlgo_MeshGen::AutoSelectLinearDeflection(const TopoDS_Shape& shape)
{
  double linDefl = asiAlgo_LINDEFL_MIN;
  AutoSelectLinearDeflection(shape, linDefl);
  return linDefl;
}

//-----------------------------------------------------------------------------

double asiAlgo_MeshGen::AutoSelectAngularDeflection(const TopoDS_Shape& asiAlgo_NotUsed(shape))
{
  return 0.5; // In degrees.
}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshGen::DoNative(const TopoDS_Shape& shape,
                               const double        linearDeflection,
                               const double        angularDeflection_deg,
                               asiAlgo_MeshInfo&   info)
{
  // Clean up polygonal data.
  BRepTools::Clean(shape);

  // Tessellate.
  try
  {
    OCC_CATCH_SIGNALS

#if defined FILE_DEBUG
    static int callId = 0;
    callId++;

    std::cout << "Running b-rep mesh: " << callId << std::endl;
    TCollection_AsciiString filename("C:/users/ssv/desktop/tmp/");
    filename += callId;
    filename += ".brep";
    //
    BRepTools::Write(shape, filename.ToCString());
#endif

    // Notice that parallel mode is enabled.
    IMeshTools_Parameters params;
    params.Deflection = linearDeflection;
    params.Angle      = angularDeflection_deg;
    params.InParallel = true;
    //
    BRepMesh_IncrementalMesh MeshGen(shape, params);
  }
  catch ( ... )
  {
    std::cout << "B-Rep mesh crash" << std::endl;
    return false;
  }

  //---------------------------------------------------------------------------
  // Accumulate summary info
  //---------------------------------------------------------------------------

  double maxDeflection = 0.0;
  int    nTriangles    = 0;
  int    nNodes        = 0;
  //
  for ( TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next() )
  {
    const TopoDS_Face&                F = TopoDS::Face( ex.Current() );
    TopLoc_Location                   L;
    const Handle(Poly_Triangulation)& T = BRep_Tool::Triangulation(F, L);
    //
    if ( T.IsNull() )
      continue;

    nTriangles += T->NbTriangles();
    nNodes     += T->NbNodes();
    //
    if ( T->Deflection() > maxDeflection )
      maxDeflection = T->Deflection();
  }

  // Store results
  info.maxDeflection = maxDeflection;
  info.nFacets       = nTriangles;
  info.nNodes        = nNodes;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshGen::DoNative(const TopoDS_Shape& shape)
{
  const double linDefl = AutoSelectLinearDeflection(shape);
  const double angDefl = AutoSelectAngularDeflection(shape);
  //
  asiAlgo_MeshInfo info;

  return DoNative(shape, linDefl, angDefl, info);
}

//-----------------------------------------------------------------------------

#if defined USE_NETGEN
  #pragma warning(push,0)

  #ifndef OCCGEOMETRY
  #define OCCGEOMETRY
  #endif

  #define NO_PARALLEL_THREADS

  #include <occgeom.hpp>

  namespace nglib
  {
  #include "nglib.h"
  }

  #pragma warning(pop)
#endif

bool asiAlgo_MeshGen::DoNetGen(const TopoDS_Shape&         shape,
                               Handle(Poly_Triangulation)& mesh,
                               ActAPI_ProgressEntry        progress)
{
#if defined USE_NETGEN
  const double linDefl = AutoSelectLinearDeflection(shape);
  const double minh    = linDefl*0.05;
  const double maxh    = linDefl*5.5;
  const double grading = 0.8;

  std::unordered_map<int, std::unordered_set<int>> faceElems;

  return DoNetGen(shape, minh, maxh, grading, mesh, faceElems, progress);
#else
  (void) shape;
  (void) mesh;

  progress.SendLogMessage(LogErr(Normal) << "NetGen is not available. Consider turning on the USE_NETGEN cmake flag.");
  return false;
#endif
}

bool asiAlgo_MeshGen::DoNetGen(const TopoDS_Shape&                               shape,
                               const double                                      minh,
                               const double                                      maxh,
                               const double                                      grading,
                               Handle(Poly_Triangulation)&                       mesh,
                               std::unordered_map<int, std::unordered_set<int>>& faceElems,
                               ActAPI_ProgressEntry                              progress)
{
#if defined USE_NETGEN
  TopoDS_Shape sh = shape;

  const double linDefl = AutoSelectLinearDeflection(shape);

  netgen::MeshingParameters ngParam;
  ngParam.minh        = minh;
  ngParam.maxh        = maxh;
  ngParam.uselocalh   = true;
  ngParam.secondorder = false;
  ngParam.grading     = grading;

  netgen::OCCParameters occParam;

  nglib::Ng_Init();

  static netgen::Mesh ngMesh;

  netgen::OCCGeometry geom;
  geom.shape = sh;
  geom.BuildFMap();
  geom.BuildVisualizationMesh(linDefl);
  geom.CalcBoundingBox();
  geom.changed = 1;
  geom.PrintNrShapes();

  netgen::OCCSetLocalMeshSize (geom, ngMesh, ngParam, occParam);
  netgen::OCCFindEdges        (geom, ngMesh, ngParam);
  netgen::OCCMeshSurface      (geom, ngMesh, ngParam);

  const int nbNodes     = (int) ngMesh.GetNP();
  const int nbTriangles = (int) ngMesh.GetNSE();

  progress.SendLogMessage(LogNotice(Normal) << "Num. of mesh nodes     generated: %1." << nbNodes);
  progress.SendLogMessage(LogNotice(Normal) << "Num. of mesh triangles generated: %1." << nbTriangles);

  std::cout << "Num. of mesh nodes     generated: " << nbNodes << std::endl;
  std::cout << "Num. of mesh triangles generated: " << nbTriangles << std::endl;

  if ( !nbNodes || !nbTriangles )
  {
    nglib::Ng_Exit();
    return false;
  }

  // Populate the result.
  mesh = new Poly_Triangulation(nbNodes, nbTriangles, false);
  //
  for ( int i = 1; i <= nbNodes; ++i )
  {
    const netgen::MeshPoint& point = ngMesh.Point(netgen::PointIndex(i));
    mesh->ChangeNode(i).SetCoord(point[0], point[1], point[2]);
  }

  for ( int i = 1; i <= nbTriangles; ++i )
  {
    const netgen::Element2d& elem = ngMesh.SurfaceElement(netgen::ElementIndex(i));
    mesh->ChangeTriangle(i).Set(elem[0], elem[1], elem[2]);
  }

  std::cout << "Mesh was generated." << std::endl;

  for ( int fidx = 1; fidx <= geom.fmap.Extent(); ++fidx )
  {
    ngcore::Array<netgen::SurfaceElementIndex> elemIds;
    ngMesh.GetSurfaceElementsOfFace(fidx, elemIds);

    // Collect element IDs.
    std::unordered_set<int> eids;
    //
    for ( const auto& elem : elemIds )
    {
      eids.insert(elem);
    }

    faceElems.insert({fidx, eids});
  }

  ngMesh.DeleteMesh();

  nglib::Ng_Exit();
  return true;
#else
  (void) shape;
  (void) minh;
  (void) maxh;
  (void) grading;
  (void) mesh;
  (void) faceElems;

  progress.SendLogMessage(LogErr(Normal) << "NetGen is not available. Consider turning on the USE_NETGEN cmake flag.");
  return false;
#endif
}
