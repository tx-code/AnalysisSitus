//-----------------------------------------------------------------------------
// Created on: 26 February 2016
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

#ifndef asiAlgo_AAG_h
#define asiAlgo_AAG_h

// asiAlgo includes
#include <asiAlgo_FeatureAttr.h>
#include <asiAlgo_Naming.h>
#include <asiAlgo_Utils.h>

// STL includes
#include <stack>

// OCCT includes
#include <NCollection_DataMap.hxx>
#include <NCollection_Vector.hxx>
#include <Standard_OStream.hxx>
#include <TColStd_PackedMapOfInteger.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

class asiAlgo_AAGRandomIterator;

//-----------------------------------------------------------------------------

//! \brief Attributed Adjacency Graph for faces of a CAD model.
//!
//! Attributed Adjacency Graph (AAG) is a complementary structure for
//! B-Rep allowing for extension of geometry with semantics (features).
//! Some details on the structure can be found in the following paper:
//!
//! [Joshi S., Chang T.C. Graph-based heuristics for recognition of machined
//! features from a 3D solid model // Comput. Des. 1988. Vol. 20, N 2. P. 58-66.]
//!
//! AAG is a structure to store semantics of a CAD model just like TopoDS_Shape
//! is a structure to store its geometric form. Using AAG gives the following
//! advantages:
//!
//! - AAG gives a formal graph view on the CAD model. This enables using graph
//!   algorithms such as finding connectivity components, search, etc.
//!
//! - AAG caches serial indices of sub-shapes. Therefore, it is not necessary
//!   to use expensive TopExp utilities for a CAD model being unchanged as
//!   an algorithm runs.
//!
//! - AAG extends B-Rep with attributes to represent semantics of the model.
//!   Therefore, this structure is a storage for features recognized from
//!   geometry.
//!
//! - AAG is a convenience structure for traversing adjacent faces. The adjacency
//!   relation is explicit in AAG by definition.
//!
//! AAG is a main working structure for feature recognition. Any feature
//! recognition algorithm accepts or constructs AAG for operation.
class asiAlgo_AAG : public Standard_Transient
{
public:

  //! Enumerates options for caching maps of shapes.
  enum CachedMap
  {
    CachedMap_SubShapes  = 0x0001,
    CachedMap_Faces      = 0x0002,
    CachedMap_Edges      = 0x0004,
    CachedMap_Vertices   = 0x0008,
    CachedMap_EdgesFaces = 0x0010,
    //
    CachedMap_Minimal = CachedMap_Faces,
    CachedMap_All = CachedMap_Faces     |
                    CachedMap_Edges     |
                    CachedMap_Vertices  |
                    CachedMap_SubShapes |
                    CachedMap_EdgesFaces
  };

  //---------------------------------------------------------------------------

  //! Type definition for adjacency matrix.
  typedef NCollection_DataMap<int, TColStd_PackedMapOfInteger> t_adjacency;

  //! Type definition for map of attributes.
  typedef NCollection_DataMap<Standard_GUID,
                              Handle(asiAlgo_FeatureAttr),
                              Standard_GUID> t_attrMap;

  //---------------------------------------------------------------------------

  //! Arc between two nodes of AAG. The arc is the explicit representation
  //! for adjacency relation.
  struct t_arc
  {
    int F1; //!< First face.
    int F2; //!< Second face.

    //! ctor default.
    t_arc() : F1(0), F2(0) {}

    //! ctor with parameters.
    t_arc(const int _F1, const int _F2) : F1(_F1), F2(_F2) {}

    //! \return hash code for the arc.
    static int HashCode(const t_arc& arc, const int upper)
    {
      int key = arc.F1 + arc.F2;
      key += (key << 10);
      key ^= (key >> 6);
      key += (key << 3);
      key ^= (key >> 11);
      return (key & 0x7fffffff) % upper;
    }

