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
#include <asiAlgo_DiscrClassifier2d.h>

// asiAlgo includes
#include <asiAlgo_DiscrEdge.h>

// OpenCascade includes
#include <Bnd_Box2d.hxx>
#include <Standard_OutOfMemory.hxx>
#include <Precision.hxx>
#include <NCollection_IncAllocator.hxx>
#include <gp_Pnt2d.hxx>

using namespace asiAlgo::discr;

//-----------------------------------------------------------------------------

Classifier2d::ClassifierRow::ClassifierSeg::ClassifierSeg(const gp_Pnt2d&    theP1,
                                                          const gp_Pnt2d&    theP2,
                                                          const RayDirection theDir,
                                                          const SegAddress&  theSource)
{
  gp_Pnt2d aP1, aP2;

  if ( theDir == AlongX )
  {
    aP1.SetCoord(theP1.Y(),theP1.X());
    aP2.SetCoord(theP2.Y(),theP2.X());
  }
  else
  {
    aP1.SetCoord(theP1.X(),theP1.Y());
    aP2.SetCoord(theP2.X(),theP2.Y());
  }

  // Start point, end point
  if ( aP1.X() > aP2.X() )
  {
    myIsForward = false;
    myA1 = aP2.X();
    myO1 = aP2.Y();
    myA2 = aP1.X();
    myO2 = aP1.Y();
  }
  else
  {
    myIsForward = true;
    myA1 = aP1.X();
    myO1 = aP1.Y();
    myA2 = aP2.X();
    myO2 = aP2.Y();
  }

  // The unity vector of direction
  const double dfL=aP1.Distance(aP2);
  if ( dfL > gp::Resolution() )
  {
    myDA = (myA2-myA1)/dfL;
    myDO = (myO2-myO1)/dfL;
  }
  else
  {
    myDA = 0.;
    myDO = 0.;
  }
  mySource = theSource;
}

//-----------------------------------------------------------------------------

