//-----------------------------------------------------------------------------
// Created on: 29 March 2022
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
#include <asiAlgo_IntersectBoxMesh.h>

// asiAlgo includes
#include <asiAlgo_BVHIterator.h>

// OpenCascade includes
#include <gp_Pln.hxx>
#include <Extrema_ExtElC.hxx>
#include <IntAna_QuadQuadGeo.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

#define PmcPrec 1.e-6

#define Proj(onA, B) ( onA.X()*B.X() + onA.Y()*B.Y() + onA.Z()*B.Z() )

//-----------------------------------------------------------------------------

struct t_interInfo
{
  RTCD::AABB aabb;
  gp_Pnt     corners[8];
  gp_Pnt     triNodes[3];
};

//-----------------------------------------------------------------------------

namespace
{
  bool IsOut(const BVH_Vec3d&  boxMin,
             const BVH_Vec3d&  boxMax,
             const RTCD::AABB& box)
  {
    RTCD::AABB B;
    B.Add( boxMin.x(), boxMin.y(), boxMin.z() );
    B.Add( boxMax.x(), boxMax.y(), boxMax.z() );

    if ( B.IsOut(box, PmcPrec) )
      return true;

    return false;
  }

  bool IsSeparated(const gp_Vec&      axis,
                   const t_interInfo& iinfo)
  {
    double boxBnd[2] = { +Precision::Infinite(), -Precision::Infinite() };
    double triBnd[2] = { +Precision::Infinite(), -Precision::Infinite() };

    // Projection for the box points.
    for ( int i = 0; i < 8; ++i )
    {
      const double proj = Proj(axis, iinfo.corners[i]);
      //
      if ( proj < boxBnd[0] )
        boxBnd[0] = proj;
      if ( proj > boxBnd[1] )
        boxBnd[1] = proj;
    }

    // Projection for the box points.
    for ( int i = 0; i < 3; ++i )
    {
      const double proj = Proj(axis, iinfo.triNodes[i]);
      //
      if ( proj < triBnd[0] )
        triBnd[0] = proj;
      if ( proj > triBnd[1] )
        triBnd[1] = proj;
    }

    if ( boxBnd[1] + Precision::PConfusion() < triBnd[0] ||
         boxBnd[0] > triBnd[1] + Precision::PConfusion() )
    {
      return true;
    }

    return false;
  }
}

//-----------------------------------------------------------------------------

asiAlgo_IntersectBoxMesh::asiAlgo_IntersectBoxMesh()
{}

//-----------------------------------------------------------------------------

