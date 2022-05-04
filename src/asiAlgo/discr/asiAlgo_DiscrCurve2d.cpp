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

#include <asiAlgo_DiscrCurve2d.h>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

void Curve2d::Append(const gp_Pnt2d& thePoint)
{
  myPoints.SetValue(NbPoints(), thePoint);
}

//-----------------------------------------------------------------------------

void Curve2d::InsertAfter(const int theIndex, const gp_Pnt2d& thePoint)
{
  #ifdef DEB
    if (theIndex < 1 || theIndex > NbPoints())
      throw Standard_OutOfRange("asiAlgo_DiscrCurve2d::InsertAfter");
  #endif

  int i(myPoints.Size());
  for(; i > theIndex; --i)
    myPoints.SetValue(i, myPoints.Value(i - 1));
  myPoints.SetValue(i, thePoint);
}

//-----------------------------------------------------------------------------

void Curve2d::InsertBefore(const int theIndex, const gp_Pnt2d& thePoint)
{
  #ifdef DEB
    if (theIndex < 1 || theIndex > NbPoints())
      throw Standard_OutOfRange("asiAlgo_DiscrCurve2d::InsertBefore");
  #endif

  int i(myPoints.Size());
  for(; i > theIndex - 1; --i)
    myPoints.SetValue(i, myPoints.Value(i - 1));
  myPoints.SetValue(i, thePoint);
}

//-----------------------------------------------------------------------------

void Curve2d::Remove(const int theFrom, const int theTo)
{
  #ifdef DEB
    if(theFrom < 1 || theFrom > NbPoints() ||
       theTo < 1 || theTo > NbPoints() || theFrom > theTo)
       throw Standard_OutOfRange("asiAlgo_DiscrCurve2d::Remove");
  #endif

  NCollection_Vector<gp_Pnt2d> aVector;
  int aRange[2] = {theFrom - 1, theTo - 1};
  for(int i(0); i < NbPoints(); ++i)
  {
    if(i >= aRange[0] && i <= aRange[1])
      continue;
    aVector.Append(myPoints.Value(i));
  }

  myPoints.Clear();
  myPoints.Assign(aVector);
}
