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

#ifndef _fraXCAFDoc_VisMaterial_HeaderFile
#define _fraXCAFDoc_VisMaterial_HeaderFile

// glTF includes
#include <gltf_Entities.h>

// OpenCascade includes
#include <Graphic3d_AlphaMode.hxx>
#include <TDF_Attribute.hxx>

class Graphic3d_Aspects;
class Graphic3d_MaterialAspect;

namespace asiAsm
{

//! Attribute storing Material definition for visualization purposes.
//!
//! Copied from `XCAFDoc_VisMaterial` class of OpenCascade 7.5.0.
class gltf_MaterialAttr : public TDF_Attribute
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(gltf_MaterialAttr, TDF_Attribute)

public:

  //! \return attribute GUID.
  gltf_EXPORT static const Standard_GUID&
    GetID();

public:

  //! Default ctor.
  gltf_EXPORT
    gltf_MaterialAttr();

public:

  //! Return TRUE if material definition is empty.
  bool IsEmpty() const { return !myPbrMat.IsDefined && !myCommonMat.IsDefined; }

  //! Return TRUE if metal-roughness PBR material is defined.
  bool HasPbrMaterial() const { return myPbrMat.IsDefined; }

  //! Return metal-roughness PBR material.
  const gltf_MaterialPbr& PbrMaterial() const { return myPbrMat; }

  //! Setup metal-roughness PBR material.
  gltf_EXPORT void SetPbrMaterial (const gltf_MaterialPbr& theMaterial);

  //! Setup undefined metal-roughness PBR material.
  void UnsetPbrMaterial() { SetPbrMaterial (gltf_MaterialPbr()); }

  //! Return TRUE if common material is defined.
  bool HasCommonMaterial() const { return myCommonMat.IsDefined; }

  //! Return common material.
  const gltf_MaterialCommon& CommonMaterial() const { return myCommonMat; }

  //! Setup common material.
  gltf_EXPORT void SetCommonMaterial (const gltf_MaterialCommon& theMaterial);

  //! Setup undefined common material.
  void UnsetCommonMaterial() { SetCommonMaterial (gltf_MaterialCommon()); }

  //! Return base color.
  gltf_EXPORT Quantity_ColorRGBA BaseColor() const;

  //! Return alpha mode; Graphic3d_AlphaMode_BlendAuto by default.
  Graphic3d_AlphaMode AlphaMode() const { return myAlphaMode; }

  //! Return alpha cutoff value; 0.5 by default.
  float AlphaCutOff() const { return myAlphaCutOff; }

  //! Set alpha mode.
  gltf_EXPORT void SetAlphaMode (Graphic3d_AlphaMode theMode,
                                     float  theCutOff = 0.5f);

  //! Specifies whether the material is double sided; TRUE by default.
  bool IsDoubleSided() const { return myIsDoubleSided; }

  //! Specifies whether the material is double sided.
  gltf_EXPORT void SetDoubleSided (bool theIsDoubleSided);

  //! Return material name / tag (transient data, not stored in the document).
  const Handle(TCollection_HAsciiString)& RawName() const { return myRawName; }

  //! Set material name / tag (transient data, not stored in the document).
  void SetRawName (const Handle(TCollection_HAsciiString)& theName) { myRawName = theName; }

  //! Compare two materials.
  //! Performs deep comparison by actual values - e.g. can be useful for merging materials.
  bool IsEqual(const Handle(gltf_MaterialAttr)& other) const
  {
    if ( other.get() == this )
    {
      return true;
    }
    return other->myIsDoubleSided == myIsDoubleSided
        && other->myAlphaCutOff   == myAlphaCutOff
        && other->myAlphaMode     == myAlphaMode
        && other->myCommonMat.IsEqual(myCommonMat)
        && other->myPbrMat.IsEqual(myPbrMat);
  }

  //! Return Common material or convert PBR into Common material.
  gltf_EXPORT gltf_MaterialCommon ConvertToCommonMaterial();

  //! Return PBR material or convert Common into PBR material.
  gltf_EXPORT gltf_MaterialPbr ConvertToPbrMaterial();

public: //! @name interface implementation

  //! Return GUID of this attribute type.
  virtual const Standard_GUID& ID() const Standard_OVERRIDE { return GetID(); }

  //! Restore attribute from specified state.
  //! \param theWith [in] attribute state to restore (copy into this)
  gltf_EXPORT virtual void Restore (const Handle(TDF_Attribute)& theWith) Standard_OVERRIDE;

  //! Create a new empty attribute.
  gltf_EXPORT virtual Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;

  //! Paste this attribute into another one.
  //! \param theInto [in/out] target attribute to copy this into
  //! \param theRelTable [in] relocation table
  gltf_EXPORT virtual void Paste (const Handle(TDF_Attribute)& theInto,
                                      const Handle(TDF_RelocationTable)& theRelTable) const Standard_OVERRIDE;

private:

  Handle(TCollection_HAsciiString) myRawName;       //!< material name / tag (transient data)
  gltf_MaterialPbr                 myPbrMat;        //!< metal-roughness material definition
  gltf_MaterialCommon              myCommonMat;     //!< common material definition
  Graphic3d_AlphaMode              myAlphaMode;     //!< alpha mode; Graphic3d_AlphaMode_BlendAuto by default
  float                            myAlphaCutOff;   //!< alpha cutoff value; 0.5 by default
  bool                             myIsDoubleSided; //!< specifies whether the material is double sided; TRUE by default

};

}

#endif
