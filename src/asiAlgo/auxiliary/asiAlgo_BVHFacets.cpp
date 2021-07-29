//-----------------------------------------------------------------------------
// Created on: 21 September 2016
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
#include <asiAlgo_BVHFacets.h>

// asiAlgo includes
#include <asiAlgo_BVHIterator.h>

// OCCT includes
#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BVH_BinnedBuilder.hxx>
#include <BVH_LinearBuilder.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>

// Standard includes
#include <map>

#if defined USE_MOBIUS
#include <mobius/cascade.h>

using namespace mobius;
#endif

//-----------------------------------------------------------------------------

asiAlgo_BVHFacets::asiAlgo_BVHFacets(const TopoDS_Shape&  model,
                                     const BuilderType    builderType,
                                     ActAPI_ProgressEntry progress,
                                     ActAPI_PlotterEntry  plotter)
: BVH_PrimitiveSet<double, 3> (),
  m_fBoundingDiag             (0.0),
  m_progress                  (progress),
  m_plotter                   (plotter)
{
  this->init(model, builderType);
  this->MarkDirty();
}

//-----------------------------------------------------------------------------

asiAlgo_BVHFacets::asiAlgo_BVHFacets(const Handle(Poly_Triangulation)& mesh,
                                     const BuilderType                 builderType,
                                     ActAPI_ProgressEntry              progress,
                                     ActAPI_PlotterEntry               plotter)
