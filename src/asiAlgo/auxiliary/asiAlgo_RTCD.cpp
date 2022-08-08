//-----------------------------------------------------------------------------
// Created on: 29 March 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

// asiAlgo includes
#include <asiAlgo_RTCD.h>

//-----------------------------------------------------------------------------

RTCD::Vector::Vector(const Point& P) : x(P.x), y(P.y), z(P.z) {}

//-----------------------------------------------------------------------------

RTCD::Point::Point(const Vector& V)
{
  x = V.x;
  y = V.y;
  z = V.z;
}

//-----------------------------------------------------------------------------

RTCD::Point RTCD::Point::operator+=(const Vector& V)
{
  x += V.x;
  y += V.y;
  z += V.z;
  return *this;
}

//-----------------------------------------------------------------------------

RTCD::Point RTCD::Point::operator-=(const Vector& V)
{
  x -= V.x;
  y -= V.y;
  z -= V.z;
  return *this;
}

//-----------------------------------------------------------------------------

RTCD::Point RTCD::Point::operator+(const Vector& V)
{
  Point result(*this);
  result += V;
  return result;
}

//-----------------------------------------------------------------------------

RTCD::Point RTCD::Point::operator-(const Vector& V)
{
  Point result(*this);
  result -= V;
  return result;
}

//-----------------------------------------------------------------------------

RTCD::AABB::AABB(const Bnd_Box& bbox)
{
  gp_Pnt Pmin = bbox.CornerMin();
  gp_Pnt Pmax = bbox.CornerMax();

  this->min = Point( Pmin.X(), Pmin.Y(), Pmin.Z() );
  this->max = Point( Pmax.X(), Pmax.Y(), Pmax.Z() );
}

//-----------------------------------------------------------------------------

void RTCD::AABB::Get(double& xmin, double& ymin, double& zmin,
                     double& xmax, double& ymax, double& zmax) const
{
  xmin = min.x;
  ymin = min.y;
  zmin = min.z;
  xmax = max.x;
  ymax = max.y;
  zmax = max.z;
}

//-----------------------------------------------------------------------------

void RTCD::AABB::Add(const RTCD::Point& P)
{
  if ( IsVoid )
  {
    min    = P;
    max    = P;
    IsVoid = false;
  }
  else
  {
    if ( P.x < min.x ) min.x = P.x;
    if ( P.y < min.y ) min.y = P.y;
    if ( P.z < min.z ) min.z = P.z;
    if ( P.x > max.x ) max.x = P.x;
    if ( P.y > max.y ) max.y = P.y;
    if ( P.z > max.z ) max.z = P.z;
  }
}

//-----------------------------------------------------------------------------

void RTCD::AABB::Add(const double x, const double y, const double z)
{
  this->Add( Point(x, y, z) );
}

//-----------------------------------------------------------------------------

bool RTCD::AABB::IsOut(const Point& P,
                       const double tolerance) const
{
  double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
  this->Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);

  if ( P.x < Xmin && Abs(P.x - Xmin) > tolerance ) return true;
  if ( P.x > Xmax && Abs(P.x - Xmax) > tolerance ) return true;

  if ( P.y < Ymin && Abs(P.y - Ymin) > tolerance ) return true;
  if ( P.y > Ymax && Abs(P.y - Ymax) > tolerance ) return true;

  if ( P.z < Zmin && Abs(P.z - Zmin) > tolerance ) return true;
  if ( P.z > Zmax && Abs(P.z - Zmax) > tolerance ) return true;

  return false;
}

//-----------------------------------------------------------------------------

bool RTCD::AABB::IsOut(const AABB&  other,
                       const double tolerance) const
{
  double OXmin, OYmin, OZmin, OXmax, OYmax, OZmax;
  other.Get(OXmin, OYmin, OZmin, OXmax, OYmax, OZmax);

  double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
  this->Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);

  if ( OXmax < Xmin && Abs(OXmax - Xmin) > tolerance ) return true;
  if ( OXmin > Xmax && Abs(OXmin - Xmax) > tolerance ) return true;

  if ( OYmax < Ymin && Abs(OYmax - Ymin) > tolerance ) return true;
  if ( OYmin > Ymax && Abs(OYmin - Ymax) > tolerance ) return true;

  if ( OZmax < Zmin && Abs(OZmax - Zmin) > tolerance ) return true;
  if ( OZmin > Zmax && Abs(OZmin - Zmax) > tolerance ) return true;

  return false;
}

//-----------------------------------------------------------------------------

