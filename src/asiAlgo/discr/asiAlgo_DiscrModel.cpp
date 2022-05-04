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

// Own include
#include <asiAlgo_DiscrModel.h>

// asiAlgo includes
#include <asiAlgo_DiscrBgrMesh.h>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

void Model::InitFaces (const int aNbFaces)
{
  if (myNbFaces) delete [] myFaces;
  myNbFaces = aNbFaces;
  if (aNbFaces) myFaces = new Face [aNbFaces];
}

//-----------------------------------------------------------------------------

void Model::InitEdges (const int aNbEdges)
{
  if (myNbEdges) delete [] myEdges;
  myNbEdges = aNbEdges;
  if (aNbEdges) myEdges = new Edge [aNbEdges];
}

//-----------------------------------------------------------------------------

Model::~Model ()
{
  if (myNbFaces) delete [] myFaces;
  if (myNbEdges) delete [] myEdges;
  // delete vertices and fronts
  int i;
  for (i=1; i <= GetNbVertices(); i++)
    delete (Vertex*)myVertices(i);
  for (i=1; i <= GetNbFronts(); i++)
    delete (Wire*)myFronts(i);
}

//-----------------------------------------------------------------------------

bool Model::RemoveVertex (const int aIndex)
{
  // Assure that there is no edges referring to this vertex
  Vertex& aVertex = ChangeVertex(aIndex);
  int i;
  for (i=1; i <= aVertex.GetNbEdges(); i++) {
    const Edge& anEdge = aVertex.GetEdge(i);
    if (aVertex == anEdge.FirstVertex() || aVertex == anEdge.LastVertex())
      return false;
  }
  myVertices.Remove(aIndex);
  return true;
}

//-----------------------------------------------------------------------------

void Model::RemoveFronts ()
{
  while (GetNbFronts())
    RemoveFront(1);
}

//-----------------------------------------------------------------------------

void Model::RemoveFront (const int aIndex)
{
  Wire& aFront = ChangeFront(aIndex);
  // Detach the front from the edges
  int j;
  for (j=1; j <= aFront.GetNbEdges(); j++) {
    const PairOfPEdgeBoolean& anEdgeData = aFront.GetEdgeData(j);
    Edge& anEdge = *anEdgeData.first;
    int k;
    for (k=1; k <= anEdge.GetNbFronts();) {
      if (aFront == anEdge.GetFront(k))
        anEdge.RemoveFront(k);
      else
        k++;
    }
  }
  myFronts.Remove(aIndex);
}

//-----------------------------------------------------------------------------

void Model::Dump (Standard_OStream& S) const
{
  S << "Parameters : min " << myMeshParams.MinElemSize();
  if (myMeshParams.IsMaxElemSize())
    S << " max " << myMeshParams.MaxElemSize();
  if (myMeshParams.IsDeviationAngle())
    S << " angle " << myMeshParams.DeviationAngle()*180./M_PI;
  if (myMeshParams.IsDeflection())
    S << " deflection " << myMeshParams.Deflection();
  S << std::endl;

  int nbv = GetNbVertices();
  int nbe = GetNbEdges();
  int nbf = GetNbFaces();
  int nbfr = GetNbFronts();
  S << "Vertices " << nbv << std::endl;
  S << "Edges " << nbe << std::endl;
  S << "Faces " << nbf << std::endl;
  S << "Fronts " << nbfr << std::endl;

  int i;
  for (i=1; i <= nbv; i++) {
    const Vertex& vertex = GetVertex(i);
    S << "Vertex " << i << " ";
    DumpVertex(vertex,S);
  }
  for (i=1; i <= nbe; i++) {
    const Edge& edge = GetEdge(i);
    S << "Edge " << i << " ";
    DumpEdge(edge,S);
  }
  for (i=1; i <= nbf; i++) {
    const Face& face = GetFace(i);
    S << "Face " << i << " ";
    DumpFace(face,S);
  }
  for (i=1; i <= nbfr; i++) {
    const Wire& wire = GetFront(i);
    S << "Wire " << i << " ";
    DumpWire(wire,S);
  }
}

//-----------------------------------------------------------------------------

void Model::DumpVertex (const Vertex& vertex,
                               Standard_OStream& S)
{
  S << &vertex << " : "
    << vertex.GetPnt().X() << " " << vertex.GetPnt().Y()
    << " " << vertex.GetPnt().Z() << std::endl;
}

//-----------------------------------------------------------------------------

void Model::DumpEdge (const Edge& edge,
                             Standard_OStream& S)
{
  S << &edge;
  if (edge.IsNull()) {
    S << " is null" << std::endl;
  }
  else {
    S << (edge.IsDegenerated() ? " degenerated" : "")
      << (edge.GetNbFronts() ? " front" : "")
      << " : vertices " << &edge.FirstVertex() << " " << &edge.LastVertex()
      << ", " << edge.GetNbPCurves() << " pcurves "
      << edge.GetCurve().NbPoints() << " points" << std::endl;
  }
}

//-----------------------------------------------------------------------------

void Model::DumpFace (const Face& face,
                             Standard_OStream& S)
{
  S << &face;
  if (face.IsNull()) {
    S << " is null" << std::endl;
  }
  else {
    S << " : " << face.GetNbWires() << " wires";
    if (face.Mesh().NbNodes())
      S << "; " << face.Mesh().NbNodes() << " nodes " <<
                   face.Mesh().NbTriangles() << " triangles";
    S << std::endl;
    for (int i=1; i <= face.GetNbWires(); i++) {
      const Wire& wire = face.GetWire(i);
      S << "  Wire " << i << " ";
      DumpWire(wire,S,"  ");
    }
  }
}

//-----------------------------------------------------------------------------

void Model::DumpWire (const Wire& wire,
                             Standard_OStream& S,
                             const char* indent)
{
  S << &wire
    << (wire.IsClosed() ? "" : " open")
    << " : " << wire.GetNbEdges() << " edges" << std::endl;
  for (int j=1; j <= wire.GetNbEdges(); j++)
  {
    const PairOfPEdgeBoolean& edgeData = wire.GetEdgeData(j);
    S << indent << "  " << j << " " << edgeData.first
      << (edgeData.second ? "" : " reversed") << std::endl;
  }
}
