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
#include <asiAlgo_DiscrCurve.h>

// OpenCascade includes
#include <gp_Lin.hxx>
#include <Precision.hxx>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

void Curve::InsertAfter(const int     index,
                        const gp_Pnt& point)
{
  #ifdef DEB
    if (index < 1 || index > NbPoints())
      throw Standard_OutOfRange("Curve::InsertAfter");
  #endif

  int i( m_points.Size() );

  for ( ; i > index; --i )
    m_points.SetValue( i, m_points.Value(i - 1) );

  m_points.SetValue(i, point);
}

//-----------------------------------------------------------------------------

void Curve::InsertBefore(const int     index,
                         const gp_Pnt& point)
{
  #ifdef DEB
    if ( index < 1 || index > NbPoints() )
      throw Standard_OutOfRange("Curve::InsertBefore");
  #endif

  int i( m_points.Size() );

  for( ; i > index - 1; --i )
    m_points.SetValue( i, m_points.Value(i - 1) );

  m_points.SetValue(i, point);
}

//-----------------------------------------------------------------------------

void Curve::Remove(const int from,
                   const int to)
{
  #ifdef DEB
    if(from < 1 || from > m_points.Size() ||
       to < 1 || to > m_points.Size() || from > theTo)
       throw Standard_OutOfRange("Curve::Remove");
  #endif

  NCollection_Vector<gp_Pnt> vector;
  int range[2] = {from - 1, to - 1};
  for ( int i(0); i < NbPoints(); ++i )
  {
    if ( i >= range[0] && i <= range[1] )
      continue;

    vector.Append( m_points.Value(i) );
  }

  m_points.Clear();
  m_points.Assign(vector);
}

//-----------------------------------------------------------------------------

double Curve::ComputeLength(const int    firstIndex,
                            const double firstParameter,
                            const int    secondIndex,
                            const double secondParameter,
                            const double maxLength) const
{
  if ( firstIndex < 1 || firstIndex > NbPoints () ||
       secondIndex < 1 || secondIndex > NbPoints () ||
       firstIndex > secondIndex)
  {
    Standard_OutOfRange::Raise("Curve::ComputeLength");
  }

  gp_Pnt points[2];

  Interpolate(firstIndex, firstParameter, points[0]);
  Interpolate(secondIndex, secondParameter, points[1]);

  double length(0.);

  if ( firstIndex == secondIndex )
    length = points[0].Distance(points[1]);
  else
    length = points[0].Distance( m_points.Value(firstIndex) );

  int i = firstIndex;
  for ( ; i <= secondIndex - 1 && length < maxLength; ++i )
    length += m_points.Value(i).Distance(m_points.Value(i + 1));

  if ( firstIndex < secondIndex && length < maxLength )
    length += points[1].Distance( Point(secondIndex - 1) );

  return length;
}

//-----------------------------------------------------------------------------

double Curve::ComputeWidth(const int    firstIndex,
                           const double firstParameter,
                           const int    secondIndex,
                           const double secondParameter) const
{
  Standard_OutOfRange_Raise_if(firstIndex < 1 || firstIndex > NbPoints() ||
                               secondIndex < 1 || secondIndex > NbPoints() ||
                               firstIndex > secondIndex,
    "Curve::ComputeWidth"
  );

  gp_Pnt points[2];

  Interpolate(firstIndex, firstParameter, points[0]);
  Interpolate(secondIndex, secondParameter, points[1]);

  double length = points[0].Distance(points[1]);

  if ( length < Precision::Confusion() )
    return 0.;

  double width(0.);

  gp_Lin line( points[0], gp_Dir( points[1].XYZ() - points[0].XYZ() ) );
  int i(firstIndex);
  double distance(0.);

  for ( ; i <= secondIndex - 1; ++i )
  {
    distance = line.Distance( m_points.Value(i) );
    width    = Max(distance, width);
  }

  return width;
}
