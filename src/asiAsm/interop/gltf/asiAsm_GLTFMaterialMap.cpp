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
#include <asiAsm_GLTFMaterialMap.h>

// GlTF includes
#if defined USE_RAPIDJSON
  #include <asiAsm_GLTFJsonSerializer.h>
#endif

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

const Handle(Image_Texture)&
  glTFMaterialMap::baseColorTexture(const Handle(glTFMaterialAttr)& mat)
{
  static const Handle(Image_Texture) NullTexture;

  if ( mat.IsNull() )
  {
    return NullTexture;
  }

  if ( mat->HasPbrMaterial() && !mat->PbrMaterial().BaseColorTexture.IsNull() )
  {
    return mat->PbrMaterial().BaseColorTexture;
  }

  if ( mat->HasCommonMaterial() && !mat->CommonMaterial().DiffuseTexture.IsNull() )
  {
    return mat->CommonMaterial().DiffuseTexture;
  }

  return NullTexture;
}

//-----------------------------------------------------------------------------

glTFMaterialMap::glTFMaterialMap(const TCollection_AsciiString& filename,
                                 const int                      defSamplerId)
: glTFMaterialMapBase (filename),
  m_pWriter           (nullptr),
  m_iDefSamplerId     (defSamplerId),
  m_iNbImages         (0)
{
  m_matNameAsKey = false;
}

//-----------------------------------------------------------------------------

glTFMaterialMap::~glTFMaterialMap()
{}

//-----------------------------------------------------------------------------

void glTFMaterialMap::AddImages(glTFJsonSerializer*       writer,
                                const glTFXdeVisualStyle& style,
                                bool&                     isStarted)
{
  if ( writer == nullptr || style.GetMaterial().IsNull()|| style.GetMaterial()->IsEmpty() )
  {
    return;
  }

  this->addImage(writer, this->baseColorTexture( style.GetMaterial() ),               isStarted);
  this->addImage(writer, style.GetMaterial()->PbrMaterial().MetallicRoughnessTexture, isStarted);
  this->addImage(writer, style.GetMaterial()->PbrMaterial().NormalTexture,            isStarted);
  this->addImage(writer, style.GetMaterial()->PbrMaterial().EmissiveTexture,          isStarted);
  this->addImage(writer, style.GetMaterial()->PbrMaterial().OcclusionTexture,         isStarted);
}

//-----------------------------------------------------------------------------

void glTFMaterialMap::addImage(glTFJsonSerializer*          writer,
                               const Handle(Image_Texture)& texture,
                               bool&                        isStarted)
{
  if ( texture.IsNull() || m_imageMap.IsBound1(texture) || m_imageFailMap.Contains(texture) )
  {
    return;
  }

  TCollection_AsciiString gltfImgKey = m_iNbImages;
  ++m_iNbImages;
  //
  for ( ; m_imageMap.IsBound2(gltfImgKey); ++m_iNbImages )
  {
    gltfImgKey = m_iNbImages;
  }

  TCollection_AsciiString textureUri;
  if ( !CopyTexture(textureUri, texture, gltfImgKey) )
  {
    m_imageFailMap.Add (texture);
    return;
  }

  m_imageMap.Bind(texture, gltfImgKey);

  if ( !isStarted )
  {
    writer->Key( glTFRootElementName(glTFRootElement_Images) );
    writer->StartArray();
    isStarted = true;
  }

  writer->StartObject();
  {
    writer->Key ("uri");
    writer->String( textureUri.ToCString() );
  }
  writer->EndObject();
}

//-----------------------------------------------------------------------------

void glTFMaterialMap::AddMaterial(glTFJsonSerializer*       writer,
                                  const glTFXdeVisualStyle& style,
                                  bool&                     isStarted)
{
  if ( writer == NULL || ( ( style.GetMaterial().IsNull() || style.GetMaterial()->IsEmpty() )
                       && !( style.IsSetColorSurf() || style.IsSetColorCurve() ) ) )
  {
    return;
  }

  if ( !isStarted )
  {
    writer->Key( glTFRootElementName(glTFRootElement_Materials) );
    writer->StartArray();
    isStarted = true;
  }

  m_pWriter = writer;
  AddMaterial(style);
  m_pWriter = NULL;
}

//-----------------------------------------------------------------------------

