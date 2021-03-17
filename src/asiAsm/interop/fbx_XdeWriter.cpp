//-----------------------------------------------------------------------------
// Created on: 06 March 2021
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Own include
#include <fbx_XdeWriter.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <gp_Quaternion.hxx>
#include <Interface_Static.hxx>
#include <Poly.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TopExp_Explorer.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#if defined FBXSDK_SHARED
  // FBX SDK includes
  #include <fbxsdk.h>
#endif

//-----------------------------------------------------------------------------

using namespace asiAsm;

//-----------------------------------------------------------------------------

struct t_fbxState
{
#if defined FBXSDK_SHARED
  typedef NCollection_DataMap<FbxMesh*, NCollection_Vector<FbxSurfacePhong*> > FbxMeshFbxSurfacePhongMap;
  typedef NCollection_DataMap<TopoDS_Shape, FbxMesh*>                          ShapeFbxMeshMap;
  typedef NCollection_DataMap<TDF_Label, FbxMesh*, TDF_LabelMapHasher>         LabelFbxMeshMap;
  typedef NCollection_DataMap<TDF_Label, FbxSurfacePhong*, TDF_LabelMapHasher> LabelFbxSurfacePhongMap;
  typedef NCollection_DataMap<FbxMesh*, TDF_Label>                             FbxMeshLabelMap;

  FbxScene*                 fbxScene;
  ShapeFbxMeshMap           shapeFbxMeshMap;           //!< Map which stores converted meshes for shape with facets.
  LabelFbxMeshMap           labelFbxMeshMap;           //!< Map which stores converted meshes for label with mesh.
  LabelFbxSurfacePhongMap   labelFbxSurfacePhongMap;   //!< Map which stores converted color for label with color.
  FbxMeshFbxSurfacePhongMap fbxMeshFbxSurfacePhongMap; //!< Map which stores vector of fbx colors for fbx mesh.

  t_fbxState() : fbxScene(nullptr) {} //!< Default ctor.

  void ClearState()
  {
    shapeFbxMeshMap.Clear();
    labelFbxMeshMap.Clear();
    labelFbxSurfacePhongMap.Clear();
    fbxMeshFbxSurfacePhongMap.Clear();
  }
#endif
};

//-----------------------------------------------------------------------------

#if defined FBXSDK_SHARED
namespace
{
  TCollection_AsciiString getNameFromLabel(const TDF_Label& label)
  {
    TCollection_AsciiString str;
    Handle(TDataStd_Name) nodeNameAttr;
    //
    if ( label.FindAttribute(TDataStd_Name::GetID(), nodeNameAttr) )
    {
      str = TCollection_AsciiString( nodeNameAttr->Get() );
    }

    return str;
  }

  int addMaterialToNode(FbxNode*         fbxNode,
                        FbxSurfacePhong* fbxSurfacePhong)
  {
    const int materialsCount = fbxNode->GetMaterialCount();

    for ( int index = 0; index < materialsCount; ++index )
    {
      if ( fbxNode->GetMaterial(index)->GetUniqueID() == fbxSurfacePhong->GetUniqueID() )
      {
        return index;
      }
    }

    return fbxNode->AddMaterial(fbxSurfacePhong);
  }

