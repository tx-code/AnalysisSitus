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

#ifndef asiAlgo_DiscrCurve2d_HeaderFile
#define asiAlgo_DiscrCurve2d_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

#include <TColgp_SequenceOfPnt2d.hxx>
#include <NCollection_Vector.hxx>
#include <gp_Pnt2d.hxx>
#ifdef DEB
#include <Standard_OutOfRange.hxx>
#endif

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

class Face;

//! A discrete 2D curve that is a sequence of points in the parametric
//! space of a surface.
class Curve2d
{
public:

  Curve2d(const Face& aFace)
  : myFace(&aFace), myPoints()
  {
  }

  ~Curve2d()
  {
    myPoints.Clear();
  }
  // Constructor defining the face.

  // ---------- ACCESS METHODS --------

  //! Returns face associated with <me>.
  const Face& GetFace () const
  {
    return *myFace;
  }

  //! Returns the number of points.
  int NbPoints() const
  {
    return myPoints.Size();
  }

  //! Returns constant refernce to a point.
  const gp_Pnt2d& Point(const int theIndex) const
  {
    return myPoints.Value(theIndex - 1);
  }

  //! Returns refernce to a point.
  gp_Pnt2d& ChangePoint(const int theIndex)
  {
    return myPoints.ChangeValue(theIndex - 1);
  }

  //! Not thread-safe
  //! Adds point to the end of array.
  asiAlgo_EXPORT void Append(const gp_Pnt2d& thePoint);

  //! Not thread-safe
  //! Adds new point after point with theIndex index.
  asiAlgo_EXPORT void InsertAfter(const int theIndex, const gp_Pnt2d& thePoint);

  //! Not thread-safe
  //! Adds new point before point with theIndex index.
  asiAlgo_EXPORT void InsertBefore(const int theIndex, const gp_Pnt2d& thePoint);

  //! Not thread-safe
  //! Removes a set of points in the interval [theFrom, theTo].
  asiAlgo_EXPORT void Remove(const int theFrom, const int theTo);

  //! Performs linear interpolation between points using parameter[0...1].
  //! After computation thePoint lies on a segment
  //! which is educated by points with theIndex and theIndex + 1 indexes.
  void Interpolate(const int theIndex,
                   const double theParameter,
                   gp_Pnt2d& thePoint) const
  {
    #ifdef DEB
    if (theIndex < 1 || theIndex >= NbPoints())
      throw Standard_OutOfRange("asiAlgo_DiscrCurve2d::Interpolate");
    #endif

    thePoint.ChangeCoord().SetLinearForm(1. - theParameter,
      myPoints.Value(theIndex - 1).XY(), theParameter,
      myPoints.Value(theIndex).XY());
  }

  // ---------- PRIVATE FIELDS ----------
private:
  const Face* myFace;
  NCollection_Vector<gp_Pnt2d> myPoints;
};

}
}

#endif