Location Classifier2d::ClassifierRow::Locate(const gp_Pnt2d& thePnt,
                                             double&         theTol,
                                             SegAddress&     theSrc,
                                             double&         thePar,
                                             const int       theWireIndex) const
{
  // OUT if no segments fell into the row
  if (mySegments.Extent() == 0) 
    return Location_OUT;
  double       aTol = theTol * theTol;
  int    iNbCrossed = 0;
  Location     aResult = Location_OUT;
  thePar = 0.;

  // Count intersections with segments
  gp_Vec2d aMinD;
  double aDistMini = RealLast();
  const ClassifierSeg* aClosestSeg = NULL;
  NCollection_List<const ClassifierSeg *>::Iterator anIter(mySegments);
  for (; anIter.More(); anIter.Next()) 
  {
    const ClassifierSeg& aSeg = * anIter.Value();
    if (theWireIndex && aSeg.Source().WireIndex() != theWireIndex)
      // selection by WireIndex
      continue;

    gp_Vec2d aDRef(aSeg.DA(), aSeg.DO());
    gp_Vec2d aD1  (thePnt, gp_Pnt2d(aSeg.A1(), aSeg.O1()));
    gp_Vec2d aD2  (gp_Pnt2d(aSeg.A2(), aSeg.O2()), thePnt);

    // The ray is launched only in the selected direction.
    // It is necessary to check how many segments the point
    // lies under.
    //
    // Example:
    //
    // (aSeg.A2(), aSeg.O2())        (aSeg.A1(), aSeg.O1())
    //                      -------->
    //                        _    /
    //                       |\  |/_             ^
    //                         \                 |
    //                          O                | ray
    //                           thePnt
    // aScalar1 <= 0
    // aScalar2 <= 0
    //
    // (aSeg.A2(), aSeg.O2())        (aSeg.A1(), aSeg.O1())
    //                      -------->
    //                  __          *
    //                   *|     *       ^
    //                 *   |*__         |
    //               O                  | ray
    //                thePnt
    // aScalar1 < 0
    // aScalar2 > 0
    //
    // (aSeg.A2(), aSeg.O2())        (aSeg.A1(), aSeg.O1())
    //                      -------->
    //                          __   *             ^
    //                        |*     _*|           |
    //                            *                | ray
    //                               *  O
    //                                   thePnt
    // aScalar1 > 0
    // aScalar2 < 0
    //

    // Check whether the point within segment's boundaries
    double aScalar1 = aD1 * aDRef;
    double aScalar2 = aD2 * aDRef;

    bool isWithinBoundaries = (aScalar1 <= 0. && aScalar2 <= 0.);

    // Geometrically skew product is the oriented area of the parallelogram =>
    // => S'(P1, P2, P3) = 0.5 * [P2P1, P1P3].
    //
    // On the other hand, we know the formula for calculating the area of a triangle =>
    // => S"(P1, P2, P3) = 0.5 * h * P1P2.
    //
    // Knowing the above, we can find h.
    //
    // In our case:
    // P1 = (aSeg.A1(), aSeg.O1())
    // P2 = (aSeg.A2(), aSeg.O2())
    // P3 = thePnt
    // h  = aDist -- We can use aDRef and get rid of the denominator.
    //
    //     P2                               P1
    //       O----------------------------O
    //       *                *_|        *
    //          *             *        *
    //             *        h *      *
    //                *       *    *
    //                   *    *  *
    //                       *O*
    //                        P3
    //
    double aDist   = (aD1 ^ aDRef);
    bool isBelow = aDist < 0.;
    if (isWithinBoundaries)
      aDist = aDist * aDist;
    else
      aDist = Min(aD1.SquareMagnitude(), aD2.SquareMagnitude());

    if (aDist > aTol)
    {
      // It is necessary to take into account the discretization error of segments =>
      // => Precision::Confusion().
      if (isBelow && aD1.X() < Precision::Confusion() && aD2.X() <= -Precision::Confusion())
        ++iNbCrossed;
    }
    else
    {
      // Among ONs we seek the nearest segment, so do not break the loop
      aResult = Location_ON;
      aTol    = aDist;
    }

    // Remember parameters of the nearest segment
    if (aDistMini > aDist)
    {
      aDistMini   = aDist;
      aClosestSeg = &aSeg;
      aMinD       = aD1;
    }
  }

  // If the ray from the point up to infinity crosses even number of 
  // segments, then this point is out of face
  if (aResult == Location_ON)
  {
    theTol = Sqrt(aTol);
    const double cA = aClosestSeg->A2() - aClosestSeg->A1();
    const double cO = aClosestSeg->O2() - aClosestSeg->O1();
    thePar  = - (aMinD.X() * cA + aMinD.Y() * cO) / (cA * cA + cO * cO);
    if(!aClosestSeg->IsForward())
      thePar = 1 - thePar;
  }
  else if ((iNbCrossed & 1) != 0)
    aResult = Location_IN;

  theSrc = aClosestSeg->Source();
  return aResult;
}

//-----------------------------------------------------------------------------

Classifier2d::Classifier2d(const Face&  face,
                           const double tol)