  bool convertTriangulation(FbxMesh*                          fbxMesh,
                            const Handle(Poly_Triangulation)& triangulation,
                            const bool                        storeNormals,
                            const bool                        /*storeUVNodes*/)
  {
    const int nbNodes     = triangulation->NbNodes();
    const int nbTriangles = triangulation->NbTriangles();

    // Initialize the control point array of the mesh.
    fbxMesh->InitControlPoints(nbNodes);
    FbxVector4* fbxControlPoints = fbxMesh->GetControlPoints();
    //
    for ( int nodeIndex = 0; nodeIndex < nbNodes; ++nodeIndex )
    {
      gp_Pnt nodePnt = triangulation->Node(nodeIndex + 1);
      fbxControlPoints[nodeIndex] = FbxVector4( nodePnt.X(), nodePnt.Y(), nodePnt.Z() );
    }

    // Define each triangle of the mesh.
    for ( int itri = 1; itri <= nbTriangles; ++itri )
    {
      int n1, n2, n3;
      triangulation->Triangle(itri).Get(n1, n2, n3);

      // Create polygons. Assign texture.
      // All faces of the mesh have the same texture.
      fbxMesh->BeginPolygon (-1, -1, -1, false);
      fbxMesh->AddPolygon   (n1 - 1);
      fbxMesh->AddPolygon   (n2 - 1);
      fbxMesh->AddPolygon   (n3 - 1);
      fbxMesh->EndPolygon   ();
    }

    // Store normals.
    if ( storeNormals && triangulation->HasNormals() )
    {
      const TShort_Array1OfShortReal& normalsArray = triangulation->Normals();

      FbxGeometryElementNormal*
        fbxNormalElement = fbxMesh->CreateElementNormal();
      //
      fbxNormalElement->SetMappingMode(FbxLayerElement::eByControlPoint);
      fbxNormalElement->SetReferenceMode(FbxLayerElement::eDirect);
      fbxNormalElement->GetDirectArray().SetCount( fbxMesh->GetControlPointsCount() );
      //
      for ( int nodeIndex = 1; nodeIndex <= nbNodes; ++nodeIndex )
      {
        FbxVector4 normal( normalsArray(nodeIndex * 3 - 2),
                           normalsArray(nodeIndex * 3 - 1),
                           normalsArray(nodeIndex * 3 - 0) );
        fbxNormalElement->GetDirectArray().SetAt(nodeIndex - 1, normal);
      }
    }

    // Store UV Nodes.
    if ( triangulation->HasUVNodes())
    {
      const TColgp_Array1OfPnt2d& uvNodes = triangulation->UVNodes();

      FbxGeometryElementUV* fbxUVElement = fbxMesh->CreateElementUV("UV");
      fbxUVElement->SetMappingMode(FbxLayerElement::eByControlPoint);
      fbxUVElement->SetReferenceMode(FbxLayerElement::eDirect);
      fbxUVElement->GetDirectArray().SetCount(fbxMesh->GetControlPointsCount());

      for ( int nodeIndex = 1; nodeIndex <= nbNodes; ++nodeIndex )
      {
        FbxVector2 uvNode( uvNodes.Value(nodeIndex).X(),
                           uvNodes.Value(nodeIndex).Y() );
        fbxUVElement->GetDirectArray().SetAt(nodeIndex - 1, uvNode);
      }
    }

    return fbxMesh != nullptr;
  }

  bool getMaterialForShape(const Handle(asiAsm_XdeDoc)& doc,
                           FbxSurfacePhong*&            fbxSurfacePhong,
                           const TDF_Label&             shapeLabel,
                           t_fbxState*                  pState)
  {
    TDF_Label colorLabel;
    Handle(XCAFDoc_ColorTool) colorTool = doc->GetColorTool();

    bool isColorFound = colorTool->GetColor(shapeLabel, XCAFDoc_ColorSurf, colorLabel);
    //
    if ( !isColorFound )
      isColorFound = colorTool->GetColor(shapeLabel, XCAFDoc_ColorGen, colorLabel);

    if ( !isColorFound )
    {
      return false;
    }

    // Check if there is already a converted color for the current color label.
    FbxSurfacePhong* const*
      phongPtr = pState->labelFbxSurfacePhongMap.Seek(colorLabel);
    //
    if ( phongPtr != nullptr )
    {
      // Get already a converted color for the current color label.
      fbxSurfacePhong = (*phongPtr);
    }
    else
    {
      // Get color from label.
      Quantity_ColorRGBA color;
      colorTool->GetColor(colorLabel, color);

      TCollection_AsciiString name = getNameFromLabel(colorLabel);

      // Create material
      fbxSurfacePhong = FbxSurfacePhong::Create( pState->fbxScene, name.ToCString() );

      // Set its diffuse color
      fbxSurfacePhong->Diffuse.Set(FbxDouble3(color.GetRGB().Red(), color.GetRGB().Green(), color.GetRGB().Blue()));
      fbxSurfacePhong->TransparencyFactor.Set( 1 - color.Alpha() );

      pState->labelFbxSurfacePhongMap.Bind(colorLabel, fbxSurfacePhong);
    }

    return fbxSurfacePhong != nullptr;
  }