asiAlgo_IntersectBoxMesh::asiAlgo_IntersectBoxMesh(const Handle(asiAlgo_BVHFacets)& bvh)
: m_bvh(bvh)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_IntersectBoxMesh::Perform(const RTCD::AABB& box)
{
  const opencascade::handle<BVH_Tree<double, 3> >& BVH = m_bvh->BVH();
  //
  if ( BVH.IsNull() )
    return false;

  // Create intersection info for the input box.
  t_interInfo iinfo;
  double XMin, YMin, ZMin, XMax, YMax, ZMax;
  box.Get(XMin, YMin, ZMin, XMax, YMax, ZMax);

  iinfo.aabb       = box;
  iinfo.corners[0] = gp_Pnt(XMin, YMin, ZMin);
  iinfo.corners[1] = gp_Pnt(XMin, YMin, ZMax);
  iinfo.corners[2] = gp_Pnt(XMin, YMax, ZMin);
  iinfo.corners[3] = gp_Pnt(XMin, YMax, ZMax);
  iinfo.corners[4] = gp_Pnt(XMax, YMin, ZMin);
  iinfo.corners[5] = gp_Pnt(XMax, YMin, ZMax);
  iinfo.corners[6] = gp_Pnt(XMax, YMax, ZMin);
  iinfo.corners[7] = gp_Pnt(XMax, YMax, ZMax);

  for ( asiAlgo_BVHIterator it(BVH); it.More(); it.Next() )
  {
    const BVH_Vec4i& nodeData = it.Current();

    if ( it.IsLeaf() )
    {
      // Perform precise test.
      const bool isIntFound = this->intersectLeaves(nodeData, iinfo);
      //
      if ( isIntFound )
        return true;
    }
    else // sub-volume
    {
      const BVH_Vec3d& minPntLft = BVH->MinPoint( nodeData.y() );
      const BVH_Vec3d& maxPntLft = BVH->MaxPoint( nodeData.y() );
      const BVH_Vec3d& minPntRgh = BVH->MinPoint( nodeData.z() );
      const BVH_Vec3d& maxPntRgh = BVH->MaxPoint( nodeData.z() );

      const bool out1 = ::IsOut(minPntLft, maxPntLft, box);
      const bool out2 = ::IsOut(minPntRgh, maxPntRgh, box);

      if ( out1 )
        it.BlockLeft();

      if ( out2 )
        it.BlockRight();
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_IntersectBoxMesh::intersectLeaves(const BVH_Vec4i& leaf,
                                               t_interInfo&     iinfo)
{
  /* Implementation method is based on the Separation Axis Theorem. */

  // Loop over the tentative facets.
  for ( int tidx = leaf.y(); tidx <= leaf.z(); ++tidx )
  {
    // Get triangle and parent solid.
    const asiAlgo_BVHFacets::t_facet& facet = m_bvh->GetFacet(tidx);

    iinfo.triNodes[0] = gp_Pnt( facet.P0.x(), facet.P0.y(), facet.P0.z() );
    iinfo.triNodes[1] = gp_Pnt( facet.P1.x(), facet.P1.y(), facet.P1.z() );
    iinfo.triNodes[2] = gp_Pnt( facet.P2.x(), facet.P2.y(), facet.P2.z() );

    const RTCD::Point
      triP[3] = { RTCD::Point( iinfo.triNodes[0].X(), iinfo.triNodes[0].Y(), iinfo.triNodes[0].Z() ),
                  RTCD::Point( iinfo.triNodes[1].X(), iinfo.triNodes[1].Y(), iinfo.triNodes[1].Z() ),
                  RTCD::Point( iinfo.triNodes[2].X(), iinfo.triNodes[2].Y(), iinfo.triNodes[2].Z() ) };

    RTCD::AABB triBox;
    triBox.Add(triP[0]);
    triBox.Add(triP[1]);
    triBox.Add(triP[2]);

    // 3 tests: axises are (1, 0, 0), (0, 1, 0), (0, 0, 1).
    if ( iinfo.aabb.IsOut(triBox, PmcPrec) )
      continue;

    if ( !iinfo.aabb.IsOut(triP[0], PmcPrec) &&
         !iinfo.aabb.IsOut(triP[1], PmcPrec) &&
         !iinfo.aabb.IsOut(triP[2], PmcPrec) )
    {
      return true;
    }

    // 1 test: Plane / AABB intersection.
    gp_Vec V[3] = { gp_Vec(iinfo.triNodes[0], iinfo.triNodes[1]),
                    gp_Vec(iinfo.triNodes[0], iinfo.triNodes[2]),
                    gp_Vec(iinfo.triNodes[1], iinfo.triNodes[2]) };

    if ( V[0].SquareMagnitude() < Precision::SquareConfusion() ||
         V[1].SquareMagnitude() < Precision::SquareConfusion() ||
         V[2].SquareMagnitude() < Precision::SquareConfusion() )
    {
      continue;
    }

    gp_Pln pln( iinfo.triNodes[0], V[0].Crossed(V[2]) );

    if ( iinfo.aabb.IsOut(pln, PmcPrec) )
      continue;

    // 9 tests: e_i x f_j
    gp_Vec N[3] = { gp::DX(), gp::DY(), gp::DZ() };

    bool isOut = false;
    for ( int i = 0; i < 3; ++i )
    {
      for ( int j = 0; j < 3; ++j )
      {
        gp_Vec axis = V[i].Crossed(N[j]);

        if ( ::IsSeparated(axis, iinfo) )
          isOut = true;
      }
    }

    if ( !isOut )
      return true;
  }

  return false;
}
