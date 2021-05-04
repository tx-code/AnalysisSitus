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
#include <gltf_XdeWriter.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// glTF includes
#include <gltf_FaceIterator.h>
#include <gltf_MaterialMap.h>
//
#if defined USE_RAPIDJSON
  #include <gltf_JsonSerializer.h>
#endif

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
#include <XCAFPrs_DocumentExplorer.hxx>

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

#if defined USE_RAPIDJSON
  //! Reads name attribute.
  static TCollection_AsciiString readNameAttribute(const Handle(XCAFDoc_ShapeTool)& ST,
                                                   const TDF_Label&                 refLabel,
                                                   const bool                       usePrototypeNames = false)
  {
    TDF_Label lab;

    if ( usePrototypeNames )
    {
      if ( ST->IsReference(refLabel) )
      {
        ST->GetReferredShape(refLabel, lab);
      }
      else
      {
        lab = refLabel;
      }
    }
    else
    {
      lab = refLabel;
    }

    Handle(TDataStd_Name) nodeName;
    //
    if ( !lab.FindAttribute(TDataStd_Name::GetID(), nodeName) )
    {
      return TCollection_AsciiString();
    }
    return TCollection_AsciiString( nodeName->Get() );
  }
#endif
}

//-----------------------------------------------------------------------------

