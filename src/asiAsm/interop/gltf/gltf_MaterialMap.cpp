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
#include <gltf_MaterialMap.h>

// glTF includes
#if defined USE_RAPIDJSON
  #include <gltf_JsonSerializer.h>
#endif

//-----------------------------------------------------------------------------

const Handle(Image_Texture)&
  asiAsm::gltf_MaterialMap::baseColorTexture(const Handle(gltf_MaterialAttr)& theMat)
{
  static const Handle(Image_Texture) THE_NULL_TEXTURE;
  if (theMat.IsNull())
  {
    return THE_NULL_TEXTURE;
  }
  else if (theMat->HasPbrMaterial()
       && !theMat->PbrMaterial().BaseColorTexture.IsNull())
  {
    return theMat->PbrMaterial().BaseColorTexture;
  }
  else if (theMat->HasCommonMaterial()
       && !theMat->CommonMaterial().DiffuseTexture.IsNull())
  {
    return theMat->CommonMaterial().DiffuseTexture;
  }
  return THE_NULL_TEXTURE;
}

//-----------------------------------------------------------------------------

asiAsm::gltf_MaterialMap::gltf_MaterialMap(const TCollection_AsciiString& theFile,
                                           const int                      theDefSamplerId)
: gltf_MaterialMapBase (theFile),
  myWriter             (NULL),
  myDefSamplerId       (theDefSamplerId),
  myNbImages           (0)
{
  myMatNameAsKey = false;
}

//-----------------------------------------------------------------------------

