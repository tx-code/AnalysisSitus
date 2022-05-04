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

#ifndef asiAlgo_DiscrVertex_HeaderFile
#define asiAlgo_DiscrVertex_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrSequenceOfPointer.h>

// OpenCascade includes
#include <gp_Pnt.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

class Edge;

// This class describes a vertex
class Vertex
{
public:

  Vertex()
  : myPnt (0.,0.,0.),
    myTolerance(1e-7) {}

  // ---------- POINT ACCESS ----------

  const gp_Pnt& GetPnt () const { return myPnt; }
  // Returns the point

  gp_Pnt&       ChangePnt () { return myPnt; }
  // Returns the point

  const double& Tolerance () const { return myTolerance; }
  // Returns the tolerance

  void SetTolerance (const double theTol) { myTolerance = theTol; }
  // sets a tolerance

  // ---------- CONNECTIONS TO EDGES ---------

  void                  AddEdge (const Edge& theEdge)
        { myEdges.Append((void*)&theEdge); }
  // Adds an edge

  int      GetNbEdges () const { return myEdges.Length();}
  // Returns the number of adjacent edges

  const Edge&    GetEdge (const int aIndex) const
        { return *(Edge*)myEdges(aIndex); }
  // Returns the edge by index

  Edge&          ChangeEdge (const int aIndex)
        { return *(Edge*)myEdges(aIndex); }
  // Returns the non-const edge by index

  void                  RemoveEdge (const int aIndex)
        { myEdges.Remove(aIndex); }
  // Removes the edge by index

  // ---------- CONSULTING -------------

  bool operator == (const Vertex& aOther) const
  { return this == &aOther; }

 private:

  Vertex (const Vertex&);
  // Hides copy constructor

  Vertex& operator = (const Vertex&);
  // Hides assignment operator

  // ---------- PRIVATE FIELDS ----------

  gp_Pnt            myPnt;
  double            myTolerance;
  SequenceOfPointer myEdges;

};

}
}

#endif
