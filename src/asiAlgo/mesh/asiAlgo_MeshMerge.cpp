//-----------------------------------------------------------------------------
// Created on: 23 May 2016
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
#include <asiAlgo_MeshMerge.h>

// asiAlgo includes
#include <asiAlgo_MeshComputeShapeNorms.h>

// Mobius includes
#if defined USE_MOBIUS
#include <mobius/cascade.h>
#endif

// OCCT includes
#include <BRep_Tool.hxx>
#include <NCollection_CellFilter.hxx>
#include <Poly_CoherentTriangulation.hxx>
#include <Precision.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#if defined USE_MOBIUS
using namespace mobius;
#endif

namespace
{
  //---------------------------------------------------------------------------
  // Inspectors for spatial nodes
  //---------------------------------------------------------------------------

  //! Spatial point enriched with numeric identifier.
  struct TriNode
  {
    gp_XYZ Point; //!< Geometric representation.
    int    ID;    //!< Associated ID.
  };

  //! Auxiliary class to search for coincident spatial points.
  class InspectXYZ : public NCollection_CellFilter_InspectorXYZ
  {
  public:

    typedef gp_XYZ Target;

    //! Constructor accepting resolution distance and point.
    InspectXYZ(const double tol, const gp_XYZ& P) : m_fTol(tol), m_bFound(false), m_P(P) {}

    //! \return true/false depending on whether the node was found or not.
    bool IsFound() const { return m_bFound; }

    //! Implementation of inspection method.
    NCollection_CellFilter_Action Inspect(const gp_XYZ& Target)
    {
      m_bFound = ( (m_P - Target).SquareModulus() <= Square(m_fTol) );
      return CellFilter_Keep;
    }

  private:

    double m_fTol;   //!< Resolution to check for coincidence.
    bool   m_bFound; //!< Whether two points are coincident or not.
    gp_XYZ m_P;      //!< Source point.

  };

  //! Auxiliary class to search for coincident tessellation nodes.
  class InspectNode : public InspectXYZ
  {
  public:

    typedef TriNode Target;

    //! Constructor accepting resolution distance and point.
    InspectNode(const double tol, const gp_XYZ& P) : InspectXYZ(tol, P), m_iID(-1) {}

    int GetID() const { return m_iID; }

    //! Implementation of inspection method.
    NCollection_CellFilter_Action Inspect(const TriNode& Target)
    {
      InspectXYZ::Inspect(Target.Point);

      if ( InspectXYZ::IsFound() )
        m_iID = Target.ID;

      return CellFilter_Keep;
    }

  private:

    int m_iID; //!< Found target ID.

  };

  //---------------------------------------------------------------------------
  // Auxiliary functions
  //---------------------------------------------------------------------------

