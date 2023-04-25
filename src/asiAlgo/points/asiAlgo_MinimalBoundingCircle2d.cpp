//-----------------------------------------------------------------------------
// Created on: 25 April 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Andrey Voevodin
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
#include <asiAlgo_MinimalBoundingCircle2d.h>

// OCCT includes
#include <gp_Lin2d.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

bool asiAlgo_MinimalBoundingCircle2d::Perform(const std::vector<gp_Pnt2d>& points,
                                              gp_Circ2d&                   circle2d)
{
  if (points.size() < 2)
  {
    return false;
  }

  std::vector<gp_Pnt2d> mixedPoints;
  mix(points, mixedPoints);

  circle2d = circularOnTwoPoints(mixedPoints[0], mixedPoints[1]);

  std::vector<gp_Pnt2d> checkedPoints;
  checkedPoints.push_back(mixedPoints[0]);
  checkedPoints.push_back(mixedPoints[1]);

  for (int index = 2; index < mixedPoints.size(); ++index)
  {
    if (!isPointInCircle(mixedPoints[index], circle2d))
    {
      if (!minCircleWithPoint(checkedPoints, mixedPoints[index], circle2d))
      {
        return false;
      }
    }
    checkedPoints.push_back(mixedPoints[index]);
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MinimalBoundingCircle2d::
  minCircleWithPoint(const std::vector<gp_Pnt2d>& points,
                     const gp_Pnt2d&              point,
                     gp_Circ2d&                   circle)
{
  if (points.size() < 1)
  {
    return false;
  }

  std::vector<gp_Pnt2d> mixedPoints;
  mix(points, mixedPoints);

  circle = circularOnTwoPoints(mixedPoints[0], point);

  std::vector<gp_Pnt2d> checkedPoints;
  checkedPoints.push_back(mixedPoints[0]);

  for (int index = 1; index < mixedPoints.size(); ++index)
  {
    if (!isPointInCircle(mixedPoints[index], circle))
    {
      if (!minCircleWithPoints(checkedPoints, mixedPoints[index], point, circle))
      {
        return false;
      }
    }
    checkedPoints.push_back(mixedPoints[index]);
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MinimalBoundingCircle2d::
  isFormTriangle(const gp_Pnt2d& point1,
                 const gp_Pnt2d& point2,
                 const gp_Pnt2d& point3)
{
  gp_Lin2d line(point1, gp_Dir2d(point2.XY() - point1.XY()));
  const double dist = line.Distance(point3);
  if (dist < Precision::Confusion())
  {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

double asiAlgo_MinimalBoundingCircle2d::
  findMaxDist(const gp_Pnt2d& point1,
              const gp_Pnt2d& point2,
              const gp_Pnt2d& point3,
              gp_Pnt2d&       maxPoint1,
              gp_Pnt2d&       maxPoint2)
{
  double dist = 0.0;

  double dist1 = point1.Distance(point2);
  if (dist1 >= dist + Precision::Confusion())
  {
    dist = dist1;
    maxPoint1 = point1;
    maxPoint2 = point2;
  }

  dist1 = point2.Distance(point3);
  if (dist1 >= dist + Precision::Confusion())
  {
    dist = dist1;
    maxPoint1 = point2;
    maxPoint2 = point3;
  }

  dist1 = point1.Distance(point3);
  if (dist1 >= dist + Precision::Confusion())
  {
    dist = dist1;
    maxPoint1 = point1;
    maxPoint2 = point3;
  }

  return dist;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MinimalBoundingCircle2d::
  minCircleWithPoints(const std::vector<gp_Pnt2d>& points,
                      const gp_Pnt2d&              point1,
                      const gp_Pnt2d&              point2,
                      gp_Circ2d&                   circle)
{

  circle = circularOnTwoPoints(point1, point2);

  for (int index = 0; index < points.size(); ++index)
  {
    if (!isPointInCircle(points.at(index), circle))
    {
      if (!isFormTriangle(points.at(index), point1, point2))
      {
        gp_Pnt2d maxPoint1;
        gp_Pnt2d maxPoint2;
        findMaxDist(points.at(index), point1, point2, maxPoint1, maxPoint2);
        circle = circularOnTwoPoints(maxPoint1, maxPoint2);
      }
      else if (!circularOnThreePoints(points.at(index), point1, point2, circle))
      {
        return false;
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

gp_Circ2d asiAlgo_MinimalBoundingCircle2d::
  circularOnTwoPoints(const gp_Pnt2d& point1,
                      const gp_Pnt2d& point2)
{
  gp_Circ2d circle2d;

  circle2d.SetLocation(0.5 * (point1.XY() + point2.XY()));
  circle2d.SetRadius(0.5 * point1.Distance(point2));

  return circle2d;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MinimalBoundingCircle2d::
  circularOnThreePoints(const gp_Pnt2d& point1,
                        const gp_Pnt2d& point2,
                        const gp_Pnt2d& point3,
                        gp_Circ2d&      circle)
{
  // p     - semiperimeter of the triangle.
  // a,b,c - sides of a triangle.
  // R     - radius of the circumcircle.
  // x0,y0 - coordinates of the center of the circumscribed circle.
  // p1,p2,p3 - point1, point2, point3.
  //
  //      a+b+c
  // p = -------
  //        2
  //
  //               a*b*c
  // R = ---------------------------
  //     4.0*sqrt(p*(p-a)*(p-b)*(p-c))
  //
  //             p1.y*(p2.x^2+p2.y^2-p3.x^2-p3.y^2)+p2.y*(p3.x^2+p3.y^2-p1.x^2-p1.y^2)+p3.y*(p1.x^2+p1.y^2-p2.x^2-p2.y^2)
  // x0 = -1.0 * -------------------------------------------------------------------------------------------------------
  //                                  2.0*(p1.x*(p2.y-p3.y)+p2.x*(p3.y-p1.y)+p3.x*(p1.y-p2.y))
  //
  //      p1.x*(p2.x^2+p2.y^2-p3.x^2-p3.y^2)+p2.x*(p3.x^2+p3.y^2-p1.x^2-p1.y^2)+p3.x*(p1.x^2+p1.y^2-p2.x^2-p2.y^2)
  // y0 = -------------------------------------------------------------------------------------------------------
  //                           2.0*(p1.x*(p2.y-p3.y)+p2.x*(p3.y-p1.y)+p3.x*(p1.y-p2.y))
  //
  const double a = point1.Distance(point2);
  const double b = point2.Distance(point3);
  const double c = point3.Distance(point1);
  const double p = 0.5 * (a + b + c);
  const gp_Pnt2d& p1 = point1;
  const gp_Pnt2d& p2 = point2;
  const gp_Pnt2d& p3 = point3;

  const double sqrtValue = p * (p - a) * (p - b) * (p - c);
  if (sqrtValue <= 0.0)//Precision::Confusion())
  {
    return false;
  }

  const double denominatorOfRadius = 4.0 * std::sqrt(sqrtValue);
  double R = (a * b * c) / denominatorOfRadius;
  circle.SetRadius(R);

  const double denominatorOfCenter = 2.0 * (p1.X() * (p2.Y() - p3.Y()) + p2.X() * (p3.Y() - p1.Y()) + p3.X() * (p1.Y() - p2.Y()));
  if (abs(denominatorOfCenter) <= 0.0)// Precision::Confusion())
  {
    return false;
  }

  double x0 = -1.0 * (p1.Y() * (p2.X() * p2.X() + p2.Y() * p2.Y() - p3.X() * p3.X() - p3.Y() * p3.Y()) +
                      p2.Y() * (p3.X() * p3.X() + p3.Y() * p3.Y() - p1.X() * p1.X() - p1.Y() * p1.Y()) +
                      p3.Y() * (p1.X() * p1.X() + p1.Y() * p1.Y() - p2.X() * p2.X() - p2.Y() * p2.Y())) / denominatorOfCenter;

  double y0 = (p1.X() * (p2.X() * p2.X() + p2.Y() * p2.Y() - p3.X() * p3.X() - p3.Y() * p3.Y()) +
               p2.X() * (p3.X() * p3.X() + p3.Y() * p3.Y() - p1.X() * p1.X() - p1.Y() * p1.Y()) +
               p3.X() * (p1.X() * p1.X() + p1.Y() * p1.Y() - p2.X() * p2.X() - p2.Y() * p2.Y())) / denominatorOfCenter;

  circle.SetLocation(gp_Pnt2d(x0, y0));

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MinimalBoundingCircle2d::
  isPointInCircle(const gp_Pnt2d&  point,
                  const gp_Circ2d& circle2d)
{
  const double dist = point.Distance(circle2d.Location());
  if (dist > circle2d.Radius() - Precision::Confusion())
  {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_MinimalBoundingCircle2d::
  mix(const std::vector<gp_Pnt2d>& points,
      std::vector<gp_Pnt2d>&       mixedPoints)
{
  // TODO.

  mixedPoints = points;
}
