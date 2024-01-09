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
#include <fbx_XdeReader.h>

// asiAlgo includes
#include <asiAlgo_MeshMerge.h>

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// OCCT includes
#include <Interface_Static.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <TDataStd_Name.hxx>

#if defined FBXSDK_SHARED
  // FBX SDK includes
  #include <fbxsdk.h>
#endif

//-----------------------------------------------------------------------------

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

struct asiAsm::xde::t_fbxReadState
{
#if defined FBXSDK_SHARED

  typedef NCollection_DataMap<FbxMesh*, TDF_Label>  FbxMeshLabelMap;
  FbxMeshLabelMap fbxMeshLabelMap;   //! Map which stores created labels for meshes.

  FbxScene*       fbxScene;          //!< FBX model scene
  double          factor;            //!< Scale unit factor for import
  bool            isToSaveStructure; //!< Divide structure by colored mesh
  bool            isReadToFacets;    //!< Read to facets or to mesh
  bool            toMergeNodes;      //!< Flag whethe to conglomerate near vertices or not

  t_fbxReadState()
    : fbxScene(nullptr),
      factor(0.0),
      isToSaveStructure(false),
      isReadToFacets(false),
      toMergeNodes(false) {} //!< Default ctor.

  void ClearState()
  {
    fbxMeshLabelMap.Clear();
  }
#endif
};

//-----------------------------------------------------------------------------

#if defined FBXSDK_SHARED
namespace
{
  //-----------------------------------------------------------------------------

  bool getColor(FbxNode*            fbxNode,
                const int           matId,
                Quantity_ColorRGBA& color)
  {
    if (matId < 0)
      return false;

    bool isColorFound = false;
    FbxSurfaceMaterial* mat = fbxNode->GetMaterial(matId);
    FbxProperty prop = mat->FindProperty("DiffuseColor");
    if (prop.IsValid())
    {
      FbxColor fbxColor = prop.Get<FbxColor>();
      Quantity_ColorRGBA noLinSRGBAColor((float)fbxColor.mRed,
                                         (float)fbxColor.mGreen,
                                         (float)fbxColor.mBlue,
                                         (float)1.0);
#if OCC_VERSION_HEX > 0x060700
      color = Quantity_ColorRGBA(Quantity_ColorRGBA::Convert_sRGB_To_LinearRGB(noLinSRGBAColor));
#else
      color = noLinSRGBAColor;
#endif
      FbxProperty trProp = mat->FindProperty("TransparencyFactor");
      if (trProp.IsValid())
      {
        FbxDouble alpha = trProp.Get<FbxDouble>();
        color.SetAlpha((float)(1.0 - alpha));
      }
      isColorFound = true;
    }
    return isColorFound;
  }

  //-----------------------------------------------------------------------------

  bool toTrsf(FbxAMatrix& fbxLoc, double factor, gp_Trsf& trsf)
  {
    const FbxDouble3 scaleVec = fbxLoc.GetS();
    if (Abs(scaleVec[0] - scaleVec[1]) > Precision::Confusion()
     || Abs(scaleVec[1] - scaleVec[2]) > Precision::Confusion()
     || Abs(scaleVec[0] - scaleVec[2]) > Precision::Confusion())
    {
      return false;
    }

    trsf.SetValues(fbxLoc.Get(0, 0), fbxLoc.Get(1, 0), fbxLoc.Get(2, 0), fbxLoc.Get(3, 0) * factor,
                   fbxLoc.Get(0, 1), fbxLoc.Get(1, 1), fbxLoc.Get(2, 1), fbxLoc.Get(3, 1) * factor,
                   fbxLoc.Get(0, 2), fbxLoc.Get(1, 2), fbxLoc.Get(2, 2), fbxLoc.Get(3, 2) * factor);

    return trsf.Form() != gp_Identity;
  }

  //-----------------------------------------------------------------------------

  FbxAMatrix transformation(FbxNode* pNode)
  {
    FbxAMatrix matrixGeo;
    matrixGeo.SetIdentity();

    if (pNode->GetNodeAttribute())
    {
      const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
      const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
      const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
      matrixGeo.SetT(lT);
      matrixGeo.SetR(lR);
      matrixGeo.SetS(lS);
    }
    FbxAMatrix localMatrix = pNode->EvaluateLocalTransform();

    FbxNode* pParentNode = pNode->GetParent();
    FbxAMatrix parentMatrix = pParentNode->EvaluateLocalTransform();
    while ((pParentNode = pParentNode->GetParent()) != NULL) 
    {
      parentMatrix = pParentNode->EvaluateLocalTransform() * parentMatrix;
    }

    FbxAMatrix matrix = parentMatrix * localMatrix * matrixGeo;
    return matrix;
  }

