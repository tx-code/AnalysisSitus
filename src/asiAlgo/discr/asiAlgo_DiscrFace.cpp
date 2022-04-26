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

// Own include
#include <asiAlgo_DiscrFace.h>

// asiAlgo includes
#include <asiAlgo_DiscrEdge.h>
#include <asiAlgo_DiscrSegAddress.h>
#include <asiAlgo_DiscrBgrMesh.h>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

Face::Face () :
       myAlgos(0, AKNumber-1),
       myMesh(new BgrMesh),
       myNbWires(0),
       myOuterWire(0),
       myIsForward(true)
{
}

//-----------------------------------------------------------------------------

Face::~Face ()
{
  Nullify();
  delete myMesh;
}

//-----------------------------------------------------------------------------

void Face::InitWires (const int aNbWires)
{
  if (myNbWires) delete [] myWires;
  myNbWires = aNbWires;
  if (aNbWires) myWires = new Wire [aNbWires];
  myOuterWire = 0;
}

//-----------------------------------------------------------------------------

void Face::Nullify ()
{
  NullifyPCurves();
  NullifyWiresAndAlgo();
}

//-----------------------------------------------------------------------------

void Face::NullifyPCurves()
{
  // remove pcurves on this face from all edges belonging to it
  int i;
  for (i = 1; i <= myNbWires; i++) {
    Wire& wire = ChangeWire(i);
    for (int j = 1; j <= wire.GetNbEdges(); j++) {
      const PairOfPEdgeBoolean& edgeData = wire.GetEdgeData(j);
      Edge& edge = *edgeData.first;
      bool ori = edgeData.second;
      int ic = edge.FindPCurve(*this, ori);
      if (ic > 0)
        edge.RemovePCurve(ic);
    }
  }
}

//-----------------------------------------------------------------------------

void Face::NullifyWiresAndAlgo()
{
  if (myNbWires)
  {
    // free my wires
    delete [] myWires;
    myNbWires = myOuterWire = 0;
  }
  // free attached algorithms
  for (int i = myAlgos.Lower(); i <= myAlgos.Upper(); i++)
    myAlgos(i).Nullify();
}

//-----------------------------------------------------------------------------

void Face::UpdateWires ()
{
  if (!GetNbWires()) return;

  myOuterWire = 0;
  double UMin=0., UMax=0., VMin=0., VMax=0.;
  double umin, umax, vmin, vmax;
  int i;
  for (i=1; i <= GetNbWires(); i++)
  {
    Wire& wire = ChangeWire(i);
    wire.Update(*this);
    const Bnd_Box2d& aBox = wire.BndBox();
    if (aBox.IsVoid())
      continue;

    aBox.Get(umin, vmin, umax, vmax);
    if (myOuterWire == 0 ||
        ((umin <= UMin) &&
         (umax >= UMax) &&
         (vmin <= VMin) &&
         (vmax >= VMax))) {
      UMin = umin;
      UMax = umax;
      VMin = vmin;
      VMax = vmax;
      myOuterWire = i;
    }
  }
}

//-----------------------------------------------------------------------------

bool Face::
  FindAdjacentFace(const SegAddress& theInAddress,
                   SegAddress&       theOutAddress,
                   Face const*&          theOutFace) const
{
  bool isFound = false;
  const Wire& aWire = GetWire(theInAddress.WireIndex());
  const PairOfPEdgeBoolean& aEdgeData = aWire.GetEdgeData(theInAddress.EdgeIndex());
  const Edge& anEdge = *aEdgeData.first;

  // curve index must be 1 or 2 (we work only with manifold toppology)
  if(anEdge.GetNbPCurves() == 2)
  {
    bool anEdgeOri = !aEdgeData.second;
    anEdgeOri = anEdgeOri ^ !IsForward();
    theOutAddress = theInAddress;
    theOutAddress.ChangeCurveIndex() = 3 - theOutAddress.CurveIndex();
    theOutFace = &anEdge.GetPCurve(theOutAddress.CurveIndex()).GetFace();
    anEdgeOri = anEdgeOri ^ !theOutFace->IsForward();

    for(int aWireIndex = 1;
        aWireIndex <= theOutFace->GetNbWires() && !isFound;
        aWireIndex++)
    {
      const Wire& aCurWire = theOutFace->GetWire(aWireIndex);
      for(int anEdgeIndex = 1; anEdgeIndex <= aCurWire.GetNbEdges(); anEdgeIndex++)
      {
        const PairOfPEdgeBoolean& aEdgeData2 = aCurWire.GetEdgeData(anEdgeIndex);
        if(*aEdgeData2.first == anEdge && aEdgeData2.second == anEdgeOri)
        {
          theOutAddress.ChangeWireIndex() = aWireIndex;
          theOutAddress.ChangeEdgeIndex() = anEdgeIndex;
          isFound = true;
          break;
        }
      }
    }
  }
  return isFound;
}

//-----------------------------------------------------------------------------

bool Face::LocateEdge(const Edge& theEdge,
                      int&  theWireIndex,
                      int&  theEdgeIndex,
                      const bool isForward) const
{
  int iWire = myNbWires;
  for(; iWire > 0; iWire--) {
    const Wire& aWire = GetWire(iWire);
    int iEdge = aWire.GetNbEdges();
    for(; iEdge > 0; iEdge--)
    {
      const PairOfPEdgeBoolean& aEdgeData = aWire.GetEdgeData(iEdge);
      if(*aEdgeData.first == theEdge)
      {
        if(theEdge.GetNbPCurves() > 1)
        {
          const Face& aFace1 = theEdge.GetPCurve(1).GetFace();
          const Face& aFace2 = theEdge.GetPCurve(2).GetFace();
          if(aFace1 == aFace2 && aEdgeData.second != isForward)
            continue;
        }

        theWireIndex = iWire;
        theEdgeIndex = iEdge;
        return true;
      }
    }
  }
  return false;
}
