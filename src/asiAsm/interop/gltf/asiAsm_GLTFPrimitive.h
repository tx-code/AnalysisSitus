//-----------------------------------------------------------------------------
// Created on: 03 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Julia Slyadneva
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
struct glTFPrimitive
{
  glTFPrimitive()
  {
    Name                              = "";
    Mode                              = glTFPrimitiveMode::glTFPrimitiveMode_Triangles;
    PosAccessor                       = glTFAccessor();
    NormAccessor                      = glTFAccessor();
    UVAccessor                        = glTFAccessor();
    ColorAccessor                     = glTFAccessor();
    IndAccessor                       = glTFAccessor();
  };

  glTFPrimitiveMode              Mode;           //! how to interpret the vertex data: points, lines or triangles
  TCollection_AsciiString               Name;           //! primitive name
  glTFXdeVisualStyle             Style;          //! primitive material

  glTFAccessor                   PosAccessor;    //!< accessor for nodal positions
  glTFAccessor                   NormAccessor;   //!< accessor for nodal normals
  glTFAccessor                   UVAccessor;     //!< accessor for nodal UV texture coordinates
  glTFAccessor                   ColorAccessor;  //!< accessor for nodal colors
  glTFAccessor                   IndAccessor;    //!< accessor for indices

  NCollection_Vector<gp_XYZ>            NodePositions;  //! vertex positions
  NCollection_Vector<Graphic3d_Vec3>    NodeNormals;    //! vertex normals
  NCollection_Vector<gp_Pnt2d>          NodeTextures;   //! UV texture coordinates per vertex
  NCollection_Vector<Graphic3d_Vec3>    NodeColors;     //! vertex colors
  NCollection_Vector<Poly_Triangle>     NodeIndices;    //! vertex indices
};

} // xde
} // asiAsm

