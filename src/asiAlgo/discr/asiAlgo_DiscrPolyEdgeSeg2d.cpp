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

#include <asiAlgo_DiscrPolyEdgeSeg2d.h>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

void PolyEdgeSeg2d::Init (const Edge& aEdge,
                                 const int aCurveIndex)
{
  if (NbSegments()) DestroySegments();

  const Curve2d& aCurve = aEdge.GetPCurve(aCurveIndex);
  SetNbSeg (aCurve.NbPoints() - 1);
  mySegments = new EdgeSeg2d [NbSegments()];
  int i;
  for (i=1; i <= NbSegments(); i++)
    mySegments[i-1].Init (aEdge, aCurveIndex, i);

  Prepare();
}

//-----------------------------------------------------------------------------

void PolyEdgeSeg2d::Init (const int theWireIndex,
                          const Face&     theFace)
{
  int iNbSeg=-1;
  int iEdge;
  const Wire& aWire=theFace.GetWire(theWireIndex);

  for (iEdge=1; iEdge<=aWire.GetNbEdges(); iEdge++)
    iNbSeg += aWire.GetEdgeData(iEdge).first->GetCurve().NbPoints();

  SetNbSeg (iNbSeg);
  mySegments = new EdgeSeg2d [iNbSeg];

  iNbSeg = 0;
  for (iEdge=1; iEdge<=aWire.GetNbEdges(); iEdge++)
  {
    // Every edge pcurve (on theFace) is appended as segments
    const PairOfPEdgeBoolean& anEdgeData = aWire.GetEdgeData(iEdge);
    const Edge& anEdge  = *anEdgeData.first;
    bool isForward = anEdgeData.second;
    int iCur=anEdge.FindPCurve(theFace,isForward);
    if (iCur==0)
    {
#     ifdef DEB
      std::cout << "Warning : (asiAlgo_DiscrPolyEdgeSeg2d) no pcurve "<<iEdge<<std::endl;
#     endif
      continue;
    }
    int iPt;
    if (isForward)
      for (iPt=1; iPt<anEdge.GetCurve().NbPoints(); iPt++)
        mySegments[iNbSeg++].Init (anEdge, iCur, iPt);
    else
      for (iPt=anEdge.GetCurve().NbPoints(); iPt>1; iPt--)
        mySegments[iNbSeg++].Init (anEdge, iCur, iPt, false);
  }

  Prepare();
}
