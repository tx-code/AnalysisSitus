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
#include <gltf_MaterialAttr.h>

// OpenCascade includes
#include <Graphic3d_Aspects.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Standard_GUID.hxx>
#include <TDF_Label.hxx>

//-----------------------------------------------------------------------------

const Standard_GUID& asiAsm::gltf_MaterialAttr::GetID()
{
  static Standard_GUID THE_VIS_MAT_ID ("EBB00255-03A0-4845-BD3B-A70EEDEEFA78");
  return THE_VIS_MAT_ID;
}

//-----------------------------------------------------------------------------

asiAsm::gltf_MaterialAttr::gltf_MaterialAttr()
: myAlphaMode (Graphic3d_AlphaMode_BlendAuto),
  myAlphaCutOff (0.5f),
  myIsDoubleSided (true)
{
  //
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialAttr::SetPbrMaterial(const gltf_MaterialPbr& theMaterial)
{
  Backup();
  myPbrMat = theMaterial;
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialAttr::SetCommonMaterial(const gltf_MaterialCommon& theMaterial)
{
  Backup();
  myCommonMat = theMaterial;
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialAttr::SetAlphaMode(Graphic3d_AlphaMode theMode,
                                             float               theCutOff)
{
  Backup();
  myAlphaMode   = theMode;
  myAlphaCutOff = theCutOff;
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialAttr::SetDoubleSided (bool theIsDoubleSided)
{
  Backup();
  myIsDoubleSided = theIsDoubleSided;
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialAttr::Restore (const Handle(TDF_Attribute)& theWith)
{
  gltf_MaterialAttr* anOther = dynamic_cast<gltf_MaterialAttr* >(theWith.get());
  myPbrMat        = anOther->myPbrMat;
  myCommonMat     = anOther->myCommonMat;
  myAlphaMode     = anOther->myAlphaMode;
  myAlphaCutOff   = anOther->myAlphaCutOff;
  myIsDoubleSided = anOther->myIsDoubleSided;
}

//-----------------------------------------------------------------------------

Handle(TDF_Attribute) asiAsm::gltf_MaterialAttr::NewEmpty() const
{
  return new gltf_MaterialAttr();
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialAttr::Paste(const Handle(TDF_Attribute)& theInto,
                                      const Handle(TDF_RelocationTable)& ) const
{
  gltf_MaterialAttr* anOther = dynamic_cast<gltf_MaterialAttr* >(theInto.get());
  anOther->Backup();
  anOther->myPbrMat        = myPbrMat;
  anOther->myCommonMat     = myCommonMat;
  anOther->myAlphaMode     = myAlphaMode;
  anOther->myAlphaCutOff   = myAlphaCutOff;
  anOther->myIsDoubleSided = myIsDoubleSided;
}

//-----------------------------------------------------------------------------

Quantity_ColorRGBA asiAsm::gltf_MaterialAttr::BaseColor() const
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

asiAsm::gltf_MaterialCommon asiAsm::gltf_MaterialAttr::ConvertToCommonMaterial()
{
  if (myCommonMat.IsDefined)
  {
    return myCommonMat;
  }
  else if (!myPbrMat.IsDefined)
  {
    return gltf_MaterialCommon();
  }

  // convert metal-roughness into common
  gltf_MaterialCommon aComMat;
  aComMat.IsDefined = true;
  aComMat.DiffuseTexture = myPbrMat.BaseColorTexture;
  aComMat.DiffuseColor  = myPbrMat.BaseColor.GetRGB();
  aComMat.SpecularColor = Quantity_Color (Graphic3d_Vec3 (myPbrMat.Metallic));
  aComMat.Transparency = 1.0f - myPbrMat.BaseColor.Alpha();
  aComMat.Shininess    = 1.0f - myPbrMat.Roughness;
  return aComMat;
}

//-----------------------------------------------------------------------------

asiAsm::gltf_MaterialPbr asiAsm::gltf_MaterialAttr::ConvertToPbrMaterial()
{
  if (myPbrMat.IsDefined)
  {
    return myPbrMat;
  }
  else if (!myCommonMat.IsDefined)
  {
    return gltf_MaterialPbr();
  }

  gltf_MaterialPbr aPbrMat;
  aPbrMat.IsDefined = true;
  aPbrMat.BaseColorTexture = myCommonMat.DiffuseTexture;
  aPbrMat.BaseColor.SetRGB (myCommonMat.DiffuseColor);
  aPbrMat.BaseColor.SetAlpha (1.0f - myCommonMat.Transparency);
  aPbrMat.Metallic  = 0;//Graphic3d_PBRMaterial::MetallicFromSpecular (myCommonMat.SpecularColor);
  aPbrMat.Roughness = 0;//Graphic3d_PBRMaterial::RoughnessFromSpecular (myCommonMat.SpecularColor, myCommonMat.Shininess);
  return aPbrMat;
}
