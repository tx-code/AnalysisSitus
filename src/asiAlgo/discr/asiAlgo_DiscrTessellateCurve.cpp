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

#include <asiAlgo_DiscrTessellateCurve.h>

#include <Adaptor2d_Curve2d.hxx>
#include <Adaptor3d_Surface.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
#include <NCollection_List.hxx>
#include <Precision.hxx>
#include <Standard_Assert.hxx>
#include <Standard_ProgramError.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <ElCLib.hxx>

#ifdef DEBUG_ApproximateCurveByPolyline_Shape
  #include <DBRep.hxx>
  #include <BRep_Builder.hxx>
  #include <BRepBuilderAPI_MakeEdge.hxx>
  #ifdef _WIN32
    #pragma comment (lib, "TKDraw")
  #endif
#endif

const double GOLD1 = 0.381966011;
const double GOLD2 = 0.618033989;

using namespace asiAlgo::discr;

//==============================================================================
//function : AddPointsToPolyline
//purpose  : 
//==============================================================================
bool TessellateCurve::AddPointsToPolyline (Adaptor3d_Curve& theCurve,
                                                           const int               thePointCount,
                                                           const double theMinSize,
                                                           NCollection_Sequence<double>& theParameters,
                                                           NCollection_Sequence<gp_XYZ>&        thePoints)
{
  bool aResult = true;

  NCollection_Sequence<double> aSqDists;

  int aPointCount = thePoints.Size();
  for (int aJ = 1; aJ < aPointCount; ++aJ)
  {
    double aSqDist = (thePoints (aJ + 1) - thePoints (aJ)).SquareModulus();
    aSqDists.Append (aSqDist);
  }

  double aPrecisionUpCoef = 1.1;

  double aMinSize = theMinSize * aPrecisionUpCoef;
  if (aMinSize < Precision::Confusion() * aPrecisionUpCoef)
  {
    aMinSize = Precision::Confusion() * aPrecisionUpCoef;
  }

  double aPConfusion = Precision::PConfusion() * aPrecisionUpCoef;
  double aSqMinSize = aMinSize * aMinSize;

  for (int aK = 0; aK < thePointCount; ++aK)
  {
    double    aMaxSqDist = 4 * aSqMinSize;
    double    aMaxSqDist2 = -RealLast();
    int aSplitInd = 0;
    int aSplitInd2 = 0;
    for (int aJ = 1; aJ < aPointCount; ++aJ)
    {
      double aSqDist = aSqDists (aJ);
      if (aMaxSqDist < aSqDist)
      {
        double aParam1 = theParameters (aJ);
        double aParam2 = theParameters (aJ + 1);
        double aParam  = 0.5 * (aParam1 + aParam2);
        if (aPConfusion <= aParam - aParam1 && aPConfusion <= aParam2 - aParam)
        {
          aMaxSqDist  = aSqDist;
          aSplitInd   = aJ;
        }
      }
      if (aMaxSqDist2 < aSqDist)
      {
        aMaxSqDist2 = aSqDist;
        aSplitInd2 = aJ;
      }
    }

    if (aSplitInd == 0)
    {
      aSplitInd = aSplitInd2;
      aResult = false;
    }
    double aParam =
        0.5 * (theParameters (aSplitInd) + theParameters (aSplitInd + 1));

    gp_XYZ aPoint1 = thePoints(aSplitInd);
    gp_XYZ aPoint2 = thePoints(aSplitInd + 1);
    gp_XYZ aPoint  = theCurve.Value(aParam).XYZ();
    aSqDists.InsertAfter (aSplitInd, (aPoint2 - aPoint).SquareModulus());
    aSqDists (aSplitInd) = (aPoint - aPoint1).SquareModulus();
    theParameters.InsertAfter (aSplitInd, aParam);
    thePoints.InsertAfter (aSplitInd, aPoint);
    ++aPointCount;
  }

  return aResult;
}

