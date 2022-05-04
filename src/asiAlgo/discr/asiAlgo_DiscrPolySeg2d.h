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

#ifndef asiAlgo_DiscrPolySeg2d_HeaderFile
#define asiAlgo_DiscrPolySeg2d_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <Intf_Polygon2d.hxx>
#include <Bnd_Box2d.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

class Seg2d;

// This class describes the sequence of 2D segments
// to be used in the algo of intersection of polygons
class PolySeg2d : public Intf_Polygon2d
{
 public:
  // ---------- PUBLIC METHODS ----------

  PolySeg2d (const double aTol)
    : myTol(aTol), myNbSeg(0), myClosed(false) {}
  // Constructor

  virtual ~PolySeg2d () {}
  // Destructor

  virtual int NbSegments () const { return myNbSeg; }

  virtual const Seg2d& Segment (const int index) const = 0;

  virtual bool Closed () const { return myClosed; }

  virtual double DeflectionOverEstimation () const { return myTol; }

  double Tolerance () const { return myTol; }

  asiAlgo_EXPORT virtual void
    Segment(const int theIndex, gp_Pnt2d& theBegin, gp_Pnt2d& theEnd) const;

 protected:
  // ---------- PROTECTED METHODS ----------

  void SetNbSeg (const int nbSeg) { myNbSeg = nbSeg; }
  asiAlgo_EXPORT void Prepare ();

 private:
  // ---------- PRIVATE FIELDS ----------

  double myTol;
  int    myNbSeg;
  bool   myClosed;

};

}
}

#endif
