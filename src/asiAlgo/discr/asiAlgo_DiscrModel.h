//-----------------------------------------------------------------------------
// Created on: 17 April 2022
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

#ifndef asiAlgo_DiscrModel_HeaderFile
#define asiAlgo_DiscrModel_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrEdge.h>
#include <asiAlgo_DiscrFace.h>
#include <asiAlgo_DiscrParams.h>
#include <asiAlgo_DiscrVertex.h>
#include <asiAlgo_DiscrWire.h>
#include <asiAlgo_DiscrSequenceOfPointer.h>

// OpenCascade includes
#include <Standard_Transient.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

//! Discrete representation for precise B-rep. This data structure is supposed
//! to be populated by `Builder`.
class Model : public Standard_Transient
{
  // Declaration of CASCADE RTTI
  DEFINE_STANDARD_RTTI_INLINE(Model,Standard_Transient)

public:
  // ---------- CONSTRUCTORS ----------

  Model () : myNbFaces(0), myNbEdges(0) {}
  // Empty constructor

  asiAlgo_EXPORT virtual ~Model ();

  // ---------- INITIALISATION ----------

  void LoadParameters (const Params& aMeshParams)
  { myMeshParams = aMeshParams; }
  // Loads meshing parameters

  asiAlgo_EXPORT void InitFaces (const int aNbFaces);
  // Initializes internal array of faces

  asiAlgo_EXPORT void InitEdges (const int aNbEdges);
  // Initializes internal array of edges

  // ---------- Meshing Parameters ----------

  const Params& GetParameters () const { return myMeshParams; }

  // ---------- FACES ----------

  int GetNbFaces () const { return myNbFaces;}

  const asiAlgo::discr::Face& GetFace (const int aIndex) const
  {
    return myFaces[aIndex-1];
  }

  asiAlgo::discr::Face& ChangeFace (const int aIndex)
  {
    return myFaces[aIndex-1];
  }

  // ---------- EDGES ----------

  int GetNbEdges () const { return myNbEdges;}

  const asiAlgo::discr::Edge& GetEdge (const int aIndex) const
  {
    return myEdges[aIndex-1];
  }

  asiAlgo::discr::Edge& ChangeEdge (const int aIndex)
  {
    return myEdges[aIndex-1];
  }

  // ---------- VERTICES ----------

  int AddVertex ()
  {
    myVertices.Append(new asiAlgo::discr::Vertex);
    return myVertices.Length();
  }

  int GetNbVertices () const { return myVertices.Length(); }

  const asiAlgo::discr::Vertex& GetVertex (const int aIndex) const
  {
    return *(asiAlgo::discr::Vertex*)myVertices(aIndex);
  }

  asiAlgo::discr::Vertex& ChangeVertex (const int aIndex)
  {
    return *(asiAlgo::discr::Vertex*)myVertices(aIndex);
  }

  asiAlgo_EXPORT bool RemoveVertex (const int aIndex);

  // ----------- FRONTS -------------

  int AddFront ()
  {
    myFronts.Append(new asiAlgo::discr::Wire);
    return myFronts.Length();
  }

  int GetNbFronts () const { return myFronts.Length();}

  const asiAlgo::discr::Wire& GetFront (const int aIndex) const
  {
    return *(asiAlgo::discr::Wire*)myFronts(aIndex);
  }

  asiAlgo::discr::Wire& ChangeFront (const int aIndex)
  {
    return *(asiAlgo::discr::Wire*)myFronts(aIndex);
  }

  asiAlgo_EXPORT void RemoveFronts ();

  asiAlgo_EXPORT void RemoveFront (const int aIndex);

  // ----------- DUMP -------------

  asiAlgo_EXPORT void Dump (Standard_OStream& S) const;
  static asiAlgo_EXPORT void DumpVertex (const asiAlgo::discr::Vertex& vertex,
                                          Standard_OStream& S);
  static asiAlgo_EXPORT void DumpEdge (const asiAlgo::discr::Edge& edge, Standard_OStream& S);
  static asiAlgo_EXPORT void DumpFace (const asiAlgo::discr::Face& face, Standard_OStream& S);
  static asiAlgo_EXPORT void DumpWire (const asiAlgo::discr::Wire& wire, Standard_OStream& S,
                                          const char* indent = "");

 private:

  Params                            myMeshParams;
  asiAlgo::discr::Face             *myFaces;
  asiAlgo::discr::Edge             *myEdges;
  asiAlgo::discr::SequenceOfPointer myVertices;
  asiAlgo::discr::SequenceOfPointer myFronts;
  int                               myNbFaces;
  int                               myNbEdges;
};

}
}

#endif