  //-----------------------------------------------------------------------------

  TopLoc_Location parseLocation(FbxNode*             sceneNode,
                                ActAPI_ProgressEntry progress,
                                double               factor)
  {
    gp_Trsf trsf;

    FbxAMatrix& fbxLoc = sceneNode->EvaluateLocalTransform();
    if (!fbxLoc.IsIdentity())
    {
      if (toTrsf(fbxLoc, factor, trsf))
      {
        if (trsf.Form() != gp_Identity)
        {
          const FbxDouble3 scaleVec = fbxLoc.GetS();
          progress.SendLogMessage(LogWarn(Normal)
            << "FBX node '%1' defines unsupported scaling %2 %3 %4"
            << sceneNode->GetName() << scaleVec[0] << scaleVec[1] << scaleVec[2]);
        }
      }
    }
    if (sceneNode->GetGeometry() != nullptr)
    {
      FbxAMatrix trsfMatrix = transformation(sceneNode);
      toTrsf(trsfMatrix, factor, trsf);
    }
    return TopLoc_Location(trsf);
  }

  //-----------------------------------------------------------------------------

  std::map<int, Handle(Poly_Triangulation)> importTriangulation(FbxMesh*        fbxMesh,
                                                                int             nbMat,
                                                                t_fbxReadState* pState)
  {
    std::map<int, Handle(Poly_Triangulation)> meshes;
    if (fbxMesh == NULL)
      return meshes;

    const int nbNodes    = fbxMesh->GetControlPointsCount();
    const int nbPolygons = fbxMesh->GetPolygonCount();
    if ( nbNodes <= 0 || nbPolygons <= 0 )
      return meshes;

    Handle(TColStd_HArray1OfInteger) materials;
    if ( nbMat > 0 )
      materials = new TColStd_HArray1OfInteger(0, nbMat - 1);
    for ( int i = 0; i < nbMat; ++i )
      materials->SetValue(i, 0);

    FbxLayerElementMaterial* layerElement = fbxMesh->GetElementMaterial();
    bool isColored = nbMat > 0 && (layerElement != NULL);

    TColgp_Array1OfPnt nodes(1, nbNodes);
    for (int nodeIt = 1; nodeIt <= nbNodes; nodeIt++)
    {
      FbxVector4 node = fbxMesh->GetControlPointAt(nodeIt - 1);
      nodes.SetValue(nodeIt, gp_Pnt(node[0] * pState->factor,
                                    node[1] * pState->factor,
                                    node[2] * pState->factor));
    }

    std::map<int, std::vector<Poly_Triangle>> triangles;
    for (int polyIt = 0; polyIt < nbPolygons; polyIt++)
    {
      int curMatId = 0;
      if (isColored)
      {
        int curId = layerElement->GetIndexArray()[polyIt];
        materials->ChangeValue(curId)++;
        if (!pState->isToSaveStructure)
          curMatId = curId;
      }
      else
        curMatId = -1;

      int polySize = fbxMesh->GetPolygonSize(polyIt);
      if (polySize == 3)
      {
        Poly_Triangle triangle;
        for (int vertIt = 0; vertIt < 3; vertIt++)
        {
          int vertexNb = fbxMesh->GetPolygonVertex(polyIt, vertIt);
          triangle.Set(vertIt + 1, vertexNb + 1);
        }
        triangles[curMatId].push_back(triangle);
      }
      else
      {
        // TODO: compute polygon3D triangulation based on polygon vertices of fbxMesh
      }
    }

    // Build triangulations and map it to material indices.
    for (int matIt = -1; matIt < nbMat; matIt++)
    {
      std::vector<Poly_Triangle> trVector = triangles[matIt];
      int nbTriangles = (int)trVector.size();
      if (nbTriangles == 0)
        continue;

      Poly_Array1OfTriangle trianglesPoly = Poly_Array1OfTriangle(1, nbTriangles);
      int trIt = 0;
      for (std::vector<Poly_Triangle>::iterator it = trVector.begin(); it != trVector.end(); it++)
        trianglesPoly.SetValue(++trIt, *it);

      bool hasUVNodes = (fbxMesh->GetElementUVCount() > 0);

      Handle(Poly_Triangulation) triangulation = new Poly_Triangulation(nbNodes, nbTriangles, hasUVNodes);

      for ( int n = nodes.Lower(); n <= nodes.Upper(); ++n )
        triangulation->SetNode( n, nodes(n) );

      //triangulation->ChangeNodes() = nodes;
      triangulation->ChangeTriangles() = trianglesPoly;

      if (hasUVNodes)
      {
        FbxGeometryElementUV* fbxUVs = fbxMesh->GetElementUV();
        TColgp_Array1OfPnt2d nodesUV = TColgp_Array1OfPnt2d(1, nbNodes);
        for (int nodeIt = 0; nodeIt < nbNodes; nodeIt++)
        {
          gp_Pnt2d point(fbxUVs->GetDirectArray()[nodeIt][0], fbxUVs->GetDirectArray()[nodeIt][1]);
          nodesUV.SetValue(nodeIt + 1, point);
        }

        for ( int n = nodesUV.Lower(); n <= nodesUV.Upper(); ++n )
        triangulation->SetUVNode( n, nodesUV(n) );
        //triangulation->ChangeUVNodes() = nodesUV;
      }

      if (fbxMesh->GetElementNormalCount() > 0)
      {
        FbxGeometryElementNormal* fbxNormals = fbxMesh->GetElementNormal();
        Handle(TShort_HArray1OfShortReal) normals = new TShort_HArray1OfShortReal(1, nbNodes * 3);
        for (int normalIt = 0; normalIt < nbNodes; normalIt++)
        {
          int newIt = normalIt * 3;
          normals->SetValue(newIt + 1, (Standard_ShortReal)fbxNormals->GetDirectArray()[normalIt][0]);
          normals->SetValue(newIt + 2, (Standard_ShortReal)fbxNormals->GetDirectArray()[normalIt][1]);
          normals->SetValue(newIt + 3, (Standard_ShortReal)fbxNormals->GetDirectArray()[normalIt][2]);
        }
        triangulation->SetNormals(normals);
      }

      if (pState->toMergeNodes)
      {
        std::vector<Handle(Poly_Triangulation)> triangulationVec;
        triangulationVec.push_back(triangulation);
        asiAlgo_MeshMerge conglomerate(triangulationVec, asiAlgo_MeshMerge::Mode_Mesh);
        triangulation = conglomerate.GetResultTris();
      }
      meshes[matIt] = triangulation;
    }

    return meshes;
  }

