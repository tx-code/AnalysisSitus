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

#ifndef asiAlgo_DiscrFaceNode_HeaderFile
#define asiAlgo_DiscrFaceNode_HeaderFile

#include <gp_Pnt2d.hxx>
#include <asiAlgo_DiscrMetrics.h>
#include <asiAlgo_DiscrSegAddress.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

// The class describing a point on a face.
// The smallest amount of data stored is a 2D point.
// An object of this class can store metrics and/or
// boundary information (if the point is on boundary).
// The info may be destroyed to release memory.

class FaceNode : public gp_Pnt2d
{
 public:
  // ---------- PUBLIC METHODS ----------

  FaceNode ()
    : gp_Pnt2d(0.,0.), myMetrics(0), myBndInfo(0) {}
  // Empty constructor

  FaceNode (const gp_Pnt2d& thePoint)
    : gp_Pnt2d(thePoint), myMetrics(0), myBndInfo(0) {}
  // Constructor

  asiAlgo_EXPORT FaceNode (const FaceNode& theFaceNode);
  // Copy constructor

  asiAlgo_EXPORT FaceNode& operator= (const FaceNode&);
  // operator =

  virtual ~FaceNode () { DestroyMetrics(); DestroyBndInfo(); }
  // Destructor

  bool              AreMetrics () const { return myMetrics != 0; }
  // Returns true if the metrics are initialised

  asiAlgo_EXPORT void          SetMetrics (const Metrics& theMetrics);
  // Initializes the metrics

  void                          DestroyMetrics ()
        { if (myMetrics) delete myMetrics; myMetrics = 0; }
  // Destroys the metrics (frees memory)

  const Metrics&         GetMetrics () const { return *myMetrics; }
  // Get metrics.
  // !!! Must not be called if AreMetrics() returns false

  bool              IsOnBound () const { return myBndInfo != 0; }
  // Returns true if the point lies on boundary

  void                          SetOnBound
        (const SegAddress& theSegAddr, const double thePar)
        { if(myBndInfo)
            myBndInfo->Init(theSegAddr, thePar);
          else
            myBndInfo = new PointOnBound (theSegAddr, thePar);
        }
  // Sets boundary info for the point: the address of segment of boundary
  // and the parameter (0...1) inside the segment

  void                          DestroyBndInfo ()
        { if (myBndInfo) delete myBndInfo; myBndInfo = 0; }
  // Destroys the boundary information (frees memory)

  const SegAddress&  BndSegAddress () const
        { return myBndInfo->myAddr; }
  double                 BndSegParameter () const
        { return myBndInfo->myPar; }
  // Get boundary info (the address of segment and the parameter on it)
  // !!! Must not be called if IsOnBound() returns false

 private:
  // ---------- PRIVATE FIELDS ----------

  struct PointOnBound
  {
    PointOnBound (const SegAddress& theSegAddr,
                  const double          thePar)
      : myPar(thePar), myAddr(theSegAddr) {}
    void Init(const SegAddress& theSegAddr,
              const double          thePar)
    { myPar = thePar; myAddr = theSegAddr; }

    double     myPar;
    SegAddress myAddr;
  };

  Metrics        *myMetrics;
  PointOnBound          *myBndInfo;

};

}
}

#endif
