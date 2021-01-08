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

#ifndef gltf_MaterialMap_HeaderFile
#define gltf_MaterialMap_HeaderFile

// glTF includes
#include <gltf_Entities.h>
#include <gltf_MaterialMapBase.h>

namespace asiAsm
{

//! Material manager for exporting into glTF format.
class gltf_MaterialMap : public gltf_MaterialMapBase
{
public:

  //! Ctor.
  gltf_EXPORT
    gltf_MaterialMap(const TCollection_AsciiString& theFile,
                     const int                      theDefSamplerId);

  //! Dtor.
  gltf_EXPORT virtual
    ~gltf_MaterialMap();

public:

  //! Add material images.
  gltf_EXPORT void
    AddImages(gltf_JsonSerializer*       theWriter,
              const gltf_XdeVisualStyle& theStyle,
              bool&                      theIsStarted);

  //! Add material.
  gltf_EXPORT void
    AddMaterial(gltf_JsonSerializer*       theWriter,
                const gltf_XdeVisualStyle& theStyle,
                bool&                      theIsStarted);

  //! Add material textures.
  gltf_EXPORT void
    AddTextures(gltf_JsonSerializer*       theWriter,
                const gltf_XdeVisualStyle& theStyle,
                bool&                      theIsStarted);

public:

  //! Return extent of images map.
  int NbImages() const { return myImageMap.Extent(); }

  //! Return extent of textures map.
  int NbTextures() const { return myTextureMap.Extent(); }

public:

  //! Return base color texture.
  gltf_EXPORT static const Handle(Image_Texture)&
    baseColorTexture(const Handle(gltf_MaterialAttr)& theMat);

protected:

  //! Add texture image.
  gltf_EXPORT void
    addImage(gltf_JsonSerializer*         theWriter,
             const Handle(Image_Texture)& theTexture,
             bool&                        theIsStarted);

  //! Add texture.
  gltf_EXPORT void
    addTexture(gltf_JsonSerializer*         theWriter,
               const Handle(Image_Texture)& theTexture,
               bool&                        theIsStarted);

  //! Add material
  gltf_EXPORT virtual TCollection_AsciiString
    AddMaterial(const gltf_XdeVisualStyle& theStyle) Standard_OVERRIDE;

  //! Virtual method actually defining the material (e.g. export to the file).
  gltf_EXPORT virtual void
    DefineMaterial(const gltf_XdeVisualStyle&     theStyle,
                   const TCollection_AsciiString& theKey,
                   const TCollection_AsciiString& theName) Standard_OVERRIDE;

protected:

  gltf_JsonSerializer* myWriter;
  NCollection_DoubleMap<Handle(Image_Texture), TCollection_AsciiString,
                        Image_Texture, TCollection_AsciiString> myImageMap;
  NCollection_Map<Handle(Image_Texture), Image_Texture> myTextureMap;
  int myDefSamplerId;
  int myNbImages;

};

}

#endif
