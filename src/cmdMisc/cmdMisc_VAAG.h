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

#ifndef cmdMisc_VAAG_h
#define cmdMisc_VAAG_h

// cmdMisc includes
#include <cmdMisc.h>

// asiAlgo includes
#include <asiAlgo_AdjacencyMx.h>
#include <asiAlgo_FeatureAttr.h>

//-----------------------------------------------------------------------------

//! \brief Vertex Attributed Adjacency Graph.
//!
//! This graph represents topology of wireframe models, i.e. the models composed
//! of edges with shared vertices.
class cmdMisc_VAAG : public Standard_Transient
{
public:

  //! Enumerates options for caching maps of shapes.
  enum CachedMap
  {
    CachedMap_SubShapes     = 0x0001,
    CachedMap_Edges         = 0x0002,
    CachedMap_Vertices      = 0x0004,
    CachedMap_VerticesEdges = 0x0008,
    //
    CachedMap_Minimal = CachedMap_Edges,
    CachedMap_All = CachedMap_Edges     |
                    CachedMap_Vertices  |
                    CachedMap_SubShapes |
                    CachedMap_VerticesEdges
  };

  //---------------------------------------------------------------------------

  //! Type definition for map of attributes.
  typedef NCollection_DataMap<Standard_GUID,
                              Handle(asiAlgo_FeatureAttr),
                              Standard_GUID> t_attrMap;

  //---------------------------------------------------------------------------

  //! Arc between two nodes of VAAG. The arc is the explicit representation
  //! for adjacency relation.
  struct t_arc
  {
    t_topoId V1; //!< First vertex.
    t_topoId V2; //!< Second vertex.

    //! Ctor default.
    t_arc() : V1(0), V2(0) {}

    //! Ctor with parameters.
    t_arc(const t_topoId _V1, const t_topoId _V2) : V1(_V1), V2(_V2) {}

    //! \return hash code for the arc.
    static int HashCode(const t_arc& arc, const int upper)
    {
      int key = arc.V1 + arc.V2;
      key += (key << 10);
      key ^= (key >> 6);
      key += (key << 3);
      key ^= (key >> 11);
      return (key & 0x7fffffff) % upper;
    }

    //! \return true if two arcs are equal.
    static int IsEqual(const t_arc& arc1, const t_arc& arc2)
    {
      return ( (arc1.V1 == arc2.V1) && (arc1.V2 == arc2.V2) ) ||
             ( (arc1.V2 == arc2.V1) && (arc1.V1 == arc2.V2) );
    }
  };

  //---------------------------------------------------------------------------

  //! Collection of attributes.
  class t_attr_set
  {
  public:

    //! Convenience iterator for the set of attributes associated with
    //! node or arc in VAAG.
    class Iterator
    {
    public:

      //! Ctor accepting the set of attributes to iterate.
      //! \param[in] attributes set of attributes to iterate.
      Iterator(const t_attr_set& attributes) : m_attrs(attributes)
      {
        m_it.Initialize( m_attrs.GetMap() );
      }

      //! \return true if there is something more to iterate starting from
      //!         current position, false -- otherwise.
      bool More() const
      {
        return m_it.More();
      }

      //! Moves iterator to the next position.
      void Next()
      {
        m_it.Next();
      }

      //! \return GUID of the currently iterated attribute.
      const Standard_GUID& GetGUID() const
      {
        return m_it.Key();
      }

      //! \return currently iterated attribute.
      const Handle(asiAlgo_FeatureAttr)& GetAttr() const
      {
        return m_it.Value();
      }

      //! \return non-const reference to the currently iterated attribute.
      Handle(asiAlgo_FeatureAttr)& ChangeAttr()
      {
        return m_it.ChangeValue();
      }

    protected:

      //! Attributes to iterate over.
      const t_attr_set& m_attrs;

      //! Internal iterator.
      t_attrMap::Iterator m_it;

    private:

      // To avoid C4512 "assignment operator could not be generated".
      Iterator& operator=(const Iterator&) { return *this; }
    };

  public:

    t_attr_set() {} //!< Default ctor.

    //! Constructor accepting a single attribute to populate the internal set.
    //! \param[in] single_attr single attribute to populate the set with.
    t_attr_set(const Handle(asiAlgo_FeatureAttr)& single_attr)
    {
      this->Add(single_attr);
    }