//==============================================================================
//function : ApproximateCurveByPolyline
//purpose  : 
//==============================================================================
bool TessellateCurve::ApproximateCurveByPolyline (Adaptor3d_Curve& theCurve,
                                             const double                  theMinParameter,
                                             const double                  theMaxParameter,
                                             const double                  theMinSize,
                                             const double                  theMaxSize,
                                             const double                  theDeflection,
                                             const double                  theMaxAngle,
                                             const int               thePartCountDivisor,
                                             const int               thePartCountReminder,
                                             NCollection_Sequence<double>& theParameters,
                                             NCollection_Sequence<gp_XYZ>&        thePoints)
{
  double aMinSize = theMinSize;
  if (aMinSize < Precision::Confusion())
  {
    aMinSize = Precision::Confusion();
  }

  double aMaxSize = theMaxSize;
  if (aMaxSize < aMinSize)
  {
    aMaxSize = aMinSize;
  }

  double aDeflection = theDeflection;
  double aMinDeflection = 1e-5; // minimal allowed deflection
  if (aDeflection < aMinDeflection)
  {
    aDeflection = aMinDeflection;
  }

  double aMaxAngle = theMaxAngle;
  if (aMaxAngle < Precision::Angular())
  {
    aMaxAngle = Precision::Angular();
  }

  int aPartCountDivisor = (0 < thePartCountDivisor) ? thePartCountDivisor : 1;

  int aPartCountReminder = thePartCountReminder % aPartCountDivisor;
  aPartCountReminder = (0 <= aPartCountReminder) ?
    aPartCountReminder : aPartCountReminder + aPartCountReminder;

  double    aCurveLength = GCPnts_AbscissaPoint::Length (theCurve, theMinParameter, theMaxParameter);
  int aPartCount   = (int )Ceiling(aCurveLength / aMaxSize);

  int anAdditionalPartCount =
    aPartCountReminder - aPartCount % aPartCountDivisor;
  if (anAdditionalPartCount < 0)
  {
    aPartCount += anAdditionalPartCount + aPartCountDivisor;
  }
  else if (0 < anAdditionalPartCount)
  {
    aPartCount += anAdditionalPartCount;
  }

  GCPnts_UniformAbscissa aSplitter(
    theCurve, aPartCount + 1, theMinParameter, theMaxParameter, Precision::Confusion());
  int aPointCount = aSplitter.IsDone() ? aSplitter.NbPoints() : 2;
  bool aResult = true;

  theParameters.Append (theMinParameter);
  thePoints.Append (theCurve.Value (theMinParameter).XYZ());
  for (int aJ = 2; aJ < aPointCount; ++aJ)
  {
    double aParam = aSplitter.Parameter (aJ);
    theParameters.Append (aParam);
    thePoints.Append (theCurve.Value (aParam).XYZ());
  }
  theParameters.Append (theMaxParameter);
  thePoints.Append (theCurve.Value (theMaxParameter).XYZ());

  const double aSqMinSize = aMinSize * aMinSize;

  for (;;)
  {
    double aSqMaxSize2 = 0;
    for (int aJ = 1; aJ < aPointCount; ++aJ)
    {
      double aSqSize =
        (thePoints (aJ + 1) - thePoints (aJ)).SquareModulus();
      if (aSqSize > aSqMaxSize2)
      {
        aSqMaxSize2 = aSqSize;
      }
    }

    if (aSqMaxSize2 <= aMaxSize * aMaxSize * 1.2)
    {
      break;
    }

    aSqMaxSize2 /= 1.1;
    bool isDone = true;

    for (int aJ = aPointCount - 1; aJ >= 1; --aJ)
    {
      gp_XYZ aPoints[] = {thePoints (aJ), thePoints (aJ + 1)};
      double aSqSize = (aPoints[1] - aPoints[0]).SquareModulus();
      if (aSqSize > aSqMaxSize2)
      {
        double aParams[] = {theParameters (aJ), theParameters (aJ + 1)};
        if (aParams[1] - aParams[0] < 2 * Precision::PConfusion())
        {
          aResult = false;
        }
        else
        {
          double aParam = 0.5 * (aParams[0] + aParams[1]);
          gp_XYZ aPoint = theCurve.Value (aParam).XYZ();

          if ((aPoint - aPoints[0]).SquareModulus() < aSqMinSize ||
              (aPoints[1] - aPoint).SquareModulus() < aSqMinSize)
          {
            aResult = false;
          }
          else
          {
            theParameters.InsertAfter (aJ, aParam);
            thePoints    .InsertAfter (aJ, aPoint);
            ++aPointCount;
            isDone = false;
          }
        }
      }
    }

    if (isDone)
    {
      break;
    }
  }

  for (;;)
  {
    double aSqMaxDeflection = 0;
    for (int aJ = 1; aJ < aPointCount; ++aJ)
    {
      double aParams[] = {theParameters (aJ), theParameters (aJ + 1)};
      double aParam = 0.5 * (aParams[0] + aParams[1]);
      gp_XYZ aPoint = theCurve.Value (aParam).XYZ();
      double aSqDeflection =
        (aPoint - 0.5 * (thePoints (aJ) + thePoints (aJ + 1))).SquareModulus();
      if (aSqDeflection > aSqMaxDeflection)
      {
        aSqMaxDeflection = aSqDeflection;
      }
    }

    if (aSqMaxDeflection <= aDeflection * aDeflection * 1.2)
    {
      break;
    }

    aSqMaxDeflection /= 1.1;
    bool isDone = true;

    for (int aJ = 1; aJ < aPointCount; ++aJ)
    {
      double aParams[] = {theParameters (aJ), theParameters (aJ + 1)};
      gp_XYZ aPoints[] = {thePoints (aJ), thePoints (aJ + 1)};
      double aParam = 0.5 * (aParams[0] + aParams[1]);
      gp_XYZ aPoint = theCurve.Value (aParam).XYZ();

      if ((aPoint - 0.5 * (aPoints[1] + aPoints[0])).SquareModulus() >
          aSqMaxDeflection)
      {
        if (aParams[1] - aParams[0] < 2 * Precision::PConfusion() ||
          (aPoint - aPoints[0]).SquareModulus() < aSqMinSize ||
          (aPoints[1] - aPoint).SquareModulus() < aSqMinSize)
        {
          aResult = false;
        }
        else
        {
          theParameters.InsertAfter (aJ, aParam);
          thePoints    .InsertAfter (aJ, aPoint);
          ++aPointCount;
          isDone = false;
        }
      }
    }

    if (isDone)
    {
      break;
    }
  }

  if (theMaxAngle < M_PI)
  {
    for (;;)
    {
      double aMaxAngle2 = 0;
      for (int aJ = 2; aJ < aPointCount; ++aJ)
      {
        gp_XYZ aPoints[] = {thePoints(aJ - 1), thePoints(aJ), thePoints(aJ + 1)};
        gp_XYZ aVec1 = aPoints[1] - aPoints[0];
        gp_XYZ aVec2 = aPoints[2] - aPoints[1];
        double anAngle = gp_Vec(aVec1).Angle(aVec2);
        if (anAngle > aMaxAngle2)
        {
          aMaxAngle2 = anAngle;
        }
      }

      if (aMaxAngle2 <= aMaxAngle * 1.1)
      {
        break;
      }

      aMaxAngle2 /= 1.05;
      bool isDone = true;

      bool wasDivided = false;
      for (int aJ = aPointCount - 1; aJ > 1; --aJ)
      {
        bool wasDivided2 = wasDivided;
        wasDivided = false;
        gp_XYZ aPoints[] = {thePoints(aJ - 1), thePoints(aJ), thePoints(aJ + 1)};
        gp_XYZ aVec1 = aPoints[1] - aPoints[0];
        gp_XYZ aVec2 = aPoints[2] - aPoints[1];

        if (gp_Vec(aVec1).Angle(aVec2) <= aMaxAngle2)
        {
          continue;
        }

        double aParams[] =
          {theParameters(aJ - 1), theParameters(aJ), theParameters(aJ + 1)};
        for (int aK = wasDivided2 ? 0 : 1; aK >= 0; --aK)
        {
          double aParam2 = 0.5 * (aParams[aK] + aParams[aK + 1]);
          gp_XYZ aPoint2 = theCurve.Value (aParam2).XYZ();
          if (aParams[aK + 1] - aParams[aK] < 2 * Precision::PConfusion() ||
              (aPoint2 - aPoints[aK]).SquareModulus() < aSqMinSize ||
              (aPoints[aK + 1] - aPoint2).SquareModulus() < aSqMinSize)
          {
            aResult = false;
          }
          else
          {
            theParameters.InsertAfter (aJ - 1 + aK, aParam2);
            thePoints.InsertAfter (aJ - 1 + aK, aPoint2);
            ++aPointCount;
            wasDivided = (aK == 0);
            isDone = false;
          }
        }
      }

      if (isDone)
      {
        break;
      }
    }
  }

  anAdditionalPartCount =
    aPartCountReminder - (theParameters.Size() - 1) % aPartCountDivisor;
  if (anAdditionalPartCount != 0)
  {
    if (anAdditionalPartCount < 0)
    {
      anAdditionalPartCount += aPartCountDivisor;
    }
    aResult = AddPointsToPolyline (theCurve, anAdditionalPartCount,
      theMinSize, theParameters, thePoints) && aResult;
  }

//#define DEBUG_ApproximateCurveByPolyline
#ifdef DEBUG_ApproximateCurveByPolyline
  {
    static int aWireInd = 0;
    ++aWireInd;
    int aPointCount = thePoints.Size();
    std::cout << "\n";
    for (int aJ = 1; aJ <= aPointCount; ++aJ)
    {
      gp_XYZ aP = thePoints(aJ);
      std::cout << "vertex v" << aWireInd << "_" << aJ << " " << aP.X() << " " << aP.Y() << " " << aP.Z() << "\n";
    }
    for (int aJ = 1; aJ < aPointCount; ++aJ)
    {
      std::cout << "edge e" << aWireInd << "_" << aJ << " " << "v" << aWireInd << "_"  << aJ << " v" << aWireInd << "_"  << aJ + 1 << "\n";
    }
  }
#endif

//#define DEBUG_ApproximateCurveByPolyline_Shape
#ifdef DEBUG_ApproximateCurveByPolyline_Shape
  {
    char aShapeName[20] = "Discr";
    Standard_CString aShapeName2 = aShapeName;
    TopoDS_Shape aCompound = DBRep::Get(aShapeName2);
    int aPntCount = thePoints.Size();
    BRep_Builder aBuilder;
    TopoDS_Vertex aV1;
    aBuilder.MakeVertex(aV1, thePoints(1), Precision::Confusion());
    aBuilder.Add(aCompound, aV1);
    for (int aJ = 2; aJ <= aPntCount; ++aJ)
    {
      TopoDS_Vertex aV2;
      aBuilder.MakeVertex(aV2, thePoints(aJ), Precision::Confusion());
      aBuilder.Add(aCompound, aV2);
      aBuilder.Add(aCompound, BRepBuilderAPI_MakeEdge(aV1, aV2));
      aV1 = aV2;
    }
    DBRep::Set(aShapeName, aCompound);
  }
#endif

  return aResult;
}

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================