  void appendColor(const Handle(asiAsm_XdeDoc)&          doc,
                   FbxNode*                              fbxNode,
                   FbxMesh*                              fbxMesh,
                   const TDF_Label&                      label,
                   const asiAlgo_MeshMerge::t_faceElems& faceMeshIndicesMap,
                   t_fbxState*                           pState)
  {
    if ( faceMeshIndicesMap.IsEmpty() )
      return;

    FbxSurfacePhong* fbxSurfacePhong = nullptr;
    getMaterialForShape(doc, fbxSurfacePhong, label, pState);
    //
    if ( fbxSurfacePhong == nullptr )
      return;

    // Set material mapping.
    FbxGeometryElementMaterial* fbxMaterialElement;
    if ( fbxMesh->GetElementMaterialCount() == 0 )
    {
      fbxMaterialElement = fbxMesh->CreateElementMaterial();
      fbxMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
      fbxMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

      // We need indices for each polygon.
      fbxMaterialElement->GetIndexArray().SetCount( fbxMesh->GetPolygonCount() );
    }
    else
    {
      fbxMaterialElement = fbxMesh->GetElementMaterial();
    }

    int materialIndex = addMaterialToNode(fbxNode, fbxSurfacePhong);
    NCollection_Vector<FbxSurfacePhong*>*
      mapPtr = pState->fbxMeshFbxSurfacePhongMap.ChangeSeek(fbxMesh);
    //
    if ( mapPtr != nullptr )
    {
      if ( (materialIndex + 1) > (*mapPtr).Size() )
      {
        (*mapPtr).Append(fbxSurfacePhong);
      }
    }
    else
    {
      NCollection_Vector<FbxSurfacePhong*> surfVec;
      surfVec.Append(fbxSurfacePhong);
      pState->fbxMeshFbxSurfacePhongMap.Bind(fbxMesh, surfVec);
    }

    TopoDS_Shape shape;
    doc->GetShapeTool()->GetShape(label, shape);
    //
    for ( TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next() )
    {
      const TopoDS_Face&             face      = TopoDS::Face( exp.Current() );
      const NCollection_Vector<int>* vectorPtr = faceMeshIndicesMap.Seek(face);
      //
      if ( vectorPtr != nullptr )
      {
        const NCollection_Vector<int>& indices = (*vectorPtr);
        for ( int i = 0; i < indices.Size(); ++i )
        {
          fbxMaterialElement->GetIndexArray().SetAt(indices.Value(i) - 1, materialIndex);
        }
      }
    }
  }

