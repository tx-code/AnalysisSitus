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
#include <asiAsm_GlTFWriter.h>

// GlTF includes
#include <asiAsm_GlTFMaterialMap.h>
#include <asiAsm_GlTFFacePropertyExtractor.h>
//
#if defined USE_RAPIDJSON
  #include <asiAsm_GlTFJsonSerializer.h>
#endif

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <gp_Quaternion.hxx>
#include <NCollection_DataMap.hxx>
#include <OSD_OpenFile.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <Poly_Triangulation.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs.hxx>
#include <XCAFPrs_IndexedDataMapOfShapeStyle.hxx>

// Rapidjson includes
#if defined USE_RAPIDJSON
  #include <rapidjson/prettywriter.h>
  #include <rapidjson/ostreamwrapper.h>
#endif

using namespace asiAsm::xde;

namespace
{
  //! Writes three floating-point values.
  static void writeVec3(std::ostream& stream,
                        const gp_XYZ& _vec3)
  {
    Graphic3d_Vec3 vec3( float( _vec3.X() ), float( _vec3.Y() ), float( _vec3.Z() ) );
    stream.write( (const char*) vec3.GetData(), sizeof(vec3) );
  }

  //! Writes three floating-point values.
  static void writeVec3(std::ostream&         stream,
                        const Graphic3d_Vec3& _vec3)
  {
    stream.write( (const char*) _vec3.GetData(), sizeof(_vec3) );
  }

  //! Writes two floating-point values.
  static void writeVec2(std::ostream& stream,
                        const gp_XY&  _vec2)
  {
    Graphic3d_Vec2 vec2( float( _vec2.X() ), float( _vec2.Y() ) );
    stream.write( (const char*) vec2.GetData(), sizeof(vec2) );
  }

  //! Writes triangle indices.
  static void writeTriangle32(std::ostream&          stream,
                              const Graphic3d_Vec3i& tri)
  {
    stream.write( (const char*) tri.GetData(), sizeof(tri) );
  }

  //! Writes triangle indices.
  static void writeTriangle16(std::ostream&                     stream,
                              const NCollection_Vec3<uint16_t>& tri)
  {
    stream.write( (const char*) tri.GetData(), sizeof(tri) );
  }
}

//-----------------------------------------------------------------------------

gltf_Writer::gltf_Writer(const TCollection_AsciiString& filename,
                       const bool                     isBinary,
                       ActAPI_ProgressEntry           progress,
                       ActAPI_PlotterEntry            plotter)
//
: ActAPI_IAlgorithm (progress, plotter),
  m_filename        (filename),
  m_trsfFormat      (gltf_WriterTrsfFormat_Compact),
  m_bIsBinary       (isBinary),
  m_binDataLen64    (0)
{
  m_CSTrsf.SetOutputLengthUnit(1.0); // meters
  m_CSTrsf.SetOutputCoordinateSystem(gltf_CoordinateSystem_glTF);

  TCollection_AsciiString dir, filenameShort, filenameShortBase, filenameExt;
  OSD_Path::FolderAndFileFromPath(filename, dir, filenameShort);
  asiAlgo_Utils::Str::FileNameAndExtension(filenameShort, filenameShortBase, filenameExt);

  m_binFilenameShort = filenameShortBase + ".bin" + (m_bIsBinary ? ".tmp" : "");
  m_binFilenameFull  = !dir.IsEmpty() ? dir + m_binFilenameShort : m_binFilenameShort;
}

//-----------------------------------------------------------------------------

