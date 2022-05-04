//-----------------------------------------------------------------------------
// Created on: 26 April 2022
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

#ifndef asiAlgo_DiscrTessellateCurve_HeaderFile
#define asiAlgo_DiscrTessellateCurve_HeaderFile

#include <asiAlgo_DiscrParams.h>
#include <asiAlgo_DiscrCurveAdaptor.h>

#include <NCollection_Sequence.hxx>
#include <NCollection_DataMap.hxx>
#include <gp_Pnt.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

// This class contains the algorithm to discretise a 3D curve
// using parameters of discretisation asiFaceter_MeshParameters
class TessellateCurve
{
 public:
  // ---------- PUBLIC METHODS ----------

  //! Adds thePointCount points on polyline
  //! given by at least 2 points in thePoints
  //! with corresponding parameters in theParameters.
  //! The parameters must be ordered in ascending order.
  //! Inserts new points in thePoints.
  //! Inserts parameters corresponding to new points in theParameters.
  //! Returns true on success.
  asiAlgo_EXPORT static bool
    AddPointsToPolyline(Adaptor3d_Curve&              theCurve,
                        const int                     thePointCount,
                        const double                  theMinSize,
                        NCollection_Sequence<double>& theParams,
                        NCollection_Sequence<gp_XYZ>& thePoints);

  //! Approximates curve theCurve by a polyline
  //! on the curve parametric segment [theMinParameter, theMaxParameter]
  //! so that the polyline segment count is divisible by thePartCountDivisor,
  //! any segment of the polyline is not shorter than theMinSize and
  //! is not longer than theMaxSize,
  //! angle between any two adjacent segments of the polyline
  //! is not more than theMaxAngle and
  //! distance from middle of every segment of the polyline
  //! to point of the curve with average parameter
  //! between the segment end parameters is not more than theDeflection.
  //! Stores points of the polylines in thePoints.
  //! Stores parameters of the points on the curve in theParameters.
  //! The parameters are ordered in ascending order.
  //! Returns true on success.
  //! theMaxAngle is ignored if it is more than pi.
  asiAlgo_EXPORT static bool
    ApproximateCurveByPolyline(Adaptor3d_Curve&              theCurve,
                               const double                  theMinParameter,
                               const double                  theMaxParameter,
                               const double                  theMinSize,
                               const double                  theMaxSize,
                               const double                  theDeflection,
                               const double                  theMaxAngle,
                               const int                     thePartCountDivisor,
                               const int                     thePartCountReminder,
                               NCollection_Sequence<double>& theParameters,
                               NCollection_Sequence<gp_XYZ>& thePoints);

  TessellateCurve (const CurveAdaptor& theCurve,
                   const Params&       theMeshParams,
                   const bool          isSameParameter = false,
                   const double        theCurveTolerance = Precision::Confusion())
    : myCurve(theCurve), myMeshParams(theMeshParams), myDone(false),
      myIsSameParam(isSameParameter), myCurveTolerance(theCurveTolerance)
    { Perform(); }
  // Constructor initiating computation

  asiAlgo_EXPORT void Perform ();
  // Performs the process of discretisation

  bool IsDone () const {return myDone;}
  // Returns true if Perform() has been successful

  int NbPoints () const {return myPoints.Length();}
  // Returns the number of computed points

  const gp_Pnt& Point (const int aIndex) const
  { return myPoints(aIndex).Point(); }
  // Returns the point of the given index

  double Parameter (const int aIndex) const
  { return myPoints(aIndex).Parameter(); }
  // Returns the parameter on curve of the point of the given index

 public:

   //! Structure to store some projection parameters for performance gain.
   //! According to the capabilities of ShapeAnalysis_Curve tool, we can use
   //! the previous projection result in order to optimize the
   //! projection algorithm.
   struct ProjectionCache
   {
     //! Indicates whether the previous projection result is available.
     bool IsPrevDefined;

     //! Previous projection parameter.
     double PrevParameter;

     //! Default constructor.
     ProjectionCache()
       : IsPrevDefined(false),
         PrevParameter(0.)
     {
     };
   };