    //! \return true if two links are equal.
    static int IsEqual(const t_arc& arc1, const t_arc& arc2)
    {
      return ( (arc1.F1 == arc2.F1) && (arc1.F2 == arc2.F2) ) ||
             ( (arc1.F2 == arc2.F1) && (arc1.F1 == arc2.F2) );
    }
  };

  //---------------------------------------------------------------------------

  //! Collection of attributes.
  class t_attr_set
  {
  public:

    //! Convenience iterator for the set of attributes associated with
    //! node or arc in AAG.
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
  typedef NCollection_DataMap<int, t_attr_set> t_node_attributes;

  //---------------------------------------------------------------------------

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_AAG, Standard_Transient)

public:

  /** @name Construction and destruction
   *  Functions to build and destroy AAG.
   */
  //@{

  //! Initializes AAG from the given master model and selected faces.
  //! \param[in] masterCAD        master model (full CAD).
  //! \param[in] selectedFaces    selected faces.
  //! \param[in] allowSmooth      indicates whether to allow "smooth" value for
  //!                             arc attribute. This value means that the
  //!                             joint between faces is at least G1, so in
  //!                             order to calculate dihedral angle, the neighborhood
  //!                             of transition has to be analyzed. The latter
  //!                             analysis introduces additional cost, so you
  //!                             may disable it if you are Ok to attribute G0
  //!                             joints only.
  //! \param[in] smoothAngularTol angular tolerance used for recognition
  //!                             of smooth dihedral angles. A smooth angle
  //!                             may appear to be imperfect by construction,
  //!                             but still smooth by the design intent. With
  //!                             this parameter you're able to control it.
  //! \param[in] cachedMaps       flag indicating which maps of sub-shapes to
  //!                             cache. Since building topo maps is costly,
  //!                             it is generally a good idea to reuse them
  //!                             as much as possible. Using this flag you
  //!                             can control which maps will be built.
  asiAlgo_EXPORT
    asiAlgo_AAG(const TopoDS_Shape&               masterCAD,
                const TopTools_IndexedMapOfShape& selectedFaces,
                const bool                        allowSmooth      = false,
                const double                      smoothAngularTol = 1.e-4,
                const int                         cachedMaps       = CachedMap_Minimal);

  //! Constructor accepting master CAD only.
  //! \param[in] masterCAD        master CAD.
  //! \param[in] allowSmooth      indicates whether "smooth" attribution for arcs
  //!                             is allowed (true) or not (false).
  //! \param[in] smoothAngularTol angular tolerance used for recognition
  //!                             of smooth dihedral angles. A smooth angle
  //!                             may appear to be imperfect by construction,
  //!                             but still smooth by the design intent. With
  //!                             this parameter you're able to control it.
  //! \param[in] cachedMaps       flag indicating which maps of sub-shapes to
  //!                             cache. Since building topo maps is costly,
  //!                             it is generally a good idea to reuse them
  //!                             as much as possible. Using this flag you
  //!                             can control which maps will be built.
  asiAlgo_EXPORT
    asiAlgo_AAG(const TopoDS_Shape& masterCAD,
                const bool          allowSmooth      = false,
                const double        smoothAngularTol = 1.e-4,
                const int           cachedMaps       = CachedMap_Minimal);

  //! Destructor.
  asiAlgo_EXPORT
    ~asiAlgo_AAG();

  //@}

public:

  //! Sets diagnostic tools to the AAG.
  //! \param[in] progress progress notifier to set.
  //! \param[in] plotter  imperative plotter to set.
  void SetDiagnosticTools(ActAPI_ProgressEntry progress,
                          ActAPI_PlotterEntry  plotter)
  {
    m_progress = progress;
    m_plotter  = plotter;
  }

public:

  /** @name Derived graphs
   *  Methods to construct derived graphs and sub-graphs from AAG.
   */
  //@{

  //! \brief Constructs deep copy of AAG.
  //! \return copy of this AAG.
  asiAlgo_EXPORT Handle(asiAlgo_AAG)
    Copy() const;

