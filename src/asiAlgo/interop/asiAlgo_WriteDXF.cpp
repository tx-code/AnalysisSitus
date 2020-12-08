/***************************************************************************
 *   Copyright (c) 2015 Yorik van Havre (yorik@uncreated.net)              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

// Own include
#include <asiAlgo_WriteDxf.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <Approx_Curve3d.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_HCurve.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>

//-----------------------------------------------------------------------------

namespace
{
  void gPntToTuple(const gp_Pnt& p,
                   double*       result)
  {
    result[0] = p.X();
    result[1] = p.Y();
    result[2] = p.Z();
  }

  point3D gPntTopoint3D(gp_Pnt& p)
  {
    point3D result;
    result.x = p.X();
    result.y = p.Y();
    result.z = p.Z();
    return result;
  }
}

//-----------------------------------------------------------------------------

asiAlgo_WriteDXF::asiAlgo_WriteDXF(const char*          filepath,
                                   ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
//
: CDxfWrite    (filepath, progress, plotter),
  m_fSegLength (1.)
{
  this->SetDxfVersion(DxfVersion_12); // R14 supports splines, hence no discretisation, and,
                                      //     therefore, better accuracy, higher performance,
                                      //     smaller file size.
                                      // R12 discretises splines, but its compatibility is
                                      //     better (FreeCAD 0.18 cannot import splines from
                                      //     DXF R14).
                                      //
                                      // Therefore, we set R12 as the default version since
                                      // it is highly interoperable.
}

//-----------------------------------------------------------------------------

bool asiAlgo_WriteDXF::CanOpen() const
{
  return !this->failed();
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::SetSegmentLength(const double val)
{
  m_fSegLength = val;
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::SetDxfVersion(const DxfVersion ver)
{
  switch ( ver )
  {
    case DxfVersion_0:
      m_version = 0;
      break;
    case DxfVersion_12:
      m_version = 12;
      break;
    case DxfVersion_14:
      m_version = 14;
      break;
    default: break;
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::SetDxfVersion(const int ver)
{
  DxfVersion verEnum = DxfVersion_0;

  if ( ver <= 0 )
    verEnum = DxfVersion_0;
  else if ( ver < 14 )
    verEnum = DxfVersion_12;
  else if ( ver >= 14 )
    verEnum = DxfVersion_14;

  this->setVersion(verEnum);
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::SetAutoOrient(const bool on)
{
  m_bAutoOrient = on;
}

//-----------------------------------------------------------------------------

bool asiAlgo_WriteDXF::Perform(const TopoDS_Shape& shape)
{
  this->init();
  //
  const bool
    isOk = this->exportShape(shape);
  //
  this->endRun();

  return isOk;
}

//-----------------------------------------------------------------------------

bool asiAlgo_WriteDXF::exportShape(const TopoDS_Shape& shape)
{
  TopoDS_Shape input = shape; // Reassign for relocation.

  if ( m_bAutoOrient )
  {
    gp_Ax3 drawingPlnAx( gp::XOY() );

    // Get the referene plane.
    gp_Ax3 fpAx3;
    for ( TopExp_Explorer fexp(input, TopAbs_FACE); fexp.More(); fexp.Next() )
    {
      const TopoDS_Face& face = TopoDS::Face( fexp.Current() );

      // We're looking for any plane.
      Handle(Geom_Plane) plane;
      //
      if ( asiAlgo_Utils::IsPlanar(face, plane) )
      {
        fpAx3 = plane->Position();
        break;
      }
    }

    // B goes to global origin.
    gp_Trsf T_B;
    T_B.SetTransformation(fpAx3);

    // Global origin goes to A.
    gp_Trsf T_A;
    T_A.SetTransformation(drawingPlnAx);
    T_A.Invert();

    // Final transformation from B to A.
    gp_Trsf T = T_A * T_B;

    // Transform.
    BRepBuilderAPI_Transform mkTransform(input, T, true);
    input = mkTransform.Shape();
  }

  // Export edges, no matter how they are organized (wires, compounds, etc.).
  TopExp_Explorer edges(input, TopAbs_EDGE);
  //
  for ( int i = 1; edges.More(); edges.Next(), ++i )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( edges.Current() );

    // Process different types of host geometry.
    // ...

    BRepAdaptor_Curve edgeAdt(edge);

    /* LINE */
    if ( edgeAdt.GetType() == GeomAbs_Line )
    {
      this->exportLine(edgeAdt);
    }

    /* CIRCLE */
    else if ( edgeAdt.GetType() == GeomAbs_Circle )
    {
      const double f = edgeAdt.FirstParameter();
      const double l = edgeAdt.LastParameter();
      const gp_Pnt s = edgeAdt.Value(f);
      const gp_Pnt e = edgeAdt.Value(l);

      if ( fabs(l - f) > 1.0 && s.SquareDistance(e) < 0.001 )
      {
        this->exportCircle(edgeAdt);
      }
      else
      {
        this->exportArc(edgeAdt);
      }
    }

    /* ELLIPSE */
    else if ( edgeAdt.GetType() == GeomAbs_Ellipse )
    {
      const double f = edgeAdt.FirstParameter();
      const double l = edgeAdt.LastParameter();
      const gp_Pnt s = edgeAdt.Value(f);
      const gp_Pnt e = edgeAdt.Value(l);

      if ( fabs(l - f) > 1.0 && s.SquareDistance(e) < 0.001 )
      {
        if ( m_polyOverride )
        {
          ( m_version >= 14 ) ? this->exportLWPoly(edgeAdt) : this->exportPolyline(edgeAdt);
        }
        else // no overrides, do what's right!
        {
          ( m_version < 14 ) ? this->exportPolyline(edgeAdt) : this->exportEllipse(edgeAdt);
        }
      }
      else // it's an arc
      {
        if ( m_polyOverride )
        {
          ( m_version >= 14 ) ? this->exportLWPoly(edgeAdt) : this->exportPolyline(edgeAdt);
        }
        else // no overrides, do what's right!
        {
          ( m_version < 14 ) ? this->exportPolyline(edgeAdt) : this->exportEllipseArc(edgeAdt);
        }
      }
    }

    /* B-CURVE */
    else if ( edgeAdt.GetType() == GeomAbs_BSplineCurve )
    {
      if ( m_polyOverride )
      {
        ( m_version >= 14 ) ? this->exportLWPoly(edgeAdt) : this->exportPolyline(edgeAdt);
      }
      else
      {
        ( m_version < 14 ) ? this->exportPolyline(edgeAdt) : this->exportBSpline(edgeAdt);
      }
    }

    /* BEZIER */
    else if ( edgeAdt.GetType() == GeomAbs_BezierCurve )
    {
      this->exportBezier(edgeAdt);
    }

    /* OTHER */
    else
    {
      m_progress.SendLogMessage( LogErr(Normal) << "ImpExpDxf - unknown curve type: %1."
                                                << edgeAdt.GetType() );
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportCircle(BRepAdaptor_Curve& c)
{
  gp_Circ       circ = c.Circle();
  const gp_Pnt& p    = circ.Location();
  const double  r    = circ.Radius();

  // Convert.
  double center[3] = {0, 0, 0};
  ::gPntToTuple(p, center);

  // Serialize.
  this->writeCircle(center, r);
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportArc(BRepAdaptor_Curve& c)
{
  gp_Circ       circ = c.Circle();
  const gp_Pnt& p    = circ.Location();

  double center[3] = {0, 0, 0};
  ::gPntToTuple(p, center);

  const double f = c.FirstParameter();
  const double l = c.LastParameter();
  gp_Pnt       s = c.Value(f);

  double start[3] = {0, 0, 0};
  ::gPntToTuple(s, start);

  gp_Pnt m = c.Value( (l + f)/2.0 );
  gp_Pnt e = c.Value(l);

  double end[3] = {0, 0, 0};
  ::gPntToTuple(e, end);

  gp_Vec v1(m, s);
  gp_Vec v2(m, e);
  gp_Vec v3(0, 0, 1);
  const double a = v3.DotCross(v1, v2);
  bool dir = (a < 0) ? true : false;

  this->writeArc(start, end, center, dir);
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportEllipse(BRepAdaptor_Curve& c)
{
  gp_Elips      ellp = c.Ellipse();
  const gp_Pnt& p    = ellp.Location();

  double center[3] = {0,0,0};
  ::gPntToTuple(p, center);

  const double major = ellp.MajorRadius();
  const double minor = ellp.MinorRadius();

  gp_Dir xaxis = ellp.XAxis().Direction(); // direction of major axis

  // rotation appears to be the clockwise(?) angle between major & +Y??
  const double rotation = xaxis.AngleWithRef(gp_Dir(0, 1, 0), gp_Dir(0, 0, 1));

  // 2*M_PI = 6.28319 is invalid(doesn't display in LibreCAD), but 2PI = 6.28318 is valid!
  // writeEllipse(center, major, minor, rotation, 0.0, 2 * M_PI, true );
  this->writeEllipse(center, major, minor, rotation, 0.0, 6.28318, true );
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportEllipseArc(BRepAdaptor_Curve& c)
{
  gp_Elips      ellp = c.Ellipse();
  const gp_Pnt& p    = ellp.Location();

  double center[3] = {0, 0, 0};
  ::gPntToTuple(p, center);

  double major = ellp.MajorRadius();
  double minor = ellp.MinorRadius();

  gp_Dir xaxis = ellp.XAxis().Direction();       //direction of major axis
  //rotation appears to be the clockwise angle between major & +Y??
  double rotation = xaxis.AngleWithRef(gp_Dir(0, 1, 0), gp_Dir(0, 0, 1));

  double f = c.FirstParameter();
  double l = c.LastParameter();
  gp_Pnt s = c.Value(f);
  gp_Pnt m = c.Value((l+f)/2.0);
  gp_Pnt e = c.Value(l);

  gp_Vec v1(m,s);
  gp_Vec v2(m,e);
  gp_Vec v3(0,0,1);
  double a = v3.DotCross(v1,v2);     // a = v3 dot (v1 cross v2)
                                      // relates to "handedness" of 3 vectors
                                      // a > 0 ==> v2 is CCW from v1 (righthanded)?
                                      // a < 0 ==> v2 is CW from v1 (lefthanded)?

  double startAngle = fmod(f,2.0*M_PI);  //revolutions
  double endAngle = fmod(l,2.0*M_PI);
  bool endIsCW = (a < 0) ? true: false;      //if !endIsCW swap(start,end)
  //not sure if this is a hack or not. seems to make valid arcs.
  if (!endIsCW) {
      startAngle = -startAngle;
      endAngle   = -endAngle;
  }

  this->writeEllipse(center, major, minor, rotation, startAngle, endAngle, endIsCW);
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportBSpline(BRepAdaptor_Curve& c)
{
  SplineDataOut sd;
  Handle(Geom_BSplineCurve) spline;
  double f, l;
  gp_Pnt s, ePt;

  double tol3D = 0.001;
  int maxDegree = 3, maxSegment = 200;
  Handle(BRepAdaptor_HCurve) hCurve = new BRepAdaptor_HCurve(c);

  // Approximate.
  Approx_Curve3d approx(hCurve, tol3D, GeomAbs_C0, maxSegment, maxDegree);
  //
  if ( approx.IsDone() && approx.HasResult() )
  {
    spline = approx.Curve();
  }
  else
  {
    if ( approx.HasResult() ) // result, but not within tolerance
    {
      spline = approx.Curve();
      m_progress.SendLogMessage(LogWarn(Normal) << "DxfWrite::exportBSpline - result not within tolerance.");
    }
    else
    {
      f = c.FirstParameter();
      l = c.LastParameter();
      s = c.Value(f);
      ePt = c.Value(l);
      m_progress.SendLogMessage( LogWarn(Normal) << "DxfWrite::exportBSpline - no result- from:(%1, %2) to:(%3, %4) poles: %5."
                                                 << s.X() << s.Y() << ePt.X() << ePt.Y() << spline->NbPoles() );
      TColgp_Array1OfPnt controlPoints(0,1);
      controlPoints.SetValue(0,s);
      controlPoints.SetValue(1,ePt);
      spline = GeomAPI_PointsToBSpline(controlPoints,1).Curve();
    }
  }

  //WF? norm of surface containing curve??
  sd.norm.x = 0.0;
  sd.norm.y = 0.0;
  sd.norm.z = 1.0;

  sd.flag =  spline->IsClosed();
  sd.flag += spline->IsPeriodic()*2;
  sd.flag += spline->IsRational()*4;
  sd.flag += 8;   //planar spline

  sd.degree = spline->Degree();
  sd.control_points = spline->NbPoles();
  sd.knots  = spline->NbKnots();
  gp_Pnt p;
  spline->D0(spline->FirstParameter(),p);
  sd.starttan = ::gPntTopoint3D(p);
  spline->D0(spline->LastParameter(),p);
  sd.endtan = ::gPntTopoint3D(p);

  //next bit is from DrawingExport.cpp (Dan Falk?).
  int m = 0;
  if  (spline->IsPeriodic() )
  {
    m = spline->NbPoles() + 2*spline->Degree() - spline->Multiplicity(1) + 2;
  }
  else
  {
    for ( int i=1; i<= spline->NbKnots(); i++ )
      m += spline->Multiplicity(i);
  }

  TColStd_Array1OfReal knotsequence(1, m);
  spline->KnotSequence(knotsequence);
  for (int i = knotsequence.Lower() ; i <= knotsequence.Upper(); i++)
  {
    sd.knot.push_back(knotsequence(i));
  }
  sd.knots = knotsequence.Length();

  TColgp_Array1OfPnt poles(1,spline->NbPoles());
  spline->Poles(poles);
  for (int i = poles.Lower(); i <= poles.Upper(); i++)
  {
    sd.control.push_back(gPntTopoint3D(poles(i)));
  }

  this->writeSpline(sd);
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportBezier(BRepAdaptor_Curve& c)
{
  (void) c;
  m_progress.SendLogMessage(LogErr(Normal) << "Bezier dxf export not yet supported.");
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportLine(BRepAdaptor_Curve& c)
{
  const double f = c.FirstParameter();
  const double l = c.LastParameter();

  gp_Pnt s        = c.Value(f);
  double start[3] = {0, 0, 0};
  //
  ::gPntToTuple(s, start);

  gp_Pnt e      = c.Value(l);
  double end[3] = {0, 0, 0};
  //
  ::gPntToTuple(e, end);

  this->writeLine(start, end);
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportLWPoly(BRepAdaptor_Curve& c)
{
  LWPolyDataOut pd;
  pd.Flag   = c.IsClosed();
  pd.Elev   = 0.0;
  pd.Thick  = 0.0;
  pd.Extr.x = 0.0;
  pd.Extr.y = 0.0;
  pd.Extr.z = 1.0;
  pd.nVert  = 0;

  GCPnts_UniformAbscissa discretizer;
  discretizer.Initialize(c, m_fSegLength);
  //
  if ( discretizer.IsDone() && discretizer.NbPoints() > 0 )
  {
    const int nbPoints = discretizer.NbPoints();
    //
    for ( int i = 1; i <= nbPoints; ++i )
    {
      gp_Pnt p = c.Value( discretizer.Parameter(i) );
      pd.Verts.push_back( ::gPntTopoint3D(p) );
    }
    pd.nVert = discretizer.NbPoints();

    this->writeLWPolyLine(pd);
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_WriteDXF::exportPolyline(BRepAdaptor_Curve& c)
{
  LWPolyDataOut pd;
  pd.Flag   = c.IsClosed();
  pd.Elev   = 0.0;
  pd.Thick  = 0.0;
  pd.Extr.x = 0.0;
  pd.Extr.y = 0.0;
  pd.Extr.z = 1.0;
  pd.nVert  = 0;

  GCPnts_UniformAbscissa discretizer;
  discretizer.Initialize(c, m_fSegLength);
  //
  if ( discretizer.IsDone() && discretizer.NbPoints() > 0 )
  {
    const int nbPoints = discretizer.NbPoints();
    //
    for ( int i = 1; i <= nbPoints; ++i )
    {
      gp_Pnt p = c.Value( discretizer.Parameter(i) );
      pd.Verts.push_back( ::gPntTopoint3D(p) );
    }
    pd.nVert = discretizer.NbPoints();

    this->writePolyline(pd);
  }
}
