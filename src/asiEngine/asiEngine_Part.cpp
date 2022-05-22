//-----------------------------------------------------------------------------
// Created on: 21 March 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2016-present, Sergey Slyadnev
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
#include <asiEngine_Part.h>

// asiEngine includes
#include <asiEngine_Curve.h>
#include <asiEngine_STEPReaderOutput.h>
#include <asiEngine_TolerantShapes.h>

// asiVisu includes
#include <asiVisu_PartPrs.h>
#include <asiVisu_PartNodeInfo.h>
#include <asiVisu_PrsManager.h>
#include <asiVisu_Utils.h>

// asiData includes
#include <asiData_MetadataAttr.h>

// asiAlgo includes
#include <asiAlgo_CheckDeviations.h>
#include <asiAlgo_FileFormat.h>
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_ReadSTEPWithMeta.h>
#include <asiAlgo_STEP.h>
#include <asiAlgo_IGES.h>
#include <asiAlgo_Utils.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// VTK includes
#pragma warning(push, 0)
#include <vtkProperty.h>
#pragma warning(pop)

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

//-----------------------------------------------------------------------------

Handle(asiData_PartNode) asiEngine_Part::GetPart()
{
  return m_model->GetPartNode();
}

//-----------------------------------------------------------------------------

Handle(asiData_PartNode) asiEngine_Part::CreatePart()
{
  // Add Part Node to Partition
  Handle(asiData_PartNode) geom_n = Handle(asiData_PartNode)::DownCast( asiData_PartNode::Instance() );
  m_model->GetPartPartition()->AddNode(geom_n);

  // Initialize geometry
  geom_n->Init(true);
  geom_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
  geom_n->SetName("Part");

  // Create underlying face representation Node
  {
    Handle(ActAPI_INode) geom_face_base = asiData_FaceNode::Instance();
    m_model->GetFacePartition()->AddNode(geom_face_base);

    // Initialize
    Handle(asiData_FaceNode) geom_face_n = Handle(asiData_FaceNode)::DownCast(geom_face_base);
    geom_face_n->Init();
    geom_face_n->SetUserFlags(NodeFlag_IsPresentedInDomainView | NodeFlag_IsPresentationVisible);
    geom_face_n->SetName("Face domain");

    // Set as child
    geom_n->AddChildNode(geom_face_n);
  }

  // Create underlying face norms representation Node
  {
    Handle(ActAPI_INode) geom_face_norms_base = asiData_FaceNormsNode::Instance();
    m_model->GetFaceNormsPartition()->AddNode(geom_face_norms_base);

    // Initialize
    Handle(asiData_FaceNormsNode) geom_face_norms_n = Handle(asiData_FaceNormsNode)::DownCast(geom_face_norms_base);
    geom_face_norms_n->Init();
    geom_face_norms_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
    geom_face_norms_n->SetName("Face norms");

    // Set as child
    geom_n->AddChildNode(geom_face_norms_n);
  }

  // Create underlying surface representation Node
  {
    Handle(ActAPI_INode) geom_surf_base = asiData_SurfNode::Instance();
    m_model->GetSurfPartition()->AddNode(geom_surf_base);

    // Initialize
    Handle(asiData_SurfNode) geom_surf_n = Handle(asiData_SurfNode)::DownCast(geom_surf_base);
    geom_surf_n->Init();
    geom_surf_n->SetUserFlags(NodeFlag_IsPresentedInHostView | NodeFlag_IsPresentationVisible);
    geom_surf_n->SetName("Host surface");

    // Set as child
    geom_n->AddChildNode(geom_surf_n);
  }

  // Create underlying contour representation Node
  {
    Handle(ActAPI_INode) geom_face_contour_base = asiData_FaceContourNode::Instance();
    m_model->GetFaceContourPartition()->AddNode(geom_face_contour_base);

    // Initialize
    Handle(asiData_FaceContourNode) geom_face_contour_n = Handle(asiData_FaceContourNode)::DownCast(geom_face_contour_base);
    geom_face_contour_n->Init();
    geom_face_contour_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
    geom_face_contour_n->SetName("Face contour");

    // Set as child
    geom_n->AddChildNode(geom_face_contour_n);
  }

  // Create underlying hatching representation Node
  {
    Handle(ActAPI_INode) geom_face_hatching_base = asiData_HatchingNode::Instance();
    m_model->GetHatchingPartition()->AddNode(geom_face_hatching_base);

    // Initialize
    Handle(asiData_HatchingNode) geom_face_hatching_n = Handle(asiData_HatchingNode)::DownCast(geom_face_hatching_base);
    geom_face_hatching_n->Init();
    geom_face_hatching_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
    geom_face_hatching_n->SetName("Face hatching");

    // Set as child
    geom_n->AddChildNode(geom_face_hatching_n);
  }

  // Create underlying edge representation Node
  {
    Handle(ActAPI_INode) geom_edge_base = asiData_EdgeNode::Instance();
    m_model->GetEdgePartition()->AddNode(geom_edge_base);

    // Initialize
    Handle(asiData_EdgeNode) geom_edge_n = Handle(asiData_EdgeNode)::DownCast(geom_edge_base);
    geom_edge_n->Init();
    geom_edge_n->SetUserFlags(NodeFlag_IsPresentedInDomainView | NodeFlag_IsPresentationVisible);
    geom_edge_n->SetName("Edge domain");

    // Set as child
    geom_n->AddChildNode(geom_edge_n);
  }

  // Create underlying curve representation Node
  {
    asiEngine_Curve(m_model).Create("Host curve", geom_n);
  }

  // Create underlying boundary edges representation Node
  {
    Handle(ActAPI_INode) geom_edges_base = asiData_BoundaryEdgesNode::Instance();
    m_model->GetBoundaryEdgesPartition()->AddNode(geom_edges_base);

    // Initialize
    Handle(asiData_BoundaryEdgesNode) geom_edges_n = Handle(asiData_BoundaryEdgesNode)::DownCast(geom_edges_base);
    geom_edges_n->Init();
    geom_edges_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
    geom_edges_n->SetName("Boundary edges");

    // Set as child
    geom_n->AddChildNode(geom_edges_n);
  }

  // Create underlying Contour Node
  {
    Handle(ActAPI_INode) geom_contour_base = asiData_ContourNode::Instance();
    m_model->GetContourPartition()->AddNode(geom_contour_base);

    // Initialize
    Handle(asiData_ContourNode) geom_contour_n = Handle(asiData_ContourNode)::DownCast(geom_contour_base);
    geom_contour_n->Init();
    geom_contour_n->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);
    geom_contour_n->SetName("Custom contour");

    // Set as child
    geom_n->AddChildNode(geom_contour_n);
  }

  // Create underlying Vertex Node
  {
    Handle(ActAPI_INode) geom_vertex_base = asiData_VertexNode::Instance();
    m_model->GetVertexPartition()->AddNode(geom_vertex_base);

    // Initialize
    Handle(asiData_VertexNode) geom_vertex_n = Handle(asiData_VertexNode)::DownCast(geom_vertex_base);
    geom_vertex_n->Init();
    geom_vertex_n->SetUserFlags(NodeFlag_IsPresentedInDomainView | NodeFlag_IsPresentationVisible);
    geom_vertex_n->SetName("Vertex");

    // Set as child
    geom_n->AddChildNode(geom_vertex_n);
  }

  // Create underlying Tolerant Shapes Node
  {
    Handle(ActAPI_INode) tolshapes_base = asiData_TolerantShapesNode::Instance();
    m_model->GetTolerantShapesPartition()->AddNode(tolshapes_base);

    // Initialize
    Handle(asiData_TolerantShapesNode)
      tolshapes_n = Handle(asiData_TolerantShapesNode)::DownCast(tolshapes_base);
    //
    tolshapes_n->Init();
    tolshapes_n->SetName("Tolerant shapes");

    // Set as child
    geom_n->AddChildNode(tolshapes_n);
  }

  // Create underlying Metadata Node
  this->CreateMetadata();

  // Return the just created Node
  return geom_n;
}