void TessellateCurve::Perform ()
{
  typedef NCollection_List<PointOnCurve> StackOfPointOnCurve;

  if (myDone) return;
  myPoints.Clear();
  myProjCache.Clear();

  double fpar = myCurve.FirstParameter();
  double lpar = myCurve.LastParameter();

  {
    // take first point
    PointOnCurve poc (myCurve,fpar);
    myPoints.Append (poc);

    // create stack and push last point
    StackOfPointOnCurve stk;
    poc.Init (myCurve,lpar);
    stk.Append (poc);

    if (myCurve.IsClosed()) {
      // ensure 3 intermediate points on a closed curve
      const PointOnCurve& poc2 = stk.First();
      PointOnCurve pocM1, pocM2, pocM3;
      IsSegmentValid (myPoints.Last(), poc2, pocM2);
      IsSegmentValid (myPoints.Last(), pocM2, pocM1);
      IsSegmentValid (pocM2, poc2, pocM3);
      stk.Prepend (pocM3);
      stk.Prepend (pocM2);
      stk.Prepend (pocM1);
    }

    while (!stk.IsEmpty()) {
      const PointOnCurve& poc2 = stk.First();
      PointOnCurve pocM;
      // check segment between last added point and top point
      if (IsSegmentValid (myPoints.Last(), poc2, pocM)) {
        // take top point
        myPoints.Append(poc2);
        stk.RemoveFirst();
      }
      else {
        // divide segment and push middle point
        stk.Prepend (pocM);
      }
    }
  }

  myDone = true;
}

