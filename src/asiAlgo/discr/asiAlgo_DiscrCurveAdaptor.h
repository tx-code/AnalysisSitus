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

#ifndef asiAlgo_DiscrCurveAdaptor_HeaderFile
#define asiAlgo_DiscrCurveAdaptor_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <Adaptor2d_Curve2d.hxx>
#include <Adaptor3d_Curve.hxx>
#include <NCollection_List.hxx>
#include <LProp3d_SLProps.hxx>
#include <Precision.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
namespace asiAlgo {
namespace discr {

class Params;

//! This class provides services of inquiring of properties of a point
//! on curve including local size on adjacent surfaces.
class CurveAdaptor
{
public:

  // The class for computing the local size on curve on surface (CONS).
  class CurveOnSurface
  {
  public:
    CurveOnSurface () : myAdaptor (2, Precision::Confusion()) {}

    void Init (const Handle(Adaptor2d_Curve2d)& theCurve2d,
               const Handle(Adaptor3d_Surface)& theSurf)
    {
      myCurve2d = theCurve2d;
      mySurface = theSurf;
      myAdaptor.SetSurface (theSurf);
    }

    //! Computes the local size at the point with the given parameter on curve 2d.
    asiAlgo_EXPORT double
      LocalSize(const double  t,
                const Params& meshParams) const;

    asiAlgo_EXPORT gp_Pnt2d
      Value2d(const double t) const;

    asiAlgo_EXPORT gp_Pnt
      ValueOnSurf(const gp_Pnt2d& uv) const;

  public:

    Handle(Adaptor2d_Curve2d) Curve2d() const
    {
      return myCurve2d;
    }

    Handle(Adaptor3d_Surface) Surface() const
    {
      return mySurface;
    }

  private:
    Handle(Adaptor2d_Curve2d) myCurve2d;
    Handle(Adaptor3d_Surface) mySurface;
    mutable LProp3d_SLProps   myAdaptor;
  };
  typedef NCollection_List<CurveOnSurface> ListOfCurveOnSurface;

  // ---------- PUBLIC METHODS ----------

  CurveAdaptor (const Adaptor3d_Curve& theCurve,
                const Params&          theMeshParams)
  : myCurve (theCurve),
    myMeshParams (theMeshParams)
  {}

  void AddCurveOnSurface(const Handle(Adaptor2d_Curve2d)& theCurve2d,
                         const Handle(Adaptor3d_Surface)& theSurf)
  {
    myCOnSList.Append(CurveOnSurface()).Init (theCurve2d, theSurf);
  }

  double FirstParameter() const
  {
    return myCurve.FirstParameter();
  }

  double LastParameter() const
  {
    return myCurve.LastParameter();
  }

  void D0(const double theParam, gp_Pnt& thePnt) const
  {
    myCurve.D0(theParam, thePnt);
  }

  asiAlgo_EXPORT double LocalSize (const double theParam) const;
  // Computes the local size at the point with the given parameter on curve,
  // averaging among all curves on surfaces

  bool IsClosed() const
  // Returns true if the curve is closed
  { return myCurve.IsClosed(); }

  const ListOfCurveOnSurface& GetListOfCurveOnSurface() const
  {return myCOnSList;}

  Adaptor3d_Curve& Adaptor() const
  {
    return (Adaptor3d_Curve&)myCurve;
  }

 private:
  // ---------- PRIVATE FIELDS ----------

  const Adaptor3d_Curve& myCurve;
  const Params&          myMeshParams;
  ListOfCurveOnSurface   myCOnSList;

  //! Protection against compiler warning
  void operator= (const CurveAdaptor&);
};

}

}

#endif
