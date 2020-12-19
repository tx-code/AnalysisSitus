//-----------------------------------------------------------------------------
// Created on: 12 December 2020
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <cmdMisc_VAAG.h>

// OpenCascade includes
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

cmdMisc_VAAG::cmdMisc_VAAG(const TopoDS_Shape& masterCAD,
                           const int           cachedMaps)
{
  this->init(masterCAD, cachedMaps);
}

//-----------------------------------------------------------------------------

cmdMisc_VAAG::~cmdMisc_VAAG()
{}

//-----------------------------------------------------------------------------

Handle(cmdMisc_VAAG) cmdMisc_VAAG::Copy() const
{
  Handle(cmdMisc_VAAG) copy = new cmdMisc_VAAG;
  //
  copy->m_master         = this->m_master;
  copy->m_subShapes      = this->m_subShapes;
  copy->m_edges          = this->m_edges;
  copy->m_vertices       = this->m_vertices;
  copy->m_neighborsStack = this->m_neighborsStack;
  copy->m_arcAttributes  = this->m_arcAttributes;
  copy->m_nodeAttributes = this->m_nodeAttributes;
  //
  return copy;
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::PushSubgraph(const asiAlgo_Feature& vertices2Keep)
{
  asiAlgo_AdjacencyMx& currentMx = m_neighborsStack.top();

  // Gather all present vertex indices into a single map.
  asiAlgo_Feature allVertices;
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator it(currentMx.mx); it.More(); it.Next() )
    allVertices.Add( it.Key() );

  // Prepare a collection of indices to eliminate.
  asiAlgo_Feature vertices2Exclude;
  vertices2Exclude.Subtraction(allVertices, vertices2Keep);

  // Erase vertices.
  this->PushSubgraphX(vertices2Exclude);
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::PushSubgraphX(const t_topoId vertex2Exclude)
{
  asiAlgo_Feature vertices2Exclude;
  vertices2Exclude.Add(vertex2Exclude);

  // Erase vertex.
  this->PushSubgraphX(vertices2Exclude);
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::PushSubgraphX(const asiAlgo_Feature& vertices2Exclude)
{
  asiAlgo_AdjacencyMx& currentMx = m_neighborsStack.top();
  asiAlgo_AdjacencyMx subgraphMx(m_alloc);

  // Compose new adjacency matrix.
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator it(currentMx.mx); it.More(); it.Next() )
  {
    const t_topoId vid = it.Key();
    //
    if ( vertices2Exclude.Contains(vid) )
      continue;

    asiAlgo_Feature row{ it.Value() };
    row.Subtract(vertices2Exclude);

    subgraphMx.mx.Bind(vid, row);
  }

  // Push sub-graph to stack.
  m_neighborsStack.push(subgraphMx);
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::PopSubgraph()
{
  m_neighborsStack.pop();
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::PopSubgraphs()
{
  while ( m_neighborsStack.size() != 1 )
    m_neighborsStack.pop();
}

//-----------------------------------------------------------------------------

const TopoDS_Shape& cmdMisc_VAAG::GetMasterShape() const
{
  return m_master;
}

//-----------------------------------------------------------------------------

int cmdMisc_VAAG::GetNumberOfNodes() const
{
  const asiAlgo_AdjacencyMx& neighborhood = this->GetNeighborhood();
  //
  return neighborhood.mx.Extent();
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::HasVertex(const t_topoId vid) const
{
  return (vid > 0) && ( vid <= m_vertices.Extent() );
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::HasVertex(const TopoDS_Shape& vertex) const
{
  return m_vertices.Contains(vertex);
}

//-----------------------------------------------------------------------------

const TopoDS_Vertex& cmdMisc_VAAG::GetVertex(const t_topoId vid) const
{
  return TopoDS::Vertex( m_vertices.FindKey(vid) );
}

//-----------------------------------------------------------------------------

t_topoId cmdMisc_VAAG::GetVertexId(const TopoDS_Shape& vertex) const
{
  return m_vertices.FindIndex(vertex);
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::HasNeighbors(const t_topoId vid) const
{
  return m_neighborsStack.top().mx.IsBound(vid);
}

//-----------------------------------------------------------------------------

const asiAlgo_Feature& cmdMisc_VAAG::GetNeighbors(const t_topoId vid) const
{
  return m_neighborsStack.top().mx.Find(vid);
}

//-----------------------------------------------------------------------------

const asiAlgo_AdjacencyMx& cmdMisc_VAAG::GetNeighborhood() const
{
  return m_neighborsStack.top();
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& cmdMisc_VAAG::GetMapOfVertices() const
{
  return m_vertices;
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& cmdMisc_VAAG::RequestMapOfEdges()
{
  if ( m_edges.IsEmpty() )
    TopExp::MapShapes(m_master, TopAbs_EDGE, m_edges);

  return m_edges;
}

//-----------------------------------------------------------------------------

const TopTools_IndexedMapOfShape& cmdMisc_VAAG::RequestMapOfSubShapes()
{
  if ( m_subShapes.IsEmpty() )
    TopExp::MapShapes(m_master, m_subShapes);

  return m_subShapes;
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::RequestMapOf(const TopAbs_ShapeEnum      ssType,
                                TopTools_IndexedMapOfShape& map)
{
  switch ( ssType )
  {
    case TopAbs_VERTEX:
      map = this->GetMapOfVertices();
      break;
    case TopAbs_EDGE:
      map = this->RequestMapOfEdges();
      break;
    default: break;
  }
}

//-----------------------------------------------------------------------------

const TopTools_IndexedDataMapOfShapeListOfShape&
  cmdMisc_VAAG::RequestMapOfVerticesEdges()
{
  if ( m_verticesEdges.IsEmpty() )
    TopExp::MapShapesAndAncestors(m_master, TopAbs_VERTEX, TopAbs_EDGE, m_edgesFaces);

  return m_verticesEdges;
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::HasArc(const t_arc& arc) const
{
  const asiAlgo_AdjacencyMx& mx = m_neighborsStack.top();

  // Seek for adjacency record.
  const asiAlgo_Feature* pRow = mx.mx.Seek(arc.V1);
  //
  if ( !pRow ) return false;

  return pRow->Contains(arc.V2);
}

//-----------------------------------------------------------------------------

const cmdMisc_VAAG::t_arc_attributes&
  cmdMisc_VAAG::GetArcAttributes() const
{
  return m_arcAttributes;
}

//-----------------------------------------------------------------------------

const Handle(asiAlgo_FeatureAttr)&
  cmdMisc_VAAG::GetArcAttribute(const t_arc& arc) const
{
  return m_arcAttributes.Find(arc);
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::HasNodeAttributes(const t_topoId node) const
{
  return m_nodeAttributes.IsBound(node);
}

//-----------------------------------------------------------------------------

const cmdMisc_VAAG::t_node_attributes& cmdMisc_VAAG::GetNodeAttributes() const
{
  return m_nodeAttributes;
}

//-----------------------------------------------------------------------------

const cmdMisc_VAAG::t_attr_set&
  cmdMisc_VAAG::GetNodeAttributes(const t_topoId node) const
{
  return m_nodeAttributes(node);
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_FeatureAttr)
  cmdMisc_VAAG::GetNodeAttribute(const t_topoId       node,
                                 const Standard_GUID& attr_id) const
{
  const t_attr_set* attrSetPtr = m_nodeAttributes.Seek(node);
  if ( attrSetPtr == nullptr )
    return nullptr;

  const Handle(asiAlgo_FeatureAttr)* attrPtr = (*attrSetPtr).Seek(attr_id);
  if ( attrPtr == nullptr )
    return nullptr;

  return (*attrPtr);
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::RemoveNodeAttribute(const t_topoId       node,
                                       const Standard_GUID& attr_id)
{
  t_attr_set* attrSetPtr = m_nodeAttributes.ChangeSeek(node);
  if ( attrSetPtr == nullptr )
    return false;

  const Handle(asiAlgo_FeatureAttr)* attrPtr = (*attrSetPtr).Seek(attr_id);
  if ( attrPtr == nullptr )
    return false;

  return (*attrSetPtr).ChangeMap().UnBind(attr_id);
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::RemoveNodeAttributes()
{
  m_nodeAttributes.Clear();
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::SetNodeAttributes(const t_node_attributes& attrs)
{
  m_nodeAttributes = attrs;
}

//-----------------------------------------------------------------------------

bool cmdMisc_VAAG::SetNodeAttribute(const t_topoId                     node,
                                    const Handle(asiAlgo_FeatureAttr)& attr)
{
  if ( attr.IsNull() )
    return false;

  Handle(asiAlgo_FeatureAttr) existing = this->GetNodeAttribute( node, attr->GetGUID() );
  //
  if ( !existing.IsNull() )
    return false; // Already there

  // Set owner AAG
  attr->setAAG(this);

  // Set face ID to the attribute representing a feature face
  if ( attr->IsKind( STANDARD_TYPE(asiAlgo_FeatureAttrFace) ) )
    Handle(asiAlgo_FeatureAttrFace)::DownCast(attr)->SetFaceId(node);

  t_attr_set* attrSetPtr = m_nodeAttributes.ChangeSeek(node);
  if ( attrSetPtr == nullptr )
    m_nodeAttributes.Bind( node, t_attr_set(attr) );
  else
    (*attrSetPtr).Add(attr);

  return true;
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::Remove(const TopTools_IndexedMapOfShape& vertices)
{
  // NOTICE: indexed map of shapes is not affected as we want to keep
  //         using the original indices of vertices.

  // Find IDs of the vertices to remove.
  asiAlgo_Feature toRemove;
  for ( t_topoId v = 1; v <= vertices.Extent(); ++v )
  {
    const t_topoId vid = this->GetVertexId( vertices.FindKey(v) );
    toRemove.Add(vid);
  }

  // Remove by indices.
  this->Remove(toRemove);
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::Remove(const asiAlgo_Feature& vertexIndices)
{
  // NOTICE: indexed map of shapes is not affected as we want to keep
  //         using the original indices of vertices.

  // Loop over the target vertices.
  for ( asiAlgo_Feature::Iterator vit(vertexIndices); vit.More(); vit.Next() )
  {
    const t_topoId vid = vit.Key();

    // Unbind node attributes.
    m_nodeAttributes.UnBind(vid);

    // Find all neighbors.
    const asiAlgo_Feature& nids = m_neighborsStack.top().mx.Find(vid);
    for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
    {
      const t_topoId nid = nit.Key();

      // Unbind arc attributes.
      m_arcAttributes.UnBind( t_arc(vid, nid) );

      // Kill the corresponding chunks from the list of neighbors.
      asiAlgo_Feature* mapPtr = m_neighborsStack.top().mx.ChangeSeek(nid);
      if ( mapPtr != nullptr )
        (*mapPtr).Subtract(vertexIndices);
    }

    // Unbind node.
    m_neighborsStack.top().mx.UnBind(vid);
  }
}

//-----------------------------------------------------------------------------

int asiAlgo_AAG::GetConnectedComponentsNb()
{
  std::vector<asiAlgo_Feature> ccomps;
  this->GetConnectedComponents(ccomps);

  return int( ccomps.size() );
}

//-----------------------------------------------------------------------------

int asiAlgo_AAG::GetConnectedComponentsNb(const asiAlgo_Feature& excludedFaceIndices)
{
  Handle(asiAlgo_AAG) aagCopy = this->Copy();
  aagCopy->Remove(excludedFaceIndices);
  return aagCopy->GetConnectedComponentsNb();
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::GetConnectedComponents(const asiAlgo_Feature&        seeds,
                                         std::vector<asiAlgo_Feature>& res)
{
  res.clear();

  Handle(asiAlgo_AAGSetIterator) seed_it = new asiAlgo_AAGSetIterator(this, seeds);
  asiAlgo_Feature traversed;

  for ( ; seed_it->More() ; seed_it->Next() )
  {
    // Get seed face
    const t_topoId seed_face_id = seed_it->GetFaceId();

    if ( traversed.Contains(seed_face_id) )
      continue; // Skip checked nodes

    traversed.Add(seed_face_id);
    res.push_back( asiAlgo_Feature() );
    res.back().Add(seed_face_id);

    // Width-first search
    asiAlgo_Feature seed_neighbor_ids = this->GetNeighbors(seed_face_id);
    asiAlgo_Feature seed_neighbor_next_iter;

    do
    {
      seed_neighbor_next_iter.Clear();

      for ( asiAlgo_Feature::Iterator nit(seed_neighbor_ids); nit.More(); nit.Next() )
      {
        const t_topoId  seed_face_id_new       = nit.Key();
        asiAlgo_Feature seed_neighbor_ids_cand = this->GetNeighbors(seed_face_id_new);

        if ( !seeds.Contains(seed_face_id_new) )
          continue; // Skip

        traversed.Add(seed_face_id_new);

        // Set faces for the next iteration
        seed_neighbor_ids_cand.Subtract(traversed);
        seed_neighbor_ids_cand.Intersect(seeds);
        seed_neighbor_next_iter.Unite(seed_neighbor_ids_cand);
        res.back().Add(seed_face_id_new);
      }

      seed_neighbor_ids = seed_neighbor_next_iter;
    }
    while ( seed_neighbor_ids.Extent() != 0 );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::ClearCache()
{
  m_edges.Clear();
  m_vertices.Clear();
  m_subShapes.Clear();
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::GetConnectedComponents(std::vector<asiAlgo_Feature>& res)
{
  // Gather all present face indices into a single map.
  asiAlgo_Feature allFaces;
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator it( m_neighborsStack.top().mx );
        it.More(); it.Next() )
  {
    const t_topoId face = it.Key();
    //
    allFaces.Add(face);
  }

  // Collect connected components using all faces as seeds.
  this->GetConnectedComponents(allFaces, res);
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::GetConnectedComponents(NCollection_Vector<asiAlgo_Feature>& res)
{
  std::vector<asiAlgo_Feature> ccomps;
  //
  this->GetConnectedComponents(ccomps);

  // Repack from the standard vector to the OpenCascade's collection.
  for ( auto cit = ccomps.cbegin(); cit != ccomps.cend(); ++cit )
    res.Append(*cit);
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::Dump(Standard_OStream& out) const
{
  out << "===================================================\n";
  out << "***AAG structure\n";
  out << "---------------------------------------------------\n";
  out << " Adjacency\n";
  out << "---------------------------------------------------\n";

  // Dump neighborhood
  for ( t_topoId f = 1; f <= m_faces.Extent(); ++f )
  {
    out << "\t" << f << " -> ";
    const asiAlgo_Feature& neighbors = this->GetNeighbors(f);
    //
    for ( asiAlgo_Feature::Iterator nit(neighbors); nit.More(); nit.Next() )
    {
      out << nit.Key() << " ";
    }
    out << "\n";
  }

  // Dump arc attributes
  out << "---------------------------------------------------\n";
  out << " Node attributes\n";
  out << "---------------------------------------------------\n";
  for ( t_topoId f = 1; f <= m_faces.Extent(); ++f )
  {
    if ( !this->HasNodeAttributes(f) )
      continue;

    const t_attrMap& attrs = this->GetNodeAttributes(f).GetMap();
    //
    if ( attrs.IsEmpty() )
      continue;

    out << "\t" << f << " ~ ";
    //
    for ( t_attrMap::Iterator ait(attrs); ait.More(); ait.Next() )
    {
      out << "[" << ait.Value()->DynamicType()->Name() << "]\n";
      out << ">>>\n";
      ait.Value()->Dump(out);
      out << "\n<<<\n";
    }
    out << "\n";
  }
  out << "===================================================\n";
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::DumpJSON(Standard_OStream& out,
                           const int         whitespaces) const
{
  std::string prefix(whitespaces, ' ');

  out << std::setprecision( std::numeric_limits<double>::max_digits10 );
  out << prefix << "{";
  out << "\n" << prefix << "  \"nodes\": {";
  //
  this->dumpNodesJSON(out, whitespaces);
  //
  out << "\n" << prefix << "  },"; // End 'nodes'.
  //
  out << "\n" << prefix << "  \"arcs\": [";
  //
  this->dumpArcsJSON(out, whitespaces);
  //
  out << "\n" << prefix << "  ]"; // End 'arcs'.
  out << "\n" << prefix << "}";
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::init(const TopoDS_Shape& masterCAD,
                        const int           cachedMaps)
{
  // Prepare allocator.
  m_alloc = new NCollection_IncAllocator;

  // Set basic members.
  m_master = masterCAD;

  //---------------------------------------------------------------------------

  // Put main adjacency matrix to the stack of graph states.
  m_neighborsStack.push( asiAlgo_AdjacencyMx(m_alloc) );

  //---------------------------------------------------------------------------

  // Extract all sub-shapes with unique indices from the master CAD.
  if ( cachedMaps & CachedMap_SubShapes )
    TopExp::MapShapes(masterCAD, m_subShapes);

  // Extract all vertices with unique indices from the master CAD.
  if ( cachedMaps & CachedMap_Vertices )
    TopExp::MapShapes(masterCAD, TopAbs_VERTEX, m_vertices);

  // Extract all edges with unique indices from the master CAD.
  if ( cachedMaps & CachedMap_Edges )
    TopExp::MapShapes(masterCAD, TopAbs_EDGE, m_edges);

  // Build child-parent map for vertices and their edges.
  if ( cachedMaps & CachedMap_VerticesEdges )
    TopExp::MapShapesAndAncestors(masterCAD, TopAbs_VERTEX, TopAbs_EDGE, m_verticesEdges);

  // Fill adjacency map with empty buckets.
  for ( t_topoId v = 1; v <= m_vertices.Extent(); ++v )
  {
    m_neighborsStack.top().mx.Bind( v, asiAlgo_Feature() );
  }

  //---------------------------------------------------------------------------

  const TopTools_IndexedDataMapOfShapeListOfShape&
    ChildParentMap = this->RequestMapOfVerticesEdges();

  // Build adjacency graph.
  for ( TopExp_Explorer exp(masterCAD, TopAbs_VERTEX); exp.More(); exp.Next() )
  {
    const TopoDS_Edge&          commonEdge    = TopoDS::Edge( exp.Current() );
    const TopTools_ListOfShape& adjacentFaces = ChildParentMap.FindFromKey(commonEdge);
    //
    this->addMates(adjacentFaces);
  }

  // Set selected faces
  this->SetSelectedFaces(selectedFaces);
}

//-----------------------------------------------------------------------------

void cmdMisc_VAAG::addMates(const TopTools_ListOfShape& mateVertices)
{
  // Prepare dihedral angle calculation.
  asiAlgo_CheckDihedralAngle checkDihAngle(nullptr, nullptr);

  // Now analyze the face pairs
  for ( TopTools_ListIteratorOfListOfShape lit(mateFaces); lit.More(); lit.Next() )
  {
    const t_topoId     face_idx   = m_faces.FindIndex( lit.Value() );
    asiAlgo_Feature&   face_links = m_neighborsStack.top().mx.ChangeFind(face_idx);
    const TopoDS_Face& face       = TopoDS::Face( m_faces.FindKey(face_idx) );

    // Add all the rest faces as neighbors.
    for ( TopTools_ListIteratorOfListOfShape lit2(mateFaces); lit2.More(); lit2.Next() )
    {
      const t_topoId linked_face_idx = m_faces.FindIndex( lit2.Value() );

      if ( linked_face_idx == face_idx )
        continue; // Skip the same index to avoid loop arcs in the graph.

      if ( face_links.Contains(linked_face_idx) )
        continue;

      face_links.Add(linked_face_idx);

      // The graph is not oriented, so we do not want to compute arc
      // attribute G-F is previously we have already done F-G attribution.
      t_arc arc(face_idx, linked_face_idx);
      if ( m_arcAttributes.IsBound(arc) )
        continue;

      //-----------------------------------------------------------------------
      // Associate attributes
      //-----------------------------------------------------------------------

      const TopoDS_Face& linked_face = TopoDS::Face( m_faces.FindKey(linked_face_idx) );
      //
      TopTools_IndexedMapOfShape commonEdges;

      // Here we let client code decide whether to allow smooth transitions
      // or not. Smooth transition normally requires additional processing
      // in order to classify feature angle as concave or convex.
      double angRad = 0.0;
      //
      const asiAlgo_FeatureAngleType
        angle = checkDihAngle.AngleBetweenFaces(face,
                                                linked_face,
                                                m_bAllowSmooth,
                                                m_fSmoothAngularTol,
                                                commonEdges,
                                                angRad);

      // Convert transient edge pointers to a collection of indices
      asiAlgo_Feature commonEdgeIndices;
      //
      for ( t_topoId eidx = 1; eidx <= commonEdges.Extent(); ++eidx )
      {
        const t_topoId
          globalEdgeIdx = this->RequestMapOfEdges().FindIndex( commonEdges(eidx) );
        //
        commonEdgeIndices.Add(globalEdgeIdx);
      }

      // Create attribute
      Handle(asiAlgo_FeatureAttr)
        attrAngle = new asiAlgo_FeatureAttrAngle(angle, angRad, commonEdgeIndices);

      // Set owner
      attrAngle->setAAG(this);

      // Bind
      m_arcAttributes.Bind(arc, attrAngle);
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpNodesJSON(Standard_OStream& out,
                                const int         whitespaces) const
{
  int nidx = 0;
  //
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator nit( m_neighborsStack.top().mx );
        nit.More(); nit.Next(), ++nidx )
  {
    const t_topoId nodeId = nit.Key();
    //
    this->dumpNodeJSON(nodeId, nidx == 0, out, whitespaces);
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpNodeJSON(const t_topoId    node,
                               const bool        isFirst,
                               Standard_OStream& out,
                               const int         whitespaces) const
{
  std::string prefix(whitespaces, ' ');

  // One attribute which should always be dumped is the surface type.
  std::string
    surfName = asiAlgo_Utils::SurfaceName( BRep_Tool::Surface( this->GetFace(node) ) );

  if ( !isFirst )
    out << ",";
  //
  out << "\n" << prefix << "    \"" << node << "\": {";
  out << "\n" << prefix << "      \"surface\": \"" << surfName << "\"";
  //
  if ( this->HasNodeAttributes(node) )
  {
    out << ",\n" << prefix << "      \"attributes\": [";

    // Dump attributes.
    const t_attr_set& attrs = this->GetNodeAttributes(node);
    //
    int attridx = 0;
    //
    for ( t_attr_set::Iterator ait(attrs); ait.More(); ait.Next(), ++attridx )
    {
      if ( attridx != 0 )
        out << ",";

      ait.GetAttr()->DumpJSON(out, 8 + whitespaces);
    }

    out << "\n" << prefix << "      ]";
  }
  //
  out << "\n" << prefix << "    }";
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpArcsJSON(Standard_OStream& out,
                               const int         whitespaces) const
{
  // Map to filter out the already visited arcs.
  NCollection_Map<t_arc, t_arc> visited;

  int arcidx = 0;
  //
  for ( asiAlgo_AdjacencyMx::t_mx::Iterator it( m_neighborsStack.top().mx );
        it.More(); it.Next() )
  {
    const t_topoId f_idx = it.Key();

    // Get neighbors.
    const asiAlgo_Feature& localNeighbors = it.Value();

    // Dump arc for each neighbor.
    for ( asiAlgo_Feature::Iterator mit(localNeighbors); mit.More(); mit.Next(), ++arcidx )
    {
      const t_topoId neighbor_f_idx = mit.Key();

      // Check if the arc was not traversed before.
      t_arc arc(f_idx, neighbor_f_idx);
      //
      if ( visited.Contains(arc) )
        continue;
      //
      visited.Add(arc);

      // Dump arc.
      this->dumpArcJSON(arc, arcidx == 0, out, whitespaces);
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_AAG::dumpArcJSON(const t_arc&      arc,
                              const bool        isFirst,
                              Standard_OStream& out,
                              const int         whitespaces) const
{
  std::string prefix(whitespaces, ' ');

  Handle(asiAlgo_FeatureAttr) arcAttr = this->GetArcAttribute(arc);
  //
  Handle(asiAlgo_FeatureAttrAngle)
    arcAttrAngle = Handle(asiAlgo_FeatureAttrAngle)::DownCast(arcAttr);

  // Prepare a label for the angle type.
  std::string angleTypeStr;
  //
  if ( arcAttrAngle->GetAngleType() == FeatureAngleType_Convex )
    angleTypeStr = "convex";
  else if ( arcAttrAngle->GetAngleType() == FeatureAngleType_Concave )
    angleTypeStr = "concave";
  else if ( arcAttrAngle->GetAngleType() == FeatureAngleType_Smooth )
    angleTypeStr = "smooth";
  else if ( arcAttrAngle->GetAngleType() == FeatureAngleType_SmoothConcave )
    angleTypeStr = "smooth concave";
  else if ( arcAttrAngle->GetAngleType() == FeatureAngleType_SmoothConvex )
    angleTypeStr = "smooth convex";
  else
    angleTypeStr = "undefined";

  // Prepare a label for the angle value (degrees).
  std::string angleDegStr = asiAlgo_Utils::Str::ToString<double>(arcAttrAngle->GetAngleRad() * 180. / M_PI);

  // Dump to the stream.
  if ( !isFirst )
    out << ",";
  //
  out << "\n" << prefix << "    [\"" << arc.F1 << "\", \""
                                     << arc.F2 << "\", \""
                                     << angleTypeStr << "\", "
                                     << angleDegStr << "]";
}
