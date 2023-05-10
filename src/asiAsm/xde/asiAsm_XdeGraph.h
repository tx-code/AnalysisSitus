//-----------------------------------------------------------------------------
// Created on: 22 September 2020 (*)
// Created by: Sergey SLYADNEV
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

#ifndef asiAsm_XdeGraph_h
#define asiAsm_XdeGraph_h

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// OpenCascade includes
#include <TColStd_PackedMapOfInteger.hxx>

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! \ingroup ASIASM
//!
//! \brief Assembly graph.
//!
//! This tool gives a clear OCAF-agnostic interface to
//! the assembly structure of a product. A graph is essentially a set of
//! nodes {N} and a set of arcs {A} as defined formally.
//!
//! <pre>
//!   G = <N, A>
//! </pre>
//!
//! Using this tool, you can map XDE assembly items to a formal graph
//! structure. Each node in the graph preserves a link to the data storage
//! (OCAF document) by means of \ref core_ObjectId.
//!
//! You can find more on Hierarchical Assembly Graphs in the paper
//! "A Scheme for Single Instance Representation In Hierarchical Assembly Graphs"
//! by Ari Rappoport in "Modeling in Computer Graphics", 1993.
//!
//! \sa core_ObjectId
class Graph : public Standard_Transient
{
public:

  //! \brief Type of the graph node.
  enum NodeType
  {
    NodeType_UNDEFINED = 0,  //!< Undefined node type.
    //
    NodeType_SubassemblyOccurrence, //!< Subassembly occurrence.
    NodeType_Subassembly,           //!< Subassembly prototype.
    NodeType_PartOccurrence,        //!< Part usage occurrence.
    NodeType_Part                   //!< Optional leaf node to represent parts. Note that
                                    //!< this node type is activated by a dedicated flag in
                                    //!< the constructor. If activated, the part occurrence nodes
                                    //!< are not leafs anymore.
  };

public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(Graph, Standard_Transient)

public:

  //! \brief Graph iterator.
  class Iterator
  {
  public:

    //! Default ctor.
    Iterator() : m_iCurrentIndex(0) {}

    //! ctor accepting the assembly graph to iterate.
    //! \param[in] asmGraph assembly graph to iterate.
    Iterator(const Handle(Graph)& asmGraph)
    {
      this->Init(asmGraph);
    }

  public:

    //! Initializes iterator with assembly graph.
    //! \param[in] asmGraph assembly graph to iterate.
    void Init(const Handle(Graph)& asmGraph)
    {
      m_graph         = asmGraph;
      m_iCurrentIndex = 1;
    }

    //! Checks if there are more graph nodes to iterate.
    //! \return true/false.
    bool More() const
    {
      return m_iCurrentIndex <= m_graph->GetNodes().Extent();
    }

    //! \return 1-based ID of the current node.
    int GetCurrentNode() const
    {
      return m_iCurrentIndex;
    }

    //! Moves iterator to the next position.
    void Next()
    {
      ++m_iCurrentIndex;
    }

  protected:

    Handle(Graph) m_graph;         //!< Assembly graph to iterate.
    int           m_iCurrentIndex; //!< Current 1-based node ID.

  };

public:

  //! Type definition for graph adjacency matrix. This is how parent-component
  //! links are realized in the assembly graph.
  typedef NCollection_DataMap<int, TColStd_PackedMapOfInteger> t_adjacency;

public:

  //! \brief Initializes graph from Data Model.
  //!
  //! Construction of a formal graph will be done immediately at ctor.
  //!
  //! \param[in] M Data Model to iterate.
  asiAsm_EXPORT
    Graph(const Handle(Doc)& M);

public:

  //! \brief Dumps graph structure to output stream.
  //!
  //! The output format is DOT. You may use graph rendering tools like
  //! Graphviz to parse the output.
  //!
  //! \param[out] out output stream.
  asiAsm_EXPORT void
    Dump(Standard_OStream& out) const;

  //! Dumps HAG structure as JSON.
  //! \param[in,out] out target stream.
  asiAsm_EXPORT void
    DumpJSON(Standard_OStream& out) const;

  //! \brief Calculates short summary for the assembly.
  //!
  //! Short summary gives you the total number of nodes of a particular
  //! type.
  //!
  //! \param[out] numRoots           number of root nodes.
  //! \param[out] numSubassemblies   number of subassembly nodes.
  //! \param[out] numPartOccurrences number of part usage occurrence nodes.
  //! \param[out] numParts           number of parts (if available).
  asiAsm_EXPORT void
    CalculateSummary(int& numRoots,
                     int& numSubassemblyOccurrences,
                     int& numSubassemblies,
                     int& numPartOccurrences,
                     int& numParts) const;

public:

  //! \brief Returns IDs of the root nodes.
  //! \return IDs of the root nodes.
  const TColStd_PackedMapOfInteger& GetRoots() const
  {
    return m_roots;
  }

