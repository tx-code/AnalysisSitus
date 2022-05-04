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

#include <asiAlgo_DiscrEdge.h>
#include <asiAlgo_DiscrFace.h>
#include <asiAlgo_DiscrWire.h>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

void Edge::Init ()
{
  DestroyCurves();
  myParams.Clear();
  myFronts.Clear();
  myDegenerated = false;
}

//-----------------------------------------------------------------------------

void Edge::InitDegenerated (const Face& theFace)
{
  Init();
  myPCurves.Append(new Curve2d(theFace));
  myDegenerated = true;
}

//-----------------------------------------------------------------------------

int Edge::AddPCurve (const Face& theFace,
                     const bool isEforward)
{
  if (IsNull() || IsDegenerated()) return 0;

  // Try to find a pcurve on this face
  int i;
  ListOfPointer::Iterator anIter(myPCurves);

  for(i = 1; anIter.More(); anIter.Next(), ++i)
  {
    Curve2d& aCurve = *(Curve2d*) anIter.Value();
    if(aCurve.GetFace() == theFace)
      break;
  }
  if (i > GetNbPCurves()) {
    // not found --> add new
    myPCurves.Append(new Curve2d(theFace));
  }
  else
  {
    if(i < GetNbPCurves())
    {
      ListOfPointer::Iterator anItNext = anIter;
      anItNext.Next();

      const Curve2d& aNextCurve =
        *(Curve2d * const &) anItNext.Value();

      if(aNextCurve.GetFace() == theFace)
        return 0;  // do not add the 3rd
    }
    // insert new pcurve, forward must be first
    Curve2d* aCurve = new Curve2d(theFace);
    if (isEforward)
      myPCurves.InsertBefore(aCurve, anIter);
    else
    {
      myPCurves.InsertAfter(aCurve, anIter);
      ++i;
    }
  }
  return i;
}

//-----------------------------------------------------------------------------

int Edge::FindPCurve
  (const Face& theFace, const bool isEforward) const
{
  // Find a first (forward) pcurve on this face
  int i = 1;
  ListOfPointer::Iterator anIter(myPCurves);

  for(; anIter.More(); anIter.Next(), ++i)
  {
    const Curve2d& aCurve =
      *(Curve2d * const &) anIter.Value();

    if(aCurve.GetFace() == theFace)
      break;
  }

  if (i > GetNbPCurves()) i = 0;
  else
  {
    // Check if the second exists
    if(i < GetNbPCurves())
    {
      anIter.Next();
      const Curve2d& aCurve =
        *(Curve2d * const &) anIter.Value();

      if(aCurve.GetFace() == theFace && !isEforward)
        ++i; // Take the second
    }
  }
  return i;
}

//-----------------------------------------------------------------------------

bool Edge::FindAdjacentFace
  (const bool isEForward,
   Face const*&    theOutFace,
   const bool useFaceOri) const
{
  for(int i = 1; i <= GetNbPCurves(); i++) {
    const Curve2d& aPCurve = GetPCurve(i);
    const Face& aFace = aPCurve.GetFace();
    const bool isFReversed = (useFaceOri && !aFace.IsForward());
    for(int iW = 1; iW <= aFace.GetNbWires(); iW++) {
      const Wire& aWire = aFace.GetWire(iW);
      for(int iE = 1; iE <= aWire.GetNbEdges(); iE++) {
        const PairOfPEdgeBoolean& anEdgeData = aWire.GetEdgeData(iE);
        if (this == anEdgeData.first) {
          const bool isEReversed = (!anEdgeData.second != isFReversed);
          if (isEReversed == !isEForward) {
            theOutFace = &aFace;
            return true;
          }
        }
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------

void Edge::AddFront (const Wire& aFront)
{
  bool exists = false;
  int i;
  for (i=1; i <= GetNbFronts() && !exists; i++)
    if (GetFront(i) == aFront)
      exists = true;
  if (!exists)
    myFronts.Append((void*)&aFront);
}

//-----------------------------------------------------------------------------

void Edge::DestroyCurves ()
{
  myCurve.Clear();

  if(!myPCurves.IsEmpty())
  {
    ListOfPointer::Iterator anIter(myPCurves);
    for(; anIter.More(); anIter.Next())
      delete (Curve2d*) anIter.Value();
    myPCurves.Clear();
  }

  int i;
  if (!myFronts.IsEmpty()) {
    for (i=1; i <= GetNbFronts(); i++)
      delete (Wire*)myFronts(i);
    myFronts.Clear();
  }
}

//-----------------------------------------------------------------------------

void Edge::RemoveFront (const int aIndex)
{
  delete (Wire*)myFronts(aIndex);
  myFronts.Remove(aIndex);
}

//-----------------------------------------------------------------------------

void Edge::RemovePCurve (const int theIndex)
{
  #if !defined No_Exception && !defined No_Standard_OutOfRange
    if (theIndex < 1 || theIndex > GetNbPCurves())
      throw Standard_OutOfRange("asiAlgo_DiscrEdge::RemovePCurve");
  #endif

  ListOfPointer::Iterator anIter(myPCurves);
  for(int i = 1; i != theIndex; anIter.Next(), ++i);
  delete (Curve2d*) anIter.Value();
  myPCurves.Remove(anIter);
}
