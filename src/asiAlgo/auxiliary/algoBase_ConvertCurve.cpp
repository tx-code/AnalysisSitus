//-----------------------------------------------------------------------------
// Created on: September 2021
// Created by: Sergey KISELEV
//-----------------------------------------------------------------------------

// Own include
#include <algoBase_ConvertCurve.h>

// OCCT includes
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <DBRep.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeLine.hxx>
#include <GC_MakeSegment.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI_IntSS.hxx>
#include <GeomConvert.hxx>
#include <GeomConvert_BSplineCurveToBezierCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <Standard_Assert.hxx>
#include <TopoDS.hxx>

// Standard includes
#include <vector>

#include <IntAna_QuadQuadGeo.hxx>

//-----------------------------------------------------------------------------

namespace
{
  bool intersect(const gp_Pln& plane1,
                 const gp_Pln& plane2,
                 gp_Lin&       line)
  {
    IntAna_QuadQuadGeo intSS(plane1, plane2, Precision::Angular(), Precision::Confusion());

    if ( !intSS.IsDone() || (intSS.NbSolutions() != 1) )
      return false;

    line = intSS.Line(1);

    return true;/*
    bool found = false;

    Handle(Geom_Plane) gp1 = new Geom_Plane( plane1 );
    Handle(Geom_Plane) gp2 = new Geom_Plane( plane2 );

    GeomAPI_IntSS intSS( gp1, gp2, Precision::Confusion() );

    if ( intSS.IsDone() )
    {
      int numSol = intSS.NbLines();

      if ( numSol > 0 )
      {
        Handle(Geom_Curve) curve = intSS.Line( 1 );

        line = Handle(Geom_Line)::DownCast( curve )->Lin();

        found = true;
      }
    }

    return found;*/
  }

  //-----------------------------------------------------------------------------

  void closestPointsOnLines(const gp_Lin& line1, const gp_Lin& line2, gp_Pnt& p1, gp_Pnt& p2)
  {
    // They might be the same point.
    gp_Vec v1( line1.Direction() );
    gp_Vec v2( line2.Direction() );

    gp_Vec v3( line2.Location(), line1.Location() );

    double a = v1 * v1;
    double b = v1 * v2;
    double c = v2 * v2;
    double d = v1 * v3;
    double e = v2 * v3;
    double D = a * c - b * b;

    double s, t;

    // D = (v1 x v2) * (v1 x v2)
    if ( D < Precision::Angular() )
    {
      // The lines are considered parallel.
      s = 0.0;
      t = ( b > c ? d / b
                  : e / c );
    }
    else
    {
      s = ( b * e - c * d ) / D;
      t = ( a * e - b * d ) / D;
    }

    p1 = line1.Location().XYZ() + s * v1.XYZ();
    p2 = line2.Location().XYZ() + t * v2.XYZ();
  }

  //-----------------------------------------------------------------------------

  bool calculateBiArcPoints(const gp_Pnt& p0,
                            gp_Vec        v_start,
                            const gp_Pnt& p4,
                            gp_Vec        v_end,
                            gp_Pnt&       p1,
                            gp_Pnt&       p2,
                            gp_Pnt&       p3)
  {
    if ( v_start.Magnitude() < Precision::Intersection() )
    {
      v_start = gp_Vec( p0, p1 );
    }

    if ( v_end.Magnitude() < Precision::Intersection() )
    {
      v_end = gp_Vec( p3, p4 );
    }

    v_start.Normalize();
    v_end.Normalize();

    gp_Vec v = p0.XYZ() - p4.XYZ();

    double a = 2.0 * ( v_start * v_end - 1.0 );
    double c = v * v;
    double b = ( v * 2.0 ) * ( v_start + v_end );

    if ( abs( a ) < Precision::Intersection() )
    {
      return false;
    }

    double d = b * b - 4.0 * a * c;

    if ( d < 0.0 )
    {
      return false;
    }

    double sd = sqrt( d );

    double e1 = ( -b - sd ) / ( 2.0 * a );
    double e2 = ( -b + sd ) / ( 2.0 * a );

    if ( e1 > 0.0 && e2 > 0.0 )
    {
      return false;
    }

    double e = e1;

    if ( e2 > e )
    {
      e = e2;
    }

    if ( e < 0 )
    {
      return false;
    }

    p1 = p0.XYZ() + v_start.XYZ() * e;
    p3 = p4.XYZ() - v_end.XYZ() * e;
    p2 = p1.XYZ() * 0.5 + p3.XYZ() * 0.5;

    return true;
  }

