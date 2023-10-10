//-----------------------------------------------------------------------------
// Created on: 07 October 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

// asiAlgo includes
#include <asiAlgo_RTCD.h>

// OpenCascade includes
#include <Poly_Triangulation.hxx>

// Mobius includes
#include <mobius/poly_Jacobian.h>

//-----------------------------------------------------------------------------

//! Convex hull defined as a polyhedron in 3D.
struct asiAlgo_ConvexHull
{
  //! Convex hull as a mesh.
  Handle(Poly_Triangulation) Mesh;

  //! Convex hull as a series of halfspaces defined with planes.
  std::vector<RTCD::Plane> Halfspaces;

  //! \return true if this convex hull is not initialized, i.e., the
  //!         stored mesh is null.
  bool IsNull() const
  {
    return Mesh.IsNull();
  }

  //! Turns the stored convex hull mesh into a collection of planes
  //! in the format of `RTCD` ("Real-time collision detection")
  //! namespace. This representation is useful for computations
  //! based on halfspaces.
  void ComputeHullPlanes()
  {
    for ( int i = 1; i <= Mesh->NbTriangles(); ++i )
    {
      int N[3];
      Mesh->Triangle(i).Get(N[0], N[1], N[2]);

      RTCD::Point a = Mesh->Node(N[0]);
      RTCD::Point b = Mesh->Node(N[1]);
      RTCD::Point c = Mesh->Node(N[2]);

      // Make sure that skewed triangles are skipped.
      mobius::t_xyz v0(a.x, a.y, a.z);
      mobius::t_xyz v1(b.x, b.y, b.z);
      mobius::t_xyz v2(c.x, c.y, c.z);
      //
      double jacobian = DBL_MAX;
      for ( int k = 0; k < 3; ++k )
      {
        mobius::t_uv uv[3];
        double J[2][2] = { {0, 0}, {0, 0} };
        double J_det   = 0.;
        double J_det_n = 0.;

        // Compute for element.
        mobius::poly_Jacobian::Compute(v0, v1, v2, k, uv[0], uv[1], uv[2], J, J_det, J_det_n);

        jacobian = std::min(jacobian, J_det_n);
      }
      //
      if ( jacobian < 0.1 )
        continue;

      RTCD::Plane plane = RTCD::ComputePlane(a, b, c);

      // Set optional anchor point.
      RTCD::Point mid = ( RTCD::Vector(a) + RTCD::Vector(b) + RTCD::Vector(c) )*(1./3.);
      //
      plane.anchor = mid;

      Halfspaces.push_back(plane);
    }
  }
};
