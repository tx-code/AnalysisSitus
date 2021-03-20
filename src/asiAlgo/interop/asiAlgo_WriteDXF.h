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

// Modified by Quaoar 2020-12-23: added treatment of Poly_Polygon3D edges.

#ifndef asiAlgo_WriteDxf_h
#define asiAlgo_WriteDxf_h

// Dxf by Dan Heeks
#include <dxf.h>

// Forward declarations
class BRepAdaptor_Curve;
class Poly_Polygon3D;

//-----------------------------------------------------------------------------

//! Exports B-rep wireframe to DXF.
//!
//! The code is originally taken from FreeCAD (LGPL terms). We use protected
//! inheritance to expose different API (public) methods.
class asiAlgo_WriteDXF : protected CDxfWrite // By D. Heeks.
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_WriteDXF, CDxfWrite)

public:

  //! Supported versions.
  enum DxfVersion
  {
    DxfVersion_0  = 0, // ???
    DxfVersion_12 = 12,
    DxfVersion_14 = 14
  };

public:

  //! Ctor.
  //! \param[in] filepath the target filename.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_WriteDXF(const char*          filepath,
                     ActAPI_ProgressEntry progress = nullptr,
                     ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! \return false if the output file could not be open.
  asiAlgo_EXPORT bool
    CanOpen() const;

  //! Sets max segment length for the discretization.
  //! \param[in] val the value to set.
  asiAlgo_EXPORT void
    SetSegmentLength(const double val);

  //! Sets DXF format version.
  //! \param[in] ver the version to set.
  asiAlgo_EXPORT void
    SetDxfVersion(const DxfVersion ver);

  //! Sets DXF format version as int.
  //! \param[in] ver the version to set.
  asiAlgo_EXPORT void
    SetDxfVersion(const int ver);

  //! Sets the auto-orientation mode on/off. If enabled, the shape
  //! will be relocated to XOY plane, which is the default drawing
  //! plane for the DXF writer. It is assumed that the shape is planar,
  //! so any of its faces can be taken for computing the reference frame.
  //! \param[in] on the Boolean value to set.
  asiAlgo_EXPORT void
    SetAutoOrient(const bool on);

  //! Exports the edges of the passed shape to the DXF file with the
  //! specified filename.
  //! \param[in] shape the shape to export.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const TopoDS_Shape& shape);

protected:

  //! Exports the passed shape to DXF.
  //! \param[in] shape the shape to export.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    exportShape(const TopoDS_Shape& shape);

protected:

  //! Exports the passed curve adapter as a circle.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportCircle(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as a circular arc.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportArc(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as an ellipse.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportEllipse(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as an elliptical arc.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportEllipseArc(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as a B-curve.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportBSpline(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as a Bezier curve.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportBezier(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as a straight line.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportLine(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as lwpolyline entity.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportLWPoly(const BRepAdaptor_Curve& c);

  //! Exports the passed curve adapter as polyline.
  //! \param[in] c the curve to export.
  asiAlgo_EXPORT void
    exportPolyline(const BRepAdaptor_Curve& c);

  //! Exports the passed polygon as polyline.
  //! \param[in] c the curve to export.
  //! \param[in] T the transformation to apply.
  asiAlgo_EXPORT void
    exportPolyline(const Handle(Poly_Polygon3D)& c,
                   const gp_Trsf&                T);

protected:

  //! Discretization step.
  double m_fSegLength;

  //! Indicates whether auto-orientation mode is enabled.
  bool m_bAutoOrient;

};

#endif // asiAlgo_WriteDxf_h