   //! Projects the given 3D point on surface the given pcurve is defined.
   asiAlgo_EXPORT static bool PntToCurve2d(
     const gp_Pnt&                       thePoint,
     const CurveAdaptor::CurveOnSurface& theCoS,
     const double                        theProjToler,
     ProjectionCache&                    theProjCache,
     gp_Pnt2d&                           theUVPoint,
     double&                             theCoSParam);

 protected:

  // ---------- INTERNAL TYPES ----------

  class PointOnCurve 
  {
  public:
    PointOnCurve () : myCurve(NULL), myPnt(0.,0.,0.), myPar(0.),
    myLocalSize(1e100), myIsLSComputed(false) {}
    // Empty constructor

    PointOnCurve (const CurveAdaptor& aCurve, const double aPar)
      : myCurve(&aCurve), myPar(aPar),
      myLocalSize(1e100), myIsLSComputed(false)
      { aCurve.D0 (myPar, myPnt); }
    // Constructor

    //! Constructor.
    PointOnCurve (const CurveAdaptor& theCurve,
      const double theParameter,
      const gp_XYZ& theXYZ)
    {
      myCurve = &theCurve;
      myPar = theParameter;
      myPnt = theXYZ;
      myLocalSize = 1e100;
      myIsLSComputed = false;
    }

    void Init (const CurveAdaptor& aCurve, const double aPar)
    { myCurve = &aCurve; myPar = aPar; aCurve.D0 (myPar, myPnt); }
    // Initialize me

    const gp_Pnt& Point () const { return myPnt; }
    gp_Pnt&       Point () { return myPnt; }
    // Access to 3D point

    double  Parameter () const { return myPar; }
    double& Parameter () { return myPar; }
    // Access to parameter of point on curve

    double Distance (const PointOnCurve& aOther) const
    { return myPnt.Distance(aOther.myPnt); }
    // Computes the distance to another point

    double LocalSize () const
    // Computes the local size
    {
      if (myCurve && !myIsLSComputed)
      {
        PointOnCurve* me = (PointOnCurve*) this;
        me->myLocalSize = myCurve->LocalSize (myPar);
        me->myIsLSComputed = true;
      }
      return myLocalSize;
    }

   private:
    const CurveAdaptor* myCurve;
    gp_Pnt myPnt;
    double myPar;
    double myLocalSize;
    bool myIsLSComputed;
  };

  // ---------- PROTECTED METHODS ----------

  asiAlgo_EXPORT void ComputeInsidePoint (const PointOnCurve& aPoc1,
                                          const PointOnCurve& aPoc2,
                                          const double aRelPar,
                                          PointOnCurve& aPoc) const;
  // Computes point on curve between the given two points
  // using relative parameter aRelPar

  asiAlgo_EXPORT bool IsSegmentValid
    (const PointOnCurve& aPoc1, const PointOnCurve& aPoc2,
     PointOnCurve& aPocM);
  // Checks if segment satisfies criteria of myMeshParams;
  // computes the middle point

  asiAlgo_EXPORT bool CheckInsidePoint
    (const double len, const double len1, const double len2) const;

  asiAlgo_EXPORT bool CheckDeflection2d (const PointOnCurve& thePoc1, 
                                         const PointOnCurve& thePoc2, 
                                         const PointOnCurve& thePocM);

 private:
  // ---------- PRIVATE FIELDS ----------

  typedef NCollection_Sequence<PointOnCurve> SequenceOfPointOnCurve;
  typedef NCollection_DataMap<Standard_Address, ProjectionCache> PCurveProjCache;

  //! Projection cache for current discretisation process.
  PCurveProjCache        myProjCache;
  const CurveAdaptor&    myCurve;
  Params                 myMeshParams;
  SequenceOfPointOnCurve myPoints;
  bool                   myDone;
  bool                   myIsSameParam;
  double                 myCurveTolerance;

  //! Protection against compiler warning
  void operator= (const TessellateCurve&);
};

}
}

#endif