void glTFMaterialMap::AddTextures(glTFJsonSerializer*       writer,
                                  const glTFXdeVisualStyle& style,
                                  bool&                     isStarted)
{
  if ( writer == NULL || style.GetMaterial().IsNull() || style.GetMaterial()->IsEmpty() )
  {
    return;
  }

  addTexture(writer, baseColorTexture( style.GetMaterial() ),                     isStarted);
  addTexture(writer, style.GetMaterial()->PbrMaterial().MetallicRoughnessTexture, isStarted);
  addTexture(writer, style.GetMaterial()->PbrMaterial().NormalTexture,            isStarted);
  addTexture(writer, style.GetMaterial()->PbrMaterial().EmissiveTexture,          isStarted);
  addTexture(writer, style.GetMaterial()->PbrMaterial().OcclusionTexture,         isStarted);
}

//-----------------------------------------------------------------------------

void glTFMaterialMap::addTexture(glTFJsonSerializer*          writer,
                                 const Handle(Image_Texture)& texture,
                                 bool&                        isStarted)
{
  if ( texture.IsNull() || m_textureMap.Contains(texture) || !m_imageMap.IsBound1(texture) )
  {
    return;
  }

  const TCollection_AsciiString imgKey = m_imageMap.Find1(texture);
  m_textureMap.Add(texture);
  //
  if ( imgKey.IsEmpty() )
  {
    return;
  }

  if ( !isStarted )
  {
    writer->Key( glTFRootElementName(glTFRootElement_Textures) );
    writer->StartArray();
    isStarted = true;
  }

  writer->StartObject();
  {
    writer->Key( "sampler" );
    writer->Int( m_iDefSamplerId ); // mandatory field by specs
    writer->Key( "source" );
    writer->Int( imgKey.IntegerValue() );
  }
  writer->EndObject();
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  glTFMaterialMap::AddMaterial(const glTFXdeVisualStyle& style)
{
  return glTFMaterialMapBase::AddMaterial(style);
}

//-----------------------------------------------------------------------------

void glTFMaterialMap::DefineMaterial(const glTFXdeVisualStyle&      style,
                                     const TCollection_AsciiString&,
                                     const TCollection_AsciiString& name)
{
  if ( m_pWriter == NULL )
  {
    Standard_ProgramError::Raise ("glTFMaterialMap::DefineMaterial() should be called with JSON Writer");
    return;
  }

  glTFMaterialPbr pbrMat;
  const bool hasMaterial = !style.GetMaterial().IsNull()
                        && !style.GetMaterial()->IsEmpty();
  if ( hasMaterial )
  {
    pbrMat = style.GetMaterial()->ConvertToPbrMaterial();
  }
  else if ( !m_defaultStyle.GetMaterial().IsNull()
          && m_defaultStyle.GetMaterial()->HasPbrMaterial() )
  {
    pbrMat = m_defaultStyle.GetMaterial()->PbrMaterial();
  }
  if ( style.IsSetColorSurf() )
  {
    pbrMat.BaseColor.SetRGB( style.GetColorSurf() );

    if ( style.GetColorSurfRGBA().Alpha() < 1.0f )
    {
      pbrMat.BaseColor.SetAlpha( style.GetColorSurfRGBA().Alpha() );
    }
  }
  else if ( style.IsSetColorCurve() )
  {
    pbrMat.BaseColor.SetRGB( style.GetColorCurve() );
  }

  m_pWriter->StartObject();
  {
    m_pWriter->Key    ( "name" );
    m_pWriter->String ( name.ToCString() );

    m_pWriter->Key ("pbrMetallicRoughness");
    m_pWriter->StartObject();
    {
      m_pWriter->Key ("baseColorFactor");
      m_pWriter->StartArray();
      {
        m_pWriter->Double( pbrMat.BaseColor.GetRGB().Red() );
        m_pWriter->Double( pbrMat.BaseColor.GetRGB().Green() );
        m_pWriter->Double( pbrMat.BaseColor.GetRGB().Blue() );
        m_pWriter->Double( pbrMat.BaseColor.Alpha() );
      }
      m_pWriter->EndArray();

      if ( const Handle(Image_Texture)& baseTexture = baseColorTexture( style.GetMaterial() ) )
      {
        if ( m_imageMap.IsBound1(baseTexture) )
        {
          m_pWriter->Key("baseColorTexture");
          m_pWriter->StartObject();
          {
            m_pWriter->Key("index");
            const TCollection_AsciiString& imageIdx = m_imageMap.Find1(baseTexture);

            if ( !imageIdx.IsEmpty() )
            {
              m_pWriter->Int( imageIdx.IntegerValue() );
            }
          }
          m_pWriter->EndObject();
        }
      }

      if ( hasMaterial || pbrMat.Metallic != 1.0f )
      {
        m_pWriter->Key("metallicFactor");
        m_pWriter->Double(pbrMat.Metallic);
      }

      if ( !pbrMat.MetallicRoughnessTexture.IsNull()
         && m_imageMap.IsBound1(pbrMat.MetallicRoughnessTexture) )
      {
        m_pWriter->Key("metallicRoughnessTexture");
        m_pWriter->StartObject();
        {
          m_pWriter->Key("index");
          const TCollection_AsciiString& imageIdx = m_imageMap.Find1(pbrMat.MetallicRoughnessTexture);

          if ( !imageIdx.IsEmpty() )
          {
            m_pWriter->Int( imageIdx.IntegerValue() );
          }
        }
        m_pWriter->EndObject();
      }

      if ( hasMaterial || pbrMat.Roughness != 1.0f )
      {
        m_pWriter->Key("roughnessFactor");
        m_pWriter->Double(pbrMat.Roughness);
      }
    }
    m_pWriter->EndObject();

    if ( style.GetMaterial().IsNull() || style.GetMaterial()->IsDoubleSided() )
    {
      m_pWriter->Key("doubleSided");
      m_pWriter->Bool(true);
    }

    const Graphic3d_AlphaMode
      alphaMode = !style.GetMaterial().IsNull() ? style.GetMaterial()->AlphaMode() : Graphic3d_AlphaMode_BlendAuto;
    //
    switch ( alphaMode )
    {
      case Graphic3d_AlphaMode_BlendAuto:
      {
        if ( pbrMat.BaseColor.Alpha() < 1.0f )
        {
          m_pWriter->Key("alphaMode");
          m_pWriter->String("BLEND");
        }
        break;
      }
      case Graphic3d_AlphaMode_Opaque:
      {
        break;
      }
      case Graphic3d_AlphaMode_Mask:
      {
        m_pWriter->Key("alphaMode");
        m_pWriter->String("MASK");
        break;
      }
      case Graphic3d_AlphaMode_Blend:
      {
        m_pWriter->Key("alphaMode");
        m_pWriter->String("BLEND");
        break;
      }
    }
    if ( !style.GetMaterial().IsNull() && style.GetMaterial()->AlphaCutOff() != 0.5f )
    {
      m_pWriter->Key("alphaCutoff");
      m_pWriter->Double( style.GetMaterial()->AlphaCutOff() );
    }

    if ( !pbrMat.EmissiveFactor.IsEqual( Graphic3d_Vec3 (0.0f, 0.0f, 0.0f) ) )
    {
      m_pWriter->Key ("emissiveFactor");
      m_pWriter->StartArray();
      {
        m_pWriter->Double( pbrMat.EmissiveFactor.r() );
        m_pWriter->Double( pbrMat.EmissiveFactor.g() );
        m_pWriter->Double( pbrMat.EmissiveFactor.b() );
      }
      m_pWriter->EndArray();
    }
    if ( !pbrMat.EmissiveTexture.IsNull() && m_imageMap.IsBound1(pbrMat.EmissiveTexture) )
    {
      m_pWriter->Key("emissiveTexture");
      m_pWriter->StartObject();
      {
        m_pWriter->Key("index");

        const TCollection_AsciiString& imageIdx = m_imageMap.Find1(pbrMat.EmissiveTexture);

        if ( !imageIdx.IsEmpty() )
        {
          m_pWriter->Int( imageIdx.IntegerValue() );
        }
      }
      m_pWriter->EndObject();
    }

    if ( !pbrMat.NormalTexture.IsNull() && m_imageMap.IsBound1(pbrMat.NormalTexture) )
    {
      m_pWriter->Key("normalTexture");
      m_pWriter->StartObject();
      {
        m_pWriter->Key("index");

        const TCollection_AsciiString& imageIdx = m_imageMap.Find1(pbrMat.NormalTexture);
        //
        if ( !imageIdx.IsEmpty() )
        {
          m_pWriter->Int( imageIdx.IntegerValue() );
        }
      }
      m_pWriter->EndObject();
    }

    if ( !pbrMat.OcclusionTexture.IsNull() && m_imageMap.IsBound1(pbrMat.OcclusionTexture) )
    {
      m_pWriter->Key("occlusionTexture");
      m_pWriter->StartObject();
      {
        m_pWriter->Key("index");

        const TCollection_AsciiString& imageIdx = m_imageMap.Find1(pbrMat.OcclusionTexture);
        //
        if ( !imageIdx.IsEmpty() )
        {
          m_pWriter->Int( imageIdx.IntegerValue() );
        }
      }
      m_pWriter->EndObject();
    }
  }
  m_pWriter->EndObject();
}