  void appendNodeInGlobalTri(const int                            localNodeId,
                             int&                                 globalNodeId,
                             const gp_XYZ&                        xyz,
                             Handle(Poly_CoherentTriangulation)&  GlobalTri,
                             NCollection_CellFilter<InspectNode>& NodeFilter,
                             NCollection_DataMap<int, int>&       LocGlobMap)
  {
    const double prec = Precision::Confusion();
    InspectNode Inspect(prec, xyz);
    gp_XYZ XYZ_min = Inspect.Shift( xyz, -prec );
    gp_XYZ XYZ_max = Inspect.Shift( xyz,  prec );

    // Coincidence test
    NodeFilter.Inspect(XYZ_min, XYZ_max, Inspect);
    const bool isFound = Inspect.IsFound();
    //
    if ( !isFound )
    {
      TriNode N;
      N.ID    = globalNodeId;
      N.Point = xyz;
      //
      NodeFilter.Add(N, xyz);
      GlobalTri->SetNode(xyz, globalNodeId);
      LocGlobMap.Bind(localNodeId, globalNodeId);

      // (!!!) Increment global node ID
      ++globalNodeId;
    }
    else
    {
      const int equalID = Inspect.GetID();
      LocGlobMap.Bind(localNodeId, equalID);
    }
  }

#if defined USE_MOBIUS
  void appendNodeInGlobalTri(const int                            localNodeId,
                             int&                                 globalNodeId,
                             const gp_XYZ&                        xyz,
                             t_ptr<t_mesh>&                       GlobalTri,
                             NCollection_CellFilter<InspectNode>& NodeFilter,
                             NCollection_DataMap<int, int>&       LocGlobMap)
  {
    const double prec = Precision::Confusion();
    InspectNode Inspect(prec, xyz);
    gp_XYZ XYZ_min = Inspect.Shift( xyz, -prec );
    gp_XYZ XYZ_max = Inspect.Shift( xyz,  prec );

    // Coincidence test
    NodeFilter.Inspect(XYZ_min, XYZ_max, Inspect);
    const bool isFound = Inspect.IsFound();
    //
    if ( !isFound )
    {
      TriNode N;
      N.ID    = globalNodeId;
      N.Point = xyz;
      //
      NodeFilter.Add(N, xyz);
      GlobalTri->AddVertex( cascade::GetMobiusPnt(xyz) );
      LocGlobMap.Bind(localNodeId, globalNodeId);

      // (!!!) Increment global node ID
      ++globalNodeId;
    }
    else
    {
      const int equalID = Inspect.GetID();
      LocGlobMap.Bind(localNodeId, equalID);
    }
  }
#endif

  Handle(Poly_Triangulation) TriangulationFromFace(const TopoDS_Face& F)
  {
    // Extract triangulation of a patch
    TopLoc_Location Loc;
    const Handle(Poly_Triangulation)& LocalTri = BRep_Tool::Triangulation(F, Loc);
    //
    if ( LocalTri.IsNull() )
      return nullptr;
    //
    const int nNodes     = LocalTri->NbNodes();
    const int nTriangles = LocalTri->NbTriangles();

    Handle(Poly_Triangulation)
      result = new Poly_Triangulation(nNodes, nTriangles, false);

    // Add nodes with the applied transformation.
    for ( int localNodeId = 1; localNodeId <= nNodes; ++localNodeId )
    {
      gp_XYZ xyz;
      if ( Loc.IsIdentity() )
        xyz = LocalTri->Node(localNodeId).XYZ();
      else
        xyz = LocalTri->Node(localNodeId).Transformed( Loc.Transformation() ).XYZ();

      result->SetNode(localNodeId, xyz);
    }

    // Add triangles taking into account face orientation.
    for ( int i = 1; i <= nTriangles; ++i )
    {
      int n1, n2, n3;
      LocalTri->Triangle(i).Get(n1, n2, n3);
      int m[3] = {n1, n2, n3};

      if ( F.Orientation() == TopAbs_REVERSED )
      {
        m[1] = n3;
        m[2] = n2;
      }

      result->SetTriangle(i, Poly_Triangle(m[0], m[1], m[2]));
    }

    // Build normals.
    asiAlgo_MeshComputeShapeNorms::ComputeNormals(F, result);

    return result;
  }

}

//-----------------------------------------------------------------------------