  //! \brief Checks whether the assembly graph contains (n1, n2) directed arc.
  //! \param[in] n1 one-based ID of the first node.
  //! \param[in] n2 one-based ID of the second node.
  //! \return true/false.
  bool HasArc(const int n1, const int n2) const
  {
    if ( !this->HasChildren(n1) )
      return false;

    return this->GetChildren(n1).Contains(n2);
  }

  //! \brief Checks whether children exist for the given node.
  //! \param[in] oneBasedNodeId one-based node ID.
  //! \return true/false.
  bool HasChildren(const int oneBasedNodeId) const
  {
    return m_arcs.IsBound(oneBasedNodeId);
  }

  //! \brief Returns IDs of child nodes for the given node.
  //! \param[in] oneBasedNodeId one-based node ID.
  //! \return set of child IDs.
  const TColStd_PackedMapOfInteger& GetChildren(const int oneBasedNodeId) const
  {
    return m_arcs(oneBasedNodeId);
  }

  //! \brief Returns the node type from \ref NodeType enum.
  //! \param[in] oneBasedNodeId one-based node ID.
  //! \return node type.
  //! \sa NodeType
  NodeType GetNodeType(const int oneBasedNodeId) const
  {
    const NodeType* typePtr = m_nodeTypes.Seek(oneBasedNodeId);
    if ( typePtr == NULL )
      return NodeType_UNDEFINED;

    return (*typePtr);
  }

  //! \brief Returns object ID by node ID.
  //! \param[in] oneBasedNodeId one-based node ID.
  //! \return persistent ID.
  const PersistentId& GetPersistentId(const int oneBasedNodeId) const
  {
    return m_nodes(oneBasedNodeId);
  }

  //! \brief Returns the unordered set of graph nodes.
  //! \return graph nodes.
  const NCollection_IndexedMap<PersistentId>& GetNodes() const
  {
    return m_nodes;
  }

  //! \brief Returns the number of graph nodes.
  //! \return number of graph nodes.
  int GetNumberOfNodes() const
  {
    return m_nodes.Extent();
  }

  //! \brief Returns the collection of graph arcs in form of adjacency matrix.
  //! \return graph arcs.
  const t_adjacency& GetArcs() const
  {
    return m_arcs;
  }

  //! \brief Returns the number of graph arcs.
  //! \return number of graph arcs.
  int GetNumberOfArcs() const
  {
    int numArcs = 0;
    //
    for ( t_adjacency::Iterator it(m_arcs); it.More(); it.Next() )
      numArcs += it.Value().Extent();

    return numArcs;
  }

  //! Returns quantity of part usage occurrences.
  //! \param[in] oneBasedPartId 1-based part ID.
  //! \return usage occurrence quantity.
  int GetUsageOccurrenceQuantity(const int oneBasedPartId) const
  {
    const int* usageOQPtr = m_usages.Seek(oneBasedPartId);
    if ( usageOQPtr == NULL )
      return 0;

    return (*usageOQPtr);
  }

protected:

  //! Builds graph out of OCAF XDE structure.
  asiAsm_EXPORT void
    buildGraph();

  //! Adds components for the given root to the graph structure.
  //! \param[in] parent    OCAF label of the parent object.
  //! \param[in] iParentId ID of the already registered node
  //!                      representing the parent object in the assembly
  //!                      graph being populated.
  asiAsm_EXPORT void
    addComponents(const TDF_Label& parent,
                  const int        iParentId);

  //! Adds node into the graph. To add node properly, it is necessary to
  //! know whether in XDE it represents an original object or its occurence
  //! This information in encoded by two labels following XDE structure.
  //! \param[in]  insertionLevelLabel   label at insertion level.
  //! \param[in]  declarationLevelLabel label at declaration level.
  //! \param[out] nextLeaf              ID of the last added node. Since we
  //!                                   add not only instances but also the
  //!                                   prototypes, the returned ID may
  //!                                   differ from this one.
  //! \return 1-based internal ID of the added node.
  asiAsm_EXPORT int
    addNode(const TDF_Label& insertionLevelLabel,
            const TDF_Label& declarationLevelLabel,
            int&             nextLeaf);

protected:

  // INPUTS
  Handle(Doc) m_model; //!< Data Model instance.

  // OUTPUTS
  TColStd_PackedMapOfInteger           m_roots;     //!< IDs of the root nodes.
  NCollection_IndexedMap<PersistentId> m_nodes;     //!< Graph nodes.
  t_adjacency                          m_arcs;      //!< "Part-of" relations.
  NCollection_DataMap<int, NodeType>   m_nodeTypes; //!< Node types (cached for efficiency).
  NCollection_DataMap<int, int>        m_usages;    //!< Prototype usage occurrences.

};

} // xde
} // asiAsm

#endif