  //! \brief Captures sub-graph.
  //!
  //! Prepares a sub-graph containing the passed faces only. This sub-graph
  //! is pushed to the internal stack of sub-graphs eliminating all neighborhood
  //! relations which are out of interest in the current recognition setting.
  //!
  //! \param[in] faces2Keep indices of faces to keep in the model.
  //!
  //! \sa PopSubgraph() method to pop the created sub-graph from the stack.
  asiAlgo_EXPORT void
    PushSubgraph(const TColStd_PackedMapOfInteger& faces2Keep);

  //! \brief Captures sub-graph.
  //!
  //! Prepares a sub-graph by removing the passed faces from the lower graph
  //! in the stack. This sub-graph is then pushed to the internal stack of
  //! sub-graphs eliminating all neighborhood relations which are out of
  //! interest in the current recognition setting.
  //!
  //! \param[in] faces2Exclude indices of faces to exclude from the model.
  //!
  //! \sa PopSubgraph() method to pop the created sub-graph from the stack.
  asiAlgo_EXPORT void
    PushSubgraphX(const TColStd_PackedMapOfInteger& faces2Exclude);

  //! \brief Captures sub-graph.
  //!
  //! Prepares a sub-graph by removing the passed face from the lower graph
  //! in the stack. This sub-graph is then pushed to the internal stack of
  //! sub-graphs eliminating all neighborhood relations which are out of
  //! interest in the current recognition setting.
  //!
  //! \param[in] face2Exclude index of face to exclude from the model.
  //!
  //! \sa PopSubgraph() method to pop the created sub-graph from the stack.
  asiAlgo_EXPORT void
    PushSubgraphX(const int face2Exclude);

  //! \brief Pops the top sub-graph from the internal stack.
  //!
  //! Goes back to the parent graph from sub-graph. Use this method to
  //! recover the previous state of AAG.
  asiAlgo_EXPORT void
    PopSubgraph();

  //@}

public:

  //! Returns face by its sequential ID using the internal naming service.
  //! \param[in] fid 1-based face ID.
  //! \return transient face.
  asiAlgo_EXPORT TopoDS_Face
    GetNamedFace(const int fid);

  //! Returns edge by its sequential ID using the internal naming service.
  //! \param[in] eid 1-based edge ID.
  //! \return transient edge.
  asiAlgo_EXPORT TopoDS_Edge
    GetNamedEdge(const int eid);

  //! Returns vertex by its sequential ID using the internal naming service.
  //! \param[in] vid 1-based vertex ID.
  //! \return transient vertex.
  asiAlgo_EXPORT TopoDS_Vertex
    GetNamedVertex(const int vid);

  //! Returns the naming index (the suffix of the name) for the passed
  //! shape which should be the alive shape of the model.
  //! \param[in] aliveShape shape of interest.
  //! \return 1-based persistent index of the shape or 0 if the shape cannot
  //!         be found.
  asiAlgo_EXPORT int
    GetNamingIndex(const TopoDS_Shape& aliveShape);

public:

  //! \return master CAD model.
  asiAlgo_EXPORT const TopoDS_Shape&
    GetMasterCAD() const;

  //! \return non-const reference to master CAD model.
  asiAlgo_EXPORT TopoDS_Shape&
    ChangeMasterCAD();

  //! Sets the collection of "selected" faces, i.e., the faces which are
  //! of particular interest to the client code. There is no logic behind
  //! this technique of marking some faces as selected.
  //! \param[in] selectedFaces faces to set as "selected".
  asiAlgo_EXPORT void
    SetSelectedFaces(const TopTools_IndexedMapOfShape& selectedFaces);

  //! Returns all selected faces.
  //! \return collection of 1-based indices of the selected faces.
  asiAlgo_EXPORT const TColStd_PackedMapOfInteger&
    GetSelectedFaces() const;

  //! Returns true if the index is in range.
  //! \param[in] face_idx face index.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasFace(const int face_idx) const;

  //! Returns true if the passed face is in graph.
  //! \param[in] face face to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasFace(const TopoDS_Shape& face) const;

