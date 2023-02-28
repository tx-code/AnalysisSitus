//-----------------------------------------------------------------------------
// Created on: 14 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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
#include <asiAlgo_CheckClearance.h>

// asiAlgo includes
#include <asiAlgo_HitFacet.h>
#include <asiAlgo_MeshField.h>
#include <asiAlgo_MeshMerge.h>

// OpenCascade includes
#include <gp_Lin.hxx>

#if defined USE_MOBIUS
// Mobius includes
#include <mobius/cascade.h>

using namespace mobius;
#endif

//-----------------------------------------------------------------------------

asiAlgo_CheckClearance::asiAlgo_CheckClearance(const TopoDS_Shape&  shape,
                                               ActAPI_ProgressEntry progress,
                                               ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_fMinClr         ( 0. ),
  m_fMaxClr         ( 0. )
{
#if defined USE_MOBIUS
  // Merge facets.
  asiAlgo_MeshMerge meshMerge(shape, asiAlgo_MeshMerge::Mode_MobiusMesh, false);
  //
  m_resField.triangulation = meshMerge.GetMobiusMesh();

  // Build BVH.
  m_bvh = new asiAlgo_BVHFacets(m_resField.triangulation);
#else
  (void) shape;
#endif
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS

asiAlgo_CheckClearance::asiAlgo_CheckClearance(const t_ptr<t_mesh>& tris,
                                               ActAPI_ProgressEntry progress,
                                               ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm ( progress, plotter ),
  m_fMinClr         ( 0. ),
  m_fMaxClr         ( 0. )
{
  m_resField.triangulation = tris;

  // Build BVH.
  m_bvh = new asiAlgo_BVHFacets(m_resField.triangulation,
                                asiAlgo_BVHFacets::Builder_Binned,
                                false);
}

#endif

//-----------------------------------------------------------------------------

bool asiAlgo_CheckClearance::Perform()
{
#if defined USE_MOBIUS
  if ( m_resField.triangulation.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Null triangulation.");
    return false;
  }

  asiAlgo_HitFacet HitFacet(m_bvh/*, m_progress, m_plotter*/);

  // Prepare scalar field.
  Handle(asiAlgo_MeshScalarField) field = new asiAlgo_MeshScalarField;
  m_resField.fields.push_back(field);

  // Cast a ray from each facet.
  double minScalar = Precision::Infinite(), maxScalar = -Precision::Infinite();
  //
  for ( t_mesh::TriangleIterator tit(m_resField.triangulation); tit.More(); tit.Next() )
  {
    const poly_TriangleHandle th = tit.Current();

    poly_Triangle<> t;
    if ( !m_resField.triangulation->GetTriangle(th, t) || t.IsDeleted() )
      continue;

    // Skip faces lying out of a subdomain, if any.
    const int fid = t.GetFaceRef();
    //
    if ( this->HasSubdomain() && !this->IsInSubdomain(fid) )
      continue;

    // Get nodes.
    poly_VertexHandle vh[3];
    t.GetVertices(vh[0], vh[1], vh[2]);
    //
    t_xyz P[3];
    //
    for ( int k = 0; k < 3; ++k )
      m_resField.triangulation->GetVertex(vh[k], P[k]);

    // Center point.
    t_xyz C = (P[0] + P[1] + P[2]) / 3.;

    /* Initialize norm. */

    t_xyz V1 = P[1] - P[0];
    //
    if ( V1.SquaredModulus() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V1.Normalize();

    t_xyz V2 = P[2] - P[0];
    //
    if ( V2.SquaredModulus() < 1e-8 )
      continue; // Skip invalid facet.
    //
    V2.Normalize();

    // Compute norm.
    t_xyz N = V1 ^ V2;
    //
    if ( N.SquaredModulus() < 1e-8 )
      continue; // Skip invalid facet
    //
    N.Normalize();

    // Direction to analyze clearance locally.
    t_xyz dir = N;

    /* Shoot a ray to find intersection. */

    // Exclude the originating face from the intersection test.
    HitFacet.SetFaceToSkip(th.iIdx);

    // Clearance scalar.
    double clearance = 0.;

    gp_Lin ray( cascade::GetOpenCascadePnt(C), cascade::GetOpenCascadeVec(dir) );

    // Do the intersection test.
    gp_XYZ hit;
    int facetIdx = -1;
    //
    HitFacet(ray, facetIdx, hit);

    // Now clearance is simply a distance.
    if ( facetIdx != -1 )
      clearance = cascade::GetOpenCascadePnt(C).Distance(hit);

    /*if ( !m_bIsCustomDir && (tidx == 51121) )
    {
      m_plotter.REDRAW_POINT("C", C, Color_Blue);
      m_plotter.REDRAW_POINT("hit", hit, Color_Red);
      m_plotter.REDRAW_VECTOR_AT("N", C, N.Reversed(), Color_Blue);

      return false;
    }*/

    // Store scalars in the field.
    if ( facetIdx != -1 )
    {
      double *pClr = field->data.ChangeSeek(th.iIdx);
      //
      if ( pClr == nullptr )
        field->data.Bind(th.iIdx, clearance);
      else if ( clearance > *pClr)
        *pClr = clearance;

      // Update the extreme values.
      if ( clearance < minScalar )
      {
        minScalar = clearance;
      }
      if ( clearance > maxScalar )
      {
        maxScalar = clearance;
      }
    }
  }

  // Set extreme clearance values.
  if ( !Precision::IsInfinite(minScalar) )
    m_fMinClr = minScalar;
  //
  if ( !Precision::IsInfinite(maxScalar) )
    m_fMaxClr = maxScalar;

  return true;
#else
  m_progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return false;
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_CheckClearance::SetSubdomain(const TColStd_PackedMapOfInteger& subdomain)
{
  m_subdomain = subdomain;
}

//-----------------------------------------------------------------------------

bool asiAlgo_CheckClearance::HasSubdomain() const
{
  return !m_subdomain.IsEmpty();
}

//-----------------------------------------------------------------------------

bool asiAlgo_CheckClearance::IsInSubdomain(const int id) const
{
  return m_subdomain.Contains(id);
}
