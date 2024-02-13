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

#ifndef asiAlgo_ConvertCanonicalMod_h
#define asiAlgo_ConvertCanonicalMod_h

// asiAlgo includes
#include <asiAlgo_BRepNormalization.h>

// OpenCascade includes
#include <BRepTools_Modification.hxx>
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>

class TopoDS_Face;
class Geom_Surface;
class TopLoc_Location;
class TopoDS_Edge;
class Geom_Curve;
class TopoDS_Vertex;
class gp_Pnt;
class Geom2d_Curve;

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Modification class to perform canonical conversion at the level of
//! elementary B-rep shapes.
class asiAlgo_ConvertCanonicalMod : public asiAlgo_BRepNormalization
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_ConvertCanonicalMod, asiAlgo_BRepNormalization)

public:

  //! Default ctor.
  asiAlgo_EXPORT
    asiAlgo_ConvertCanonicalMod();

public:

  //! Sets the recognition tolerance.
  asiAlgo_EXPORT void
    SetTolerance(const double toler);

  //! Sets the surface recognition mode on/off.
  asiAlgo_EXPORT void
    SetSurfaceMode(const bool SurfMode);

  //! Sets the curve recognition mode on/off.
  asiAlgo_EXPORT void
    SetCurveMode(const bool CurvMode);

/* Modification API */
public:

  //! Returns true if the face `F` has been modified.
  //! In this case, `S` is the new geometric support of
  //! the face, `L` is the new location, `Tol` is the new
  //! tolerance.  Otherwise, returns false, and `S`, `L`, `Tol`
  //! are not significant.
  asiAlgo_EXPORT virtual bool
    NewSurface(const TopoDS_Face&    F,
               Handle(Geom_Surface)& S,
               TopLoc_Location&      L,
               double&               Tol,
               bool&                 RevWires,
               bool&                 RevFace) override;

  //! Returns true if the edge `E` has been modified.
  //! In this case, `C` is the new geometric support of
  //! the edge, `L` is the new location, `Tol` is the
  //! new tolerance. Otherwise, returns false, and `C`,
  //! `L`, `Tol` are not significant.
  asiAlgo_EXPORT virtual bool
    NewCurve(const TopoDS_Edge&  E,
             Handle(Geom_Curve)& C,
             TopLoc_Location&    L,
             double&             Tol) override;

  //! Returns true if the vertex `V` has been modified.
  //! In this case, `P` is the new geometric support of
  //! the vertex, `Tol` is the new tolerance. Otherwise,
  //! returns false, and `P`, `Tol` are not significant.
  asiAlgo_EXPORT virtual bool
    NewPoint(const TopoDS_Vertex& V,
             gp_Pnt&              P,
             double&              Tol) override;

  //! Returns true if the edge `E` has got a new
  //! curve on the surface on the face `F`. In this case, `C`
  //! is the new geometric support of the edge, `L` is the
  //! new location, `Tol` is the new tolerance.
  //!
  //! Otherwise, returns  false, and `C`, `L`, and `Tol`
  //! are not significant.
  //!
  //! The `NewE` is the new edge created from `E`. The `NewF`
  //! is the new face created from `F`.
  asiAlgo_EXPORT virtual bool
    NewCurve2d(const TopoDS_Edge&    E,
               const TopoDS_Face&    F,
               const TopoDS_Edge&    NewE,
               const TopoDS_Face&    NewF,
               Handle(Geom2d_Curve)& C,
               double&               Tol) override;

  //! Returns true if the Vertex `V` has got a new
  //! parameter on the edge `E`. In this case, `P` is
  //! the parameter, `Tol` is the new tolerance.
  //! Otherwise, returns false, and `P`, `Tol`
  //! arguments are not significant.
  asiAlgo_EXPORT virtual bool
    NewParameter(const TopoDS_Vertex& V,
                 const TopoDS_Edge&   E,
                 double&              P,
                 double&              Tol) override;

  //! Returns the order of continuity of the edge `NewE` between
  //! the faces `NewF1` and `NewF2`.
  //!
  //! The `NewE` is the new edge created from `E`. The `NewF1`
  //! (resp. `NewF2`) is the new face created from `F1`
  //! (resp. `F2`).
  asiAlgo_EXPORT virtual GeomAbs_Shape
    Continuity(const TopoDS_Edge& E,
               const TopoDS_Face& F1,
               const TopoDS_Face& F2,
               const TopoDS_Edge& NewE,
               const TopoDS_Face& NewF1,
               const TopoDS_Face& NewF2) override;

public:

  asiAlgo_EXPORT bool
    GetConverted(Handle(Geom_Surface)& S);

  asiAlgo_EXPORT bool
    GetConverted(const TopoDS_Edge&  E,
                 Handle(Geom_Curve)& C,
                 const double        F,
                 const double        L,
                 double&             NF,
                 double&             NL);

private:

  double                                     m_fToler;     //!< Conversion tolerance.
  TColStd_IndexedDataMapOfTransientTransient m_cache;      //!< Cached geometric entities.
  bool                                       m_bSurfMode;  //!< Indicates whether to convert surfaces.
  bool                                       m_bCurveMode; //!< Indicates whether to convert curves.

};

#endif // asiAlgo_ConvertCanonicalMod_h