asiAsm::gltf_MaterialMap::~gltf_MaterialMap()
{
  //
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialMap::AddImages(gltf_JsonSerializer*       theWriter,
                                         const gltf_XdeVisualStyle& theStyle,
                                         bool&                      theIsStarted)
{
  if (theWriter == NULL
   || theStyle.GetMaterial().IsNull()
   || theStyle.GetMaterial()->IsEmpty())
  {
    return;
  }

  addImage (theWriter, baseColorTexture (theStyle.GetMaterial()), theIsStarted);
  addImage (theWriter, theStyle.GetMaterial()->PbrMaterial().MetallicRoughnessTexture, theIsStarted);
  addImage (theWriter, theStyle.GetMaterial()->PbrMaterial().NormalTexture,    theIsStarted);
  addImage (theWriter, theStyle.GetMaterial()->PbrMaterial().EmissiveTexture,  theIsStarted);
  addImage (theWriter, theStyle.GetMaterial()->PbrMaterial().OcclusionTexture, theIsStarted);
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialMap::addImage(gltf_JsonSerializer*         theWriter,
                                        const Handle(Image_Texture)& theTexture,
                                        bool&                        theIsStarted)
{
#if defined USE_RAPIDJSON
  if (theTexture.IsNull()
   || myImageMap.IsBound1 (theTexture)
   || myImageFailMap.Contains (theTexture))
  {
    return;
  }

  TCollection_AsciiString aGltfImgKey = myNbImages;
  ++myNbImages;
  for (; myImageMap.IsBound2 (aGltfImgKey); ++myNbImages)
  {
    aGltfImgKey = myNbImages;
  }

  TCollection_AsciiString aTextureUri;
  if (!CopyTexture (aTextureUri, theTexture, aGltfImgKey))
  {
    myImageFailMap.Add (theTexture);
    return;
  }

  myImageMap.Bind (theTexture, aGltfImgKey);

  if (!theIsStarted)
  {
    theWriter->Key (gltf_RootElementName (gltf_RootElement_Images));
    theWriter->StartArray();
    theIsStarted = true;
  }

  theWriter->StartObject();
  {
    theWriter->Key ("uri");
    theWriter->String (aTextureUri.ToCString());
  }
  theWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) theWriter;
  (void) theTexture;
  (void) theIsStarted;
#endif
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialMap::AddMaterial(gltf_JsonSerializer*       theWriter,
                                           const gltf_XdeVisualStyle& theStyle,
                                           bool&                      theIsStarted)
{
#if defined USE_RAPIDJSON
  if (theWriter == NULL
   || ((theStyle.GetMaterial().IsNull() || theStyle.GetMaterial()->IsEmpty())
    && !theStyle.IsSetColorSurf()))
  {
    return;
  }

  if (!theIsStarted)
  {
    theWriter->Key (gltf_RootElementName (gltf_RootElement_Materials));
    theWriter->StartArray();
    theIsStarted = true;
  }
  myWriter = theWriter;
  AddMaterial (theStyle);
  myWriter = NULL;
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) theWriter;
  (void) theStyle;
  (void) theIsStarted;
#endif
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialMap::AddTextures(gltf_JsonSerializer*       theWriter,
                                           const gltf_XdeVisualStyle& theStyle,
                                           bool&                      theIsStarted)
{
#if defined USE_RAPIDJSON
  if (theWriter == NULL
   || theStyle.GetMaterial().IsNull()
   || theStyle.GetMaterial()->IsEmpty())
  {
    return;
  }

  addTexture (theWriter, baseColorTexture (theStyle.GetMaterial()), theIsStarted);
  addTexture (theWriter, theStyle.GetMaterial()->PbrMaterial().MetallicRoughnessTexture, theIsStarted);
  addTexture (theWriter, theStyle.GetMaterial()->PbrMaterial().NormalTexture,    theIsStarted);
  addTexture (theWriter, theStyle.GetMaterial()->PbrMaterial().EmissiveTexture,  theIsStarted);
  addTexture (theWriter, theStyle.GetMaterial()->PbrMaterial().OcclusionTexture, theIsStarted);
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) theWriter;
  (void) theStyle;
  (void) theIsStarted;
#endif
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialMap::addTexture(gltf_JsonSerializer*         theWriter,
                                          const Handle(Image_Texture)& theTexture,
                                          bool&                        theIsStarted)
{
#if defined USE_RAPIDJSON
  if (theTexture.IsNull()
  ||  myTextureMap.Contains (theTexture)
  || !myImageMap  .IsBound1 (theTexture))
  {
    return;
  }

  const TCollection_AsciiString anImgKey = myImageMap.Find1 (theTexture);
  myTextureMap.Add (theTexture);
  if (anImgKey.IsEmpty())
  {
    return;
  }

  if (!theIsStarted)
  {
    theWriter->Key (gltf_RootElementName (gltf_RootElement_Textures));
    theWriter->StartArray();
    theIsStarted = true;
  }

  theWriter->StartObject();
  {
    theWriter->Key ("sampler");
    theWriter->Int (myDefSamplerId); // mandatory field by specs
    theWriter->Key ("source");
    theWriter->Int (anImgKey.IntegerValue());
  }
  theWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) theWriter;
  (void) theTexture;
  (void) theIsStarted;
#endif
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAsm::gltf_MaterialMap::AddMaterial(const gltf_XdeVisualStyle& theStyle)
{
  return gltf_MaterialMapBase::AddMaterial (theStyle);
}

//-----------------------------------------------------------------------------

void asiAsm::gltf_MaterialMap::DefineMaterial(const gltf_XdeVisualStyle& theStyle,
                                              const TCollection_AsciiString& /*theKey*/,
                                              const TCollection_AsciiString& theName)
{
#if defined USE_RAPIDJSON
  if (myWriter == NULL)
  {
    Standard_ProgramError::Raise ("gltf_MaterialMap::DefineMaterial() should be called with JSON Writer");
    return;
  }

  gltf_MaterialPbr aPbrMat;
  const bool hasMaterial = !theStyle.GetMaterial().IsNull()
                        && !theStyle.GetMaterial()->IsEmpty();
  if (hasMaterial)
  {
    aPbrMat = theStyle.GetMaterial()->ConvertToPbrMaterial();
  }
  else if (!myDefaultStyle.GetMaterial().IsNull()
         && myDefaultStyle.GetMaterial()->HasPbrMaterial())
  {
    aPbrMat = myDefaultStyle.GetMaterial()->PbrMaterial();
  }
  if (theStyle.IsSetColorSurf())
  {
    aPbrMat.BaseColor.SetRGB (theStyle.GetColorSurf());
    if (theStyle.GetColorSurfRGBA().Alpha() < 1.0f)
    {
      aPbrMat.BaseColor.SetAlpha (theStyle.GetColorSurfRGBA().Alpha());
    }
  }
  myWriter->StartObject();
  {
    myWriter->Key ("name");
    myWriter->String (theName.ToCString());

    myWriter->Key ("pbrMetallicRoughness");
    myWriter->StartObject();
    {
      myWriter->Key ("baseColorFactor");
      myWriter->StartArray();
      {
        myWriter->Double (aPbrMat.BaseColor.GetRGB().Red());
        myWriter->Double (aPbrMat.BaseColor.GetRGB().Green());
        myWriter->Double (aPbrMat.BaseColor.GetRGB().Blue());
        myWriter->Double (aPbrMat.BaseColor.Alpha());
      }
      myWriter->EndArray();

      if (const Handle(Image_Texture)& aBaseTexture = baseColorTexture (theStyle.GetMaterial()))
      {
        if (myImageMap.IsBound1 (aBaseTexture))
        {
          myWriter->Key ("baseColorTexture");
          myWriter->StartObject();
          {
            myWriter->Key ("index");
            const TCollection_AsciiString& anImageIdx = myImageMap.Find1 (aBaseTexture);
            if (!anImageIdx.IsEmpty())
            {
              myWriter->Int (anImageIdx.IntegerValue());
            }
          }
          myWriter->EndObject();
        }
      }

      if (hasMaterial
       || aPbrMat.Metallic != 1.0f)
      {
        myWriter->Key ("metallicFactor");
        myWriter->Double (aPbrMat.Metallic);
      }

      if (!aPbrMat.MetallicRoughnessTexture.IsNull()
        && myImageMap.IsBound1 (aPbrMat.MetallicRoughnessTexture))
      {
        myWriter->Key ("metallicRoughnessTexture");
        myWriter->StartObject();
        {
          myWriter->Key ("index");
          const TCollection_AsciiString& anImageIdx = myImageMap.Find1 (aPbrMat.MetallicRoughnessTexture);
          if (!anImageIdx.IsEmpty())
          {
            myWriter->Int (anImageIdx.IntegerValue());
          }
        }
        myWriter->EndObject();
      }

      if (hasMaterial
       || aPbrMat.Roughness != 1.0f)
      {
        myWriter->Key ("roughnessFactor");
        myWriter->Double (aPbrMat.Roughness);
      }
    }
    myWriter->EndObject();

    if (theStyle.GetMaterial().IsNull()
     || theStyle.GetMaterial()->IsDoubleSided())
    {
      myWriter->Key ("doubleSided");
      myWriter->Bool (true);
    }

    const Graphic3d_AlphaMode anAlphaMode = !theStyle.GetMaterial().IsNull() ? theStyle.GetMaterial()->AlphaMode() : Graphic3d_AlphaMode_BlendAuto;
    switch (anAlphaMode)
    {
      case Graphic3d_AlphaMode_BlendAuto:
      {
        if (aPbrMat.BaseColor.Alpha() < 1.0f)
        {
          myWriter->Key ("alphaMode");
          myWriter->String ("BLEND");
        }
        break;
      }
      case Graphic3d_AlphaMode_Opaque:
      {
        break;
      }
      case Graphic3d_AlphaMode_Mask:
      {
        myWriter->Key ("alphaMode");
        myWriter->String ("MASK");
        break;
      }
      case Graphic3d_AlphaMode_Blend:
      {
        myWriter->Key ("alphaMode");
        myWriter->String ("BLEND");
        break;
      }
    }
    if (!theStyle.GetMaterial().IsNull()
      && theStyle.GetMaterial()->AlphaCutOff() != 0.5f)
    {
      myWriter->Key ("alphaCutoff");
      myWriter->Double (theStyle.GetMaterial()->AlphaCutOff());
    }

    if ( !aPbrMat.EmissiveFactor.IsEqual( Graphic3d_Vec3 (0.0f, 0.0f, 0.0f) ) )
    {
      myWriter->Key ("emissiveFactor");
      myWriter->StartArray();
      {
        myWriter->Double (aPbrMat.EmissiveFactor.r());
        myWriter->Double (aPbrMat.EmissiveFactor.g());
        myWriter->Double (aPbrMat.EmissiveFactor.b());
      }
      myWriter->EndArray();
    }
    if (!aPbrMat.EmissiveTexture.IsNull()
      && myImageMap.IsBound1 (aPbrMat.EmissiveTexture))
    {
      myWriter->Key ("emissiveTexture");
      myWriter->StartObject();
      {
        myWriter->Key ("index");
        const TCollection_AsciiString& anImageIdx = myImageMap.Find1 (aPbrMat.EmissiveTexture);
        if (!anImageIdx.IsEmpty())
        {
          myWriter->Int (anImageIdx.IntegerValue());
        }
      }
      myWriter->EndObject();
    }

    if (!aPbrMat.NormalTexture.IsNull()
      && myImageMap.IsBound1 (aPbrMat.NormalTexture))
    {
      myWriter->Key ("normalTexture");
      myWriter->StartObject();
      {
        myWriter->Key ("index");
        const TCollection_AsciiString& anImageIdx = myImageMap.Find1 (aPbrMat.NormalTexture);
        if (!anImageIdx.IsEmpty())
        {
          myWriter->Int (anImageIdx.IntegerValue());
        }
      }
      myWriter->EndObject();
    }

    if (!aPbrMat.OcclusionTexture.IsNull()
      && myImageMap.IsBound1 (aPbrMat.OcclusionTexture))
    {
      myWriter->Key ("occlusionTexture");
      myWriter->StartObject();
      {
        myWriter->Key ("index");
        const TCollection_AsciiString& anImageIdx = myImageMap.Find1 (aPbrMat.OcclusionTexture);
        if (!anImageIdx.IsEmpty())
        {
          myWriter->Int (anImageIdx.IntegerValue());
        }
      }
      myWriter->EndObject();
    }
  }
  myWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) theStyle;
  (void) theName;
#endif
}