  //! Returns topological face by its internal index (e.g. coming from iterator).
  //! \param[in] face_idx face index.
  //! \return topological face.
  asiAlgo_EXPORT const TopoDS_Face&
    GetFace(const int face_idx) const;

  //! Returns face ID.
  //! \param[in] face face of interest.
  //! \return face ID.
  asiAlgo_EXPORT int
    GetFaceId(const TopoDS_Shape& face) const;

  //! Checks whether the given face has any neighbors recorded in the AAG.
  //! Normally it has, but in some abnormal situations no neighbors could
  //! be there.
  //! \param[in] face_idx face index.
  //! \return true in case if at least one neighbor presents, false -- otherwise.
  asiAlgo_EXPORT bool
    HasNeighbors(const int face_idx) const;

  //! Returns neighbors for the face having the given internal index.
  //! \param[in] face_idx face index.
  //! \return indices of the neighbor faces.
  asiAlgo_EXPORT const TColStd_PackedMapOfInteger&
    GetNeighbors(const int face_idx) const;

  //! Returns only those neighbor faces which share the given edge with the
  //! passed face of interest.
  //! \param[in] face_idx ID of the face of interest.
  //! \param[in] edge     common edge.
  //! \return indices of the neighbor faces sharing the given edge.
  asiAlgo_EXPORT TColStd_PackedMapOfInteger
    GetNeighborsThru(const int face_idx, const TopoDS_Edge& edge);

  //! Returns only those neighbor faces which share the given edge with the
  //! passed face of interest.
  //! \param[in] face_idx ID of the face of interest.
  //! \param[in] edge_idx ID of the common edge.
  //! \return indices of the neighbor faces sharing the given edge.
  asiAlgo_EXPORT TColStd_PackedMapOfInteger
    GetNeighborsThru(const int face_idx, const int edge_idx);

  //! Returns neighbor faces for the given face of interest with additional
  //! filter on edges realizing the neighborhood.
  //! \param[in] face_idx index of the face of interest.
  //! \param[in] edge_ids indices of edges of interest.
  //! \return indices of the neighbor faces.
  asiAlgo_EXPORT TColStd_PackedMapOfInteger
    GetNeighbors(const int                         face_idx,
                 const TColStd_PackedMapOfInteger& edge_ids) const;

  //! Returns full collection of neighbor faces.
  //! \return neighborhood data.
  asiAlgo_EXPORT const t_adjacency&
    GetNeighborhood() const;

  //! Returns all faces of the master model.
  //! If the map is empty, it is constructed.
  //! \return all faces.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    GetMapOfFaces() const;

  //! Returns all faces of the master model having unique TShape pointers.
  //! If the map is empty, it is constructed.
  //! \return all faces.
  asiAlgo_EXPORT const asiAlgo_IndexedMapOfTShape&
    RequestTMapOfFaces();

  //! Returns all edges of the master model.
  //! If the map is empty, it is constructed.
  //! \return all edges.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    RequestMapOfEdges();

  //! Returns all edges of the master model having unique TShape pointers.
  //! If the map is empty, it is constructed.
  //! \return all edges.
  asiAlgo_EXPORT const asiAlgo_IndexedMapOfTShape&
    RequestTMapOfEdges();

  //! Returns all vertices of the master model.
  //! If the map is empty, it is constructed.
  //! \return map of all vertices.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    RequestMapOfVertices();

  //! Returns all vertices of the master model having unique TShape pointers.
  //! If the map is empty, it is constructed.
  //! \return all vertices.
  asiAlgo_EXPORT const asiAlgo_IndexedMapOfTShape&
    RequestTMapOfVertices();

  //! Returns all subshapes of the master model.
  //! If the map is empty, it is constructed.
  //! \return map of all sub-shapes.
  asiAlgo_EXPORT const TopTools_IndexedMapOfShape&
    RequestMapOfSubShapes();

  //! Returns all sub-shapes of the master model having unique TShape pointers.
  //! If the map is empty, it is constructed.
  //! \return all sub-shapes.
  asiAlgo_EXPORT const asiAlgo_IndexedMapOfTShape&
    RequestTMapOfSubShapes();