  public:

    //! Finds attribute by its global type ID.
    //! \param[in] id attribute's global ID.
    //! \return attribute instance.
    const Handle(asiAlgo_FeatureAttr)& operator()(const Standard_GUID& id) const
    {
      return m_set(id);
    }

    //! Adds the given attribute to the set.
    //! \param[in] attr attribute to add.
    void Add(const Handle(asiAlgo_FeatureAttr)& attr)
    {
      m_set.Bind(attr->GetGUID(), attr);
    }

    //! Checks whether the set of attributes contains an attribute of the
    //! given type.
    //! \param[in] id attribute's global ID.
    //! \return true in case of success, false -- otherwise.
    bool Contains(const Standard_GUID& id) const
    {
      return m_set.IsBound(id);
    }

    //! Returns pointer to Item by Key.
    //! Returns NULL is Key was not bound.
    //! \param[in] id attribute's global ID.
    //! \return pointer to attribute instance.
    const Handle(asiAlgo_FeatureAttr)* Seek(const Standard_GUID& id) const
    {
      return m_set.Seek(id);
    }

    //! \return internal collection.
    const t_attrMap& GetMap() const
    {
      return m_set;
    }

    //! \return internal collection.
    t_attrMap& ChangeMap()
    {
      return m_set;
    }

  private:

    //! Internal set storing attributes in association with their global IDs.
    t_attrMap m_set;

  };

  //---------------------------------------------------------------------------

  //! Arc attributes.
  typedef NCollection_DataMap<t_arc, Handle(asiAlgo_FeatureAttr), t_arc> t_arc_attributes;

  //! Node attributes.
  typedef NCollection_DataMap<t_topoId, t_attr_set> t_node_attributes;

  //---------------------------------------------------------------------------

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(cmdMisc_VAAG, Standard_Transient)

public:

  /** @name Construction and destruction
   *  Functions to build and destroy VAAG.
   */
  //@{

  //! Initializes VAAG from the given wireframe model.
  //! \param[in] masterCAD  master model (full CAD).
  //! \param[in] cachedMaps flag indicating which maps of sub-shapes to
  //!                       cache. Since building topo maps is costly,
  //!                       it is generally a good idea to reuse them
  //!                       as much as possible. Using this flag you
  //!                       can control which maps will be built.
  cmdMisc_EXPORT
    cmdMisc_VAAG(const TopoDS_Shape& masterCAD,
                 const int           cachedMaps = CachedMap_Minimal);

  //! Dtor.
  asiAlgo_EXPORT
    ~cmdMisc_VAAG();

  //@}

public:

  /** @name Derived graphs
   *  Methods to construct derived graphs and sub-graphs from VAAG.
   */
  //@{

  //! \brief Constructs deep copy of VAAG.
  //! \return copy of this VAAG.
  asiAlgo_EXPORT Handle(cmdMisc_VAAG)
    Copy() const;

  //! \brief Captures sub-graph.
  //!
  //! Prepares a sub-graph containing the passed vertices only. This sub-graph
  //! is pushed to the internal stack of sub-graphs eliminating all neighborhood
  //! relations which are out of interest in the current recognition setting.
  //!
  //! \param[in] vertices2Keep indices of vertices to keep in the model.
  //!
  //! \sa PopSubgraph() method to pop the created sub-graph from the stack.
  asiAlgo_EXPORT void
    PushSubgraph(const asiAlgo_Feature& vertices2Keep);

  //! \brief Captures sub-graph.
  //!
  //! Prepares a sub-graph by removing the passed vertices from the lower graph
  //! in the stack. This sub-graph is then pushed to the internal stack of
  //! sub-graphs eliminating all neighborhood relations which are out of
  //! interest in the current recognition setting.
  //!
  //! \param[in] vertices2Exclude indices of vertices to exclude from the model.
  //!
  //! \sa PopSubgraph() method to pop the created sub-graph from the stack.
  asiAlgo_EXPORT void
    PushSubgraphX(const asiAlgo_Feature& vertices2Exclude);

