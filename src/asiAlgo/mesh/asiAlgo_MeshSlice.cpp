//-----------------------------------------------------------------------------
// Created on: 17 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
#include <asiAlgo_MeshSlice.h>

// asiAlgo includes
#include <asiAlgo_MeshInfo.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <ElCLib.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <Poly_CoherentTriangulation.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopExp_Explorer.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

// Standard includes
#include <unordered_map>

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

namespace {

  //! Mesh link
  struct t_link
  {
    int n[2];

    t_link() { n[0] = n[1] = 0; }
    t_link(const int _n0, const int _n1) { n[0] = _n0; n[1] = _n1; }
    t_link(const std::initializer_list<int>& init) { n[0] = *init.begin(); n[1] = *(init.end() - 1); }

    struct Hasher
    {
      //! \return hash code for the link.
      static int HashCode(const t_link& link, const int upper)
      {
        int key = link.n[0] + link.n[1];
        key += (key << 10);
        key ^= (key >> 6);
        key += (key << 3);
        key ^= (key >> 11);
        return (key & 0x7fffffff) % upper;
      }

      //! \return true if two links are equal.
      static int IsEqual(const t_link& link0, const t_link& link1)
      {
        return ( (link0.n[0] == link1.n[0]) && (link0.n[1] == link1.n[1]) ) ||
               ( (link0.n[1] == link1.n[0]) && (link0.n[0] == link1.n[1]) );
      }
    };
  };


  //! Computes area of the passed shape.
  double computeArea(const TopoDS_Shape& shape)
  {
    GProp_GProps props;
    BRepGProp::SurfaceProperties(shape, props, 1.0e-2);
    //
    return props.Mass();
  }
}

//-----------------------------------------------------------------------------

asiAlgo_MeshSlice::asiAlgo_MeshSlice(const TopoDS_Shape&  shape,
                                     ActAPI_ProgressEntry progress,
                                     ActAPI_PlotterEntry  plotter)
//
: ActAPI_IAlgorithm (progress, plotter),
  m_shape           (shape),
  m_bMaximize       (false),
  m_tMin            (0.),
  m_tMax            (0.)
{}

//-----------------------------------------------------------------------------

asiAlgo_MeshSlice::asiAlgo_MeshSlice(const Handle(Poly_Triangulation)& tris,
                                     ActAPI_ProgressEntry              progress,
                                     ActAPI_PlotterEntry               plotter)
