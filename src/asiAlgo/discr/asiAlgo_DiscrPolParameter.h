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

#ifndef asiAlgo_DiscrPolParameter_HeaderFile
#define asiAlgo_DiscrPolParameter_HeaderFile

#include <asiAlgo.h>

#include <Standard_TypeDef.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

// Class describing the parameter of an interpolated point
// on a polygon
class PolParameter
{
 public:
  // ---------- PUBLIC METHODS ----------

  PolParameter ()
    : myPar(0.), myIndex(0), isVertex(false) {}
  // Empty constructor

  PolParameter (const bool isVert, int ind, double par)
    : myPar(par), myIndex(ind), isVertex(isVert) {}
  // Full constructor

  void Set (const bool isVert, int ind, double par)
  { myPar=par; myIndex=ind; isVertex=isVert; }
  // Reinitialize

  const bool&   IsVertex () const {return isVertex;}
  const int&    Index ()    const {return myIndex;}
  const double& Param ()    const {return myPar;}
  bool&         IsVertex ()       {return isVertex;}
  int&          Index ()          {return myIndex;}
  double&       Param ()          {return myPar;}
  // Access methods

  bool operator< (const PolParameter& other) const
  {
    double p1 = double(myIndex) + myPar;
    double p2 = double(other.myIndex) + other.myPar;
    return p2-p1 > 1e-9;
  }
  // Compares me with other

 private:
  // ---------- PRIVATE FIELDS ----------

  double myPar;    // Parameter on segment [0...1]
  int    myIndex;  // Index of first point of segment
  bool   isVertex; // True if this parameter points
                   // to one of the ends of segment
};

}
}

#endif
