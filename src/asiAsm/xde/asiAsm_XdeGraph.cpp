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

// Own include
#include <asiAsm_XdeGraph.h>

// OCCT includes
#include <TColStd_MapIteratorOfPackedMapOfInteger.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

#define NodeLetter "N"
#define Whitespace "    "

//-----------------------------------------------------------------------------

Graph::Graph(const Handle(Doc)& M)
: Standard_Transient (),
  m_model            (M)
{
  try
  {
    this->buildGraph();
  }
  catch (...)
  {
    std::cout << "Exception during building assembly graph.\n";
  }
}

//-----------------------------------------------------------------------------

void Graph::Dump(Standard_OStream& out) const
{
  // Directed graph header.
  out << "digraph asiAsm_XdeGraph {\n";
  out << "\n";

  // Dump nodes with attributes.
  const NCollection_IndexedMap<PersistentId>& nodes = this->GetNodes();
  //
  for ( int n = 1; n <= nodes.Extent(); ++n )
  {
    // Get name of persistent object.
    TCollection_ExtendedString name;
    m_model->GetObjectName(nodes(n), name);

    // Generate label
    TCollection_AsciiString label(name);
    label += "\\n"; label += nodes(n);

    // Dump node with label.
    out << Whitespace << NodeLetter << n << " [label=\"" << label.ToCString() << "\"];\n";
  }
  out << "\n";

  // Dump arcs.
  for ( t_adjacency::Iterator it(m_arcs); it.More(); it.Next() )
  {
    const int                         parentId = it.Key();
    const TColStd_PackedMapOfInteger& children = it.Value();

    // Loop over the children.
    for ( TColStd_MapIteratorOfPackedMapOfInteger cit(children); cit.More(); cit.Next() )
    {
      const int childId = cit.Key();

      out << Whitespace
          << NodeLetter << parentId
          << " -> "
          << NodeLetter << childId
          << ";\n";
    }
  }

  out << "\n";
  out << "}\n";
}

//-----------------------------------------------------------------------------

void Graph::DumpJSON(Standard_OStream&) const
{
  // TODO: NYI
}

//-----------------------------------------------------------------------------

void Graph::CalculateSummary(int& numRoots,
                             int& numSubassemblyOccurrences,
                             int& numSubassemblies,
                             int& numPartOccurrences,
                             int& numParts) const
{
  numRoots                  = m_roots.Extent();
  numSubassemblyOccurrences = 0;
  numSubassemblies          = 0;
  numPartOccurrences        = 0;
  numParts                  = 0;

  // Loop over the nodes.
  for ( int n = 1; n <= this->GetNodes().Extent(); ++n )
  {
    const NodeType* nodeTypePtr = m_nodeTypes.Seek(n);
    if ( nodeTypePtr == NULL ) continue; // This should never happen.

    switch ( (*nodeTypePtr) )
    {
      case NodeType_SubassemblyOccurrence: ++numSubassemblyOccurrences; break;
      case NodeType_Subassembly:           ++numSubassemblies;          break;
      case NodeType_PartOccurrence:        ++numPartOccurrences;        break;
      case NodeType_Part:                  ++numParts;                  break;
      default: break;
    }
  }
}

//-----------------------------------------------------------------------------

void Graph::buildGraph()
{
  // We start from those shapes which are "free" in terms of XDE.
  TDF_LabelSequence roots;
  //
  m_model->GetShapeTool()->GetFreeShapes(roots);
  //
  for ( TDF_LabelSequence::Iterator it(roots); it.More(); it.Next() )
  {
    TDF_Label label = it.Value();

    // Add root node.
    int iNextChildId;
    const int iRootId = this->addNode( label, m_model->GetOriginal(label), iNextChildId );
    //
    m_roots.Add(iRootId);

    // The following processing is necessary for top-level assembly instances.
    if ( m_model->GetShapeTool()->IsReference(label) )
    {
      Handle(TDataStd_TreeNode) jumpTreeNode;
      if ( !label.FindAttribute(XCAFDoc::ShapeRefGUID(), jumpTreeNode) )
        continue;

      label = jumpTreeNode->Father()->Label(); // Declaration-level origin.
    }

    // Add components (the objects nested into the current one).
    if ( m_model->GetShapeTool()->IsAssembly(label) )
      this->addComponents(label, iNextChildId);
  }
}

//-----------------------------------------------------------------------------

