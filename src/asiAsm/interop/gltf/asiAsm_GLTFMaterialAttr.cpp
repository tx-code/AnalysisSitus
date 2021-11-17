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
#include <asiAsm_GLTFMaterialAttr.h>

// OpenCascade includes
#include <Graphic3d_Aspects.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Standard_GUID.hxx>
#include <TDF_Label.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

const Standard_GUID& glTFMaterialAttr::GetID()
{
  static Standard_GUID THE_VIS_MAT_ID ("EBB00255-03A0-4845-BD3B-A70EEDEEFA78");
  return THE_VIS_MAT_ID;
}

//-----------------------------------------------------------------------------

glTFMaterialAttr::glTFMaterialAttr()
: myAlphaMode (Graphic3d_AlphaMode_BlendAuto),
  myAlphaCutOff (0.5f),
  myIsDoubleSided (true)
{
  //
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetPbrMaterial(const glTFMaterialPbr& theMaterial)
{
  Backup();
  myPbrMat = theMaterial;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetCommonMaterial(const glTFMaterialCommon& theMaterial)
{
  Backup();
  myCommonMat = theMaterial;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetAlphaMode(Graphic3d_AlphaMode theMode,
                                     float               theCutOff)
{
  Backup();
  myAlphaMode   = theMode;
  myAlphaCutOff = theCutOff;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetDoubleSided(bool theIsDoubleSided)
{
  Backup();
  myIsDoubleSided = theIsDoubleSided;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::Restore(const Handle(TDF_Attribute)& theWith)
{
  glTFMaterialAttr* anOther = dynamic_cast<glTFMaterialAttr* >(theWith.get());
  myPbrMat        = anOther->myPbrMat;
  myCommonMat     = anOther->myCommonMat;
  myAlphaMode     = anOther->myAlphaMode;
  myAlphaCutOff   = anOther->myAlphaCutOff;
  myIsDoubleSided = anOther->myIsDoubleSided;
}

//-----------------------------------------------------------------------------

Handle(TDF_Attribute) glTFMaterialAttr::NewEmpty() const
{
  return new glTFMaterialAttr();
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::Paste(const Handle(TDF_Attribute)& theInto,
                              const Handle(TDF_RelocationTable)& ) const
{
  glTFMaterialAttr* anOther = dynamic_cast<glTFMaterialAttr* >(theInto.get());
  anOther->Backup();
  anOther->myPbrMat        = myPbrMat;
  anOther->myCommonMat     = myCommonMat;
  anOther->myAlphaMode     = myAlphaMode;
  anOther->myAlphaCutOff   = myAlphaCutOff;
  anOther->myIsDoubleSided = myIsDoubleSided;
}

//-----------------------------------------------------------------------------

Quantity_ColorRGBA glTFMaterialAttr::BaseColor() const
{
  if (myPbrMat.IsDefined)
  {
    return myPbrMat.BaseColor;
  }
  else if (myCommonMat.IsDefined)
  {
    return Quantity_ColorRGBA (myCommonMat.DiffuseColor, 1.0f - myCommonMat.Transparency);
  }
  return Quantity_ColorRGBA (Quantity_NOC_WHITE);
}

//-----------------------------------------------------------------------------

glTFMaterialCommon glTFMaterialAttr::ConvertToCommonMaterial()
{
  if (myCommonMat.IsDefined)
  {
    return myCommonMat;
  }
  else if (!myPbrMat.IsDefined)
  {
    return glTFMaterialCommon();
  }

  // convert metal-roughness into common
  glTFMaterialCommon aComMat;
  aComMat.IsDefined = true;
  aComMat.DiffuseTexture = myPbrMat.BaseColorTexture;
  aComMat.DiffuseColor  = myPbrMat.BaseColor.GetRGB();
  aComMat.SpecularColor = Quantity_Color (Graphic3d_Vec3 (myPbrMat.Metallic));
  aComMat.Transparency = 1.0f - myPbrMat.BaseColor.Alpha();
  aComMat.Shininess    = 1.0f - myPbrMat.Roughness;
  return aComMat;
}

//-----------------------------------------------------------------------------

glTFMaterialPbr glTFMaterialAttr::ConvertToPbrMaterial()
{
  if (myPbrMat.IsDefined)
  {
    return myPbrMat;
  }
  else if (!myCommonMat.IsDefined)
  {
    return glTFMaterialPbr();
  }

  glTFMaterialPbr aPbrMat;
  aPbrMat.IsDefined = true;
  aPbrMat.BaseColorTexture = myCommonMat.DiffuseTexture;
  aPbrMat.BaseColor.SetRGB (myCommonMat.DiffuseColor);
  aPbrMat.BaseColor.SetAlpha (1.0f - myCommonMat.Transparency);
  aPbrMat.Metallic  = 0;//Graphic3d_PBRMaterial::MetallicFromSpecular (myCommonMat.SpecularColor);
  aPbrMat.Roughness = 0;//Graphic3d_PBRMaterial::RoughnessFromSpecular (myCommonMat.SpecularColor, myCommonMat.Shininess);
  return aPbrMat;
}