//-----------------------------------------------------------------------------

bool asiEngine_Part::Import(const TCollection_AsciiString& filename)
{
  // Auto-recognize file format.
  asiAlgo_FileFormat
    format = asiAlgo_FileFormatTool::FormatFromFileContent(filename);
  //
  if ( format == FileFormat_Unknown )
  {
    // Recognize file format from file extension.
    format = asiAlgo_FileFormatTool::FormatFromFileExtension(filename);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = m_model->GetPartNode();

  TCollection_AsciiString unitString = "mm";

  // Load CAD data.
  switch ( format )
  {
    case FileFormat_STEP:
    {
      // Prepare output
      Handle(asiEngine_STEPReaderOutput)
        output = new asiEngine_STEPReaderOutput(m_model);

      // Prepare reader.
      asiAlgo_ReadSTEPWithMeta reader(m_progress, m_plotter);
      reader.SetOutput(output);

      // Read & translate.
      if ( !reader.Perform(filename) )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "STEP reader failed.");
        return false;
      }

      unitString = reader.GetUnitString();
      break;
    }
    case FileFormat_BREP:
    {
      // Read BREP.
      TopoDS_Shape shape;
      if ( !asiAlgo_Utils::ReadBRep(filename, shape) )
      {
        m_progress.SendLogMessage(LogErr(Normal) << "BREP reader failed.");
        return false;
      }

      // Update geometric data structures.
      this->Update( shape, nullptr, !partNode->IsKeepTessParams() );

      break;
    }
    case FileFormat_IGES:
    {
      TopoDS_Shape shape;
      asiAlgo_IGES reader(m_progress, m_plotter);
      if ( !reader.Read(filename, shape) )
      {
        return false;
      }

      // Update geometric data structures.
      this->Update(shape, nullptr, !partNode->IsKeepTessParams());

      break;
    }
    default:
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Unsupported file format.");
      return false;
    }
  }

  // Set filename and original units for reference.
  partNode->SetFilenameIn(filename);
  partNode->SetOriginalUnits(unitString);

  return true;
}