: myDirection    ( AlongY ),
  myRows         ( NULL ),
  myNbRows       ( 0 ),
  myAMin         (-RealLast() ),
  myAMax         ( RealLast() ),
  myAStep        ( 0. ),
  myTol          ( tol ),
  myMemAllocator ( new NCollection_IncAllocator(3000) )
{
  if ( face.IsNull() )
    return;

  // ----------------------- Compute the preferrable parameters
  // Find the box including all nodes
  // Plus calculate the number of rows
  // Plus find the preferrable ray direction
  int iSlopeX=0;

  NCollection_List <ClsSeg> aBulk;
  double dfXMin =  RealLast();
  double dfXMax = -RealLast();
  double dfYMin =  RealLast();
  double dfYMax = -RealLast();
  double dfTotXSpan = 0.;
  double dfTotYSpan = 0.;
  Bnd_Box2d aBox;

  int iNbWires = face.GetNbWires();
  int iWire;
  for (iWire=1; iWire<=iNbWires; iWire++)
  {
    // The face boundary consists of several wires, we shall look through
    // them all
    const Wire& aWire  = face.GetWire(iWire);
    const int iNbEdges = aWire.GetNbEdges();
    int iEdge;
    for (iEdge=1; iEdge<=iNbEdges; iEdge++)
    {
      // Each wire is a set of edges, we shall take the corresponding discrete
      // pcurve from each and scratch the points to create segments2d
      const PairOfPEdgeBoolean& anEdgeData =  aWire.GetEdgeData(iEdge);
      const Edge&               anEdge     = *anEdgeData.first;
      int                       iCurve     =  anEdge.FindPCurve(face, anEdgeData.second);
      const Curve2d&            aDPCurve   =  anEdge.GetPCurve(iCurve);
      const int                 iNbPoints  =  aDPCurve.NbPoints();
      int iPnt;
      for ( iPnt=1; iPnt<iNbPoints; iPnt++ )
      {
        const gp_Pnt2d& aP1 = aDPCurve.Point(iPnt);
        const gp_Pnt2d& aP2 = aDPCurve.Point(iPnt+1);

        // Update the boundaries and measure the segment
        aBox.Add(aP1);
        aBox.Add(aP2);

        double dfXSpan = Abs(aP2.X()-aP1.X());
        double dfYSpan = Abs(aP2.Y()-aP1.Y());

        // Look at the slope
        if (dfXSpan > dfYSpan)
          iSlopeX++;
        else
          iSlopeX--;

        // Save the segment to avoid re-exploring
        SegAddress aSource (iWire, iEdge, iCurve, iPnt);
        aBulk.Append (ClsSeg(aP1, aP2, aSource));

      } // for (iPnt=1; iPnt<iNbPoints; iPnt++)
    } // for (iEdge=1; iEdge<=iNbEdges; iEdge++)
  } // for (iWire=1; iWire<=iNbWires; iWire++)

  // Calculate total spans to estimate the row width
  aBox.Get(dfXMin, dfYMin, dfXMax, dfYMax);
  dfTotXSpan = dfXMax - dfXMin;
  dfTotYSpan = dfYMax - dfYMin;

  completeConstruction (aBulk, dfXMin, dfXMax, dfTotXSpan,
                        dfYMin, dfYMax, dfTotYSpan, iSlopeX);
}

//-----------------------------------------------------------------------------

Classifier2d::Classifier2d(NCollection_List<gp_Pnt2d> thePList,
                           const double               theTol)
 : myDirection    (AlongY),
   myRows         (NULL),
   myNbRows       (0),
   myAMin         (-RealLast()),
   myAMax         (RealLast()),
   myAStep        (0.),
   myTol          (theTol),
   myMemAllocator (new NCollection_IncAllocator (3000))
{
  // ----------------------- Compute the preferrable parameters
  // Find the box including all nodes                                 
  // Plus calculate the number of rows
  // Plus find the preferrable ray direction
  int iSlopeX=0;

  NCollection_List <ClsSeg> aBulk;
  double dfXMin =  RealLast();
  double dfXMax = -RealLast();
  double dfYMin =  RealLast();
  double dfYMax = -RealLast();
  double dfTotXSpan = 0.;
  double dfTotYSpan = 0.;
  Bnd_Box2d aBox;

  NCollection_List<gp_Pnt2d>::Iterator anPIter(thePList);
  if (!anPIter.More())
    // No points
    return;
  const gp_Pnt2d& aFirstPoint = anPIter.Value();
  while (anPIter.More())
  {
    const gp_Pnt2d& aP1 = anPIter.Value();
    anPIter.Next();
    gp_Pnt2d aP2;
    
    if (anPIter.More())
    {
      aP2 = anPIter.Value();
    }
    else
    {
      aP2 = aFirstPoint;
    }
   
    // Update the boundaries and measure the segment
    aBox.Add(aP1);
    aBox.Add(aP2);

    double dfXSpan = Abs(aP2.X()-aP1.X());
    double dfYSpan = Abs(aP2.Y()-aP1.Y());

    // Look at the slope
    if (dfXSpan > dfYSpan)
      iSlopeX++;
    else
      iSlopeX--;
    
    // Save the segment to avoid re-exploring
    SegAddress aSource(0,0,0,0);
    aBulk.Append (ClsSeg(aP1, aP2, aSource));
  }

  // Calculate total spans to estimate the row width
  aBox.Get(dfXMin, dfYMin, dfXMax, dfYMax);
  dfTotXSpan = dfXMax - dfXMin;
  dfTotYSpan = dfYMax - dfYMin;

  completeConstruction (aBulk, dfXMin, dfXMax, dfTotXSpan,
                        dfYMin, dfYMax, dfTotYSpan, iSlopeX);
}