//=======================================================================
//function : ComputeInsidePoint
//purpose  : 
//=======================================================================

void TessellateCurve::ComputeInsidePoint (const PointOnCurve& aPoc1,
                                             const PointOnCurve& aPoc2,
                                             const double aRelPar,
                                             PointOnCurve& aPoc) const
{
  double par = (1.-aRelPar)*aPoc1.Parameter() + aRelPar*aPoc2.Parameter();
  aPoc.Init (myCurve,par);
}

//=======================================================================
//function : RealPar
//purpose  : Recompute parameter for the case when parameter of middle point
//           is not equal to 0.5
//=======================================================================

inline double RealPar (double par, double mid)
{
  return par*mid / (par*mid + (1.-par)*(1.-mid));
}

//=======================================================================
//function : IsSegmentValid
//purpose  : 
//=======================================================================

bool TessellateCurve::IsSegmentValid
  (const PointOnCurve& aPoc1, const PointOnCurve& aPoc2,
   PointOnCurve& aPocM)
{
  // compute middle point
  double par1=0, par2=1, eps=1e-3, midpar, len1, len2;
  do {
    midpar = (par1 + par2) * 0.5;
    ComputeInsidePoint (aPoc1, aPoc2, midpar, aPocM);
    len1 = aPoc1.Distance(aPocM);
    len2 = aPoc2.Distance(aPocM);
    double relpar = len2 / (len1 + len2);
    if (Abs(relpar-0.5) < 0.1)
      break;
    // use half-division method to treat a badly parameterized curve
    if (relpar > 0.5)
      par1 = midpar;
    else
      par2 = midpar;
  } while (par2-par1 > eps);

  // check for min size
  if (len1 < myMeshParams.MinElemSize() || len2 < myMeshParams.MinElemSize())
    return true;

  // check for max size,
  // in addition, verify distances to the middle point to avoid treatment as a line of
  // highly-bent curves, which boundaries are too close (like almost closed circles)
  double len = aPoc1.Distance(aPoc2);
  if (myMeshParams.IsMaxElemSize() && (len > myMeshParams.MaxElemSize() ||
      len1 > myMeshParams.MaxElemSize() || len2 > myMeshParams.MaxElemSize()))
    return false;

  // check if aPoc1 and aPoc2 are the ends of a closed curve
  if (len < myMeshParams.MinElemSize())
    return false;

  if (myMeshParams.IsDeflection()) {
    if(!CheckDeflection2d (aPoc1, aPoc2, aPocM))
      return false;
  }

  if (myMeshParams.IsDeviationAngle() || myMeshParams.IsDeflection()) {
    // check local size at ends
    //double aMinLS, aMaxLS, aLS1, aLS2;
    //aLS1 = aPoc1.LocalSize();
    //aLS2 = aPoc2.LocalSize();
    //if (aLS1 > aLS2) {
    //  aMinLS = aLS2;
    //  aMaxLS = aLS1;
    //}
    //else {
    //  aMinLS = aLS1;
    //  aMaxLS = aLS2;
    //}
    //double aLS = (aMaxLS < 1e50) ? (aMinLS+aMaxLS)/2 : aMinLS;
    //if (aLS < len)
    //  return false;

    // check inside points
    if (!CheckInsidePoint (len, len1, len2))
      return false;

    PointOnCurve pocm1;
    ComputeInsidePoint (aPoc1, aPoc2, RealPar(GOLD1,midpar), pocm1);
    len1 = aPoc1.Distance(pocm1);
    len2 = aPoc2.Distance(pocm1);
    if (!CheckInsidePoint (len, len1, len2))
      return false;
    PointOnCurve pocm2;
    ComputeInsidePoint (aPoc1, aPoc2, RealPar(GOLD2,midpar), pocm2);
    len1 = aPoc1.Distance(pocm2);
    len2 = aPoc2.Distance(pocm2);
    if (!CheckInsidePoint (len, len1, len2))
      return false;
  }

  if (myMeshParams.IsAspectRatio())
  {
    double anAvLocSize = myCurve.LocalSize(aPoc1.Parameter()) +
      myCurve.LocalSize(aPoc2.Parameter()) +
      myCurve.LocalSize(aPocM.Parameter());
    anAvLocSize /= 3.;

    if (anAvLocSize < len / myMeshParams.AspectRatio() && anAvLocSize < len1)
    {
      return false;
    }
  }

  return true;
}