//
: ActAPI_IAlgorithm (progress, plotter),
  m_bMaximize       (false),
  m_tMin            (0.),
  m_tMax            (0.)
{
  // Create a fictive face where the facets will sit.
  TopoDS_Face fictiveFace;
  BRep_Builder().MakeFace(fictiveFace);
  BRep_Builder().UpdateFace(fictiveFace, tris);
  //
  m_shape = fictiveFace;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshSlice::Perform(const int numSlices)
{
  m_tMin = 0.;
  m_tMax = 0.;

  /* ==============================
   *  Prepare visualization meshes.
   * ============================== */

  // Prepare data structure for all triangles with back refs.
  Handle(Poly_CoherentTriangulation)
    tris = new Poly_CoherentTriangulation;

  // Check if the shape should be meshed beforehand.
  asiAlgo_MeshInfo meshInfo = asiAlgo_MeshInfo::Extract(m_shape);
  //
  if ( !meshInfo.nFacets )
  {
    BRepMesh_IncrementalMesh meshGen(m_shape, 1.0);
  }

  // Add all triangulations from faces to the common collection.
  std::vector<gp_XYZ> allMeshNodes;
  //
  for ( TopExp_Explorer fexp(m_shape, TopAbs_FACE); fexp.More(); fexp.Next() )
  {
    const TopoDS_Face& face = TopoDS::Face( fexp.Current() );

    TopLoc_Location L;
    const Handle(Poly_Triangulation)&
      poly = BRep_Tool::Triangulation(face, L);

    if ( poly.IsNull() )
      continue;

    // Add nodes.
    std::unordered_map<int, int> nodesMap;
    for ( int iNode = 1; iNode <= poly->NbNodes(); ++iNode )
    {
      // Make sure to apply location, e.g., see the effect in /cad/ANC101.brep
      gp_XYZ    P = poly->Node(iNode).Transformed(L).XYZ();
      const int n = tris->SetNode(P);

      // Local to global node index mapping.
      nodesMap.insert({iNode, n});
      allMeshNodes.push_back(P);
    }

    // Add triangles.
    for ( int iTri = 1; iTri <= poly->NbTriangles(); ++iTri )
    {
      const Poly_Triangle& tri = poly->Triangle(iTri);

      int iNodes[3];
      tri.Get(iNodes[0], iNodes[1], iNodes[2]);

      tris->AddTriangle(nodesMap[iNodes[0]], nodesMap[iNodes[1]], nodesMap[iNodes[2]]);
    }
  }

#if defined DRAW_DEBUG
  // Display the triangulation to be sure it's consistent.
  m_plotter.REDRAW_TRIANGULATION("tris", tris->GetTriangulation(), Color_Default, 1.);
#endif

  /* ===============================
   *  Compute the longest dimension.
   * =============================== */

  gp_Ax1 axis;
  gp_XYZ axOrigin;

  if ( !m_axis.has_value() )
  {
    // Get the bounding box.
    Bnd_Box aabb;
    BRepBndLib::Add(m_shape, aabb, true); // Use triangulation.

    // Compute the dimensions.
    gp_XYZ Pmin = aabb.CornerMin().XYZ();
    gp_XYZ Pmax = aabb.CornerMax().XYZ();
    gp_XYZ D    = Pmax - Pmin;
    //
    double dims[3] = { Abs(D.X()), Abs(D.Y()), Abs(D.Z()) };

    axOrigin = Pmin;

    // Construct the axis.
    if ( (dims[0] > dims[1]) && (dims[0] > dims[2]) ) // X
    {
      axis = gp_Ax1( Pmin, gp::DX() );
      m_tMin = 0;
      m_tMax = dims[0];
    }
    if ( (dims[1] > dims[0]) && (dims[1] > dims[2]) ) // Y
    {
      axis = gp_Ax1( Pmin, gp::DY() );
      m_tMin = 0;
      m_tMax = dims[1];
    }
    if ( (dims[2] > dims[0]) && (dims[2] > dims[1]) ) // Z
    {
      axis = gp_Ax1( Pmin, gp::DZ() );
      m_tMin = 0;
      m_tMax = dims[2];
    }
  }
  else // Axis is defined externally.
  {
    axis     = *m_axis;
    axOrigin = axis.Location().XYZ();

    // Get the dimensions by projecting mesh nodes onto the axis.
    double dotMin =  DBL_MAX;
    double dotMax = -DBL_MAX;
    //
    for ( const auto& P : allMeshNodes )
    {
      const double dot = (P - axOrigin).Dot( axis.Direction().XYZ() );

      if ( dot < dotMin )
        dotMin = dot;
      if ( dot > dotMax )
        dotMax = dot;
    }

    m_tMin = dotMin;
    m_tMax = dotMax;
  }

  // Construct a ray line.
  gp_Lin axisLin(axis);

  //vout << BRepBuilderAPI_MakeVertex(Pmin);
  //vout << BRepBuilderAPI_MakeVertex(Pmax);

  /* =================================
   *  Build a stack of slicing planes.
   * ================================= */

  const int    numPlanes = numSlices;
  const double step      = (m_tMax - m_tMin) / (numPlanes + 1);

  std::vector<gp_Pln> planes;
  //
  for ( int i = 0; i < numPlanes; ++i )
  {
    const double ti = m_tMin + step*(i + 1);
    const gp_Pnt Pi = ElCLib::Value(ti, axisLin);

    gp_Pln pln( Pi, axis.Direction() );
    //
    planes.push_back(pln);

    // Diagnostic dump.
    /*const double d = Abs(tMax - tMin)/2;
      vout << BRepBuilderAPI_MakeFace(pln, -d, d, -d, d);*/
  }

  /* =====================================
   *  Build mesh links and intersect them.
   * ===================================== */

  tris->ComputeLinks();

  // <slice : intersection point>
  typedef std::unordered_map<int, gp_XYZ> t_slicePts;

  // Intersect each mesh link.
  NCollection_DataMap<t_link, t_slicePts, t_link::Hasher> linkPts; // Intersection points over the links.
  //
  for ( Poly_CoherentTriangulation::IteratorOfLink lit(tris);
        lit.More(); lit.Next() )
  {
    const Poly_CoherentLink& link = lit.Value();
    //
    const int n[2] = { link.Node(0), link.Node(1) };
    gp_XYZ    V[2] = { tris->Node(n[0]), tris->Node(n[1]) };

    //vout << BRepBuilderAPI_MakeVertex(V[0]) << BRepBuilderAPI_MakeVertex(V[1]);

    double Vt[2] = { (V[0] - axOrigin)*axis.Direction().XYZ(),
                     (V[1] - axOrigin)*axis.Direction().XYZ() };
    bool   rev   = false;
    //
    if ( Vt[1] < Vt[0] )
    {
      std::swap(Vt[0], Vt[1]);
      rev = true;
    }

    const int start = int( (Vt[0] - m_tMin)*numPlanes/(m_tMax - m_tMin) );
    const int end   = int( (Vt[1] - m_tMin)*numPlanes/(m_tMax - m_tMin) );

    for ( int i = start; i <= end; ++i )
    {
      // Position of the slicing plane along the axis.
      const double t = m_tMin + step*(i + 1);

      // The edge should have intersection point.
      if ( t < Vt[0] || t > Vt[1] )
        continue;

      //vout << BRepBuilderAPI_MakeVertex( ElCLib::Value(t, axisLin) );

      // Intersection point on the edge.
      const gp_XYZ edir = V[1] - V[0];
      const double tl   = rev ? Abs(t - Vt[1])/(Vt[1] - Vt[0]) : Abs(t - Vt[0])/(Vt[1] - Vt[0]); // Along link.
      const gp_XYZ p    = V[0] + tl*edir;
      double       pt   = (p - axOrigin)*axis.Direction().XYZ();

      //vout << BRepBuilderAPI_MakeVertex(p);

      if ( (pt > Vt[1]) || (pt < Vt[0]) )
        continue; // Skip out of range intersection points.

      if ( std::isnan(pt) )
        continue;

      // Add new intersection point to the link.
      t_slicePts* pPts = linkPts.ChangeSeek({n[0], n[1]});
      //
      if ( !pPts )
      {
        t_slicePts slicePts;
        slicePts.insert({i, p});
        linkPts.Bind({n[0], n[1]}, slicePts);
      }
      else
      {
        pPts->insert({i, p});
      }

      //vout << BRepBuilderAPI_MakeVertex(p);
    }
  }

  /* ===============
   *  Connect links.
   * =============== */

  std::vector<Handle(TopTools_HSequenceOfShape)> edgesBySlices;
  std::vector<Handle(TopTools_HSequenceOfShape)> wiresBySlices;

  for ( int i = 0; i < numPlanes; ++i )
  {
    edgesBySlices.push_back(new TopTools_HSequenceOfShape);

    // Loop over triangles.
    for ( Poly_CoherentTriangulation::IteratorOfTriangle tit(tris);
          tit.More(); tit.Next() )
    {
      const Poly_CoherentTriangle& t    = tit.Value();
      const Poly_CoherentLink*     l[3] = { t.GetLink(0), t.GetLink(1), t.GetLink(2) };

      // Get all intersection points for the current triangle.
      std::vector<gp_XYZ> ps;
      //
      for ( int k = 0; k < 3; ++k )
      {
        int               n[2]      = { l[k]->Node(0), l[k]->Node(1) };
        const t_slicePts* pSlicePts = linkPts.Seek( {n[0], n[1]} );
        //
        if ( pSlicePts )
        {
          for ( const auto& slice : *pSlicePts )
          {
            if ( slice.first == i ) // For the current section
            {
              ps.push_back(slice.second);
            }
          }
        }
      }

      if ( ps.size() == 2 )
      {
        if ( (ps[0] - ps[1]).Modulus() > Precision::Confusion() )
        {
          edgesBySlices[i]->Append( BRepBuilderAPI_MakeEdge(ps[0], ps[1]) );
        }
      }
    }

    // Connect edges to wires in the current slice. Notice that there is initially
    // no sharing between the vertices of edges, so the edges will by stitched.
    Handle(TopTools_HSequenceOfShape) wires;
    ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edgesBySlices[i], 1e-3, false, wires);
    wiresBySlices.push_back(wires);
  }

  /* =================
   *  Construct faces.
   * ================= */

  BRep_Builder bbuilder;
  TopoDS_Compound wiresComp;
  bbuilder.MakeCompound(wiresComp);

  for ( int i = 0; i < numPlanes; ++i )
  {
    Handle(TopTools_HSequenceOfShape) facesBySlice = new TopTools_HSequenceOfShape;
    //
    for ( TopTools_SequenceOfShape::Iterator wit(*wiresBySlices[i]); wit.More(); wit.Next() )
    {
      TopoDS_Shape wire = wit.Value();

      // Maximize edges.
      if ( m_bMaximize )
      {
        ShapeUpgrade_UnifySameDomain Maximizer(wire);
        Maximizer.Build();
        wire = Maximizer.Shape();
      }

      bbuilder.Add(wiresComp, wire);
      //vout << wire;

      // Use the slicing plane and do not let OpenCascade to deduce a plane by itself
      // as it would be too expensive.
      TopoDS_Face face = BRepBuilderAPI_MakeFace( planes[i], TopoDS::Wire(wire) );
      //
      facesBySlice->Append(face);
    }

    /* Check whether the slice contains the nested faces. The nested faces
     * could appear in hollowed solids and would essentially correspond
     * to cavities which slicers just fills in. For such cases, we do the
     * Boolean cut to get back the lost inner contours. */

    // Search for a biggest face.
    TopTools_HSequenceOfShape::Iterator fIt;
    TopoDS_Shape biggest;
    double maxArea = 0.0;
    for ( fIt.Init(*facesBySlice.get()); fIt.More(); fIt.Next() )
    {
      double area = computeArea(fIt.Value());
      if ( area - maxArea > 0.001 )
      {
        maxArea = area;
        biggest = fIt.Value();
      }
    }

    // Do the Boolean cut for the biggest face against others.
    TopoDS_Shape result = biggest;
    for ( fIt.Init(*facesBySlice.get()); fIt.More(); fIt.Next() )
    {
      if ( !fIt.Value().IsSame(biggest))
      {
        result = asiAlgo_Utils::BooleanCut( result, fIt.Value() );
      }
    }

    // Keep the resultant face to slice.
    facesBySlice->Clear();
    for ( TopExp_Explorer exp(result, TopAbs_FACE); exp.More(); exp.Next() )
    {
      // ~DEBUGGING~
      //m_plotter.DRAW_SHAPE(exp.Value(), "slice");

      facesBySlice->Append(exp.Value());
    }

    m_faces.push_back(facesBySlice);
  }

  return true;
}
