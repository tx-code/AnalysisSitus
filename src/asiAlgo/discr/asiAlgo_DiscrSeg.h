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

#ifndef asiAlgo_DiscrSeg_HeaderFile
#define asiAlgo_DiscrSeg_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <gp_Pnt.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASIEXT_Faceter
//!
//! Module namespace.
namespace asiAlgo
{

// This class describes a straight segment of line restricted by
// two points in 3D space
class Seg
{
 public:
  // ---------- CONSTRUCTORS ----------

  Seg () : myP1(0,0,0), myP2(0,0,0) {}
  // Empty constructor

  Seg (const gp_Pnt& p1, const gp_Pnt& p2)
    : myP1(p1), myP2(p2) {}
  // Complete constructor

  // ---------- GET METHODS ----------

  const gp_Pnt& GetPoint1 () const { return myP1; }
  // Returns first point

  const gp_Pnt& GetPoint2 () const { return myP2; }
  // Returns second point

  gp_Pnt& ChangePoint1 () { return myP1; }
  // Returns first point to change

  gp_Pnt& ChangePoint2 () { return myP2; }
  // Returns second point to change

 private:
  // ---------- PRIVATE FIELDS ----------

  gp_Pnt myP1, myP2;

};

}

#endif
