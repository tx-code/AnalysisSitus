//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Kiselev
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
#include <asiAlgo_ConvertCurve.h>

// asiAlgo includes
#include <asiAlgo_AnalyzeWire.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <DBRep.hxx>
#include <ElCLib.hxx>
#include <Extrema_ExtElC.hxx>
#include <Extrema_POnCurv.hxx>
#include <GccAna_Circ2d3Tan.hxx>
#include <gce_MakeCirc.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomConvert.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <gp_Lin.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Wire.hxx>
#include <TopoDS.hxx>

// Standard includes
#include <vector>

//-----------------------------------------------------------------------------

namespace
{
  // Structure to represent an arc or a line segment.
  struct arcInfo
  {
    gp_Pnt m_p0; // Start point.
    gp_Pnt m_p1; // End point.

    gp_Circ m_c;

    double m_t0;
    double m_t1;

    // Ctor for line.
    arcInfo(const gp_Pnt& p0, const gp_Pnt& p1, double t0, double t1)
      : m_p0( p0 ),
        m_p1( p1 ),
        m_t0( t0 ),
        m_t1( t1 )
    {
      MarkAsLine();
    }

    // Ctor for circle arc.
    arcInfo(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Circ& c, double t0, double t1)
      : m_p0( p0 ),
        m_p1( p1 ),
        m_c ( c  ),
        m_t0( t0 ),
        m_t1( t1 )
    {}

    bool IsLine() const
    {
      return m_c.Radius() == 0.0;
    }

    void MarkAsLine()
    {
      m_c.SetRadius( 0.0 );
    }
  };

  //-----------------------------------------------------------------------------

  template<typename T>
  inline bool CheckRes(GeomAdaptor_Curve& adaptor,
                       const double       f,
                       const double       l,
                       const double       t,
                       T&                 res,
                       const int          nbPts = 20)
  {
    // Check the result.
    const double du = ( l - f ) / nbPts;

    for ( int i = 1; i <= nbPts; ++i )
    {
      double u = f + ( du * i );

      gp_Pnt PP = adaptor.Value( u );

      double dist = res.Distance( PP );

      if ( dist > t )
      {
        return false;
      }
    }

    return true;
  }

  //-----------------------------------------------------------------------------

  inline double getNewParameterAndPoint(GeomAdaptor_Curve& adaptor,
                                        const double       f,
                                        const double       l,
                                        const gp_Pnt&      p,
                                        gp_Pnt&            projP)
  {
    double dIP = 0.0;

    ShapeAnalysis_Curve analyzer;
    analyzer.Project( adaptor.Curve(), p, Precision::Confusion(), projP, dIP, f, l, false );

    return dIP;
  }

  //-----------------------------------------------------------------------------

  inline void CalcNewCirc(const gp_Pnt& pCommon,
                          const gp_Pnt& p0,
                          const gp_Pnt& p1,
                          gp_Circ&      c)
  {
    gp_Vec p1p0Old( pCommon, p0 );
    gp_Vec p1p0New( pCommon, p1 );

    gp_Pnt pMidOld = pCommon.Translated( p1p0Old / 2. );
    gp_Pnt pMidNew = pCommon.Translated( p1p0New / 2. );

    const double h0 = c.Radius() - pMidOld.Distance( c.Location() );

    gp_Vec vH( c.Location(), pMidOld );
    vH.Normalize();
    vH.Scale( h0 );

    vH.Rotate( gp_Ax1( c.Location(), c.Axis().Direction() ), p1p0Old.Angle( p1p0New ) );

    gp_Pnt pH = pMidNew.Translated( vH );

    gp_Circ newC = gce_MakeCirc( p1, pH, pCommon );

    if ( newC.Axis().Direction().IsOpposite( c.Axis().Direction(), Precision::Confusion() ) )
    {
      newC.SetAxis( gp_Ax1( newC.Location(),c.Axis().Direction() )  );
    }

    c = newC;
  }

  //-----------------------------------------------------------------------------

  enum SegmentsTypes
  {
    SegmentsTypes_Undefined,
    SegmentsTypes_TwoLines,
    SegmentsTypes_TwoArcs,
    SegmentsTypes_LineAndArc,
    SegmentsTypes_ArcAndLine
  };

  //-----------------------------------------------------------------------------

