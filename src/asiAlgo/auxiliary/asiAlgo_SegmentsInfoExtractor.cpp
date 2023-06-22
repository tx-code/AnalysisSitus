//-----------------------------------------------------------------------------
// Created on: 24 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Kiselev
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
#include <asiAlgo_SegmentsInfoExtractor.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <Geom_Curve.hxx>
#include <GProp_GProps.hxx>
#include <ShapeExtend_WireData.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

namespace
{
  struct curveData
  {
    Handle(Geom_Curve) C;
    double f, l;

    curveData()
      : f( 0.0 ),
        l( 0.0 )
    {}

    curveData(const TopoDS_Edge& E)
    {
      C = BRep_Tool::Curve( E, f, l );
    }

    gp_Pnt GetMidPoint() const
    {
      gp_Pnt mp;
      C->D0( ( l + f ) / 2., mp );

      return mp;
    }
  };

  gp_Dir GetNormalForWire(const TopoDS_Wire& W)
  {
    std::vector< curveData > dataVec;

    TopExp_Explorer exp( W, TopAbs_EDGE );
    for ( ; exp.More(); exp.Next() )
    {
      curveData data( TopoDS::Edge( exp.Current() ) );

      if ( !data.C.IsNull() )
        dataVec.push_back( data );

      if ( dataVec.size() == 2 )
      {
        break;
      }
    }

    if ( dataVec.empty() )
    {
      return gp::DZ();
    }

    gp_Pnt p1, p2, p3;

    if ( dataVec.size() == 1 )
    {
      // Single curve could be only in case of circle/arc.
      const curveData& data = dataVec[0];

      p1 = data.GetMidPoint();

      const double delta = ( data.l - data.f ) / 4.;

      data.C->D0( data.f + delta,      p2 );
      data.C->D0( data.f + delta * 3., p3 );
    }
    else
    {
      const curveData& data0 = dataVec[0];
      const curveData& data1 = dataVec[1];

      data0.C->D0( data0.f, p1 );

      p2 = data0.GetMidPoint();
      p3 = data1.GetMidPoint();
    }

    gp_Vec v1( p1, p2 );
    gp_Vec v2( p1, p3 );

    if ( v1.IsParallel( v2, Precision::Confusion() ) )
    {
      return gp::DZ();
    }

    return gp_Vec( p1, p2 ).Crossed( gp_Vec( p1, p3 ) );
  }
}

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfoExtractor::asiAlgo_SegmentsInfoExtractor()
  : m_tol( 1.e-3 )
{}

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfoExtractor::~asiAlgo_SegmentsInfoExtractor()
{
  m_infoVec.clear();
}

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfoExtractor::asiAlgo_SegmentsInfoExtractor(const TopoDS_Wire& wire)
  : asiAlgo_SegmentsInfoExtractor( wire, GetNormalForWire( wire ) )
{}

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfoExtractor::asiAlgo_SegmentsInfoExtractor(const TopoDS_Wire& wire,
                                                             const gp_Dir&      normal)
  : asiAlgo_SegmentsInfoExtractor()
{
  // Remove previous results.
  m_infoVec.clear();

  ShapeExtend_WireData wData( wire );

  bool isSingleSegment = wData.NbEdges() == 1;

  for ( int i = 1; i <= wData.NbEdges(); ++i )
  {
    // Get current edge.
    TopoDS_Edge edge = wData.Edge( i );

    // Get curve.
    double U1 = 0., U2 = 0.;
    Handle(Geom_Curve) curve = BRep_Tool::Curve( edge, U1, U2 );

    if ( curve.IsNull() )
      continue;

    // Get next segment id.
    int nextSegmentId = i < wData.NbEdges() ? ( i + 1 )
                                              : 1;

    // Get next edge.
    TopoDS_Edge nextEdge = wData.Edge( nextSegmentId );

    // Get edge length.
    GProp_GProps nvsProps;
    BRepGProp::LinearProperties( edge, nvsProps );

    asiAlgo_SegmentsInfo info( i, asiAlgo_Utils::CurveName( curve ), nvsProps.Mass() );

    // Handle circular curves.
    bool isCircular = asiAlgo_Utils::IsCircular( curve );

    if ( isCircular )
    {
      double edgeRadius = 0.0;
      double edgeAngle  = 0.0;
      angularity( edge, edgeRadius, edgeAngle );

      edgeAngle = angleToDegree( edgeAngle );

      info.radius = edgeRadius;
      info.angle = edgeAngle;
    }

    if ( !isSingleSegment )
    {
      info.nextSegment = nextSegmentId;
      info.angleToNextSegment = angleToDegree( angleTo( edge, nextEdge, normal ) );
    }

    m_infoVec.push_back( info );
  }
}

//-----------------------------------------------------------------------------

double asiAlgo_SegmentsInfoExtractor::angleToDegree(const double& angle)
{
  return 180. * angle / M_PI;
}

//-----------------------------------------------------------------------------