bool RTCD::AABB::IsOut(const gp_Pln& P,
                       const double  tolerance) const
{
  if ( IsVoid )
    return true;

  double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
  this->Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);

  const double Gap = tolerance;

  double A,B,C,D;
  P.Coefficients(A, B ,C ,D);
  double d = A * (Xmin-Gap) + B * (Ymin-Gap) + C * (Zmin-Gap) + D;
  int plus = d > 0;

  if (plus != ((A*(Xmin-Gap) + B*(Ymin-Gap) + C*(Zmax+Gap) + D) > 0))
    return false;
  if (plus != ((A*(Xmin-Gap) + B*(Ymax+Gap) + C*(Zmin-Gap) + D) > 0))
    return false;
  if (plus != ((A*(Xmin-Gap) + B*(Ymax+Gap) + C*(Zmax+Gap) + D) > 0))
    return false;
  if (plus != ((A*(Xmax+Gap) + B*(Ymin-Gap) + C*(Zmin-Gap) + D) > 0))
    return false;
  if (plus != ((A*(Xmax+Gap) + B*(Ymin-Gap) + C*(Zmax+Gap) + D) > 0))
    return false;
  if (plus != ((A*(Xmax+Gap) + B*(Ymax+Gap) + C*(Zmin-Gap) + D) > 0))
    return false;
  if (plus != ((A*(Xmax+Gap) + B*(Ymax+Gap) + C*(Zmax+Gap) + D) > 0))
    return false;

  return true;
}

//-----------------------------------------------------------------------------

double RTCD::Dot(const Vector& V1, const Vector& V2)
{
  return V1.x*V2.x + V1.y*V2.y + V1.z*V2.z;
}

//-----------------------------------------------------------------------------

void RTCD::ClosestPtPointOBB(const Point& p, const OBB& b, Point& q)
{
  Vector d(p - b.c);

  // Start result at center of box; make steps from there.
  q = b.c;

  // For each OBB axis...
  for ( int i = 0; i < 3; ++i )
  {
    // ... project `d` onto that axis to get the distance
    // along the axis of `d` from the box center.
    double dist = Dot(d, b.u[i]);

    // If distance farther than the box extents, clamp to the box.
    if ( dist > b.e[i] ) dist = b.e[i];
    if ( dist < -b.e[i] ) dist = -b.e[i];

    // Step that distance along the axis to get world coordinate.
    q += b.u[i]*dist;
  }
}

//-----------------------------------------------------------------------------

bool RTCD::TestSphereOBB(const Sphere& s, const OBB& b, Point& p)
{
  // Find point `p` on OBB closest to sphere center.
  ClosestPtPointOBB(s.c, b, p);

  // Sphere and OBB intersect if the (squared) distance from
  // sphere center to point `p` is less than the (squared)
  // sphere radius.
  Vector v(p - s.c);
  return Dot(v, v) <= s.r * s.r;
}

//-----------------------------------------------------------------------------

void RTCD::ClosestPtPointSegment(const Point& c, const Point& a, const Point& b, double& t, Point& d)
{
  Vector ab = b - a;
  Vector ac = c - a;

  // Project `c` onto `ab`, but deferring divide by `Dot(ab, ab)`.
  t = Dot(ac, ab);
  //
  if ( t <= 0.0f )
  {
    // `c` projects outside the `[a,b]` interval, on the `a` side; clamp to `a`.
    t = 0.0f;
    d = a;
  }
  else
  {
    double denom = Dot(ab, ab); // Always nonnegative since `denom = ||ab||^2`.
    if ( t >= denom )
    {
      // `c` projects outside the `[a,b]` interval, on the `b` side; clamp to `b`.
      t = 1.0f;
      d = b;
    }
    else
    {
      // `c` projects inside the `[a,b]` interval; must do deferred divide now.
      t = t / denom;
      d = a + ab*t;
    }
  }
}

//-----------------------------------------------------------------------------

double RTCD::SqDistPointSegment(const Point& a, const Point& b, const Point& c)
{
  Vector ab = b - a, ac = c - a, bc = c - b;
  double e = Dot(ac, ab);

  // Handle cases where `c` projects outside `ab`.
  if ( e < 0. ) return Dot(ac, ac);
  double f = Dot(ab, ab);
  if ( e > f ) return Dot(bc, bc);

  // Handle cases where `c` projects onto `ab`.
  return Dot(ac, ac) - e*e/f;
}

//-----------------------------------------------------------------------------

int RTCD::IntersectRayAABB(Point p, Vector d, AABB a, double &tmin, double &tmax)
{
  const double EPSILON = RealEpsilon();

  tmin = -DBL_MAX; // set to -DBL_MAX to get first hit on line.
  tmax = DBL_MAX; // set to max distance ray can travel (for segment).

  // For all three slabs
  for ( int i = 0; i < 3; ++i )
  {
    if ( Abs(d[i]) < EPSILON )
    {
      // Ray is parallel to slab. No hit if origin not within slab.
      if ( p[i] < a.min[i] || p[i] > a.max[i] )
        return 0;
    }
    else
    {
      // Compute intersection `t` value of ray with near and far plane of slab.
      double ood = 1.0f / d[i];
      double t1 = (a.min[i] - p[i]) * ood;
      double t2 = (a.max[i] - p[i]) * ood;

      // Make `t1` be intersection with near plane, `t2` with far plane.
      if ( t1 > t2 )
        std::swap(t1, t2);

      // Compute the intersection of slab intersection intervals.
      tmin = Max(tmin, t1);
      tmax = Min(tmax, t2);

      // Exit with no collision as soon as slab intersection becomes empty.
      if ( tmin > tmax )
        return 0;
    }
  }

  // Ray intersects all 3 slabs. Return `tmin` and `tmax`.
  return 1;
}
