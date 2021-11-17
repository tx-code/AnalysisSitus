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

// asiAsm includes
#include <asiAsm.h>

// OpenCascade includes
#include <Graphic3d_BndBox3d.hxx>
#include <Graphic3d_Vec3.hxx>
#include <Image_Texture.hxx>
#include <XCAFPrs_DocumentNode.hxx>

//-----------------------------------------------------------------------------

#define gltf_VendorName "Analysis Situs"

//-----------------------------------------------------------------------------

#define gltf_EXPORT asiAsm_EXPORT

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! Root elements within glTF JSON document.
enum glTFRootElement
{
  /* Mandatory elements */
  glTFRootElement_Asset,       //!< "asset"       element, mandatory.
  glTFRootElement_Scenes,      //!< "scenes"      element, mandatory.
  glTFRootElement_Scene,       //!< "scene"       element, mandatory.
  glTFRootElement_Nodes,       //!< "nodes"       element, mandatory.
  glTFRootElement_Meshes,      //!< "meshes"      element, mandatory.
  glTFRootElement_Accessors,   //!< "accessors"   element, mandatory.
  glTFRootElement_BufferViews, //!< "bufferViews" element, mandatory.
  glTFRootElement_Buffers,     //!< "buffers"     element, mandatory.
  NB_MANDATORY,                //!< number of mandatory elements.

  /* Optional elements */
  glTFRootElement_Animations = NB_MANDATORY, //!< "animations" element.
  glTFRootElement_Materials,                 //!< "materials"  element.
  glTFRootElement_Programs,                  //!< "programs"   element.
  glTFRootElement_Samplers,                  //!< "samplers"   element.
  glTFRootElement_Shaders,                   //!< "shaders"    element.
  glTFRootElement_Skins,                     //!< "skins"      element.
  glTFRootElement_Techniques,                //!< "techniques" element.
  glTFRootElement_Textures,                  //!< "textures"   element.
  glTFRootElement_Images,                    //!< "images"     element.
  glTFRootElement_ExtensionsUsed,            //!< "extensionsUsed"     element.
  glTFRootElement_ExtensionsRequired,        //!< "extensionsRequired" element.

  NB //!< overall number of elements.
};

inline const char* glTFRootElementName(glTFRootElement elem)
{
  static const char* ROOT_NAMES[glTFRootElement::NB] =
  {
    "asset",
    "scenes",
    "scene",
    "nodes",
    "meshes",
    "accessors",
    "bufferViews",
    "buffers",
    "animations",
    "materials",
    "programs",
    "samplers",
    "shaders",
    "skins",
    "techniques",
    "textures",
    "images",
    "extensionsUsed",
    "extensionsRequired"
  };
  return ROOT_NAMES[elem];
}

//-----------------------------------------------------------------------------

//! Accessors store lists of numeric, vector, or matrix elements in a typed array.
//! All large data for Mesh, Skin, and Animation properties is stored in Accessors,
//! organized into one or more Buffers. Each accessor provides data in typed arrays,
//! with two abstractions:
//!
//! 1. Elements are the logical divisions of the data into useful types: "SCALAR",
//!    "VEC2", "VEC3", "VEC4", "MAT3", or "MAT4".
//! 2. Components are the numeric values within an element, e.g. x and y for "VEC2".
//!    Various component types are available: BYTE, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT,
//!    UNSIGNED_INT, and FLOAT.
enum glTFAccessorComponentType
{
  glTFAccessorComponentType_UNKNOWN,
  glTFAccessorComponentType_Int8    = 5120, //!< GL_BYTE
  glTFAccessorComponentType_UInt8   = 5121, //!< GL_UNSIGNED_BYTE
  glTFAccessorComponentType_Int16   = 5122, //!< GL_SHORT
  glTFAccessorComponentType_UInt16  = 5123, //!< GL_UNSIGNED_SHORT
  glTFAccessorComponentType_UInt32  = 5125, //!< GL_UNSIGNED_INT
  glTFAccessorComponentType_Float32 = 5126  //!< GL_FLOAT
};

