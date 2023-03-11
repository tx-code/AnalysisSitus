//-----------------------------------------------------------------------------
// Created on: 04 March 2023
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
#include "asiAlgo_AppSurf2.h"

// asiAlgo includes
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_PlaneOnPoints.h>
#include <asiAlgo_Timer.h>

// Mobius includes
#include <mobius/cascade.h>
#include <mobius/geom_ApproxBSurf.h>

// OCCT includes
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <GeomConvert.hxx>
#include <NCollection_CellFilter.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#define MAGIC 23

using namespace mobius;

namespace
{
  //! Spatial filter to avoid repeated pinpoint constraints.
  class ApproxInspector : public NCollection_CellFilter_InspectorXYZ
  {
  public:
    typedef gp_XYZ Target;

    //! Constructor with the tolerance to set.
    ApproxInspector(const double tol)
    { m_tol = tol * tol; m_isFind = false; }

    void ClearFind()
    { m_isFind = false; }

    const bool IsFind() const
    { return m_isFind; }

    //! Set current point to search for coincidence.
    void SetCurrent (const gp_XYZ& pnt)
    { m_current = pnt; }

    //! Implementation of inspection method.
    NCollection_CellFilter_Action Inspect (const Target& obj)
    {
      const gp_XYZ pt = m_current.Subtracted(obj);
      const double sqDist = pt.SquareModulus();
      if( sqDist < m_tol )
        m_isFind = true;

      return CellFilter_Keep;
    }

  private:
    double m_tol;     //!< Squared comparison tolerance.
    gp_XYZ m_current; //!< Current point.
    bool   m_isFind;  //!< Detection state.
  };

  //! Sets pinpoint constraints for the approximation tool.
  void fillConstraints(const Handle(TopTools_HSequenceOfShape)& edges,
                       const Handle(asiAlgo_BaseCloud<double>)& extras,
                       const t_ptr<t_pcloud>&                   pts)
  {
    const double tol = 0.5;
    NCollection_CellFilter<ApproxInspector> cellFilter(tol);

    /* ================================
     *  Add constraints over the edges.
     * ================================ */

    // Iterate over edges to add point constraints.
    for ( int eidx = 1; eidx <= edges->Size(); ++eidx )
    {
      const TopoDS_Edge& edge = TopoDS::Edge( edges->Value(eidx) );
      BRepAdaptor_Curve curve(edge);
      //
      const double f = curve.FirstParameter();
      const double l = curve.LastParameter();

      std::vector<double> params;

      // Get points from the current edge.
      if ( curve.GetType() == GeomAbs_BSplineCurve )
      {
        Handle(Geom_BSplineCurve) bcurve = curve.BSpline();

        const TColStd_Array1OfReal& knots = bcurve->Knots();

        params.push_back(f);

        // Span-wise discretization.
        double    prevPar   = f;
        const int numSegPts = Max(bcurve->Degree(), 1);
        //
        for ( int i = 1; ( i < knots.Length() ) && ( knots(i) < ( l - Precision::PConfusion() ) ); ++i )
        {
          if ( knots(i + 1) < f + Precision::PConfusion() )
            continue;

          double step = ( knots(i + 1) - knots(i) ) / numSegPts;
          for ( int k = 1; k <= numSegPts ; ++k )
          {
            double par = knots(i) + k * step;
            if ( par > l - Precision::PConfusion() )
              break;

            if ( par > prevPar + Precision::PConfusion() )
            {
              params.push_back(par);
              prevPar = par;
            }
          }
        }
        params.push_back(l);
      }
      else
      {
        const int nbPnt = MAGIC;
        for ( int j = 0; j < nbPnt; ++j )
        {
          const double param = f + j * (l - f) / (nbPnt - 1);
          params.push_back(param);
        }
      }

      // Add points to the collection of constraints using cell filter.
      for ( const auto p : params )
      {
        const gp_Pnt pnt = curve.Value(p);

        ApproxInspector inspector(tol);
        const gp_XYZ min = inspector.Shift(pnt.XYZ(), -tol);
        const gp_XYZ max = inspector.Shift(pnt.XYZ(),  tol);

        inspector.ClearFind();
        inspector.SetCurrent( pnt.XYZ() );
        cellFilter.Inspect(min, max, inspector);

        if ( !inspector.IsFind() )
        {
          cellFilter.Add( pnt.XYZ(), pnt.XYZ() );

          pts->AddPoint( cascade::GetMobiusPnt(pnt) );
        }
      }
    }

    /* ================================
     *  Add optional inner constraints.
     * ================================ */

    if ( !extras.IsNull() )
    {
      for ( int i = 0; i < extras->GetNumberOfElements(); ++i )
      {
        gp_Pnt Pi = extras->GetElement(i);

        pts->AddPoint( cascade::GetMobiusPnt(Pi) );
      }
    }
  }
}