Handle(Poly_Triangulation)
  asiAlgo_MeshMerge::PutTogether(const std::vector<Handle(Poly_Triangulation)>& tris)
{
  // The first pass is to check how many nodes and triangles it's going to be.
  int nodeCount = 0;
  int triCount  = 0;
  //
  for ( const auto& T : tris )
  {
    nodeCount += T->NbNodes();
    triCount  += T->NbTriangles();
  }
  //
  if ( !nodeCount || !triCount )
    return nullptr;

  Handle(Poly_Triangulation)
    result = new Poly_Triangulation(nodeCount, triCount, false);
  //
  result->AddNormals();

  // The second pass is to compose the united triangulation.
  int globalNodeIdx = 1;
  int globalTriangleIdx = 1;
  //
  for ( const auto& T : tris )
  {
    NCollection_DataMap<int, int> nodeMapping;

    // Pass nodes.
    Handle(TColgp_HArray1OfPnt) nodes = T->MapNodeArray();
    //
    for ( int pidx = nodes->Lower(); pidx <= nodes->Upper(); ++pidx )
    {
      result->SetNode(globalNodeIdx, nodes->Value(pidx));

      if ( T->HasNormals() )
        result->SetNormal( globalNodeIdx, T->Normal(pidx) );

      nodeMapping.Bind(pidx, globalNodeIdx++);
    }

    // Pass triangles.
    Handle(Poly_HArray1OfTriangle) TT = T->MapTriangleArray();
    //
    for ( int tidx = TT->Lower(); tidx <= TT->Upper(); ++tidx )
    {
      int nids[3];
      TT->Value(tidx).Get(nids[0], nids[1], nids[2]);
      int nnids[3] = {nodeMapping(nids[0]), nodeMapping(nids[1]), nodeMapping(nids[2])};

      result->SetTriangle(globalTriangleIdx++, Poly_Triangle(nnids[0], nnids[1], nnids[2]));
    }
  }

  return result;
}

//-----------------------------------------------------------------------------

Handle(Poly_Triangulation)
  asiAlgo_MeshMerge::PutTogether(const TopoDS_Shape& shape,
                                 t_faceElems&        history)
{
  std::vector<Handle(Poly_Triangulation)> tris;
  int triangleIndex = 0;

  for ( TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next() )
  {
    const TopoDS_Face& F = TopoDS::Face( exp.Current() );

    // Add for processing.
    Handle(Poly_Triangulation) FT = ::TriangulationFromFace(F);
    //
    if ( FT.IsNull() )
      continue;

    tris.push_back(FT);

    // Add mapping of indices.
    NCollection_Vector<int>* mapPtr = history.ChangeSeek(F);
    if ( mapPtr == nullptr )
      mapPtr = history.Bound( F, NCollection_Vector<int>() );
    //
    for ( int tidx = 0; tidx < FT->NbTriangles(); ++tidx )
    {
      (*mapPtr).Append(++triangleIndex);
    }
  }

  return PutTogether(tris);
}

//-----------------------------------------------------------------------------
// Conglomeration tool
//-----------------------------------------------------------------------------

asiAlgo_MeshMerge::asiAlgo_MeshMerge(const TopoDS_Shape& body,
                                     const Mode          mode,
                                     const bool          storeFaceIds)
{
  this->build(body, mode, storeFaceIds);
}

//-----------------------------------------------------------------------------

