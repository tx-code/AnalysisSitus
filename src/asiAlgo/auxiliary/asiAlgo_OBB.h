//-----------------------------------------------------------------------------
// Created on: 03 February 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Elizaveta Krylova
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

#ifndef asiAlgo_OBB_h
#define asiAlgo_OBB_h

// Asi include
#include <asiAlgo.h>

// OCCT includes
#include <gp_Ax3.hxx>
#include <TopoDS_Shape.hxx>

//-----------------------------------------------------------------------------

//! Oriented Bounding Box.
struct asiAlgo_OBB
{
  gp_Ax3  Placement;      //!< Placement of the local axes of the OBB.
  gp_Trsf Trsf;           //!< Transformation.
  gp_Pnt  LocalCornerMin; //!< Min corner.
  gp_Pnt  LocalCornerMax; //!< Max corner.

  // Dumps OBB to the output stream.
  void Dump() const
  {
    // Access data
    const gp_Pnt& pos = this->Placement.Location();
    const gp_Dir& OZ  = this->Placement.Direction();
    const gp_Dir& OX  = this->Placement.XDirection();
    //
    const gp_Pnt& MinCorner = this->LocalCornerMin;
    const gp_Pnt& MaxCorner = this->LocalCornerMax;

    // Dump
    std::cout << "Pos       = (" << pos.X()       << ", " << pos.Y()       << ", " << pos.Z()       << ")" << std::endl;
    std::cout << "OZ        = (" << OZ.X()        << ", " << OZ.Y()        << ", " << OZ.Z()        << ")" << std::endl;
    std::cout << "OX        = (" << OX.X()        << ", " << OX.Y()        << ", " << OX.Z()        << ")" << std::endl;
    std::cout << "MinCorner = (" << MinCorner.X() << ", " << MinCorner.Y() << ", " << MinCorner.Z() << ")" << std::endl;
    std::cout << "MaxCorner = (" << MaxCorner.X() << ", " << MaxCorner.Y() << ", " << MaxCorner.Z() << ")" << std::endl;
  }

  //! Builds transformation to move object to the OBB placement.
  //! \return placement transformation.
  asiAlgo_EXPORT gp_Trsf
    BuildTrsf() const;

  //! Creates a topological solid representing the oriented bounding box. This
  //! solid will be positioned according to the known placement.
  //! \param[out] T the outcome placement transformation.
  //! \return the constructed solid.
  asiAlgo_EXPORT TopoDS_Shape
    BuildSolid(gp_Trsf& T) const;

  //! Builds a medial line segment representing the longer OBB axis.
  //! \param[out] P1 the first point on the axial range.
  //! \param[out] P2 the second point on the axial range.
  asiAlgo_EXPORT void
    BuildMedialAxis(gp_Pnt& P1,
                    gp_Pnt& P2) const;

  //! Builds an equivalent cylinder for the OBB of a part in question.
  //! \param[out] T   the placement transformation.
  //! \param[out] Ax2 the local axes of the constructed cylinder.
  //! \return equivalent tight cylinder.
  asiAlgo_EXPORT TopoDS_Shape
    BuildEquiCylinder(gp_Trsf& T,
                      gp_Ax2&  Ax2) const;

  //! Builds an oriented circumscribed cylinder for the part.
  //! \return cylinder.
  asiAlgo_EXPORT TopoDS_Shape
    BuildCircumscribedCylinder() const;

  //! Builds an oriented circumscribed sphere for the part.
  //! \return sphere.
  asiAlgo_EXPORT TopoDS_Shape
    BuildCircumscribedSphere() const;

  //! Builds an oriented circumscribed cylinder for the part.
  //! \param[out] Ax2    the local axes of the constructed cylinder.
  //! \param[out] radius the radius of the constructed cylinder.
  //! \param[out] height the height of the constructed cylinder.
  //! \return cylinder.
  asiAlgo_EXPORT TopoDS_Shape
    BuildCircumscribedCylinder(double& radius,
                               double& height) const;

  //! Builds an oriented circumscribed sphere for the part.
  //! \param[out] radius the radius of the constructed sphere.
  //! \return sphere.
  asiAlgo_EXPORT TopoDS_Shape
    BuildCircumscribedSphere(double& radius) const;
};

#endif
