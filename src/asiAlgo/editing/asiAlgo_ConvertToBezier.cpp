//-----------------------------------------------------------------------------
// Created on: 16 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Elizaveta Krylova
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
#include "asiAlgo_ConvertToBezier.h"

// Asi include
#include <asiAlgo_Utils.h>
#include <asiAlgo_AppSurfUtils.h>

// OpenCascade include
#include <ShapeUpgrade_ConvertCurve3dToBezier.hxx>
#include <ShapeUpgrade_ConvertSurfaceToBezierBasis.hxx>
#include <ShapeExtend_CompositeSurface.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomConvert.hxx>
#include <GeomConvert_ApproxSurface.hxx>
#include <GeomConvert_ApproxCurve.hxx>

// tigl includes
#include <CTiglBSplineAlgorithms.h>

// Mobius includes
#include <mobius/cascade.h>
#include <mobius/geom_BSplineSurface.h>

using namespace mobius;

//-----------------------------------------------------------------------------

namespace
{
  //! Decreases the formal continuity of the passed curve by knot insertion
  //! down to C1 continuity.
  Handle(Geom_BSplineCurve)
    Defectize(const Handle(Geom_BSplineCurve)& C)
  {
    Handle(Geom_BSplineCurve)
      res = Handle(Geom_BSplineCurve)::DownCast( C->Copy() );

    const int deg = res->Degree();

    const TColStd_Array1OfInteger& M = res->Multiplicities();

    for ( int ii = M.Lower() + 1; ii < M.Upper(); ++ii )
    {
      res->IncreaseMultiplicity(ii, deg - 1);
    }

    return res;
  }

  //! Decreases the formal continuity of the passed surface by knot insertion
  //! down to C1 continuity.
  Handle(Geom_BSplineSurface)
    Defectize(const Handle(Geom_BSplineSurface)& S)
  {
    Handle(Geom_BSplineSurface)
      res = Handle(Geom_BSplineSurface)::DownCast( S->Copy() );

    const int uDeg = res->UDegree();
    const int vDeg = res->VDegree();

    const TColStd_Array1OfInteger& UM = res->UMultiplicities();
    const TColStd_Array1OfInteger& VM = res->VMultiplicities();

    for ( int ii = UM.Lower() + 1; ii < UM.Upper(); ++ii )
    {
      res->IncreaseUMultiplicity(ii, uDeg - 1);
    }

    for ( int ii = VM.Lower() + 1; ii < VM.Upper(); ++ii )
    {
      res->IncreaseVMultiplicity(ii, vDeg - 1);
    }

    return res;
  }

  //! Improves continuity of the passed surface by knot removal.
  static void SimplifySurface(Handle(Geom_BSplineSurface)& BS,
                              const double                 Tol,
                              const int                    MultMinU,
                              const int                    MultMinV)

  {
    int   ii;
    const TColStd_Array1OfReal& U = BS->UKnots();
    const TColStd_Array1OfReal& V = BS->VKnots();

    for ( ii = U.Length() - 1; ii > 1; ii-- )
    {
      BS->RemoveUKnot(ii, MultMinU, Tol);
    }

    for ( ii = V.Length() - 1; ii > 1; ii-- )
    {
      BS->RemoveVKnot(ii, MultMinV, Tol);
    }
  }
}

//-----------------------------------------------------------------------------

Handle(Geom_Curve)
  asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Curve)& curve)
{
  m_fMaxError = 0.;

  Handle(Geom_BSplineCurve)
    converted = GeomConvert::CurveToBSplineCurve( Handle(Geom_Curve)::DownCast( curve->Copy() ) );

  int deg = converted->Degree();

  m_progress.SendLogMessage(LogInfo(Normal) << "The initial curve degree %1."
                                            << deg);

  /* =====================================================
   *  Stage 1: make sure that the curve is at least cubic.
   * ===================================================== */

  bool isCubic = false;

  if ( deg == 3 )
  {
    isCubic = true;
  }
  else if ( deg < 3 )
  {
    // Elevate degree.
    converted->IncreaseDegree(3);
    isCubic = true;
  }
  else
  {
    isCubic = false;
  }

  // Update degree to be sure we have variable synced up after
  // potential degree elevation.
  deg = converted->Degree();

  /* =====================================================
   *  Stage 2: for cubic curves, it is enough to decrease
   *           multiplicities of knots.
   * ===================================================== */

  if ( isCubic )
  {
    converted = ::Defectize(converted);
  }

  // If here, then non-cubic and higher degree.
  else
  {
    /* ===========================================
     *  Stage 3.1: Split curve to Bezier segments.
     * =========================================== */

    // Split to Bezier.
    Handle(ShapeUpgrade_ConvertCurve3dToBezier)
      bzConverter = new ShapeUpgrade_ConvertCurve3dToBezier;
    //
    bzConverter->Init( curve, curve->FirstParameter(), curve->LastParameter() );
    bzConverter->Perform();
    //
    Handle(TColGeom_HArray1OfCurve) curves = bzConverter->GetCurves();

    /* ====================================================
     *  Stage 3.2: Reduce Bezier degree by reapproximation.
     * ==================================================== */

    // Reduce degrees by reapproximation.
    std::vector<Handle(Geom_BSplineCurve)> bCurves;
    //
    for ( int i = curves->Lower(); i <= curves->Upper(); ++i )
    {
      const Handle(Geom_Curve)& bzSeg = curves->Value(i);

      // Since in OpenCascade there is no function for degree reduction, we can only
      // reapproximate each segment to achieve the desired degree.
      GeomConvert_ApproxCurve approx(bzSeg, 1e-15, GeomAbs_C1, 1, 3);
      //
      if ( !approx.HasResult() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "Failed to reapproximate Bezier segment %1."
                                                  << i);
        return nullptr;
      }

      Handle(Geom_BSplineCurve) bsplSeg = approx.Curve();
      bCurves.push_back(bsplSeg);
    }

    /* ===============================================
     *  Stage 3.3: Concatenate the processed segments.
     * =============================================== */

    // Concatenate curves.
    GeomConvert_CompCurveToBSplineCurve concat;
    //
    for ( const auto& seg : bCurves )
    {
      concat.Add( seg, Precision::Confusion() );
    }
    //
    converted = concat.BSplineCurve();
  }

  // Measure deviation.
  asiAlgo_AppSurfUtils::MeasureDeviation(converted, GeomConvert::CurveToBSplineCurve(curve), m_fMaxError, m_plotter);

  return converted;
}