//-----------------------------------------------------------------------------

//! Low-level glTF enumeration defining Accessor layout.
//! Similar to Graphic3d_TypeOfData but does not define actual type and includes matrices.
enum glTFAccessorLayout
{
  glTFAccessorLayout_UNKNOWN, //!< unknown or invalid type
  glTFAccessorLayout_Scalar,  //!< "SCALAR"
  glTFAccessorLayout_Vec2,    //!< "VEC2"
  glTFAccessorLayout_Vec3,    //!< "VEC3"
  glTFAccessorLayout_Vec4,    //!< "VEC4"
  glTFAccessorLayout_Mat2,    //!< "MAT2"
  glTFAccessorLayout_Mat3,    //!< "MAT3"
  glTFAccessorLayout_Mat4     //!< "MAT4"
};

//-----------------------------------------------------------------------------

//! Low-level glTF data structure defining Accessor.
struct glTFAccessor
{
  static const int INVALID_ID = -1;

public:

  int                       Id;            //!< identifier
  int64_t                   ByteOffset;    //!< byte offset
  int64_t                   Count;         //!< size
  int32_t                   ByteStride;    //!< [0, 255] for glTF 1.0
  glTFAccessorLayout        Type;          //!< layout type
  glTFAccessorComponentType ComponentType; //!< component type
  Graphic3d_BndBox3d        BndBox;        //!< bounding box

  //! Default ctor.
  glTFAccessor()
  : Id            (INVALID_ID),
    ByteOffset    (0),
    Count         (0),
    ByteStride    (0),
    Type          (glTFAccessorLayout_UNKNOWN),
    ComponentType (glTFAccessorComponentType_UNKNOWN) {}
};

//-----------------------------------------------------------------------------

//! Low-level glTF enumeration defining Primitive type.
//! Similar to Graphic3d_TypeOfData but does not define actual type and includes matrices.
enum class glTFPrimitiveMode
{
  glTFPrimitiveMode_UNKNOWN       = -1, //!< unknown or invalid type
  glTFPrimitiveMode_Points        =  0, //!< GL_POINTS
  glTFPrimitiveMode_Lines         =  1, //!< GL_LINES
  glTFPrimitiveMode_LineLoop      =  2, //!< GL_LINE_LOOP
  glTFPrimitiveMode_LineStrip     =  3, //!< GL_LINE_STRIP
  glTFPrimitiveMode_Triangles     =  4, //!< GL_TRIANGLES
  glTFPrimitiveMode_TriangleStrip =  5, //!< GL_TRIANGLE_STRIP
  glTFPrimitiveMode_TriangleFan   =  6  //!< GL_TRIANGLE_FAN
};

//-----------------------------------------------------------------------------

//! Standard coordinate system definition.
//! Open CASCADE does not force application using specific coordinate system,
//! although Draw Harness and samples define +Z-up +Y-forward coordinate system for camera view manipulation.
//! This enumeration defines two commonly used conventions: Z-up and Y-up.
enum glTFCoordinateSystem
{
  glTFCoordinateSystem_Undefined      = -1, //!< undefined
  glTFCoordinateSystem_posYfwd_posZup =  0, //!< +YForward+Zup+Xright
  glTFCoordinateSystem_negZfwd_posYup,      //!< -ZForward+Yup+Xright

  glTFCoordinateSystem_Blender = glTFCoordinateSystem_posYfwd_posZup, //!< coordinate system used by Blender (+YForward+Zup+Xright)
  glTFCoordinateSystem_glTF    = glTFCoordinateSystem_negZfwd_posYup, //!< coordinate system used by glTF    (-ZForward+Yup+Xright)
  glTFCoordinateSystem_Zup     = glTFCoordinateSystem_Blender,        //!< Z-up coordinate system (+YForward+Zup+Xright)
  glTFCoordinateSystem_Yup     = glTFCoordinateSystem_glTF            //!< Y-up coordinate system (-ZForward+Yup+Xright)
};

