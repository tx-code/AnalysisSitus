//-----------------------------------------------------------------------------
// Created on: 15 April 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Quaoar Studio LLC
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

#ifndef asiAlgo_DiscrBgrTriangle_HeaderFile
#define asiAlgo_DiscrBgrTriangle_HeaderFile

// OpenCascade includes
#include <Standard_TypeDef.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

//! This class represents a single triangle in the background mesh
//! of a discrete faces.

// Nodes/neighbors order:
//                               [0]
//                        ---(1)------(0)
//                            |       / \
//                            |      /   \
//                            |     /     \
//                       \ [1]|    /[2]
//                        \   |   /
//                         \  |  /
//                          \ | /
//                           (2)-------

class BgrTriangle
{
public:

  //! Default ctor.
  BgrTriangle()
  {
    m_nodes[2]     = m_nodes[1]     = m_nodes[0] = 0;
    m_neighbors[2] = m_neighbors[1] = m_neighbors[0] = 0;
    m_bIsInternal  = 0;
  }

  //! Ctor with initialization.
  BgrTriangle(const int nodes[3],
              const int isInternal = 0)
  //
  : m_bIsInternal(isInternal)
  {
    m_nodes[0]     = nodes[0];
    m_nodes[1]     = nodes[1];
    m_nodes[2]     = nodes[2];
    m_neighbors[2] = m_neighbors[1] = m_neighbors[0] = 0;
  }

public:

  void SetNodes(const int nodes[3])
  {
    m_nodes[0] = nodes[0];
    m_nodes[1] = nodes[1];
    m_nodes[2] = nodes[2];
  }

  void SetNodes(const int n1,
                const int n2,
                const int n3)
  {
    m_nodes[0] = n1;
    m_nodes[1] = n2;
    m_nodes[2] = n3;
  }

  int Node(const int i) const
  {
    return m_nodes[i];
  }

public:

  void SetNeighbours(const int neigh[3])
  {
    m_neighbors[0] = neigh[0];
    m_neighbors[1] = neigh[1];
    m_neighbors[2] = neigh[2];
  }

  void SetNeighbours(const int n1,
                     const int n2,
                     const int n3)
  {
    m_neighbors[0] = n1;
    m_neighbors[1] = n2;
    m_neighbors[2] = n3;
  }

  int Neighbour(const int i) const
  {
    return m_neighbors[i];
  }

public:

  bool IsInternal() const
  {
    return bool(m_bIsInternal);
  }

 private:

  int      m_nodes     [3];   // counter-clockwise order
  int      m_neighbors [3];   // neigbor[i] on link {i,(i+1)%3}
  unsigned m_bIsInternal : 1; // if it is inside the bounds

};

}
}

#endif
