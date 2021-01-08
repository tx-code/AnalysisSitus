/***************************************************************************
 *   Copyright (c) OPEN CASCADE SAS                                        *
 *                                                                         *
 *   This file is part of Open CASCADE Technology software library.        *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 ***************************************************************************/

#ifndef gltf_FaceIterator_HeaderFile
#define gltf_FaceIterator_HeaderFile

// glTF includes
#include <gltf_XdeVisualStyle.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// OpenCascade includes
#include <BRepLProp_SLProps.hxx>
#include <gp_Trsf.hxx>
#include <NCollection_DataMap.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

class TDF_Label;

namespace asiAsm
{

//! Auxiliary class to iterate over triangulated faces.
class gltf_FaceIterator
{
public:

  //! Ctor.
  //! \param[in] label       the OCAF label hosting the shape.
  //! \param[in] location    the location to apply.
  //! \param[in] toMapColors the Boolean flag indicating whether to read styles as well.
  //! \param[in] style       the default style to use.
  gltf_EXPORT
    gltf_FaceIterator(const TDF_Label&           label,
                      const TopLoc_Location&     location,
                      const bool                 toMapColors = false,
                      const gltf_XdeVisualStyle& style = gltf_XdeVisualStyle());

public:

  //! Sets diagnostics tools.
  //! \param[in] progress progress notifier.
  //! \param[in] plotter  imperative plotter.
  void SetDiagnosticsTools(ActAPI_ProgressEntry progress,
                           ActAPI_PlotterEntry  plotter)
  {
    m_progress = progress;
    m_plotter  = plotter;
  }

public:

  //! \return true if iterator points to the valid triangulation.
  gltf_EXPORT bool
    More() const;

  //! Moves iterator to the next element.
  gltf_EXPORT void
    Next();

  //! \return current B-rep face.
  gltf_EXPORT const TopoDS_Face&
    Face() const;

  //! \return true if the current mesh is empty.
  gltf_EXPORT bool
    IsEmptyMesh() const;

public:

  //! \return face's material.
  gltf_EXPORT const gltf_XdeVisualStyle&
    FaceStyle() const;

  //! \return true if the face has a color assigned.
  gltf_EXPORT bool
    HasFaceColor() const;

  //! \return face color.
  gltf_EXPORT const Quantity_ColorRGBA&
    FaceColor() const;

public:

  //! \return the number of elements of specific type for the current face.
  gltf_EXPORT int
    NbTriangles() const;

  //! \return the lower element index in the current triangulation.
  gltf_EXPORT int
    ElemLower() const;

  //! \return the upper element index in current triangulation.
  gltf_EXPORT int
    ElemUpper() const;

  //! Returns the triangle of the specified index with applied face's orientation.
  //! \param[in] elemIndex element index.
  //! \return triangle requested.
  gltf_EXPORT Poly_Triangle
    TriangleOriented(int elemIndex) const;

public:

  //! Return true if triangulation has defined normals.
  gltf_EXPORT bool
    HasNormals() const;

  //! Return true if triangulation has defined normals.
  gltf_EXPORT bool
    HasTexCoords() const;

  //! Return normal at specified node index with face transformation applied and face orientation applied.
  gltf_EXPORT gp_Dir
    NormalTransformed(int N);

  //! Return number of nodes for the current face.
  gltf_EXPORT int
    NbNodes() const;

  //! Lower node index in current triangulation.
  gltf_EXPORT int
    NodeLower() const;

  //! Upper node index in current triangulation.
  gltf_EXPORT int
    NodeUpper() const;

  //! Return the node with specified index with applied transformation.
  gltf_EXPORT gp_Pnt
    NodeTransformed(const int theNode) const;

  //! Return texture coordinates for the node.
  gltf_EXPORT gp_Pnt2d
    NodeTexCoord (const int theNode) const;

private:

  //! Return the node with specified index with applied transformation.
  gltf_EXPORT gp_Pnt
    node(const int N) const;

  //! Return normal at specified node index without face transformation applied.
  gltf_EXPORT gp_Dir
    normal(int N);

  //! Return triangle with specified index.
  gltf_EXPORT Poly_Triangle
    triangle(int E) const;

  //! Reads styles from OCAF document to internal map. As a result, each face
  //! gets an associated style.
  gltf_EXPORT void
    readStyles(const TDF_Label&           label,
               const TopLoc_Location&     location,
               const gltf_XdeVisualStyle& style);

  //! Reset information for current face.
  gltf_EXPORT void
    resetFace();

  //! Initializes face's properties.
  gltf_EXPORT void
    initFace();

private:

  NCollection_DataMap<TopoDS_Shape,
                      gltf_XdeVisualStyle,
                      TopTools_ShapeMapHasher>
                                  m_styles;        //!< Face -> Style map
  gltf_XdeVisualStyle             m_defStyle;      //!< default style for faces without dedicated style
  bool                            m_bMapColors;    //!< flag to dispatch styles
  TopExp_Explorer                 m_faceExp;       //!< face explorer
  TopoDS_Face                     m_face;          //!< current face
  Handle(Poly_Triangulation)      m_polyTriang;    //!< triangulation of current face
  TopLoc_Location                 m_faceLocation;  //!< current face location
  BRepLProp_SLProps               m_SLTool;        //!< auxiliary tool for fetching normals from surface
  BRepAdaptor_Surface             m_faceAdaptor;   //!< surface adaptor for fetching normals from surface
  const TColgp_Array1OfPnt*       m_pNodes;        //!< node positions of current face
  const TShort_Array1OfShortReal* m_pNormals;      //!< node normals of current face
  const TColgp_Array1OfPnt2d*     m_pNodeUVs;      //!< node UV coordinates of current face
  bool                            m_bHasNormals;   //!< flag indicating that current face has normals
  gp_Trsf                         m_trsf;          //!< current face transformation
  bool                            m_bMirrored;     //!< flag indicating that face triangles should be mirrored
  gltf_XdeVisualStyle             m_faceStyle;     //!< current face style
  Quantity_ColorRGBA              m_faceColor;     //!< current face color
  bool                            m_bHasFaceColor; //!< flag indicating that current face has assigned color

  /* Diagnostics tools */
  ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

}

#endif