//-----------------------------------------------------------------------------

//! Low-level glTF enumeration defining BufferView target.
enum glTFBufferViewTarget
{
  glTFBufferViewTarget_UNKNOWN,                      //!< unknown or invalid type
  glTFBufferViewTarget_ARRAY_BUFFER         = 34962, //!< GL_ARRAY_BUFFER
  glTFBufferViewTarget_ELEMENT_ARRAY_BUFFER = 34963  //!< GL_ELEMENT_ARRAY_BUFFER
};

//-----------------------------------------------------------------------------

//! Low-level glTF data structure defining BufferView.
struct glTFBufferView
{
  static const int INVALID_ID = -1;

public:

  int                   Id;
  int64_t               ByteOffset;
  int64_t               ByteLength;
  int32_t               ByteStride; //!< [0, 255]
  glTFBufferViewTarget Target;

  glTFBufferView() //!< Default ctor.
  : Id         (INVALID_ID),
    ByteOffset (0),
    ByteLength (0),
    ByteStride (0),
    Target     (glTFBufferViewTarget_UNKNOWN)
  {}

};

//-----------------------------------------------------------------------------

class glTFJsonSerializer;

//-----------------------------------------------------------------------------

//! Transformation format.
enum glTFWriterTrsfFormat
{
  glTFWriterTrsfFormat_Compact = 0, //!< automatically choose most compact representation between Mat4 and TRS
  glTFWriterTrsfFormat_Mat4    = 1, //!< 4x4 transformation Matrix
  glTFWriterTrsfFormat_TRS     = 2, //!< transformation decomposed into Translation vector, Rotation quaternion and Scale factor (T * R * S)
};
enum { glTFWriterTrsfFormat_LOWER = 0, glTFWriterTrsfFormat_UPPER = glTFWriterTrsfFormat_TRS }; // aliases

//-----------------------------------------------------------------------------

//! Hasher for XCAF nodes.
struct gltf_DocumentNodeHasher
{
  //! Returns hash code based on node string identifier.
  static int HashCode(const XCAFPrs_DocumentNode& N,
                      const int                   upper)
  {
    return ::HashCode(N.Id, upper);
  }

  //! Returns `true` if two document nodes have the same
  //! string identifier.
  static bool IsEqual(const XCAFPrs_DocumentNode& N1,
                      const XCAFPrs_DocumentNode& N2)
  {
    return N1.Id == N2.Id;
  }
};

//-----------------------------------------------------------------------------

//! Indexed map of scene nodes with custom search algorithm.
class gltf_SceneNodeMap : public NCollection_IndexedMap<XCAFPrs_DocumentNode,
                                                        gltf_DocumentNodeHasher>
{
public:

  //! Default ctor.
  gltf_SceneNodeMap() {}

  //! Finds index from the document node's string identifier.
  int FindIndex(const TCollection_AsciiString& nid) const
  {
    if ( this->IsEmpty() )
    {
      return 0;
    }

    for ( IndexedMapNode* it = (IndexedMapNode*) myData1[ ::HashCode( nid, this->NbBuckets() ) ];
          it != nullptr;
          it = (IndexedMapNode*) it->Next() )
    {
      if ( ::IsEqual(it->Key1().Id, nid) )
      {
        return it->Index();
      }
    }
    return 0;
  }
};

//-----------------------------------------------------------------------------

//! Common (obsolete) material definition.
struct glTFMaterialCommon
{
  Handle(Image_Texture) DiffuseTexture; //!< image defining diffuse color
  Quantity_Color        AmbientColor;   //!< ambient  color
  Quantity_Color        DiffuseColor;   //!< diffuse  color
  Quantity_Color        SpecularColor;  //!< specular color
  Quantity_Color        EmissiveColor;  //!< emission color
  float                 Shininess;      //!< shininess value
  float                 Transparency;   //!< transparency value within [0, 1] range with 0 meaning opaque
  bool                  IsDefined;      //!< defined flag; FALSE by default

