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

#ifndef gltf_Entities_HeaderFile
#define gltf_Entities_HeaderFile

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
enum gltf_RootElement
{
  /* Mandatory elements */
  gltf_RootElement_Asset,       //!< "asset"       element, mandatory.
  gltf_RootElement_Scenes,      //!< "scenes"      element, mandatory.
  gltf_RootElement_Scene,       //!< "scene"       element, mandatory.
  gltf_RootElement_Nodes,       //!< "nodes"       element, mandatory.
  gltf_RootElement_Meshes,      //!< "meshes"      element, mandatory.
  gltf_RootElement_Accessors,   //!< "accessors"   element, mandatory.
  gltf_RootElement_BufferViews, //!< "bufferViews" element, mandatory.
  gltf_RootElement_Buffers,     //!< "buffers"     element, mandatory.
  NB_MANDATORY,                 //!< number of mandatory elements.

  /* Optional elements */
  gltf_RootElement_Animations = NB_MANDATORY, //!< "animations" element.
  gltf_RootElement_Materials,                 //!< "materials"  element.
  gltf_RootElement_Programs,                  //!< "programs"   element.
  gltf_RootElement_Samplers,                  //!< "samplers"   element.
  gltf_RootElement_Shaders,                   //!< "shaders"    element.
  gltf_RootElement_Skins,                     //!< "skins"      element.
  gltf_RootElement_Techniques,                //!< "techniques" element.
  gltf_RootElement_Textures,                  //!< "textures"   element.
  gltf_RootElement_Images,                    //!< "images"     element.
  gltf_RootElement_ExtensionsUsed,            //!< "extensionsUsed"     element.
  gltf_RootElement_ExtensionsRequired,        //!< "extensionsRequired" element.

  NB //!< overall number of elements.
};

