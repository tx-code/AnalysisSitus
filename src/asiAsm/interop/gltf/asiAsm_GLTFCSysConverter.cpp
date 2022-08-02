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

// Own include
#include <asiAsm_GLTFCSysConverter.h>

// OpenCascade includes
#include <gp_Quaternion.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

glTFCSysConverter::glTFCSysConverter()
//
: m_fInputLengthUnit  (-1.0),
  m_fOutputLengthUnit (-1.0),
  m_bHasInputAx3      (false),
  m_bHasOutputAx3     (false),
  m_fUnitFactor       (1.0),
  m_bHasScale         (false),
  m_bIsEmpty          (true)
{}

//-----------------------------------------------------------------------------

void glTFCSysConverter::Init(const gp_Ax3& inputSystem,
                             double        inputLengthUnit,
                             const gp_Ax3& outputSystem,
                             double        outputLengthUnit)
{
  m_fInputLengthUnit  = inputLengthUnit;
  m_fOutputLengthUnit = outputLengthUnit;
  m_inputAx3          = inputSystem;
  m_outputAx3         = outputSystem;

  if ( inputLengthUnit > 0.0 && outputLengthUnit > 0.0 )
  {
    m_fUnitFactor = inputLengthUnit / outputLengthUnit;
    m_bHasScale   = Abs(m_fUnitFactor - 1.0) > gp::Resolution();
  }
  else
  {
    m_fUnitFactor = 1.0;
    m_bHasScale   = false;
  }

  gp_Trsf trsf;
  if ( m_bHasInputAx3 && m_bHasOutputAx3 )
  {
    trsf.SetTransformation(outputSystem, inputSystem);

    if ( trsf.TranslationPart().IsEqual( gp_XYZ(0.0, 0.0, 0.0), gp::Resolution() ) &&
         trsf.GetRotation().IsEqual( gp_Quaternion() ) )
    {
      trsf = gp_Trsf();
    }
  }

  m_trsf    = trsf;
  m_trsfInv = trsf.Inverted();
  m_trsf.GetMat4(m_normTrsf);
  m_bIsEmpty = !m_bHasScale && m_trsf.Form() == gp_Identity;
}
