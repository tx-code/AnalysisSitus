//-----------------------------------------------------------------------------
// Created on: 18 March 2023
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
#include "asiAlgo_AppSurfUtils.h"

// Mobius includes
#include <mobius/cascade.h>

// OpenCascade includes
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <GeomConvert.hxx>
#include <GeomLib.hxx>
#include <NCollection_CellFilter.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

using namespace mobius;

//-----------------------------------------------------------------------------

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

}

//-----------------------------------------------------------------------------

Handle(Geom_BSplineSurface)
  asiAlgo_AppSurfUtils::PrepareInitialPlane(const Handle(asiAlgo_BaseCloud<double>)& pts,
                                            const int                                numUKnots,
                                            const int                                numVKnots,
                                            const int                                degU,
                                            const int                                degV)
{
  /* =================
   *  Contract checks.
   * ================= */

  const int numPts = pts->GetNumberOfElements();

  if ( numPts < 4 )
    return nullptr;

  if ( (numUKnots < 2) || (numVKnots < 2) )
    return nullptr;

  /* ============
   *  Make plane.
   * ============ */

  // Convert points to array.
  TColgp_Array1OfPnt ptsArray(1, numPts);
  //
  for ( int i = 0; i < numPts; ++i )
    ptsArray(i + 1).ChangeCoord() = pts->GetElement(i);

  gp_Ax2 planeAx;
  bool   isSingular;
  //
  GeomLib::AxeOfInertia(ptsArray, planeAx, isSingular);
  //
  if ( isSingular )
    return nullptr;

  // Compute UV bounds of the points cloud on the plane.
  gp_Pln PlanInit(planeAx);
  const gp_XYZ& O  = PlanInit.Location().XYZ();
  const gp_XYZ& dX = PlanInit.XAxis().Direction().XYZ();
  const gp_XYZ& dY = PlanInit.YAxis().Direction().XYZ();
  double U, V, UMax, UMin, VMax, VMin;
  UMax = VMax = RealFirst();
  UMin = VMin = RealLast();
  //
  for ( int i = ptsArray.Lower(); i <= ptsArray.Upper(); ++i )
  {
    const gp_XYZ theV( ptsArray(i).XYZ() - O );
    U = theV.Dot(dX);
    V = theV.Dot(dY);
    if ( UMax < U ) UMax = U;
    if ( UMin > U ) UMin = U;
    if ( VMax < V ) VMax = V;
    if ( VMin > V ) VMin = V;
  }

  // Compute correction of UV bounds.
  const double DeltaU = 1.e-3 * (UMax - UMin);
  const double DeltaV = 1.e-3 * (VMax - VMin);
  UMax += DeltaU;
  UMin -= DeltaU;
  VMax += DeltaV;
  VMin -= DeltaV;

  // Create trimmed plane and convert it to B-spline.
  Handle(Geom_Surface)
    SurfInit = new Geom_RectangularTrimmedSurface(new Geom_Plane(PlanInit), UMin, UMax, VMin, VMax);
  //
  Handle(Geom_BSplineSurface)
    SurfRes = GeomConvert::SurfaceToBSplineSurface(SurfInit);

  SurfRes->IncreaseDegree(degU, degV);

  /* ===========
   *  Add knots.
   * =========== */

  // Insert knots if needed
  if ( numUKnots > 2 )
  {
    const double            ParaU = UMax - UMin;
    TColStd_Array1OfReal    UKnotB(1, numUKnots - 2);
    TColStd_Array1OfInteger UMultB(1, numUKnots - 2);
    //
    for ( int i = 1; i <= numUKnots - 2; ++i )
    {
      UKnotB.SetValue( i, UMin + (ParaU*i)/(double)(numUKnots - 1) );
      UMultB.SetValue( i, 1 );
    }
    SurfRes->InsertUKnots(UKnotB, UMultB);
  }
  if ( numVKnots > 2 )
  {
    const double            ParaV = VMax - VMin;
    TColStd_Array1OfReal    VKnotB(1, numVKnots - 2);
    TColStd_Array1OfInteger VMultB(1, numVKnots - 2);
    //
    for ( int i = 1; i <= numVKnots - 2; ++i )
    {
      VKnotB.SetValue( i, VMin + (ParaV*i)/(double)(numVKnots - 1) );
      VMultB.SetValue( i, 1);
    }
    SurfRes->InsertVKnots(VKnotB, VMultB);
  }

  return SurfRes;
}

//-----------------------------------------------------------------------------

void asiAlgo_AppSurfUtils::PrepareConstraints(const double                             prec,
                                              const Handle(TopTools_HSequenceOfShape)& edges,
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

    GCPnts_UniformAbscissa splitter( curve, prec, Precision::Confusion() );
    int pointCount = splitter.IsDone() ? splitter.NbPoints() : 2;
    std::vector<double> params;

    params.push_back(f);
    for ( int aJ = 2; aJ < pointCount; ++aJ )
    {
      double param = splitter.Parameter(aJ);
      params.push_back(param);
    }
    params.push_back(l);

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