  //! \brief Captures sub-graph.
  //!
  //! Prepares a sub-graph by removing the passed vertex from the lower graph
  //! in the stack. This sub-graph is then pushed to the internal stack of
  //! sub-graphs eliminating all neighborhood relations which are out of
  //! interest in the current recognition setting.
  //!
  //! \param[in] vertex2Exclude index of the vertex to exclude from the model.
  //!
  //! \sa PopSubgraph() method to pop the created sub-graph from the stack.
  asiAlgo_EXPORT void
    PushSubgraphX(const t_topoId vertex2Exclude);

  //! \brief Pops the top sub-graph from the internal stack.
  //!
  //! Goes back to the parent graph from sub-graph. Use this method to
  //! recover the previous state of VAAG.
  asiAlgo_EXPORT void
    PopSubgraph();

  //! \brief Pops all subgraphs except for the top one.
  //!
  //! Use this method to get rid of any subgraphs that might persist
  //! in the stack.
  asiAlgo_EXPORT void
    PopSubgraphs();

  //@}

public:

  //! \return master CAD shape.
  asiAlgo_EXPORT const TopoDS_Shape&
    GetMasterShape() const;

  //! \return number of graph nodes.
  asiAlgo_EXPORT int
    GetNumberOfNodes() const;

  //! Returns true if the index is in range.
  //! \param[in] vid vertex index.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasVertex(const t_topoId vid) const;

  //! Returns true if the passed vertex is in graph.
  //! \param[in] vertex vertex to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasVertex(const TopoDS_Shape& vertex) const;

  //! Returns topological vertex by its internal index (e.g. coming from iterator).
  //! \param[in] vid vertex index.
  //! \return topological vertex.
  asiAlgo_EXPORT const TopoDS_Vertex&
    GetVertex(const t_topoId vid) const;

  //! Returns vertex ID.
  //! \param[in] vertex vertex of interest.
  //! \return vertex ID.
  asiAlgo_EXPORT t_topoId
    GetVertexId(const TopoDS_Shape& vertex) const;

  //! Checks whether the given vertex has any neighbors recorded in the VAAG.
  //! \param[in] vid vertex index.
  //! \return true in case if at least one neighbor presents, false -- otherwise.
  asiAlgo_EXPORT bool
    HasNeighbors(const t_topoId vid) const;

  //! Returns neighbors for the vertex having the given internal index.
  //! \param[in] vid vertex index.
  //! \return indices of the neighbor vertices.
  asiAlgo_EXPORT const asiAlgo_Feature&
    GetNeighbors(const t_topoId vid) const;

  //! Returns full adjacency matrix.
  //! \return neighborhood data.
  asiAlgo_EXPORT const asiAlgo_AdjacencyMx&
    GetNeighborhood() const;

  //! Returns all vertices of the master model.
  //! \return all vertices.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    GetMapOfVertices() const;

  //! Returns all edges of the master model.
  //! If the map is empty, it is constructed.
  //! \return all edges.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    RequestMapOfEdges();

  //! Returns all subshapes of the master model.
  //! If the map is empty, it is constructed.
  //! \return map of all sub-shapes.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    RequestMapOfSubShapes();

  //! \brief Returns map of indexed sub-shapes of the given type.
  //! If the map is empty, it is constructed.
  //!
  //! \param[in]  ssType sub-shape type (TopAbs_VERTEX, TopAbs_EDGE or TopAbs_FACE).
  //! \param[out] map    requested map of sub-shapes.
  asiAlgo_EXPORT void
    RequestMapOf(const TopAbs_ShapeEnum      ssType,
                 TopTools_IndexedMapOfShape& map);

  //! Returns vertices and their owner edges.
  //! If the map is empty, it is constructed.
  //! \return map of vertices and their owner edges.
  asiAlgo_EXPORT const TopTools_IndexedDataMapOfShapeListOfShape&
    RequestMapOfVerticesEdges();

  //! Checks if the graph contains the passed arc (i.e., the referenced
  //! faces are adjacent).
  //! \param[in] arc graph arc to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasArc(const t_arc& arc) const;

  //! \return attributes associated with graph arcs.
  asiAlgo_EXPORT const t_arc_attributes&
    GetArcAttributes() const;

  //! Accessor for an arc attribute.
  //! \param[in] arc graph arc in question.
  //! \return attribute associated with the given arc.
  asiAlgo_EXPORT const Handle(asiAlgo_FeatureAttr)&
    GetArcAttribute(const t_arc& arc) const;