  bool exportFacets(const Handle(asiAsm_XdeDoc)& doc,
                    FbxNode*                     fbxParentNode,
                    const TDF_Label&             label,
                    const TopoDS_Shape&          shape,
                    t_fbxState*                  pState)
  {
    bool isOk = false;

    // If shape is compound recursively iterate over subshapes and store them on
    // separate nodes. This is to avoid problems with visualization of resulting
    // *.fbx. If we store triangulations for shapes from compound in a single
    // FbxMesh then some viewers (e.g. FBX Review) corrupts presentation due to
    // overlays in the mesh.
    if ( shape.ShapeType() == TopAbs_COMPOUND )
    {
      for ( TopoDS_Iterator it(shape); it.More(); it.Next() )
      {
        const TopoDS_Shape& subShape           = it.Value();
        std::string         shapeType          = asiAlgo_Utils::ShapeTypeStr(subShape);
        FbxNode*            fbxChildNode       = FbxNode::Create( pState->fbxScene, shapeType.c_str() );
        bool                isSubShapeExported = exportFacets(doc, fbxChildNode, label, subShape, pState);

        if ( isSubShapeExported )
        {
          fbxParentNode->AddChild(fbxChildNode);
          isOk = true;
        }
        else
        {
          fbxChildNode->Destroy();
        }
      }

      return isOk;
    }

    FbxMesh*        fbxMesh    = nullptr;
    FbxMesh* const* fbxMeshPtr = pState->shapeFbxMeshMap.Seek(shape);
    //
    if ( fbxMeshPtr != nullptr )
    {
      // Get already a converted mesh for the current shape.
      fbxMesh = (*fbxMeshPtr);

      // Append colors to node.
      const NCollection_Vector<FbxSurfacePhong*>*
        vectorPtr = pState->fbxMeshFbxSurfacePhongMap.Seek(fbxMesh);
      //
      if ( vectorPtr != nullptr )
      {
        const NCollection_Vector<FbxSurfacePhong*> surfVec = (*vectorPtr);

        for ( int i = 0; i < surfVec.Size(); ++i )
        {
          addMaterialToNode( fbxParentNode, surfVec.Value(i) );
        }
      }
    }
    else
    {
      asiAlgo_MeshMerge::t_faceElems faces2facets;

      // Generate mesh from facets.
      Handle(Poly_Triangulation)
        triangulation = asiAlgo_MeshMerge::PutTogether(shape, faces2facets);
      //
      if ( triangulation.IsNull() )
        return false;

      // Get name from shape type.
      std::string name = asiAlgo_Utils::ShapeTypeStr(shape);

      // Create a mesh.
      fbxMesh = FbxMesh::Create( pState->fbxScene, name.c_str() );

      // Convert triangulation to FBX mesh.
      convertTriangulation(fbxMesh, triangulation, true, true);

      // Set colors to mesh.
      // Firstly append color to whole shape.
      appendColor(doc, fbxParentNode, fbxMesh, label, faces2facets, pState);

      // Append colors for subshapes.
      for ( TDF_ChildIterator childIter(label); childIter.More(); childIter.Next() )
      {
        TDF_Label childLabel = childIter.Value();
        appendColor(doc, fbxParentNode, fbxMesh, childLabel, faces2facets, pState);
      }

      // Cache converted mesh.
      pState->shapeFbxMeshMap.Bind(shape, fbxMesh);
    }

    if ( fbxMesh == nullptr )
      return false;

    fbxParentNode->SetNodeAttribute(fbxMesh);

    return true;
  }

  void appendNode(const Handle(asiAsm_XdeDoc)& doc,
                  FbxNode*                     fbxParentNode,
                  const TDF_Label&             label,
                  t_fbxState*                  pState,
                  ActAPI_ProgressEntry         progress)
  {
    if ( !fbxParentNode || label.IsNull() )
      return;

    if ( progress.IsCancelling() )
      return;

    // Create a node for passed label in the scene.
    TCollection_AsciiString nodeNameStr = getNameFromLabel(label);
    FbxNode*
      fbxChildNode = FbxNode::Create( pState->fbxScene, nodeNameStr.ToCString() );

    // Append data from XDE to FBX nodes.
    Handle(XCAFDoc_ShapeTool) shapeTool = doc->GetShapeTool();
    //
    if ( shapeTool->IsAssembly(label) )
    {
      // Passed label is assembly, append it components to created node.
      TDF_LabelSequence assemblyLabels;
      shapeTool->GetComponents(label, assemblyLabels);

      for ( TDF_LabelSequence::Iterator it(assemblyLabels); it.More(); it.Next() )
      {
        appendNode(doc, fbxChildNode, it.Value(), pState, progress);

        if ( progress.IsCancelling() )
          return;
      }
    }
    else if ( shapeTool->IsReference(label) )
    {
      // Passed label is reference. Get original label and append it.
      TDF_Label originalLabel;
      if ( shapeTool->GetReferredShape(label, originalLabel) )
      {
        appendNode(doc, fbxChildNode, originalLabel, pState, progress);

        if ( progress.IsCancelling() )
          return;
      }
    }
    else if ( shapeTool->IsSimpleShape(label) )
    {
      progress.StepProgress(1);

      TopoDS_Shape shape = doc->GetShape(label);

      const bool
        isOk = exportFacets(doc, fbxChildNode, label, shape, pState);

      if ( !isOk )
      {
        fbxChildNode->Destroy();
        return;
      }
    }
    else
    {
      fbxChildNode->Destroy();
      return;
    }

    // Append location to node.
    TopLoc_Location location    = shapeTool->GetLocation(label);
    gp_Trsf         trsf        = location.Transformation();
    gp_XYZ          translation = trsf.TranslationPart();
    gp_Quaternion   rotation    = trsf.GetRotation();
    //
    double alpha, beta, gamma;
    rotation.GetEulerAngles(gp_Extrinsic_XYZ, alpha, beta, gamma);
    alpha *= (180.0 / M_PI);
    beta  *= (180.0 / M_PI);
    gamma *= (180.0 / M_PI);
    //
    const double scale = trsf.ScaleFactor();
    fbxChildNode->LclTranslation .Set( FbxDouble3( translation.X(), translation.Y(), translation.Z() ) );
    fbxChildNode->LclRotation    .Set( FbxDouble3( alpha, beta, gamma ) );
    fbxChildNode->LclScaling     .Set( FbxDouble3( scale, scale, scale ) );

    fbxParentNode->AddChild(fbxChildNode);
  }
}
#endif // FBXSDK_SHARED

