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

#ifndef asiAlgo_DiscrBgrMesh_HeaderFile
#define asiAlgo_DiscrBgrMesh_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrBgrNode.h>
#include <asiAlgo_DiscrBgrTriangle.h>

// OpenCascade includes
#include <NCollection_HArray1.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

NCOLLECTION_HARRAY1(HArray1OfBgrNode, BgrNode)
NCOLLECTION_HARRAY1(HArray1OfBgrTriangle, BgrTriangle)

//! Background mesh associated with a discrete face. This data structure is
//! essentially a couple of arrays: one for mesh nodes and another
//! for triangles.
class BgrMesh
{
public:

  //! Default ctor.
  BgrMesh() = default;

public:

  void Reset()
  {
    m_nodes.Nullify();
    m_triangles.Nullify();
  }

  void Init(const int nbNodes,
            const int nbTriangles)
  {
    m_nodes     = new HArray1OfBgrNode(1, nbNodes);
    m_triangles = new HArray1OfBgrTriangle(1, nbTriangles);
  }

  const BgrNode& Node(const int i) const
  {
    return m_nodes->Value(i);
  }

  const BgrTriangle& Triangle(const int i) const
  {
    return m_triangles->Value(i);
  }

  BgrNode& ChangeNode(const int i)
  {
    return m_nodes->ChangeValue(i);
  }

  BgrTriangle& ChangeTriangle(const int i)
  {
    return m_triangles->ChangeValue(i);
  }

  int NbNodes(void) const
  {
    return m_nodes.IsNull() ? 0 : m_nodes->Length();
  }

  int NbTriangles(void) const
  {
    return m_triangles.IsNull() ? 0 : m_triangles->Length();
  }

 private:

  BgrMesh(const BgrMesh& other) = delete;
  BgrMesh& operator=(const BgrMesh& other) = delete;

private:

  Handle(HArray1OfBgrNode)     m_nodes;
  Handle(HArray1OfBgrTriangle) m_triangles;

};

}
}

#endif
