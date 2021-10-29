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
#include <asiAlgo_ConvertCanonicalMod.h>

// asiAlgo includes
#include <asiAlgo_ConvertCanonicalCurve.h>
#include <asiAlgo_ConvertCanonicalSurface.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeFix_Edge.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_TShape.hxx>
#include <TopExp.hxx>

//-----------------------------------------------------------------------------

struct t_curveWithParams : public Standard_Transient
{
  // RTTI
  DEFINE_STANDARD_RTTI_INLINE(t_curveWithParams, Standard_Transient)

  t_curveWithParams() : f(0.), l(0.) {} //!< Default ctor.

  double             f, l; //!< First and last parameters.
  Handle(Geom_Curve) C;    //!< Host curve.
};

//-----------------------------------------------------------------------------

asiAlgo_ConvertCanonicalMod::asiAlgo_ConvertCanonicalMod()
: asiAlgo_BRepNormalization()
{
  m_fToler     = 0.;
  m_bSurfMode  = true;
  m_bCurveMode = true;
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonicalMod::SetTolerance(const double tol)
{
  m_fToler = tol;
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonicalMod::SetSurfaceMode(const bool SurfMode)
{
  m_bSurfMode = SurfMode;
}

//-----------------------------------------------------------------------------

void asiAlgo_ConvertCanonicalMod::SetCurveMode(const bool CurvMode)
{
  m_bCurveMode = CurvMode;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::GetConverted(Handle(Geom_Surface)& S)
{
  if ( !m_bSurfMode )
    return false;

  const Handle(Standard_Transient)* ptr = m_cache.Seek(S);
  //
  if ( ptr )
  {
    S = Handle(Geom_Surface)::DownCast(*ptr);
    //
    if ( S.IsNull() )
      return false;

    return true;
  }

  // Untrim RTC.
  while ( S->IsKind( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
    S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();

  // Perform conversion.
  asiAlgo_ConvertCanonicalSurface scs(S);
  Handle(Geom_Surface) newsurf;
  newsurf = scs.Perform(m_fToler);

  // Cache the outcome.
  m_cache.Add(S, newsurf);

  // Set the outcome.
  S = newsurf;
  //
  if ( !S.IsNull() )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::GetConverted(const TopoDS_Edge&  E,
                                               Handle(Geom_Curve)& C,
                                               const double        F,
                                               const double        L,
                                               double&             NF,
                                               double&             NL)
{
  if ( !m_bCurveMode )
    return false;

  Handle(TopoDS_TShape) TE = E.TShape();

  const Handle(Standard_Transient)* ptr = m_cache.Seek(TE);

  // Get the cached curve.
  if ( ptr )
  {
    Handle(t_curveWithParams)
      res = Handle(t_curveWithParams)::DownCast(*ptr);
    //
    if ( res.IsNull() || res->C.IsNull() )
      return false;

    NF = res->f;
    NL = res->l;
    C  = res->C;
    return true;
  }

  // Perform conversion.
  asiAlgo_ConvertCanonicalCurve scc(C);
  Handle(Geom_Curve) resultCurve;
  scc.Perform(m_fToler, resultCurve, F, L, NF, NL);
  //
  Handle(t_curveWithParams) CAP = new t_curveWithParams;
  CAP->C = resultCurve;
  CAP->f = NF;
  CAP->l = NL;

  // Cache the outcome.
  m_cache.Add(TE, CAP);

  // Set the outcome.
  C = resultCurve;
  //
  if ( !C.IsNull() )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::NewSurface(const TopoDS_Face&    F,
                                             Handle(Geom_Surface)& S,
                                             TopLoc_Location&      L,
                                             double&               Tol,
                                             bool&                 RevWires,
                                             bool&                 RevFace)
{
  TopLoc_Location LS;
  Handle(Geom_Surface) SIni = BRep_Tool::Surface(F, LS);

  RevWires = false;
  RevFace  = false;
  L        = LS;
  S        = SIni;
  Tol      = BRep_Tool::Tolerance(F);

  if ( this->GetConverted(S) )
  {
    double U1, U2, V1, V2;
    SIni->Bounds(U1, U2, V1, V2);

    gp_Pnt P;
    gp_Vec D1U, D1V;
    SIni->D1( (U2 + U1)/2, (V2 + V1)/2, P, D1U, D1V );
    gp_Vec SIniNormal = D1U.Crossed(D1V);

    // Invert a point to compute the surface normal.
    Handle(ShapeAnalysis_Surface) SAS = new ShapeAnalysis_Surface(S);
    gp_Pnt2d P2d = SAS->ValueOfUV(P, 1e-05);
    S->D1(P2d.X(), P2d.Y(), P, D1U, D1V);
    gp_Vec SNormal = D1U.Crossed(D1V);

    // Check if the normal is flipped, so we have to reverse the face/wires.
    if ( SNormal.Angle(SIniNormal) >= M_PI/2 )
    {
      RevWires = true;
      RevFace = true;
    }
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::NewCurve(const TopoDS_Edge&  E,
                                           Handle(Geom_Curve)& C,
                                           TopLoc_Location&    L,
                                           double&             Tol)
{
  double f, l, nf, nl;
  TopLoc_Location LC;

  C   = BRep_Tool::Curve( E, LC, f, l );
  Tol = BRep_Tool::Tolerance(E);
  L   = LC;

  return this->GetConverted(E, C, f, l, nf, nl);
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::NewPoint(const TopoDS_Vertex& /*V*/,
                                           gp_Pnt& /*P*/,
                                           double& /*Tol*/)
{
  // Points are not converted.
  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::NewCurve2d(const TopoDS_Edge&    E,
                                             const TopoDS_Face&    F,
                                             const TopoDS_Edge&    /*NewE*/,
                                             const TopoDS_Face&    /*NewF*/,
                                             Handle(Geom2d_Curve)& C,
                                             double&               Tol)
{
  double f, l, nf, nl;
  TopLoc_Location LC, LS;

  Handle(Geom_Curve) C3d = BRep_Tool::Curve(E, LC, f, l);
  Handle(Geom_Surface) S = BRep_Tool::Surface(F, LS);

  C   = BRep_Tool::CurveOnSurface(E, F, f, l);
  Tol = BRep_Tool::Tolerance(E);

  // Take into account the orientation of a seam.

  bool isSurfConverted  = this->GetConverted(S);
  bool isCurveConverted = this->GetConverted(E, C3d, f, l, nf, nl);

  if ( isSurfConverted || isCurveConverted )
  {
    BRep_Builder B;
    TopoDS_Edge TempE;
    B.MakeEdge(TempE);
    B.Add( TempE, TopExp::FirstVertex(E) );
    B.Add( TempE, TopExp::LastVertex(E) );

    /* Prepare temporary edge. */
    if ( isCurveConverted )
    {
      B.UpdateEdge( TempE,
                    Handle(Geom_Curve)::DownCast( C3d->Transformed( LC.Transformation() ) ),
                    Precision::Confusion() );

      B.Range(TempE, nf, nl);
    }
    else
    {
      C3d = BRep_Tool::Curve(E, LC, f, l);

      if ( !C3d.IsNull() )
      {
        B.UpdateEdge( TempE,
                      Handle(Geom_Curve)::DownCast( C3d->Transformed( LC.Transformation() ) ),
                      Precision::Confusion() );
      }

      B.Range(TempE, f, l);
    }

    if ( !isSurfConverted )
      S = BRep_Tool::Surface(F, LS);

    Handle(ShapeFix_Edge) sfe   = new ShapeFix_Edge;
    Handle(Geom_Surface)  STemp = Handle(Geom_Surface)::DownCast( S->Transformed( LS.Transformation() ) );
    TopLoc_Location       LTemp;
 
    if ( isSurfConverted )
    {
      bool   isClosed = BRep_Tool::IsClosed(E, F);
      double workTol  = 2*m_fToler + Tol;

      sfe->FixAddPCurve( TempE, STemp, LTemp, isClosed, Max(Precision::Confusion(), workTol) );
      sfe->FixSameParameter(TempE);

      // Keep the orientation of original edge.
      TempE.Orientation( E.Orientation() );
      C = BRep_Tool::CurveOnSurface(TempE, STemp, LTemp, f, l);

      // Shift seam of a sphere.
      if ( isClosed && S->IsKind( STANDARD_TYPE(Geom_SphericalSurface) ) && !C.IsNull() )
      {
        double f2, l2;
        Handle(Geom2d_Curve)
          c22 = BRep_Tool::CurveOnSurface( TopoDS::Edge(TempE.Reversed() ), STemp, LTemp, f2, l2 );

        double dPreci = Precision::PConfusion()*Precision::PConfusion();

        if (    (C->Value(f).SquareDistance( c22->Value(f2) ) < dPreci)
             || (C->Value(l).SquareDistance( c22->Value(l2) ) < dPreci) )
        {
          gp_Vec2d shift(S->UPeriod(), 0.);
          C->Translate(shift);
        }
      }
    }
    else
    {
      if ( S->IsKind( STANDARD_TYPE(Geom_Plane) ) )
      {
        C.Nullify();
      }
      else
      {
        B.UpdateEdge    ( TempE, C, S, LS, Precision::Confusion() );
        B.Range         ( TempE, S, LS, f, l );
        B.SameParameter ( TempE, false );
        B.SameRange     ( TempE, false );
        //
        sfe->FixSameParameter(TempE);
        //
        C = BRep_Tool::CurveOnSurface(TempE, S, LS, f, l);
      }
    }
    Tol = BRep_Tool::Tolerance(TempE);
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ConvertCanonicalMod::NewParameter(const TopoDS_Vertex& V,
                                               const TopoDS_Edge&   E,
                                               double&              P,
                                               double&              Tol)
{
  const double       prec = Precision::PConfusion();
  double             f, l, nf, nl;
  TopLoc_Location    L;
  Handle(Geom_Curve) c3d = BRep_Tool::Curve(E, L, f, l);

  Tol = BRep_Tool::Tolerance(E);

  if ( this->GetConverted(E, c3d, f, l, nf, nl) )
  {
    const double oldParam = BRep_Tool::Parameter(V, E);

    if ( Abs(oldParam - f) < prec )
    {
      P = nf;
    }
    else if ( Abs(oldParam - l) < prec )
    {
      P = nl;
    }
    else
    {
      // Project point onto the curve to get the new parameter.
      gp_Pnt projPt;
      ShapeAnalysis_Curve SAC;
      //
      SAC.Project(c3d, BRep_Tool::Pnt(V), Tol, projPt, P);
    }

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

GeomAbs_Shape
  asiAlgo_ConvertCanonicalMod::Continuity(const TopoDS_Edge& E,
                                          const TopoDS_Face& F1,
                                          const TopoDS_Face& F2,
                                          const TopoDS_Edge& /*NewE*/,
                                          const TopoDS_Face& /*NewF1*/,
                                          const TopoDS_Face& /*NewF2*/)
{
  return BRep_Tool::Continuity(E, F1, F2);
}
