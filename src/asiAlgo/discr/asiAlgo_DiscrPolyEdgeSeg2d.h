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

#ifndef asiAlgo_DiscrPolyEdgeSeg2d_HeaderFile
#define asiAlgo_DiscrPolyEdgeSeg2d_HeaderFile

#include <asiAlgo_DiscrPolySeg2d.h>
#include <asiAlgo_DiscrEdgeSeg2d.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

class Edge;

// This class describes the sequence of 2D edge segments
class PolyEdgeSeg2d : public PolySeg2d
{
 public:
  // ---------- PUBLIC METHODS ----------

  PolyEdgeSeg2d (const double aTol) : PolySeg2d(aTol) {}
  // Empty constructor

  virtual ~PolyEdgeSeg2d () { DestroySegments(); }
  // Destructor

  asiAlgo_EXPORT void Init (const Edge& aEdge,
                             const int aCurveIndex);
  // Initializes with one discrete curve 2D

  asiAlgo_EXPORT void Init (const int theWireIndex,
                             const Face&     theFace);
  // Initializes with whole discrete wire (pcurves on theFace)

  virtual const Seg2d& Segment (const int index) const
  { return mySegments[index-1]; }

  // Interpolate the point by theParam on theIndex'th segment
  gp_Pnt2d Interpolate(const int theIndex,
                       const double    theParam)
  { return gp_Pnt2d(mySegments[theIndex-1].GetPoint1().XY()*(1.-theParam)+
                    mySegments[theIndex-1].GetPoint2().XY()*theParam); }

 protected:

  void DestroySegments ()
  { if (NbSegments()) { delete [] mySegments; SetNbSeg(0); } }

 private:

  EdgeSeg2d *mySegments;

};

}
}

#endif
