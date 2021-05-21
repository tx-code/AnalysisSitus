//-----------------------------------------------------------------------------
// Copyright (c) 2017-present, Sergey Slyadnev
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
#include <asiAlgo_RecognizeDrillHolesRule.h>

// asiAlgo includes
#include <asiAlgo_CanRecTools.h>
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_FindFeatureHints.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_Ax1.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Lin2d.hxx>
#include <Precision.hxx>
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopoDS.hxx>

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

namespace {

  //! Returns the indices of all vertical edges in the UV space of the
  //! passed face `fid`.
  //! \param[in] fid       the 1-based index of the face to inspect.
  //! \param[in] aag       the attributed adjacency graph.
  //! \param[in] tolAngDeg the angular tolerance (in degrees) to use.
  //! \return the collection of 1-based edge indices.
  TColStd_PackedMapOfInteger
    GetVerticalEdges(const int                  fid,
                     const Handle(asiAlgo_AAG)& aag,
                     const double               tolAngDeg)
  {
    const TopoDS_Face& face = aag->GetFace(fid);

    // Get all edges.
    TopTools_IndexedMapOfShape faceEdges;
    TopExp::MapShapes(face, TopAbs_EDGE, faceEdges);

    gp_Dir2d OV = gp_Dir2d(0, 1);

    // Keep vertical edges.
    TColStd_PackedMapOfInteger eids;
    //
    for ( int eidx = 1; eidx <= faceEdges.Extent(); ++eidx )
    {
      const TopoDS_Edge& edge = TopoDS::Edge( faceEdges(eidx) );

      double f, l;
      Handle(Geom2d_Curve) c2d = BRep_Tool::CurveOnSurface(edge, face, f, l);

      gp_Lin2d c2dlin;
      if ( asiAlgo_Utils::IsStraightPCurve(c2d, c2dlin, true) )
      {
        const gp_Dir2d& DL = c2dlin.Direction();

        if ( DL.IsParallel(OV, tolAngDeg) )
        {
          eids.Add( aag->RequestMapOfEdges().FindIndex(edge) );
        }
      }
    }

    return eids;
  }

}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeDrillHolesRule::recognize(TopTools_IndexedMapOfShape& featureFaces,
                                               TColStd_PackedMapOfInteger& featureIndices)
{
  // Extract suspected face and check if it is a cylinder. Even if a hole
  // is composed of other types of surfaces (e.g. conical), we assume that
  // a cylinder is something primary.
  gp_Ax1      ref_ax;
  double      suspected_ang_min = 0.0;
  double      suspected_ang_max = 0.0;
  double      suspected_radius  = 0.0;
  const int   suspected_face_id = m_it->GetFaceId();
  TopoDS_Face suspected_face    = m_it->GetGraph()->GetFace(suspected_face_id);

  //---------------------------------------------------------------------------
  this->SetTraversed(suspected_face_id);
  //---------------------------------------------------------------------------

  asiAlgo_Feature neighbors;
  //
  if ( !m_it->GetNeighbors(neighbors) )
    return false;

  if ( neighbors.Extent() == 1 )
  {
    // If there is just one neighbor, it should be a periodic surface.
    const int nid = neighbors.GetMinimalMapped();
    //
    if ( !asiAlgo_Utils::IsCylindrical( m_it->GetGraph()->GetFace(nid) ) )
      return false;
  }

  // In soft mode we do not allow any imprints in the parametric portrait
  // of the suspected face. Any imprint is a signal that there is a feature
  // interaction in that local zone. This situation is, however, absolutely,
  // normal for hard features.

  if ( !m_bHardMode && !this->isCylindrical(suspected_face,
                                            true,
                                            suspected_radius,
                                            suspected_ang_min,
                                            suspected_ang_max,
                                            ref_ax) )
    return false;

  if ( m_bHardMode && !this->isCylindrical(suspected_face,
                                           false,
                                           suspected_radius,
                                           suspected_ang_min,
                                           suspected_ang_max,
                                           ref_ax) )
    return false;

  // Check if this face is attributed. Attribution is normally done for
  // closed periodical faces in order to mark their "self-adjacency". Such
  // self-adjacency situation is somewhat very typical for OpenCascade's
  // B-Rep, but it is not maintained in Joshi's AAG, so here is the extension.
  const Handle(asiAlgo_FeatureAttrAngle)&
    faceAngleAttr = m_it->GetGraph()->ATTR_NODE<asiAlgo_FeatureAttrAngle>(suspected_face_id);
  //
  if ( !faceAngleAttr.IsNull() )
  {
    // A hole has concave self-adjacency.
    if ( !asiAlgo_FeatureAngle::IsConcave( faceAngleAttr->GetAngleType() ) )
      return false;
  }

  // Check radius of the cylinder against the requested barrier value.
  if ( suspected_radius > m_fTargetRadius )
    return false;

  // Traverse other cylindrical neighbors as a cylindrical hole is very often
  // composed of several patches.
  asiAlgo_Feature cyls;
  int             nbSuspectedSupports = 0;
  double          sum_angle           = Abs(suspected_ang_max - suspected_ang_min);
  //
  this->visitNeighborCylinders(suspected_face_id,
                               suspected_face_id,
                               suspected_radius,
                               ref_ax,
                               sum_angle,
                               cyls);
  //-------------------------------------------------------------------
  this->AddTraversed(cyls);
  //-------------------------------------------------------------------

  neighbors.Subtract(cyls);
  nbSuspectedSupports = neighbors.Extent();

  // In soft feature mode we expect to have two support faces only.
  if ( !m_bHardMode && nbSuspectedSupports != 2 )
    return false;

  // The hole should be complete (round).
  if ( (sum_angle < 2*M_PI) && (Abs(sum_angle - 2*M_PI) > m_fAngToler) )
    return false;

  // From all feature face neighbors keep only those satisfying the hard-coded
  // rules below. Notice that graph pattern matching is a more general
  // approach, comparing to what we are doing here. But here we can
  // conduct a precise analysis of host geometry, so we prefer rule-based
  // approach of feature identification.
  asiAlgo_Feature suspected_endings;
  //
  for ( asiAlgo_Feature::Iterator fit( this->JustTraversed() ); fit.More(); fit.Next() )
  {
    const int              feature_face_id        = fit.Key();
    const asiAlgo_Feature& feature_face_neighbors = m_it->GetGraph()->GetNeighbors(feature_face_id);

    // Each neighbor is suspected to be an ending now.
    for ( asiAlgo_Feature::Iterator nit(feature_face_neighbors); nit.More(); nit.Next() )
    {
      const int          neighbor_id   = nit.Key();
      const TopoDS_Face& neighbor_face = m_it->GetGraph()->GetFace(neighbor_id);
      //
      if ( asiAlgo_Utils::IsConical(neighbor_face) || asiAlgo_Utils::IsPlanar(neighbor_face) )
        suspected_endings.Add(neighbor_id);
    }
  }

  // Now check that each suspected ending has neighbors which are feature
  // faces or other endings. If not (i.e. there is at least one neighbor
  // which is not enumerated neither in the list of feature faces, nor in the
  // list of other endings), then such a suspected item is rejected.
  for ( asiAlgo_Feature::Iterator eit(suspected_endings); eit.More(); eit.Next() )
  {
    const int              ending_id        = eit.Key();
    const asiAlgo_Feature& ending_neighbors = m_it->GetGraph()->GetNeighbors(ending_id);

    bool isRealEnding = true;
    for ( asiAlgo_Feature::Iterator enit(ending_neighbors); enit.More(); enit.Next() )
    {
      const int ending_neighbor_id = enit.Key();
      if ( !this->JustTraversed().Contains(ending_neighbor_id) && !suspected_endings.Contains(ending_neighbor_id) )
      {
        isRealEnding = false;
        break;
      }
    }

    if ( isRealEnding )
    {
      //-----------------------------------------------------------------------
      this->SetTraversed(ending_id);
      //-----------------------------------------------------------------------
    }
    else
    {
      const TopoDS_Face& ending_face = m_it->GetGraph()->GetFace(ending_id);
      //
      gp_Ax1 cone_ax;
      if ( asiAlgo_Utils::IsConical(ending_face, cone_ax) )
      {
        if ( cone_ax.IsCoaxial(ref_ax, m_fAngToler, m_fLinToler) ||
             cone_ax.IsOpposite(ref_ax, m_fAngToler) )
        {
          // Co-axial but position of the axis can be different.
          const gp_Pnt& cone_ax_P = cone_ax.Location();
          const gp_Pnt& ref_ax_P  = ref_ax.Location();

#if defined DRAW_DEBUG
          this->GetPlotter().DRAW_POINT(cone_ax_P, Color_Green);
          this->GetPlotter().DRAW_POINT(ref_ax_P,  Color_Red);
#endif

          const double axes_dist = cone_ax_P.Distance(ref_ax_P);
          if ( axes_dist < m_fLinToler )
          {
            //-----------------------------------------------------------------
            this->SetTraversed(ending_id);
            //-----------------------------------------------------------------
          }
          else
          {
            gp_Ax1 sample_ax( ref_ax_P, gp_Vec( cone_ax_P.XYZ() - ref_ax_P.XYZ() ) );
            //
            if ( sample_ax.IsParallel(ref_ax, m_fAngToler) )
            {
              //-----------------------------------------------------------------
              this->SetTraversed(ending_id);
              //-----------------------------------------------------------------
            }
          }
        }
      }
    }
  }

  // Fill collection of feature faces.
  featureIndices.Unite( this->JustTraversed() );
  //
  for ( asiAlgo_Feature::Iterator mit( this->JustTraversed() ); mit.More(); mit.Next() )
    featureFaces.Add( m_it->GetGraph()->GetFace( mit.Key() ) );

  // Set radius.
  m_fRadius = suspected_radius;
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeDrillHolesRule::isCylindrical(const TopoDS_Face& face) const
{
  double radius;
  double angle_min;
  double angle_max;
  gp_Ax1 ax;

  return this->isCylindrical(face, false, radius, angle_min, angle_max, ax);
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeDrillHolesRule::isCylindrical(const TopoDS_Face& face,
                                                   const bool         checkNoHints,
                                                   double&            radius,
                                                   double&            angle_min,
                                                   double&            angle_max,
                                                   gp_Ax1&            ax) const
{
  bool isCyl = asiAlgo_Utils::IsCylindrical(face, radius, ax, angle_min, angle_max);
  //
  if ( !isCyl )
  {
    Handle(Geom_Surface) surface = BRep_Tool::Surface(face);

    // Avoid computing UV bounds for any surface type except for splines which
    // are potentially non-canonical cylinders.
    GeomAdaptor_Surface surfaceAdt(surface);

    if ( surfaceAdt.GetType() == GeomAbs_BSplineSurface )
    {
      double uMin = DBL_MAX, uMax = -DBL_MAX, vMin = DBL_MAX, vMax = -DBL_MAX;
      double uMinRec, uMaxRec, vMinRec, vMaxRec;

      // Take the UV values cached in vertices to avoid the expensive
      // UV-bounds computation with BRepTools.
      TopTools_IndexedMapOfShape faceVertices;
      TopExp::MapShapes(face, TopAbs_VERTEX, faceVertices);
      //
      for ( int v = 1; v <= faceVertices.Extent(); ++v )
      {
        const TopoDS_Vertex& V = TopoDS::Vertex( faceVertices(v) );
        gp_Pnt2d uv = BRep_Tool::Parameters(V, face);

        uMin = Min( uMin, uv.X() );
        uMax = Max( uMax, uv.X() );
        vMin = Min( vMin, uv.Y() );
        vMax = Max( vMax, uv.Y() );
      }
      //
      if ( Abs(uMin - uMax) < Precision::Confusion() )
        return false;
      //
      if ( Abs(vMin - vMax) < Precision::Confusion() )
        return false;

      // Give a shot to canonical recognition. This function will do nothing for
      // non-freeform types, such as planes, conical surfaces, etc. For splines,
      // it will attempt to recognize a cylinder with some extra geometric checks.
      gp_Cylinder cyl;
      if ( !asiAlgo_CanRecTools::IsCylindrical(BRep_Tool::Surface(face),
                                               uMin, uMax, vMin, vMax,
                                               m_fCanRecPrec, cyl,
                                               uMinRec, uMaxRec, vMinRec, vMaxRec,
                                               m_progress, m_plotter) )
        return false;

      // Get the props.
      radius    = cyl.Radius();
      angle_min = uMinRec;
      angle_max = uMaxRec;
      ax        = cyl.Axis();
    }
    else
    {
      return false;
    }
  }

  if ( checkNoHints )
  {
    asiAlgo_FindFeatureHints hint(face, nullptr, nullptr);
    //
    if ( hint.IsPuzzled() )
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeDrillHolesRule::visitNeighborCylinders(const int        sid,
                                                            const int        fid,
                                                            const double     refRadius,
                                                            const gp_Ax1&    refAxis,
                                                            double&          sumAng,
                                                            asiAlgo_Feature& collected)
{
  const TopoDS_Face& face = m_it->GetGraph()->GetFace(fid);

  // For cylindrical faces, we know that the neighbors of interest share with
  // the starting face its vertical edges. There is no such a cue for the spline
  // faces (for example), so we give them the default treatment.
  asiAlgo_Feature nids;
  //
  if ( asiAlgo_Utils::IsCylindrical(face) )
  {
    TColStd_PackedMapOfInteger
      verticalEids = ::GetVerticalEdges( fid, m_it->GetGraph(), 1.*M_PI/180. );

    nids = m_it->GetGraph()->GetNeighborsThru(fid, verticalEids);
  }
  else
  {
    nids = m_it->GetGraph()->GetNeighbors(fid);
  }

  // Visit neighbors.
  for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
  {
    const int nid = nit.Key();
    //
    if ( collected.Contains(nid) || (nid == sid) )
      continue;

    const TopoDS_Face& nface = m_it->GetGraph()->GetFace(nid);

    // Props.
    gp_Ax1 ax;
    double angMin = 0.0;
    double angMax = 0.0;
    double nr     = 0.0;

    // Check cylindricity.
    if ( ( !m_bHardMode && this->isCylindrical(nface, true,  nr, angMin, angMax, ax) ) ||
         (  m_bHardMode && this->isCylindrical(nface, false, nr, angMin, angMax, ax) ) )
    {
      if ( Abs(nr - refRadius) > m_fLinToler )
      {
        continue; // If this neighbor cylinder is a patch of the primary hole's
                  // geometry, then we expect it to have the same radius.
      }

      // Another criterion is to have concave angle between patches. If the
      // angle is convex, then this hole is not a cavity, but a kind of a boss.
      Handle(asiAlgo_FeatureAttrAngle)
        attr = m_it->GetGraph()->ATTR_ARC<asiAlgo_FeatureAttrAngle>( asiAlgo_AAG::t_arc(fid, nid) );
      //
      if ( !asiAlgo_FeatureAngle::IsConcave( attr->GetAngleType() ) )
        continue; // Might be a cylindrical support, so let's go further.

      if ( ax.IsCoaxial  (refAxis, m_fAngToler, m_fLinToler) ||
           ax.IsOpposite (refAxis, m_fAngToler) )
      {
        // Co-axial but position of the axis can be different.
        const gp_Pnt& ax_P     = ax.Location();
        const gp_Pnt& ref_ax_P = refAxis.Location();
        //
        if ( ax_P.Distance(ref_ax_P) < m_fLinToler )
        {
          collected.Add(nid);

          sumAng += Abs(angMax - angMin);

          // Continue recursively.
          this->visitNeighborCylinders(sid, nid, refRadius, refAxis, sumAng, collected);
        }
      }
    }
  }
}
