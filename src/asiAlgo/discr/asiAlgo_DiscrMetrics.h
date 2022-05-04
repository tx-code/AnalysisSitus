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

#ifndef asiAlgo_DiscrMetrics_HeaderFile
#define asiAlgo_DiscrMetrics_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <gp_XYZ.hxx>
#include <Standard_TypeDef.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

// The class describing properties of a point on a face:
//  - Metric as the first quadratic form
//  - Local Size
//  - Min and Max curvature vectors
class Metrics
{
 public:
  // ---------- PUBLIC METHODS ----------

  inline Metrics ();
  // Constructor

  inline Metrics (const double theMetric[3],
                   const double theLocalSize = 0.0);
  // constructor

  double         LocalSize () const      { return myLocalSize; }
  void           SetLocalSize (const double theSz)
                                                { myLocalSize = theSz;}
  // Accessing the Local Size

  const double * Metric () const { return myMetric; }
  double         Metric (const int i) const
                                        { return myMetric[i]; }
  inline void           SetMetric (const double theCoef[3]);
  // Accessing the Metric coefficients

  const Standard_ShortReal * CurvMin () const   { return &myCurv[0]; }
  inline void                SetCurvMin (const gp_XYZ& theDir);
  // Accessing the Minimal Curvature

  const Standard_ShortReal * CurvMax () const   { return &myCurv[3]; }
  inline void                SetCurvMax (const gp_XYZ& theDir);
  // Accessing the Maximal Curvature

 private:
  // ---------- PRIVATE FIELDS ----------

  double myLocalSize; // on Export: distance to nearest node
  double myMetric[3]; // on Export: normal direction
  float  myCurv[6];   // min curvature, then max curvature
                      // on Export: partial derivative vectors

};

// =================== INLINE METHODS ======================

inline Metrics::Metrics ()
     : myLocalSize(0.)
{
  myMetric[2] = myMetric[1] = myMetric[0] = 0.;
  myCurv[5] = myCurv[4] = myCurv[3] = myCurv[2] = myCurv[1] = myCurv[0] = 0.;
}

inline Metrics::Metrics (const double theMetric[3],
                                       const double theLocalSize)
     : myLocalSize(theLocalSize)
{
  myMetric[0] = theMetric[0];
  myMetric[1] = theMetric[1];
  myMetric[2] = theMetric[2];

  myCurv[5] = myCurv[4] = myCurv[3] = myCurv[2] = myCurv[1] = myCurv[0] = 0.;
}

//=======================================================================
//function : SetMetric
//purpose  : Set the first fundamental form coefficients at the node
//=======================================================================

inline void Metrics::SetMetric (const double theCoef[3])
{
  myMetric[0] = theCoef[0];
  myMetric[1] = theCoef[1];
  myMetric[2] = theCoef[2];
}

//=======================================================================
//function : SetCurvMin
//purpose  : Set the vector of minimal curvature direction
//=======================================================================

inline void Metrics::SetCurvMin (const gp_XYZ& theDir)
{
  myCurv[0] = Standard_ShortReal (theDir.X());
  myCurv[1] = Standard_ShortReal (theDir.Y());
  myCurv[2] = Standard_ShortReal (theDir.Z());
}

//=======================================================================
//function : SetCurvMax
//purpose  : Set the vector of maximal curvature direction
//=======================================================================

inline void Metrics::SetCurvMax (const gp_XYZ& theDir)
{
  myCurv[3] = Standard_ShortReal (theDir.X());
  myCurv[4] = Standard_ShortReal (theDir.Y());
  myCurv[5] = Standard_ShortReal (theDir.Z());
}

}
}

#endif
