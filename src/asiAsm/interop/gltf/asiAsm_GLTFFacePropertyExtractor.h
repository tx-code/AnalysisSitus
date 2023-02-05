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

#pragma once

// glTF includes
#include <asiAsm_GLTFXdeVisualStyle.h>

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

namespace asiAsm {
namespace xde {

//! Auxiliary class to iterate over triangulated faces.
class glTFFacePropertyExtractor
{
public:

  //! Ctor.
  gltf_EXPORT
    glTFFacePropertyExtractor(const TopoDS_Face& face);

public:

  //! \return current B-rep face.
  gltf_EXPORT const TopoDS_Face&
    Face() const;

  //! \return true if the current mesh is empty.
  gltf_EXPORT bool
    IsEmptyMesh() const;

public:

  //! \return the number of elements of triangles type for the current face.
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
    TriangleOriented(const int elemIndex) const;

public:

  //! Return true if triangulation has defined normals.
  gltf_EXPORT bool
    HasNormals() const;

  //! Return true if 2D nodes are associated with the 3D nodes of the triangulation.
  gltf_EXPORT bool
    HasTexCoords() const;

  //! Return normal at specified node index with face transformation applied and face orientation applied.
  gltf_EXPORT gp_Dir
    NormalTransformed(const int N);

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
    NodeTransformed(const int N) const;

  //! Return texture coordinates for the node.
  gltf_EXPORT gp_Pnt2d
    NodeTexCoord(const int N) const;

private:

  //! Return the node with specified index with applied transformation.
  gltf_EXPORT gp_Pnt
    node(const int N) const;

  //! Return normal at specified node index without face transformation applied.
  gltf_EXPORT gp_Dir
    normal(const int N);

  //! Return triangle with specified index.
  gltf_EXPORT const Poly_Triangle&
    triangle(const int E) const;

  //! Initializes face's properties.
  gltf_EXPORT void
    initFace();

private:

  TopoDS_Face                     m_face;         //!< current face
  Handle(Poly_Triangulation)      m_polyTriang;   //!< triangulation of current face
  TopLoc_Location                 m_faceLocation; //!< current face location
  BRepLProp_SLProps               m_SLTool;       //!< auxiliary tool for fetching normals from surface
  BRepAdaptor_Surface             m_faceAdaptor;  //!< surface adaptor for fetching normals from surface
  bool                            m_bHasNormals;  //!< flag indicating that current face has normals
  gp_Trsf                         m_trsf;         //!< current face transformation
  bool                            m_bMirrored;    //!< flag indicating that face triangles should be mirrored
};
} // xde
} // asiAsm