//-----------------------------------------------------------------------------

Handle(Geom_Surface)
  asiAlgo_ConvertToBezier::Perform(const Handle(Geom_Surface)& surface,
                                   const bool                  toApprox)
{
  m_fMaxError = 0.;

  Handle(Geom_BSplineSurface)
    converted = GeomConvert::SurfaceToBSplineSurface( Handle(Geom_Surface)::DownCast( surface->Copy() ) );

  int uDeg = converted->UDegree();
  int vDeg = converted->VDegree();

  m_progress.SendLogMessage(LogInfo(Normal) << "The initial surface has (%1,%2) (u,v) degrees."
                                            << uDeg << vDeg);

  /* =======================================================
   *  Stage 1: make sure that the surface is at least cubic.
   * ======================================================= */

  bool isCubic = false;

  if ( (uDeg == 3) && (vDeg == 3) )
  {
    isCubic = true;
  }
  else if ( (uDeg < 3) && (vDeg < 3) )
  {
    // Elevate both degrees.
    converted->IncreaseDegree(3, 3);
    isCubic = true;
  }
  else if ( uDeg < 3 )
  {
    // Elevate U degree.
    converted->IncreaseDegree(3, vDeg);
    isCubic = true;
  }
  else if ( vDeg < 3 )
  {
    // Elevate U degree
    converted->IncreaseDegree(uDeg, 3);
    isCubic = true;
  }
  else
  {
    isCubic = false;
  }

  /* =======================================================
   *  Stage 2: for cubic surfaces, it is enough to decrease
   *           multiplicities of knots.
   * ======================================================= */

  if ( isCubic )
  {
    converted = ::Defectize(converted);
  }

  // If here, then non-cubic and higher degree.
  else
  {
    /* ============================================
     *  Stage 3.1: Split surface to Bezier patches.
     * ============================================ */

    Handle(ShapeUpgrade_ConvertSurfaceToBezierBasis)
      bzConverter = new ShapeUpgrade_ConvertSurfaceToBezierBasis;
    //
    bzConverter->Init(converted);
    bzConverter->Perform();

    Handle(ShapeExtend_CompositeSurface)
      surfaces = bzConverter->ResSurfaces();
    //
    std::vector<std::vector<t_ptr<t_bsurf>>> mobBSurfaces;
    std::vector<std::vector<Handle(Geom_BSplineSurface)>> ocBSurfaces;

    Handle(TColStd_HArray1OfReal) uJoints = surfaces->UJointValues();
    Handle(TColStd_HArray1OfReal) vJoints = surfaces->VJointValues();

    mobBSurfaces.resize( uJoints->Length() );
    ocBSurfaces.resize( uJoints->Length() );

    /* ========================================================
     *  Stage 3.2: Process each patch, ideally, reduce degrees.
     * ======================================================== */

    // Process each patch individually.
    int k = 0;
    for ( int i = uJoints->Lower(); i <= uJoints->Upper(); ++i )
    {
      const double u = uJoints->Value(i);

      for ( int j = vJoints->Lower(); j <= vJoints->Upper(); ++j )
      {
        const double v = vJoints->Value(j);

        Handle(Geom_BezierSurface)
          bzPatch = Handle(Geom_BezierSurface)::DownCast(surfaces->Patch({u, v}));
        //
        if ( bzPatch.IsNull() )
        {
          m_progress.SendLogMessage(LogErr(Normal) << "Failed to extract Bezier patch (%1,%2)."
                                                   << i << j);
          return nullptr;
        }

        // Convert Bezier to B-spline basis and add more knots.
        Handle(Geom_BSplineSurface)
          bsplPatch = GeomConvert::SurfaceToBSplineSurface(bzPatch);

        if ( !toApprox )
        {
          /* Add knots to make the surface better prepared for forced degree reduction */

          double uKnot0, uKnot1, vKnot0, vKnot1;
          //
          bsplPatch->Bounds(uKnot0, uKnot1, vKnot0, vKnot1);

          // Add extra knots.
          bsplPatch->InsertUKnot( (3*uKnot0 +   uKnot1)*0.25, 1, Precision::PConfusion() );
          bsplPatch->InsertUKnot( (  uKnot0 + 3*uKnot1)*0.25, 1, Precision::PConfusion() );
          bsplPatch->InsertVKnot( (3*vKnot0 +   vKnot1)*0.25, 1, Precision::PConfusion() );
          bsplPatch->InsertVKnot( (  vKnot0 + 3*vKnot1)*0.25, 1, Precision::PConfusion() );
        }
        else
        {
          /* Approximate to reduce degree */

          if ( (bzPatch->NbUPoles() > 4) || (bzPatch->NbVPoles() > 4) ) // Higher degree Bezier.
          {
            GeomConvert_ApproxSurface approx(bzPatch, 1e-15, GeomAbs_C1, GeomAbs_C1, 3, 3, 0, 0);
            //
            Handle(Geom_Surface) result = approx.Surface();
            bsplPatch = GeomConvert::SurfaceToBSplineSurface(result);
          }
          else
          {
            bsplPatch->IncreaseDegree(3, 3);
          }
        }

        t_ptr<t_bsurf> mobBSpline = cascade::GetMobiusBSurface(bsplPatch);
        ocBSurfaces[k].push_back(bsplPatch);
      }
      k++;
    }

    /* ==============================================
     *  Stage 3.3: Concatenate the processed patches.
     * ============================================== */

    k = 0;

    // Reparameterize patches.
    for ( auto& surfs : ocBSurfaces )
    {
      bool   isFirst = true;
      double prevU   = -1;
      double prevV   = -1;

      for ( auto surf : surfs )
      {
        if ( !isFirst )
        {
          tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*surf.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
        }
        t_ptr<t_bsurf> mobBSpline = cascade::GetMobiusBSurface(surf);
        mobBSurfaces[k].push_back(mobBSpline);
        isFirst = false;
        prevU = surf->UKnot(surf->UKnots().Length());
        prevV = surf->VKnot(surf->VKnots().Length());
      }
      k++;
    }

    std::vector<t_ptr<t_bsurf>> spln;
    double prevU   = -1;
    double prevV   = -1;
    bool   isFirst = true;
    //
    if ( mobBSurfaces.empty() || mobBSurfaces[0].empty() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Failed to reparameterize patches.");
      return nullptr;
    }

    // Concatenate patches.
    for ( size_t i = 0; i < mobBSurfaces.size(); ++i )
    {
      t_ptr<t_bsurf> res = mobBSurfaces[i][0];
      for ( size_t j = 1; j < mobBSurfaces[i].size(); ++j )
      {
        if ( !res->ConcatenateCompatible(mobBSurfaces[i][j], true) )
          res->ConcatenateCompatible(mobBSurfaces[i][j], false);
      }
      Handle(Geom_BSplineSurface) cascSurf = cascade::GetOpenCascadeBSurface(res);
      if ( !isFirst )
      {
        tigl::CTiglBSplineAlgorithms::reparametrizeBSpline(*cascSurf.get(), prevU, prevU + 1., prevV, prevV + 1., 1e-15);
      }
      t_ptr<t_bsurf> mobBSpline = cascade::GetMobiusBSurface(cascSurf);
      isFirst = false;
      prevU = cascSurf->UKnot(cascSurf->UKnots().Length());
      prevV = cascSurf->VKnot(cascSurf->VKnots().Length());
      spln.push_back(mobBSpline);
    }

    if ( spln.empty() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Failed to concatenate patches.");
      return nullptr;
    }

    // Concatenate rows.
    t_ptr<t_bsurf> res = spln[0];
    for ( size_t j = 1; j < spln.size(); ++j )
    {
      if ( !res->ConcatenateCompatible(spln[j], true) )
        res->ConcatenateCompatible(spln[j], false);
    }
    //
    converted = cascade::GetOpenCascadeBSurface(res);

    // Fix continuity.
    if ( toApprox || (converted->Continuity() == GeomAbs_C0) )
    {
      ::SimplifySurface(converted, 1e10, uDeg - 1, vDeg - 1);
    }
  }

  // Measure deviation.
  asiAlgo_AppSurfUtils::MeasureDeviation(converted, GeomConvert::SurfaceToBSplineSurface(surface), m_fMaxError, m_plotter);

  return converted;
}
