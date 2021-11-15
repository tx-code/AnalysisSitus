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
#include <asiAsm_GlTFEntities.h>

// OpenCascade includes
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <gp_XYZ.hxx>
#include <gp_Trsf.hxx>
#include <Graphic3d_Mat4.hxx>
#include <Graphic3d_Vec.hxx>

namespace asiAsm {
namespace xde {

//! Coordinate system converter defining the following tools:
//! - Initialization for commonly used coordinate systems Z-up and Y-up.
//! - Perform length unit conversion (scaling).
//! - Conversion of three basic elements:
//!   a) mesh node Positions,
//!   b) mesh node Normals,
//!   c) model nodes Transformations (locations).
//!
//! fraRWMesh_CoordinateSystem enumeration is used for convenient conversion between two commonly
//! used coordinate systems, to make sure that imported model is oriented up.
//! But gp_Ax3 can be used instead for defining a conversion between arbitrary systems (e.g. including non-zero origin).
//!
//! The converter requires defining explicitly both input and output systems,
//! so that if either input or output is undefined, then conversion will be skipped.
//! Length units conversion and coordinate system conversion are decomposed,
//! so that application might specify no length units conversion but Y-up to Z-up coordinate system conversion.
//!
//! Class defines dedicated methods for parameters of input and output systems.
//! This allows passing tool through several initialization steps,
//! so that a reader can initialize input length units (only if file format defines such information),
//! while application specifies output length units, and conversion will be done only when both defined.
class glTFCSysConverter
{
public:

  //! Return a standard coordinate system definition.
  static gp_Ax3 StandardCoordinateSystem(glTFCoordinateSystem theSys)
  {
    switch (theSys)
    {
      case glTFCoordinateSystem_posYfwd_posZup: return gp_Ax3 (gp::Origin(), gp::DZ(), gp::DX());
      case glTFCoordinateSystem_negZfwd_posYup: return gp_Ax3 (gp::Origin(), gp::DY(), gp::DX());
      case glTFCoordinateSystem_Undefined: break;
    }
    return gp_Ax3();
  }

public:

  //! Empty constructor.
  gltf_EXPORT glTFCSysConverter();

  //! Return TRUE if there is no transformation (target and current coordinates systems are same).
  bool IsEmpty() const { return myIsEmpty; }

  //! Return source length units, defined as scale factor to m (meters).
  //! -1.0 by default, which means that NO conversion will be applied (regardless output length unit).
  double InputLengthUnit() const { return myInputLengthUnit; }

  //! Set source length units as scale factor to m (meters).
  void SetInputLengthUnit (double theInputScale)
  {
    Init (myInputAx3, theInputScale, myOutputAx3, myOutputLengthUnit);
  }

  //! Return destination length units, defined as scale factor to m (meters).
  //! -1.0 by default, which means that NO conversion will be applied (regardless input length unit).
  double OutputLengthUnit() const { return myOutputLengthUnit; }

  //! Set destination length units as scale factor to m (meters).
  void SetOutputLengthUnit (double theOutputScale)
  {
    Init (myInputAx3, myInputLengthUnit, myOutputAx3, theOutputScale);
  }

  //! Return TRUE if source coordinate system has been set; FALSE by default.
  bool HasInputCoordinateSystem() const { return myHasInputAx3; }

  //! Source coordinate system; UNDEFINED by default.
  const gp_Ax3& InputCoordinateSystem() const { return myInputAx3; }

  //! Set source coordinate system.
  void SetInputCoordinateSystem (const gp_Ax3& theSysFrom)
  {
    myHasInputAx3 = true;
    Init (theSysFrom, myInputLengthUnit, myOutputAx3, myOutputLengthUnit);
  }

  //! Set source coordinate system.
  void SetInputCoordinateSystem (glTFCoordinateSystem theSysFrom)
  {
    myHasInputAx3 = theSysFrom != glTFCoordinateSystem_Undefined;
    Init (StandardCoordinateSystem (theSysFrom), myInputLengthUnit, myOutputAx3, myOutputLengthUnit);
  }

  //! Return TRUE if destination coordinate system has been set; FALSE by default.
  bool HasOutputCoordinateSystem() const { return myHasOutputAx3; }

  //! Destination coordinate system; UNDEFINED by default.
  const gp_Ax3& OutputCoordinateSystem() const { return myOutputAx3; }

  //! Set destination coordinate system.
  void SetOutputCoordinateSystem (const gp_Ax3& theSysTo)
  {
    myHasOutputAx3 = true;
    Init (myInputAx3, myInputLengthUnit, theSysTo, myOutputLengthUnit);
  }

  //! Set destination coordinate system.
  void SetOutputCoordinateSystem (glTFCoordinateSystem theSysTo)
  {
    myHasOutputAx3 = theSysTo != glTFCoordinateSystem_Undefined;
    Init (myInputAx3, myInputLengthUnit, StandardCoordinateSystem (theSysTo), myOutputLengthUnit);
  }

  //! Initialize transformation.
  gltf_EXPORT void Init(const gp_Ax3& theInputSystem,
                        double theInputLengthUnit,
                        const gp_Ax3& theOutputSystem,
                        double theOutputLengthUnit);

public:

  //! Transform transformation.
  void TransformTransformation (gp_Trsf& theTrsf) const
  {
    if (myHasScale)
    {
      gp_XYZ aTransPart = theTrsf.TranslationPart();
      aTransPart *= myUnitFactor;
      theTrsf.SetTranslationPart (aTransPart);
    }
    if (myTrsf.Form() != gp_Identity)
    {
      theTrsf = myTrsf * theTrsf * myTrsfInv;
    }
  }

  //! Transform position.
  void TransformPosition (gp_XYZ& thePos) const
  {
    if (myHasScale)
    {
      thePos *= myUnitFactor;
    }
    if (myTrsf.Form() != gp_Identity)
    {
      myTrsf.Transforms (thePos);
    }
  }

  //! Transform normal (e.g. exclude translation/scale part of transformation).
  void TransformNormal (Graphic3d_Vec3& theNorm) const
  {
    if (myTrsf.Form() != gp_Identity)
    {
      const Graphic3d_Vec4 aNorm = myNormTrsf * Graphic3d_Vec4 (theNorm, 0.0f);
      theNorm = aNorm.xyz();
    }
  }

private:

  gp_Ax3         myInputAx3;         //!< source      coordinate system
  gp_Ax3         myOutputAx3;        //!< destination coordinate system
  double         myInputLengthUnit;  //!< source      length units, defined as scale factor to m (meters); -1.0 by default which means UNDEFINED
  double         myOutputLengthUnit; //!< destination length units, defined as scale factor to m (meters); -1.0 by default which means UNDEFINED
  bool           myHasInputAx3;      //!< flag indicating if source coordinate system is defined or not
  bool           myHasOutputAx3;     //!< flag indicating if destination coordinate system is defined or not

  gp_Trsf        myTrsf;       //!< transformation from input Ax3 to output Ax3
  gp_Trsf        myTrsfInv;    //!< inversed transformation from input Ax3 to output Ax3
  Graphic3d_Mat4 myNormTrsf;   //!< transformation 4x4 matrix from input Ax3 to output Ax3
  double         myUnitFactor; //!< unit scale factor
  bool           myHasScale;   //!< flag indicating that length unit transformation should be performed
  bool           myIsEmpty;    //!< flag indicating that transformation is empty

};

} // xde
} // asiAsm
