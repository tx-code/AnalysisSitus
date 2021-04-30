//-----------------------------------------------------------------------------
// Created on: 30 April 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiAlgo_CanRecTools.h>

// OpenCascade includes
#include <gp_Lin2d.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

bool asiAlgo_CanRecTools::IsLinear(const TColgp_Array1OfPnt2d& pts,
                                   const double                toler,
                                   double&                     dev,
                                   gp_Lin2d&                   lin)
{
  const int nbPoles = pts.Length();
  //
  if ( nbPoles < 2 )
    return false;

  /* ========================
   *  Construct fitting line.
   * ======================== */

  double dMax = 0;
  int    iMax1 = 0, iMax2 = 0;
  int    i;

  for ( i = 1; i < nbPoles; ++i )
  {
    for ( int j = i + 1; j <= nbPoles; ++j )
    {
      const double dist = pts(i).SquareDistance( pts(j) );
      //
      if ( dist > dMax )
      {
        dMax  = dist;
        iMax1 = i;
        iMax2 = j;
      }
    }
  }

  double dPreci = Precision::Confusion()*Precision::Confusion();
  //
  if ( dMax < dPreci )
    return false;

  /* =========================================
   *  Test fitting line w.r.t. the input data.
   * ========================================= */

  // Prepare test line.
  gp_Vec2d vec( pts(iMax1), pts(iMax2) );
  gp_Dir2d dir( vec );
  //
  lin = gp_Lin2d( pts(iMax1), dir );

  const double tol2   = toler*toler;
  double       devMax = 0.;
  //
  for ( i = 1; i <= nbPoles; ++i )
  {
    const double
      dist = lin.SquareDistance( pts(i) );

    if ( dist > tol2 )
      return false; // Bad accuracy of approximation.

    if ( dist > devMax )
      devMax = dist;
  }

  // Return the reached deviation.
  dev = sqrt(devMax);
  return true;
}
