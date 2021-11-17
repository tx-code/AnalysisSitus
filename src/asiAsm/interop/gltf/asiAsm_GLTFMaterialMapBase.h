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
#include <asiAsm_GLTFXdeVisualStyle.h>

// OpenCascade includes
#include <NCollection_DoubleMap.hxx>
#include <NCollection_Map.hxx>

namespace asiAsm {
namespace xde {

//! Material manager.
//! Provides an interface for collecting all materials within the document before writing it into file,
//! and for copying associated image files (textures) into sub-folder near by exported model.
class glTFMaterialMapBase
{
public:

  //! Main constructor.
  gltf_EXPORT glTFMaterialMapBase (const TCollection_AsciiString& theFile);

  //! Destructor.
  gltf_EXPORT virtual ~glTFMaterialMapBase();

  //! Return default material definition to be used for nodes with only color defined.
  const glTFXdeVisualStyle& DefaultStyle() const { return myDefaultStyle; }

  //! Set default material definition to be used for nodes with only color defined.
  void SetDefaultStyle (const glTFXdeVisualStyle& theStyle) { myDefaultStyle = theStyle; }

  //! Find already registered material
  TCollection_AsciiString FindMaterial (const glTFXdeVisualStyle& theStyle) const
  {
    if (myStyles.IsBound1 (theStyle))
    {
      return myStyles.Find1 (theStyle);
    }
    return TCollection_AsciiString();
  }

  //! Register material and return its name identifier.
  gltf_EXPORT virtual TCollection_AsciiString AddMaterial (const glTFXdeVisualStyle& theStyle);

  //! Create texture folder "modelName/textures"; for example:
  //! MODEL:  Path/ModelName.gltf
  //! IMAGES: Path/ModelName/textures/
  //! Warning! Output folder is NOT cleared.
  gltf_EXPORT virtual bool CreateTextureFolder();

  //! Copy and rename texture file to the new location.
  //! \param theResTexture [out] result texture file path (relative to the model)
  //! \param theTexture [in] original texture
  //! \param theKey [in] material key
  gltf_EXPORT virtual bool CopyTexture(TCollection_AsciiString& theResTexture,
                                       const Handle(Image_Texture)& theTexture,
                                       const TCollection_AsciiString& theKey);

  //! Virtual method actually defining the material (e.g. export to the file).
  virtual void DefineMaterial (const glTFXdeVisualStyle& theStyle,
                               const TCollection_AsciiString& theKey,
                               const TCollection_AsciiString& theName) = 0;

  //! Return failed flag.
  bool IsFailed() const { return myIsFailed; }

protected:

  //! Copy file to another place.
  gltf_EXPORT static bool copyFileTo(const TCollection_AsciiString& theFileSrc,
                                     const TCollection_AsciiString& theFileDst);

protected:

  TCollection_AsciiString myFolder;            //!< output folder for glTF file
  TCollection_AsciiString myTexFolder;         //!< output folder for images (full  path)
  TCollection_AsciiString myTexFolderShort;    //!< output folder for images (short path)
  TCollection_AsciiString myFileName;          //!< output glTF file path
  TCollection_AsciiString myShortFileNameBase; //!< output glTF file name without extension
  TCollection_AsciiString myKeyPrefix;         //!< prefix for generated keys
  NCollection_DoubleMap<glTFXdeVisualStyle, TCollection_AsciiString,
                        glTFXdeVisualStyle::Hasher, TCollection_AsciiString>
                          myStyles;            //!< map of processed styles
  NCollection_Map<Handle(Image_Texture), Image_Texture>
                          myImageFailMap;      //!< map of images failed to be copied
  glTFXdeVisualStyle      myDefaultStyle;      //!< default material definition to be used for nodes with only color defined
  int                     myNbMaterials;       //!< number of registered materials
  bool                    myIsFailed;          //!< flag indicating failure
  bool                    myMatNameAsKey;      //!< flag indicating usage of material name as key

};

} // xde
} // asiAsm