//-----------------------------------------------------------------------------

void Classifier2d::completeConstruction(const NCollection_List<ClsSeg>& theBulk,
                                        const double                    theXMin,
                                        const double                    theXMax,
                                        const double                    theTotXSpan,
                                        const double                    theYMin,
                                        const double                    theYMax,
                                        const double                    theTotYSpan,
                                        const int                       theSlopeX)
{
  const int iTotNbSegments = theBulk.Extent();
  if ( iTotNbSegments == 0 )
    return; // No segments were found - infinite face?

  // Now estimations:
  // 1. If more often dx/dy > 1. then ray direction is better along Y,
  //    i.e. abscissa=x, ordinate=y
  // 2. Let the step (row width) be an average span along the abscissa
  if (theSlopeX > 0)
  {
    myDirection = AlongY;
    myAMin  = theXMin;
    myAMax  = theXMax;
    myAStep = theTotXSpan / iTotNbSegments;
  }
  else
  {
    myDirection = AlongX;
    myAMin  = theYMin;
    myAMax  = theYMax;
    myAStep = theTotYSpan / iTotNbSegments;
  }

  // ----------------------- Sort the segments by rows

  if (myTol < 0.)
    myTol = 0.01 * myAStep;

  // Slightly correct the step and number of rows
  myAMin -= 2 * myTol;
  myAMax += 2 * myTol;
  if (myAStep < myTol)
    myNbRows = int ((myAMax-myAMin)/myTol);
  else 
    myNbRows = int ((myAMax-myAMin)/myAStep) + 1;
  myAStep = (myAMax-myAMin) / myNbRows;

  // Allocation
  myRows = new ClassifierRow[myNbRows];
  if (myRows == NULL)
    throw Standard_OutOfMemory("asiAlgo_DiscrClassifier2d constructor");
  int iRow;
  for (iRow=0; iRow<myNbRows; iRow++)
    myRows[iRow] = ClassifierRow(myDirection,myMemAllocator);

  // Fill every row in the table                                       
  //    Loop on segments
  NCollection_List<ClsSeg>::Iterator anIter(theBulk);
  for (; anIter.More(); anIter.Next()) 
  {
    const ClsSeg& aClsSeg = anIter.Value();
    ClassifierRow::ClassifierSeg aSegTmp (aClsSeg.Point(0), aClsSeg.Point(1),
                                          myDirection, aClsSeg.Source());
    const ClassifierRow::ClassifierSeg& aSeg = mySegments.Append (aSegTmp);

    // Add the current edge to all rows where it is present geometrically
    const int iFirst = int((aSeg.AMin()-myTol-myAMin)/ myAStep);
    const int iLast  = Min(myNbRows - 1, int((aSeg.AMax()+myTol-myAMin)/ myAStep));
    for (int i = iFirst; i <= iLast; i++)
      myRows[i].AddSegment (aSeg);
  }
}

//-----------------------------------------------------------------------------

Location Classifier2d::Locate(const gp_Pnt2d& thePnt,
                              double&         theDist,
                              SegAddress&     theSource,
                              double&         theParam,
                              const int       theWireIndex) const
{
  // If no boundary supplied, consider IN 
  if (myNbRows == 0)
    return Location_IN;

  // Transform XY to AO (abscissa,ordinate)
  gp_Pnt2d anAO;
  if (myDirection == AlongX)
    anAO.SetCoord (thePnt.Y(), thePnt.X());
  else 
    anAO.SetCoord (thePnt.X(), thePnt.Y());

  const int iRow = int ((anAO.X() - myAMin)/ myAStep);
  // If the point is beyond the bounds, then OUT
  if (iRow < 0 || iRow >= myNbRows)
    return Location_OUT;

  // Pass the point to the row it belongs to
  double aTol = theDist < 0. ? myTol : theDist;
//  Standard_ProgramError_Raise_if (aTol > myTol + Precision::Confusion(),
//                                  "asiAlgo_DiscrClassifier2d::Locate: "
//                                  "wrong tolerance");
  Location aResult = myRows[iRow].Locate(anAO,
                                         aTol,
                                         theSource,
                                         theParam,
                                         theWireIndex);
  if (aResult == Location_ON)
    theDist = aTol;

  return aResult;
}