  //-----------------------------------------------------------------------------

  class TangentialArc
  {
    public:

      gp_Pnt m_p0; // start point
      gp_Vec m_v0; // start direction
      gp_Pnt m_p1; // end point
      gp_Pnt m_c;  // center point
      gp_Dir m_a;  // axis

      bool m_isLine;

    public:

      TangentialArc(const gp_Pnt& p0,
                    const gp_Vec& v0,
                    const gp_Pnt& p1)
        : m_p0( p0 ),
          m_v0( v0 ),
          m_p1( p1 )
      {
        /*static int callcount = 0; callcount++;

        if ( callcount % 1000 == 0 )
          std::cout << "TangentialArc cc = " << callcount << std::endl;*/

        // Calculate a tangential arc that goes through p0 and p1, with a direction of v0 at p0.
        m_isLine = !tangentialArc( m_p0, m_v0, m_p1, m_c, m_a );
      }

      TangentialArc(const gp_Pnt& p0,
                    const gp_Pnt& p1)
        : m_p0( p0 ),
          m_v0( gp_Vec( p0, p1 ) ),
          m_p1( p1 ),
          m_isLine( true )
      {}

      //-----------------------------------------------------------------------------

      bool IsLine() const
      {
        return m_isLine;
      }

      //-----------------------------------------------------------------------------

      bool IsRadiusEqual(const gp_Pnt& p,
                         double        tol) const
      {
        if ( m_isLine )
        {
          return true;
        }

        double point_radius = gp_Vec( m_c.XYZ() - p.XYZ() ).Magnitude();

        double diff =  abs( point_radius - Radius() );

        return diff <= tol;
      }

      //-----------------------------------------------------------------------------

      double Radius() const
      {
        double r0 = gp_Vec( m_p0.XYZ() - m_c.XYZ() ).Magnitude();
        double r1 = gp_Vec( m_p1.XYZ() - m_c.XYZ() ).Magnitude();

        double r = ( r0 + r1 ) / 2.0;

        return r;
      }

      //-----------------------------------------------------------------------------

    private:

      bool tangentialArc(const gp_Pnt& p0,
                         const gp_Vec& v0,
                         const gp_Pnt& p1,
                         gp_Pnt&       c,
                         gp_Dir&       axis)
      {
        if ( p0.Distance(p1) > Precision::Intersection() &&
             v0.Magnitude()  > Precision::Intersection() )
        {
          gp_Vec v1( p0, p1 );
          gp_Pnt halfway( p0.XYZ() + v1.XYZ() * 0.5 );

          gp_Pln pln1( halfway, v1 );
          gp_Pln pln2( p0, v0 );

          gp_Lin plane_line;

          if ( intersect( pln1, pln2, plane_line ) )
          {
            gp_Lin l1( halfway, v1 );

            gp_Pnt p2;


            closestPointsOnLines( plane_line, l1, c, p2 );

            axis = -( plane_line.Direction() );

            return true;
          }
        }

        return false;
      }
  };

  //-----------------------------------------------------------------------------