: BVH_PrimitiveSet<double, 3> (),
  m_fBoundingDiag             (0.0),
  m_progress                  (progress),
  m_plotter                   (plotter)
{
  this->init(mesh, builderType);
  this->MarkDirty();
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
asiAlgo_BVHFacets::asiAlgo_BVHFacets(const t_ptr<poly_Mesh>& mesh,
                                     const BuilderType       builderType,
                                     const bool              useFaceRefs,
                                     ActAPI_ProgressEntry    progress,
                                     ActAPI_PlotterEntry     plotter)
{
  this->init(mesh, builderType, useFaceRefs);
  this->MarkDirty();
}
#endif

//-----------------------------------------------------------------------------

int asiAlgo_BVHFacets::Size() const
{
  return (int) m_facets.size();
}

//-----------------------------------------------------------------------------

BVH_Box<double, 3> asiAlgo_BVHFacets::Box(const int index) const
{
  BVH_Box<double, 3> box;
  const t_facet& facet = m_facets[index];

  box.Add(facet.P0);
  box.Add(facet.P1);
  box.Add(facet.P2);

  return box;
}

//-----------------------------------------------------------------------------

double asiAlgo_BVHFacets::Center(const int index, const int axis) const
{
  const t_facet& facet = m_facets[index];

  if ( axis == 0 )
    return (1.0 / 3.0) * ( facet.P0.x() + facet.P1.x() + facet.P2.x() );
  else if ( axis == 1 )
    return (1.0 / 3.0) * ( facet.P0.y() + facet.P1.y() + facet.P2.y() );

  // The last possibility is "axis == 2"
  return (1.0 / 3.0) * ( facet.P0.z() + facet.P1.z() + facet.P2.z() );
}

//-----------------------------------------------------------------------------

void asiAlgo_BVHFacets::Swap(const int index1, const int index2)
{
  std::swap(m_facets[index1], m_facets[index2]);
}

//-----------------------------------------------------------------------------

inline void asiAlgo_BVHFacets::GetVertices(const int  index,
                                           BVH_Vec3d& vertex1,
                                           BVH_Vec3d& vertex2,
                                           BVH_Vec3d& vertex3) const
{
  const t_facet& facet = m_facets[index];

  vertex1 = facet.P0;
  vertex2 = facet.P1;
  vertex3 = facet.P2;
}

//-----------------------------------------------------------------------------

double asiAlgo_BVHFacets::GetBoundingDiag() const
{
  return m_fBoundingDiag;
}

//-----------------------------------------------------------------------------

void asiAlgo_BVHFacets::Dump(ActAPI_PlotterEntry IV)
{
  // Access (build) hierarchy of boxes
  const opencascade::handle<BVH_Tree<double, 3>>& bvh = this->BVH();
  //
  if ( bvh.IsNull() )
  {
    std::cout << "Error: BVH construction failed" << std::endl;
    return;
  }

  // Prepare a topological structure to store BVH primitives in explicit form
  TopoDS_Compound comp_left, comp_right;
  BRep_Builder BB;
  BB.MakeCompound(comp_left);
  BB.MakeCompound(comp_right);

  // Loop over the BVH nodes
  for ( asiAlgo_BVHIterator it(bvh); it.More(); it.Next() )
  {
    const BVH_Vec4i& nodeData = it.Current();

    if ( !it.IsLeaf() )
    {
      const BVH_Vec3d& minCorner_Left  = bvh->MinPoint( nodeData.y() );
      const BVH_Vec3d& maxCorner_Left  = bvh->MaxPoint( nodeData.y() );
      const BVH_Vec3d& minCorner_Right = bvh->MinPoint( nodeData.z() );
      const BVH_Vec3d& maxCorner_Right = bvh->MaxPoint( nodeData.z() );

      // Left box
      {
        gp_Pnt Pmin( minCorner_Left.x(), minCorner_Left.y(), minCorner_Left.z() );
        gp_Pnt Pmax( maxCorner_Left.x(), maxCorner_Left.y(), maxCorner_Left.z() );

        const gp_Pnt P2 = gp_Pnt( Pmax.X(), Pmin.Y(), Pmin.Z() );
        const gp_Pnt P3 = gp_Pnt( Pmax.X(), Pmax.Y(), Pmin.Z() );
        const gp_Pnt P4 = gp_Pnt( Pmin.X(), Pmax.Y(), Pmin.Z() );
        const gp_Pnt P5 = gp_Pnt( Pmin.X(), Pmin.Y(), Pmax.Z() );
        const gp_Pnt P6 = gp_Pnt( Pmax.X(), Pmin.Y(), Pmax.Z() );
        const gp_Pnt P8 = gp_Pnt( Pmin.X(), Pmax.Y(), Pmax.Z() );

        if ( Pmin.Distance(P2) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(Pmin, P2) );

        if ( P2.Distance(P3) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P2, P3) );

        if ( P3.Distance(P4) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P3, P4) );

        if ( P4.Distance(Pmin) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P4, Pmin) );

        if ( P5.Distance(P6) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P5, P6) );

        if ( P6.Distance(Pmax) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P6, Pmax) );

        if ( Pmax.Distance(P8) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(Pmax, P8) );

        if ( P8.Distance(P5) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P8, P5) );

        if ( P6.Distance(P2) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P6, P2) );

        if ( Pmax.Distance(P3) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(Pmax, P3) );

        if ( P8.Distance(P4) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P8, P4) );

        if ( P5.Distance(Pmin) > 1.0e-6 )
          BB.Add( comp_left, BRepBuilderAPI_MakeEdge(P5, Pmin) );
      }

      // Right box
      {
        gp_Pnt Pmin( minCorner_Right.x(), minCorner_Right.y(), minCorner_Right.z() );
        gp_Pnt Pmax( maxCorner_Right.x(), maxCorner_Right.y(), maxCorner_Right.z() );

        const gp_Pnt P2 = gp_Pnt( Pmax.X(), Pmin.Y(), Pmin.Z() );
        const gp_Pnt P3 = gp_Pnt( Pmax.X(), Pmax.Y(), Pmin.Z() );
        const gp_Pnt P4 = gp_Pnt( Pmin.X(), Pmax.Y(), Pmin.Z() );
        const gp_Pnt P5 = gp_Pnt( Pmin.X(), Pmin.Y(), Pmax.Z() );
        const gp_Pnt P6 = gp_Pnt( Pmax.X(), Pmin.Y(), Pmax.Z() );
        const gp_Pnt P8 = gp_Pnt( Pmin.X(), Pmax.Y(), Pmax.Z() );

        if ( Pmin.Distance(P2) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(Pmin, P2) );

        if ( P2.Distance(P3) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P2, P3) );

        if ( P3.Distance(P4) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P3, P4) );

        if ( P4.Distance(Pmin) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P4, Pmin) );

        if ( P5.Distance(P6) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P5, P6) );

        if ( P6.Distance(Pmax) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P6, Pmax) );

        if ( Pmax.Distance(P8) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(Pmax, P8) );

        if ( P8.Distance(P5) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P8, P5) );

        if ( P6.Distance(P2) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P6, P2) );

        if ( Pmax.Distance(P3) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(Pmax, P3) );

        if ( P8.Distance(P4) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P8, P4) );

        if ( P5.Distance(Pmin) > 1.0e-6 )
          BB.Add( comp_right, BRepBuilderAPI_MakeEdge(P5, Pmin) );
      }
    }
  }

  // Draw BVH
  IV.REDRAW_SHAPE("BVH Left",  comp_left,  Color_Yellow, 1.0, true);
  IV.REDRAW_SHAPE("BVH Right", comp_right, Color_Yellow, 1.0, true);
}