gltfWriter::gltfWriter(const TCollection_AsciiString& filename,
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

gltfWriter::~gltfWriter()
{
  m_jsonWriter.reset();
}

//-----------------------------------------------------------------------------

bool gltfWriter::toSkipFaceMesh(const gltf_FaceIterator& faceIter)
{
  return faceIter.IsEmptyMesh();
}

//-----------------------------------------------------------------------------

void gltfWriter::saveNodes(gltf_Face&               gltfFace,
                           std::ostream&            binFile,
                           const gltf_FaceIterator& faceIter,
                           int&                     accessorNb) const
{
  gltfFace.NodePos.Id            = accessorNb++;
  gltfFace.NodePos.Count         = faceIter.NbNodes();
  gltfFace.NodePos.ByteOffset    = (int64_t) binFile.tellp() - m_buffViewPos.ByteOffset;
  gltfFace.NodePos.Type          = gltf_AccessorLayout_Vec3;
  gltfFace.NodePos.ComponentType = gltf_AccessorComponentType_Float32;

  const int nodeUpper = faceIter.NodeUpper();
  //
  for ( int nit = faceIter.NodeLower(); nit <= nodeUpper; ++nit )
  {
    gp_XYZ node = faceIter.NodeTransformed(nit).XYZ();
    m_CSTrsf.TransformPosition(node);
    gltfFace.NodePos.BndBox.Add( Graphic3d_Vec3d( node.X(), node.Y(), node.Z() ) );
    writeVec3(binFile, node);
  }
}

//-----------------------------------------------------------------------------

void gltfWriter::saveNormals(gltf_Face&         gltfFace,
                             std::ostream&      binFile,
                             gltf_FaceIterator& faceIter,
                             int&               accessorNb) const
{
  if ( !faceIter.HasNormals() )
  {
    return;
  }

  gltfFace.NodeNorm.Id            = accessorNb++;
  gltfFace.NodeNorm.Count         = faceIter.NbNodes();
  gltfFace.NodeNorm.ByteOffset    = (int64_t) binFile.tellp() - m_buffViewNorm.ByteOffset;
  gltfFace.NodeNorm.Type          = gltf_AccessorLayout_Vec3;
  gltfFace.NodeNorm.ComponentType = gltf_AccessorComponentType_Float32;

  const int nodeUpper = faceIter.NodeUpper();
  //
  for ( int nit = faceIter.NodeLower(); nit <= nodeUpper; ++nit )
  {
    const gp_Dir norm = faceIter.NormalTransformed(nit);
    Graphic3d_Vec3 vecNormal( (float) norm.X(), (float) norm.Y(), (float) norm.Z() );
    m_CSTrsf.TransformNormal(vecNormal);
    writeVec3(binFile, vecNormal);
  }
}

//-----------------------------------------------------------------------------

void gltfWriter::saveTextCoords(gltf_Face&               gltfFace,
                                std::ostream&            binFile,
                                const gltf_FaceIterator& faceIter,
                                int&                     accessorNb) const
{
  if ( !faceIter.HasTexCoords() )
  {
    return;
  }
  if ( !m_bIsForcedUVExport )
  {
    if ( faceIter.FaceStyle().GetMaterial().IsNull() )
    {
      return;
    }

    if ( gltf_MaterialMap::baseColorTexture( faceIter.FaceStyle().GetMaterial() ).IsNull()
     && faceIter.FaceStyle().GetMaterial()->PbrMaterial().MetallicRoughnessTexture.IsNull()
     && faceIter.FaceStyle().GetMaterial()->PbrMaterial().EmissiveTexture.IsNull()
     && faceIter.FaceStyle().GetMaterial()->PbrMaterial().OcclusionTexture.IsNull()
     && faceIter.FaceStyle().GetMaterial()->PbrMaterial().NormalTexture.IsNull())
    {
      return;
    }
  }

  gltfFace.NodeUV.Id            = accessorNb++;
  gltfFace.NodeUV.Count         = faceIter.NbNodes();
  gltfFace.NodeUV.ByteOffset    = (int64_t) binFile.tellp() - m_buffViewTextCoord.ByteOffset;
  gltfFace.NodeUV.Type          = gltf_AccessorLayout_Vec2;
  gltfFace.NodeUV.ComponentType = gltf_AccessorComponentType_Float32;

  const int nodeUpper = faceIter.NodeUpper();
  //
  for ( int nit = faceIter.NodeLower(); nit <= nodeUpper; ++nit )
  {
    gp_Pnt2d texCoord = faceIter.NodeTexCoord(nit);
    texCoord.SetY( 1.0 - texCoord.Y() );
    writeVec2( binFile, texCoord.XY() );
  }
}

//-----------------------------------------------------------------------------

void gltfWriter::saveIndices(gltf_Face&               gltfFace,
                             std::ostream&            binFile,
                             const gltf_FaceIterator& faceIter,
                             int&                     accessorNb)
{
  gltfFace.Indices.Id            = accessorNb++;
  gltfFace.Indices.Count         = faceIter.NbTriangles() * 3;
  gltfFace.Indices.ByteOffset    = (int64_t) binFile.tellp() - m_buffViewInd.ByteOffset;
  gltfFace.Indices.Type          = gltf_AccessorLayout_Scalar;
  gltfFace.Indices.ComponentType = gltfFace.NodePos.Count > std::numeric_limits<uint16_t>::max()
                                    ? gltf_AccessorComponentType_UInt32
                                    : gltf_AccessorComponentType_UInt16;

  const int elemLower = faceIter.ElemLower();
  const int elemUpper = faceIter.ElemUpper();
  //
  for ( int eit = elemLower; eit <= elemUpper; ++eit )
  {
    Poly_Triangle tri = faceIter.TriangleOriented (eit);
    tri(1) -= elemLower;
    tri(2) -= elemLower;
    tri(3) -= elemLower;

    if ( gltfFace.Indices.ComponentType == gltf_AccessorComponentType_UInt16 )
    {
      writeTriangle16( binFile,
                       NCollection_Vec3<uint16_t>( (uint16_t) tri(1),
                                                   (uint16_t) tri(2),
                                                   (uint16_t) tri(3) ) );
    }
    else
    {
      writeTriangle32( binFile, Graphic3d_Vec3i( tri(1), tri(2), tri(3) ) );
    }
  }
  if ( gltfFace.Indices.ComponentType == gltf_AccessorComponentType_UInt16 )
  {
    // alignment by 4 bytes
    int64_t contentLen64 = (int64_t) binFile.tellp();
    //
    while ( contentLen64 % 4 != 0 )
    {
      binFile.write(" ", 1);
      ++contentLen64;
    }
  }
}

//-----------------------------------------------------------------------------

bool gltfWriter::Perform(const Handle(TDocStd_Document)&             doc,
                         const TColStd_IndexedDataMapOfStringString& fileInfo)
{
  TDF_LabelSequence rootLabs;
  m_shapeTool = XCAFDoc_DocumentTool::ShapeTool( doc->Main() );
  m_shapeTool->GetFreeShapes(rootLabs);

  return this->Perform(doc, rootLabs, nullptr, fileInfo);
}

//-----------------------------------------------------------------------------

bool gltfWriter::Perform(const Handle(TDocStd_Document)&             doc,
                         const TDF_LabelSequence&                    rootLabs,
                         const TColStd_MapOfAsciiString*             labFilter,
                         const TColStd_IndexedDataMapOfStringString& fileInfo)
{
  if ( m_shapeTool.IsNull() )
    m_shapeTool = XCAFDoc_DocumentTool::ShapeTool( doc->Main() );

  if ( !this->writeBinData(doc, rootLabs, labFilter) )
    return false;

  return this->writeJson(doc, rootLabs, labFilter, fileInfo);
}

//-----------------------------------------------------------------------------

const gltf_CSysConverter&
  gltfWriter::CoordinateSystemConverter() const
{
  return m_CSTrsf;
}

//-----------------------------------------------------------------------------

gltf_CSysConverter&
  gltfWriter::ChangeCoordinateSystemConverter()
{
  return m_CSTrsf;
}

//-----------------------------------------------------------------------------

void gltfWriter::SetCoordinateSystemConverter(const gltf_CSysConverter& converter)
{
  m_CSTrsf = converter;
}

//-----------------------------------------------------------------------------

bool gltfWriter::IsBinary() const
{
  return m_bIsBinary;
}

//-----------------------------------------------------------------------------

gltf_WriterTrsfFormat
  gltfWriter::TransformationFormat() const
{
  return m_trsfFormat;
}

//-----------------------------------------------------------------------------

void gltfWriter::SetTransformationFormat(const gltf_WriterTrsfFormat fmt)
{
  m_trsfFormat = fmt;
}

//-----------------------------------------------------------------------------

bool gltfWriter::IsForcedUVExport() const
{
  return m_bIsForcedUVExport;
}

//-----------------------------------------------------------------------------

void gltfWriter::SetForcedUVExport(const bool toForce)
{
  m_bIsForcedUVExport = toForce;
}

//-----------------------------------------------------------------------------

const gltf_XdeVisualStyle&
  gltfWriter::DefaultStyle() const
{
  return m_defaultStyle;
}

//-----------------------------------------------------------------------------

void gltfWriter::SetDefaultStyle(const gltf_XdeVisualStyle& style)
{
  m_defaultStyle = style;
}

//-----------------------------------------------------------------------------

bool gltfWriter::writeBinData(const Handle(TDocStd_Document)& doc,
                              const TDF_LabelSequence&        rootLabs,
                              const TColStd_MapOfAsciiString* labFilter)
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

  m_binDataMap.Clear();
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
  for ( XCAFPrs_DocumentExplorer docExp(doc, rootLabs, XCAFPrs_DocumentExplorerFlags_OnlyLeafNodes);
        docExp.More(); docExp.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = docExp.Current();
    //
    if ( labFilter != nullptr && !labFilter->Contains(docNode.Id) )
    {
      continue;
    }

    // transformation will be stored at scene nodes
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( m_binDataMap.IsBound( faceIter.Face() ) || this->toSkipFaceMesh(faceIter) )
      {
        continue;
      }

      gltf_Face gltfFace;
      this->saveNodes(gltfFace, binFile, faceIter, nbAccessors);

      if ( !binFile.good() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
        return false;
      }

      m_binDataMap.Bind(faceIter.Face(), gltfFace);
    }
  }
  m_buffViewPos.ByteLength = (int64_t) binFile.tellp() - m_buffViewPos.ByteOffset;

  /* ===============
   *  Write normals.
   * =============== */

  m_buffViewNorm.ByteOffset = binFile.tellp();
  //
  for ( XCAFPrs_DocumentExplorer docExp(doc, rootLabs, XCAFPrs_DocumentExplorerFlags_OnlyLeafNodes);
        docExp.More(); docExp.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = docExp.Current();
    //
    if ( labFilter != nullptr && !labFilter->Contains(docNode.Id) )
    {
      continue;
    }
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( this->toSkipFaceMesh(faceIter) )
      {
        continue;
      }

      gltf_Face& gltfFace = m_binDataMap.ChangeFind( faceIter.Face() );
      //
      if ( gltfFace.NodeNorm.Id != gltf_Accessor::INVALID_ID )
      {
        continue;
      }

      this->saveNormals(gltfFace, binFile, faceIter, nbAccessors);

      if ( !binFile.good() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
        return false;
      }
    }
  }
  m_buffViewNorm.ByteLength = (int64_t) binFile.tellp() - m_buffViewNorm.ByteOffset;

  /* ===========================
   *  Write texture coordinates.
   * =========================== */

  m_buffViewTextCoord.ByteOffset = binFile.tellp();
  //
  for ( XCAFPrs_DocumentExplorer docExp(doc, rootLabs, XCAFPrs_DocumentExplorerFlags_OnlyLeafNodes);
        docExp.More(); docExp.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = docExp.Current();
    //
    if ( labFilter != nullptr && !labFilter->Contains(docNode.Id) )
    {
      continue;
    }

    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( this->toSkipFaceMesh(faceIter) )
      {
        continue;
      }

      gltf_Face& gltfFace = m_binDataMap.ChangeFind( faceIter.Face() );
      //
      if ( gltfFace.NodeUV.Id != gltf_Accessor::INVALID_ID )
      {
        continue;
      }

      this->saveTextCoords(gltfFace, binFile, faceIter, nbAccessors);

      if ( !binFile.good() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
        return false;
      }
    }
  }
  m_buffViewTextCoord.ByteLength = (int64_t) binFile.tellp() - m_buffViewTextCoord.ByteOffset;

  /* ===============
   *  Write indices.
   * =============== */

  m_buffViewInd.ByteOffset = binFile.tellp();
  for ( XCAFPrs_DocumentExplorer docExp(doc, rootLabs, XCAFPrs_DocumentExplorerFlags_OnlyLeafNodes);
        docExp.More(); docExp.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = docExp.Current();
    //
    if ( labFilter != nullptr && !labFilter->Contains (docNode.Id) )
    {
      continue;
    }

    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( this->toSkipFaceMesh(faceIter) )
      {
        continue;
      }

      gltf_Face& gltfFace = m_binDataMap.ChangeFind( faceIter.Face() );
      //
      if ( gltfFace.Indices.Id != gltf_Accessor::INVALID_ID )
      {
        continue;
      }

      this->saveIndices(gltfFace, binFile, faceIter, nbAccessors);

      if ( !binFile.good() )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "File '%1' cannot be written." << m_binFilenameFull);
        return false;
      }
    }
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

