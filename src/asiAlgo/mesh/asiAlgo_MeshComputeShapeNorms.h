//-----------------------------------------------------------------------------
// Created on: 17 March 2021
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

#ifndef asiAlgo_MeshComputeShapeNorms_h
#define asiAlgo_MeshComputeShapeNorms_h

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <Poly_Connect.hxx>
#include <Poly_Triangulation.hxx>
#include <Prs3d_Drawer.hxx>
#include <Standard.hxx>
#include <Standard_Macro.hxx>
#include <TColgp_Array1OfDir.hxx>

class TopoDS_Face;
class TopLoc_Location;
class TopoDS_Shape;
class Prs3d_Drawer;
class Poly_Triangulation;
class Poly_Connect;

//! Originates from StdPrs_ToolTriangulatedShape.
class asiAlgo_MeshComputeShapeNorms
{
public:

  //! Similar to BRepTools::Triangulation() but without extra checks.
  //! @return true if all faces within shape are triangulated.
  asiAlgo_EXPORT static bool
    IsTriangulated(const TopoDS_Shape& theShape);

  //! Checks back faces visibility for specified shape (to activate back-face culling). <br>
  //! @return true if shape is closed manifold Solid or compound of such Solids. <br>
  asiAlgo_EXPORT static bool
    IsClosed(const TopoDS_Shape& theShape);

  //! Computes nodal normals for Poly_Triangulation structure using UV coordinates and surface.
  //! Does nothing if triangulation already defines normals.
  //! @param theFace [in] the face
  //! @param theTris [in] the definition of a face triangulation
  static void ComputeNormals(const TopoDS_Face& theFace,
                             const Handle(Poly_Triangulation)& theTris)
  {
    Poly_Connect aPolyConnect;
    ComputeNormals (theFace, theTris, aPolyConnect);
  }

  //! Computes nodal normals for Poly_Triangulation structure using UV coordinates and surface.
  //! Does nothing if triangulation already defines normals.
  //! @param theFace [in] the face
  //! @param theTris [in] the definition of a face triangulation
  //! @param thePolyConnect [in,out] optional, initialized tool for exploring triangulation
  asiAlgo_EXPORT static void
    ComputeNormals(const TopoDS_Face& theFace,
                   const Handle(Poly_Triangulation)& theTris,
                   Poly_Connect& thePolyConnect);

  //! Evaluate normals for a triangle of a face.
  //! @param theFace [in] the face.
  //! @param thePolyConnect [in] the definition of a face triangulation.
  //! @param theNormal [out] the array of normals for each triangle.
  asiAlgo_EXPORT static void
    Normal(const TopoDS_Face& theFace,
           Poly_Connect& thePolyConnect,
           TColgp_Array1OfDir& theNormals);
};

#endif