  //! Checks whether the given node has any attributes or not.
  //! \param[in] node ID of the graph node to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasNodeAttributes(const t_topoId node) const;

  //! Accessor for the entire collection of nodal attributes.
  //! \return attributes associated with all graph node.
  asiAlgo_EXPORT const t_node_attributes&
    GetNodeAttributes() const;

  //! Accessor for the collection of nodal attributes.
  //! \param[in] node ID of the graph node of interest.
  //! \return attributes associated with the given graph node.
  asiAlgo_EXPORT const t_attr_set&
    GetNodeAttributes(const t_topoId node) const;

  //! Returns attribute associated with the given graph node.
  //! \param[in] node    ID of the graph node of interest.
  //! \param[in] attr_id ID of the attribute to access.
  //! \return attribute associated with the given node.
  asiAlgo_EXPORT Handle(asiAlgo_FeatureAttr)
    GetNodeAttribute(const t_topoId       node,
                     const Standard_GUID& attr_id) const;

  //! Removes attribute with the passed GUID from the given graph node.
  //! \param[in] node    ID of the graph node of interest.
  //! \param[in] attr_id ID of the attribute to remove.
  //! \return true if the attribute was removed, false -- otherwise (e.g., if
  //!         such attribute does not exist).
  asiAlgo_EXPORT bool
    RemoveNodeAttribute(const t_topoId       node,
                        const Standard_GUID& attr_id);

  //! Removes all attributes assigned to nodes.
  asiAlgo_EXPORT void
    RemoveNodeAttributes();

  //! Sets the entire collection of nodal attributes.
  //! \param[in] attrs attributes to set.
  asiAlgo_EXPORT void
    SetNodeAttributes(const t_node_attributes& attrs);

  //! Sets the given attribute for a node in AAG. If an attribute of this type
  //! is already there, this method does nothing and returns false.
  //! \param[in] node ID of the graph node of interest.
  //! \param[in] attr attribute to set.
  asiAlgo_EXPORT bool
    SetNodeAttribute(const t_topoId                     node,
                     const Handle(asiAlgo_FeatureAttr)& attr);

  //! Removes the passed faces with all corresponding arcs from AAG.
  //! \param[in] faces faces to remove.
  asiAlgo_EXPORT void
    Remove(const TopTools_IndexedMapOfShape& faces);

  //! Removes the passed faces with all corresponding arcs from AAG.
  //! \param[in] faceIndices indices of faces to remove.
  asiAlgo_EXPORT void
    Remove(const asiAlgo_Feature& faceIndices);

  //! Calculates number of the connected components.
  //! \return number of connected components in a graph.
  asiAlgo_EXPORT int
    GetConnectedComponentsNb();

  //! Calculates the number of the connected components in AAG without the
  //! given faces represented by their indexes.
  //! \param[in] excludedFaceIndices face indices to exclude from consideration.
  //! \return number of connected components in a graph.
  asiAlgo_EXPORT int
    GetConnectedComponentsNb(const asiAlgo_Feature& excludedFaceIndices);

  //! Calculates connected components for the given set of indexes.
  //! \param[in]  seeds seed nodes for the detection.
  //! \param[out] res   found connected components.
  asiAlgo_EXPORT void
    GetConnectedComponents(const asiAlgo_Feature&        seeds,
                           std::vector<asiAlgo_Feature>& res);

  //! Calculates connected components for the full graph.
  //! \param[out] res found connected components.
  asiAlgo_EXPORT void
    GetConnectedComponents(std::vector<asiAlgo_Feature>& res);

  //! \deprecated Kept for compatibility only (use std::vector version instead).
  //!
  //! Calculates connected components for the full graph.
  //! \param[out] res found connected components.
  [[deprecated("Use the overloaded version with std::vector instead.")]]
  asiAlgo_EXPORT void
    GetConnectedComponents(NCollection_Vector<asiAlgo_Feature>& res);

  //! Clears cached maps.
  asiAlgo_EXPORT void
    ClearCache();

  /* Convenience methods */

  //! Returns the casted handle to the node attribute of the template type
  //! for the given face ID.
  //! \param[in] fid face ID in question.
  //! \return attribute.
  template<typename t_attr_type>
  Handle(t_attr_type) ATTR_NODE(const t_topoId fid) const
  {
    return Handle(t_attr_type)::DownCast( this->GetNodeAttribute( fid, t_attr_type::GUID() ) );
  }

