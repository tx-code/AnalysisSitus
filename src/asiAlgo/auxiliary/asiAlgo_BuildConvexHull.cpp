//-----------------------------------------------------------------------------
// Created on: 03 August 2021
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
#include <asiAlgo_BuildConvexHull.h>

// asiAlgo includes
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_MeshInfo.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_QuickHull.h>

// OCCT includes
#include <gp_Pln.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

bool asiAlgo_BuildConvexHull::Perform(const TopoDS_Shape&         shape,
                                      Handle(Poly_Triangulation)& hull)
{
  // Check if the shape should be meshed beforehand.
  asiAlgo_MeshInfo meshInfo = asiAlgo_MeshInfo::Extract(shape);
  //
  if ( !meshInfo.nFacets )
  {
    const double linDefl = asiAlgo_MeshGen::AutoSelectLinearDeflection(shape);
    const double angDefl = asiAlgo_MeshGen::AutoSelectAngularDeflection(shape);

    if ( !asiAlgo_MeshGen::DoNative(shape,
                                    linDefl,
                                    angDefl,
                                    meshInfo) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Failed to mesh body shape.");
      return false;
    }

    m_progress.SendLogMessage(LogInfo(Normal) << "The body shape was tessellated with "
                                                 "linear deflection %1 and angular deflection %2."
                                              << linDefl << angDefl);
  }

  // Merge meshes into one piece.
  asiAlgo_MeshMerge meshMerge(shape);
  //
  Handle(Poly_Triangulation)
    inputTris = meshMerge.GetResultPoly()->GetTriangulation();

  if ( inputTris.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Input triangulation is null.");
    return false;
  }

  return this->Perform(inputTris->Nodes(), hull);
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildConvexHull::Perform(const std::vector<gp_Pnt>&  data,
                                      Handle(Poly_Triangulation)& hull)
{
  if ( data.size() < 4 )
    return false; // Early exit.

  // Prepare data.
  std::vector<double> convertedData;
  for( int i = 0; i < data.size(); ++i )
  {
    convertedData.push_back(data[i].Coord(1));
    convertedData.push_back(data[i].Coord(2));
    convertedData.push_back(data[i].Coord(3));
  }

  return this->Perform(convertedData, hull);
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildConvexHull::Perform(const TColgp_Array1OfPnt&   data,
                                      Handle(Poly_Triangulation)& hull)
{
  if ( data.Size() < 4 )
    return false; // Early exit.

  // Prepare data.
  std::vector<double> convertedData;
  for( int i = 1; i <= data.Size(); ++i )
  {
    convertedData.push_back(data[i].Coord(1));
    convertedData.push_back(data[i].Coord(2));
    convertedData.push_back(data[i].Coord(3));
  }

  return this->Perform(convertedData, hull);
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildConvexHull::Perform(const std::vector<double>&  data,
                                      Handle(Poly_Triangulation)& hull)
{
  if ( data.size() < 12 )
    return false;

  // Antti Kuukka's implementation does not check input data validity.
  // The next block is intended to check initial simplex existence. This
  // block protects from data of lower dimensions (0D, 1D, 2D). The idea is
  // to construct 3D coordinate system. Its existence means that convex hull
  // can be constructed.
  {
    bool isInputValid = false;
    const gp_Pnt origin(data[0], data[1], data[2]);
    gp_Vec axis1(0.0, 0.0, 0.0);
    gp_Vec axis2(0.0, 0.0, 0.0);

    for ( int i = 3; i < (int)data.size(); i += 3 )
    {
      gp_Pnt pnt(data[i], data[i + 1], data[i + 2]);
      gp_Vec axis(origin, pnt);
      if ( axis.SquareMagnitude() < Precision::Confusion() )
        continue; // No-axis.

      if ( axis1.SquareMagnitude() < gp::Resolution() )
      {
        // Set first axis.
        axis1 = axis.Normalized();
        continue;
      }

      // Check that current axis differs from the first axis.
      const double ANGLE_THRESHOLD = 0.001;
      const double angle1 = axis1.Angle(axis);
      if ( angle1 < ANGLE_THRESHOLD || angle1 > M_PI - ANGLE_THRESHOLD )
        continue; // Current axis is collinear to the first one.

      if ( axis2.SquareMagnitude() < gp::Resolution() )
      {
        // Set second axis.
        axis2 = axis.Normalized();
        continue;
      }

      // Current point should lie outside the plane.
      gp_Pln plane = gp_Pln(origin, axis1.Crossed(axis2));
      if ( plane.Distance(pnt) < Precision::Confusion() )
        continue;

      isInputValid = true;
      break;
    }

    if ( !isInputValid )
      return false;
  }

  const int size = (int) (data.size() / 3);
  double tol = Precision::Confusion() * 0.01;

  // Type declaration (C++11 style) to avoid long type names.
  using HalfEdgeMesh = quickhull::HalfEdgeMesh<double, quickhull::IndexType>;

  // Construct
  quickhull::QuickHull<double> quickHull;
  HalfEdgeMesh heMesh = quickHull.getConvexHullAsMesh(&data[0], size, true, tol);

  const std::vector<quickhull::Vector3<double>>& vertices = heMesh.m_vertices;
  const std::vector<HalfEdgeMesh::Face>&         faces    = heMesh.m_faces;

  // Nodes.
  hull = new Poly_Triangulation((int) vertices.size(), (int) faces.size(), false);
  for( int i = 0; i < hull->NbNodes(); ++i )
    hull->ChangeNode(i + 1).SetCoord(vertices[i].x, vertices[i].y, vertices[i].z);

  // Triangles.
  for( int i = 1; i <= hull->NbTriangles(); ++i )
  {
    HalfEdgeMesh::Face face = faces[i - 1];
    HalfEdgeMesh::HalfEdge he1 = heMesh.m_halfEdges[face.m_halfEdgeIndex];
    HalfEdgeMesh::HalfEdge he2 = heMesh.m_halfEdges[he1.m_next];
    HalfEdgeMesh::HalfEdge he3 = heMesh.m_halfEdges[he2.m_next];

    // Half-edge mesh uses zero-based indexation.
    hull->ChangeTriangle(i).Set((int) he1.m_endVertex + 1, (int) he2.m_endVertex + 1, (int) he3.m_endVertex + 1);
  }

  return true;
}