//-----------------------------------------------------------------------------

bool
  asiEngine_Part::CheckDeviation(const Handle(asiData_IVPointSetNode)& pcNode)
{
  Handle(asiData_DeviationNode) devNode;
  return this->CheckDeviation(pcNode, devNode);
}

//-----------------------------------------------------------------------------

bool
  asiEngine_Part::CheckDeviation(const Handle(asiData_IVPointSetNode)& pcNode,
                                 Handle(asiData_DeviationNode)&        devNode)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = m_model->GetPartNode();

  // Check deviations.
  asiAlgo_CheckDeviations checkDeviations( pcNode->GetPoints(),
                                           m_progress,
                                           m_plotter );
  //
  if ( !checkDeviations.Perform( partNode->GetShape() ) )
    return false;

  // Create Deviation Node.
  Handle(ActAPI_INode) devNodeBase = asiData_DeviationNode::Instance();
  m_model->GetDeviationPartition()->AddNode(devNodeBase);

  // Initialize.
  devNode = Handle(asiData_DeviationNode)::DownCast(devNodeBase);
  //
  devNode->Init();
  devNode->SetName("Deviation");

  // Store deviations.
  devNode->SetMeshWithScalars( checkDeviations.GetResult() );

  // Add Deviation Node as a child of the Part Node.
  partNode->AddChildNode(devNode);

  return true;
}

//-----------------------------------------------------------------------------

Handle(asiData_MetadataNode) asiEngine_Part::GetMetadata() const
{
  return m_model->GetMetadataNode();
}

//-----------------------------------------------------------------------------

Handle(asiData_MetadataNode) asiEngine_Part::CreateMetadata()
{
  Handle(ActAPI_INode) metadata_base = asiData_MetadataNode::Instance();
  m_model->GetMetadataPartition()->AddNode(metadata_base);

  // Initialize
  Handle(asiData_MetadataNode)
    metadata_n = Handle(asiData_MetadataNode)::DownCast(metadata_base);
  //
  metadata_n->Init();
  metadata_n->SetName("Metadata");

  // Set as child for the Part Node
  m_model->GetPartNode()->AddChildNode(metadata_n);

  return metadata_n;
}

//-----------------------------------------------------------------------------

void asiEngine_Part::CleanMetadata()
{
  this->_cleanChildren( m_model->GetMetadataNode() );
}

//-----------------------------------------------------------------------------

void asiEngine_Part::UpdateMetadata(const Handle(asiAlgo_History)& history)
{
  Handle(asiData_MetadataNode) N = this->GetMetadata();

  // Get all metadata records.
  asiData_MetadataAttr::t_shapeColorMap shapeColorMap;
  N->GetShapeColorMap(shapeColorMap);

  // Clean up the existing metadata.
  this->CleanMetadata();

  // If there is no history, let's simply leave the
  // metadata container cleaned up.
  if ( history.IsNull() )
    return;

  // Create new metadata records from the collected DTOs.
  for ( int k = 1; k <= shapeColorMap.Extent(); ++k )
  {
    const TopoDS_Shape& sshape = shapeColorMap.FindKey(k);

    std::cout << "\t" << asiAlgo_Utils::ShapeAddrWithPrefix(sshape) << " >>> ";

    TopoDS_Shape imSh = history->GetLastImageOrArg(sshape);
    //
    if ( imSh.IsNull() ) // Image is null, i.e., the shape was deleted.
    {
      std::cout << "null" << std::endl;
      continue;
    }
    //
    std::cout << asiAlgo_Utils::ShapeAddrWithPrefix(imSh) << std::endl;

    // Add metadata record.
    N->SetColor( imSh, shapeColorMap.FindFromIndex(k) );
  }
}

//-----------------------------------------------------------------------------

int asiEngine_Part::GetNumOfMetadata() const
{
  Handle(asiData_MetadataNode) N = this->GetMetadata();

  // Get all metadata records.
  asiData_MetadataAttr::t_shapeColorMap shapeColorMap;
  N->GetShapeColorMap(shapeColorMap);

  return shapeColorMap.Extent();
}

//-----------------------------------------------------------------------------