  //! \brief Returns map of indexed sub-shapes of the given type.
  //! If the map is empty, it is constructed.
  //!
  //! \param[in]  ssType sub-shape type (TopAbs_VERTEX, TopAbs_EDGE or TopAbs_FACE).
  //! \param[out] map    requested map of sub-shapes.
  asiAlgo_EXPORT void
    RequestMapOf(const TopAbs_ShapeEnum      ssType,
                 TopTools_IndexedMapOfShape& map);

  //! \brief Returns map of indexed sub-shapes with unique TShape pointers
  //!        of the given type.
  //!
  //! If the map is empty, it is constructed.
  //!
  //! \param[in]  ssType sub-shape type (TopAbs_VERTEX, TopAbs_EDGE or TopAbs_FACE).
  //! \param[out] map    requested map of sub-shapes.
  asiAlgo_EXPORT void
    RequestTMapOf(const TopAbs_ShapeEnum      ssType,
                  asiAlgo_IndexedMapOfTShape& map);

  //! Returns edges and their owner faces.
  //! If the map is empty, it is constructed.
  //! \return map of edges and their owner faces.
  asiAlgo_EXPORT const TopTools_IndexedDataMapOfShapeListOfShape&
    RequestMapOfEdgesFaces();

  //! Returns edges with unique TShape pointers and their owner faces.
  //! If the map is empty, it is constructed.
  //! \return map of edges and their owner faces.
  asiAlgo_EXPORT const asiAlgo_IndexedDataMapOfTShapeListOfShape&
    RequestTMapOfEdgesFaces();

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
    HasNodeAttributes(const int node) const;

  //! Accessor for the collection of nodal attributes.
  //! \param[in] node ID of the graph node of interest.
  //! \return attributes associated with the given graph node.
  asiAlgo_EXPORT const t_attr_set&
    GetNodeAttributes(const int node) const;

  //! Returns attribute associated with the given graph node.
  //! \param[in] node    ID of the graph node of interest.
  //! \param[in] attr_id ID of the attribute to access.
  //! \return attribute associated with the given node.
  asiAlgo_EXPORT Handle(asiAlgo_FeatureAttr)
    GetNodeAttribute(const int            node,
                     const Standard_GUID& attr_id) const;

  //! Removes attribute with the passed GUID from the given graph node.
  //! \param[in] node    ID of the graph node of interest.
  //! \param[in] attr_id ID of the attribute to remove.
  //! \return true if the attribute was removed, false -- otherwise (e.g., if
  //!         such attribute does not exist).
  asiAlgo_EXPORT bool
    RemoveNodeAttribute(const int            node,
                        const Standard_GUID& attr_id);

  //! Sets the given attribute for a node in AAG. If an attribute of this type
  //! is already there, this method does nothing and returns false.
  //! \param[in] node ID of the graph node of interest.
  //! \param[in] attr attribute to set.
  asiAlgo_EXPORT bool
    SetNodeAttribute(const int                          node,
                     const Handle(asiAlgo_FeatureAttr)& attr);

  //! Searches for those faces having ALL neighbors attributed with convex link.
  //! \param[out] resultFaces found faces (if any).
  //! \return true if anything has been found, false -- otherwise.
  asiAlgo_EXPORT bool
    FindConvexOnly(TopTools_IndexedMapOfShape& resultFaces) const;

  //! Searches for those faces having ALL neighbors attributed with concave link.
  //! \param[out] resultFaces found faces (if any).
  //! \return true if anything has been found, false -- otherwise.
  asiAlgo_EXPORT bool
    FindConcaveOnly(TopTools_IndexedMapOfShape& resultFaces) const;

  //! Removes the passed faces with all corresponding arcs from AAG.
  //! \param[in] faces faces to remove.
  asiAlgo_EXPORT void
    Remove(const TopTools_IndexedMapOfShape& faces);

