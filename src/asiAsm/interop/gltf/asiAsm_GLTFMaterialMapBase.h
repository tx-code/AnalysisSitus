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
  gltf_EXPORT glTFMaterialMapBase (const TCollection_AsciiString& file);

  //! Destructor.
  gltf_EXPORT virtual ~glTFMaterialMapBase();

public:

  //! Return default material definition to be used for nodes with only color defined.
  const glTFXdeVisualStyle& DefaultStyle() const
  {
    return m_defaultStyle;
  }

  //! Set default material definition to be used for nodes with only color defined.
  void SetDefaultStyle(const glTFXdeVisualStyle& style)
  {
    m_defaultStyle = style;
  }

  //! Find already registered material
  TCollection_AsciiString FindMaterial(const glTFXdeVisualStyle& style) const
  {
    if ( m_styles.IsBound1(style) )
    {
      return m_styles.Find1(style);
    }
    return TCollection_AsciiString();
  }

  //! Register material and return its name identifier.
  gltf_EXPORT virtual TCollection_AsciiString
    AddMaterial (const glTFXdeVisualStyle& style);

  //! Create texture folder "modelName/textures"; for example:
  //! MODEL:  Path/ModelName.gltf
  //! IMAGES: Path/ModelName/textures/
  //! Warning! Output folder is NOT cleared.
  gltf_EXPORT virtual bool
    CreateTextureFolder();

  //! Copy and rename texture file to the new location.
  //! \param resTexture [out] result texture file path (relative to the model)
  //! \param texture [in] original texture
  //! \param key [in] material key
  gltf_EXPORT virtual bool
    CopyTexture(TCollection_AsciiString&       resTexture,
                const Handle(Image_Texture)&   texture,
                const TCollection_AsciiString& key);

  //! Virtual method actually defining the material (e.g. export to the file).
  virtual void DefineMaterial(const glTFXdeVisualStyle&      style,
                              const TCollection_AsciiString& key,
                              const TCollection_AsciiString& name) = 0;

  //! Return failed flag.
  bool IsFailed() const
  {
    return m_bIsFailed;
  }

protected:

  //! Copy file to another place.
  gltf_EXPORT static bool
    copyFileTo(const TCollection_AsciiString& fileSrc,
               const TCollection_AsciiString& fileDst);

protected:

  TCollection_AsciiString m_folder;            //!< output folder for glTF file
  TCollection_AsciiString m_texFolder;         //!< output folder for images (full  path)
  TCollection_AsciiString m_texFolderShort;    //!< output folder for images (short path)
  TCollection_AsciiString m_fileName;          //!< output glTF file path
  TCollection_AsciiString m_shortFileNameBase; //!< output glTF file name without extension
  TCollection_AsciiString m_keyPrefix;         //!< prefix for generated keys
  NCollection_DoubleMap<glTFXdeVisualStyle, TCollection_AsciiString,
                        glTFXdeVisualStyle::Hasher, TCollection_AsciiString>
                          m_styles;            //!< map of processed styles
  NCollection_Map<Handle(Image_Texture), Image_Texture>
                          m_imageFailMap;      //!< map of images failed to be copied
  glTFXdeVisualStyle      m_defaultStyle;      //!< default material definition to be used for nodes with only color defined
  int                     m_nbMaterials;       //!< number of registered materials
  bool                    m_bIsFailed;         //!< flag indicating failure
  bool                    m_matNameAsKey;      //!< flag indicating usage of material name as key

};

} // xde
} // asiAsm
