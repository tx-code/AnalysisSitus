//-----------------------------------------------------------------------------
// Created on: 19 September 2021
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

#ifndef ActAPI_IPlotter_HeaderFile
#define ActAPI_IPlotter_HeaderFile

// Active Data (API) includes
#include <ActAPI_Common.h>

// OCCT includes
#include <Quantity_Color.hxx>

// STL includes
#include <vector>

// Forward declarations
class Geom2d_Curve;
class Geom_Curve;
class Geom_Plane;
class Geom_Surface;
class Poly_Triangulation;
class ActData_Mesh;

//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Wrapper for Quantity_Color which allows undefined colors (we need such
//! for "default color" option in imperative plotter).
class ActAPI_Color : public Quantity_Color
{
public:

  //! Ctor.
  ActAPI_Color() : Quantity_Color (),
                   m_bIsDefined   (Standard_False) {}

  //! Ctor.
  ActAPI_Color(Quantity_NameOfColor name) : Quantity_Color (name),
                                            m_bIsDefined   (Standard_True) {}

  //! Ctor.
  ActAPI_Color(const Standard_Real        R1,
               const Standard_Real        R2,
               const Standard_Real        R3,
               const Quantity_TypeOfColor Type) : Quantity_Color (R1, R2, R3, Type),
                                                  m_bIsDefined   (Standard_True) {}

public:

  //! \return true if the color is defined.
  Standard_Boolean IsDefined() const
  {
    return m_bIsDefined;
  }

protected:

  Standard_Boolean m_bIsDefined; //!< Whether the color is defined or not.

};

//-----------------------------------------------------------------------------

#define Color_Default   ActAPI_Color(Quantity_NOC_SNOW)
#define Color_Red       ActAPI_Color(Quantity_NOC_RED)
#define Color_Orange    ActAPI_Color(Quantity_NOC_ORANGE)
#define Color_Purple    ActAPI_Color(Quantity_NOC_PURPLE)
#define Color_Pink      ActAPI_Color(Quantity_NOC_PINK)
#define Color_Green     ActAPI_Color(Quantity_NOC_GREEN)
#define Color_Khaki     ActAPI_Color(Quantity_NOC_KHAKI)
#define Color_Blue      ActAPI_Color(Quantity_NOC_BLUE1)
#define Color_Yellow    ActAPI_Color(Quantity_NOC_YELLOW)
#define Color_White     ActAPI_Color(Quantity_NOC_WHITE)
#define Color_Snow      ActAPI_Color(Quantity_NOC_SNOW)
#define Color_Black     ActAPI_Color(Quantity_NOC_BLACK)
#define Color_Violet    ActAPI_Color(Quantity_NOC_VIOLET)
#define Color_Magenta   ActAPI_Color(Quantity_NOC_MAGENTA1)
#define Color_Maroon    ActAPI_Color(Quantity_NOC_MAROON)
#define Color_LightGray ActAPI_Color(Quantity_NOC_LIGHTGRAY)

//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Interface for Imperative Viewer. A particular algorithm may benefit
//! from immediate plotting of its geometric variables in a unified way
//! thanks to this abstract class.
class ActAPI_IPlotter : public Standard_Transient
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(ActAPI_IPlotter, Standard_Transient)

public:

  ActData_EXPORT static Quantity_NameOfColor
    Color_Sparse(const int index);

public:

  ActData_EXPORT virtual
    ~ActAPI_IPlotter();

// COMMON:
public:

  virtual void
    ERASE_ALL() {}

  virtual void
    ERASE(const TCollection_AsciiString&) {}