Handle(asiData_FeaturesNode) asiEngine_Part::CreateFeatures()
{
  Handle(ActAPI_INode) features_base = asiData_FeaturesNode::Instance();
  m_model->GetFeaturesPartition()->AddNode(features_base);

  // Initialize.
  Handle(asiData_FeaturesNode)
    features_n = Handle(asiData_FeaturesNode)::DownCast(features_base);
  //
  features_n->Init();
  features_n->SetName("Features");

  // Set as child for the Part Node.
  m_model->GetPartNode()->AddChildNode(features_n);

  return features_n;
}

//-----------------------------------------------------------------------------

Handle(asiData_FeatureNode)
  asiEngine_Part::CreateFeature(const TCollection_ExtendedString& name,
                                const int                         id,
                                const TColStd_PackedMapOfInteger& feature)
{
  Handle(asiData_FeatureNode)
    node = Handle(asiData_FeatureNode)::DownCast( asiData_FeatureNode::Instance() );
  //
  m_model->GetFeaturePartition()->AddNode(node);

  // Initialize.
  node->Init();
  node->SetName(name);
  node->SetFeatureId(id);
  node->SetMask( new TColStd_HPackedMapOfInteger(feature) );

  // Set as child for the Features Node.
  m_model->GetFeaturesNode()->AddChildNode(node);

  // Add reference in the Part Node.
  m_model->GetPartNode()->ConnectReferenceToList(asiData_PartNode::PID_Features,
                                                 node);

  return node;
}

//-----------------------------------------------------------------------------

void asiEngine_Part::CleanFeatures()
{
  this->_cleanChildren( m_model->GetFeaturesNode() );
}

//-----------------------------------------------------------------------------

