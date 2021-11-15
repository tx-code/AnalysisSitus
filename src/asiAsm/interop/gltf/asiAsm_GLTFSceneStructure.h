//-----------------------------------------------------------------------------
// Created on: 12 July 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Julia Slyadneva
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
#pragma once

// asiAsm includes
#include <asiAsm.h>

// OpenCascade includes
#include <TCollection_AsciiString.hxx>
#include <TopLoc_Location.hxx>

//C++ includes
#include <vector>

namespace asiAsm {
  namespace xde {

//! This structure represents a node of glTF scene which contains of array of such nodes placed under "nodes" element.
//! The node may have a name and own transformation. If the node is a container of other nodes, keep them as children array.
//! If the node is a mesh leaf of the scene, it must point to the index of the corresponding mesh item under "meshes" element.
//! For getting a better idea follow this : https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_004_ScenesNodes.md
struct glTFNode
{
  friend class glTFSceneStructure;

  static const int INVALID_ID = -1;

  TCollection_AsciiString  Name;
  TopLoc_Location          Trsf;
  std::vector<glTFNode*>  Children;
  int                      MeshIndex;

  private:

    glTFNode()
      : MeshIndex(INVALID_ID) {}
};

//-----------------------------------------------------------------------------

//! This class is used to describe a scene structure for glTF writer.
//! The scene structure is a series of nodes referring to each other through the index.
//! The index is a position of node in array of all possible nodes including roots, containers, meshes, cameras, etc.
//!   * The root(s) is the top-most node(s) of the scene structure. The scene must have at least one root node. The root node is always a container.
//!     The glTF structure refers to the indices of these nodes in the "scene" section.
//!   * The container(s) is a node simply referring to other nodes through "children" attribute.
//!   * The mesh(es) is a node having a 3D representation. The 3D representations are stored under "meshes" element as an array. 
//!     So that each 3D representation also has its own unique index to which the mesh node reffers through "mesh" attribute.
//! All nodes are placed under "nodes" element of json-alike glTF file. The index numbering starts with '0'.
//! For example,
//!     "scenes" :
//!      [
//!         nodes: [0]-----------|
//!      ]                       |
//!     "nodes":                 |
//!      [                       |
//!         {                    |
//!           "name": "Car",  <--|
//!           "children" : [1, 2, 3, 4]  <-- indices of following nodes
//!         }, <-- node of index 0
//!         {
//!             "name": "wheel_1"
//!             "mesh": 0  -------------------|
//!         }, <-- node of index 1            |
//!         {                                 |
//!             "name": "wheel_2"             |
//!             "mesh": 1  -----------------------|
//!         }, <-- node of index 2            |   |
//!         {                                 |   |
//!             "name": "wheel_3"             |   |
//!             "mesh": 2  -----------------------|---|
//!         }, <-- node of index 3            |   |   |
//!         {                                 |   |   |
//!             "name": "wheel_4"             |   |   |
//!             "mesh": 3  -------------------|---|---|---|
//!         } <-- node of index 4             |   |   |   |
//!     ],                                    |   |   |   |
//!                                           |   |   |   |
//!     "meshes" :                            |   |   |   |
//!     [                                     |   |   |   |
//!       { ... } <-- mesh node of index 0 <--|   |   |   |
//!       { ... } <-- mesh node of index 1 <------|   |   |
//!       { ... } <-- mesh node of index 2 <----------|   |
//!       { ... } <-- mesh node of index 3 <--------------|
//!       ...
//!     ]
//! 
//! To know more follow this :
//! https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_002_BasicGltfStructure.md#the-basic-structure-of-gltf
//! https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_004_ScenesNodes.md#scenes-and-nodes

class glTFSceneStructure
{
public:

  static const int INVALID_ID = -1;

public:

  //! Destructor.
  asiAsm_EXPORT 
    ~glTFSceneStructure();

  //! Adds a new node to the end of array.
  asiAsm_EXPORT 
    glTFNode* PrependNode();

  //! Sets the node as root.
  //! \param[in] n the node to set as root.
  asiAsm_EXPORT 
    void MarkNodeAsRoot(glTFNode* N);

  //! Removes all nodes.
  asiAsm_EXPORT 
    void Clear();

public:

  //! Gets the top-level nodes.
  asiAsm_EXPORT 
    const std::vector<glTFNode*>& GetRoots() const;

  //! Gets all nodes of scene structure.
  asiAsm_EXPORT 
    const std::vector<glTFNode*>& GetNodes() const;

  //! Gets the index of a certain node.
  //! \param[in] n the node which index is in question.
  asiAsm_EXPORT 
    int GetIndex(glTFNode* N) const;

private:

  std::vector<glTFNode*> m_nodes;
  std::vector<glTFNode*> m_roots;
};
} // xde
} // asiAsm

