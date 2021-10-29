//-----------------------------------------------------------------------------
// Created on: 29 October 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiAlgo_ConvertCanonicalSurface.h>

// asiAlgo includes
#include <asiAlgo_ConvertCanonicalCurve.h>
#include <asiAlgo_RecognizeCanonical.h>

// OpenCascade includes
#include <ElSLib.hxx>
#include <Geom_Line.hxx>
#include <GeomAdaptor_HSurface.hxx>

#define MagicTwist 1000

//-----------------------------------------------------------------------------

asiAlgo_ConvertCanonicalSurface::asiAlgo_ConvertCanonicalSurface(const Handle(Geom_Surface)& S)
: m_surf (S),
  m_fGap (0.)
{}

//-----------------------------------------------------------------------------

double asiAlgo_ConvertCanonicalSurface::GetFitError() const
{
  return m_fGap;
}

//-----------------------------------------------------------------------------

Handle(Geom_Surface)
  asiAlgo_ConvertCanonicalSurface::Perform(const double tol)
{
  double toler = tol;
  Handle(Geom_Surface) newSurf;

  // Skip non-spline and non-Bezier surfaces.
  if( !m_surf->IsKind( STANDARD_TYPE(Geom_BSplineSurface) ) &&
      !m_surf->IsKind( STANDARD_TYPE(Geom_BezierSurface) ) )
  {
    return newSurf;
  }

  // Try to fit a plane first.
  gp_Pln pln;
  //
  if ( asiAlgo_RecognizeCanonical::CheckIsPlanar(m_surf, toler, pln) )
  {
    return new Geom_Plane(pln);
  }

  // Check for infinite bounds.
  double U1, U2, V1, V2;
  m_surf->Bounds(U1, U2, V1, V2);
  //
  if ( Precision::IsInfinite(U1) && Precision::IsInfinite(U2) )
  {
    U1 = -1.;
    U2 =  1.;
  }
  if ( Precision::IsInfinite(V1) && Precision::IsInfinite(V2) )
  {
    V1 = -1.;
    V2 =  1.;
  }

  double diagonal = m_surf->Value(U1, V1).Distance( m_surf->Value( (U1 + U2), (V1 + V2)/2 ) );
  //
  if ( toler > diagonal/MagicTwist )
    toler = diagonal/MagicTwist;

  bool isCylinderCone = false;
  bool isTorusSphere  = false;

  // Convert middle isos to canonical representation.
  double VMid = 0.5*(V1 + V2);
  double UMid = 0.5*(U1 + U2);
  //
  Handle(Geom_Surface) TrSurf = new Geom_RectangularTrimmedSurface(m_surf, U1, U2, V1, V2);
  Handle(Geom_Curve)   UIso   = TrSurf->UIso(UMid);
  Handle(Geom_Curve)   VIso   = TrSurf->VIso(VMid);

  double cuf, cul, cvf, cvl;
  Handle(Geom_Curve)
    umidiso = asiAlgo_ConvertCanonicalCurve::ConvertCurve(UIso, toler, V1, V2, cuf, cul);
  //
  Handle(Geom_Curve)
    vmidiso = asiAlgo_ConvertCanonicalCurve::ConvertCurve(VIso, toler, U1, U2, cvf, cvl);
  //
  if ( umidiso.IsNull() || vmidiso.IsNull() )
  {
    return newSurf;
  }

  // Verify plane using the middle isolines.
  gp_Pnt FP, LP;
  gp_Pln plane;
  if ( umidiso->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    gp_Circ circ = Handle(Geom_Circle)::DownCast(umidiso)->Circ();

    plane = gp_Pln( circ.Location(),
                    circ.Axis().Direction() );

    FP = vmidiso->Value(cuf);
    LP = vmidiso->Value(cul);

    if ( (plane.Distance(FP) <= toler) && (plane.Distance(LP) <= toler) )
    {
      if ( !newSurf.IsNull() )
        return newSurf;
    }
  }
  else if ( vmidiso->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    gp_Circ circ = Handle(Geom_Circle)::DownCast(vmidiso)->Circ();

    plane = gp_Pln( circ.Location(),
                    circ.Axis().Direction() );

    FP = umidiso->Value(cvf);
    LP = umidiso->Value(cvl);

    if ( (plane.Distance(FP) <= toler) && (plane.Distance(LP) <= toler) )
    {
      if ( !newSurf.IsNull() )
        return newSurf;
    }
  }
  //
  bool VCase = false;

  if ( umidiso->IsKind( STANDARD_TYPE(Geom_Line) ) && vmidiso->IsKind( STANDARD_TYPE(Geom_Line) ) )
  {
    return newSurf;
  }

  if ( umidiso->IsKind( STANDARD_TYPE(Geom_Circle) ) && vmidiso->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    isTorusSphere = true;
  }
  else if ( umidiso->IsKind( STANDARD_TYPE(Geom_Line) ) && vmidiso->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    isCylinderCone = true; VCase = true;
  }
  else if ( umidiso->IsKind(STANDARD_TYPE(Geom_Circle)) && vmidiso->IsKind( STANDARD_TYPE(Geom_Line) ) )
  {
    isCylinderCone = true;
  }

  double cl = 0.0;

  /* torus or sphere */
  if ( isTorusSphere )
  {
    Handle(Geom_Circle) Ucircle = Handle(Geom_Circle)::DownCast(umidiso);
    Handle(Geom_Circle) Vcircle = Handle(Geom_Circle)::DownCast(vmidiso);

    // torus
    // try when V isolines is with same radius
    Handle(Geom_Surface) obj;
    asiAlgo_RecognizeCanonical::CheckIsTorusSphere(m_surf, Vcircle, Ucircle, V1, V2, U1, U2, toler, true, obj);
    //
    if ( obj.IsNull() ) // try when U isolines is with same radius
      asiAlgo_RecognizeCanonical::CheckIsTorusSphere(m_surf, Ucircle, Vcircle, U1, U2, V1, V2, toler, false, obj);

    if ( !obj.IsNull() )
      newSurf = obj;
  }

  /* cone or cylinder */
  else if ( isCylinderCone )
  {
    double              param1, param2, cf1, cf2;
    Handle(Geom_Curve)  firstiso, lastiso;
    Handle(Geom_Circle) firstisocirc, lastisocirc, midisocirc;
    gp_Dir              isoline;

    if ( VCase )
    {
      param1     = U1;
      param2     = U2;
      firstiso   = TrSurf->VIso(V1);
      lastiso    = TrSurf->VIso(V2);
      midisocirc = Handle(Geom_Circle)::DownCast(vmidiso);
      isoline    = Handle(Geom_Line)::DownCast(umidiso)->Lin().Direction();
    }
    else
    {
      param1     = V1;
      param2     = V2;
      firstiso   = TrSurf->UIso(U1);
      lastiso    = TrSurf->UIso(U2);
      midisocirc = Handle(Geom_Circle)::DownCast(umidiso);
      isoline    = Handle(Geom_Line)::DownCast(vmidiso)->Lin().Direction();
    }

    firstisocirc = Handle(Geom_Circle)::DownCast( asiAlgo_ConvertCanonicalCurve::ConvertCurve(firstiso, toler, param1, param2, cf1, cl) );
    lastisocirc  = Handle(Geom_Circle)::DownCast( asiAlgo_ConvertCanonicalCurve::ConvertCurve(lastiso, toler, param1, param2, cf2, cl) );

    if ( !firstisocirc.IsNull() || !lastisocirc.IsNull() )
    {
      double R1, R2, R3;
      gp_Pnt P1, P2, P3;

      if ( !firstisocirc.IsNull() )
      {
        R1 = firstisocirc->Circ().Radius();
        P1 = firstisocirc->Circ().Location();
      }
      else
      {
        R1 = 0;
        P1 = firstiso->Value( (firstiso->LastParameter() - firstiso->FirstParameter() )/2 );
      }

      R2 = midisocirc->Circ().Radius();
      P2 = midisocirc->Circ().Location();

      if ( !lastisocirc.IsNull() )
      {
        R3 = lastisocirc->Circ().Radius();
        P3 = lastisocirc->Circ().Location();
      }
      else
      {
        R3 = 0;
        P3 = lastiso->Value( (lastiso->LastParameter() - lastiso->FirstParameter() )/2 );
      }

      // cylinder
      if ( ( Abs(R2-R1) < toler ) && ( Abs(R3-R1) < toler ) && ( Abs(R3-R2) < toler ) )
      {
        gp_Ax3 Axes( P1, gp_Vec(P1, P3) );

        newSurf = new Geom_CylindricalSurface(Axes, R1);
      }

      // cone
      else if ( ((Abs(R1) > Abs(R2)) && (Abs(R2) > Abs(R3))) ||
                ((Abs(R3) > Abs(R2)) && (Abs(R2) > Abs(R1))) )
      {
        double radius;
        gp_Ax3 Axes;
        double semiangle = gp_Vec(isoline).Angle( gp_Vec(P3, P1) );

        if ( semiangle > M_PI/2 )
          semiangle = M_PI - semiangle;

        if ( R1 > R3 )
        {
          radius = R3;
          Axes = gp_Ax3( P3, gp_Vec(P3, P1) );
        }
        else
        {
          radius = R1;
          Axes = gp_Ax3( P1, gp_Vec(P1, P3) );
        }

        newSurf = new Geom_ConicalSurface(Axes, semiangle, radius);
      }
    }
    else return newSurf;
  }

  if ( newSurf.IsNull() )
    return newSurf;

  /* ==============
   *  Verification.
   * ============== */

  m_fGap = 0.;

  Handle(GeomAdaptor_HSurface)
    SurfAdapt = new GeomAdaptor_HSurface(newSurf);

  const int          NP = 21;
  double             S = 0., T = 0.;
  gp_Pnt             P3d, P3d2;
  bool               onSurface = true;
  double             dis;
  double             DU, DV;
  int                j, i;
  Handle(Geom_Curve) iso;

  DU = (U2 - U1) / (NP - 1);
  DV = (V2 - V1) / (NP - 1);

  for ( j = 1; (j <= NP) && onSurface; ++j )
  {
    double V = V1 + DV*(j - 1);

    iso = TrSurf->VIso(V);

    for ( i = 1; i <= NP; ++i )
    {
      double U = U1 + DU*(i - 1);
      iso->D0(U, P3d);
      switch ( SurfAdapt->GetType() )
      {
        case GeomAbs_Plane:
        {
          gp_Pln Plane = SurfAdapt->Plane();
          ElSLib::Parameters(Plane, P3d, S, T);
          break;
        }
        case GeomAbs_Cylinder:
        {
          gp_Cylinder Cylinder = SurfAdapt->Cylinder();
          ElSLib::Parameters(Cylinder, P3d, S, T);
          break;
        }
        case GeomAbs_Cone:
        {
          gp_Cone Cone = SurfAdapt->Cone();
          ElSLib::Parameters(Cone, P3d, S, T);
          break;
        }
        case GeomAbs_Sphere:
        {
          gp_Sphere Sphere = SurfAdapt->Sphere();
          ElSLib::Parameters(Sphere, P3d, S, T);
          break;
        }
        case GeomAbs_Torus:
        {
          gp_Torus Torus = SurfAdapt->Torus();
          ElSLib::Parameters(Torus, P3d, S, T);
          break;
        }
        default: break;
      }

      newSurf->D0(S, T, P3d2);
      dis = P3d.Distance(P3d2);

      if ( dis > m_fGap )
        m_fGap = dis;

      if ( dis > toler )
      {
        onSurface = false;
        newSurf.Nullify();
        break;
      }
    }
  }
  return newSurf;
}