  //-----------------------------------------------------------------------------

  void readNode(const Handle(Doc)&   doc,
                FbxNode*             fbxNode,
                const TDF_Label&     parentLabel,
                ActAPI_ProgressEntry progress,
                t_fbxReadState*      pState)
  {
    progress.StepProgress(1);
    int nbChild = fbxNode->GetChildCount();
    bool isPart = nbChild == 0;

    TDF_Label newLabel, newMeshLabel;
    bool isAssemblyMesh = false;

    // Try to find part among already imported
    FbxMesh* mesh = fbxNode->GetMesh();
    if (mesh != NULL)
    {
      const TDF_Label* labelPtr = pState->fbxMeshLabelMap.Seek(mesh);
      if (labelPtr != NULL)
        newLabel = newMeshLabel = (*labelPtr);
    }
    TopLoc_Location location(parseLocation(fbxNode, progress, pState->factor));
    if (newLabel.IsNull())
    {
      newLabel = isPart ? doc->CreateEmptyPart() : doc->CreateEmptyAssembly();
      newMeshLabel = newLabel;
      if (mesh != NULL)
      {
        if (!isPart)
        {
          isAssemblyMesh = true;
          newMeshLabel = doc->CreateEmptyPart();
          doc->AddComponent(newLabel, newMeshLabel, location);
        }
        pState->fbxMeshLabelMap.Bind(mesh, newMeshLabel);
      }
    }

    doc->AddComponent(parentLabel, newLabel, location);

    TCollection_ExtendedString name(fbxNode->GetName());
    if (name.IsEmpty())
      name = isPart ? TCollection_ExtendedString("Part") : TCollection_ExtendedString("Assembly");
    TDataStd_Name::Set(newLabel, name);
    if (!newLabel.IsEqual(newMeshLabel))
      TDataStd_Name::Set(newMeshLabel, name);

    int nbMat = fbxNode->GetMaterialCount();
    std::map<int, Handle(Poly_Triangulation)> meshes;
    if (mesh != NULL)
      meshes = importTriangulation(fbxNode->GetMesh(), nbMat, pState);

    std::vector<std::pair<int, Handle(Poly_Triangulation)>> triangulations;
    // "-1" for triangulations without materials.
    for (int meshIt = -1; meshIt < nbMat; meshIt++)
    {
      Handle(Poly_Triangulation) triangulation = meshes[meshIt];
      if (triangulation.IsNull())
        continue;

      std::pair<int, Handle(Poly_Triangulation)> pair;
      pair.first = meshIt;
      pair.second = triangulation;
      triangulations.push_back(pair);
    }

    if (triangulations.size() == 1)
    {
      pState->isReadToFacets ? doc->SetFacets(newMeshLabel, triangulations[0].second)
                             : doc->SetMesh  (newMeshLabel, triangulations[0].second);
      Quantity_ColorRGBA color;
      if (getColor(fbxNode, triangulations[0].first, color))
        doc->SetColor(newMeshLabel, color, true);
    }
    else if (triangulations.size() > 1)
    {
      for (int meshIt = 0; meshIt < triangulations.size(); meshIt++)
      {
        Handle(Poly_Triangulation) triangulation = triangulations[meshIt].second;
        TDF_Label newComponent = doc->CreateEmptyPart();
        doc->AddComponent(newMeshLabel, newComponent, TopLoc_Location());
        if (pState->isReadToFacets)
          doc->SetFacets(newComponent, triangulation);
        else
          doc->SetMesh(newComponent, triangulation);
        Quantity_ColorRGBA color;
        if (getColor(fbxNode, triangulations[meshIt].first, color))
          doc->SetColor(newComponent, color, true);
      }
    }
    for (int childIt = 0; childIt < nbChild; childIt++)
    {
      readNode(doc, fbxNode->GetChild(childIt), newLabel, progress, pState);
    }
  }
}
#endif // FBXSDK_SHARED

