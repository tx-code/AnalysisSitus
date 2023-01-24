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

// Own include
#include <asiAlgo_OBB.h>

// OCCT includes
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

//-----------------------------------------------------------------------------

gp_Trsf asiAlgo_OBB::BuildTrsf() const
{
  gp_Trsf T;
  T.SetTransformation(this->Placement);
  T.Invert();
  return T;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_OBB::BuildSolid(gp_Trsf& T) const
{
  T = this->BuildTrsf();

  // Do not attempt to build a solid for degenerated case.
  gp_XYZ       d = this->LocalCornerMax.XYZ() - this->LocalCornerMin.XYZ();
  const double dx = d.X();
  const double dy = d.Y();
  const double dz = d.Z();
  //
  if ((dx <= Precision::Confusion()) ||
      (dy <= Precision::Confusion()) ||
      (dz <= Precision::Confusion()))
  {
    return TopoDS_Shape();
  }

  // Build a properly located box solid representing OBB.
  TopoDS_Shape solid;
  try
  {
    BRepPrimAPI_MakeBox mkOBB(this->LocalCornerMin, this->LocalCornerMax);
    solid = BRepBuilderAPI_Transform(mkOBB.Solid(), T, 1);
  }
  catch (...) {}

  return solid;
}

//-----------------------------------------------------------------------------

void asiAlgo_OBB::BuildMedialAxis(gp_Pnt& P1,
                                  gp_Pnt& P2) const
{
  // ZOY projection
  gp_Pnt P1_ZOY, P2_ZOY;
  {
    double X = 0;
    double Ymin = this->LocalCornerMin.Y();
    double Ymax = this->LocalCornerMax.Y();
    double Z = (this->LocalCornerMin.Z() + this->LocalCornerMax.Z()) * 0.5;
    //
    P1_ZOY = gp_Pnt(X, Ymin, Z);
    P2_ZOY = gp_Pnt(X, Ymax, Z);
  }
  const double d_ZOY = P1_ZOY.Distance(P2_ZOY);

  // XOY projection
  gp_Pnt P1_XOY, P2_XOY;
  {
    double Xmin = this->LocalCornerMin.X();
    double Xmax = this->LocalCornerMax.X();
    double Y = (LocalCornerMin.Y() + LocalCornerMax.Y()) * 0.5;
    double Z = 0.0;
    //
    P1_XOY = gp_Pnt(Xmin, Y, Z);
    P2_XOY = gp_Pnt(Xmax, Y, Z);
  }
  const double d_XOY = P1_XOY.Distance(P2_XOY);

  // XOZ projection
  gp_Pnt P1_XOZ, P2_XOZ;
  {
    double X = (this->LocalCornerMin.X() + this->LocalCornerMax.X()) * 0.5;
    double Y = 0.0;
    double Zmin = this->LocalCornerMin.Z();
    double Zmax = this->LocalCornerMax.Z();
    //
    P1_XOZ = gp_Pnt(X, Y, Zmin);
    P2_XOZ = gp_Pnt(X, Y, Zmax);
  }
  const double d_XOZ = P1_XOZ.Distance(P2_XOZ);

  // Choose the result for case when cylinder height is smaller than its radius.
  const double deviation = 0.05; // 5 % deviation.
  const double maxSide = Max(Max(d_XOY, d_XOZ), d_ZOY);
  //
  if ((maxSide - d_XOY < deviation * maxSide) &&
      (maxSide - d_XOZ < deviation * maxSide) &&
      (d_ZOY < maxSide * (1.0 - deviation)) &&
      (d_ZOY != 0.0))
  {
    P1 = gp_Pnt(P1_XOZ.X(), P1_ZOY.Y(), P1_ZOY.Z());
    P2 = gp_Pnt(P1_XOZ.X(), P2_ZOY.Y(), P2_ZOY.Z());
    return;
  }
  else if ((maxSide - d_XOY < deviation * maxSide) &&
      (maxSide - d_ZOY < deviation * maxSide) &&
      (d_XOZ < maxSide * (1.0 - deviation)) &&
      (d_XOZ != 0.0))
  {
    P1 = gp_Pnt(P1_XOZ.X(), P1_XOY.Y(), P1_XOZ.Z());
    P2 = gp_Pnt(P2_XOZ.X(), P1_XOY.Y(), P2_XOZ.Z());
    return;
  }
  else if ((maxSide - d_XOZ < deviation * maxSide) &&
      (maxSide - d_ZOY < deviation * maxSide) &&
      (d_XOY < maxSide * (1.0 - deviation)) &&
      (d_XOY != 0.0))
  {
    P1 = gp_Pnt(P1_XOY.X(), P1_XOY.Y(), P1_ZOY.Z());
    P2 = gp_Pnt(P2_XOY.X(), P2_XOY.Y(), P1_ZOY.Z());
    return;
  }

  // Choose the result
  if ((d_ZOY > d_XOY) && (d_ZOY > d_XOZ))
  {
    P1 = gp_Pnt(P1_XOZ.X(), P1_ZOY.Y(), P1_ZOY.Z());
    P2 = gp_Pnt(P1_XOZ.X(), P2_ZOY.Y(), P2_ZOY.Z());
  }
  else if ((d_XOY > d_ZOY) && (d_XOY > d_XOZ))
  {
    P1 = gp_Pnt(P1_XOY.X(), P1_XOY.Y(), P1_ZOY.Z());
    P2 = gp_Pnt(P2_XOY.X(), P2_XOY.Y(), P1_ZOY.Z());
  }
  else
  {
    P1 = gp_Pnt(P1_XOZ.X(), P1_XOY.Y(), P1_XOZ.Z());
    P2 = gp_Pnt(P2_XOZ.X(), P1_XOY.Y(), P2_XOZ.Z());
  }
}

//-----------------------------------------------------------------------------

TopoDS_Shape
asiAlgo_OBB::BuildEquiCylinder(gp_Trsf& T,
                               gp_Ax2& Ax2) const
{
  // Calculate height
  gp_Pnt boundMin, boundMax;
  this->BuildMedialAxis(boundMin, boundMax);
  const double height = boundMin.Distance(boundMax);

  // Build solid to compute its volume
  TopoDS_Shape box = this->BuildSolid(T);
  //
  if (box.IsNull())
    return TopoDS_Shape();

  // Calculate volume
  GProp_GProps props;
  BRepGProp::VolumeProperties(box, props);
  const double volume = props.Mass();

  // Calculate radius
  const double radius = Sqrt(volume / (M_PI * height));

  // Build a primitive
  gp_Pnt localOrigin = boundMin;
  gp_Dir localAxis = boundMax.XYZ() - boundMin.XYZ();
  //
  Ax2 = gp_Ax2(localOrigin, localAxis);
  //
  BRepPrimAPI_MakeCylinder mkCyl(Ax2, radius, height);
  TopoDS_Shape solid = BRepBuilderAPI_Transform(mkCyl.Solid(), T, 1);
  return solid;
}

//-----------------------------------------------------------------------------

TopoDS_Shape
asiAlgo_OBB::BuildCircumscribedCylinder(double& radius, double& height) const
{
  // Calculate height
  gp_Pnt boundMin, boundMax;
  this->BuildMedialAxis(boundMin, boundMax);
  height = boundMin.Distance(boundMax);
  //
  radius = LocalCornerMin.Distance(boundMin);
  // Build a primitive
  gp_Pnt localOrigin = boundMin;
  gp_Dir localAxis = boundMax.XYZ() - boundMin.XYZ();
  //
  gp_Ax2 Ax2 = gp_Ax2(localOrigin, localAxis);
  //
  BRepPrimAPI_MakeCylinder mkCyl(Ax2, radius, height);
  TopoDS_Shape solid = BRepBuilderAPI_Transform(mkCyl.Solid(), Trsf.Inverted(), 1);
  return solid;
}

//-----------------------------------------------------------------------------

TopoDS_Shape
asiAlgo_OBB::BuildCircumscribedCylinder() const
{
  double radius = 0;
  double height = 0;
  return BuildCircumscribedCylinder(radius, height);
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_OBB::BuildCircumscribedSphere(double& radius) const
{
  // Build a properly located sphere solid
  TopoDS_Shape solid;
  try
  {
    double xLen = abs(LocalCornerMax.X() - LocalCornerMin.X());
    double yLen = abs(LocalCornerMax.Y() - LocalCornerMin.Y());
    double zLen = abs(LocalCornerMax.Z() - LocalCornerMin.Z());
    gp_Pnt centerPnt = (LocalCornerMax.XYZ() + LocalCornerMin.XYZ()) * 0.5;
    radius = sqrt(xLen * xLen + yLen * yLen + zLen * zLen) * 0.5;
    // Build a primitive
    BRepPrimAPI_MakeSphere mkSphere(centerPnt.Transformed(Trsf.Inverted()), radius);
    return mkSphere.Shape();
  }
  catch (...) {}

  return solid;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_OBB::BuildCircumscribedSphere() const
{
  double radius = 0;
  return BuildCircumscribedSphere(radius);
}
