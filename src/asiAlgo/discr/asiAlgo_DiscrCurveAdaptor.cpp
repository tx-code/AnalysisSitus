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

// Own include
#include <asiAlgo_DiscrCurveAdaptor.h>

// asiAlgo includes
#include <asiAlgo_DiscrParams.h>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

double CurveAdaptor::LocalSize(const double t) const
{
  double aLocalSize = 1e100;

  if (!myMeshParams.IsDeflection() && !myMeshParams.IsDeviationAngle())
    return aLocalSize;

  double aSum = 0.;
  int aN = 0;
  ListOfCurveOnSurface::Iterator it (myCOnSList);
  for (; it.More(); it.Next())
  {
    const CurveOnSurface& aCOnS = it.Value();
    double aLS = aCOnS.LocalSize(t, myMeshParams);
    if (aLS < 1e50)
    {
      aSum += aLS;
      aN++;
    }
  }
  if (aN)
    aLocalSize = aSum / aN;
  return aLocalSize;
}

//-----------------------------------------------------------------------------

double CurveAdaptor::CurveOnSurface::LocalSize(const double t,
                                               const Params& meshParams) const
{
  // Note: the code was copied from asiFaceter_QuadTree::CalcMetrics
  double aLocalSize = 1e100;
  gp_Pnt2d aUV;
  myCurve2d->D0 (t, aUV);
  LProp3d_SLProps& adaptor = ((CurveOnSurface*)this)->myAdaptor;
  adaptor.SetParameters (aUV.X(), aUV.Y());

  // Calculate the directions of two principal curvatures
  if (adaptor.IsCurvatureDefined())
  {
    // curvatures are signed values, and we need the
    // maximum ABSOLUTE of them.
    double aMaxCurvature (adaptor.MaxCurvature());
    double aMinCurvature (adaptor.MinCurvature());
    if (aMinCurvature < -aMaxCurvature)
      aMaxCurvature = -aMinCurvature;

    // Calculate the local size at the node
    if (aMaxCurvature > Precision::Confusion())
    {
      const double R (1. / aMaxCurvature);
      if (meshParams.IsDeflection())
      {
        double aDefl = meshParams.Deflection();
        if ((R - aDefl) < Precision::Confusion())
          aLocalSize = 2. * aDefl;
        else
        {
          const double aTmp = 2. * R - aDefl;
          aLocalSize = 2.0 * Sqrt(aDefl * aTmp);
        }
        if (aLocalSize < meshParams.MinElemSize())
          aLocalSize = meshParams.MinElemSize();
      }
      else // if (theMeshParams.IsDeviationAngle())
        aLocalSize = 2. * R * meshParams.SinAngle();
    }
  }
  return aLocalSize;
}

//-----------------------------------------------------------------------------

gp_Pnt2d CurveAdaptor::CurveOnSurface::Value2d(const double t) const
{
  return myCurve2d->Value(t);
}

//-----------------------------------------------------------------------------

gp_Pnt CurveAdaptor::CurveOnSurface::ValueOnSurf(const gp_Pnt2d& uv) const
{
  myAdaptor.SetParameters(uv.X(), uv.Y());
  return myAdaptor.Value();
}