bool gltfWriter::writeJson(const Handle(TDocStd_Document)&             doc,
                           const TDF_LabelSequence&                    rootLabs,
                           const TColStd_MapOfAsciiString*             labFilter,
                           const TColStd_IndexedDataMapOfStringString& fileInfo)
{
#if defined USE_RAPIDJSON
  m_jsonWriter.reset();

  const int binDatbufferId = 0;
  const int defSamplerId    = 0;
  const int defSceneId      = 0;

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

  // Prepare an indexed map of scene nodes (without assemblies) in correct order.
  // Note: this is also order of meshes in glTF document array.
  gltf_SceneNodeMap scNodeMap;
  for ( XCAFPrs_DocumentExplorer docExp(doc, rootLabs, XCAFPrs_DocumentExplorerFlags_OnlyLeafNodes);
        docExp.More(); docExp.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = docExp.Current();
    //
    if ( labFilter != nullptr && !labFilter->Contains(docNode.Id) )
    {
      continue;
    }

    bool hasMeshData = false;
    if ( !docNode.IsAssembly )
    {
      for ( gltf_FaceIterator fit(docNode.RefLabel,
                                  TopLoc_Location(),
                                  true,
                                  docNode.Style);
            fit.More(); fit.Next() )
      {
        if ( !this->toSkipFaceMesh(fit) )
        {
          hasMeshData = true;
          break;
        }
      }
    }
    if ( hasMeshData )
    {
      scNodeMap.Add(docNode);
    }
    else
    {
      // glTF does not permit empty meshes / primitive arrays.
      m_progress.SendLogMessage( LogWarn(Normal) << "gltf_XdeWriter skips node '%1' without meshes."
                                                 << ::readNameAttribute(m_shapeTool, docNode.RefLabel) );
    }
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
    this->writeAccessors   (scNodeMap);
    this->writeAnimations  ();
    this->writeAsset       (fileInfo);
    this->writeBufferViews (binDatbufferId);
    this->writeBuffers     ();
    this->writeExtensions  ();
    this->writeImages      (scNodeMap, materialMap);
    this->writeMaterials   (scNodeMap, materialMap);
    this->writeMeshes      (scNodeMap, materialMap);
    this->writeNodes       (doc, rootLabs, labFilter, scNodeMap, scRootIds);
    this->writeSamplers    (materialMap);
    this->writeScene       (defSceneId);
    this->writeScenes      (scRootIds);
    this->writeSkins       ();
    this->writeTextures    (scNodeMap, materialMap);
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
  (void) doc;
  (void) rootLabs;
  (void) labFilter;
  (void) fileInfo;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
  return false;
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeAccessors(const gltf_SceneNodeMap& scNodeMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeAccessors()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Accessors) );
  m_jsonWriter->StartArray();

  /* =================
   *  Write positions.
   * ================= */

  NCollection_Map<TopoDS_Shape, TopTools_ShapeMapHasher> writtenFaces;
  //
  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( !writtenFaces.Add( faceIter.Face() ) // skip repeated
         || this->toSkipFaceMesh(faceIter) )    // ... or empty faces
      {
        continue;
      }

      const gltf_Face& gltfFace = m_binDataMap.Find( faceIter.Face() );
      //
      this->writePositions(gltfFace);
    }
  }

  /* ===============
   *  Write normals.
   * =============== */

  writtenFaces.Clear();
  //
  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( !writtenFaces.Add(faceIter.Face() ) // skip repeated
         || this->toSkipFaceMesh(faceIter) )   // ... or empty faces
      {
        continue;
      }

      const gltf_Face& gltfFace = m_binDataMap.Find( faceIter.Face() );
      //
      this->writeNormals(gltfFace);
    }
  }

  /* ===========================
   *  Write texture coordinates.
   * =========================== */

  writtenFaces.Clear();
  //
  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( !writtenFaces.Add( faceIter.Face() ) // skip repeated
         || this->toSkipFaceMesh(faceIter) )    // ... or empty faces
      {
        continue;
      }

      const gltf_Face& gltfFace = m_binDataMap.Find( faceIter.Face() );
      //
      this->writeTextCoords(gltfFace);
    }
  }

  /* ===============
   *  Write indices.
   * =============== */

  writtenFaces.Clear();
  //
  for ( gltf_SceneNodeMap::Iterator snIt (scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      if ( !writtenFaces.Add( faceIter.Face() ) // skip repeated
         || this->toSkipFaceMesh(faceIter) )    // ... or empty faces
      {
        continue;
      }

      const gltf_Face& gltfFace = m_binDataMap.Find(faceIter.Face());
      //
      this->writeIndices(gltfFace);
    }
  }

  m_jsonWriter->EndArray();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) scNodeMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writePositions(const gltf_Face& gltfFace)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writePositions()");

  if ( gltfFace.NodePos.Id == gltf_Accessor::INVALID_ID )
  {
    return;
  }

  m_jsonWriter->StartObject();
  m_jsonWriter->Key   ("bufferView");
  m_jsonWriter->Int   (m_buffViewPos.Id);
  m_jsonWriter->Key   ("byteOffset");
  m_jsonWriter->Int64 (gltfFace.NodePos.ByteOffset);
  m_jsonWriter->Key   ("componentType");
  m_jsonWriter->Int   (gltfFace.NodePos.ComponentType);
  m_jsonWriter->Key   ("count");
  m_jsonWriter->Int64 (gltfFace.NodePos.Count);

  if ( gltfFace.NodePos.BndBox.IsValid() )
  {
    m_jsonWriter->Key ("max");
    m_jsonWriter->StartArray();
    m_jsonWriter->Double ( gltfFace.NodePos.BndBox.CornerMax().x() );
    m_jsonWriter->Double ( gltfFace.NodePos.BndBox.CornerMax().y() );
    m_jsonWriter->Double ( gltfFace.NodePos.BndBox.CornerMax().z() );
    m_jsonWriter->EndArray();

    m_jsonWriter->Key("min");
    m_jsonWriter->StartArray();
    m_jsonWriter->Double ( gltfFace.NodePos.BndBox.CornerMin().x() );
    m_jsonWriter->Double ( gltfFace.NodePos.BndBox.CornerMin().y() );
    m_jsonWriter->Double ( gltfFace.NodePos.BndBox.CornerMin().z() );
    m_jsonWriter->EndArray();
  }
  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("VEC3");

  m_jsonWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) gltfFace;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeNormals(const gltf_Face& gltfFace)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeNormals()");

  if ( gltfFace.NodeNorm.Id == gltf_Accessor::INVALID_ID )
  {
    return;
  }

  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("bufferView");
  m_jsonWriter->Int    (m_buffViewNorm.Id);
  m_jsonWriter->Key    ("byteOffset");
  m_jsonWriter->Int64  (gltfFace.NodeNorm.ByteOffset);
  m_jsonWriter->Key    ("componentType");
  m_jsonWriter->Int    (gltfFace.NodeNorm.ComponentType);
  m_jsonWriter->Key    ("count");
  m_jsonWriter->Int64  (gltfFace.NodeNorm.Count);

  /* min/max values are optional, and not very useful for normals - skip them */

  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("VEC3");

  m_jsonWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) gltfFace;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeTextCoords(const gltf_Face& gltfFace)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeTextCoords()");

  if ( gltfFace.NodeUV.Id == gltf_Accessor::INVALID_ID )
  {
    return;
  }

  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("bufferView");
  m_jsonWriter->Int    (m_buffViewTextCoord.Id);
  m_jsonWriter->Key    ("byteOffset");
  m_jsonWriter->Int64  (gltfFace.NodeUV.ByteOffset);
  m_jsonWriter->Key    ("componentType");
  m_jsonWriter->Int    (gltfFace.NodeUV.ComponentType);
  m_jsonWriter->Key    ("count");
  m_jsonWriter->Int64  (gltfFace.NodeUV.Count);

  /* min/max values are optional, and not very useful for UV coordinates - skip them */

  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("VEC2");

  m_jsonWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) gltfFace;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeIndices (const gltf_Face& gltfFace)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeIndices()");

  if ( gltfFace.Indices.Id == gltf_Accessor::INVALID_ID )
  {
    return;
  }

  m_jsonWriter->StartObject();
  m_jsonWriter->Key    ("bufferView");
  m_jsonWriter->Int    (m_buffViewInd.Id);
  m_jsonWriter->Key    ("byteOffset");
  m_jsonWriter->Int64  (gltfFace.Indices.ByteOffset);
  m_jsonWriter->Key    ("componentType");
  m_jsonWriter->Int    (gltfFace.Indices.ComponentType);
  m_jsonWriter->Key    ("count");
  m_jsonWriter->Int64  (gltfFace.Indices.Count);

  m_jsonWriter->Key    ("type");
  m_jsonWriter->String ("SCALAR");

  m_jsonWriter->EndObject();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) gltfFace;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeAnimations()
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void gltfWriter::writeAsset(const TColStd_IndexedDataMapOfStringString& fileInfo)
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