//-----------------------------------------------------------------------------

bool asiAlgo_BVHFacets::init(const TopoDS_Shape& model,
                             const BuilderType   builderType)
{
  if ( model.IsNull() )
    return false;

  // Prepare builder
  if ( builderType == Builder_Binned )
    myBuilder = new BVH_BinnedBuilder<double, 3, 32>(5, 32);
  else
    myBuilder = new BVH_LinearBuilder<double, 3>(5, 32);

  // Explode shape on faces to get face indices
  if ( m_faces.IsEmpty() )
    TopExp::MapShapes(model, TopAbs_FACE, m_faces);

  // Initialize with facets taken from faces
  for ( int fidx = 1; fidx <= m_faces.Extent(); ++fidx )
  {
    const TopoDS_Face& face = TopoDS::Face( m_faces(fidx) );
    //
    if ( !this->addFace(face, fidx) )
      continue; // Do not return false, just skip as otherwise
                // BVH will be incorrect for faulty shapes!
  }

  // Calculate bounding diagonal
  Bnd_Box aabb;
  BRepBndLib::Add(model, aabb);
  //
  m_fBoundingDiag = ( aabb.CornerMax().XYZ() - aabb.CornerMin().XYZ() ).Modulus();

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BVHFacets::init(const Handle(Poly_Triangulation)& mesh,
                             const BuilderType                 builderType)
{
  // Prepare builder
  if ( builderType == Builder_Binned )
    myBuilder = new BVH_BinnedBuilder<double, 3, 32>(5, 32);
  else
    myBuilder = new BVH_LinearBuilder<double, 3>(5, 32);

  // Initialize with the passed facets
  if ( !this->addTriangulation(mesh, TopLoc_Location(), -1, false) )
    return false;

  // Calculate bounding diagonal using fictive face to satisfy OpenCascade's API
  BRep_Builder BB;
  TopoDS_Face F;
  BB.MakeFace(F, mesh);
  Bnd_Box aabb;
  BRepBndLib::Add(F, aabb);
  //
  m_fBoundingDiag = ( aabb.CornerMax().XYZ() - aabb.CornerMin().XYZ() ).Modulus();

  return true;
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_BVHFacets::init(const t_ptr<poly_Mesh>& mesh,
                             const BuilderType       builderType,
                             const bool              useFaceRefs)
{
  if ( mesh.IsNull() )
    return false;

  // Prepare builder
  if ( builderType == Builder_Binned )
    myBuilder = new BVH_BinnedBuilder<double, 3, 32>(5, 32);
  else
    myBuilder = new BVH_LinearBuilder<double, 3>(5, 32);

  Bnd_Box aabb;

  // Iterate the mesh.
  for ( poly_Mesh::TriangleIterator tit(mesh); tit.More(); tit.Next() )
  {
    poly_TriangleHandle th = tit.Current();
    poly_Triangle       t;
    poly_VertexHandle   vh[3];

    // Get triangle and skip it if it's marked as "deleted".
    mesh->GetTriangle(th, t);
    //
    if ( t.IsDeleted() )
      continue;

    // Access the vertices.
    t.GetVertices(vh[0], vh[1], vh[2]);
    //
    t_xyz  nodes[3];
    gp_Pnt pts[3];
    //
    for ( int k = 0; k < 3; ++k )
    {
      mesh->GetVertex(vh[k], nodes[k]);
      pts[k] = cascade::GetOpenCascadePnt(nodes[k]);

      aabb.Add(pts[k]);
    }

    // Create a new facet
    const int face_idx = t.GetFaceRef();
    t_facet facet( (face_idx == Mobius_InvalidHandleIndex || !useFaceRefs) ? th.iIdx : face_idx );

    // Initialize nodes
    facet.P0 = BVH_Vec3d( nodes[0].X(), nodes[0].Y(), nodes[0].Z() );
    facet.P1 = BVH_Vec3d( nodes[1].X(), nodes[1].Y(), nodes[1].Z() );
    facet.P2 = BVH_Vec3d( nodes[2].X(), nodes[2].Y(), nodes[2].Z() );

    /* Initialize normal */

    gp_Vec V1(pts[0], pts[1]);
    //
    if ( V1.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V1.Normalize();

    gp_Vec V2(pts[0], pts[2]);
    //
    if ( V2.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V2.Normalize();

    // Compute norm
    facet.N = V1.Crossed(V2);
    //
    if ( facet.N.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet
    //
    facet.N.Normalize();

    // Store facet in the internal collection
    m_facets.push_back(facet);
  }

  m_fBoundingDiag = ( aabb.CornerMax().XYZ() - aabb.CornerMin().XYZ() ).Modulus();

  return true;
}
#endif

//-----------------------------------------------------------------------------

bool asiAlgo_BVHFacets::addFace(const TopoDS_Face& face,
                                const int          face_idx)
{
  TopLoc_Location loc;
  const Handle(Poly_Triangulation)& tris = BRep_Tool::Triangulation(face, loc);

  return this->addTriangulation( tris, loc, face_idx, (face.Orientation() == TopAbs_REVERSED) );
}

//-----------------------------------------------------------------------------

bool asiAlgo_BVHFacets::addTriangulation(const Handle(Poly_Triangulation)& triangulation,
                                         const TopLoc_Location&            loc,
                                         const int                         face_idx,
                                         const bool                        isReversed)
{
  if ( triangulation.IsNull() )
    return false;

  // Internal collections of triangles and nodes
  const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
  const TColgp_Array1OfPnt&    nodes     = triangulation->Nodes();

  for ( int elemId = triangles.Lower(); elemId <= triangles.Upper(); ++elemId )
  {
    const Poly_Triangle& tri = triangles(elemId);

    int n1, n2, n3;
    tri.Get(n1, n2, n3);

    gp_Pnt P0 = nodes(isReversed ? n3 : n1);
    P0.Transform(loc);
    //
    gp_Pnt P1 = nodes(n2);
    P1.Transform(loc);
    //
    gp_Pnt P2 = nodes(isReversed ? n1 : n3);
    P2.Transform(loc);

    // Create a new facet
    t_facet facet(face_idx == -1 ? elemId : face_idx);

    // Initialize nodes
    facet.P0 = BVH_Vec3d( P0.X(), P0.Y(), P0.Z() );
    facet.P1 = BVH_Vec3d( P1.X(), P1.Y(), P1.Z() );
    facet.P2 = BVH_Vec3d( P2.X(), P2.Y(), P2.Z() );

    /* Initialize normal */

    gp_Vec V1(P0, P1);
    //
    if ( V1.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V1.Normalize();

    gp_Vec V2(P0, P2);
    //
    if ( V2.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V2.Normalize();

    // Compute norm
    facet.N = V1.Crossed(V2);
    //
    if ( facet.N.SquareMagnitude() < 1e-8 )
      continue; // Skip invalid facet
    //
    facet.N.Normalize();

    // Store facet in the internal collection
    m_facets.push_back(facet);
  }

  return true;
}