int asiEngine_Part::GetNumOfFeatures() const
{
  Handle(ActData_ReferenceListParameter)
    refListParam = ActParamTool::AsReferenceList( m_model->GetPartNode()->Parameter(asiData_PartNode::PID_Features) );
  //
  const int numElems = refListParam->NbTargets();

  return numElems;
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetFeatures(Handle(ActAPI_HNodeList)& nodes) const
{
  nodes = new ActAPI_HNodeList;

  for ( Handle(ActAPI_IChildIterator) cit = m_model->GetFeaturesNode()->GetChildIterator();
        cit->More(); cit->Next() )
  {
    Handle(asiData_FeatureNode)
      feature_n = Handle(asiData_FeatureNode)::DownCast( cit->Value() );
    //
    if ( !feature_n.IsNull() && feature_n->IsWellFormed() )
      nodes->Append(feature_n);
  }
}

//-----------------------------------------------------------------------------

Handle(asiData_FeatureNode) asiEngine_Part::FindFeature(const int  featureId,
                                                        const bool create)
{
  // Access Features.
  Handle(asiData_FeaturesNode) features_n = m_model->GetFeaturesNode();
  //
  if ( features_n.IsNull() )
    features_n = this->CreateFeatures();

  // Access Feature.
  Handle(asiData_FeatureNode)
    feature_n = m_model->GetFeaturesNode()->FindFeature(featureId);

  // Create if requested.
  if ( feature_n.IsNull() && create )
  {
    // Prepare name.
    TCollection_AsciiString nodeName("Feature ");
    nodeName += featureId;

    // Create elementary Feature Node.
    feature_n = this->CreateFeature( nodeName.ToCString(), featureId, TColStd_PackedMapOfInteger() );
  }

  return feature_n;
}

//-----------------------------------------------------------------------------

Handle(asiData_Grid2dNode) asiEngine_Part::CreateFaceGrid2d()
{
  Handle(asiData_Grid2dNode)
    node = Handle(asiData_Grid2dNode)::DownCast( asiData_Grid2dNode::Instance() );
  //
  m_model->GetGrid2dPartition()->AddNode(node);

  // Initialize.
  node->Init();
  node->SetName("UV grid");

  Handle(asiData_PartNode) partNode = this->GetPart();

  // Set as child for the Part Node.
  partNode->AddChildNode(node);

  return node;
}

//-----------------------------------------------------------------------------

Handle(asiData_Grid2dNode)
  asiEngine_Part::FindFaceGrid2d(const bool create)
{
  Handle(asiData_PartNode)   partNode = this->GetPart();
  Handle(asiData_Grid2dNode) gridNode = partNode->GetGrid2d();

  if ( gridNode.IsNull() && create )
    return this->CreateFaceGrid2d();

  return gridNode;
}

//-----------------------------------------------------------------------------

Handle(asiData_DiscrFaceNode) asiEngine_Part::CreateDiscrFace()
{
  Handle(asiData_DiscrFaceNode)
    node = Handle(asiData_DiscrFaceNode)::DownCast( asiData_DiscrFaceNode::Instance() );
  //
  m_model->GetDiscrFacePartition()->AddNode(node);

  // Initialize.
  node->Init();
  node->SetName("Discrete face");
  node->AddUserFlags(NodeFlag_IsPresentedInPartView);

  Handle(asiData_PartNode) partNode = this->GetPart();

  // Set as child for the Part Node.
  partNode->AddChildNode(node);

  return node;
}

//-----------------------------------------------------------------------------

Handle(asiData_DiscrFaceNode)
  asiEngine_Part::FindDiscrFace(const bool create)
{
  Handle(asiData_PartNode)      partNode      = this->GetPart();
  Handle(asiData_DiscrFaceNode) discrFaceNode = partNode->GetDiscrFace();

  if ( discrFaceNode.IsNull() && create )
    return this->CreateDiscrFace();

  return discrFaceNode;
}

//-----------------------------------------------------------------------------

Handle(asiData_PartNode) asiEngine_Part::Update(const TopoDS_Shape&            model,
                                                const Handle(asiAlgo_History)& history,
                                                const bool                     doResetTessParams)
{
  // Get Part Node.
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return part_n;

  // Actualize metadata.
  this->UpdateMetadata(history);

  // Reset data without cleaning up metadata.
  this->Clean(false);

  // Set working structures
  Handle(ActData_ShapeParameter)
    shapeParam = Handle(ActData_ShapeParameter)::DownCast( part_n->Parameter(asiData_PartNode::PID_Geometry) );
  //
  Handle(asiData_AAGParameter)
    aagParam = Handle(asiData_AAGParameter)::DownCast( part_n->Parameter(asiData_PartNode::PID_AAG) );
  //
  shapeParam->SetShape(model);

  // If BVH exists, we clean it up.
  if ( !part_n->GetBVH().IsNull() )
  {
    // Store in OCAF
    Handle(asiData_BVHParameter)
      bvhParam = Handle(asiData_BVHParameter)::DownCast( part_n->Parameter(asiData_PartNode::PID_BVH) );
    //
    bvhParam->SetBVH(nullptr);
  }

  // Build AAG automatically (if not auto-build is not disabled).
  if ( part_n->IsAutoAAG() )
    aagParam->SetAAG( new asiAlgo_AAG(model) );

  // Reset tessellation parameters if requested.
  if ( doResetTessParams )
  {
    part_n->SetLinearDeflection( asiAlgo_MeshGen::AutoSelectLinearDeflection(model) );
    part_n->SetAngularDeflection( asiAlgo_MeshGen::AutoSelectAngularDeflection(model) );
  }

  // Actualize naming if it is initialized.
  if ( part_n->HasNaming() )
    part_n->GetNaming()->Actualize(model);

  // Actualize presentation.
  if ( m_prsMgr )
    m_prsMgr->Actualize(part_n);

  return part_n;
}

//-----------------------------------------------------------------------------

void asiEngine_Part::SetAAG(const Handle(asiAlgo_AAG)& aag)
{
  // Store AAG in the corresponding Parameter.
  Handle(asiData_PartNode) partNode = m_model->GetPartNode();
  //
  Handle(asiData_AAGParameter)::DownCast( partNode->Parameter(asiData_PartNode::PID_AAG) )->SetAAG(aag);
}

//-----------------------------------------------------------------------------

bool asiEngine_Part::HasNaming() const
{
  // Get Part Node.
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return false;

  return part_n->HasNaming();
}

//-----------------------------------------------------------------------------

void asiEngine_Part::InitializeNaming()
{
  // Get Part Node.
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return;

  // Get part shape.
  TopoDS_Shape partShape = part_n->GetShape();
  //
  if ( partShape.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Part contains no B-Rep.");
    return;
  }

  // Prepare naming service.
  Handle(asiAlgo_Naming) naming = new asiAlgo_Naming(partShape, m_progress);
  //
  if ( !naming->InitNames() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Naming initialization failed.");
    return;
  }

  // Set naming service to part.
  Handle(asiData_NamingParameter)
    namingParam = Handle(asiData_NamingParameter)::DownCast( part_n->Parameter(asiData_PartNode::PID_Naming) );
  //
  namingParam->SetNaming(naming);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::StoreHistory(const Handle(asiAlgo_History)& history)
{
  // Get Part Node.
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return;

  // Set naming service to part.
  Handle(asiData_NamingParameter)
    namingParam = Handle(asiData_NamingParameter)::DownCast( part_n->Parameter(asiData_PartNode::PID_Naming) );
  //
  namingParam->SetNaming( new asiAlgo_Naming(history) );
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_BVHFacets) asiEngine_Part::BuildBVH(const bool store)
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();

  // Build BVH for facets
  Handle(asiAlgo_BVHFacets)
    bvh = new asiAlgo_BVHFacets(part_n->GetShape(true),
                                asiAlgo_BVHFacets::Builder_Binned,
                                m_progress,
                                m_plotter);

  if ( store ) // Store in OCAF
  {
    Handle(asiData_BVHParameter)
      bvhParam = Handle(asiData_BVHParameter)::DownCast( part_n->Parameter(asiData_PartNode::PID_BVH) );
    //
    bvhParam->SetBVH(bvh);
  }

  return bvh;
}

//-----------------------------------------------------------------------------

void asiEngine_Part::Clean(const bool cleanMeta)
{
  // Get Part Node.
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return;

  // Reset data.
  part_n->GetFaceRepresentation()          ->Init();
  part_n->GetSurfaceRepresentation()       ->Init();
  part_n->GetEdgeRepresentation()          ->Init();
  part_n->GetCurveRepresentation()         ->Init();
  part_n->GetBoundaryEdgesRepresentation() ->Init();
  part_n->SetTransformation(0., 0., 0., 0., 0., 0.);

  // Clean up tolerant shapes.
  asiEngine_TolerantShapes tolApi(m_model, m_prsMgr, m_progress, m_plotter);
  //
  tolApi.Clean_All();

  // Clean up metadata.
  if ( cleanMeta )
    this->CleanMetadata();
}

//-----------------------------------------------------------------------------

TopoDS_Face asiEngine_Part::GetFace(const int oneBasedId)
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return TopoDS_Face();

  // Get AAG.
  Handle(asiAlgo_AAG) aag = part_n->GetAAG();
  //
  if ( aag.IsNull() )
    return TopoDS_Face();

  // Get face.
  if ( !aag->HasFace(oneBasedId) )
    return TopoDS_Face();

  return aag->GetFace(oneBasedId);
}

