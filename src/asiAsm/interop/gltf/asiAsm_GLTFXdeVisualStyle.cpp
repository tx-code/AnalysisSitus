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
#include <asiAsm_GLTFXdeVisualStyle.h>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

glTFXdeVisualStyle::glTFXdeVisualStyle()
//
: m_bHasColorSurf  (false),
  m_bHasColorCurve (false),
  m_bIsVisible     (true)
{}

//-----------------------------------------------------------------------------

glTFXdeVisualStyle::glTFXdeVisualStyle(const XCAFPrs_Style& other)
{
  m_colorSurf.SetRGB( other.GetColorSurf() );
  //
  m_colorCurve     = other.GetColorCurv();
  m_bHasColorSurf  = other.IsSetColorSurf();
  m_bHasColorCurve = other.IsSetColorCurv();
  m_bIsVisible     = other.IsVisible();
}

//-----------------------------------------------------------------------------

const Handle(glTFMaterialAttr)&
  glTFXdeVisualStyle::GetMaterial() const
{
  return m_material;
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::SetMaterial(const Handle(glTFMaterialAttr)& material)
{
  m_material = material;
}

//-----------------------------------------------------------------------------

bool glTFXdeVisualStyle::IsSetColorSurf() const
{
  return m_bHasColorSurf;
}

//-----------------------------------------------------------------------------

const Quantity_Color& glTFXdeVisualStyle::GetColorSurf() const
{
  return m_colorSurf.GetRGB();
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::SetColorSurf(const Quantity_Color& color)
{
  this->SetColorSurf( Quantity_ColorRGBA(color) );
}

//-----------------------------------------------------------------------------

const Quantity_ColorRGBA& glTFXdeVisualStyle::GetColorSurfRGBA() const
{
  return m_colorSurf;
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::SetColorSurf(const Quantity_ColorRGBA& color)
{
  m_colorSurf     = color;
  m_bHasColorSurf = true;
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::UnSetColorSurf()
{
  m_bHasColorSurf = false;
  m_colorSurf.ChangeRGB().SetValues(Quantity_NOC_YELLOW);
  m_colorSurf.SetAlpha(1.0f);
}

//-----------------------------------------------------------------------------

bool glTFXdeVisualStyle::IsSetColorCurve() const
{
  return m_bHasColorCurve;
}

//-----------------------------------------------------------------------------

const Quantity_Color& glTFXdeVisualStyle::GetColorCurve() const
{
  return m_colorCurve;
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::SetColorCurve(const Quantity_Color& color)
{
  m_colorCurve     = color;
  m_bHasColorCurve = true;
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::UnSetColorCurve()
{
  m_bHasColorCurve = false;
  m_colorCurve.SetValues(Quantity_NOC_YELLOW);
}

//-----------------------------------------------------------------------------

void glTFXdeVisualStyle::SetVisibility(const bool on)
{
  m_bIsVisible = on;
}

//-----------------------------------------------------------------------------

bool glTFXdeVisualStyle::IsVisible() const
{
  return m_bIsVisible;
}

//-----------------------------------------------------------------------------

bool glTFXdeVisualStyle::IsEqual(const glTFXdeVisualStyle& other) const
{
  if ( m_bIsVisible != other.m_bIsVisible )
  {
    return false;
  }
  else if ( !m_bIsVisible )
  {
    return true;
  }

  return m_bHasColorSurf  == other.m_bHasColorSurf
      && m_bHasColorCurve == other.m_bHasColorCurve
      && m_material       == other.m_material
      && (!m_bHasColorSurf  || m_colorSurf  == other.m_colorSurf)
      && (!m_bHasColorCurve || m_colorCurve == other.m_colorCurve);
}

//-----------------------------------------------------------------------------

bool glTFXdeVisualStyle::operator==(const glTFXdeVisualStyle& other) const
{
  return this->IsEqual(other);
}

//-----------------------------------------------------------------------------

int glTFXdeVisualStyle::Hasher::HashCode(const glTFXdeVisualStyle& style,
                                          const int                  upper)
{
  if ( !style.m_bIsVisible )
    return 1; // ??? Taken from OpenCascade like this.

  int hashCode = 0;
  if ( style.m_bHasColorSurf )
  {
    hashCode = hashCode ^ Quantity_ColorRGBAHasher::HashCode(style.m_colorSurf, upper);
  }
  if ( style.m_bHasColorCurve )
  {
    hashCode = hashCode ^ Quantity_ColorHasher::HashCode(style.m_colorCurve, upper);
  }
  if ( !style.m_material.IsNull() )
  {
    hashCode = hashCode ^ ::HashCode(style.m_material, upper);
  }
  return ::HashCode(hashCode, upper);
}

//-----------------------------------------------------------------------------

bool glTFXdeVisualStyle::Hasher::IsEqual(const glTFXdeVisualStyle& S1,
                                          const glTFXdeVisualStyle& S2)
{
  return S1.IsEqual(S2);
}
