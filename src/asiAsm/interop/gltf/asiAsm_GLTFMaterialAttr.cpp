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
//
: m_alphaMode      (Graphic3d_AlphaMode_BlendAuto),
  m_fAlphaCutOff   (0.5f),
  m_bIsDoubleSided (true)
{}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetPbrMaterial(const glTFMaterialPbr& material)
{
  Backup();
  m_pbrMat = material;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetCommonMaterial(const glTFMaterialCommon& material)
{
  Backup();
  m_commonMat = material;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetAlphaMode(Graphic3d_AlphaMode mode,
                                    float               cutOff)
{
  Backup();
  m_alphaMode    = mode;
  m_fAlphaCutOff = cutOff;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::SetDoubleSided(bool isDoubleSided)
{
  Backup();
  m_bIsDoubleSided = isDoubleSided;
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::Restore(const Handle(TDF_Attribute)& with)
{
  glTFMaterialAttr* pOther = dynamic_cast<glTFMaterialAttr*>( with.get() );
  //
  m_pbrMat         = pOther->m_pbrMat;
  m_commonMat      = pOther->m_commonMat;
  m_alphaMode      = pOther->m_alphaMode;
  m_fAlphaCutOff   = pOther->m_fAlphaCutOff;
  m_bIsDoubleSided = pOther->m_bIsDoubleSided;
}

//-----------------------------------------------------------------------------

Handle(TDF_Attribute) glTFMaterialAttr::NewEmpty() const
{
  return new glTFMaterialAttr();
}

//-----------------------------------------------------------------------------

void glTFMaterialAttr::Paste(const Handle(TDF_Attribute)& into,
                             const Handle(TDF_RelocationTable)&) const
{
  glTFMaterialAttr* pOther = dynamic_cast<glTFMaterialAttr*>( into.get() );
  //
  pOther->Backup();
  pOther->m_pbrMat         = m_pbrMat;
  pOther->m_commonMat      = m_commonMat;
  pOther->m_alphaMode      = m_alphaMode;
  pOther->m_fAlphaCutOff   = m_fAlphaCutOff;
  pOther->m_bIsDoubleSided = m_bIsDoubleSided;
}

//-----------------------------------------------------------------------------

Quantity_ColorRGBA glTFMaterialAttr::BaseColor() const
{
  if ( m_pbrMat.IsDefined )
  {
    return m_pbrMat.BaseColor;
  }
  else if ( m_commonMat.IsDefined )
  {
    return Quantity_ColorRGBA( m_commonMat.DiffuseColor, 1.0f - m_commonMat.Transparency );
  }
  return Quantity_ColorRGBA(Quantity_NOC_WHITE);
}

//-----------------------------------------------------------------------------

glTFMaterialCommon glTFMaterialAttr::ConvertToCommonMaterial()
{
  if ( m_commonMat.IsDefined )
  {
    return m_commonMat;
  }
  else if ( !m_pbrMat.IsDefined )
  {
    return glTFMaterialCommon();
  }

  // convert metal-roughness into common
  glTFMaterialCommon comMat;
  comMat.IsDefined      = true;
  comMat.DiffuseTexture = m_pbrMat.BaseColorTexture;
  comMat.DiffuseColor   = m_pbrMat.BaseColor.GetRGB();
  comMat.SpecularColor  = Quantity_Color( Graphic3d_Vec3(m_pbrMat.Metallic) );
  comMat.Transparency   = 1.0f - m_pbrMat.BaseColor.Alpha();
  comMat.Shininess      = 1.0f - m_pbrMat.Roughness;
  return comMat;
}

//-----------------------------------------------------------------------------

glTFMaterialPbr glTFMaterialAttr::ConvertToPbrMaterial()
{
  if ( m_pbrMat.IsDefined )
  {
    return m_pbrMat;
  }
  else if ( !m_commonMat.IsDefined )
  {
    return glTFMaterialPbr();
  }

  glTFMaterialPbr pbrMat;
  pbrMat.IsDefined = true;
  pbrMat.BaseColorTexture = m_commonMat.DiffuseTexture;
  pbrMat.BaseColor.SetRGB(m_commonMat.DiffuseColor);
  pbrMat.BaseColor.SetAlpha(1.0f - m_commonMat.Transparency);
  pbrMat.Metallic  = 0;
  pbrMat.Roughness = 0;
  return pbrMat;
}
