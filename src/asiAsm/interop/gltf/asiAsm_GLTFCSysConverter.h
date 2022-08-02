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
#include <asiAsm_GLTFEntities.h>

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
//! `glTFCoordinateSystem` enumeration is used for convenient conversion between two commonly
//! used coordinate systems, to make sure that imported model is oriented up.
//! Still, `gp_Ax3` can be used instead for defining conversion between arbitrary
//! systems (e.g. including non-zero origin).
//!
//! The converter requires defining explicitly both input and output systems,
//! so that if either input or output is undefined, then conversion will be skipped.
//! Length units conversion and coordinate system conversion are decomposed,
//! so that application might specify no length units conversion but Y-up to Z-up
//! coordinate system conversion.
//!
//! Class defines dedicated methods for parameters of input and output systems.
//! This allows passing tool through several initialization steps,
//! so that a reader can initialize input length units (only if file format
//! defines such information), while application specifies output length units,
//! and conversion will be done only when both defined.
class glTFCSysConverter
{
public:

  //! Return a standard coordinate system definition.
  static gp_Ax3 StandardCoordinateSystem(glTFCoordinateSystem sys)
  {
    switch ( sys )
    {
      case glTFCoordinateSystem_posYfwd_posZup: return gp_Ax3( gp::Origin(), gp::DZ(), gp::DX() );
      case glTFCoordinateSystem_negZfwd_posYup: return gp_Ax3( gp::Origin(), gp::DY(), gp::DX() );
      case glTFCoordinateSystem_Undefined:      break;
    }
    return gp_Ax3();
  }

public:

  //! Empty constructor.
  gltf_EXPORT glTFCSysConverter();

public:

  //! Return TRUE if there is no transformation (target and current coordinates systems are same).
  bool IsEmpty() const
  {
    return m_bIsEmpty;
  }

  //! Return source length units, defined as scale factor to [m] (meters).
  //! -1.0 by default, which means that NO conversion will be
  //! applied (regardless output length unit).
  double InputLengthUnit() const
  {
    return m_fInputLengthUnit;
  }

  //! Set source length units as scale factor to m (meters).
  void SetInputLengthUnit(double inputScale)
  {
    Init(m_inputAx3, inputScale, m_outputAx3, m_fOutputLengthUnit);
  }

  //! Return destination length units, defined as scale factor to [m] (meters).
  //! -1.0 by default, which means that NO conversion will be applied (regardless input length unit).
  double OutputLengthUnit() const
  {
    return m_fOutputLengthUnit;
  }

  //! Set destination length units as scale factor to m (meters).
  void SetOutputLengthUnit(double theOutputScale)
  {
    Init(m_inputAx3, m_fInputLengthUnit, m_outputAx3, theOutputScale);
  }

  //! Return TRUE if source coordinate system has been set; FALSE by default.
  bool HasInputCoordinateSystem() const
  {
    return m_bHasInputAx3;
  }

  //! Source coordinate system; UNDEFINED by default.
  const gp_Ax3& InputCoordinateSystem() const
  {
    return m_inputAx3;
  }

  //! Set source coordinate system.
  void SetInputCoordinateSystem(const gp_Ax3& sysFrom)
  {
    m_bHasInputAx3 = true;
    Init(sysFrom, m_fInputLengthUnit, m_outputAx3, m_fOutputLengthUnit);
  }

  //! Set source coordinate system.
  void SetInputCoordinateSystem(glTFCoordinateSystem sysFrom)
  {
    m_bHasInputAx3 = (sysFrom != glTFCoordinateSystem_Undefined);
    Init(StandardCoordinateSystem(sysFrom), m_fInputLengthUnit, m_outputAx3, m_fOutputLengthUnit);
  }

  //! Return TRUE if destination coordinate system has been set; FALSE by default.
  bool HasOutputCoordinateSystem() const
  {
    return m_bHasOutputAx3;
  }

  //! Destination coordinate system; UNDEFINED by default.
  const gp_Ax3& OutputCoordinateSystem() const
  {
    return m_outputAx3;
  }

  //! Set destination coordinate system.
  void SetOutputCoordinateSystem(const gp_Ax3& sysTo)
  {
    m_bHasOutputAx3 = true;
    Init(m_inputAx3, m_fInputLengthUnit, sysTo, m_fOutputLengthUnit);
  }

  //! Set destination coordinate system.
  void SetOutputCoordinateSystem(glTFCoordinateSystem sysTo)
  {
    m_bHasOutputAx3 = (sysTo != glTFCoordinateSystem_Undefined);
    Init(m_inputAx3, m_fInputLengthUnit, StandardCoordinateSystem(sysTo), m_fOutputLengthUnit);
  }

  //! Initialize transformation.
  gltf_EXPORT void Init(const gp_Ax3& inputSystem,
                        double        inputLengthUnit,
                        const gp_Ax3& outputSystem,
                        double        outputLengthUnit);

public:

  //! Transform transformation.
  void TransformTransformation(gp_Trsf& trsf) const
  {
    if ( m_bHasScale )
    {
      gp_XYZ transPart = trsf.TranslationPart();
      transPart *= m_fUnitFactor;
      trsf.SetTranslationPart(transPart);
    }
    if ( m_trsf.Form() != gp_Identity )
    {
      trsf = m_trsf * trsf * m_trsfInv;
    }
  }

  //! Transform position.
  void TransformPosition(gp_XYZ& pos) const
  {
    if ( m_bHasScale )
    {
      pos *= m_fUnitFactor;
    }
    if ( m_trsf.Form() != gp_Identity )
    {
      m_trsf.Transforms(pos);
    }
  }

  //! Transform normal (e.g. exclude translation/scale part of transformation).
  void TransformNormal(Graphic3d_Vec3& norm) const
  {
    if ( m_trsf.Form() != gp_Identity )
    {
      const Graphic3d_Vec4 _norm = m_normTrsf * Graphic3d_Vec4(norm, 0.0f);
      norm = _norm.xyz();
    }
  }

private:

  gp_Ax3         m_inputAx3;          //!< source      coordinate system
  gp_Ax3         m_outputAx3;         //!< destination coordinate system
  double         m_fInputLengthUnit;  //!< source      length units, defined as scale factor to m (meters); -1.0 by default which means UNDEFINED
  double         m_fOutputLengthUnit; //!< destination length units, defined as scale factor to m (meters); -1.0 by default which means UNDEFINED
  bool           m_bHasInputAx3;      //!< flag indicating if source coordinate system is defined or not
  bool           m_bHasOutputAx3;     //!< flag indicating if destination coordinate system is defined or not

  gp_Trsf        m_trsf;        //!< transformation from input Ax3 to output Ax3
  gp_Trsf        m_trsfInv;     //!< inversed transformation from input Ax3 to output Ax3
  Graphic3d_Mat4 m_normTrsf;    //!< transformation 4x4 matrix from input Ax3 to output Ax3
  double         m_fUnitFactor; //!< unit scale factor
  bool           m_bHasScale;   //!< flag indicating that length unit transformation should be performed
  bool           m_bIsEmpty;    //!< flag indicating that transformation is empty

};

} // xde
} // asiAsm