asiAlgo_MeshMerge::asiAlgo_MeshMerge(const std::vector<Handle(Poly_Triangulation)>& triangulations,
                                     const Mode                                     mode)
{
  this->build(triangulations, mode);
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshMerge::build(const TopoDS_Shape& body,
                              const Mode          mode,
                              const bool          storeFaceIds)
{
  // Working tools and variables
  int globalNodeId = 0;
  NCollection_CellFilter<InspectNode> NodeFilter( Precision::Confusion() );

  /* Mobius data structure */
  if ( mode == Mode_MobiusMesh )
  {
#if defined USE_MOBIUS
    m_resultMobMesh = new t_mesh;

    // [BEGIN] Iterate over the faces
    int faceId = 0;
    //
    for ( TopExp_Explorer exp(body, TopAbs_FACE); exp.More(); exp.Next() )
    {
      faceId++;
      const TopoDS_Face& F = TopoDS::Face( exp.Current() );
      NCollection_DataMap<int, int> FaceNodeIds_ToGlobalNodeIds;

      // Extract triangulation of a patch
      TopLoc_Location Loc;
      const Handle(Poly_Triangulation)& LocalTri = BRep_Tool::Triangulation(F, Loc);
      //
      if ( LocalTri.IsNull() )
        continue;
      //
      const int nLocalNodes     = LocalTri->NbNodes();
      const int nLocalTriangles = LocalTri->NbTriangles();

      // Add nodes with coincidence test
      for ( int localNodeId = 1; localNodeId <= nLocalNodes; ++localNodeId )
      {
        gp_XYZ xyz;
        if ( Loc.IsIdentity() )
          xyz = LocalTri->Node(localNodeId).XYZ();
        else
          xyz = LocalTri->Node(localNodeId).Transformed( Loc.Transformation() ).XYZ();

        // Add node to the conglomerate after coincidence test
        ::appendNodeInGlobalTri(localNodeId,                  // [in]     local node ID in a face
                                globalNodeId,                 // [in,out] global node ID in the conglomerate mesh
                                xyz,                          // [in]     coordinates for coincidence test
                                m_resultMobMesh,              // [in,out] result conglomerate mesh
                                NodeFilter,                   // [in]     cell filter
                                FaceNodeIds_ToGlobalNodeIds); // [in,out] face-conglomerate map of node IDs
      }

      // Add triangles taking into account face orientations
      for ( int i = 1; i <= nLocalTriangles; ++i )
      {
        int n1, n2, n3;
        LocalTri->Triangle(i).Get(n1, n2, n3);
        int m[3] = {n1, n2, n3};

        if ( F.Orientation() == TopAbs_REVERSED )
        {
          m[1] = n3;
          m[2] = n2;
        }

        m[0] = FaceNodeIds_ToGlobalNodeIds.Find(m[0]);
        m[1] = FaceNodeIds_ToGlobalNodeIds.Find(m[1]);
        m[2] = FaceNodeIds_ToGlobalNodeIds.Find(m[2]);

        if ( m[0] == m[1] || m[0] == m[2] || m[1] == m[2] )
          continue;

        m_resultMobMesh->AddTriangle( poly_VertexHandle(m[0]),
                                      poly_VertexHandle(m[1]),
                                      poly_VertexHandle(m[2]),
                                      storeFaceIds ? faceId : -1 );
      }
    }
    // [END] Iterate over the faces
#endif
  }
  /* OpenCascade-based data structures */
  else
  {
    // Create result as coherent triangulation
    m_resultPoly = new Poly_CoherentTriangulation;

    // [BEGIN] Iterate over the faces
    int triangleIndex = 1;
    //
    for ( TopExp_Explorer exp(body, TopAbs_FACE); exp.More(); exp.Next() )
    {
      const TopoDS_Face& F = TopoDS::Face( exp.Current() );
      NCollection_DataMap<int, int> FaceNodeIds_ToGlobalNodeIds;

      // Extract triangulation of a patch
      TopLoc_Location Loc;
      const Handle(Poly_Triangulation)& LocalTri = BRep_Tool::Triangulation(F, Loc);
      //
      if ( LocalTri.IsNull() )
        continue;
      //
      const int nLocalNodes     = LocalTri->NbNodes();
      const int nLocalTriangles = LocalTri->NbTriangles();

      // Add nodes with coincidence test
      for ( int localNodeId = 1; localNodeId <= nLocalNodes; ++localNodeId )
      {
        gp_XYZ xyz;
        if ( Loc.IsIdentity() )
          xyz = LocalTri->Node(localNodeId).XYZ();
        else
          xyz = LocalTri->Node(localNodeId).Transformed( Loc.Transformation() ).XYZ();

        // Add node to the conglomerate after coincidence test
        ::appendNodeInGlobalTri(localNodeId,                  // [in]     local node ID in a face
                                globalNodeId,                 // [in,out] global node ID in the conglomerate mesh
                                xyz,                          // [in]     coordinates for coincidence test
                                m_resultPoly,                 // [in,out] result conglomerate mesh
                                NodeFilter,                   // [in]     cell filter
                                FaceNodeIds_ToGlobalNodeIds); // [in,out] face-conglomerate map of node IDs
      }

      // Add triangles taking into account face orientations
      for ( int i = 1; i <= nLocalTriangles; ++i )
      {
        int n1, n2, n3;
        LocalTri->Triangle(i).Get(n1, n2, n3);
        int m[3] = {n1, n2, n3};

        if ( F.Orientation() == TopAbs_REVERSED )
        {
          m[1] = n3;
          m[2] = n2;
        }

        m[0] = FaceNodeIds_ToGlobalNodeIds.Find(m[0]);
        m[1] = FaceNodeIds_ToGlobalNodeIds.Find(m[1]);
        m[2] = FaceNodeIds_ToGlobalNodeIds.Find(m[2]);

        if ( m[0] == m[1] || m[0] == m[2] || m[1] == m[2] )
          continue;

        m_resultPoly->AddTriangle(m[0], m[1], m[2]);

        // Add mapping of indices.
        NCollection_Vector<int>* mapPtr = m_faceElems.ChangeSeek(F);
        if ( mapPtr == nullptr )
          mapPtr = m_faceElems.Bound(F, NCollection_Vector<int>());
        //
        (*mapPtr).Append(triangleIndex);
        ++triangleIndex;
      }
    }
    // [END] Iterate over the faces

    if ( mode == Mode_Mesh )
      if ( !m_resultPoly->GetTriangulation().IsNull() )
        m_resultMesh = new ActData_Mesh( m_resultPoly->GetTriangulation() );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_MeshMerge::build(const std::vector<Handle(Poly_Triangulation)>& triangulations,
                              const Mode                                     mode)
{
  // Create result as coherent triangulation
  m_resultPoly = new Poly_CoherentTriangulation;

  // Working tools and variables
  int globalNodeId = 0;
  NCollection_CellFilter<InspectNode> NodeFilter( Precision::Confusion() );

  //###########################################################################
  // [BEGIN] Iterate over the faces
  for ( size_t tidx = 0; tidx < triangulations.size(); ++tidx )
  {
    const Handle(Poly_Triangulation)& LocalTri = triangulations[tidx];
    //
    if ( LocalTri.IsNull() )
      continue;
    //
    const int nLocalNodes     = LocalTri->NbNodes();
    const int nLocalTriangles = LocalTri->NbTriangles();
    //
    NCollection_DataMap<int, int> FaceNodeIds_ToGlobalNodeIds;

    // Add nodes with coincidence test
    for ( int localNodeId = 1; localNodeId <= nLocalNodes; ++localNodeId )
    {
      gp_XYZ xyz = LocalTri->Node(localNodeId).XYZ();

      // Add node to the conglomerate after coincidence test
      ::appendNodeInGlobalTri(localNodeId,                  // [in]     local node ID in a face
                              globalNodeId,                 // [in,out] global node ID in the conglomerate mesh
                              xyz,                          // [in]     coordinates for coincidence test
                              m_resultPoly,                 // [in,out] result conglomerate mesh
                              NodeFilter,                   // [in]     cell filter
                              FaceNodeIds_ToGlobalNodeIds); // [in,out] face-conglomerate map of node IDs
    }

    // Add triangles to the result
    for ( int i = 1; i <= nLocalTriangles; ++i )
    {
      int n1, n2, n3;
      LocalTri->Triangle(i).Get(n1, n2, n3);
      int m[3] = {n1, n2, n3};

      m[0] = FaceNodeIds_ToGlobalNodeIds.Find(m[0]);
      m[1] = FaceNodeIds_ToGlobalNodeIds.Find(m[1]);
      m[2] = FaceNodeIds_ToGlobalNodeIds.Find(m[2]);

      if ( m[0] == m[1] || m[0] == m[2] || m[1] == m[2] )
        continue;

      m_resultPoly->AddTriangle(m[0], m[1], m[2]);
    }
  }
  // [END] Iterate over the patches
  //###########################################################################

  if ( mode == Mode_Mesh )
    if ( !m_resultPoly->GetTriangulation().IsNull() )
      m_resultMesh = new ActData_Mesh( m_resultPoly->GetTriangulation() );
}
