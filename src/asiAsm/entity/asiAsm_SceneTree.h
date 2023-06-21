//-----------------------------------------------------------------------------
// Created on: 19 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

#ifndef asiAsm_SceneTree_h
#define asiAsm_SceneTree_h

// asiAsm includes
#include <asiAsm_XdeDoc.h>
#include <asiAsm_XdeGraph.h>

// For shared pointer
#include <memory>

// Forward declarations.
class asiAsm_SceneTree_Child;
class asiAsm_SceneTree_Part;
class asiAsm_SceneTree_Assembly;
class asiAsm_SceneTree_Instance;

//-----------------------------------------------------------------------------

//! Scene tree as a result of HAG (hierarchical assembly graph) traversal.
class asiAsm_SceneTree
{
public:

  //! Matches two scene hierarchy results.
  asiAsm_EXPORT static bool
    Match(const asiAsm_SceneTree& R1,
          const asiAsm_SceneTree& R2,
          ActAPI_ProgressEntry    progress);

  //! Constructs a scene tree structure from a stream keeping description of the structure.
  //! \param[in]  in   the input stream containing description of the structure.
  //! \param[out] info the outcome data structure.
  asiAsm_EXPORT static void
    FromJSON(std::ifstream&     in,
             asiAsm_SceneTree& info);

  //! Constructs the scene tree structure from a JSON object.
  //! \param[in]  pJsonGenericObj the JSON object to construct the data structure from.
  //! \param[out] info            the outcome data structure.
  asiAsm_EXPORT static void
    FromJSON(void*             pJsonGenericObj,
             asiAsm_SceneTree& info);

  //! Converts the passed data structure to JSON (the passed `out` stream).
  //! \param[in]     info   the data structure to serialize.
  //! \param[in]     indent the pretty indentation shift.
  //! \param[in]     self   the Boolean flag indicating whether this document is self-sufficient.
  //! \param[in,out] out    the output JSON string stream.
  asiAsm_EXPORT static void
    ToJSON(const asiAsm_SceneTree& info,
           const int               indent,
           const bool              self,
           std::ostream&           out);

public:

  //! Constructor.
  asiAsm_EXPORT
    asiAsm_SceneTree();

public:

  //! Computes scene hierarchy of a model.
  //! \param[in] doc          XDE document with the CAD assembly.
  //! \param[in] doDumpShapes flag indicating whether shapes representations should be dumped.
  asiAsm_EXPORT void
    Build(const Handle(asiAsm::xde::Doc)& doc,
          const bool                      doDumpShapes = false);

  //! Displays the geometric entities of the structure in the given plotter. 
  asiAsm_EXPORT void
    Dislay(ActAPI_PlotterEntry plotter);

public:

  //! Get assemblies.
  asiAsm_EXPORT const std::vector<Handle(asiAsm_SceneTree_Assembly)>&
    GetAssemblies() const;

  //! Get instances.
  asiAsm_EXPORT const std::vector<Handle(asiAsm_SceneTree_Instance)>&
    GetInstances() const;

  //! Get parts.
  asiAsm_EXPORT const std::vector<Handle(asiAsm_SceneTree_Part)>&
    GetParts() const;

  //! Get roots.
  asiAsm_EXPORT const std::vector<int>&
    GetRoots() const;

private:

  //! Collects information about children.
  //! \param[in] doc          XDE document with the CAD assembly.
  //! \param[in] graph        assembly graph.
  //! \param[in] parentId     parent Id which children needs to be found.
  //! \param[in] parent       pointer to parent.
  //! \param[in] path         assembly item id of the parent.
  //! \param[in] doDumpShapes flag indicating whether shapes representations should be dumped.
  void populate(const Handle(asiAsm::xde::Doc)&   doc,
                const Handle(asiAsm::xde::Graph)& graph,
                const int                         parentId,
                Handle(asiAsm_SceneTree_Child)&   parent,
                const std::string&                path,
                const bool                        doDumpShapes);

  //! Collects information about child.
  //! \param[in] doc      XDE document with the CAD assembly.
  //! \param[in] graph    assembly graph.
  //! \param[in] child    pointer to child.
  //! \param[in] childId  child id.
  //! \param[in] path     assembly item id of the child.
  void getChildInfo(const Handle(asiAsm::xde::Doc)&       doc,
                    const Handle(asiAsm::xde::Graph)&     graph,
                    const Handle(asiAsm_SceneTree_Child)& child,
                    const int                             childId,
                    const std::string&                    path);

  //! Cleans previously saved data.
  void cleanUpData();

private:

  std::vector<Handle(asiAsm_SceneTree_Assembly)> m_assemblies; // assemblies informatiom.
  std::vector<Handle(asiAsm_SceneTree_Instance)> m_instances;  // instances information.
  std::vector<Handle(asiAsm_SceneTree_Part)>     m_parts;      // parts information.
  std::vector<int>                               m_roots;      // root ids.

};

#endif