double asiAlgo_SegmentsInfoExtractor::angleTo(const TopoDS_Edge& edge,
                                              const TopoDS_Edge& nextEdge,
                                              const gp_Dir&      normal)
{
  Handle(Geom_Curve) curve;
  double U1 = 0., U2 = 0.;

  curve = BRep_Tool::Curve( edge, U1, U2 );

  if ( curve.IsNull() )
    return 0.0;

  Handle(Geom_Curve) nextCurve;
  double nextU1 = 0., nextU2 = 0.;

  nextCurve = BRep_Tool::Curve( nextEdge, nextU1, nextU2 );

  if ( nextCurve.IsNull() )
    return 0.0;

  gp_Pnt firstPoint  = curve->Value( U1 );
  gp_Pnt secondPoint = curve->Value( U2 );

  gp_Pnt nextFirstPoint  = nextCurve->Value( nextU1 );
  gp_Pnt nextSecondPoint = nextCurve->Value( nextU2 );

  gp_Pnt commonPoint;

  if ( firstPoint.Distance( nextFirstPoint  ) < m_tol ||
       firstPoint.Distance( nextSecondPoint ) < m_tol )
  {
    commonPoint = firstPoint;
  }
  else if ( secondPoint.Distance( nextFirstPoint  ) < m_tol ||
            secondPoint.Distance( nextSecondPoint ) < m_tol )
  {
    commonPoint = secondPoint;
  }
  else
  {
    return 0.0;
  }

  gp_Dir curveDir, nextCurveDir;

  if ( !curveDirection( curve, commonPoint, firstPoint, secondPoint, curveDir ) ||
       !curveDirection( nextCurve, commonPoint, nextFirstPoint, nextSecondPoint, nextCurveDir ) )
  {
    return 0.0;
  }

  return nextCurveDir.AngleWithRef( curveDir, normal );
}

//-----------------------------------------------------------------------------

bool asiAlgo_SegmentsInfoExtractor::curveDirection(const Handle(Geom_Curve)& curve,
                                                   const gp_Pnt&             commonPoint,
                                                   const gp_Pnt&             firstPoint,
                                                   const gp_Pnt&             secondPoint,
                                                   gp_Dir&                   curveDir)
{
  if ( curve->IsKind( STANDARD_TYPE(Geom_Line) ) )
  {
    bool isCurveFistCommon = firstPoint.Distance(commonPoint) < m_tol;

    curveDir = isCurveFistCommon ? gp_Dir( gp_Vec( firstPoint,  secondPoint ) )
                                 : gp_Dir( gp_Vec( secondPoint, firstPoint  ) );
  }
  else if ( curve->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    Handle(Geom_Circle) circCurve = Handle(Geom_Circle)::DownCast( curve );

    gp_Circ circ = circCurve->Circ();
    gp_Pnt centerPoint = circ.Location();

    gp_Dir circleNormal = circ.Axis().Direction();
    gp_Dir dirFromCenter( gp_Vec( centerPoint, commonPoint ) );

    curveDir = dirFromCenter.Crossed( circleNormal );
  }
  else if ( curve->IsInstance( STANDARD_TYPE( Geom_TrimmedCurve) ) )
  {
    Handle(Geom_TrimmedCurve) tcurve =
      Handle(Geom_TrimmedCurve)::DownCast( curve );

    if ( tcurve->BasisCurve()->IsKind( STANDARD_TYPE(Geom_Circle) ) )
    {
      curveDirection( tcurve->BasisCurve(),
                      commonPoint,
                      firstPoint,
                      secondPoint,
                      curveDir );
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_SegmentsInfoExtractor::curveAngularity(const Handle(Geom_Curve)& curve,
                                                    const double              U1,
                                                    const double              U2,
                                                    double&                   edgeRadius,
                                                    double&                   edgeAngle)
{
  edgeRadius = 0.0;
  edgeAngle  = 0.0;

  if ( curve->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    Handle(Geom_Circle) circCurve = Handle(Geom_Circle)::DownCast( curve );

    edgeRadius = circCurve->Circ().Radius();
    edgeAngle  = U2 - U1;
  }
  else if ( curve->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
  {
    Handle(Geom_TrimmedCurve) tcurve = Handle(Geom_TrimmedCurve)::DownCast( curve );

    if ( tcurve->BasisCurve()->IsKind( STANDARD_TYPE(Geom_Circle) ) )
    {
      curveAngularity( tcurve->BasisCurve(), U1, U2, edgeRadius, edgeAngle );
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_SegmentsInfoExtractor::angularity(const TopoDS_Edge& edge,
                                               double&            edgeRadius,
                                               double&            edgeAngle)
{
  double U1 = 0., U2 = 0.;
  Handle(Geom_Curve) curve = BRep_Tool::Curve( edge, U1, U2 );

  if ( curve.IsNull() )
    return;

  curveAngularity( curve, U1, U2, edgeRadius, edgeAngle );
}
