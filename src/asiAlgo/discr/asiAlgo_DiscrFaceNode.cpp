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

#include <asiAlgo_DiscrFaceNode.h>

using namespace asiAlgo::discr;

//=======================================================================
//function : asiAlgo_DiscrFaceNode
//purpose  : Copy constructor
//=======================================================================

FaceNode::FaceNode(const FaceNode& theFaceNode)
     : gp_Pnt2d(theFaceNode), myMetrics(0), myBndInfo(0)
{
  if(theFaceNode.AreMetrics())
    myMetrics = new Metrics(*theFaceNode.myMetrics);
  if(theFaceNode.IsOnBound())
    myBndInfo = new PointOnBound(*theFaceNode.myBndInfo);
}

//=======================================================================
//function : operator=
//purpose  :
//=======================================================================

FaceNode& FaceNode::operator= (const FaceNode& theFaceNode)
{
  (gp_Pnt2d&)*this = theFaceNode;
  if(theFaceNode.AreMetrics()) {
    if(myMetrics) *myMetrics = *theFaceNode.myMetrics;
    else myMetrics = new Metrics(*theFaceNode.myMetrics);
  }
  if(theFaceNode.IsOnBound()) {
    if(myBndInfo) *myBndInfo = *theFaceNode.myBndInfo;
    else myBndInfo = new PointOnBound(*theFaceNode.myBndInfo);
  }
  return *this;
}

//=======================================================================
//function : SetMetrics
//purpose  : 
//=======================================================================

void FaceNode::SetMetrics (const Metrics& theMetrics)
{
  if(myMetrics)
    *myMetrics = theMetrics;
  else
    myMetrics = new Metrics(theMetrics);
}
