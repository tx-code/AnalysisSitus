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

#ifndef gltf_XdeVisualStyle_HeaderFile
#define gltf_XdeVisualStyle_HeaderFile

// glTF includes
#include <gltf_MaterialAttr.h>

// OpenCascade includes
#include <XCAFPrs_Style.hxx>

//-----------------------------------------------------------------------------

namespace asiAsm
{

//! Data transfer object for visual styles taken out from XDE document.
class gltf_XdeVisualStyle
{
public:

  //! Default ctor.
  gltf_EXPORT
    gltf_XdeVisualStyle();

  //! Conversion ctor. Accepts the OpenCascade-ish data structure for
  //! XDE styles and converts it to our own thing.
  //! \param[in] the data structure to convert.
  gltf_EXPORT
    gltf_XdeVisualStyle(const XCAFPrs_Style& other);

public:

  //! \return material OCAF attribute.
  gltf_EXPORT const Handle(gltf_MaterialAttr)&
    GetMaterial() const;

  //! Sets material OCAF attribute.
  //! \param[in] material the material attribute to set.
  gltf_EXPORT void
    SetMaterial(const Handle(gltf_MaterialAttr)& material);

  //! \return true if the surface color is set.
  gltf_EXPORT bool
    IsSetColorSurf() const;

  //! \return surface color.
  gltf_EXPORT const Quantity_Color&
    GetColorSurf() const;

  //! Sets surface color.
  //! \param[in] color the color to set.
  gltf_EXPORT void
    SetColorSurf(const Quantity_Color& color);

  //! \return surface color.
  gltf_EXPORT const Quantity_ColorRGBA&
    GetColorSurfRGBA() const;

  //! Sets surface color.
  //! \param[in] color the color to set.
  gltf_EXPORT void
    SetColorSurf(const Quantity_ColorRGBA& color);

  //! Unsets surface color.
  gltf_EXPORT void
    UnSetColorSurf();

  //! \return true if the curve color is set.
  gltf_EXPORT bool
    IsSetColorCurve() const;

  //! \return curve color.
  gltf_EXPORT const Quantity_Color&
    GetColorCurve() const;

  //! Sets curve color.
  //! \param[in] color the color to set.
  gltf_EXPORT void
    SetColorCurve(const Quantity_Color& color);

  //! Unsets curve color.
  gltf_EXPORT void
    UnSetColorCurve();

  //! Sets visibility flag.
  //! \param[in] on true/false.
  gltf_EXPORT void
    SetVisibility(const bool on);

  //! \return visibility flag.
  gltf_EXPORT bool
    IsVisible() const;

public:

  //! Checks if this style is equal to the passed one.
  //! \param[in] other the style to compare with.
  //! \return true in case of equality, false -- otherwise.
  gltf_EXPORT bool
    IsEqual(const gltf_XdeVisualStyle& other) const;

  //! Checks if this style is equal to the passed one.
  //! \param[in] other the style to compare with.
  //! \return true in case of equality, false -- otherwise.
  gltf_EXPORT bool
    operator==(const gltf_XdeVisualStyle& other) const;

public:

  //! Hasher for using style structure in hash maps.
  struct Hasher
  {
    //! Computes a hash code in the range [1, upper] for the given set of styling settings.
    //! \param[in] style the style structure to compute hash code for.
    //! \param[in] upper the upper bound for the hash code being computed.
    //! \return the computed hash code.
    gltf_EXPORT static int
      HashCode(const gltf_XdeVisualStyle& style,
               const int                  upper);

    //! Checks if the passed styles are equal.
    //! \param[in] S1 the first style to check.
    //! \param[in] S2 the second style to check.
    gltf_EXPORT static bool
      IsEqual(const gltf_XdeVisualStyle& S1,
              const gltf_XdeVisualStyle& S2);
  };

protected:

  Handle(gltf_MaterialAttr) m_material;       //!< Material definition.
  Quantity_ColorRGBA        m_colorSurf;      //!< Surface color.
  Quantity_Color            m_colorCurve;     //!< Curve color.
  bool                      m_bHasColorSurf;  //!< Indicates whether surface color is set.
  bool                      m_bHasColorCurve; //!< Indicates whether curve color is set.
  bool                      m_bIsVisible;     //!< Visibility flag.

};

}

#endif