inline const char* gltf_RootElementName(gltf_RootElement elem)
{
  static const char* ROOT_NAMES[gltf_RootElement::NB] =
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
enum gltf_AccessorComponentType
{
  gltf_AccessorComponentType_UNKNOWN,
  gltf_AccessorComponentType_Int8    = 5120, //!< GL_BYTE
  gltf_AccessorComponentType_UInt8   = 5121, //!< GL_UNSIGNED_BYTE
  gltf_AccessorComponentType_Int16   = 5122, //!< GL_SHORT
  gltf_AccessorComponentType_UInt16  = 5123, //!< GL_UNSIGNED_SHORT
  gltf_AccessorComponentType_UInt32  = 5125, //!< GL_UNSIGNED_INT
  gltf_AccessorComponentType_Float32 = 5126  //!< GL_FLOAT
};

//-----------------------------------------------------------------------------

//! Low-level glTF enumeration defining Accessor layout.
//! Similar to Graphic3d_TypeOfData but does not define actual type and includes matrices.
enum gltf_AccessorLayout
{
  gltf_AccessorLayout_UNKNOWN, //!< unknown or invalid type
  gltf_AccessorLayout_Scalar,  //!< "SCALAR"
  gltf_AccessorLayout_Vec2,    //!< "VEC2"
  gltf_AccessorLayout_Vec3,    //!< "VEC3"
  gltf_AccessorLayout_Vec4,    //!< "VEC4"
  gltf_AccessorLayout_Mat2,    //!< "MAT2"
  gltf_AccessorLayout_Mat3,    //!< "MAT3"
  gltf_AccessorLayout_Mat4     //!< "MAT4"
};

//-----------------------------------------------------------------------------

//! Low-level glTF data structure defining Accessor.
struct gltf_Accessor
{
  static const int INVALID_ID = -1;

public:

  int                        Id;            //!< identifier
  int64_t                    ByteOffset;    //!< byte offset
  int64_t                    Count;         //!< size
  int32_t                    ByteStride;    //!< [0, 255] for glTF 1.0
  gltf_AccessorLayout        Type;          //!< layout type
  gltf_AccessorComponentType ComponentType; //!< component type
  Graphic3d_BndBox3d         BndBox;        //!< bounding box

  //! Default ctor.
  gltf_Accessor()
  : Id            (INVALID_ID),
    ByteOffset    (0),
    Count         (0),
    ByteStride    (0),
    Type          (gltf_AccessorLayout_UNKNOWN),
    ComponentType (gltf_AccessorComponentType_UNKNOWN) {}

};

//-----------------------------------------------------------------------------

//! Low-level glTF enumeration defining Primitive type.
//! Similar to Graphic3d_TypeOfData but does not define actual type and includes matrices.
enum gltf_PrimitiveMode
{
  gltf_PrimitiveMode_UNKNOWN       = -1, //!< unknown or invalid type
  gltf_PrimitiveMode_Points        =  0, //!< GL_POINTS
  gltf_PrimitiveMode_Lines         =  1, //!< GL_LINES
  gltf_PrimitiveMode_LineLoop      =  2, //!< GL_LINE_LOOP
  gltf_PrimitiveMode_LineStrip     =  3, //!< GL_LINE_STRIP
  gltf_PrimitiveMode_Triangles     =  4, //!< GL_TRIANGLES
  gltf_PrimitiveMode_TriangleStrip =  5, //!< GL_TRIANGLE_STRIP
  gltf_PrimitiveMode_TriangleFan   =  6  //!< GL_TRIANGLE_FAN
};

//-----------------------------------------------------------------------------

//! Low-level glTF data structure holding single Face (one primitive array) definition.
struct gltf_Face
{
  gltf_Accessor NodePos;  //!< accessor for nodal positions
  gltf_Accessor NodeNorm; //!< accessor for nodal normals
  gltf_Accessor NodeUV;   //!< accessor for nodal UV texture coordinates
  gltf_Accessor Indices;  //!< accessor for indexes
};

//-----------------------------------------------------------------------------

//! Standard coordinate system definition.
//! Open CASCADE does not force application using specific coordinate system,
//! although Draw Harness and samples define +Z-up +Y-forward coordinate system for camera view manipulation.
//! This enumeration defines two commonly used conventions: Z-up and Y-up.
enum gltf_CoordinateSystem
{
  gltf_CoordinateSystem_Undefined      = -1, //!< undefined
  gltf_CoordinateSystem_posYfwd_posZup =  0, //!< +YForward+Zup+Xright
  gltf_CoordinateSystem_negZfwd_posYup,      //!< -ZForward+Yup+Xright

  gltf_CoordinateSystem_Blender = gltf_CoordinateSystem_posYfwd_posZup, //!< coordinate system used by Blender (+YForward+Zup+Xright)
  gltf_CoordinateSystem_glTF    = gltf_CoordinateSystem_negZfwd_posYup, //!< coordinate system used by glTF    (-ZForward+Yup+Xright)
  gltf_CoordinateSystem_Zup     = gltf_CoordinateSystem_Blender,        //!< Z-up coordinate system (+YForward+Zup+Xright)
  gltf_CoordinateSystem_Yup     = gltf_CoordinateSystem_glTF            //!< Y-up coordinate system (-ZForward+Yup+Xright)
};

//-----------------------------------------------------------------------------

//! Low-level glTF enumeration defining BufferView target.
enum gltf_BufferViewTarget
{
  gltf_BufferViewTarget_UNKNOWN,                      //!< unknown or invalid type
  gltf_BufferViewTarget_ARRAY_BUFFER         = 34962, //!< GL_ARRAY_BUFFER
  gltf_BufferViewTarget_ELEMENT_ARRAY_BUFFER = 34963  //!< GL_ELEMENT_ARRAY_BUFFER
};

//-----------------------------------------------------------------------------

//! Low-level glTF data structure defining BufferView.
struct gltf_BufferView
{
  static const int INVALID_ID = -1;

public:

  int                   Id;
  int64_t               ByteOffset;
  int64_t               ByteLength;
  int32_t               ByteStride; //!< [0, 255]
  gltf_BufferViewTarget Target;

  gltf_BufferView() //!< Default ctor.
  : Id         (INVALID_ID),
    ByteOffset (0),
    ByteLength (0),
    ByteStride (0),
    Target     (gltf_BufferViewTarget_UNKNOWN)
  {}

};

//-----------------------------------------------------------------------------

class gltf_JsonSerializer;

//-----------------------------------------------------------------------------

//! Transformation format.
enum gltf_WriterTrsfFormat
{
  gltf_WriterTrsfFormat_Compact = 0, //!< automatically choose most compact representation between Mat4 and TRS
  gltf_WriterTrsfFormat_Mat4    = 1, //!< 4x4 transformation Matrix
  gltf_WriterTrsfFormat_TRS     = 2, //!< transformation decomposed into Translation vector, Rotation quaternion and Scale factor (T * R * S)
};
enum { gltf_WriterTrsfFormat_LOWER = 0, gltf_WriterTrsfFormat_UPPER = gltf_WriterTrsfFormat_TRS }; // aliases

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
struct gltf_MaterialCommon
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
  gltf_MaterialCommon()
  : AmbientColor  (0.1, 0.1, 0.1, Quantity_TOC_RGB),
    DiffuseColor  (0.8, 0.8, 0.8, Quantity_TOC_RGB),
    SpecularColor (0.2, 0.2, 0.2, Quantity_TOC_RGB),
    EmissiveColor (0.0, 0.0, 0.0, Quantity_TOC_RGB),
    Shininess     (1.0f),
    Transparency  (0.0f),
    IsDefined     (false)
  {}

  //! Compare two materials.
  bool IsEqual(const gltf_MaterialCommon& other) const
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

    return other.DiffuseTexture  == this->DiffuseTexture
        && other.AmbientColor    == this->AmbientColor
        && other.DiffuseColor    == this->DiffuseColor
        && other.SpecularColor   == this->SpecularColor
        && other.EmissiveColor   == this->EmissiveColor
        && other.Shininess       == this->Shininess
        && other.Transparency    == this->Transparency;
  }

};

//-----------------------------------------------------------------------------

//! Metallic-roughness PBR material definition.
struct gltf_MaterialPbr
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
  gltf_MaterialPbr()
  : BaseColor       (1.0f, 1.0f, 1.0f, 1.0f),
    EmissiveFactor  (0.0f, 0.0f, 0.0f),
    Metallic        (1.0f),
    Roughness       (1.0f),
    RefractionIndex (1.5f),
    IsDefined       (false)
  {}

  //! Compares two materials.
  bool IsEqual(const gltf_MaterialPbr& other) const
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

#endif