//=======================================================================
//function : CheckInsidePoint
//purpose  : 
//=======================================================================

bool TessellateCurve::CheckInsidePoint
  (const double len, const double len1, const double len2) const
{
  // check for angular criterion
  if (myMeshParams.IsDeviationAngle()) {
    // - check angle between vectors (aPoc1,poc) and (poc,aPoc2)
    double cosA = - (len1*len1 + len2*len2 - len*len) / (2 * len1 * len2);
    if (cosA < myMeshParams.CosAngle())
      return false;
  }

  // check for deflection
  if (myMeshParams.IsDeflection()) {
    // we have triangle (aPoc1,poc,aPoc2)
    // height of poc over segment (aPoc1,aPoc2) is equal to 2*s/len,
    // where s is the square of triangle
    double p = (len + len1 + len2) * 0.5;
    double s = sqrt (Abs (p * (p-len) * (p-len1) * (p-len2)));
    double h = 2. * s / len;
    if (h > myMeshParams.Deflection())
      return false;
  }

  return true;
}
//=======================================================================
//function : CheckDeflection2d
//purpose  : 
//=======================================================================
bool TessellateCurve::CheckDeflection2d(const PointOnCurve& thePoc1, 
                                   const PointOnCurve& thePoc2, 
                                   const PointOnCurve& thePocM)
{
  bool aRes = true;
  const NCollection_List<CurveAdaptor::CurveOnSurface>& aLOfCOnS =
    myCurve.GetListOfCurveOnSurface();
  if(aLOfCOnS.IsEmpty())
    return aRes;

  double aPar1 = thePoc1.Parameter(), aPar2 = thePoc2.Parameter(),
    aParM = thePocM.Parameter();

  const double aD = Max(myMeshParams.IsDeflection() ?
                               myMeshParams.Deflection() :
                               Precision::Confusion(), myCurveTolerance);

  NCollection_List<CurveAdaptor::CurveOnSurface>::Iterator aCIter(aLOfCOnS);
  for(; aCIter.More(); aCIter.Next())
  {
    const CurveAdaptor::CurveOnSurface& aCOnS = aCIter.Value();
    double d;
    gp_Pnt2d aP2d1, aP2d2;

    if ( myIsSameParam )
    {
      aP2d1 = aCOnS.Value2d(aPar1);
      aP2d2 = aCOnS.Value2d(aPar2);
      gp_Pnt2d aP2dM = aCOnS.Value2d(aParM);
      gp_Pnt aPM = aCOnS.ValueOnSurf(aP2dM);
      d = aD + aPM.Distance(thePocM.Point()); //taking in account edge tolerance
      d *= d;
    }
    else { // Non-SameParameter case
      gp_Pnt aPnt1 = thePoc1.Point(), aPnt2 = thePoc2.Point();
      double aPnt1Param, aPnt2Param;

      const Standard_Address aPCOnS = (Standard_Address)(&aCOnS);
      if (!myProjCache.IsBound(aPCOnS))
        myProjCache.Bind(aPCOnS, ProjectionCache());
      ProjectionCache& aProjCache = myProjCache.ChangeFind(aPCOnS);
      
      if (!this->PntToCurve2d(aPnt1, aCOnS, aD, aProjCache, aP2d1, aPnt1Param))
        return true; // Let the segment be Ok rather than allow 
                              // the deflection check fail recursively

      if (!this->PntToCurve2d(aPnt2, aCOnS, aD, aProjCache, aP2d2, aPnt2Param))
        return true;

      d = Square(aD);
    }

    gp_Pnt aP = aCOnS.ValueOnSurf(gp_Pnt2d(0.5 * (aP2d1.XY() + aP2d2.XY())));

    if(Precision::IsInfinite(aP.X()) || Precision::IsInfinite(aP.Y()) ||
       Precision::IsInfinite(aP.Z()))
    {
      return false;
    }
    
    if (aP.SquareDistance(thePocM.Point()) > d)
      return false;
  }
  return aRes;
}