void gltfWriter::writeBufferViews(const int binDataBufferId)
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

void gltfWriter::writeBuffers()
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

void gltfWriter::writeExtensions()
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void gltfWriter::writeImages(const gltf_SceneNodeMap& scNodeMap,
                             gltf_MaterialMap&        materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeImages()");

  // empty section should NOT be written to avoid validator errors
  bool isStarted = false;
  //
  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      materialMap.AddImages(m_jsonWriter.get(), faceIter.FaceStyle(), isStarted);
    }
  }
  if ( isStarted )
  {
    m_jsonWriter->EndArray();
  }
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) scNodeMap;
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeMaterials(const gltf_SceneNodeMap& scNodeMap,
                                gltf_MaterialMap&        materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeMaterials()");

  // empty section should NOT be written to avoid validator errors
  bool isStarted = false;
  //
  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      materialMap.AddMaterial(m_jsonWriter.get(), faceIter.FaceStyle(), isStarted);
    }
  }
  if ( isStarted )
  {
    m_jsonWriter->EndArray();
  }
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) scNodeMap;
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeMeshes(const gltf_SceneNodeMap& scNodeMap,
                             const gltf_MaterialMap&  materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeMeshes()");

  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Meshes) );
  m_jsonWriter->StartArray();

  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode&   docNode  = snIt.Value();
    const TCollection_AsciiString nodeName = ::readNameAttribute(m_shapeTool, docNode.RefLabel);

    bool toStartPrims  = true;
    int  nbFacesInNode = 0;
    //
    for ( gltf_FaceIterator fit(docNode.RefLabel,
                                TopLoc_Location(),
                                true,
                                docNode.Style);
          fit.More();
          fit.Next(), ++nbFacesInNode)
    {
      if ( this->toSkipFaceMesh(fit) )
      {
        continue;
      }

      if ( toStartPrims )
      {
        toStartPrims = false;
        m_jsonWriter->StartObject();
        m_jsonWriter->Key("name");
        m_jsonWriter->String( nodeName.ToCString() );
        m_jsonWriter->Key("primitives");
        m_jsonWriter->StartArray();
      }

      const gltf_Face&              gltfFace = m_binDataMap.Find( fit.Face() );
      const TCollection_AsciiString matId    = materialMap.FindMaterial( fit.FaceStyle() );

      m_jsonWriter->StartObject();
      {
        m_jsonWriter->Key("attributes");
        m_jsonWriter->StartObject();
        {
          if ( gltfFace.NodeNorm.Id != gltf_Accessor::INVALID_ID )
          {
            m_jsonWriter->Key("NORMAL");
            m_jsonWriter->Int(gltfFace.NodeNorm.Id);
          }
          m_jsonWriter->Key("POSITION");
          m_jsonWriter->Int(gltfFace.NodePos.Id);

          if ( gltfFace.NodeUV.Id != gltf_Accessor::INVALID_ID )
          {
            m_jsonWriter->Key("TEXCOORD_0");
            m_jsonWriter->Int(gltfFace.NodeUV.Id);
          }
        }
        m_jsonWriter->EndObject();

        m_jsonWriter->Key("indices");
        m_jsonWriter->Int(gltfFace.Indices.Id);

        if ( !matId.IsEmpty() )
        {
          m_jsonWriter->Key("material");
          m_jsonWriter->Int( matId.IntegerValue() );
        }
        m_jsonWriter->Key("mode");
        m_jsonWriter->Int(gltf_PrimitiveMode_Triangles);
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
  (void) scNodeMap;
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeNodes(const Handle(TDocStd_Document)& doc,
                            const TDF_LabelSequence&        rootLabs,
                            const TColStd_MapOfAsciiString* labFilter,
                            const gltf_SceneNodeMap&        scNodeMap,
                            NCollection_Sequence<int>&      scRootIds)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeNodes()");

  // Prepare full indexed map of scene nodes in correct order.
  gltf_SceneNodeMap scNodeMapWithChildren; // indexes starting from 1
  //
  for ( XCAFPrs_DocumentExplorer docExp(doc, rootLabs, XCAFPrs_DocumentExplorerFlags_None);
        docExp.More(); docExp.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = docExp.Current();
    //
    if ( (labFilter != nullptr) && !labFilter->Contains(docNode.Id) )
    {
      continue;
    }

    int nodeIdx = scNodeMapWithChildren.Add(docNode);
    if ( docExp.CurrentDepth() == 0 )
    {
      // save root node index (starting from 0 not 1)
      scRootIds.Append(nodeIdx - 1);
    }
  }

  // Write scene nodes using prepared map for correct order of array members
  m_jsonWriter->Key( gltf_RootElementName(gltf_RootElement_Nodes) );
  m_jsonWriter->StartArray();

  for ( gltf_SceneNodeMap::Iterator snIt (scNodeMapWithChildren); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();

    m_jsonWriter->StartObject();
    {
      if ( docNode.IsAssembly )
      {
        m_jsonWriter->Key("children");
        m_jsonWriter->StartArray();
        {
          for ( TDF_ChildIterator childIt(docNode.RefLabel); childIt.More(); childIt.Next() )
          {
            const TDF_Label& childLab = childIt.Value();
            if ( childLab.IsNull() )
            {
              continue;
            }

            const TCollection_AsciiString
              childId = XCAFPrs_DocumentExplorer::DefineChildId(childLab, docNode.Id);
            //
            int childIdx = scNodeMapWithChildren.FindIndex(childId);

            if ( childIdx > 0 )
            {
              m_jsonWriter->Int(childIdx - 1);
            }
          }
        }
        m_jsonWriter->EndArray();
      }
    }
    if ( !docNode.LocalTrsf.IsIdentity() )
    {
      gp_Trsf trsf = docNode.LocalTrsf.Transformation();

      if ( trsf.Form() != gp_Identity )
      {
        m_CSTrsf.TransformTransformation(trsf);

        const gp_Quaternion qn = trsf.GetRotation();
        const bool hasRotation = Abs( qn.X() )       > gp::Resolution()
                              || Abs( qn.Y() )       > gp::Resolution()
                              || Abs( qn.Z() )       > gp::Resolution()
                              || Abs( qn.W() - 1.0 ) > gp::Resolution();
        //
        const double  scaleFactor    = trsf.ScaleFactor();
        const bool    hasScale       = Abs(scaleFactor - 1.0) > Precision::Confusion();
        const gp_XYZ& translPart     = trsf.TranslationPart();
        const bool    hasTranslation = translPart.SquareModulus() > gp::Resolution();

        gltf_WriterTrsfFormat trsfFormat = m_trsfFormat;
        if ( m_trsfFormat == gltf_WriterTrsfFormat_Compact )
        {
          trsfFormat = hasRotation && hasScale && hasTranslation
                      ? gltf_WriterTrsfFormat_Mat4
                      : gltf_WriterTrsfFormat_TRS;
        }

        if ( trsfFormat == gltf_WriterTrsfFormat_Mat4 )
        {
          // write full matrix
          Graphic3d_Mat4 mat4;
          trsf.GetMat4(mat4);
          //
          if ( !mat4.IsIdentity() )
          {
            m_jsonWriter->Key("matrix");
            m_jsonWriter->StartArray();
            //
            for ( int icol = 0; icol < 4; ++icol )
            {
              for ( int irow = 0; irow < 4; ++irow )
              {
                m_jsonWriter->Double( mat4.GetValue(irow, icol) );
              }
            }
            //
            m_jsonWriter->EndArray();
          }
        }
        else // TRS
        {
          if ( hasRotation )
          {
            m_jsonWriter->Key("rotation");
            m_jsonWriter->StartArray();
            m_jsonWriter->Double( qn.X() );
            m_jsonWriter->Double( qn.Y() );
            m_jsonWriter->Double( qn.Z() );
            m_jsonWriter->Double( qn.W() );
            m_jsonWriter->EndArray();
          }
          if ( hasScale )
          {
            m_jsonWriter->Key("scale");
            m_jsonWriter->StartArray();
            m_jsonWriter->Double(scaleFactor);
            m_jsonWriter->Double(scaleFactor);
            m_jsonWriter->Double(scaleFactor);
            m_jsonWriter->EndArray();
          }
          if ( hasTranslation )
          {
            m_jsonWriter->Key("translation");
            m_jsonWriter->StartArray();
            m_jsonWriter->Double( translPart.X() );
            m_jsonWriter->Double( translPart.Y() );
            m_jsonWriter->Double( translPart.Z() );
            m_jsonWriter->EndArray();
          }
        }
      }
    }
    if ( !docNode.IsAssembly )
    {
      // Mesh order of current node is equal to order of this node in scene nodes map
      int meshIdx = scNodeMap.FindIndex(docNode.Id);
      if ( meshIdx > 0 )
      {
        m_jsonWriter->Key("mesh");
        m_jsonWriter->Int(meshIdx - 1);
      }
    }
    {
      TCollection_AsciiString nodeName = readNameAttribute(m_shapeTool, docNode.Label);
      if ( nodeName.IsEmpty() )
      {
        nodeName = readNameAttribute(m_shapeTool, docNode.RefLabel);
      }
      m_jsonWriter->Key("name");
      m_jsonWriter->String( nodeName.ToCString() );
    }
    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) doc;
  (void) rootLabs;
  (void) labFilter;
  (void) scNodeMap;
  (void) scRootIds;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeSamplers(const gltf_MaterialMap& materialMap)
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

void gltfWriter::writeScene(const int defSceneId)
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

void gltfWriter::writeScenes(const NCollection_Sequence<int>& scRootIds)
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
    for ( NCollection_Sequence<int>::Iterator rit(scRootIds); rit.More(); rit.Next() )
    {
      m_jsonWriter->Int( rit.Value() );
    }
    //
    m_jsonWriter->EndArray();
    m_jsonWriter->EndObject();
  }
  m_jsonWriter->EndArray();
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) scRootIds;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}