//-----------------------------------------------------------------------------

asiAlgo_AppSurf2::asiAlgo_AppSurf2(ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_iNumDiscrPts    ( 10 ),
  m_fFairCoeff      ( 0.01 )
{}

//-----------------------------------------------------------------------------

void asiAlgo_AppSurf2::SetExtraPoints(const Handle(asiAlgo_BaseCloud<double>)& points)
{
  m_extraPts = points;
}

//-----------------------------------------------------------------------------

bool asiAlgo_AppSurf2::Build(const Handle(TopTools_HSequenceOfShape)& edges,
                             Handle(Geom_BSplineSurface)&             support,
                             TopoDS_Face&                             face)
{
  /* ==============================
   *  STAGE 1: prepare constraints.
   * ============================== */

  t_ptr<t_pcloud> pts = new t_pcloud;

  ::fillConstraints(edges, m_extraPts, pts);

  // Fill constraints for giving them back.
  m_pinPts = new asiAlgo_BaseCloud<double>;
  //
  for ( int k = 0; k < pts->GetNumberOfPoints(); ++k )
    m_pinPts->AddElement( cascade::GetOpenCascadePnt( pts->GetPoint(k) ) );

  /* ======================
   *  STAGE 2: approximate.
   * ====================== */

  TIMER_NEW
  TIMER_GO

  // Prepare approximation tool.
  geom_ApproxBSurf approx(pts, 3, 3);

  // Optional initial surface.
  if ( !m_initSurf.IsNull() )
  {
    Handle(Geom_BSplineSurface)
      bsurf = GeomConvert::SurfaceToBSplineSurface(m_initSurf);

    approx.SetInitSurface( cascade::GetMobiusBSurface(bsurf) );
  }

  // Approximate.
  try
  {
    if ( !approx.Perform(m_fFairCoeff) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Approximation with APPSURF2 failed.");
      return false;
    }
  }
  catch ( ... )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Approximation with APPSURF2 failed.");
    return false;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(m_progress, "APPSURF2")

  // Get result.
  t_ptr<t_bsurf> mobResSurf = approx.GetResult();

  // Convert to OpenCascade B-surface.
  support = cascade::GetOpenCascadeBSurface(mobResSurf);

  /* ====================
   *  STAGE 3: make face.
   * ==================== */

  if ( !edges->IsEmpty() )
  {
    // To get rid of `const`.
    Handle(TopTools_HSequenceOfShape)
      unsortedEdges = new TopTools_HSequenceOfShape;
    //
    for ( TopTools_SequenceOfShape::Iterator eit(*edges); eit.More(); eit.Next() )
      unsortedEdges->Append( eit.Value() );

    // Compose a new wire.
    Handle(TopTools_HSequenceOfShape) freeWires;
    ShapeAnalysis_FreeBounds::ConnectEdgesToWires(unsortedEdges, 1e-3, 0, freeWires);
    //
    const TopoDS_Wire& repatchW = TopoDS::Wire( freeWires->First() );

    // Build new face.
    TopoDS_Face newF = BRepBuilderAPI_MakeFace(support, repatchW, false);

    // Heal defects.
    ShapeFix_Shape shapeHealer(newF);
    shapeHealer.Perform();
    newF = TopoDS::Face( shapeHealer.Shape() );

    // Check if the face is valid.
    asiAlgo_CheckValidity checker;
    const double tol               = checker.GetMaxTolerance(face)*5.0;
    const bool   hasAllClosedWires = checker.HasAllClosedWires(face, tol);
    //
    if ( hasAllClosedWires )
      face = newF;
  }

  return true;
}
