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
#include <asiAsm_GlTFCSysConverter.h>

// OpenCascade includes
#include <gp_Quaternion.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

gltf_CSysConverter::gltf_CSysConverter()
: myInputLengthUnit (-1.0),
  myOutputLengthUnit(-1.0),
  myHasInputAx3 (false),
  myHasOutputAx3(false),
  //
  myUnitFactor (1),
  myHasScale (false),
  myIsEmpty  (true)
{
  //
}

//-----------------------------------------------------------------------------

void gltf_CSysConverter::Init(const gp_Ax3& theInputSystem,
                              double        theInputLengthUnit,
                              const gp_Ax3& theOutputSystem,
                              double        theOutputLengthUnit)
{
  myInputLengthUnit  = theInputLengthUnit;
  myOutputLengthUnit = theOutputLengthUnit;
  myInputAx3         = theInputSystem;
  myOutputAx3        = theOutputSystem;
  if (theInputLengthUnit  > 0.0
   && theOutputLengthUnit > 0.0)
  {
    myUnitFactor = theInputLengthUnit / theOutputLengthUnit;
    myHasScale = Abs(myUnitFactor - 1.0) > gp::Resolution();
  }
  else
  {
    myUnitFactor = 1.0;
    myHasScale = false;
  }

  gp_Trsf aTrsf;
  if (myHasInputAx3
   && myHasOutputAx3)
  {
    aTrsf.SetTransformation (theOutputSystem, theInputSystem);
    if (aTrsf.TranslationPart().IsEqual (gp_XYZ (0.0, 0.0, 0.0), gp::Resolution())
     && aTrsf.GetRotation().IsEqual (gp_Quaternion()))
    {
      aTrsf = gp_Trsf();
    }
  }

  myTrsf    = aTrsf;
  myTrsfInv = aTrsf.Inverted();
  myTrsf.GetMat4 (myNormTrsf);
  myIsEmpty = !myHasScale && myTrsf.Form() == gp_Identity;
}