//-----------------------------------------------------------------------------

void gltfWriter::writeSkins()
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void gltfWriter::writeTextures(const gltf_SceneNodeMap& scNodeMap,
                               gltf_MaterialMap&        materialMap)
{
#if defined USE_RAPIDJSON
  Standard_ProgramError_Raise_if(m_jsonWriter.get() == nullptr, "Internal error: gltf_XdeWriter::writeTextures()");

  // empty section should not be written to avoid validator errors
  bool isStarted = false;
  //
  for ( gltf_SceneNodeMap::Iterator snIt(scNodeMap); snIt.More(); snIt.Next() )
  {
    const XCAFPrs_DocumentNode& docNode = snIt.Value();
    //
    for ( gltf_FaceIterator faceIter(docNode.RefLabel, TopLoc_Location(), true, docNode.Style);
          faceIter.More(); faceIter.Next() )
    {
      materialMap.AddTextures( m_jsonWriter.get(), faceIter.FaceStyle(), isStarted );
    }
  }
  if ( isStarted )
  {
    m_jsonWriter->EndArray();
  }
#else
  // Suppress `unreferenced formal parameter` warning (C4100).
  (void) scNodeMap;
  (void) materialMap;

  m_progress.SendLogMessage(LogErr(High) << "glTF export is impossible: you have to build "
                                            "Analysis Situs with rapidjson.");
#endif
}