//-----------------------------------------------------------------------------

fbx_XdeWriter::fbx_XdeWriter(const TCollection_AsciiString& filename,
                             ActAPI_ProgressEntry           notifier,
                             ActAPI_PlotterEntry            plotter)
//
: ActAPI_IAlgorithm (notifier, plotter),
  m_filename        (filename),
  m_pFbxState       (new t_fbxState)
{}

//-----------------------------------------------------------------------------

fbx_XdeWriter::~fbx_XdeWriter()
{
  delete m_pFbxState;
}

//-----------------------------------------------------------------------------

bool fbx_XdeWriter::Perform(const Handle(asiAsm_XdeDoc)& doc)
{
#if defined FBXSDK_SHARED
  m_progress.Reset();

  if ( doc.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Model is empty.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  this->clearState();

  // Create the FBX SDK manager.
  FbxManager* fbxManager = FbxManager::Create();
  //
  if ( !fbxManager )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Unable to create FBX manager.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  m_progress.SendLogMessage( LogInfo(Normal) << "Using Autodesk FBX SDK version %1."
                                             << fbxManager->GetVersion() );

  // Create a new FBX scene so it can be populated by the data to export.
  m_pFbxState->fbxScene = FbxScene::Create( fbxManager, m_filename.ToCString() );
  //
  if ( !m_pFbxState->fbxScene )
  {
    fbxManager->Destroy();

    m_progress.SendLogMessage(LogErr(Normal) << "Unable to create FBX scene.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  // Make FBX export work in [mm] (OpenCascade workspace units) as by default
  // Autodesk SDK uses [cm].
  m_pFbxState->fbxScene->GetGlobalSettings().SetSystemUnit(FbxSystemUnit::mm);

  // Get root node where meshes will be added.
  FbxNode* fbxRootNode = m_pFbxState->fbxScene->GetRootNode();

  // Init progress.
  asiAsm_XdeAssemblyItemIds items;
  doc->GetLeafAssemblyItems(items);
  //
  int progressCapacity = (int) (items.Size() / 3.0 * 4.0);
  //
  m_progress.Init(progressCapacity);
  m_progress.SetMessageKey("Exporting mesh");

  // Get "top" shapes labels.
  TDF_LabelSequence topLabels;
  doc->GetShapeTool()->GetFreeShapes(topLabels);
  //
  for ( TDF_LabelSequence::Iterator it(topLabels); it.More(); it.Next() )
  {
    ::appendNode(doc, fbxRootNode, it.Value(), m_pFbxState, m_progress);

    if ( m_progress.IsCancelling() )
    {
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
      return false;
    }
  }

  // Step progress to writing file.
  m_progress.SetMessageKey("Writing file");

  // Write in fall back format if no ASCII format found.
  int fileFormat = fbxManager->GetIOPluginRegistry()->GetNativeWriterFormat();

  // Try to export in ASCII if possible.
  const char* asciiStr = "ascii";
  int formatCount = fbxManager->GetIOPluginRegistry()->GetWriterFormatCount();
  //
  for ( int formatIndex = 0; formatIndex < formatCount; ++formatIndex )
  {
    if ( !fbxManager->GetIOPluginRegistry()->WriterIsFBX(formatIndex) )
    {
      continue;
    }

    FbxString formatDescription = fbxManager->GetIOPluginRegistry()->GetWriterFormatDescription(formatIndex);
    if ( formatDescription.Find(asciiStr) >= 0 )
    {
      fileFormat = formatIndex;
      break;
    }
  }

  // Create an IOSettings object.
  FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
  //
  if ( !fbxIOSettings )
  {
    fbxManager->Destroy();

    m_progress.SendLogMessage(LogErr(Normal) << "Unable to create FBX IOSettings.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }
  fbxManager->SetIOSettings(fbxIOSettings);

  // Set the export states. By default, the export states are always set to
  // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below
  // shows how to change these states.
  fbxIOSettings->SetBoolProp(EXP_FBX_MATERIAL,        true);
  fbxIOSettings->SetBoolProp(EXP_FBX_TEXTURE,         true);
  fbxIOSettings->SetBoolProp(EXP_FBX_EMBEDDED,        false);
  fbxIOSettings->SetBoolProp(EXP_FBX_SHAPE,           true);
  fbxIOSettings->SetBoolProp(EXP_FBX_GOBO,            true);
  fbxIOSettings->SetBoolProp(EXP_FBX_ANIMATION,       true);
  fbxIOSettings->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

  // Create an exporter.
  FbxExporter* fbxExporter = FbxExporter::Create(fbxManager, "");
  //
  if ( !fbxExporter )
  {
    fbxManager->Destroy();

    m_progress.SendLogMessage(LogErr(Normal) << "Unable to create FBX exporter.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  // Initialize the exporter.
  bool exportStatus = fbxExporter->Initialize( m_filename.ToCString(),
                                               fileFormat,
                                               fbxManager->GetIOSettings() );
  //
  if ( !exportStatus )
  {
    fbxManager->Destroy();

    m_progress.SendLogMessage( LogErr(Normal) << "Call to FbxExporter::Initialize() failed. Error returned: %1."
                                              << fbxExporter->GetStatus().GetErrorString() );
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  int versionMajor, versionMinor, versionRevision;
  FbxManager::GetFileFormatVersion(versionMajor, versionMinor, versionRevision);
  //
  m_progress.SendLogMessage(LogInfo(Normal) << "FBX file format version %1.%2.%3"
                                            << versionMajor
                                            << versionMinor
                                            << versionRevision);

  // Export the scene.
  exportStatus = fbxExporter->Export(m_pFbxState->fbxScene);
  //
  if ( !exportStatus )
  {
    fbxManager->Destroy();

    m_progress.SendLogMessage( LogErr(Normal) << "Call to FbxExporter::Export() failed. Error returned: %1."
                                              << fbxExporter->GetStatus().GetErrorString() );
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  // Destroy the exporter.
  fbxExporter->Destroy();

  // Delete the FBX Manager.
  // All the objects that have been allocated using the FBX Manager and that
  // haven't been explicitly destroyed are also automatically destroyed.
  fbxManager->Destroy();

  this->clearState();

  m_progress.StepProgress( progressCapacity - m_progress.CurrentProgress() );
  m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);

  return true;
#else
  (void) doc;
  m_progress.SendLogMessage(LogErr(Normal) << "FBX SDK is not available.");
  return false;
#endif
}

//-----------------------------------------------------------------------------

void fbx_XdeWriter::clearState()
{
#if defined FBXSDK_SHARED
  m_pFbxState->ClearState();
#endif
}