  //! Returns the casted handle to the arc attribute of the template type
  //! for the given pair of face IDs.
  //! \param[in] arc face IDs defining the arc in question.
  //! \return attribute.
  template<typename t_attr_type>
  Handle(t_attr_type) ATTR_ARC(const t_arc& arc) const
  {
    return Handle(t_attr_type)::DownCast( this->GetArcAttribute(arc) );
  }

public:

  //! Dumps AAG structure to the passed output stream.
  //! \param[in, out] out target stream.
  asiAlgo_EXPORT void
    Dump(Standard_OStream& out) const;

  //! Dumps AAG structure as JSON.
  //! \param[in,out] out        target stream.
  //! \param[in]     whitespace num of spaces to prefix each row.
  asiAlgo_EXPORT void
    DumpJSON(Standard_OStream& out,
             const int         whitespace = 0) const;

protected:

  //! Initializes graph tool with master CAD.
  //! \param[in] masterCAD  master model (full CAD).
  //! \param[in] cachedMaps flag indicating which maps of sub-shapes to
  //!                       cache. Since building topo maps is costly,
  //!                       it is generally a good idea to reuse them
  //!                       as much as possible. Using this flag you
  //!                       can control which maps will be built.
  asiAlgo_EXPORT void
    init(const TopoDS_Shape& masterCAD,
         const int           cachedMaps);

  //! Fills graph with nodes for mate vertices.
  //! \param[in] mateVertices vertices to add (if not yet added).
  asiAlgo_EXPORT void
    addMates(const TopTools_ListOfShape& mateVertices);

  //! Dumps all graph nodes with their attributes to JSON.
  //! \param[in,out] out        target output stream.
  //! \param[in]     whitespace num of spaces to prefix each row.
  asiAlgo_EXPORT void
    dumpNodesJSON(Standard_OStream& out,
                  const int         whitespace = 0) const;

  //! Dumps single graph node with its attributes to JSON.
  //! \param[in]     node       ID of the graph node to dump.
  //! \param[in]     isFirst    indicates whether the currently dumped node is
  //!                           the first one to correctly put commas.
  //! \param[in,out] out        target output stream.
  //! \param[in]     whitespace num of spaces to prefix each row.
  asiAlgo_EXPORT void
    dumpNodeJSON(const t_topoId    node,
                 const bool        isFirst,
                 Standard_OStream& out,
                 const int         whitespace = 0) const;

  //! Dumps all graph arcs with their attributes to JSON.
  //! \param[in,out] out        target output stream.
  //! \param[in]     whitespace num of spaces to prefix each row.
  asiAlgo_EXPORT void
    dumpArcsJSON(Standard_OStream& out,
                 const int         whitespace = 0) const;

  //! Dumps the given graph arc with its attributes to JSON.
  //! \param[in]     arc        graph arc in question.
  //! \param[in]     isFirst    indicates whether the currently dumped arc is
  //!                           the first one to correctly put commas.
  //! \param[in,out] out        target output stream.
  //! \param[in]     whitespace num of spaces to prefix each row.
  asiAlgo_EXPORT void
    dumpArcJSON(const t_arc&      arc,
                const bool        isFirst,
                Standard_OStream& out,
                const int         whitespace = 0) const;

protected:

  cmdMisc_VAAG() {} //!< Default ctor.

protected:

  //! Master CAD model.
  TopoDS_Shape m_master;

  //! All sub-shapes.
  TopTools_IndexedMapOfShape m_subShapes;

  //! All edges of the master model.
  TopTools_IndexedMapOfShape m_edges;

  //! All vertices of the master model.
  TopTools_IndexedMapOfShape m_vertices;

  //! Map of vertices versus edges.
  TopTools_IndexedDataMapOfShapeListOfShape m_verticesEdges;

  //! The data maps stored in this stack represent adjacency matrices. The
  //! stack is used to keep sub-graphs.
  std::stack<asiAlgo_AdjacencyMx> m_neighborsStack;

  //! Stores attributes associated with each arc.
  t_arc_attributes m_arcAttributes;

  //! Stores attributes associated with nodes.
  t_node_attributes m_nodeAttributes;

  //! Experimental allocator (does it make any sense?).
  Handle(NCollection_IncAllocator) m_alloc;

};

#endif