  // Tries to convert a curve 'c' between 'f' and 'l' parameters using the passed
  // tolerance 't'.
  inline bool HandleArc(GeomAdaptor_Curve&      adaptor,
                        const double            f,
                        const double            l,
                        const double            t,
                        std::vector< arcInfo >& arcs)
  {
    // Get first and last points.
    gp_Pnt p0 = adaptor.Value( f );
    gp_Pnt p1 = adaptor.Value( l );

    // Avoid zero length segments. Just continue the convesion.
    if ( p0.IsEqual( p1, Precision::Confusion() ) )
    {
      return true;
    }

    bool isLine = false;

    gp_Pnt p0new, p1new;
    gp_Circ circ;

    // Check the distance between points.
    const double d = p0.Distance( p1 );

    //-----------------------------------------------------------------------------
    // Too short curve. Replace it by the line.
    //-----------------------------------------------------------------------------
    if ( d <= t )
    {
      p0new = p0;
      p1new = p1;

      isLine = true;
    }
    else
    {
      // Check if it is could be replaced just by a line.
      gp_Lin line( p0, gp_Vec( p0, p1 ) );

      // Get middle point on the initial curve.
      gp_Pnt PM = adaptor.Value( ( l + f ) / 2. );

      if ( line.Distance( PM ) <= t )
      {
        if ( CheckRes< gp_Lin >( adaptor, f, l, t, line ) )
        {
          p0new = p0;
          p1new = p1;

          isLine = true;
        }
      }

      // Finally, try to build a circle using passed tolerance.
      if ( !isLine )
      {
        if ( PM.IsEqual( p0, Precision::Confusion() ) ||
             PM.IsEqual( p1, Precision::Confusion() ) )
        {
          return false;
        }

        gp_Vec vPMP0( PM, p0 );
        gp_Vec vPMP1( PM, p1 );

        if ( vPMP0.IsParallel( vPMP1, Precision::Angular() ) )
        {
          return false;
        }

        // Move all staff to the XOY plane.
        gp_Trsf T;

        gp_Vec n = vPMP0.Crossed( vPMP1 );

        if ( !n.IsParallel( gp::DZ(), Precision::Angular() ) )
        {
          // B goes to global origin.
          gp_Trsf T_B;
          T_B.SetTransformation( gp_Ax3( PM, n ) );

          // Global origin goes to A.
          gp_Trsf T_A;
          T_A.SetTransformation( gp_Ax3( gp_Pnt(), gp::DZ() ) );
          T_A.Invert();

          // Final transformation from B to A.
          T = T_A * T_B;
        }

        gp_Pnt PFTranf( p0.Transformed( T ) );
        gp_Pnt PMTranf( PM.Transformed( T ) );
        gp_Pnt PLTranf( p1.Transformed( T ) );

        gp_Pnt2d PF2d( PFTranf.X(), PFTranf.Y() );
        gp_Pnt2d PM2d( PMTranf.X(), PMTranf.Y() );
        gp_Pnt2d PL2d( PLTranf.X(), PLTranf.Y() );

        // Build a circle throught 3 points using given tolerance.
        GccAna_Circ2d3Tan builder( PF2d, PM2d, PL2d, t );

        if ( !builder.IsDone() || builder.NbSolutions() != 1 )
        {
          return false;
        }

        // Return to the 3D space.
        gp_Circ2d c = builder.ThisSolution( 1 );

        circ = gp_Circ( gp_Ax2( gp_Pnt( c.Location().X(), c.Location().Y(), 0.0 ), gp::DZ() ).Transformed( T.Inverted() ), c.Radius() );

        // Check the resulting circle.
        if ( !CheckRes< gp_Circ >( adaptor, f, l, t, circ ) )
        {
          return false;
        }

        // Get new first/last points.
        double parSol1 = 0.0;
        double parSol3 = 0.0;

        double parAng1 = 0.0;
        double parAng3 = 0.0;

        gp_Pnt2d pSol1, pSol3;

        builder.Tangency1( 1, parSol1, parAng1, pSol1 );
        builder.Tangency3( 1, parSol3, parAng3, pSol3 );

        p0new = gp_Pnt( pSol1.X(), pSol1.Y(), 0.0 ).Transformed( T.Inverted() );
        p1new = gp_Pnt( pSol3.X(), pSol3.Y(), 0.0 ).Transformed( T.Inverted() );

        if ( !circ.Axis().Direction().IsOpposite( n, Precision::Confusion() ) )
        {
          circ.SetAxis( circ.Axis().Reversed() );
        }
      }
    }

    //-----------------------------------------------------------------------------
    // Handle the first segment. Fix its location.
    //-----------------------------------------------------------------------------
    if ( arcs.empty() )
    {
      if ( isLine )
      {
        arcs.push_back( arcInfo( p0, p1new, f, l ) );
      }
      else
      {
        // Fix gap.
        if ( p0new.Distance( p0 ) > Precision::Confusion() )
        {
          CalcNewCirc( p1new, p0new, p0, circ );

          arcs.push_back( arcInfo( p0, p1new, circ, f, l ) );
        }
        else
        {
          // No gap, add a new circle.
          arcs.push_back( arcInfo( p0new, p1new, circ, f, l ) );
        }
      }

      return true;
    }

    //-----------------------------------------------------------------------------
    // Get previuos segment.
    //-----------------------------------------------------------------------------
    arcInfo& last = arcs.back();

    // Check if they have a gap.
    double dPrev = last.m_p1.Distance( p0new );

    const bool hasGap = dPrev > Precision::Confusion();

    if ( hasGap && !isLine )
    {
      // Rotate the arc to fix the gap.
      CalcNewCirc( p1new, p0new, last.m_p1, circ );
    }

    // Try to extend previous segment.
    if ( isLine ) // Line.
    {
      gp_Vec vCur( p0new, p1new );
      gp_Vec vPrev( last.m_p0, last.m_p1 );

      // Try to extend previous line.
      if ( last.IsLine() && vPrev.IsParallel( vCur, 0.0001 ) && !vPrev.IsOpposite( vCur, 0.0001 ) )
      {
        last.m_p1 = p1new;

        return true;
      }
    }
    else // Circle.
    {
      // Try to extend previous arc.
      if ( last.m_c.Location().Distance( circ.Location() ) < Precision::Confusion() ) // < t ?
      {
        last.m_p1 = p1new;

        return true;
      }
    }

    //-----------------------------------------------------------------------------
    // Special check for corners.
    //-----------------------------------------------------------------------------
    // Check if 'penult', 'last' and 'segment to add' fit initial curve's corner.
    if ( arcs.size() > 1 )
    {
      arcInfo& penult = arcs[ arcs.size() - 2 ];

      const bool isPenultLine = penult.IsLine();

      gp_Lin curL( last.m_p1, gp_Vec( last.m_p1, p1new ) );
      gp_Lin penultL( penult.m_p0, gp_Vec( penult.m_p0, penult.m_p1 ) );

      gp_Circ curC( circ );
      gp_Circ penultC( penult.m_c  );

      Extrema_ExtElC extr;

      // Get type of intersection.
      SegmentsTypes segsTypes = SegmentsTypes_Undefined;

      if ( isLine == isPenultLine ) // Two lines or arcs.
      {
        if ( isLine ) // Two lines.
        {
          segsTypes = SegmentsTypes_TwoLines;
          extr = Extrema_ExtElC ( curL, penultL, Precision::Confusion() );
        }
        else // Two arcs.
        {
          segsTypes = SegmentsTypes_TwoArcs;
          extr = Extrema_ExtElC ( curC, penultC );
        }
      }
      else // Line and arc.
      {
        if ( isLine ) // Cur is line, penult is arc.
        {
          segsTypes = SegmentsTypes_LineAndArc;
          extr = Extrema_ExtElC ( curL, penultC, Precision::Confusion() );
        }
        else // Cur is arc, penult is line.
        {
          segsTypes = SegmentsTypes_ArcAndLine;
          extr = Extrema_ExtElC ( penultL, curC, Precision::Confusion() );
        }
      }

      // Check intersection results.
      if ( extr.IsDone() && !extr.IsParallel() && extr.NbExt() > 0 )
      {
        const int nbSol = extr.NbExt();

        bool isOk = false;

        // Check if they have some intersection point.
        for ( int i = 1; i <= nbSol; ++i )
        {
          if ( extr.SquareDistance(i) < Precision::Confusion() )
          {
            isOk = true;
            break;
          }
        }

        gp_Pnt pIP;

        if ( isOk )
        {
          // Get intersection point 'IP' and check if segments
          // [penult.m_p1; IP] and [IP; p1new] are located on the bspline curve.
          Extrema_POnCurv IP, IP2;

          gp_Pnt pIP2;

          if ( nbSol == 1 )
          {
            extr.Points( 1, IP, IP );

            pIP = IP.Value();
            pIP2 = pIP;
          }
          else
          {
            // Find the point of interest.
            double minDist = DBL_MAX;
            double dist = 0.0;
            int indexOfInterest = -1;

            for ( int i = 1; i <= nbSol; ++i )
            {
              dist = extr.SquareDistance(i);

              if ( dist < Precision::Confusion() )
              {
                extr.Points( i, IP, IP );

                pIP = IP.Value();

                dist = pIP.Distance( penult.m_p1 );

                if ( minDist > dist )
                {
                  minDist = dist;
                  indexOfInterest = i;
                }
              }
            }

            extr.Points( indexOfInterest, IP, IP2 );

            pIP = IP.Value();
            pIP2 = IP2.Value();

            isOk = pIP.IsEqual( pIP2, Precision::Confusion() );

            if ( isOk )
            {
              // Check resulting points to be located on the prepared arcs and lines.
              switch ( segsTypes )
              {
                case SegmentsTypes::SegmentsTypes_ArcAndLine:
                {
                  if ( curC.Distance( pIP2 ) > Precision::Confusion() ||
                       penultL.Distance( pIP ) > Precision::Confusion() )
                  {
                    isOk = false;
                  }
                  break;
                }
                case SegmentsTypes::SegmentsTypes_LineAndArc:
                {
                  if ( curL.Distance( pIP ) > Precision::Confusion() ||
                       penultC.Distance( pIP2 ) > Precision::Confusion() )
                  {
                    isOk = false;
                  }
                  break;
                }
                case SegmentsTypes::SegmentsTypes_TwoArcs:
                {
                  if ( curC.Distance( pIP ) > Precision::Confusion() ||
                       penultC.Distance( pIP2 ) > Precision::Confusion() )
                  {
                    isOk = false;
                  }
                  break;
                }
                case SegmentsTypes::SegmentsTypes_TwoLines:
                {
                  if ( curL.Distance( pIP ) > Precision::Confusion() ||
                       penultL.Distance( pIP2 ) > Precision::Confusion() )
                  {
                    isOk = false;
                  }
                  break;
                }
                case SegmentsTypes::SegmentsTypes_Undefined:
                {
                  isOk = false;
                  break;
                }
              }
            }
          }
        }

        if ( isOk && ( penult.m_p0.IsEqual( pIP, Precision::Confusion() ) ||
                       pIP.IsEqual( p1new, Precision::Confusion() ) ) )
        {
          isOk = false;
        }

        if ( isOk )
        {
          const double halfTol = t / 2.;

          //-----------------------------------------------------------------------------
          // Extend.
          //-----------------------------------------------------------------------------
          // Intersection point is located on a segment which we are going to add.
          if ( pIP.IsEqual( p0new, Precision::Confusion() ) )
          {
            if ( isPenultLine )
            {
              gp_Lin newL( penult.m_p0, gp_Vec( penult.m_p0, pIP ) );

              isOk = CheckRes< gp_Lin >( adaptor, penult.m_t1, f, halfTol, newL );
            }
            else
            {
              isOk = CheckRes< gp_Circ >( adaptor, penult.m_t1, f, halfTol, penult.m_c );
            }

            if ( isOk )
            {
              // Extend penult.
              penult.m_p1 = pIP;
              penult.m_t1 = f;

              // Replace 'last' by the segment we are going to add.
              last.m_p0 = pIP;
              last.m_p1 = p1new;
              last.m_t0 = f;
              last.m_t1 = l;

              if ( isLine )
              {
                last.MarkAsLine();
              }
              else
              {
                last.m_c = circ;
              }

              return true;
            }
          }
          // Intersection point is located on a penult segment.
          else if ( pIP.IsEqual( penult.m_p1, Precision::Confusion() ) )
          {
            // Extend segment we are going to add.
            if ( isLine ) // Current segment is a line.
            {
              gp_Lin newL( p1new, gp_Vec( p1new, pIP ) );

              isOk = CheckRes< gp_Lin >( adaptor, penult.m_t1, f, halfTol, newL );
            }
            else // Penult segment is an arc.
            {
              isOk = CheckRes< gp_Circ >( adaptor, penult.m_t1, f, halfTol, circ );
            }

            if ( isOk )
            {
              // Replace 'last' by the segment we are going to add and extend it till the pIP.
              last.m_p0 = pIP;
              last.m_p1 = p1new;
              last.m_t1 = l;
              last.m_t0 = penult.m_t1;

              if ( isLine )
              {
                last.MarkAsLine();
              }
              else
              {
                last.m_c = circ;
              }

              return true;
            }
          }
          // Intersection point is located on their cross point.
          // Extend penult segment till this point, update 'last' segment.
          else
          {
            gp_Pnt projP;
            double tIP1 = 0.0;

            ShapeAnalysis_Curve analyzer;
            analyzer.Project( adaptor.Curve(), pIP, Precision::Confusion(), projP, tIP1, last.m_t0, last.m_t1, false );

            if ( pIP.Distance( projP ) < halfTol )
            {
              switch ( segsTypes )
              {
                case SegmentsTypes::SegmentsTypes_ArcAndLine:
                {
                  gp_Lin newL0( penult.m_p0, gp_Vec( penult.m_p0, pIP ) );

                  if ( CheckRes< gp_Lin >( adaptor, penult.m_t0, tIP1, halfTol, newL0 ) &&
                       CheckRes< gp_Circ >( adaptor, tIP1, f, halfTol, circ ) )
                  {
                    penult.m_p1 = pIP;
                    penult.m_t1 = tIP1;

                    last.m_p0 = pIP;
                    last.m_p1 = p1new;
                    last.m_t0 = tIP1;
                    last.m_t1 = l;
                    last.m_c  = circ;

                    return true;
                  }

                  break;
                }
                case SegmentsTypes::SegmentsTypes_LineAndArc:
                {
                  gp_Lin newL0( pIP, gp_Vec( pIP, p0new ) );

                  if ( CheckRes< gp_Lin >( adaptor, penult.m_t1, tIP1, halfTol, newL0 ) &&
                       CheckRes< gp_Circ >( adaptor, tIP1, l, halfTol, penult.m_c ) )
                  {
                    penult.m_p1 = pIP;
                    penult.m_t1 = tIP1;

                    last.m_p0 = pIP;
                    last.m_p1 = p1new;
                    last.m_t0 = tIP1;
                    last.m_t1 = l;
                    last.MarkAsLine();

                    return true;
                  }

                  break;
                }
                case SegmentsTypes::SegmentsTypes_TwoArcs:
                {
                  if ( CheckRes< gp_Circ >( adaptor, penult.m_t1, tIP1, t / 2., penult.m_c ) &&
                       CheckRes< gp_Circ >( adaptor, tIP1, f, t / 2., circ ) )
                  {
                    penult.m_p1 = pIP;
                    penult.m_t1 = tIP1;

                    last.m_p0 = pIP;
                    last.m_p1 = p1new;
                    last.m_t0 = tIP1;
                    last.m_t1 = l;
                    last.m_c = circ;

                    return true;
                  }

                  break;
                }
                case SegmentsTypes::SegmentsTypes_TwoLines:
                {
                  gp_Lin newL0( penult.m_p1, gp_Vec( penult.m_p1, pIP ) );
                  gp_Lin newL1( pIP,         gp_Vec( pIP,  p0new ) );

                  if ( CheckRes< gp_Lin >( adaptor, penult.m_t1, tIP1, t / 2., newL0 ) &&
                       CheckRes< gp_Lin >( adaptor, tIP1, f, t / 2., newL1 ) )
                  {
                    penult.m_p1 = pIP;
                    penult.m_t1 = tIP1;

                    last.m_p0 = pIP;
                    last.m_p1 = p1new;
                    last.m_t0 = tIP1;
                    last.m_t1 = l;
                    last.MarkAsLine();

                    return true;
                  }

                  break;
                }
                case SegmentsTypes::SegmentsTypes_Undefined:
                {
                  return false; break;
                }
              }
            }
          }
        }
      }
    }

    // Just add a new segment.
    // Do not use 'last' here because it could be corrupted.
    if ( isLine ) // Line.
    {
      arcs.push_back( arcInfo( arcs.back().m_p1, p1new, f, l ) );
    }
    else // Circle.
    {
      arcs.push_back( arcInfo( arcs.back().m_p1, p1new, circ, f, l ) );
    }

    return true;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCurve::Perform(const Handle(Geom_Curve)& c,
                                   const double              f,
                                   const double              l,
                                   TopoDS_Wire&              w,
                                   double                    t)
{
  // Skip invalid curves.
  if ( c.IsNull() )
  {
    return false;
  }

  // Skip already 'good' types of curves.
  if ( asiAlgo_Utils::IsTypeOf<Geom_Line>(c) ||
       asiAlgo_Utils::IsTypeOf<Geom_Circle>(c) )
  {
    return true;
  }

  Handle(Geom_TrimmedCurve) trim;
  if ( c->DynamicType() != STANDARD_TYPE( Geom_TrimmedCurve ) )
  {
    trim = new Geom_TrimmedCurve( c, f, l );
  }
  else
  {
    trim = Handle(Geom_TrimmedCurve)::DownCast( c );
  }

  // Get a BSpline curve from the input curve.
  Handle(Geom_BSplineCurve) bspline =
    GeomConvert::CurveToBSplineCurve( trim );

  GeomAdaptor_Curve adaptorCurve;
  adaptorCurve.Load( bspline,
                     bspline->FirstParameter(),
                     bspline->LastParameter() );

  GCPnts_UniformAbscissa splitter( adaptorCurve, 3 );

  bool isOk = false;
  double u0 = 0.0, u1 = 0.0;
  double finalL = 0.0;

  std::vector< arcInfo > arcs;

  for ( int i = 2; i <= splitter.NbPoints(); ++i )
  {
    u0 = splitter.Parameter( i - 1 );
    u1 = splitter.Parameter( i );

    finalL = u1;

    while ( u0 < finalL )
    {
      try
      {
        isOk = HandleArc( adaptorCurve, u0, u1, t, arcs );
      }
      catch (...)
      {
        return false;
      }

      if ( !isOk )
      {
        u1 = ( u0 + u1 ) / 2.;
      }
      else
      {
        u0 = u1;
        u1 = finalL;
      }
    }
  }

  if ( arcs.empty() )
  {
    return false;
  }

  // Close path if necessary.
  gp_Pnt p;
  adaptorCurve.D0( adaptorCurve.LastParameter(), p );

  arcInfo& last = arcs.back();

  const double d = last.m_p1.Distance( p );

  if ( d > Precision::Confusion() )
  {
    if ( d > t )
    {
      arcs.push_back( arcInfo( last.m_p1, p, last.m_t1, adaptorCurve.LastParameter() ) );
    }
    else
    {
      if ( !last.IsLine() )
      {
        CalcNewCirc( last.m_p0, last.m_p1, p, last.m_c );
      }

      last.m_p1 = p;
    }
  }

  // Build edges and the final wire.
  ShapeExtend_WireData wdata;

  for ( const arcInfo& arc : arcs )
  {
    TopoDS_Shape edge = arc.IsLine() ? BRepBuilderAPI_MakeEdge( arc.m_p0, arc.m_p1 )
                                     : BRepBuilderAPI_MakeEdge( arc.m_c,  arc.m_p0, arc.m_p1 );

    wdata.Add( TopoDS::Edge( edge ) );
  }

  w = wdata.WireAPIMake();

  return true;
}

//-----------------------------------------------------------------------------

double asiAlgo_ConvertCurve::CheckGaps(const TopoDS_Wire&   w,
                                       ActAPI_ProgressEntry progress,
                                       ActAPI_PlotterEntry  plotter)
{
  asiAlgo_AnalyzeWire sw(progress, plotter);
  sw.Load(w);
  sw.CheckGaps3d();

  return sw.MaxDistance3d();
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCurve::FixGaps(const TopoDS_Wire&   input,
                                   const double         tol,
                                   TopoDS_Wire&         result,
                                   ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
{
  ShapeFix_Wire sfw;
  sfw.Load(input);
  sfw.ModifyGeometryMode() = true;
  sfw.FixEdgeCurvesMode() = true;
  sfw.FixLackingMode() = true;

  const bool isOk = sfw.FixGaps3d();
  result = sfw.Wire();

  return isOk;
}
