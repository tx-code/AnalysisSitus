//-----------------------------------------------------------------------------
// Created on: April 2012
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

#ifndef ActAux_ShapeFactory_HeaderFile
#define ActAux_ShapeFactory_HeaderFile

// Active Data (auxiliary) includes
#include <ActAux_Common.h>

//! \ingroup ALGO
//!
//! Provides a set of handy methods constructing different OCCT
//! topological shapes.
class ActAux_ShapeFactory
{
// Primitives:
public:

  ActData_EXPORT static TopoDS_Shape
    Box(const Standard_Real DX, const Standard_Real DY, const Standard_Real DZ);

  ActData_EXPORT static TopoDS_Shape
    Sphere(const Standard_Real R);

  ActData_EXPORT static TopoDS_Shape
    Sphere(const Standard_Real R, const Standard_Real A1, const Standard_Real A2);

  ActData_EXPORT static TopoDS_Shape
    Cone(const Standard_Real R1, const Standard_Real R2, const Standard_Real H);

  ActData_EXPORT static TopoDS_Shape
    Cylinder(const Standard_Real R, const Standard_Real H);

  ActData_EXPORT static TopoDS_Shape
    Torus(const Standard_Real R1, const Standard_Real R2);

  ActData_EXPORT static TopoDS_Shape
    Torus(const Standard_Real R1, const Standard_Real R2,
          const Standard_Real A1, const Standard_Real A2);

  ActData_EXPORT static TopoDS_Shape
    MakeTransform(const TopoDS_Shape& theShape,
                  const gp_Trsf& theTrsf,
                  const Standard_Boolean doCopy);

private:

  ActAux_ShapeFactory(); //!< Prohibited.
  ActAux_ShapeFactory(const ActAux_ShapeFactory&); //!< Prohibited.

};

#endif