//-----------------------------------------------------------------------------

bool Classifier2d::IsOut(const gp_Pnt2d& theLoLeft,
                         const gp_Pnt2d& theUpRite)
{
  // If no boundary supplied, consider IN 
  if (myNbRows == 0)
    return false;

  // Transform XY to AO (abscissa,ordinate)
  gp_Pnt2d aLL, aUR;
  if (myDirection == AlongX)
  {
    aLL.SetCoord (theLoLeft.Y(), theLoLeft.X());
    aUR.SetCoord (theUpRite.Y(), theUpRite.X());
  }
  else
  {
    aLL.SetCoord (theLoLeft.X(), theLoLeft.Y());
    aUR.SetCoord (theUpRite.X(), theUpRite.Y());
  }

  int iRow1 = int ((aLL.X() - myAMin) / myAStep);
  int iRow2 = int ((aUR.X() - myAMin) / myAStep);
  if (iRow1 > iRow2)
    throw Standard_ProgramError ("Classifier2d::LocateBox - wrong box "
                                 "coordinates (should be LowLeft-UpRight)");

  // If the box is beyond the bounds, then OUT
  if (iRow2 < 0 || iRow1 >= myNbRows)
    return true;

  gp_XY aCor[4], aSideOrt[2];
  aCor[0] = gp_XY (aLL.X(), aLL.Y());
  aCor[1] = gp_XY (aUR.X(), aLL.Y());
  aCor[2] = gp_XY (aUR.X(), aUR.Y());
  aCor[3] = gp_XY (aLL.X(), aUR.Y());
  aSideOrt[0] = gp_XY (1., 0.);
  aSideOrt[1] = gp_XY (0., 1.);
  const double aPrec = myTol;
  const double aPrecA = Precision::Angular();
  int iRow;
  for(iRow=iRow1; iRow<=iRow2; iRow++)
  {
    NCollection_List<const ClassifierRow::ClassifierSeg *>::Iterator
      anIter(myRows[iRow].mySegments);
    for (; anIter.More(); anIter.Next())      // loop by segments of a row
    {
      const ClassifierRow::ClassifierSeg& aSeg = * anIter.Value();

      // check if a segment vertex is IN box
      if ((aLL.X() - aSeg.A1() < aPrec && aSeg.A1() - aUR.X() < aPrec &&
          aLL.Y() - aSeg.O1() < aPrec && aSeg.O1() - aUR.Y() < aPrec) ||
          (aLL.X() - aSeg.A2() < aPrec && aSeg.A2() - aUR.X() < aPrec &&
          aLL.Y() - aSeg.O2() < aPrec && aSeg.O2() - aUR.Y() < aPrec))
        return false;

      // check intersections of box sides by segment

      // avoid intersect of collinear segments
      if (aSeg.DA() < aPrecA || aSeg.DO() < aPrecA)
        continue;

      const gp_XY aSegDir (aSeg.DA(), aSeg.DO());
      const gp_XY aSegP1 (aSeg.A1(), aSeg.O1());
      const gp_XY aSegP2 (aSeg.A2(), aSeg.O2());
      for (int i = 0; i < 4; i++) {      // loop by sides of the box
        const int i1 = (i+1) & 3;
        const gp_XY& aSideDir = aSideOrt[i & 1];
        const gp_XY aCor1P1 (aSegP1 - aCor[i]);
        const gp_XY aCor1P2 (aSegP2 - aCor[i]);
        if ((aSideDir ^ aCor1P1) * (aSideDir ^ aCor1P2) <= 0.) {
          const gp_XY aCor2P1 (aSegP1 - aCor[i1]);
          if ((aSegDir ^ aCor1P1) * (aSegDir ^ aCor2P1) <= 0.)
            return false;
        }
      }
    }
  }

  // No touch means really OUT
  return true;
}