  void buildEdges(const std::vector< TangentialArc >& arcs,
                  TopTools_SequenceOfShape&           edges)
  {
    // Combine straight lines if possible and fill gaps in a curve.
    std::vector< TangentialArc > filtered;

    for ( auto arc : arcs )
    {
      if ( arc.m_p0.IsEqual( arc.m_p1, Precision::Confusion() ) )
      {
        continue;
      }

      // Check if the previous and the current curves are lines
      // and we can just unite them into signle one (to minimize the number of edges).
      if ( !filtered.empty() )
      {
        TangentialArc& prev = filtered.back();

        if ( prev.IsLine() && arc.IsLine() )
        {
          if ( gp_Vec( prev.m_p0, prev.m_p1 ).IsParallel( gp_Vec( arc.m_p0, arc.m_p1 ), 0.0001 ) )
          {
            prev.m_p1 = arc.m_p1;

            continue;
          }
        }

        if ( !prev.IsLine() && !arc.IsLine() )
        {
          if ( prev.m_c.IsEqual( arc.m_c, Precision::Confusion() ) &&
               abs( prev.Radius() - arc.Radius() ) < Precision::Confusion() )
          {
            prev.m_p1 = arc.m_p1;

            continue;
          }
        }
      }

      // Check if it is a line.
      if ( arc.IsLine() )
      {
        if ( !filtered.empty() )
        {
          TangentialArc& prev = filtered.back();

          arc.m_p0 = prev.m_p1;
        }

        filtered.push_back( arc );
      }
      // This is a circle.
      else
      {
        if ( !filtered.empty() )
        {
          TangentialArc& prev = filtered.back();

          // Add extra segment if points are too far.
          if ( !prev.m_p1.IsEqual( arc.m_p0, Precision::Confusion() ) )
          {
            filtered.push_back( TangentialArc( prev.m_p1, arc.m_p0 ) );
          }
        }

        filtered.push_back( arc );
      }
    }

    // Build edges.
    for ( auto arc : filtered )
    {
      if ( arc.m_p0.IsEqual( arc.m_p1, Precision::Confusion() ) )
      {
        continue;
      }

      if ( arc.IsLine() )
      {
        edges.Append( BRepBuilderAPI_MakeEdge( arc.m_p0, arc.m_p1 ) );
      }
      else
      {
        gp_Circ circ( gp_Ax2( arc.m_c, arc.m_a ), arc.Radius() );

        edges.Append( BRepBuilderAPI_MakeEdge( circ, arc.m_p0, arc.m_p1 ) );
      }
    }
  }

  //-----------------------------------------------------------------------------

  static inline void getD1(const Handle(Geom_Curve)& curve,
                           const double              u,
                           gp_Pnt&                   p,
                           gp_Vec&                   v,
                           const double              step)
  {
    static int callcount = 0; callcount++;
    curve->D1( u, p, v );

    /*if ( callcount % 1000 == 0 )
      std::cout << "callcount = " << callcount << std::endl;*/

    double tempT = u;
    gp_Pnt tempP;

    while ( v.Magnitude() < RealEpsilon() )
    {
      tempT = tempT + step;



      curve->D1( tempT, tempP, v );
    }
  }
}

//-----------------------------------------------------------------------------