  //! Removes the passed faces with all corresponding arcs from AAG.
  //! \param[in] faceIndices indices of faces to remove.
  asiAlgo_EXPORT void
    Remove(const TColStd_PackedMapOfInteger& faceIndices);

  //! Calculates number of the connected components.
  //! \return number of connected components in a graph.
  asiAlgo_EXPORT int
    GetConnectedComponentsNb();

  //! Calculates the number of the connected components in AAG without the
  //! given faces represented by their indexes.
  //! \return number of connected components in a graph.
  asiAlgo_EXPORT int
    GetConnectedComponentsNb(const TColStd_PackedMapOfInteger& excludedFaceIndices);

  //! Calculates connected components for the given set of indexes.
  asiAlgo_EXPORT void
    GetConnectedComponents(const TColStd_PackedMapOfInteger&               seeds,
                           NCollection_Vector<TColStd_PackedMapOfInteger>& res);

  //! Calculates connected components for the full graph.
  asiAlgo_EXPORT void
    GetConnectedComponents(NCollection_Vector<TColStd_PackedMapOfInteger>& res);

  //! Clears cached maps.
  asiAlgo_EXPORT void
    ClearCache();

public:

  //! Sets the naming service for the AAG. If the naming service is available,
  //! AAG will use indirect indexation of the topological elements. Rather
  //! than requesting the transient shape pointers directly from the cached
  //! maps, AAG will take the TShape pointers to further pass them through
  //! the naming service. At this stage, the persistent indices will be
  //! first used to make names (e.g., face number N will be named as 'face_N').
  //! In the assumption that the passed naming service uses the conventional
  //! names, AAG will pass the name to the naming service to access the
  //! currently alive entity in the CAD model. Thus the id-to-subshape
  //! mapping becomes indirect.
  //!
  //! In general, supplying AAG with the naming service allows to capture
  //! the indices once and then reuse them as long as the modeling operator is
  //! able to preserve the continuous history. Such an approach allows us
  //! to keep the persistent indices obtained at the AAG creation stage,
  //! so there is no need to reindex the master part or to adjust anyhow the
  //! AAG in order to actualize it with the present state of the model.
  //!
  //! \param[in] naming naming service to use for indirect indexation.
  void SetNaming(const Handle(asiAlgo_Naming)& naming)
  {
    m_naming = naming;
  }

  //! Resets naming service. This method simply nullifies the internal
  //! pointer to the naming service.
  void ResetNaming()
  {
    m_naming.Nullify();
  }

  //! \return true if the naming service is enabled.
  bool HasNaming() const
  {
    return !m_naming.IsNull();
  }

  //! Returns the currently used naming service (if any).
  //! \return const reference to the naming service smart pointer.
  const Handle(asiAlgo_Naming)& GetNaming() const
  {
    return m_naming;
  }

public:

  //! Dumps AAG structure to the passed output stream.
  //! \param[in, out] out target stream.
  asiAlgo_EXPORT void
    Dump(Standard_OStream& out) const;

  //! Dumps AAG structure as JSON.
  //! \param[in,out] out target stream.
  asiAlgo_EXPORT void
    DumpJSON(Standard_OStream& out) const;

protected:

  //! Initializes graph tool with master CAD and selected faces.
  //! \param[in] masterCAD        master model (full CAD).
  //! \param[in] selectedFaces    selected faces.
  //! \param[in] allowSmooth      indicates whether "smooth" attribution for
  //!                             arcs is allowed (true) or not (false).
  //! \param[in] smoothAngularTol angular tolerance used for recognition
  //!                             of smooth dihedral angles. A smooth angle
  //!                             may appear to be imperfect by construction,
  //!                             but still smooth by the design intent. With
  //!                             this parameter you're able to control it.
  //! \param[in] cachedMaps       flag indicating which maps of sub-shapes to
  //!                             cache. Since building topo maps is costly,
  //!                             it is generally a good idea to reuse them
  //!                             as much as possible. Using this flag you
  //!                             can control which maps will be built.
  asiAlgo_EXPORT void
    init(const TopoDS_Shape&               masterCAD,
         const TopTools_IndexedMapOfShape& selectedFaces,
         const bool                        allowSmooth,
         const double                      smoothAngularTol,
         const int                         cachedMaps);

