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

#ifndef asiAlgo_BgrNode_HeaderFile
#define asiAlgo_BgrNode_HeaderFile

// asiAlgo includes
#include <asiAlgo_DiscrLocation.h>
#include <asiAlgo_DiscrMetrics.h>

// OpenCascade includes
#include <gp_XYZ.hxx>
#include <gp_Pnt2d.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

const double BgrDummyReal = 9.87654321e+33;

//! A single node in the background mesh associated with a discrete face.
class BgrNode : public gp_Pnt2d, public Metrics
{
public:

  //! Empty constructor.
  inline BgrNode();

  //! Constructor.
  inline BgrNode(const gp_XYZ& point, const gp_XY& uv);

  //! Copy constructor.
  inline BgrNode(const BgrNode& other);

  //! Assignment operator.
  inline BgrNode& operator=(const BgrNode& other);

public:

  void SetPoint(const gp_XYZ& point)
  {
    m_coords = point;
  }

  const gp_XYZ& Point() const
  {
    return m_coords;
  }

  gp_XYZ& ChangePoint()
  {
    return m_coords;
  }

  Location GetLocation() const
  {
    return (Location)(m_location & 3);
  }

  void SetLocation(const Location loc)
  {
    m_location = loc;
  }

  bool IsNearBoundary() const
  {
    return bool(m_bNearBoundary);
  }

  inline void SetIsNearBoundary(const bool isNearBnd) const;

  bool IsExtra() const
  {
    return bool(m_bIsExtra);
  }

  inline void SetIsExtra (const bool isExtra)
  {
    m_bIsExtra = isExtra ? 1 : 0;
  }

private:

  gp_XYZ       m_coords;
  Location     m_location      : 2; // location w.r.t. the boundaries
  unsigned int m_bNearBoundary : 1;
  unsigned int m_bIsExtra      : 1;
};

//-----------------------------------------------------------------------------

inline BgrNode::BgrNode()
: gp_Pnt2d        (BgrDummyReal, BgrDummyReal),
  Metrics         (),
  m_coords        (BgrDummyReal, BgrDummyReal, BgrDummyReal),
  m_location      (Location_ON),
  m_bNearBoundary (0),
  m_bIsExtra      (0)
{}

//-----------------------------------------------------------------------------

inline BgrNode::BgrNode(const gp_XYZ& point,
                        const gp_XY&  uv)
: gp_Pnt2d        (uv),
  Metrics         (),
  m_coords        (point),
  m_location      (Location_ON),
  m_bNearBoundary (0),
  m_bIsExtra      (0)
{
}

//-----------------------------------------------------------------------------

inline BgrNode::BgrNode(const BgrNode& other)
: gp_Pnt2d        (other),
  Metrics         (other),
  m_coords        (other.Point()),
  m_location      (other.m_location),
  m_bNearBoundary (other.m_bNearBoundary),
  m_bIsExtra      (other.m_bIsExtra)
{
}

//-----------------------------------------------------------------------------

inline BgrNode& BgrNode::operator=(const BgrNode& other)
{
  gp_Pnt2d::operator = (other);
  Metrics::operator  = (other);
  m_coords           = other.Point();
  m_location         = other.m_location;
  m_bNearBoundary    = other.m_bNearBoundary;
  m_bIsExtra         = other.m_bIsExtra;
  return *this;
}

//-----------------------------------------------------------------------------

inline void BgrNode::SetIsNearBoundary(const bool isNearBoundary) const
{
  ((BgrNode *) this)->m_bNearBoundary = isNearBoundary ? 1:0;
}

}
}

#endif