gltf_Writer::~gltf_Writer()
{
  m_jsonWriter.reset();
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeBinDataNodes(std::ostream&           binFile,
                                   int&                     accessorNb) const
{
  t_Meshes2Primitives::Iterator itNodes2Primitives (m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      itPrm.ChangeValue().NodePos.Id = accessorNb++;
      itPrm.ChangeValue().NodePos.ByteOffset = (int64_t)binFile.tellp() - m_buffViewPos.ByteOffset;

      NCollection_Vector<gp_XYZ>::Iterator itNodes(itPrm.Value().MeshNodes);
      for (; itNodes.More(); itNodes.Next())
      {
        gp_XYZ& node = itNodes.ChangeValue();
        //gp_XYZ node(itNodes.Value());
        m_CSTrsf.TransformPosition(node);
        itPrm.ChangeValue().NodePos.BndBox.Add(Graphic3d_Vec3d(node.X(), node.Y(), node.Z()));
        writeVec3(binFile, node);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeBinDataNormals(std::ostream&      binFile,
                                     int&               accessorNb) const
{
  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      if (itPrm.Value().Normals.Size() == 0)
        continue;

      itPrm.ChangeValue().NodeNorm.Id = accessorNb++;
      itPrm.ChangeValue().NodeNorm.ByteOffset = (int64_t)binFile.tellp() - m_buffViewNorm.ByteOffset;
      NCollection_Vector<Graphic3d_Vec3>::Iterator itNormals(itPrm.Value().Normals);
      for (; itNormals.More(); itNormals.Next())
      {
        //Graphic3d_Vec3 vecNormal(itNormals.Value());
        m_CSTrsf.TransformNormal(itNormals.ChangeValue());
        writeVec3(binFile, itNormals.Value());
      }
    }
  }
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeBinDataTextCoords(std::ostream&            binFile,
                                        int&                     accessorNb) const
{
  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      if (itPrm.Value().Textures.Size() == 0)
        continue;

      itPrm.ChangeValue().NodeUV.Id = accessorNb++;
      itPrm.ChangeValue().NodeUV.ByteOffset = (int64_t)binFile.tellp() - m_buffViewTextCoord.ByteOffset;
      NCollection_Vector<gp_Pnt2d>::Iterator itTextures(itPrm.Value().Textures);
      for (; itTextures.More(); itTextures.Next())
      {
        writeVec2(binFile, itTextures.Value().XY());
      }
    }
  }
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeBinDataIndices(std::ostream&    binFile,
                                     int&             accessorNb)
{
  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      if (itPrm.Value().Triangles.Size() == 0)
        continue;

      itPrm.ChangeValue().Indices.Id = accessorNb++;
      itPrm.ChangeValue().Indices.ByteOffset = (int64_t)binFile.tellp() - m_buffViewInd.ByteOffset;
      NCollection_Vector<Poly_Triangle>::Iterator itTriangles(itPrm.Value().Triangles);
      for (; itTriangles.More(); itTriangles.Next())
      {
        Poly_Triangle tri = itTriangles.Value();
        if (itPrm.Value().Indices.ComponentType == gltf_AccessorComponentType_UInt16)
        {
          writeTriangle16(binFile,
            NCollection_Vec3<uint16_t>((uint16_t)tri(1),
              (uint16_t)tri(2),
              (uint16_t)tri(3)));
        }
        else
        {
          writeTriangle32(binFile, Graphic3d_Vec3i(tri(1), tri(2), tri(3)));
        }
      }
      if (itPrm.Value().Indices.ComponentType == gltf_AccessorComponentType_UInt16)
      {
        // alignment by 4 bytes
        int64_t contentLen64 = (int64_t)binFile.tellp();
        //
        while (contentLen64 % 4 != 0)
        {
          binFile.write(" ", 1);
          ++contentLen64;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------

bool gltf_Writer::Perform(const Handle(gltf_IDataSourceProvider)&     dataProvider,
                         const TColStd_IndexedDataMapOfStringString& fileInfo)
{
  if (dataProvider.IsNull() )
    return false;

  m_dataProvider = dataProvider;
  m_dataProvider->Process(m_progress);

  if ( !this->writeBinData() )
    return false;

  return this->writeJson(fileInfo);
}

//-----------------------------------------------------------------------------

const gltf_CSysConverter&
  gltf_Writer::CoordinateSystemConverter() const
{
  return m_CSTrsf;
}

//-----------------------------------------------------------------------------

gltf_CSysConverter&
  gltf_Writer::ChangeCoordinateSystemConverter()
{
  return m_CSTrsf;
}

//-----------------------------------------------------------------------------

void gltf_Writer::SetCoordinateSystemConverter(const gltf_CSysConverter& converter)
{
  m_CSTrsf = converter;
}

//-----------------------------------------------------------------------------

bool gltf_Writer::IsBinary() const
{
  return m_bIsBinary;
}

//-----------------------------------------------------------------------------

gltf_WriterTrsfFormat
  gltf_Writer::TransformationFormat() const
{
  return m_trsfFormat;
}

//-----------------------------------------------------------------------------

void gltf_Writer::SetTransformationFormat(const gltf_WriterTrsfFormat fmt)
{
  m_trsfFormat = fmt;
}

//-----------------------------------------------------------------------------

bool gltf_Writer::IsForcedUVExport() const
{
  return m_bIsForcedUVExport;
}

//-----------------------------------------------------------------------------

void gltf_Writer::SetForcedUVExport(const bool toForce)
{
  m_bIsForcedUVExport = toForce;
}

//-----------------------------------------------------------------------------

const gltf_XdeVisualStyle&
  gltf_Writer::DefaultStyle() const
{
  return m_defaultStyle;
}

//-----------------------------------------------------------------------------

void gltf_Writer::SetDefaultStyle(const gltf_XdeVisualStyle& style)
{
  m_defaultStyle = style;
}

//-----------------------------------------------------------------------------

bool gltf_Writer::writeBinData()
{
  m_buffViewPos.ByteOffset       = 0;
  m_buffViewPos.ByteLength       = 0;
  m_buffViewPos.ByteStride       = 12;
  m_buffViewPos.Target           = gltf_BufferViewTarget_ARRAY_BUFFER;
  m_buffViewNorm.ByteOffset      = 0;
  m_buffViewNorm.ByteLength      = 0;
  m_buffViewNorm.ByteStride      = 12;
  m_buffViewNorm.Target          = gltf_BufferViewTarget_ARRAY_BUFFER;
  m_buffViewTextCoord.ByteOffset = 0;
  m_buffViewTextCoord.ByteLength = 0;
  m_buffViewTextCoord.ByteStride = 8;
  m_buffViewTextCoord.Target     = gltf_BufferViewTarget_ARRAY_BUFFER;
  m_buffViewInd.ByteOffset       = 0;
  m_buffViewInd.ByteLength       = 0;
  m_buffViewInd.Target           = gltf_BufferViewTarget_ELEMENT_ARRAY_BUFFER;

  m_binDataLen64 = 0;

  std::ofstream binFile;
  OSD_OpenStream(binFile, m_binFilenameFull.ToCString(), std::ios::out | std::ios::binary);
  //
  if ( !binFile.is_open()|| !binFile.good() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be created."
                                             << m_binFilenameFull);
    return false;
  }

  int nbAccessors = 0;

  /* =================
   *  Write positions.
   * ================= */

  m_buffViewPos.ByteOffset = binFile.tellp();
  //

  this->writeBinDataNodes(binFile, nbAccessors);

  if (!binFile.good())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
    return false;
  }

  m_buffViewPos.ByteLength = (int64_t) binFile.tellp() - m_buffViewPos.ByteOffset;

  /* ===============
   *  Write normals.
   * =============== */

  m_buffViewNorm.ByteOffset = binFile.tellp();

  this->writeBinDataNormals(binFile, nbAccessors);

  if (!binFile.good())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
    return false;
  }

  m_buffViewNorm.ByteLength = (int64_t) binFile.tellp() - m_buffViewNorm.ByteOffset;

  /* ===========================
   *  Write texture coordinates.
   * =========================== */

  m_buffViewTextCoord.ByteOffset = binFile.tellp();

  if (!m_bIsForcedUVExport)
  {
    this->writeBinDataTextCoords(binFile, nbAccessors);
  }

  if (!binFile.good())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
    return false;
  }

  m_buffViewTextCoord.ByteLength = (int64_t) binFile.tellp() - m_buffViewTextCoord.ByteOffset;

  /* ===============
   *  Write indices.
   * =============== */

  m_buffViewInd.ByteOffset = binFile.tellp();

  this->writeBinDataIndices(binFile, nbAccessors);

  if (!binFile.good())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
    return false;
  }

  m_buffViewInd.ByteLength = (int64_t )binFile.tellp() - m_buffViewInd.ByteOffset;

  /* ==========
   *  Finalize.
   * ========== */

  int buffViewId = 0;
  if ( m_buffViewPos.ByteLength > 0 )
  {
    m_buffViewPos.Id = buffViewId++;
  }
  if ( m_buffViewNorm.ByteLength > 0 )
  {
    m_buffViewNorm.Id = buffViewId++;
  }
  if ( m_buffViewTextCoord.ByteLength > 0 )
  {
    m_buffViewTextCoord.Id = buffViewId++;
  }
  if ( m_buffViewInd.ByteLength > 0 )
  {
    m_buffViewInd.Id = buffViewId++;
  }

  m_binDataLen64 = binFile.tellp();
  binFile.close();
  //
  if ( !binFile.good() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool gltf_Writer::writeJson(const TColStd_IndexedDataMapOfStringString& fileInfo)
{
#if defined USE_RAPIDJSON
  m_jsonWriter.reset();

  const int binDatbufferId = 0;
  const int defSamplerId   = 0;
  const int defSceneId     = 0;

  const TCollection_AsciiString fileNameGltf = m_filename;
  std::ofstream gltfContentFile;
  OSD_OpenStream(gltfContentFile, fileNameGltf.ToCString(), std::ios::out | std::ios::binary);

  if ( !gltfContentFile.is_open() || !gltfContentFile.good())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be created." << fileNameGltf);
    return false;
  }

  if ( m_bIsBinary )
  {
    const char* magic         = "glTF";
    uint32_t    version       = 2;
    uint32_t    length        = 0;
    uint32_t    contentLength = 0;
    uint32_t    contentType   = 0x4E4F534A;

    gltfContentFile.write( magic,                        4                     );
    gltfContentFile.write( (const char*) &version,       sizeof(version)       );
    gltfContentFile.write( (const char*) &length,        sizeof(length)        );
    gltfContentFile.write( (const char*) &contentLength, sizeof(contentLength) );
    gltfContentFile.write( (const char*) &contentType,   sizeof(contentType)   );
  }

  // Prepare material map.
  gltf_MaterialMap materialMap(m_filename, defSamplerId);
  materialMap.SetDefaultStyle(m_defaultStyle);

  // Root nodes indices start from 0.
  NCollection_Sequence<int> scRootIds;

  // Prepare JSON writer.
  rapidjson::OStreamWrapper fileStream(gltfContentFile);
  m_jsonWriter.reset( new gltf_JsonSerializer(fileStream) );

  // Start writing.
  m_jsonWriter->StartObject();
  {
    // Write sections.
    this->writeAccessors   ();
    this->writeAnimations  ();
    this->writeAsset       (fileInfo);
    this->writeBufferViews (binDatbufferId);
    this->writeBuffers     ();
    this->writeExtensions  ();
    this->writeImages      (materialMap);
    this->writeMaterials   (materialMap);
    this->writeMeshes      (materialMap);
    this->writeNodes       ();
    this->writeSamplers    (materialMap);
    this->writeScene       (defSceneId);
    this->writeScenes      ();
    this->writeSkins       ();
    this->writeTextures    (materialMap);
  }
  m_jsonWriter->EndObject();

  if ( !m_bIsBinary )
  {
    gltfContentFile.close();
    if ( !gltfContentFile.good() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << fileNameGltf);
      return false;
    }
    return true;
  }

  int64_t contentLen64 = (int64_t) gltfContentFile.tellp() - 20;
  while ( contentLen64 % 4 != 0 )
  {
    gltfContentFile.write(" ", 1);
    ++contentLen64;
  }

  const uint32_t binLength = (uint32_t) m_binDataLen64;
  const uint32_t binType   = 0x004E4942;
  //
  gltfContentFile.write( (const char*) &binLength, 4 );
  gltfContentFile.write( (const char*) &binType,   4 );

  const int64_t fullLen64 = contentLen64 + 20 + m_binDataLen64 + 8;
  //
  if ( fullLen64 < std::numeric_limits<uint32_t>::max() )
  {
    {
      std::ifstream binFile;
      OSD_OpenStream(binFile, m_binFilenameFull.ToCString(), std::ios::in | std::ios::binary);
      //
      if ( !binFile.is_open() || !binFile.good() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be opened." << m_binFilenameFull);
        return false;
      }
      char buffer[4096];
      for ( ; binFile.good(); )
      {
        binFile.read(buffer, 4096);
        const int readLen = (int) binFile.gcount();
        if ( readLen == 0 )
        {
          break;
        }
        gltfContentFile.write(buffer, readLen);
      }
    }
    OSD_Path binFilePath (m_binFilenameFull);
    OSD_File(binFilePath).Remove();
    //
    if ( OSD_File(binFilePath).Exists() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Unable to remove temporary glTF content file '%1'." << m_binFilenameFull);
    }
  }
  else
  {
    m_progress.SendLogMessage(LogErr(Normal) << "glTF file content is too big for binary format.");
    return false;
  }

  const uint32_t length        = (uint32_t) fullLen64;
  const uint32_t contentLength = (uint32_t) contentLen64;
  //
  gltfContentFile.seekp (8);
  gltfContentFile.write ( (const char*) &length,        4 );
  gltfContentFile.write ( (const char*) &contentLength, 4 );

  gltfContentFile.close();
  if ( !gltfContentFile.good() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << fileNameGltf);
    return false;
  }

  m_jsonWriter.reset();
  return true;
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) fileInfo;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
  return false;
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeAccessors()
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeAccessors()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Accessors) );
  m_jsonWriter->StartArray();

  /* =================
   *  Write positions.
   * ================= */

  //
  for (t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes()); itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      //
      this->writePositions(itPrm.Value());
    }
  }

  /* ===============
   *  Write normals.
   * =============== */

  //
  for (t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes()); itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      //
      this->writeNormals(itPrm.Value());
    }
  }

  /* ===========================
   *  Write texture coordinates.
   * =========================== */

  //
  for (t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes()); itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      //
      this->writeTextCoords(itPrm.Value());
    }
  }

  /* ===============
   *  Write indices.
   * =============== */

  //
  for (t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes()); itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive> primitives = itNodes2Primitives.Value();

    NCollection_Vector<gltf_Primitive>::Iterator itPrm(primitives);
    for (; itPrm.More(); itPrm.Next())
    {
      //
      this->writeIndices(itPrm.Value());
    }
  }

  m_jsonWriter->EndArray();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writePositions(const gltf_Primitive& gltfPrm)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writePositions()");

  if (gltfPrm.NodePos.Id == gltf_Accessor::INVALID_ID )
  {
    return;
  }

  m_jsonWriter->StartObject();
  m_jsonWriter->Key   ("bufferView");
  m_jsonWriter->Int   (m_buffViewPos.Id);
  m_jsonWriter->Key   ("byteOffset");
  m_jsonWriter->Int64 (gltfPrm.NodePos.ByteOffset);
  m_jsonWriter->Key   ("componentType");
  m_jsonWriter->Int   (gltfPrm.NodePos.ComponentType);
  m_jsonWriter->Key   ("count");
  m_jsonWriter->Int64 (gltfPrm.NodePos.Count);

  if (gltfPrm.NodePos.BndBox.IsValid() )
  {
    m_jsonWriter->Key ("max");
    m_jsonWriter->StartArray();
    m_jsonWriter->Double (gltfPrm.NodePos.BndBox.CornerMax().x() );
    m_jsonWriter->Double (gltfPrm.NodePos.BndBox.CornerMax().y() );
    m_jsonWriter->Double (gltfPrm.NodePos.BndBox.CornerMax().z() );
    m_jsonWriter->EndArray();

    m_jsonWriter->Key("min");
    m_jsonWriter->StartArray();
    m_jsonWriter->Double (gltfPrm.NodePos.BndBox.CornerMin().x() );
    m_jsonWriter->Double (gltfPrm.NodePos.BndBox.CornerMin().y() );
    m_jsonWriter->Double (gltfPrm.NodePos.BndBox.CornerMin().z() );
    m_jsonWriter->EndArray();
  }
  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("VEC3");

  m_jsonWriter->EndObject();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeNormals(const gltf_Primitive& gltfPrm)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeNormals()");

  if (gltfPrm.NodeNorm.Id == gltf_Accessor::INVALID_ID )
    return;

  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("bufferView");
  m_jsonWriter->Int    (m_buffViewNorm.Id);
  m_jsonWriter->Key    ("byteOffset");
  m_jsonWriter->Int64  (gltfPrm.NodeNorm.ByteOffset);
  m_jsonWriter->Key    ("componentType");
  m_jsonWriter->Int    (gltfPrm.NodeNorm.ComponentType);
  m_jsonWriter->Key    ("count");
  m_jsonWriter->Int64  (gltfPrm.NodeNorm.Count);

  /* min/max values are optional, and not very useful for normals - skip them */

  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("VEC3");

  m_jsonWriter->EndObject();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeTextCoords(const gltf_Primitive& gltfPrm)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeTextCoords()");

  if (gltfPrm.NodeUV.Id == gltf_Accessor::INVALID_ID )
    return;

  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("bufferView");
  m_jsonWriter->Int    (m_buffViewTextCoord.Id);
  m_jsonWriter->Key    ("byteOffset");
  m_jsonWriter->Int64  (gltfPrm.NodeUV.ByteOffset);
  m_jsonWriter->Key    ("componentType");
  m_jsonWriter->Int    (gltfPrm.NodeUV.ComponentType);
  m_jsonWriter->Key    ("count");
  m_jsonWriter->Int64  (gltfPrm.NodeUV.Count);

  /* min/max values are optional, and not very useful for UV coordinates - skip them */

  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("VEC2");

  m_jsonWriter->EndObject();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeIndices (const gltf_Primitive& gltfPrm)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeIndices()");

  if (gltfPrm.Indices.Id == gltf_Accessor::INVALID_ID )
    return;

  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("bufferView");
  m_jsonWriter->Int    (m_buffViewInd.Id);
  m_jsonWriter->Key    ("byteOffset");
  m_jsonWriter->Int64  (gltfPrm.Indices.ByteOffset);
  m_jsonWriter->Key    ("componentType");
  m_jsonWriter->Int    (gltfPrm.Indices.ComponentType);
  m_jsonWriter->Key    ("count");
  m_jsonWriter->Int64  (gltfPrm.Indices.Count);

  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("SCALAR");

  m_jsonWriter->EndObject();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeAnimations()
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeAsset(const TColStd_IndexedDataMapOfStringString& fileInfo)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if (m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeAsset()");

  m_jsonWriter->Key    ( gltf_RootElementName(gltf_RootElement_Asset) );
  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("generator");
  m_jsonWriter->String (gltf_VendorName);
  m_jsonWriter->Key    ("version");
  m_jsonWriter->String ("2.0"); // glTF format version

  bool isStarted = false;
  for ( TColStd_IndexedDataMapOfStringString::Iterator kvIt(fileInfo); kvIt.More(); kvIt.Next() )
  {
    if ( !isStarted )
    {
      m_jsonWriter->Key("extras");
      m_jsonWriter->StartObject();
      isStarted = true;
    }
    m_jsonWriter->Key( kvIt.Key().ToCString() );
    m_jsonWriter->String( kvIt.Value().ToCString() );
  }
  if ( isStarted )
  {
    m_jsonWriter->EndObject();
  }

  m_jsonWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) fileInfo;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeBufferViews(const int binDataBufferId)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeBufferViews()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_BufferViews) );
  m_jsonWriter->StartArray();

  if ( m_buffViewPos.Id != gltf_Accessor::INVALID_ID )
  {
    m_jsonWriter->StartObject();
    m_jsonWriter->Key    ("buffer");
    m_jsonWriter->Int    (binDataBufferId);
    m_jsonWriter->Key    ("byteLength");
    m_jsonWriter->Int64  (m_buffViewPos.ByteLength);
    m_jsonWriter->Key    ("byteOffset");
    m_jsonWriter->Int64  (m_buffViewPos.ByteOffset);
    m_jsonWriter->Key    ("byteStride");
    m_jsonWriter->Int64  (m_buffViewPos.ByteStride);
    m_jsonWriter->Key    ("target");
    m_jsonWriter->Int    (m_buffViewPos.Target);
    m_jsonWriter->EndObject();
  }
  if ( m_buffViewNorm.Id != gltf_Accessor::INVALID_ID )
  {
    m_jsonWriter->StartObject();
    m_jsonWriter->Key    ("buffer");
    m_jsonWriter->Int    (binDataBufferId);
    m_jsonWriter->Key    ("byteLength");
    m_jsonWriter->Int64  (m_buffViewNorm.ByteLength);
    m_jsonWriter->Key    ("byteOffset");
    m_jsonWriter->Int64  (m_buffViewNorm.ByteOffset);
    m_jsonWriter->Key    ("byteStride");
    m_jsonWriter->Int64  (m_buffViewNorm.ByteStride);
    m_jsonWriter->Key    ("target");
    m_jsonWriter->Int    (m_buffViewNorm.Target);
    m_jsonWriter->EndObject();
  }
  if ( m_buffViewTextCoord.Id != gltf_Accessor::INVALID_ID )
  {
    m_jsonWriter->StartObject();
    m_jsonWriter->Key    ("buffer");
    m_jsonWriter->Int    (binDataBufferId);
    m_jsonWriter->Key    ("byteLength");
    m_jsonWriter->Int64  (m_buffViewTextCoord.ByteLength);
    m_jsonWriter->Key    ("byteOffset");
    m_jsonWriter->Int64  (m_buffViewTextCoord.ByteOffset);
    m_jsonWriter->Key    ("byteStride");
    m_jsonWriter->Int64  (m_buffViewTextCoord.ByteStride);
    m_jsonWriter->Key    ("target");
    m_jsonWriter->Int    (m_buffViewTextCoord.Target);
    m_jsonWriter->EndObject();
  }
  if ( m_buffViewInd.Id != gltf_Accessor::INVALID_ID )
  {
    m_jsonWriter->StartObject();
    m_jsonWriter->Key    ("buffer");
    m_jsonWriter->Int    (binDataBufferId);
    m_jsonWriter->Key    ("byteLength");
    m_jsonWriter->Int64  (m_buffViewInd.ByteLength);
    m_jsonWriter->Key    ("byteOffset");
    m_jsonWriter->Int64  (m_buffViewInd.ByteOffset);
    m_jsonWriter->Key    ("target");
    m_jsonWriter->Int    (m_buffViewInd.Target);
    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) binDataBufferId;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeBuffers()
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeBuffers()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Buffers) );
  m_jsonWriter->StartArray();
  {
    m_jsonWriter->StartObject();
    {
      m_jsonWriter->Key   ("byteLength");
      m_jsonWriter->Int64 (m_buffViewPos.ByteLength + m_buffViewNorm.ByteLength +
                           m_buffViewTextCoord.ByteLength + m_buffViewInd.ByteLength);
      if ( !m_bIsBinary )
      {
        m_jsonWriter->Key    ("uri");
        m_jsonWriter->String (m_binFilenameShort.ToCString());
      }
    }
    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeExtensions()
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeImages(gltf_MaterialMap& materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeImages()");

  // empty section should NOT be written to avoid validator errors
  bool isStarted = false;
  //
  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      materialMap.AddImages(m_jsonWriter.get(), itPrm.Value().Style, isStarted);
    }
  }
  if ( isStarted )
  {
    m_jsonWriter->EndArray();
  }
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeMaterials(gltf_MaterialMap& materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeMaterials()");

  // empty section should NOT be written to avoid validator errors
  bool isStarted = false;
  //
  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      materialMap.AddMaterial(m_jsonWriter.get(), itPrm.Value().Style, isStarted);
    }
  }

  if (isStarted)
  {
    m_jsonWriter->EndArray();
  }
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void)materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
    "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeMeshes(const gltf_MaterialMap& materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeMeshes()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Meshes) );
  m_jsonWriter->StartArray();

  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    const TCollection_AsciiString& nodeName = itNodes2Primitives.Key()->Name;

    bool toStartPrims = true;

    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      if ( toStartPrims )
      {
        toStartPrims = false;
        m_jsonWriter->StartObject();
        m_jsonWriter->Key("name");
        m_jsonWriter->String( nodeName.ToCString() );
        m_jsonWriter->Key("primitives");
        m_jsonWriter->StartArray();
      }

      const gltf_Primitive& primitive = itPrm.Value();
      const TCollection_AsciiString matId    = materialMap.FindMaterial(primitive.Style );

      m_jsonWriter->StartObject();
      {
        m_jsonWriter->Key("attributes");
        m_jsonWriter->StartObject();
        {
          if (primitive.NodeNorm.Id != gltf_Accessor::INVALID_ID )
          {
            m_jsonWriter->Key("NORMAL");
            m_jsonWriter->Int(primitive.NodeNorm.Id);
          }
          m_jsonWriter->Key("POSITION");
          m_jsonWriter->Int(primitive.NodePos.Id);

          if (primitive.NodeUV.Id != gltf_Accessor::INVALID_ID )
          {
            m_jsonWriter->Key("TEXCOORD_0");
            m_jsonWriter->Int(primitive.NodeUV.Id);
          }
        }
        m_jsonWriter->EndObject();

        if (primitive.Indices.Id != gltf_Accessor::INVALID_ID)
        {
          m_jsonWriter->Key("indices");
          m_jsonWriter->Int(primitive.Indices.Id);
        }

        if ( !matId.IsEmpty() )
        {
          m_jsonWriter->Key("material");
          m_jsonWriter->Int( matId.IntegerValue() );
        }
        m_jsonWriter->Key("mode");
        m_jsonWriter->Int((int)primitive.Mode);

        if (!primitive.Name.IsEmpty())
        {
          m_jsonWriter->Key("name");
          m_jsonWriter->String(primitive.Name.ToCString());
        }
      }
      m_jsonWriter->EndObject();
    }

    if ( !toStartPrims )
    {
      m_jsonWriter->EndArray();
      m_jsonWriter->EndObject();
    }
  }
  m_jsonWriter->EndArray();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeNodes()
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeNodes()");

  //* Write scene nodes using prepared map for correct order of array members
  m_jsonWriter->Key(gltf_RootElementName(gltf_RootElement_Nodes));
  m_jsonWriter->StartArray();

  const gltf_SceneStructure& sceneNodeStrt = m_dataProvider->GetSceneStructure();
  for (auto node : sceneNodeStrt.GetNodes())
  {
    m_jsonWriter->StartObject();
    {
      if (node->Children.size() > 0)
      {
        m_jsonWriter->Key("children");
        m_jsonWriter->StartArray();
        {
          for (auto child : node->Children)
          {
            m_jsonWriter->Int(sceneNodeStrt.GetIndex(child));
          }
        }
        m_jsonWriter->EndArray();
      }
    }
    if (!node->Trsf.IsIdentity())
    {
      gp_Trsf trsf = node->Trsf.Transformation();

      if (trsf.Form() != gp_Identity)
      {
        m_CSTrsf.TransformTransformation(trsf);

        const gp_Quaternion qn = trsf.GetRotation();
        const bool hasRotation = Abs(qn.X()) > gp::Resolution()
          || Abs(qn.Y()) > gp::Resolution()
          || Abs(qn.Z()) > gp::Resolution()
          || Abs(qn.W() - 1.0) > gp::Resolution();
        //
        const double  scaleFactor = trsf.ScaleFactor();
        const bool    hasScale = Abs(scaleFactor - 1.0) > Precision::Confusion();
        const gp_XYZ& translPart = trsf.TranslationPart();
        const bool    hasTranslation = translPart.SquareModulus() > gp::Resolution();

        gltf_WriterTrsfFormat trsfFormat = m_trsfFormat;
        if (m_trsfFormat == gltf_WriterTrsfFormat_Compact)
        {
          trsfFormat = hasRotation && hasScale && hasTranslation
            ? gltf_WriterTrsfFormat_Mat4
            : gltf_WriterTrsfFormat_TRS;
        }

        if (trsfFormat == gltf_WriterTrsfFormat_Mat4)
        {
          // write full matrix
          Graphic3d_Mat4 mat4;
          trsf.GetMat4(mat4);
          //
          if (!mat4.IsIdentity())
          {
            m_jsonWriter->Key("matrix");
            m_jsonWriter->StartArray();
            //
            for (int icol = 0; icol < 4; ++icol)
            {
              for (int irow = 0; irow < 4; ++irow)
              {
                m_jsonWriter->Double(mat4.GetValue(irow, icol));
              }
            }
            //
            m_jsonWriter->EndArray();
          }
        }
        else // TRS
        {
          if (hasRotation)
          {
            m_jsonWriter->Key("rotation");
            m_jsonWriter->StartArray();
            m_jsonWriter->Double(qn.X());
            m_jsonWriter->Double(qn.Y());
            m_jsonWriter->Double(qn.Z());
            m_jsonWriter->Double(qn.W());
            m_jsonWriter->EndArray();
          }
          if (hasScale)
          {
            m_jsonWriter->Key("scale");
            m_jsonWriter->StartArray();
            m_jsonWriter->Double(scaleFactor);
            m_jsonWriter->Double(scaleFactor);
            m_jsonWriter->Double(scaleFactor);
            m_jsonWriter->EndArray();
          }
          if (hasTranslation)
          {
            m_jsonWriter->Key("translation");
            m_jsonWriter->StartArray();
            m_jsonWriter->Double(translPart.X());
            m_jsonWriter->Double(translPart.Y());
            m_jsonWriter->Double(translPart.Z());
            m_jsonWriter->EndArray();
          }
        }
      }
    }
    if (node->MeshIndex != gltf_Node::INVALID_ID)
    {
      m_jsonWriter->Key("mesh");
      m_jsonWriter->Int(node->MeshIndex);
    }

    m_jsonWriter->Key("name");
    m_jsonWriter->String(node->Name.ToCString());

    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
    "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeSamplers(const gltf_MaterialMap& materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeSamplers()");

  if ( materialMap.NbImages() == 0 )
    return;

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Samplers) );
  m_jsonWriter->StartArray();
  {
    m_jsonWriter->StartObject();
    {
      //m_jsonWriter->Key ("magFilter");
      //m_jsonWriter->Int (9729);
      //m_jsonWriter->Key ("minFilter");
      //m_jsonWriter->Int (9729);
    }
    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeScene(const int defSceneId)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeScene()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Scene) );
  m_jsonWriter->Int(defSceneId);
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) defSceneId;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeScenes()
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeScenes()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Scenes) );
  m_jsonWriter->StartArray();
  {
    m_jsonWriter->StartObject();
    m_jsonWriter->Key("nodes");
    m_jsonWriter->StartArray();
    //
    const gltf_SceneStructure& structure = m_dataProvider->GetSceneStructure();
    for (auto root : structure.GetRoots())
    {
      m_jsonWriter->Int(structure.GetIndex(root));
    }
    //
    m_jsonWriter->EndArray();
    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeSkins()
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void gltf_Writer::writeTextures(gltf_MaterialMap& materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeTextures()");

  // empty section should not be written to avoid validator errors
  bool isStarted = false;
  //
  t_Meshes2Primitives::Iterator itNodes2Primitives(m_dataProvider->GetSceneMeshes());
  for (; itNodes2Primitives.More(); itNodes2Primitives.Next())
  {
    NCollection_Vector<gltf_Primitive>::Iterator itPrm(itNodes2Primitives.Value());
    for (; itPrm.More(); itPrm.Next())
    {
      materialMap.AddTextures( m_jsonWriter.get(), itPrm.Value().Style, isStarted );
    }
  }
  if ( isStarted )
  {
    m_jsonWriter->EndArray();
  }
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}
