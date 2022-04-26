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

#ifndef asiAlgo_DiscrEdgeSeg2d_HeaderFile
#define asiAlgo_DiscrEdgeSeg2d_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrSeg2d.h>
#include <asiAlgo_DiscrEdge.h>

// OpenCascade includes
#include <gp_Pnt2d.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

// This class describes a segment of discrete edge in parametric space
class EdgeSeg2d : public Seg2d
{
 public:
  // ---------- CONSTRUCTORS ----------

  EdgeSeg2d () : myEdge(0), myCurveIndex(0), myIndex(0) {}
  // Empty constructor

  EdgeSeg2d (const Edge& aEdge,
             const int aCurveIndex)
    : myEdge(&aEdge), myCurveIndex(aCurveIndex), myIndex(0) {}
  // Constructor. It creates incomplete segment, use then Init(index)

  EdgeSeg2d (const Edge& aEdge,
             const int aCurveIndex,
             const int aIndex)
    : Seg2d(aEdge.GetPCurve(aCurveIndex).Point(aIndex),
                aEdge.GetPCurve(aCurveIndex).Point(aIndex+1)),
      myEdge(&aEdge), myCurveIndex(aCurveIndex), myIndex(aIndex) {}
  // Complete constructor

  void Init (const Edge& aEdge, const int aCurveIndex)
  { myEdge = &aEdge; myCurveIndex = aCurveIndex; myIndex = 0; }
  // Initialize with new pcurve. Use then Init(index)

  void Init (const Edge&     theEdge,
             const int theCurveIndex,
             const int theIndex,
             const bool theForward=true)
  { myEdge = &theEdge; myCurveIndex = theCurveIndex; myIndex = theIndex;
    int iOri = (theForward ? +1 : -1);
    ChangePoint1() = theEdge.GetPCurve(theCurveIndex).Point(theIndex);
    ChangePoint2() = theEdge.GetPCurve(theCurveIndex).Point(theIndex+iOri);
  }
  // Initialize anew completely

  void Init (const int aIndex)
  { myIndex = aIndex;
    ChangePoint1() = myEdge->GetPCurve(myCurveIndex).Point(aIndex);
    ChangePoint2() = myEdge->GetPCurve(myCurveIndex).Point(aIndex+1);
  }
  // Initialize with new index inside pcurve

  // ---------- GET METHODS ----------

  const Edge& GetEdge () const {return *myEdge;}
  // Returns edge associated with <me>

  int GetCurveIndex () const {return myCurveIndex;}
  // Returns index of pcurve of edge

  int GetIndex () const {return myIndex;}
  // Returns index of <me> inside an associated edge

 private:
  // ---------- PRIVATE FIELDS ----------

  const Edge* myEdge;
  int myCurveIndex;// index of discrete pcurve
  int myIndex;     // index of segment in the pcurve (1..nbpt-1)

};

}
}

#endif