// GEOMETRY:
public:

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_POINT(const gp_XY&,
               const ActAPI_Color&,
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_POINT(const gp_Pnt2d&,
               const ActAPI_Color&,
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_POINT(const gp_XYZ&,
               const ActAPI_Color&,
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_POINT(const gp_Pnt&,
               const ActAPI_Color&,
               const TCollection_AsciiString&) {}

  virtual void
    REDRAW_POINT(const TCollection_AsciiString&,
                 const gp_XY&,
                 const ActAPI_Color&) {}

  virtual void
    REDRAW_POINT(const TCollection_AsciiString&,
                 const gp_Pnt2d&,
                 const ActAPI_Color&) {}

  virtual void
    REDRAW_POINT(const TCollection_AsciiString&,
                 const gp_XYZ&,
                 const ActAPI_Color&) {}

  virtual void
    REDRAW_POINT(const TCollection_AsciiString&,
                 const gp_Pnt&,
                 const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_POINTS(const Handle(HRealArray)&,
                const ActAPI_Color&,
                const TCollection_AsciiString&) {}

  virtual void
    DRAW_POINTS(const std::vector<gp_XYZ>&,
                const ActAPI_Color&,
                const TCollection_AsciiString&) {}

  virtual void
    REDRAW_POINTS(const TCollection_AsciiString&,
                  const Handle(HRealArray)&,
                  const ActAPI_Color&) {}

  virtual void
    REDRAW_POINTS(const TCollection_AsciiString&,
                  const std::vector<gp_XYZ>&,
                  const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_VECTOR_AT(const gp_Pnt&,
                   const gp_Vec&,
                   const ActAPI_Color&,
                   const TCollection_AsciiString&) {}

  virtual void
    REDRAW_VECTOR_AT(const TCollection_AsciiString&,
                     const gp_Pnt&,
                     const gp_Vec&,
                     const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_VECTORS_AT(const gp_Pnt&,
                    const std::vector<gp_Vec>&,
                    const ActAPI_Color&,
                    const TCollection_AsciiString&) {}

  virtual void
    REDRAW_VECTORS_AT(const TCollection_AsciiString&,
                      const gp_Pnt&,
                      const std::vector<gp_Vec>&,
                      const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_VECTORS(const Handle(HRealArray)&,
                 const Handle(HRealArray)&,
                 const ActAPI_Color&,
                 const TCollection_AsciiString&) {}

  virtual void
    REDRAW_VECTORS(const TCollection_AsciiString&,
                   const Handle(HRealArray)&,
                   const Handle(HRealArray)&,
                   const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_CURVE(const Handle(Geom_Curve)&,
               const ActAPI_Color&,
               const bool,
               const TCollection_AsciiString&) {}

  virtual void
    REDRAW_CURVE(const TCollection_AsciiString&,
                 const Handle(Geom_Curve)&,
                 const ActAPI_Color&,
                 const bool) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_CURVE2D(const Handle(Geom2d_Curve)&,
                 const ActAPI_Color&,
                 const TCollection_AsciiString&) {}

  virtual void
    REDRAW_CURVE2D(const TCollection_AsciiString&,
                   const Handle(Geom2d_Curve)&,
                   const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_SURFACE(const Handle(Geom_Surface)&,
                 const ActAPI_Color&,
                 const TCollection_AsciiString&) {}

  virtual void
    DRAW_SURFACE(const Handle(Geom_Surface)&,
                 const ActAPI_Color&,
                 const double, // opacity
                 const TCollection_AsciiString&) {}

  virtual void
    DRAW_SURFACE(const Handle(Geom_Surface)&,
                 const double, // U min
                 const double, // U max
                 const double, // V min
                 const double, // V max
                 const ActAPI_Color&,
                 const TCollection_AsciiString&) {}

  virtual void
    DRAW_SURFACE(const Handle(Geom_Surface)&,
                 const double, // U min
                 const double, // U max
                 const double, // V min
                 const double, // V max
                 const ActAPI_Color&,
                 const double, // opacity
                 const TCollection_AsciiString&) {}

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString&,
                   const Handle(Geom_Surface)&,
                   const ActAPI_Color&) {}

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString&,
                   const Handle(Geom_Surface)&,
                   const ActAPI_Color&,
                   const double) {} // opacity

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString&,
                   const Handle(Geom_Surface)&,
                   const double, // U min
                   const double, // U max
                   const double, // V min
                   const double, // V max
                   const ActAPI_Color&) {}

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString&,
                   const Handle(Geom_Surface)&,
                   const double, // U min
                   const double, // U max
                   const double, // V min
                   const double, // V max
                   const ActAPI_Color&,
                   const double) {} // opacity

// TOPOLOGY:
public:

  virtual void
    DRAW_SHAPE(const TopoDS_Shape&,
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_SHAPE(const TopoDS_Shape&,
               const ActAPI_Color&,
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_SHAPE(const TopoDS_Shape&,
               const double, // opacity
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_SHAPE(const TopoDS_Shape&,
               const ActAPI_Color&,
               const double, // opacity
               const TCollection_AsciiString&) {}

  virtual void
    DRAW_SHAPE(const TopoDS_Shape&,
               const ActAPI_Color&,
               const double, // opacity
               const bool, // is wireframe
               const TCollection_AsciiString&) {}

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString&,
                 const TopoDS_Shape&) {}

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString&,
                 const TopoDS_Shape&,
                 const ActAPI_Color&) {}

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString&,
                 const TopoDS_Shape&,
                 const double) {} // opacity

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString&,
                 const TopoDS_Shape&,
                 const ActAPI_Color&,
                 const double) {} // opacity

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString&,
                 const TopoDS_Shape&,
                 const ActAPI_Color&,
                 const double, // opacity
                 const bool) {} // is wireframe

// FACETED GEOMETRY:
public:

  virtual void
    DRAW_LINK(const gp_Pnt&,
              const gp_Pnt&,
              const ActAPI_Color&,
              const TCollection_AsciiString&) {}

  virtual void
    DRAW_LINK(const gp_XYZ&,
              const gp_XYZ&,
              const ActAPI_Color&,
              const TCollection_AsciiString&) {}

  virtual void
    DRAW_LINK(const gp_Pnt2d&,
              const gp_Pnt2d&,
              const ActAPI_Color&,
              const TCollection_AsciiString&) {}

  virtual void
    DRAW_LINK(const gp_XY&,
              const gp_XY&,
              const ActAPI_Color&,
              const TCollection_AsciiString&) {}

  virtual void
    REDRAW_LINK(const TCollection_AsciiString&,
                const gp_Pnt&,
                const gp_Pnt&,
                const ActAPI_Color&) {}

  virtual void
    REDRAW_LINK(const TCollection_AsciiString&,
                const gp_XYZ&,
                const gp_XYZ&,
                const ActAPI_Color&) {}

  virtual void
    REDRAW_LINK(const TCollection_AsciiString&,
                const gp_Pnt2d&,
                const gp_Pnt2d&,
                const ActAPI_Color&) {}

  virtual void
    REDRAW_LINK(const TCollection_AsciiString&,
                const gp_XY&,
                const gp_XY&,
                const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_POLYLINE(const std::vector<gp_XY>&,
                  const ActAPI_Color&,
                  const TCollection_AsciiString&) {}

  virtual void
    DRAW_POLYLINE(const std::vector<gp_XYZ>&,
                  const ActAPI_Color&,
                  const TCollection_AsciiString&) {}

  virtual void
    REDRAW_POLYLINE(const TCollection_AsciiString&,
                    const std::vector<gp_XY>&,
                    const ActAPI_Color&) {}

  virtual void
    REDRAW_POLYLINE(const TCollection_AsciiString&,
                    const std::vector<gp_XYZ>&,
                    const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_TRIANGLE(const gp_Pnt&,
                  const gp_Pnt&,
                  const gp_Pnt&,
                  const ActAPI_Color&,
                  const TCollection_AsciiString&) {}

  virtual void
    DRAW_TRIANGLE(const gp_XYZ&,
                  const gp_XYZ&,
                  const gp_XYZ&,
                  const ActAPI_Color&,
                  const TCollection_AsciiString&) {}

  virtual void
    DRAW_TRIANGLE(const gp_Pnt2d&,
                  const gp_Pnt2d&,
                  const gp_Pnt2d&,
                  const ActAPI_Color&,
                  const TCollection_AsciiString&) {}

  virtual void
    DRAW_TRIANGLE(const gp_XY&,
                  const gp_XY&,
                  const gp_XY&,
                  const ActAPI_Color&,
                  const TCollection_AsciiString&) {}

  virtual void
    REDRAW_TRIANGLE(const TCollection_AsciiString&,
                    const gp_Pnt&,
                    const gp_Pnt&,
                    const gp_Pnt&,
                    const ActAPI_Color&) {}

  virtual void
    REDRAW_TRIANGLE(const TCollection_AsciiString&,
                    const gp_XYZ&,
                    const gp_XYZ&,
                    const gp_XYZ&,
                    const ActAPI_Color&) {}

  virtual void
    REDRAW_TRIANGLE(const TCollection_AsciiString&,
                    const gp_Pnt2d&,
                    const gp_Pnt2d&,
                    const gp_Pnt2d&,
                    const ActAPI_Color&) {}

  virtual void
    REDRAW_TRIANGLE(const TCollection_AsciiString&,
                    const gp_XY&,
                    const gp_XY&,
                    const gp_XY&,
                    const ActAPI_Color&) {}

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_TRIANGULATION(const Handle(Poly_Triangulation)&,
                       const ActAPI_Color&,
                       const double, // opacity
                       const TCollection_AsciiString&) {}

  virtual void
    REDRAW_TRIANGULATION(const TCollection_AsciiString&,
                         const Handle(Poly_Triangulation)&,
                         const ActAPI_Color&,
                         const double) {} // opacity

  //-------------------------------------------------------------------------//

  virtual void
    DRAW_MESH(const Handle(ActData_Mesh)&,
              const ActAPI_Color&,
              const double, // opacity
              const double, // edge width
              const TCollection_AsciiString&) {}

  virtual void
    REDRAW_MESH(const TCollection_AsciiString&,
                const Handle(ActData_Mesh)&,
                const ActAPI_Color&,
                const double, // opacity
                const double) {} // edge width
// TEXT
public:

  virtual void
    DRAW_TEXT(const TCollection_AsciiString&, // text
              const TCollection_AsciiString&) {} // name

  virtual void
    REDRAW_TEXT(const TCollection_AsciiString&, // name
                const TCollection_AsciiString&) {} // text

};

//-----------------------------------------------------------------------------

//! \ingroup AD_API
//!
//! Safe entry to IV.
class ActAPI_PlotterEntry
{
public:

  //! Default constructor.
  ActAPI_PlotterEntry() {}

  //! Dummy conversion constructor.
  ActAPI_PlotterEntry(std::nullptr_t) {}

  //! Constructor.
  //! \param iv [in] IV to wrap.
  ActAPI_PlotterEntry(const Handle(ActAPI_IPlotter)& iv) : m_iv(iv) {}

public:

  //! Assignment operator.
  //! \param iv [in] IV to wrap.
  void operator=(const Handle(ActAPI_IPlotter)& iv)
  {
    m_iv = iv;
  }

public:

  //! \return Journal instance.
  const Handle(ActAPI_IPlotter)& Access() const { return m_iv; }

public:

//---------------------------------------------------------------------------//

  void
    ERASE_ALL()
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->ERASE_ALL();
  }

//---------------------------------------------------------------------------//

  void
    ERASE(const TCollection_AsciiString& name)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->ERASE(name);
  }

public:

//---------------------------------------------------------------------------//

  void
    DRAW_POINT(const gp_XY&                   coord,
               const ActAPI_Color&            color,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POINT(coord, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POINT(const gp_Pnt2d&                point,
               const ActAPI_Color&            color,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POINT(point, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POINT(const gp_XYZ&                  coord,
               const ActAPI_Color&            color,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POINT(coord, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POINT(const gp_Pnt&                  point,
               const ActAPI_Color&            color,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POINT(point, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POINT(const TCollection_AsciiString& name,
                 const gp_XY&                   point,
                 const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POINT(name, point, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POINT(const TCollection_AsciiString& name,
                 const gp_Pnt2d&                point,
                 const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POINT(name, point, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POINT(const TCollection_AsciiString& name,
                 const gp_XYZ&                  point,
                 const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POINT(name, point, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POINT(const TCollection_AsciiString& name,
                 const gp_Pnt&                  point,
                 const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POINT(name, point, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POINTS(const Handle(HRealArray)&      coords,
                const ActAPI_Color&            color,
                const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POINTS(coords, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POINTS(const std::vector<gp_XYZ>&     pts,
                const ActAPI_Color&            color,
                const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POINTS(pts, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POINTS(const TCollection_AsciiString& name,
                  const Handle(HRealArray)&      coords,
                  const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POINTS(name, coords, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POINTS(const TCollection_AsciiString& name,
                  const std::vector<gp_XYZ>&     pts,
                  const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POINTS(name, pts, color);
  }

//-------------------------------------------------------------------------//

  virtual void
    DRAW_VECTORS(const Handle(HRealArray)&      points,
                 const Handle(HRealArray)&      vectors,
                 const ActAPI_Color&            color,
                 const TCollection_AsciiString& name)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_VECTORS(points, vectors, color, name);
  }

//-------------------------------------------------------------------------//

  virtual void
    REDRAW_VECTORS(const TCollection_AsciiString& name,
                   const Handle(HRealArray)&      points,
                   const Handle(HRealArray)&      vectors,
                   const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_VECTORS(name, points, vectors, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_VECTOR_AT(const gp_Pnt&                  P,
                   const gp_Vec&                  V,
                   const ActAPI_Color&            color,
                   const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_VECTOR_AT(P, V, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_VECTOR_AT(const TCollection_AsciiString& name,
                     const gp_Pnt&                  P,
                     const gp_Vec&                  V,
                     const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_VECTOR_AT(name, P, V, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_VECTORS_AT(const gp_Pnt&                  origin,
                    const std::vector<gp_Vec>&     vectors,
                    const ActAPI_Color&            color,
                    const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_VECTORS_AT(origin, vectors, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_VECTORS_AT(const TCollection_AsciiString& name,
                      const gp_Pnt&                  origin,
                      const std::vector<gp_Vec>&     vectors,
                      const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_VECTORS_AT(name, origin, vectors, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_CURVE(const Handle(Geom_Curve)&      curve,
               const ActAPI_Color&            color,
               const bool                     showOri,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_CURVE(curve, color, showOri, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_CURVE(const TCollection_AsciiString& name,
                 const Handle(Geom_Curve)&      curve,
                 const ActAPI_Color&            color,
                 const bool                     showOri)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_CURVE(name, curve, color, showOri);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_CURVE2D(const Handle(Geom2d_Curve)&    curve,
                 const ActAPI_Color&            color,
                 const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_CURVE2D(curve, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_CURVE2D(const TCollection_AsciiString& name,
                   const Handle(Geom2d_Curve)&    curve,
                   const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_CURVE2D(name, curve, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SURFACE(const Handle(Geom_Surface)&    surface,
                 const ActAPI_Color&            color,
                 const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SURFACE(surface, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SURFACE(const Handle(Geom_Surface)&    surface,
                 const ActAPI_Color&            color,
                 const double                   opacity,
                 const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SURFACE(surface, color, opacity, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SURFACE(const Handle(Geom_Surface)&    surface,
                 const double                   uMin,
                 const double                   uMax,
                 const double                   vMin,
                 const double                   vMax,
                 const ActAPI_Color&            color,
                 const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SURFACE(surface, uMin, uMax, vMin, vMax, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SURFACE(const Handle(Geom_Surface)&    surface,
                 const double                   uMin,
                 const double                   uMax,
                 const double                   vMin,
                 const double                   vMax,
                 const ActAPI_Color&            color,
                 const double                   opacity,
                 const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SURFACE(surface, uMin, uMax, vMin, vMax, color, opacity, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_SURFACE(const TCollection_AsciiString& name,
                   const Handle(Geom_Surface)&    surface,
                   const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SURFACE(name, surface, color);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString& name,
                   const Handle(Geom_Surface)&    surface,
                   const ActAPI_Color&            color,
                   const double                   opacity)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SURFACE(name, surface, color, opacity);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString& name,
                   const Handle(Geom_Surface)&    surface,
                   const double                   uMin,
                   const double                   uMax,
                   const double                   vMin,
                   const double                   vMax,
                   const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SURFACE(name, surface, uMin, uMax, vMin, vMax, color);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SURFACE(const TCollection_AsciiString& name,
                   const Handle(Geom_Surface)&    surface,
                   const double                   uMin,
                   const double                   uMax,
                   const double                   vMin,
                   const double                   vMax,
                   const ActAPI_Color&            color,
                   const double                   opacity)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SURFACE(name, surface, uMin, uMax, vMin, vMax, color, opacity);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SHAPE(const TopoDS_Shape&            shape,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SHAPE(shape, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SHAPE(const TopoDS_Shape&            shape,
               const ActAPI_Color&            color,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SHAPE(shape, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SHAPE(const TopoDS_Shape&            shape,
               const double                   opacity,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SHAPE(shape, opacity, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SHAPE(const TopoDS_Shape&            shape,
               const ActAPI_Color&            color,
               const double                   opacity,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SHAPE(shape, color, opacity, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_SHAPE(const TopoDS_Shape&            shape,
               const ActAPI_Color&            color,
               const double                   opacity,
               const bool                     isWireframe,
               const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_SHAPE(shape, color, opacity, isWireframe, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_SHAPE(const TCollection_AsciiString& name,
                 const TopoDS_Shape&            shape)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SHAPE(name, shape);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString& name,
                 const TopoDS_Shape&            shape,
                 const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SHAPE(name, shape, color);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString& name,
                 const TopoDS_Shape&            shape,
                 const double                   opacity)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SHAPE(name, shape, opacity);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString& name,
                 const TopoDS_Shape&            shape,
                 const ActAPI_Color&            color,
                 const double                   opacity)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SHAPE(name, shape, color, opacity);
  }

//---------------------------------------------------------------------------//

  virtual void
    REDRAW_SHAPE(const TCollection_AsciiString& name,
                 const TopoDS_Shape&            shape,
                 const ActAPI_Color&            color,
                 const double                   opacity,
                 const bool                     isWireframe)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_SHAPE(name, shape, color, opacity, isWireframe);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_LINK(const gp_Pnt&                  p1,
              const gp_Pnt&                  p2,
              const ActAPI_Color&            color,
              const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_LINK(p1, p2, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_LINK(const gp_XYZ&                  p1,
              const gp_XYZ&                  p2,
              const ActAPI_Color&            color,
              const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_LINK(p1, p2, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_LINK(const gp_Pnt2d&                p1,
              const gp_Pnt2d&                p2,
              const ActAPI_Color&            color,
              const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_LINK(p1, p2, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_LINK(const gp_XY&                   p1,
              const gp_XY&                   p2,
              const ActAPI_Color&            color,
              const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_LINK(p1, p2, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_LINK(const TCollection_AsciiString& name,
                const gp_Pnt&                  p1,
                const gp_Pnt&                  p2,
                const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_LINK(name, p1, p2, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_LINK(const TCollection_AsciiString& name,
                const gp_XYZ&                  p1,
                const gp_XYZ&                  p2,
                const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_LINK(name, p1, p2, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_LINK(const TCollection_AsciiString& name,
                const gp_Pnt2d&                p1,
                const gp_Pnt2d&                p2,
                const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_LINK(name, p1, p2, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_LINK(const TCollection_AsciiString& name,
                const gp_XY&                   p1,
                const gp_XY&                   p2,
                const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_LINK(name, p1, p2, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POLYLINE(const std::vector<gp_XY>&      poles,
                  const ActAPI_Color&            color,
                  const TCollection_AsciiString& name)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POLYLINE(poles, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_POLYLINE(const std::vector<gp_XYZ>&     poles,
                  const ActAPI_Color&            color,
                  const TCollection_AsciiString& name)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_POLYLINE(poles, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POLYLINE(const TCollection_AsciiString& name,
                    const std::vector<gp_XY>&      poles,
                    const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POLYLINE(name, poles, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_POLYLINE(const TCollection_AsciiString& name,
                    const std::vector<gp_XYZ>&     poles,
                    const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_POLYLINE(name, poles, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_TRIANGLE(const gp_Pnt&                  p1,
                  const gp_Pnt&                  p2,
                  const gp_Pnt&                  p3,
                  const ActAPI_Color&            color,
                  const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_TRIANGLE(p1, p2, p3, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_TRIANGLE(const gp_XYZ&                  p1,
                  const gp_XYZ&                  p2,
                  const gp_XYZ&                  p3,
                  const ActAPI_Color&            color,
                  const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_TRIANGLE(p1, p2, p3, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_TRIANGLE(const gp_Pnt2d&                p1,
                  const gp_Pnt2d&                p2,
                  const gp_Pnt2d&                p3,
                  const ActAPI_Color&            color,
                  const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_TRIANGLE(p1, p2, p3, color, name);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_TRIANGLE(const gp_XY&                   p1,
                  const gp_XY&                   p2,
                  const gp_XY&                   p3,
                  const ActAPI_Color&            color,
                  const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_TRIANGLE(p1, p2, p3, color, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_TRIANGLE(const TCollection_AsciiString& name,
                    const gp_Pnt&                  p1,
                    const gp_Pnt&                  p2,
                    const gp_Pnt&                  p3,
                    const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_TRIANGLE(name, p1, p2, p3, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_TRIANGLE(const TCollection_AsciiString& name,
                    const gp_XYZ&                  p1,
                    const gp_XYZ&                  p2,
                    const gp_XYZ&                  p3,
                    const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_TRIANGLE(name, p1, p2, p3, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_TRIANGLE(const TCollection_AsciiString& name,
                    const gp_Pnt2d&                p1,
                    const gp_Pnt2d&                p2,
                    const gp_Pnt2d&                p3,
                    const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_TRIANGLE(name, p1, p2, p3, color);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_TRIANGLE(const TCollection_AsciiString& name,
                    const gp_XY&                   p1,
                    const gp_XY&                   p2,
                    const gp_XY&                   p3,
                    const ActAPI_Color&            color)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_TRIANGLE(name, p1, p2, p3, color);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_TRIANGULATION(const Handle(Poly_Triangulation)& tess,
                       const ActAPI_Color&               color,
                       const double                      opacity,
                       const TCollection_AsciiString&    name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_TRIANGULATION(tess, color, opacity, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_TRIANGULATION(const TCollection_AsciiString&    name,
                         const Handle(Poly_Triangulation)& tess,
                         const ActAPI_Color&               color,
                         const double                      opacity)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_TRIANGULATION(name, tess, color, opacity);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_MESH(const Handle(ActData_Mesh)&    tess,
              const ActAPI_Color&            color,
              const double                   opacity,
              const double                   edgeWidth,
              const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_MESH(tess, color, opacity, edgeWidth, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_MESH(const TCollection_AsciiString& name,
                const Handle(ActData_Mesh)&    tess,
                const ActAPI_Color&            color,
                const double                   opacity,
                const double                   edgeWidth)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_MESH(name, tess, color, opacity, edgeWidth);
  }

//---------------------------------------------------------------------------//

  void
    DRAW_TEXT(const TCollection_AsciiString& text,
              const TCollection_AsciiString& name = "")
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->DRAW_TEXT(text, name);
  }

//---------------------------------------------------------------------------//

  void
    REDRAW_TEXT(const TCollection_AsciiString& name,
                const TCollection_AsciiString& text)
  {
    if ( m_iv.IsNull() ) return;
    //
    m_iv->REDRAW_TEXT(name, text);
  }

//---------------------------------------------------------------------------//

private:

  Handle(ActAPI_IPlotter) m_iv; //!< IV instance.

};

#endif