//=======================================================================
//function : PntToCurve2d
//purpose  : 
//=======================================================================
bool TessellateCurve::PntToCurve2d(
  const gp_Pnt&                               thePoint,
  const CurveAdaptor::CurveOnSurface& theCoS,
  const double                         theProjToler,
  ProjectionCache&        theProjCache,
  gp_Pnt2d&                                   theUVPoint,
  double&                              theCoSParam)
{
  if (!theCoS.Surface()->IsKind(STANDARD_TYPE(BRepAdaptor_Surface)))
    throw Standard_ProgramError("Unexpected type of surface");

  if (!theCoS.Curve2d()->IsKind(STANDARD_TYPE(BRepAdaptor_Curve2d)))
    throw Standard_ProgramError("Unexpected type of curve 2D");

  // Get surface from Geom
  Handle(BRepAdaptor_Surface) aGeomSurfHAdt =
    Handle(BRepAdaptor_Surface)::DownCast(theCoS.Surface());

  // Get curve from Geom
  Handle(BRepAdaptor_Curve2d) aGeomCurveHAdt =
    Handle(BRepAdaptor_Curve2d)::DownCast(theCoS.Curve2d());

  Adaptor3d_CurveOnSurface aCOnS = Adaptor3d_CurveOnSurface(aGeomCurveHAdt, aGeomSurfHAdt);

  gp_Pnt aPointOnPCurve;
  double aDist = 0.;
  ShapeAnalysis_Curve aCurveTools;
  if (theProjCache.IsPrevDefined)
  {
    aDist = aCurveTools.NextProject(theProjCache.PrevParameter, aCOnS,
      thePoint, Precision::Confusion(), aPointOnPCurve, theCoSParam);
  }

  if (!theProjCache.IsPrevDefined || aDist > theProjToler)
  {
    aDist = aCurveTools.Project(aCOnS, thePoint, Precision::Confusion(),
      aPointOnPCurve, theCoSParam, false);
  }

  theUVPoint = aGeomCurveHAdt->Value(theCoSParam);
  if (aDist > theProjToler)
  {
    Handle(Geom_Surface) aSurface = aGeomSurfHAdt->ChangeSurface().Surface();
    gp_Pnt aPoint = thePoint;

    // 7.4.0:
    // if (aGeomSurfHAdt->ChangeSurface().Trsf().Form() != gp_Identity)
    //   aPoint.Transform(aGeomSurfHAdt->ChangeSurface().Trsf().Inverted());
    if ( aGeomSurfHAdt->Trsf().Form() != gp_Identity )
      aPoint.Transform( aGeomSurfHAdt->Trsf().Inverted() );

    ShapeAnalysis_Surface aSurfTools(aSurface);
    gp_Pnt2d aUVPoint = aSurfTools.NextValueOfUV(theUVPoint, aPoint, Precision::Confusion());

    if (aGeomSurfHAdt->IsUPeriodic())
    {
      double aDU = Abs(theUVPoint.X() - aUVPoint.X());
      if (aDU > aGeomSurfHAdt->UPeriod() / 2)
      {
        double aFUPar = aGeomSurfHAdt->FirstUParameter();
        double aLUPar = aGeomSurfHAdt->LastUParameter();
        aUVPoint.SetX(ElCLib::InPeriod(aUVPoint.X(), aFUPar, aLUPar));
      }
    }
    if (aGeomSurfHAdt->IsVPeriodic())
    {
      double aDV = Abs(theUVPoint.Y() - aUVPoint.Y());
      if (aDV > aGeomSurfHAdt->VPeriod() / 2)
      {
        double aFVPar = aGeomSurfHAdt->FirstVParameter();
        double aLVPar = aGeomSurfHAdt->LastVParameter();
        aUVPoint.SetY(ElCLib::InPeriod(aUVPoint.Y(), aFVPar, aLVPar));
      }
    }

    // Check projection result
    if (aSurfTools.Gap() < 0)
      return false;
    else if (aSurfTools.Gap() < aDist)
      theUVPoint = aUVPoint;
  }

  // Set projection cache
  theProjCache.IsPrevDefined = true;
  theProjCache.PrevParameter = theCoSParam;

  return true;
}
