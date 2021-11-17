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
#include <asiAsm_GlTFEntities.h>
#include <asiAsm_GLTFMaterialMapBase.h>

namespace asiAsm {
namespace xde {

//! Material manager for exporting into glTF format.
class glTFMaterialMap : public glTFMaterialMapBase
{
public:

  //! Ctor.
  gltf_EXPORT
    glTFMaterialMap(const TCollection_AsciiString& theFile,
                     const int                      theDefSamplerId);

  //! Dtor.
  gltf_EXPORT virtual
    ~glTFMaterialMap();

public:

  //! Add material images.
  gltf_EXPORT void
    AddImages(glTFJsonSerializer*       theWriter,
              const glTFXdeVisualStyle& theStyle,
              bool&                     theIsStarted);

  //! Add material.
  gltf_EXPORT void
    AddMaterial(glTFJsonSerializer*       theWriter,
                const glTFXdeVisualStyle& theStyle,
                bool&                     theIsStarted);

  //! Add material textures.
  gltf_EXPORT void
    AddTextures(glTFJsonSerializer*       theWriter,
                const glTFXdeVisualStyle& theStyle,
                bool&                     theIsStarted);

public:

  //! Return extent of images map.
  int NbImages() const { return myImageMap.Extent(); }

  //! Return extent of textures map.
  int NbTextures() const { return myTextureMap.Extent(); }

public:

  //! Return base color texture.
  gltf_EXPORT static const Handle(Image_Texture)&
    baseColorTexture(const Handle(glTFMaterialAttr)& theMat);

protected:

  //! Add texture image.
  gltf_EXPORT void
    addImage(glTFJsonSerializer*          theWriter,
             const Handle(Image_Texture)& theTexture,
             bool&                        theIsStarted);

  //! Add texture.
  gltf_EXPORT void
    addTexture(glTFJsonSerializer*          theWriter,
               const Handle(Image_Texture)& theTexture,
               bool&                        theIsStarted);

  //! Add material
  gltf_EXPORT virtual TCollection_AsciiString
    AddMaterial(const glTFXdeVisualStyle& theStyle) Standard_OVERRIDE;

  //! Virtual method actually defining the material (e.g. export to the file).
  gltf_EXPORT virtual void
    DefineMaterial(const glTFXdeVisualStyle&      theStyle,
                   const TCollection_AsciiString& theKey,
                   const TCollection_AsciiString& theName) Standard_OVERRIDE;

protected:

  glTFJsonSerializer* myWriter;
  NCollection_DoubleMap<Handle(Image_Texture), TCollection_AsciiString,
                        Image_Texture, TCollection_AsciiString> myImageMap;

  NCollection_Map<Handle(Image_Texture), Image_Texture> myTextureMap;

  int myDefSamplerId;
  int myNbImages;
};
} // xde
} // asiAsm