bool
  pb::algoBase_ConvertCurve::Perform(const Handle(Geom_Curve)& curve,
                                     double                    first,
                                     double                    last,
                                     TopoDS_Wire&              wire,
                                     double                    tolerance)
{
  if ( curve.IsNull() )
  {
    return false;
  }

  const Handle(Standard_Type)& type = curve->DynamicType();

  Handle(Geom_BSplineCurve) bspline;

  // BSpline.
  if ( type == STANDARD_TYPE( Geom_BSplineCurve ) )
  {
    bspline = Handle(Geom_BSplineCurve)::DownCast( curve );
  }
  // Ellipse or Bezier.
  else if ( type == STANDARD_TYPE( Geom_Ellipse ) ||
            type == STANDARD_TYPE( Geom_BezierCurve ) )
  {
    Handle(Geom_TrimmedCurve) trimmed =
      new Geom_TrimmedCurve( curve, first, last );

    // Convert it to BSpline.
    bspline = GeomConvert::CurveToBSplineCurve( trimmed );
  }
  // Trimmed curve.
  else if ( type == STANDARD_TYPE( Geom_TrimmedCurve ) )
  {
    const Handle(Standard_Type)& baseType =
      Handle(Geom_TrimmedCurve)::DownCast( curve )->BasisCurve()->DynamicType();

    if ( baseType != STANDARD_TYPE( Geom_BSplineCurve ) &&
         baseType != STANDARD_TYPE( Geom_BezierCurve ) &&
         baseType != STANDARD_TYPE( Geom_Ellipse ) )
    {
      return false;
    }

    // Convert it to BSpline.
    bspline = GeomConvert::CurveToBSplineCurve( curve );
  }
  else
  {
    return false;
  }

  // Split curve to a Bezier arcs.
  GeomConvert_BSplineCurveToBezierCurve curvesConverter( bspline );

  std::vector< TangentialArc > arcsAndLines;

  const int n = curvesConverter.NbArcs();

  for ( int i = 1; i <= n; i++ )
  {
    Handle(Geom_BezierCurve) bCurve;

    double singleStep = 1.0;

    try
    {
      bCurve = curvesConverter.Arc( i );

      GeomAdaptor_Curve adaptorCurve;
      adaptorCurve.Load( bCurve, 0.0, 1.0 );

      const double length = GCPnts_AbscissaPoint::Length( adaptorCurve );

      singleStep = tolerance / length;

      if ( singleStep < Precision::Confusion() )
      {
        throw( "singleStep" );
      }
    }
    catch (...)
    {
      continue;
    }

    double t_start = 0.0;
    double t_end = 1.0;

    while ( t_start != 1.0 )
    {
      gp_Pnt p_start, p_end, pTmp;
      gp_Vec v_start, v_end;

      getD1( bCurve, t_start, p_start, v_start,  singleStep );
      getD1( bCurve, t_end,   p_end,   v_end,   -singleStep );

      if ( p_start.Distance( p_end ) < tolerance )
      {
        TangentialArc lArc( p_start, p_end );

        arcsAndLines.push_back( lArc );

        t_start = t_end;
        t_end = 1.0;

        continue;
      }

      gp_Pnt p1, p2, p3;

      bool can_do_spline_whole = calculateBiArcPoints( p_start, v_start, p_end, v_end, p1, p2, p3 );

      if ( can_do_spline_whole )
      {
        double U1 = t_start + ( ( t_end - t_start ) * 0.25 );
        double U2 = t_start + ( ( t_end - t_start ) * 0.75 );

        gp_Pnt p_middle1, p_middle2;
        gp_Vec v_middle1, v_middle2;

        getD1( bCurve, U1, p_middle1, v_middle1, singleStep );
        getD1( bCurve, U2, p_middle2, v_middle2, singleStep );

        TangentialArc arc1( p_start, v_start,                       p2    );
        TangentialArc arc2( p2,      gp_Vec( p3.XYZ() - p2.XYZ() ), p_end );

        bool isRadiusOK = arc1.IsRadiusEqual( p_middle1, tolerance ) &&
                          arc2.IsRadiusEqual( p_middle2, tolerance );

        if ( !isRadiusOK )
        {
          t_end = t_start + ( ( t_end - t_start ) * 0.5 );
        }
        else
        {
          arcsAndLines.push_back( arc1 );
          arcsAndLines.push_back( arc2 );

          t_start = t_end;
          t_end = 1.0;
        }
      }
      else
      {
        if ( p_start.Distance( p_end ) < tolerance )
        {
          TangentialArc lArc( p_start, p_end );

          arcsAndLines.push_back( lArc );

          t_start = t_end;
          t_end = 1.0;
        }
        else
        {
          t_end = t_start + ( ( t_end - t_start ) * 0.5 );
        }
      }
    }
  }

  // Build edges.
  TopTools_SequenceOfShape edges;

  buildEdges( arcsAndLines, edges );

  // Build wire.
  bool isDone = !edges.IsEmpty();

  if ( isDone )
  {
    ShapeExtend_WireData wdata;

    TopTools_SequenceOfShape::Iterator eIter( edges );
    for ( ; eIter.More(); eIter.Next() )
    {
      wdata.Add( TopoDS::Edge( eIter.Value() ) );
    }

    wire = wdata.WireAPIMake();
  }

  return isDone;
}