void Graph::addComponents(const TDF_Label& parent,
                          const int        iParentId)
{
  const bool isSubassembly = m_model->GetShapeTool()->IsAssembly(parent);
  //
  if ( !isSubassembly )
    return; // We have to return here in order to prevent iterating by
            // sub-labels. For parts, sub-labels are used to encode
            // metadata which is out of interest in conceptual design
            // intent represented by assembly graph.

  // Loop over the children (persistent representation of "part-of" relation).
  for ( TDF_ChildIterator cit(parent); cit.More(); cit.Next() )
  {
    TDF_Label child = cit.Value();

    // Protection against deleted empty labels (after expand compounds,
    // for example).
    Handle(TDataStd_TreeNode) jumpTreeNode;
    if ( !child.FindAttribute(XCAFDoc::ShapeRefGUID(), jumpTreeNode) )
      continue;

    // Jump to the referred object (the original).
    TDF_Label childOriginal;
    if ( !jumpTreeNode.IsNull() && jumpTreeNode->HasFather() )
      childOriginal = jumpTreeNode->Father()->Label(); // Declaration-level origin.
    //
    if ( childOriginal.IsNull() )
    {
#if defined COUT_DEBUG
      std::cout << "XDE format problem: insertion-level label has no original." << std::endl;
#endif
      continue;
    }

    // Add child.
    int iNextChildId = -1;
    const int iChildId = this->addNode(child, childOriginal, iNextChildId);

    // Add arc.
    TColStd_PackedMapOfInteger* mapPtr = m_arcs.ChangeSeek(iParentId);
    if ( mapPtr == NULL )
      mapPtr = m_arcs.Bound( iParentId, TColStd_PackedMapOfInteger() );
    //
    (*mapPtr).Add(iChildId);

    // Process children: add components recursively.
    this->addComponents(childOriginal, iNextChildId);
  }
}

//-----------------------------------------------------------------------------

int Graph::addNode(const TDF_Label& insertionLevelLabel,
                   const TDF_Label& declarationLevelLabel,
                   int&             nextLeaf)
{
  // Check if the current object is (sub)assembly. Root assembly will give 'true'.
  const bool isSubassembly = m_model->GetShapeTool()->IsAssembly(declarationLevelLabel);

  // Check if the current object is instance of a part or a (sub)assembly.
  const bool isInstance = (insertionLevelLabel != declarationLevelLabel);

  // Get entry of the insertion-level label (sub-label in assembly hierarchy).
  PersistentId insertionLevelId;
  TDF_Tool::Entry(insertionLevelLabel, insertionLevelId);

  // Get ID of the insertion-level node in the abstract assembly graph.
  const int iInsertionLevelId = m_nodes.Add(insertionLevelId);

  // The following processing is done for all occurrences of prototypes to
  // calculate their quantities. This way we support the quantified
  // usage occurrence for the prototypes.
  int iPrototypeId = 0;
  //
  if ( isInstance )
  {
    // Get entry of the original label.
    PersistentId partId;
    TDF_Tool::Entry(declarationLevelLabel, partId);

    // Add part node. If such part has been added already, old ID is returned.
    iPrototypeId = m_nodes.Add(partId);
  }
  else
    iPrototypeId = iInsertionLevelId; // In case if part is a root.

  // Bind usage occurrences.
  int* usageOQPtr = m_usages.ChangeSeek(iPrototypeId);
  if ( usageOQPtr == NULL )
    usageOQPtr = m_usages.Bound(iPrototypeId, 1);
  else
    (*usageOQPtr)++;

  // Bind type of the assembly element. In the abstract assembly graph, we
  // distinguish between four types of elements: (sub)assemblies, (sub)assembly
  // instances, parts, and part instances.
  if ( isInstance )
  {
    m_nodeTypes.Bind(iInsertionLevelId, isSubassembly ? NodeType_SubassemblyOccurrence
                                                      : NodeType_PartOccurrence);

    // Add arc.
    TColStd_PackedMapOfInteger* mapPtr = m_arcs.ChangeSeek(iInsertionLevelId);
    if ( mapPtr == NULL )
      mapPtr = m_arcs.Bound( iInsertionLevelId, TColStd_PackedMapOfInteger() );
    //
    (*mapPtr).Add(iPrototypeId);

    // Bind type.
    if ( !m_nodeTypes.IsBound(iPrototypeId) )
      m_nodeTypes.Bind(iPrototypeId, isSubassembly ? NodeType_Subassembly
                                                    : NodeType_Part);
  }
  else // Not instance -> free (top-level) prototype.
  {
    if ( !m_nodeTypes.IsBound(iInsertionLevelId) )
      m_nodeTypes.Bind(iInsertionLevelId, isSubassembly ? NodeType_Subassembly
                                                        : NodeType_Part);
  }

  nextLeaf = iPrototypeId;
  return iInsertionLevelId;
}
