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

#ifndef asiAlgo_DiscrSegOnFace_HeaderFile
#define asiAlgo_DiscrSegOnFace_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrSeg2d.h>
#include <asiAlgo_DiscrFaceNode.h>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

// This class describes a segment lying on a discrete face and restricted by
// two face nodes.
class SegOnFace : public Seg2d
{
public:

  //! Default ctor.
  SegOnFace () : Seg2d() {}

  //! Ctor accepting two face nodes.
  SegOnFace (const FaceNode& node1,
             const FaceNode& node2)
  : Seg2d   (node1, node2),
    myNode1 (node1), myNode2(node2) {}
  // Constructor

  const FaceNode& GetNode1 () const { return myNode1; }
  const FaceNode& GetNode2 () const { return myNode2; }

 private:

  FaceNode myNode1;
  FaceNode myNode2;

};

}
}

#endif
