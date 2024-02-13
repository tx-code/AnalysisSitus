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

#ifndef asiAlgo_DiscrCurve_HeaderFile
#define asiAlgo_DiscrCurve_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

#include <gp_Pnt.hxx>
#include <NCollection_Vector.hxx>
#include <Precision.hxx>

#ifdef DEB
#include <Standard_OutOfRange.hxx>
#endif

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

//! Discrete 3D curve.
class Curve
{
public:
  //! Empty constructor
  Curve() : m_points(), m_fMinSize(Precision::Confusion())
  {}

  //! Destructor
  ~Curve()
  {
    Clear();
  }

public:

  //! Gets value of min size of segment allowed for this curve.
  double GetSegmentMinSize() const
  {
    return m_fMinSize;
  }

  //! Sets value of min size of segment allowed for this curve.
  void SetSegmentMinSize(const double theValue)
  {
    m_fMinSize = Max(theValue, Precision::Confusion());
  }

  //! Returns number of points
  int NbPoints() const
  {
    return m_points.Size();
  }

  //! Returns constant reference to a point by its 1-based index.
  const gp_Pnt& Point(const int index) const
  {
    #ifdef DEB
      if(index < 1 || index > NbPoints())
         throw Standard_OutOfRange("asiAlgo_DiscrCurve::Point");
    #endif

    return m_points.Value(index - 1);
  }

  //! Returns reference to a point by its 1-based index.
  gp_Pnt& ChangePoint(const int index)
  {
    #ifdef DEB
      if(index < 1 || index > NbPoints())
         throw Standard_OutOfRange("asiAlgo_DiscrCurve::ChangePoint");
    #endif

    return m_points.ChangeValue(index - 1);
  }

  bool IsEmpty() const
  {
    return m_points.Size() == 0;
  }

  void Append(const gp_Pnt& point)
  {
    m_points.Append(point);
  }

  //! Inserts a new point after the point with the passed index.
  //! Not thread-safe.
  asiAlgo_EXPORT void
    InsertAfter(const int     index,
                const gp_Pnt& point);

  //! Inserts a new point before the point with the passed index.
  //! Not thread-safe.
  asiAlgo_EXPORT void
    InsertBefore(const int     index,
                 const gp_Pnt& point);

  //! Removes the point with the passed index in the range [from, last].
  //! Not thread-safe.
  asiAlgo_EXPORT void
    Remove(const int from,
           const int last);

  //! Performs linear interpolation between points using parameter [0...1].
  //! After computation, the obtained point lies on a segment which is
  //! defined by points with `index` and `index + 1` indexes.
  void Interpolate(const int    index,
                   const double parameter,
                   gp_Pnt&      point) const
  {
    #ifdef DEB
      if (index < 1 || index >= NbPoints())
        throw Standard_OutOfRange("asiAlgo_DiscrCurve::Interpolate");
    #endif

    point.ChangeCoord().SetLinearForm( 1. - parameter,
                                       m_points.Value(index - 1).XYZ(),
                                       parameter,
                                       m_points.Value(index).XYZ() );
  }

  void Clear()
  {
    m_points.Clear();
  }

  //! Computes the length of the part of the curve,
  //! breaking computation upon reachment of `maxLength`.
  asiAlgo_EXPORT double
    ComputeLength(const int    firstIndex,
                  const double firstParameter,
                  const int    secondIndex,
                  const double secondParameter,
                  const double maxLength) const;

  //! Computes the width of the contour composed by the part of the curve
  //! and the segment closing the ends of this part.
  asiAlgo_EXPORT double
    ComputeWidth(const int    firstIndex,
                 const double firstParameter,
                 const int    secondIndex,
                 const double secondParameter) const;

private:

  NCollection_Vector<gp_Pnt> m_points;
  double                     m_fMinSize;

};

}
}

#endif
