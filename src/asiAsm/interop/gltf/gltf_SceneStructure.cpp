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

#include <gltf_SceneStructure.h>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------
gltf_SceneStructure::~gltf_SceneStructure()
{
  for (auto n : m_nodes)
  {
    delete n;
    n = nullptr;
  }
  m_nodes.clear();
}

//-----------------------------------------------------------------------------
void gltf_SceneStructure::MarkNodeAsRoot(gltf_Node* N)
{
  m_roots.push_back(N);
}

//-----------------------------------------------------------------------------
gltf_Node* gltf_SceneStructure::PrependNode()
{
  gltf_Node* n = new gltf_Node();
  m_nodes.push_back(n);

  return n;
}

//-----------------------------------------------------------------------------
void gltf_SceneStructure::Clear()
{
  m_nodes.clear();
  m_nodes.shrink_to_fit();

  m_roots.clear();
  m_roots.shrink_to_fit();
}

//-----------------------------------------------------------------------------
const std::vector<gltf_Node*>& gltf_SceneStructure::GetRoots() const
{
  return m_roots;
}

//-----------------------------------------------------------------------------
const std::vector<gltf_Node*>& gltf_SceneStructure::GetNodes() const
{
  return m_nodes;
}

//-----------------------------------------------------------------------------
int gltf_SceneStructure::GetIndex(gltf_Node* N) const
{
  int _index = 0;
  for (const gltf_Node* _n : m_nodes)
  {
    if (_n == N)
      return _index;
    _index++;
  }

  return INVALID_ID;
}
