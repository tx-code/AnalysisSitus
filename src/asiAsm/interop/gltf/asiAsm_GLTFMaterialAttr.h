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
#include <Graphic3d_AlphaMode.hxx>
#include <TDF_Attribute.hxx>

class Graphic3d_Aspects;
class Graphic3d_MaterialAspect;

namespace asiAsm {
namespace xde {

//! Attribute storing Material definition for visualization purposes.
//!
//! Copied from `XCAFDoc_VisMaterial` class of OpenCascade 7.5.0.
class glTFMaterialAttr : public TDF_Attribute
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(glTFMaterialAttr, TDF_Attribute)

public:

  //! \return attribute GUID.
  gltf_EXPORT static const Standard_GUID&
    GetID();

public:

  //! Default ctor.
  gltf_EXPORT
    glTFMaterialAttr();

public:

  //! Return TRUE if material definition is empty.
  bool IsEmpty() const
  {
    return !m_pbrMat.IsDefined && !m_commonMat.IsDefined;
  }

  //! Return TRUE if metal-roughness PBR material is defined.
  bool HasPbrMaterial() const
  {
    return m_pbrMat.IsDefined;
  }

  //! Return metal-roughness PBR material.
  const glTFMaterialPbr& PbrMaterial() const
  {
    return m_pbrMat;
  }

  //! Setup metal-roughness PBR material.
  gltf_EXPORT void
    SetPbrMaterial(const glTFMaterialPbr& material);

  //! Setup undefined metal-roughness PBR material.
  void UnsetPbrMaterial()
  {
    SetPbrMaterial( glTFMaterialPbr() );
  }

  //! Return TRUE if common material is defined.
  bool HasCommonMaterial() const
  {
    return m_commonMat.IsDefined;
  }

  //! Return common material.
  const glTFMaterialCommon& CommonMaterial() const
  {
    return m_commonMat;
  }

  //! Setup common material.
  gltf_EXPORT void
    SetCommonMaterial(const glTFMaterialCommon& material);

  //! Setup undefined common material.
  void UnsetCommonMaterial()
  {
    SetCommonMaterial( glTFMaterialCommon() );
  }

  //! Return base color.
  gltf_EXPORT Quantity_ColorRGBA
    BaseColor() const;

  //! Return alpha mode; Graphic3d_AlphaMode_BlendAuto by default.
  Graphic3d_AlphaMode AlphaMode() const
  {
    return m_alphaMode;
  }

  //! Return alpha cutoff value; 0.5 by default.
  float AlphaCutOff() const
  {
    return m_fAlphaCutOff;
  }

  //! Set alpha mode.
  gltf_EXPORT void
    SetAlphaMode(Graphic3d_AlphaMode mode,
                 float               cutOff = 0.5f);

  //! Specifies whether the material is double sided; TRUE by default.
  bool IsDoubleSided() const
  {
    return m_bIsDoubleSided;
  }

  //! Specifies whether the material is double sided.
  gltf_EXPORT void
    SetDoubleSided(bool isDoubleSided);

  //! Return material name / tag (transient data, not stored in the document).
  const Handle(TCollection_HAsciiString)& RawName() const
  {
    return m_rawName;
  }

  //! Set material name / tag (transient data, not stored in the document).
  void SetRawName(const Handle(TCollection_HAsciiString)& name)
  {
    m_rawName = name;
  }

  //! Compare two materials.
  //! Performs deep comparison by actual values - e.g. can be useful for merging materials.
  bool IsEqual(const Handle(glTFMaterialAttr)& other) const
  {
    if ( other.get() == this )
    {
      return true;
    }
    return other->m_bIsDoubleSided == m_bIsDoubleSided
        && other->m_fAlphaCutOff   == m_fAlphaCutOff
        && other->m_alphaMode      == m_alphaMode
        && other->m_commonMat.IsEqual(m_commonMat)
        && other->m_pbrMat.IsEqual(m_pbrMat);
  }

  //! Return Common material or convert PBR into Common material.
  gltf_EXPORT glTFMaterialCommon ConvertToCommonMaterial();

  //! Return PBR material or convert Common into PBR material.
  gltf_EXPORT glTFMaterialPbr ConvertToPbrMaterial();

public: //! @name interface implementation

  //! Return GUID of this attribute type.
  virtual const Standard_GUID& ID() const Standard_OVERRIDE
  {
    return GetID();
  }

  //! Restore attribute from specified state.
  //! \param[in] with attribute state to restore (copy into this).
  gltf_EXPORT virtual void
    Restore(const Handle(TDF_Attribute)& with) Standard_OVERRIDE;

  //! Create a new empty attribute.
  gltf_EXPORT virtual Handle(TDF_Attribute)
    NewEmpty() const Standard_OVERRIDE;

  //! Paste this attribute into another one.
  //! \param[in,out] into     target attribute to copy this into
  //! \param[in]     relTable relocation table
  gltf_EXPORT virtual void
    Paste(const Handle(TDF_Attribute)&       into,
          const Handle(TDF_RelocationTable)& relTable) const Standard_OVERRIDE;

private:

  Handle(TCollection_HAsciiString) m_rawName;        //!< material name / tag (transient data)
  glTFMaterialPbr                  m_pbrMat;         //!< metal-roughness material definition
  glTFMaterialCommon               m_commonMat;      //!< common material definition
  Graphic3d_AlphaMode              m_alphaMode;      //!< alpha mode; Graphic3d_AlphaMode_BlendAuto by default
  float                            m_fAlphaCutOff;   //!< alpha cutoff value; 0.5 by default
  bool                             m_bIsDoubleSided; //!< specifies whether the material is double sided; TRUE by default

};

} // xde
} // asiAsm
