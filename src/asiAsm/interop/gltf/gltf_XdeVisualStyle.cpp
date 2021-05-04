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
#include <gltf_XdeVisualStyle.h>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

gltf_XdeVisualStyle::gltf_XdeVisualStyle()
//
: m_bHasColorSurf  (false),
  m_bHasColorCurve (false),
  m_bIsVisible     (true)
{}

//-----------------------------------------------------------------------------

gltf_XdeVisualStyle::gltf_XdeVisualStyle(const XCAFPrs_Style& other)
{
  m_colorSurf.SetRGB( other.GetColorSurf() );
  //
  m_colorCurve     = other.GetColorCurv();
  m_bHasColorSurf  = other.IsSetColorSurf();
  m_bHasColorCurve = other.IsSetColorCurv();
  m_bIsVisible     = other.IsVisible();
}

//-----------------------------------------------------------------------------

const Handle(gltf_MaterialAttr)&
  gltf_XdeVisualStyle::GetMaterial() const
{
  return m_material;
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::SetMaterial(const Handle(gltf_MaterialAttr)& material)
{
  m_material = material;
}

//-----------------------------------------------------------------------------

bool gltf_XdeVisualStyle::IsSetColorSurf() const
{
  return m_bHasColorSurf;
}

//-----------------------------------------------------------------------------

const Quantity_Color& gltf_XdeVisualStyle::GetColorSurf() const
{
  return m_colorSurf.GetRGB();
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::SetColorSurf(const Quantity_Color& color)
{
  this->SetColorSurf( Quantity_ColorRGBA(color) );
}

//-----------------------------------------------------------------------------

const Quantity_ColorRGBA& gltf_XdeVisualStyle::GetColorSurfRGBA() const
{
  return m_colorSurf;
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::SetColorSurf(const Quantity_ColorRGBA& color)
{
  m_colorSurf     = color;
  m_bHasColorSurf = true;
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::UnSetColorSurf()
{
  m_bHasColorSurf = false;
  m_colorSurf.ChangeRGB().SetValues(Quantity_NOC_YELLOW);
  m_colorSurf.SetAlpha(1.0f);
}

//-----------------------------------------------------------------------------

bool gltf_XdeVisualStyle::IsSetColorCurve() const
{
  return m_bHasColorCurve;
}

//-----------------------------------------------------------------------------

const Quantity_Color& gltf_XdeVisualStyle::GetColorCurve() const
{
  return m_colorCurve;
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::SetColorCurve(const Quantity_Color& color)
{
  m_colorCurve     = color;
  m_bHasColorCurve = true;
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::UnSetColorCurve()
{
  m_bHasColorCurve = false;
  m_colorCurve.SetValues(Quantity_NOC_YELLOW);
}

//-----------------------------------------------------------------------------

void gltf_XdeVisualStyle::SetVisibility(const bool on)
{
  m_bIsVisible = on;
}

//-----------------------------------------------------------------------------

bool gltf_XdeVisualStyle::IsVisible() const
{
  return m_bIsVisible;
}

//-----------------------------------------------------------------------------

bool gltf_XdeVisualStyle::IsEqual(const gltf_XdeVisualStyle& other) const
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

bool gltf_XdeVisualStyle::operator==(const gltf_XdeVisualStyle& other) const
{
  return this->IsEqual(other);
}

//-----------------------------------------------------------------------------

int gltf_XdeVisualStyle::Hasher::HashCode(const gltf_XdeVisualStyle& style,
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

bool gltf_XdeVisualStyle::Hasher::IsEqual(const gltf_XdeVisualStyle& S1,
                                          const gltf_XdeVisualStyle& S2)
{
  return S1.IsEqual(S2);
}