  //! Fills graph with nodes for mate faces.
  //! \param[in] mateFaces faces to add (if not yet added).
  asiAlgo_EXPORT void
    addMates(const TopTools_ListOfShape& mateFaces);

  //! Dumps all graph nodes with their attributes to JSON.
  //! \param[in,out] out target output stream.
  asiAlgo_EXPORT void
    dumpNodesJSON(Standard_OStream& out) const;

  //! Dumps single graph node with its attributes to JSON.
  //! \param[in]     node    ID of the graph node to dump.
  //! \param[in]     isFirst indicates whether the currently dumped node is the
  //!                        first one to correctly put commas.
  //! \param[in,out] out     target output stream.
  asiAlgo_EXPORT void
    dumpNodeJSON(const int         node,
                 const bool        isFirst,
                 Standard_OStream& out) const;

  //! Dumps all graph arcs with their attributes to JSON.
  //! \param[in,out] out target output stream.
  asiAlgo_EXPORT void
    dumpArcsJSON(Standard_OStream& out) const;

  //! Dumps the given graph arc with its attributes to JSON.
  //! \param[in]     arc     graph arc in question.
  //! \param[in]     isFirst indicates whether the currently dumped arc is the
  //!                        first one to correctly put commas.
  //! \param[in,out] out     target output stream.
  asiAlgo_EXPORT void
    dumpArcJSON(const t_arc&      arc,
                const bool        isFirst,
                Standard_OStream& out) const;

protected:

  asiAlgo_AAG() : m_bAllowSmooth(false), m_fSmoothAngularTol(0.0) {} //!< Default ctor.

protected:

  //! Master CAD model.
  TopoDS_Shape m_master;

  //! Selected faces. Selection is performed externally using any criterion
  //! which we do not care about here. One typical scenario is to select
  //! those faces corresponding to some feature in the model.
  TColStd_PackedMapOfInteger m_selected;

  //! All sub-shapes.
  TopTools_IndexedMapOfShape m_subShapes;

  //! All sub-shapes with distinct TShape pointers.
  asiAlgo_IndexedMapOfTShape m_tSubShapes;

  //! All faces of the master model.
  TopTools_IndexedMapOfShape m_faces;

  //! All faces of the master model with distinct TShape pointers.
  asiAlgo_IndexedMapOfTShape m_tFaces;

  //! All edges of the master model.
  TopTools_IndexedMapOfShape m_edges;

  //! All edges of the master model with distinct TShape pointers.
  asiAlgo_IndexedMapOfTShape m_tEdges;

  //! All vertices of the master model.
  TopTools_IndexedMapOfShape m_vertices;

  //! All vertices of the master model with distinct TShape pointers.
  asiAlgo_IndexedMapOfTShape m_tVertices;

  //! Map of edges versus faces.
  TopTools_IndexedDataMapOfShapeListOfShape m_edgesFaces;

  //! Map of edges with distinct TShape pointers versus faces.
  asiAlgo_IndexedDataMapOfTShapeListOfShape m_tEdgesFaces;

  //! The data maps stored in this stack represent adjacency matrices. The
  //! stack is used to keep sub-graphs.
  std::stack<t_adjacency> m_neighborsStack;

  //! Stores attributes associated with each arc.
  t_arc_attributes m_arcAttributes;

  //! Stores attributes associated with nodes.
  t_node_attributes m_nodeAttributes;

  //! Indicates whether to allow smooth transitions or not.
  bool m_bAllowSmooth;

  //! Angular tolerance to use for attribution of "smooth" dihedral edges.
  double m_fSmoothAngularTol;

  //! Optional naming service.
  Handle(asiAlgo_Naming) m_naming;

  //! Progress entry.
  ActAPI_ProgressEntry m_progress;

  //! Plotter.
  ActAPI_PlotterEntry m_plotter;

};

#endif
