//-----------------------------------------------------------------------------
// Created on: 2012-2015
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://dev.opencascade.org
//-----------------------------------------------------------------------------

// Own include
#include <ActAux_ShapeFactory.h>

// OCCT includes
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>

//! Creates BOX solid.
//! \param DX [in] side X.
//! \param DY [in] side Y.
//! \param DZ [in] side Z.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Box(const Standard_Real DX,
                                      const Standard_Real DY,
                                      const Standard_Real DZ)
{
  return BRepPrimAPI_MakeBox(DX, DY, DZ).Shape();
}

//! Creates SPHERE solid.
//! \param R [in] radius.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Sphere(const Standard_Real R)
{
  return BRepPrimAPI_MakeSphere(R).Shape();
}

//! Creates SPHERE solid limited by angle variation range.
//! \param R [in] radius.
//! \param A1 [in] first angle value.
//! \param A2 [in] second angle value.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Sphere(const Standard_Real R,
                                         const Standard_Real A1,
                                         const Standard_Real A2)
{
  return BRepPrimAPI_MakeSphere(R, A1, A2).Shape();
}

//! Creates CONE solid.
//! \param R1 [in] first radius.
//! \param R2 [in] second radius.
//! \param H [in] height.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Cone(const Standard_Real R1,
                                       const Standard_Real R2,
                                       const Standard_Real H)
{
  return BRepPrimAPI_MakeCone(R1, R2, H).Shape();
}

//! Creates CYLINDER solid.
//! \param R [in] radius.
//! \param H [in] height.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Cylinder(const Standard_Real R,
                                           const Standard_Real H)
{
  return BRepPrimAPI_MakeCylinder(R, H).Shape();
}

//! Creates TORUS solid.
//! \param R1 [in] first radius.
//! \param R2 [in] second radius.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Torus(const Standard_Real R1,
                                        const Standard_Real R2)
{
  return BRepPrimAPI_MakeTorus(R1, R2).Shape();
}

//! Creates TORUS solid limited by angle variation range.
//! \param R1 [in] first radius.
//! \param R2 [in] second radius.
//! \param A1 [in] first angle value.
//! \param A2 [in] second angle value.
//! \return created solid.
TopoDS_Shape ActAux_ShapeFactory::Torus(const Standard_Real R1,
                                        const Standard_Real R2,
                                        const Standard_Real A1,
                                        const Standard_Real A2)
{
  return BRepPrimAPI_MakeTorus(R1, R2, A1, A2).Shape();
}

//! Creates a transformed copy of the passed shape.
//! \param theShape [in] shape to copy & transform.
//! \param theTrsf [in] transformation to apply.
//! \param doCopy [in] indicates whether to copy the source shape.
//! \return transformed copy of the passed shape.
TopoDS_Shape ActAux_ShapeFactory::MakeTransform(const TopoDS_Shape& theShape,
                                                const gp_Trsf& theTrsf,
                                                const Standard_Boolean doCopy)
{
  BRepBuilderAPI_Transform aTransformer(theShape, theTrsf, doCopy);
  return aTransformer.Shape();
}