  //! Default ctor.
  glTFMaterialCommon()
  : AmbientColor  (0.1, 0.1, 0.1, Quantity_TOC_RGB),
    DiffuseColor  (0.8, 0.8, 0.8, Quantity_TOC_RGB),
    SpecularColor (0.2, 0.2, 0.2, Quantity_TOC_RGB),
    EmissiveColor (0.0, 0.0, 0.0, Quantity_TOC_RGB),
    Shininess     (1.0f),
    Transparency  (0.0f),
    IsDefined     (false)
  {}

  //! Compare two materials.
  bool IsEqual(const glTFMaterialCommon& other) const
  {
    if ( &other == this )
    {
      return true;
    }
    else if ( other.IsDefined != IsDefined )
    {
      return false;
    }
    else if ( !IsDefined )
    {
      return true;
    }

    return other.DiffuseTexture == this->DiffuseTexture
        && other.AmbientColor   == this->AmbientColor
        && other.DiffuseColor   == this->DiffuseColor
        && other.SpecularColor  == this->SpecularColor
        && other.EmissiveColor  == this->EmissiveColor
        && other.Shininess      == this->Shininess
        && other.Transparency   == this->Transparency;
  }
};

//-----------------------------------------------------------------------------

//! Metallic-roughness PBR material definition.
struct glTFMaterialPbr
{
  Handle(Image_Texture) BaseColorTexture;         //!< RGB texture for the base color
  Handle(Image_Texture) MetallicRoughnessTexture; //!< RG texture packing the metallic and roughness properties together
  Handle(Image_Texture) EmissiveTexture;          //!< RGB emissive map controls the color and intensity of the light being emitted by the material
  Handle(Image_Texture) OcclusionTexture;         //!< R occlusion map indicating areas of indirect lighting
  Handle(Image_Texture) NormalTexture;            //!< normal map
  Quantity_ColorRGBA    BaseColor;                //!< base color (or scale factor to the texture); [1.0, 1.0, 1.0, 1.0] by default
  Graphic3d_Vec3        EmissiveFactor;           //!< emissive color; [0.0, 0.0, 0.0] by default
  float                 Metallic;                 //!< metalness  (or scale factor to the texture) within range [0.0, 1.0]; 1.0 by default
  float                 Roughness;                //!< roughness  (or scale factor to the texture) within range [0.0, 1.0]; 1.0 by default
  float                 RefractionIndex;          //!< IOR (index of refraction) within range [1.0, 3.0]; 1.5 by default
  bool                  IsDefined;                //!< defined flag; FALSE by default

  //! Default constructor.
  glTFMaterialPbr()
  : BaseColor       (1.0f, 1.0f, 1.0f, 1.0f),
    EmissiveFactor  (0.0f, 0.0f, 0.0f),
    Metallic        (1.0f),
    Roughness       (1.0f),
    RefractionIndex (1.5f),
    IsDefined       (false)
  {}

  //! Compares two materials.
  bool IsEqual(const glTFMaterialPbr& other) const
  {
    if ( &other == this )
    {
      return true;
    }
    else if ( other.IsDefined != this->IsDefined )
    {
      return false;
    }
    else if ( !this->IsDefined )
    {
      return true;
    }

    return other.BaseColorTexture         == this->BaseColorTexture
        && other.MetallicRoughnessTexture == this->MetallicRoughnessTexture
        && other.EmissiveTexture          == this->EmissiveTexture
        && other.OcclusionTexture         == this->OcclusionTexture
        && other.NormalTexture            == this->NormalTexture
        && other.BaseColor                == this->BaseColor
        && other.EmissiveFactor           == this->EmissiveFactor
        && other.Metallic                 == this->Metallic
        && other.Roughness                == this->Roughness
        && other.RefractionIndex          == this->RefractionIndex;
  }
};

} // xde
} // asiAsm

