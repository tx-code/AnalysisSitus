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

#ifndef asiAlgo_RTCD_h
#define asiAlgo_RTCD_h

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <Bnd_Box.hxx>
#include <gp_Pln.hxx>

//! Algorithms from "Real-Time Collision Detection" by Christer Ericson.
namespace RTCD
{
  struct Point;
  struct Vector
  {
    double x, y, z;

    Vector() : x(0.), y(0.), z(0.) {}
    Vector(const double _x, const double _y, const double _z) : x(_x), y(_y), z(_z) {}
    explicit Vector(const Point& P);

    double operator[](const int i) const
    {
      return ((i == 0) ? x : ((i == 1) ? y : z));
    }

    Vector operator-(const Vector& other)
    {
      return Vector(x - other.x, y - other.y, z - other.z);
    }

    Vector operator*=(const double coeff)
    {
      x *= coeff;
      y *= coeff;
      z *= coeff;
      return *this;
    }

    Vector operator*(const double coeff) const
    {
      Vector result(*this);
      result *= coeff;
      return result;
    }

    double Modulus() const
    {
      return Sqrt(x*x + y*y + z*z);
    }
  };

  struct Point
  {
    double x, y, z;

    Point() : x(0.), y(0.), z(0.) {}
    Point(const double _x,
          const double _y,
          const double _z) : x(_x), y(_y), z(_z) {}

    asiAlgo_EXPORT Point(const Vector& V);

    double operator[](const int i)
    {
      switch ( i )
      {
        case 0: return x;
        case 1: return y;
        case 2: return z;
      }
      return DBL_MAX;
    }

    Vector operator+(const Point& P) const
    {
      Vector result;
      result.x = x + P.x;
      result.y = y + P.y;
      result.z = z + P.z;
      return result;
    }

    Vector operator-(const Point& P) const
    {
      Vector result;
      result.x = x - P.x;
      result.y = y - P.y;
      result.z = z - P.z;
      return result;
    }

    asiAlgo_EXPORT Point operator+=(const Vector& V);
    asiAlgo_EXPORT Point operator-=(const Vector& V);
    asiAlgo_EXPORT Point operator+ (const Vector& V);
    asiAlgo_EXPORT Point operator- (const Vector& V);
  };

  struct Sphere
  {
    Point  c; //!< Sphere center.
    double r; //!< Sphere radius.
  };

  struct AABB
  {
    bool  IsVoid;
    Point min;
    Point max;

    AABB() : IsVoid(true) {} //!< Default ctor.

    asiAlgo_EXPORT
      AABB(const Bnd_Box& bbox); //!< Ctor from OpenCascade type.

    asiAlgo_EXPORT void
      Get(double& xmin, double& ymin, double& zmin,
          double& xmax, double& ymax, double& zmax) const;

    asiAlgo_EXPORT void
      Add(const Point& P);

    asiAlgo_EXPORT void
      Add(const double x, const double y, const double z);

    asiAlgo_EXPORT bool
      IsOut(const Point& P,
            const double tolerance) const;

    asiAlgo_EXPORT bool
      IsOut(const AABB&  other,
            const double tolerance) const;

    asiAlgo_EXPORT bool
      IsOut(const gp_Pln& P,
            const double  tolerance) const;
  };

  struct OBB
  {
    Point  c;    //!< OBB center point.
    Vector u[3]; //!< Local x-, y-, and z-axes.
    Vector e;    //!< Positive halfwidth extents of OBB along each axis.
  };

  //! Computes dot product of two vectors.
  asiAlgo_EXPORT double
    Dot(const Vector& V1, const Vector& V2);

  //! Given point `p`, returns point `q` on (or in) OBB `b`,
  //! closest to `p`.
  asiAlgo_EXPORT void
    ClosestPtPointOBB(const Point& p, const OBB& b, Point& q);

  //! Returns true if the sphere `s` intersects the OBB `b`,
  //! false otherwise. The point `p` on the OBB closest to
  //! the sphere center is also returned.
  asiAlgo_EXPORT bool
    TestSphereOBB(const Sphere& s, const OBB& b, Point& p);

  //! Given segment `ab` and point `c`, computes closest point
  //! `d` on `ab`. Also returns `t` for the parametric position
  //! of `d`, `d(t) = a + t*(b - a)`.
  asiAlgo_EXPORT void
    ClosestPtPointSegment(const Point& c, const Point& a, const Point& b, double& t, Point& d);

  //! Returns the squared distance between point `c` and segment `ab`.
  asiAlgo_EXPORT double
    SqDistPointSegment(const Point& a, const Point& b, const Point& c);

  //! Intersects ray `R(t) = p + t*d` against AABB `a`.
  //! When intersecting, returns intersection distance `tmin` and point `q`
  //! of intersection.
  //!
  //! \sa sec. 5.3.3 in RTCD.
  asiAlgo_EXPORT int
    IntersectRayAABB(Point p, Vector d, AABB a, double &tmin, double &tmax);
}

#endif