//-----------------------------------------------------------------------------

TopoDS_Edge asiEngine_Part::GetEdge(const int oneBasedId)
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return TopoDS_Edge();

  // Get AAG.
  Handle(asiAlgo_AAG) aag = part_n->GetAAG();
  //
  if ( aag.IsNull() )
    return TopoDS_Edge();

  const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();

  // Get edge.
  if ( (oneBasedId < 1) || ( oneBasedId > allEdges.Extent() ) )
    return TopoDS_Edge();

  return TopoDS::Edge( allEdges(oneBasedId) );
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiEngine_Part::GetShape()
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return TopoDS_Shape();

  return part_n->GetShape();
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_AAG) asiEngine_Part::GetAAG()
{
  // Get Part Node
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return nullptr;

  return part_n->GetAAG();
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetSubShapeIndicesByFaceIndices(const TColStd_PackedMapOfInteger& faceIndices,
                                                     TColStd_PackedMapOfInteger&       indices)
{
  const TopTools_IndexedMapOfShape&
    AllFaces = m_model->GetPartNode()->GetAAG()->GetMapOfFaces();
  //
  TopTools_IndexedMapOfShape SelectedFaces;

  // Get selected faces in topological form
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(faceIndices); fit.More(); fit.Next() )
  {
    const int input_face_idx = fit.Key();

    if ( input_face_idx > AllFaces.Extent() )
      m_progress.SendLogMessage(LogInfo(Normal) << "Face index %1 is out of range."
                                                << input_face_idx);
    else
      SelectedFaces.Add( AllFaces.FindKey(input_face_idx) );
  }

  // Get indices of the faces among all sub-shapes
  GetSubShapeIndices(SelectedFaces, indices);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetSubShapeIndicesByEdgeIndices(const TColStd_PackedMapOfInteger& edgeIndices,
                                                     TColStd_PackedMapOfInteger&       indices)
{
  const TopTools_IndexedMapOfShape&
    AllEdges = m_model->GetPartNode()->GetAAG()->RequestMapOfEdges();
  //
  TopTools_IndexedMapOfShape SelectedEdges;

  // Get selected edges in topological form
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(edgeIndices); fit.More(); fit.Next() )
  {
    const int input_edge_idx = fit.Key();
    SelectedEdges.Add( AllEdges.FindKey(input_edge_idx) );
  }

  // Get indices of the edges among all sub-shapes
  GetSubShapeIndices(SelectedEdges, indices);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetSubShapeIndicesByVertexIndices(const TColStd_PackedMapOfInteger& vertexIndices,
                                                       TColStd_PackedMapOfInteger&       indices)
{
  const TopTools_IndexedMapOfShape&
    AllVertices = m_model->GetPartNode()->GetAAG()->RequestMapOfVertices();
  //
  TopTools_IndexedMapOfShape SelectedVertices;

  // Get selected vertices in topological form
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(vertexIndices); fit.More(); fit.Next() )
  {
    const int input_vertex_idx = fit.Key();
    SelectedVertices.Add( AllVertices.FindKey(input_vertex_idx) );
  }

  // Get indices of the vertices among all sub-shapes
  GetSubShapeIndices(SelectedVertices, indices);
}

//-----------------------------------------------------------------------------