//-----------------------------------------------------------------------------

fbxReader::fbxReader(const TCollection_AsciiString& filename,
                     ActAPI_ProgressEntry           notifier,
                     ActAPI_PlotterEntry            plotter)
//
: ActAPI_IAlgorithm (notifier, plotter),
  m_filename        (filename),
  m_pFbxState       (new t_fbxReadState)
{}

//-----------------------------------------------------------------------------

fbxReader::~fbxReader()
{
  delete m_pFbxState;
}

//-----------------------------------------------------------------------------

bool fbxReader::Perform(const Handle(Doc)& doc)
{
#if defined FBXSDK_SHARED
  m_progress.Reset();

  FbxManager* fbxManager = FbxManager::Create();
  if (!fbxManager)
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Unable to create FBX manager.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  m_progress.SendLogMessage(LogInfo(Normal) << "Using Autodesk FBX SDK version %1."
                                            << fbxManager->GetVersion());

  FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
  FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
  if (!fbxIOSettings)
  {
    GetProgress().SendLogMessage(LogErr(Normal) << "Unable to create FBX IOSettings.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    fbxManager->Destroy();
    return false;
  }

  ios->SetBoolProp(IMP_FBX_MATERIAL, true);
  ios->SetBoolProp(IMP_FBX_TEXTURE, true);

  m_pFbxState->fbxScene = FbxScene::Create(fbxManager, "");

  if (!m_pFbxState->fbxScene)
  {
    fbxManager->Destroy();

    m_progress.SendLogMessage(LogErr(Normal) << "Unable to create FBX scene.");
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Failed);
    return false;
  }

  FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
  if (!fbxImporter)
  {
    GetProgress().SendLogMessage(LogErr(Normal) << "Unable to create FBX importer.");
    fbxManager->Destroy();
    return false;
  }

  m_progress.SetMessageKey("Reading file");

  fbxImporter->Initialize(m_filename.ToCString(), -1, ios);
  if (!fbxImporter->Import(m_pFbxState->fbxScene))
  {
    GetProgress().SendLogMessage(LogErr(Normal) << "Failure while reading FBX file.");
    fbxImporter->Destroy();
    fbxManager->Destroy();
    return false;
  }

  // Settings.
  m_pFbxState->isToSaveStructure = (Interface_Static::IVal("read.divide.colored.meshes") == 0);
  m_pFbxState->factor = m_pFbxState->fbxScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(fbxsdk::FbxSystemUnit::mm);
  m_pFbxState->isReadToFacets = true; // Set this setting to true for import mesh to facets, not to TDataXtd_Triangulation
  m_pFbxState->toMergeNodes = false;  // Set this setting to true to merge coincident or near-coincident mesh vertices on import

  TDF_Label rootLabel = doc->GetLabelOfModel();
  GetProgress().Init(m_pFbxState->fbxScene->GetNodeCount());
  readNode(doc, m_pFbxState->fbxScene->GetRootNode(), rootLabel, m_progress, m_pFbxState);
  doc->UpdateAssemblies();

  fbxImporter->Destroy();
  fbxManager->Destroy();

  clearState();
  m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);

  return true;
#else
  (void) doc;
  m_progress.SendLogMessage(LogErr(Normal) << "FBX SDK is not available.");
  return false;
#endif
}

//-----------------------------------------------------------------------------

void fbxReader::clearState()
{
#if defined FBXSDK_SHARED
  m_pFbxState->ClearState();
#endif
}