void asiEngine_Part::GetSubShapeIndices(const TopTools_IndexedMapOfShape& subShapes,
                                        TColStd_PackedMapOfInteger&       indices)
{
  Handle(asiAlgo_AAG) aag = m_model->GetPartNode()->GetAAG();
  //
  if ( aag.IsNull() )
    return;

  const TopTools_IndexedMapOfShape& M = aag->RequestMapOfSubShapes();
  //
  for ( int i = 1; i <= subShapes.Extent(); ++i )
    indices.Add( M.FindIndex( subShapes.FindKey(i) ) );
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetSubShapeIndices(const TopTools_IndexedMapOfShape& subShapes,
                                        TColStd_PackedMapOfInteger&       faceIndices,
                                        TColStd_PackedMapOfInteger&       edgeIndices,
                                        TColStd_PackedMapOfInteger&       vertexIndices)
{
  const TopTools_IndexedMapOfShape&
    M = m_model->GetPartNode()->GetAAG()->RequestMapOfSubShapes();
  //
  for ( int i = 1; i <= subShapes.Extent(); ++i )
  {
    const TopoDS_Shape& sh = subShapes.FindKey(i);

    if ( sh.ShapeType() == TopAbs_FACE )
      faceIndices.Add( M.FindIndex(sh) );
    //
    else if ( sh.ShapeType() == TopAbs_WIRE )
    {
      for ( TopExp_Explorer eexp(sh, TopAbs_EDGE); eexp.More(); eexp.Next() )
        edgeIndices.Add( M.FindIndex( eexp.Current() ) );
    }
    //
    else if ( sh.ShapeType() == TopAbs_EDGE )
      edgeIndices.Add( M.FindIndex(sh) );
    //
    else if ( sh.ShapeType() == TopAbs_VERTEX )
      vertexIndices.Add( M.FindIndex(sh) );
  }
}

//-----------------------------------------------------------------------------

void asiEngine_Part::HighlightFace(const int faceIndex)
{
  // Prepare a fictive collection
  TColStd_PackedMapOfInteger faceIndices;
  faceIndices.Add(faceIndex);

  // Highlight
  HighlightFaces(faceIndices);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::HighlightFaces(const TColStd_PackedMapOfInteger& faceIndices)
{
  // Convert face indices to sub-shape indices
  TColStd_PackedMapOfInteger ssIndices;
  GetSubShapeIndicesByFaceIndices(faceIndices, ssIndices);

  // Highlight
  HighlightSubShapes(ssIndices, SelectionMode_Face);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::HighlightEdges(const TColStd_PackedMapOfInteger& edgeIndices)
{
  // Convert edge indices to sub-shape indices
  TColStd_PackedMapOfInteger ssIndices;
  GetSubShapeIndicesByEdgeIndices(edgeIndices, ssIndices);

  // Highlight
  HighlightSubShapes(ssIndices, SelectionMode_Edge);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::HighlightVertices(const TColStd_PackedMapOfInteger& vertexIndices)
{
  // Convert vertex indices to sub-shape indices
  TColStd_PackedMapOfInteger ssIndices;
  GetSubShapeIndicesByVertexIndices(vertexIndices, ssIndices);

  // Highlight
  HighlightSubShapes(ssIndices, SelectionMode_Vertex);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::HighlightSubShapes(const TColStd_PackedMapOfInteger& subShapeIndices,
                                        const asiVisu_SelectionMode       selMode)
{
  // Get Part Node.
  Handle(asiData_PartNode) N = m_model->GetPartNode();

  // Get Presentation for the Part Node.
  Handle(asiVisu_PartPrs)
    prs = Handle(asiVisu_PartPrs)::DownCast( m_prsMgr->GetPresentation(N) );

  if ( prs.IsNull() )
    return;

  // Make sure to restore the previous selection mode.
  const int prevMode = m_prsMgr->GetCurrentSelection().GetSelectionModes();
  {
    // Highlight
    if ( selMode == SelectionMode_Face )
      m_prsMgr->Highlight(N, prs->MainActor(), subShapeIndices, selMode);
    else if ( (selMode == SelectionMode_Edge) || (selMode == SelectionMode_Vertex) )
      m_prsMgr->Highlight(N, prs->ContourActor(), subShapeIndices, selMode);
  }
  m_prsMgr->ChangeCurrentSelection().SetSelectionModes(prevMode);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::HighlightSubShapes(const TopTools_IndexedMapOfShape& subShapes)
{
  // Get global relative indices of the sub-shapes in the CAD model
  TColStd_PackedMapOfInteger selectedFaces, selectedEdges, selectedVertices;
  asiEngine_Part::GetSubShapeIndices(subShapes, selectedFaces, selectedEdges, selectedVertices);

  // Highlight
  if ( !selectedFaces.IsEmpty() )
    HighlightSubShapes(selectedFaces, SelectionMode_Face);
  //
  if ( !selectedEdges.IsEmpty() )
    HighlightSubShapes(selectedEdges, SelectionMode_Edge);
  //
  if ( !selectedVertices.IsEmpty() )
    HighlightSubShapes(selectedVertices, SelectionMode_Vertex);
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetHighlightedSubShapes(TopTools_IndexedMapOfShape& subShapes)
{
  Handle(asiAlgo_AAG) aag = m_model->GetPartNode()->GetAAG();
  //
  if ( aag.IsNull() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "AAG is null.");
    return;
  }

  if ( !m_prsMgr.GetPointer() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Presentation manager is null.");
    return;
  }

  // Get the map of ALL shapes to extract topology by selected index which
  // is global (related to full accessory graph)
  const TopTools_IndexedMapOfShape& M = aag->RequestMapOfSubShapes();

  // Get actual selection
  const asiVisu_ActualSelection&          sel      = m_prsMgr->GetCurrentSelection();
  const Handle(asiVisu_CellPickerResult)& pick_res = sel.GetCellPickerResult(SelectionNature_Persistent);
  //
  asiVisu_PartNodeInfo* nodeInfo = asiVisu_PartNodeInfo::Retrieve( pick_res->GetPickedActor() );
  //
  if ( !nodeInfo )
    return;

  const TColStd_PackedMapOfInteger& subshape_mask = pick_res->GetPickedElementIds();
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger mit(subshape_mask); mit.More(); mit.Next() )
  {
    const int           subshape_idx = mit.Key();
    const TopoDS_Shape& subshape     = M.FindKey(subshape_idx);
    //
    subShapes.Add(subshape);
  }
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetHighlightedFaces(TColStd_PackedMapOfInteger& faceIndices)
{
  TopTools_IndexedMapOfShape subShapes;
  GetHighlightedSubShapes(subShapes);
  //
  if ( subShapes.IsEmpty() )
    return;

  // Get part
  Handle(asiData_PartNode) part_n = m_model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
    return;

  // Get AAG
  Handle(asiAlgo_AAG) aag = part_n->GetAAG();
  //
  if ( aag.IsNull() )
    return;

  // Take all faces
  const TopTools_IndexedMapOfShape& allFaces = aag->GetMapOfFaces();

  // Filter out non-selected faces
  for ( int f = 1; f <= allFaces.Extent(); ++f )
  {
    if ( subShapes.Contains( allFaces(f) ) )
      faceIndices.Add(f);
  }
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetHighlightedEdges(TColStd_PackedMapOfInteger& edgeIndices)
{
  TopTools_IndexedMapOfShape subShapes;
  GetHighlightedSubShapes(subShapes);
  //
  if ( subShapes.IsEmpty() )
    return;

  // Take all edges
  const TopTools_IndexedMapOfShape&
    allEdges = m_model->GetPartNode()->GetAAG()->RequestMapOfEdges();

  // Filter out non-selected edges
  for ( int e = 1; e <= allEdges.Extent(); ++e )
  {
    if ( subShapes.Contains( allEdges(e) ) )
      edgeIndices.Add(e);
  }
}

//-----------------------------------------------------------------------------

void asiEngine_Part::GetHighlightedVertices(TColStd_PackedMapOfInteger& vertIndices)
{
  TopTools_IndexedMapOfShape subShapes;
  GetHighlightedSubShapes(subShapes);
  //
  if ( subShapes.IsEmpty() )
    return;

  // Take all vertices
  const TopTools_IndexedMapOfShape&
    allVertices = m_model->GetPartNode()->GetAAG()->RequestMapOfVertices();

  // Filter out non-selected vertices
  for ( int v = 1; v <= allVertices.Extent(); ++v )
  {
    if ( subShapes.Contains( allVertices(v) ) )
      vertIndices.Add(v);
  }
}

//-----------------------------------------------------------------------------

void asiEngine_Part::TransferMetadata(const asiAsm::xde::PartId&      pid,
                                      const Handle(asiAsm::xde::Doc)& xdeDoc)
{
  Handle(asiData_MetadataNode) N = this->GetMetadata();

  // Get all metadata records.
  asiData_MetadataAttr::t_shapeColorMap shapeColorMap;
  N->GetShapeColorMap(shapeColorMap);

  // Create new metadata records from the collected DTOs.
  for ( int k = 1; k <= shapeColorMap.Extent(); ++k )
  {
    // Get shape and color.
    const TopoDS_Shape& shape  = shapeColorMap.FindKey(k);
    const int           icolor = shapeColorMap.FindFromIndex(k);

    ActAPI_Color color = asiVisu_Utils::IntToColor(icolor);

    // Pass to the XDE document.
    TDF_Label ssLab = xdeDoc->AddSubShape(pid, shape);
    xdeDoc->SetColor(ssLab, color);
  }
}
