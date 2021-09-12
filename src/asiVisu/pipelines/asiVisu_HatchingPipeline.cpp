//-----------------------------------------------------------------------------
// Created on: 03 September 2021
//-----------------------------------------------------------------------------
// Copyright (c) 2021-present, Sergey Slyadnev
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
#include <asiVisu_HatchingPipeline.h>

// asiVisu includes
#include <asiVisu_CurveSource.h>
#include <asiVisu_HatchingDataProvider.h>
#include <asiVisu_Utils.h>

// OpenCascade includes
#include <Adaptor3d_HCurve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dHatch_Hatcher.hxx>
#include <Draw_Color.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_OrientedShapeMapHasher.hxx>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------

//! Face + Array of iso.
class DBRep_Face : public Standard_Transient
{
DEFINE_STANDARD_RTTI_INLINE(DBRep_Face, Standard_Transient)

public:

  //! N is the number of iso intervals.
  DBRep_Face(const TopoDS_Face& F, const int N)
  : myFace(F),
    myTypes(N ? 1 : 0,N),
    myParams(N ? 1 : 0,3*N)
  {}

  const TopoDS_Face& Face() const
  {
    return myFace;
  }

  void Face(const TopoDS_Face& F)
  {
    myFace = F;
  }

  int NbIsos() const
  {
    return myTypes.Upper();
  }

  void Iso(const int I, const GeomAbs_IsoType T, const double Par, const double T1, const double T2)
  {
    myTypes(I) = (int) T;
    myParams(3*I - 2) = Par;
    myParams(3*I - 1) = T1;
    myParams(3*I)     = T2;
  }

  void GetIso (const int I, GeomAbs_IsoType& T, double& Par, double& T1, double& T2) const
  {
    int IntTyp = myTypes(I);
    T   =  (GeomAbs_IsoType) IntTyp;
    Par =  myParams(3*I - 2);
    T1  =  myParams(3*I - 1);
    T2  =  myParams(3*I);
  }

private:

  TopoDS_Face             myFace;
  TColStd_Array1OfInteger myTypes;
  TColStd_Array1OfReal    myParams;

};

//-----------------------------------------------------------------------------

typedef NCollection_List<Handle(DBRep_Face)> DBRep_ListOfFace;
typedef NCollection_List<Handle(DBRep_Face)>::Iterator DBRep_ListIteratorOfListOfFace;

//-----------------------------------------------------------------------------

// Providing consistency with intersection tolerance for the linear curves
static double IntersectorConfusion = Precision::PConfusion();
static double IntersectorTangency  = Precision::PConfusion();
static double HatcherConfusion2d   = 1.e-8 ;
static double HatcherConfusion3d   = 1.e-8 ;

//! Creation of isoparametric curves.
class DBRep_IsoBuilder : public Geom2dHatch_Hatcher
{
public:

  //! Creates the builder.
  DBRep_IsoBuilder(const TopoDS_Face& TopologicalFace, const double Infinite, const int NbIsos)
  : Geom2dHatch_Hatcher(Geom2dHatch_Intersector(IntersectorConfusion,
                                                IntersectorTangency),
                        HatcherConfusion2d,
                        HatcherConfusion3d,
                        true,
                        false),
    myInfinite (Infinite) ,
    myUMin     (0.0),
    myUMax     (0.0),
    myVMin     (0.0),
    myVMax     (0.0),
    myUPrm     (1, NbIsos),
    myUInd     (1, NbIsos),
    myVPrm     (1, NbIsos),
    myVInd     (1, NbIsos),
    myNbDom    (0)
  {
    myUInd.Init(0);
    myVInd.Init(0);

    //-----------------------------------------------------------------------
    // If the Min Max bounds are infinite, there are bounded to Infinite
    // value.
    //-----------------------------------------------------------------------

    BRepTools::UVBounds (TopologicalFace, myUMin, myUMax, myVMin, myVMax);
    //
    bool InfiniteUMin = Precision::IsNegativeInfinite(myUMin);
    bool InfiniteUMax = Precision::IsPositiveInfinite(myUMax);
    bool InfiniteVMin = Precision::IsNegativeInfinite(myVMin);
    bool InfiniteVMax = Precision::IsPositiveInfinite(myVMax);
    //
    if (InfiniteUMin && InfiniteUMax)
    {
      myUMin = - Infinite ;
      myUMax =   Infinite ;
    }
    else if (InfiniteUMin)
    {
      myUMin = myUMax - Infinite;
    }
    else if (InfiniteUMax)
    {
      myUMax = myUMin + Infinite;
    }
    if (InfiniteVMin && InfiniteVMax)
    {
      myVMin = - Infinite;
      myVMax =   Infinite;
    }
    else if (InfiniteVMin)
    {
      myVMin = myVMax - Infinite;
    }
    else if (InfiniteVMax)
    {
      myVMax = myVMin + Infinite;
    }

    //-----------------------------------------------------------------------
    // Retrieving the edges and its p-curves for further trimming
    // and loading them into the hatcher
    //-----------------------------------------------------------------------
    DataMapOfEdgePCurve anEdgePCurveMap;

    TopExp_Explorer ExpEdges;
    for (ExpEdges.Init (TopologicalFace, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next())
    {
      const TopoDS_Edge& TopologicalEdge = TopoDS::Edge (ExpEdges.Current());
      double U1, U2;
      const Handle(Geom2d_Curve) PCurve = BRep_Tool::CurveOnSurface (TopologicalEdge, TopologicalFace, U1, U2);

      if (PCurve.IsNull())
      {
  #ifdef OCCT_DEBUG
        std::cout << "DBRep_IsoBuilder : PCurve is null\n";
  #endif
        return;
      }
      else if (U1 == U2)
      {
  #ifdef OCCT_DEBUG
        std::cout << "DBRep_IsoBuilder PCurve : U1==U2\n";
  #endif
        return;
      }

      //-- Test if a TrimmedCurve is necessary
      if (Abs(PCurve->FirstParameter()-U1)<= Precision::PConfusion()
        && Abs(PCurve->LastParameter()-U2)<= Precision::PConfusion())
      {
        anEdgePCurveMap.Add(TopologicalEdge, PCurve);
      }
      else
      {
        if (!PCurve->IsPeriodic())
        {
          Handle (Geom2d_TrimmedCurve) TrimPCurve = Handle(Geom2d_TrimmedCurve)::DownCast (PCurve);
          if (!TrimPCurve.IsNull())
          {
            if (TrimPCurve->BasisCurve()->FirstParameter() - U1 > Precision::PConfusion() ||
              TrimPCurve->BasisCurve()->FirstParameter() - U2 > Precision::PConfusion() ||
              U1 - TrimPCurve->BasisCurve()->LastParameter()  > Precision::PConfusion() ||
              U2 - TrimPCurve->BasisCurve()->LastParameter()  > Precision::PConfusion())
            {
  #ifdef OCCT_DEBUG
              std::cout << "DBRep_IsoBuilder TrimPCurve : parameters out of range\n";
              std::cout << "    U1(" << U1 << "), Umin(" << PCurve->FirstParameter()
                << "), U2("  << U2 << "), Umax(" << PCurve->LastParameter() << ")\n";
  #endif
              return;
            }
          }
          else
          {
            if (PCurve->FirstParameter() - U1 > Precision::PConfusion())
            {
  #ifdef OCCT_DEBUG
              std::cout << "DBRep_IsoBuilder PCurve : parameters out of range\n";
              std::cout << "    U1(" << U1 << "), Umin(" << PCurve->FirstParameter() << ")\n";
  #endif
              U1 = PCurve->FirstParameter();
            }
            if (PCurve->FirstParameter() - U2 > Precision::PConfusion())
            {
  #ifdef OCCT_DEBUG
              std::cout << "DBRep_IsoBuilder PCurve : parameters out of range\n";
              std::cout << "    U2(" << U2 << "), Umin(" << PCurve->FirstParameter() << ")\n";
  #endif
              U2 = PCurve->FirstParameter();
            }
            if (U1 - PCurve->LastParameter() > Precision::PConfusion())
            {
  #ifdef OCCT_DEBUG
              std::cout << "DBRep_IsoBuilder PCurve : parameters out of range\n";
              std::cout << "    U1(" << U1 << "), Umax(" << PCurve->LastParameter() << ")\n";
  #endif
              U1 = PCurve->LastParameter();
            }
            if (U2 - PCurve->LastParameter() > Precision::PConfusion())
            {
  #ifdef OCCT_DEBUG
              std::cout << "DBRep_IsoBuilder PCurve : parameters out of range\n";
              std::cout << "    U2(" << U2 << "), Umax(" << PCurve->LastParameter() << ")\n";
  #endif
              U2 = PCurve->LastParameter();
            }
          }
        }

        // if U1 and U2 coincide-->do nothing
        if (Abs (U1 - U2) <= Precision::PConfusion()) continue;
        Handle (Geom2d_TrimmedCurve) TrimPCurve = new Geom2d_TrimmedCurve (PCurve, U1, U2);
        anEdgePCurveMap.Add(TopologicalEdge, TrimPCurve);
      }
    }

    // Fill the gaps between 2D curves, and trim the intersecting ones.
    FillGaps(TopologicalFace, anEdgePCurveMap);

    // Load trimmed curves to the hatcher
    int aNbE = anEdgePCurveMap.Extent();
    for (int iE = 1; iE <= aNbE; ++iE)
    {
      AddElement(Geom2dAdaptor_Curve(anEdgePCurveMap(iE)),
                 anEdgePCurveMap.FindKey(iE).Orientation());
    }
    //-----------------------------------------------------------------------
    // Loading and trimming the hatchings.
    //-----------------------------------------------------------------------

    int IIso ;
    double DeltaU = Abs (myUMax - myUMin) ;
    double DeltaV = Abs (myVMax - myVMin) ;
    double confusion = Min (DeltaU, DeltaV) * HatcherConfusion3d ;
    Confusion3d (confusion) ;

    double StepU = DeltaU / (double) NbIsos ;
    if (StepU > confusion) {
      double UPrm = myUMin + StepU / 2. ;
      gp_Dir2d Dir (0., 1.) ;
      for (IIso = 1 ; IIso <= NbIsos ; IIso++) {
        myUPrm(IIso) = UPrm ;
        gp_Pnt2d Ori (UPrm, 0.) ;
        Geom2dAdaptor_Curve HCur (new Geom2d_Line (Ori, Dir)) ;
        myUInd(IIso) = AddHatching (HCur) ;
        UPrm += StepU ;
      }
    }

    double StepV = DeltaV / (double) NbIsos ;
    if (StepV > confusion) {
      double VPrm = myVMin + StepV / 2. ;
      gp_Dir2d Dir (1., 0.) ;
      for (IIso = 1 ; IIso <= NbIsos ; IIso++) {
        myVPrm(IIso) = VPrm ;
        gp_Pnt2d Ori (0., VPrm) ;
        Geom2dAdaptor_Curve HCur (new Geom2d_Line (Ori, Dir)) ;
        myVInd(IIso) = AddHatching (HCur) ;
        VPrm += StepV ;
      }
    }

    //-----------------------------------------------------------------------
    // Computation.
    //-----------------------------------------------------------------------

    Trim() ;

    myNbDom = 0 ;
    for (IIso = 1 ; IIso <= NbIsos ; IIso++)
    {
      int Index ;

      Index = myUInd(IIso) ;
      if (Index != 0)
      {
        if (TrimDone (Index) && !TrimFailed (Index))
        {
          ComputeDomains (Index);
          if (IsDone (Index))
            myNbDom = myNbDom + Geom2dHatch_Hatcher::NbDomains (Index) ;
        }
      }

      Index = myVInd(IIso) ;
      if (Index != 0)
      {
        if (TrimDone (Index) && !TrimFailed (Index))
        {
          ComputeDomains (Index);
          if (IsDone (Index))
            myNbDom = myNbDom + Geom2dHatch_Hatcher::NbDomains (Index) ;
        }
      }
    }
  }

  //! Returns the total number of domains.
  int NbDomains() const
  {
    return myNbDom;
  }
  
  //! Loading of the isoparametric curves in the
  //! Data Structure of a drawable face.
  void LoadIsos (const Handle(DBRep_Face)& Face) const
  {
    int NumIso = 0 ;

    for (int UIso = myUPrm.Lower() ; UIso <= myUPrm.Upper() ; UIso++) {
      int UInd = myUInd.Value (UIso) ;
      if (UInd != 0) {
        double UPrm = myUPrm.Value (UIso) ;
        if (!IsDone (UInd)) {
          std::cout << "DBRep_IsoBuilder:: U iso of parameter: " << UPrm ;
          switch (Status (UInd))
          {
            case HatchGen_NoProblem          : std::cout << " No Problem"          << std::endl ; break ;
            case HatchGen_TrimFailure        : std::cout << " Trim Failure"        << std::endl ; break ;
            case HatchGen_TransitionFailure  : std::cout << " Transition Failure"  << std::endl ; break ;
            case HatchGen_IncoherentParity   : std::cout << " Incoherent Parity"   << std::endl ; break ;
            case HatchGen_IncompatibleStates : std::cout << " Incompatible States" << std::endl ; break ;
          }
        }
        else
        {
          int NbDom = Geom2dHatch_Hatcher::NbDomains (UInd) ;
          for (int IDom = 1 ; IDom <= NbDom ; IDom++)
          {
            const HatchGen_Domain& Dom = Domain (UInd, IDom) ;
            double V1 = Dom.HasFirstPoint()  ? Dom.FirstPoint().Parameter()  : myVMin - myInfinite ;
            double V2 = Dom.HasSecondPoint() ? Dom.SecondPoint().Parameter() : myVMax + myInfinite ;
            NumIso++ ;
            Face->Iso (NumIso, GeomAbs_IsoU, UPrm, V1, V2) ;
          }
        }
      }
    }

    for (int VIso = myVPrm.Lower() ; VIso <= myVPrm.Upper() ; VIso++) {
      int VInd = myVInd.Value (VIso) ;
      if (VInd != 0) {
        double VPrm = myVPrm.Value (VIso) ;
        if (!IsDone (VInd)) {
          std::cout << "DBRep_IsoBuilder:: V iso of parameter: " << VPrm ;
          switch (Status (VInd))
          {
            case HatchGen_NoProblem          : std::cout << " No Problem"          << std::endl ; break ;
            case HatchGen_TrimFailure        : std::cout << " Trim Failure"        << std::endl ; break ;
            case HatchGen_TransitionFailure  : std::cout << " Transition Failure"  << std::endl ; break ;
            case HatchGen_IncoherentParity   : std::cout << " Incoherent Parity"   << std::endl ; break ;
            case HatchGen_IncompatibleStates : std::cout << " Incompatible States" << std::endl ; break ;
          }
        }
        else
        {
          int NbDom = Geom2dHatch_Hatcher::NbDomains (VInd) ;
          for (int IDom = 1 ; IDom <= NbDom ; IDom++) {
            const HatchGen_Domain& Dom = Domain (VInd, IDom) ;
            double U1 = Dom.HasFirstPoint()  ? Dom.FirstPoint().Parameter()  : myVMin - myInfinite ;
            double U2 = Dom.HasSecondPoint() ? Dom.SecondPoint().Parameter() : myVMax + myInfinite ;
            NumIso++ ;
            Face->Iso (NumIso, GeomAbs_IsoV, VPrm, U1, U2) ;
          }
        }
      }
    }
  }

protected:

  typedef NCollection_IndexedDataMap
    <TopoDS_Shape, Handle(Geom2d_Curve), TopTools_OrientedShapeMapHasher>
      DataMapOfEdgePCurve;

  //! Adds to the hatcher the 2D segments connecting the p-curves
  //! of the neighboring edges to close the 2D gaps which are
  //! closed in 3D by the tolerance of vertices shared between edges.
  //! It will allow trimming correctly the iso-lines passing through
  //! such gaps.
  //! The method also trims the intersecting 2D curves of the face,
  //! forbidding the iso-lines beyond the face boundaries.
  void FillGaps(const TopoDS_Face& theFace,
                DataMapOfEdgePCurve& theEdgePCurveMap)
  {
    // Get surface of the face for getting the 3D points from 2D coordinates
    // of the p-curves bounds
    BRepAdaptor_Surface aBASurf(theFace, false);

    // Analyze each wire of the face separately
    TopoDS_Iterator aItW(theFace);
    for (; aItW.More(); aItW.Next())
    {
      const TopoDS_Shape& aW = aItW.Value();
      if (aW.ShapeType() != TopAbs_WIRE)
        continue;

      // Use WireExplorer to iterate on edges of the wire
      // to get the pairs of connected edges.
      // Using WireExplorer will also allow avoiding treatment
      // of the internal wires.
      BRepTools_WireExplorer aWExp;
      aWExp.Init(TopoDS::Wire(aW), theFace, myUMin, myUMax, myVMin, myVMax);
      if (!aWExp.More())
        continue;

      // Check the number of edges in the wire, not to
      // miss the wires containing one edge only
      if (aW.NbChildren() == 0)
      {
        continue;
      }
      bool SingleEdge = (aW.NbChildren() == 1);

      TopoDS_Edge aPrevEdge, aCurrEdge;

      // Get first edge and its p-curve
      aCurrEdge = aWExp.Current();

      // Ensure analysis of the pair of first and last edges
      TopoDS_Edge aFirstEdge = aCurrEdge;
      double bStop = false;

      // Iterate on all other edges
      while (!bStop)
      {
        // Iteration to the next edge
        aPrevEdge = aCurrEdge;
        aWExp.Next();
        // Get the current edge for analysis
        if (aWExp.More())
        {
          aCurrEdge = aWExp.Current();
        }
        else
        {
          aCurrEdge = aFirstEdge;
          bStop = true;
        }

        if (aPrevEdge.IsEqual(aCurrEdge) && !SingleEdge)
          continue;

        // Get p-curves
        Handle(Geom2d_Curve)* pPC1 = theEdgePCurveMap.ChangeSeek(aPrevEdge);
        Handle(Geom2d_Curve)* pPC2 = theEdgePCurveMap.ChangeSeek(aCurrEdge);
        if (!pPC1 || !pPC2)
          continue;

        Handle(Geom2d_Curve)& aPrevC2d = *pPC1;
        Handle(Geom2d_Curve)& aCurrC2d = *pPC2;

        // Get p-curves parameters
        double fp, lp, fc, lc;
        fp = aPrevC2d->FirstParameter();
        lp = aPrevC2d->LastParameter();
        fc = aCurrC2d->FirstParameter();
        lc = aCurrC2d->LastParameter();

        // Get common vertex to check if the gap between two edges is closed
        // by the tolerance value of this vertex.
        // Take into account the orientation of the edges to obtain the correct
        // parameter of the vertex on edges.

        // Get vertex on the previous edge
        TopoDS_Vertex aCVOnPrev = TopExp::LastVertex(aPrevEdge, true);
        if (aCVOnPrev.IsNull())
          continue;

        // Get parameter of the vertex on the previous edge
        double aTPrev = BRep_Tool::Parameter(aCVOnPrev, aPrevEdge);
        if (aTPrev < fp)
          aTPrev = fp;
        else if (aTPrev > lp)
          aTPrev = lp;

        // Get vertex on the current edge
        TopoDS_Vertex aCVOnCurr = TopExp::FirstVertex(aCurrEdge, true);
        if (aCVOnCurr.IsNull() || !aCVOnPrev.IsSame(aCVOnCurr))
          continue;

        // Get parameter of the vertex on the current edge
        double aTCurr = BRep_Tool::Parameter(aCVOnCurr, aCurrEdge);
        if (aTCurr < fc)
          aTCurr = fc;
        else if (aTCurr > lc)
          aTCurr = lc;

        // Get bounding points on the edges corresponding to the current vertex
        gp_Pnt2d aPrevP2d = aPrevC2d->Value(aTPrev),
                 aCurrP2d = aCurrC2d->Value(aTCurr);

        // Check if the vertex covers these bounding points by its tolerance
        double aTolV2 = BRep_Tool::Tolerance(aCVOnPrev);
        gp_Pnt aPV = BRep_Tool::Pnt(aCVOnPrev);
        // There is no need to check the distance if the tolerance
        // of vertex is infinite (like in the test case sewing/tol_1/R2)
        if (aTolV2 < Precision::Infinite())
        {
          aTolV2 *= aTolV2;

          // Convert bounding point on previous edge into 3D
          gp_Pnt aPrevPS = aBASurf.Value(aPrevP2d.X(), aPrevP2d.Y());

          // Check if the vertex closes the gap
          if (aPV.SquareDistance(aPrevPS) > aTolV2)
            continue;

          // Convert bounding point on current edge into 3D
          gp_Pnt aCurrPS = aBASurf.Value(aCurrP2d.X(), aCurrP2d.Y());

          // Check if the vertex closes the gap
          if (aPV.SquareDistance(aCurrPS) > aTolV2)
            continue;
        }

        // Create the segment
        gp_Vec2d aV2d(aPrevP2d, aCurrP2d);
        double aSegmLen = aV2d.Magnitude();
        // Do not add too small segments
        bool bAddSegment = (aSegmLen > Precision::PConfusion());
        // Check for periodic surfaces
        if (bAddSegment)
        {
          if (aBASurf.IsUPeriodic())
            bAddSegment = aSegmLen < aBASurf.UPeriod() / 4.;

          if (bAddSegment && aBASurf.IsVPeriodic())
            bAddSegment = aSegmLen < aBASurf.VPeriod() / 4.;
        }

        // Check that p-curves do not interfere near the vertex.
        // And, if they do interfere, avoid creation of the segment.
        if (bAddSegment && !aPrevEdge.IsEqual(aCurrEdge))
        {
          Geom2dAdaptor_Curve aPrevGC(aPrevC2d, fp, lp), aCurrGC(aCurrC2d, fc, lc);
          Geom2dInt_GInter anInter(aPrevGC, aCurrGC, Precision::PConfusion(), Precision::PConfusion());
          if (anInter.IsDone() && !anInter.IsEmpty())
          {
            // Collect intersection points
            NCollection_List<IntRes2d_IntersectionPoint> aLPInt;
            // Get bounding points from segments
            int iP, aNbInt = anInter.NbSegments();
            for (iP = 1; iP <= aNbInt; ++iP)
            {
              aLPInt.Append(anInter.Segment(iP).FirstPoint());
              aLPInt.Append(anInter.Segment(iP).LastPoint());
            }
            // Get intersection points
            aNbInt = anInter.NbPoints();
            for (iP = 1; iP <= aNbInt; ++iP)
              aLPInt.Append(anInter.Point(iP));

            // Analyze the points and find the one closest to the current vertex
            bool bPointFound = false;
            double aTPrevClosest = 0., aTCurrClosest = 0.;
            double aDeltaPrev = ::RealLast(), aDeltaCurr = ::RealLast();

            NCollection_List<IntRes2d_IntersectionPoint>::Iterator aItLPInt(aLPInt);
            for (; aItLPInt.More(); aItLPInt.Next())
            {
              const IntRes2d_IntersectionPoint& aPnt = aItLPInt.Value();
              const double aTIntPrev = aPnt.ParamOnFirst();
              const double aTIntCurr = aPnt.ParamOnSecond();
              // Check if the intersection point is in range
              if (aTIntPrev < fp || aTIntPrev > lp ||
                  aTIntCurr < fc || aTIntCurr > lc)
              {
                continue;
              }

              double aDelta1 = Abs(aTIntPrev - aTPrev);
              double aDelta2 = Abs(aTIntCurr - aTCurr);
              if (aDelta1 < aDeltaPrev || aDelta2 < aDeltaCurr)
              {
                aTPrevClosest = aTIntPrev;
                aTCurrClosest = aTIntCurr;
                aDeltaPrev = aDelta1;
                aDeltaCurr = aDelta2;
                bPointFound = true;
              }
            }

            if (bPointFound)
            {
              // Check the number of common vertices between edges.
              // If on the other end, there is also a common vertex,
              // check where the intersection point is located. It might
              // be closer to the other vertex than to the current one.
              // And here we just need to close the gap, avoiding the trimming.
              // If the common vertex is only one, do not create the segment,
              // as we have the intersection of the edges and trimmed the 2d curves.
              int aNbCV = 0;
              for (TopoDS_Iterator it1(aPrevEdge); it1.More(); it1.Next())
              {
                for (TopoDS_Iterator it2(aCurrEdge); it2.More(); it2.Next())
                {
                  if (it1.Value().IsSame(it2.Value()))
                    ++aNbCV;
                }
              }

              // Trim PCurves only if the intersection belongs to current parameter
              bool bTrim = (aNbCV == 1 ||
                                        (Abs(aTPrev - aTPrevClosest) < (lp - fp) / 2. ||
                                         Abs(aTCurr - aTCurrClosest) < (lc - fc) / 2.));

              if (bTrim)
              {
                // Check that the intersection point is covered by vertex tolerance
                gp_Pnt2d aPInt = aPrevC2d->Value(aTPrevClosest);
                const gp_Pnt aPOnS = aBASurf.Value(aPInt.X(), aPInt.Y());
                if (aTolV2 > Precision::Infinite() || aPOnS.SquareDistance(aPV) < aTolV2)
                {
                  double f, l;

                  // Trim the curves with found parameters

                  // Prepare trimming parameters for previous curve
                  if (Abs(fp - aTPrev) < Abs(lp - aTPrev))
                  {
                    f = aTPrevClosest;
                    l = lp;
                  }
                  else
                  {
                    f = fp;
                    l = aTPrevClosest;
                  }

                  // Trim previous p-curve
                  if (l - f > Precision::PConfusion())
                    aPrevC2d = new Geom2d_TrimmedCurve(aPrevC2d, f, l);

                  // Prepare trimming parameters for current p-curve
                  if (Abs(fc - aTCurr) < Abs(lc - aTCurr))
                  {
                    f = aTCurrClosest;
                    l = lc;
                  }
                  else
                  {
                    f = fc;
                    l = aTCurrClosest;
                  }

                  // Trim current p-curve
                  if (l - f > Precision::PConfusion())
                    aCurrC2d = new Geom2d_TrimmedCurve(aCurrC2d, f, l);

                  // Do not create the segment, as we performed the trimming
                  // to the intersection point.
                  bAddSegment = false;
                }
              }
            }
          }
        }

        if (bAddSegment)
        {
          // Add segment to the hatcher to trim the iso-lines
          Handle(Geom2d_Line) aLine = new Geom2d_Line(aPrevP2d, aV2d);
          Handle(Geom2d_TrimmedCurve) aLineSegm = new Geom2d_TrimmedCurve(aLine, 0.0, aSegmLen);
          AddElement(Geom2dAdaptor_Curve(aLineSegm), TopAbs_FORWARD);
        }
      }
    }
  }

private:

  double myInfinite;
  double myUMin;
  double myUMax;
  double myVMin;
  double myVMax;
  TColStd_Array1OfReal myUPrm;
  TColStd_Array1OfInteger myUInd;
  TColStd_Array1OfReal myVPrm;
  TColStd_Array1OfInteger myVInd;
  int myNbDom;

};

//-----------------------------------------------------------------------------

namespace
{
  static double IsoRatio     = 1.001;
  static int    MaxPlotCount = 5; // To avoid huge recursive calls in
  static int    PlotCount    = 0; // PlotIso for cases of "bad" curves and surfaces
                                  // Set PlotCount = 0 before first call of PlotIso

  void PlotIso(Handle(DBRep_Face)&  F,
               BRepAdaptor_Surface& S,
               GeomAbs_IsoType      T,
               double&              U,
               double&              V,
               double               Step,
               std::vector<gp_XYZ>& polyline)
  {
    ++PlotCount;
    gp_Pnt Pl, Pr, Pm;

    if (T == GeomAbs_IsoU)
    {
      S.D0(U, V, Pl);
      S.D0(U, V + Step/2., Pm);
      S.D0(U, V + Step, Pr);
    }
    else
    {
      S.D0(U, V, Pl);
      S.D0(U + Step/2., V, Pm);
      S.D0(U + Step, V, Pr);
    }

    if (PlotCount > MaxPlotCount) {
      polyline.push_back( Pr.XYZ() );
      return;
    }

    if (Pm.Distance(Pl) + Pm.Distance(Pr) <= IsoRatio*Pl.Distance(Pr)) {
      polyline.push_back( Pr.XYZ() );
    } else 
       if (T == GeomAbs_IsoU) {
         PlotIso (F, S, T, U, V, Step/2, polyline);
         double aLocalV = V + Step/2 ;
         PlotIso (F, S, T, U, aLocalV, Step/2, polyline);
       } else {
         PlotIso (F, S, T, U, V, Step/2, polyline);
         double aLocalU = U + Step/2 ;
         PlotIso (F, S, T, aLocalU, V, Step/2, polyline);
       }
  }

  void BuildIsos(const TopoDS_Face&                  ff,
                 std::vector< std::vector<gp_XYZ> >& isos)
  {
    TopoDS_Face face = TopoDS::Face( ff.Oriented(TopAbs_FORWARD) );

    GeomAbs_IsoType T;
    double Par, T1, T2, U1, U2, V1, V2, stepU = 0., stepV = 0.;
    gp_Pnt P;
    int i, j;

    DBRep_IsoBuilder IsoBuild(face, 100, 10);

    DBRep_ListOfFace faces;
    Handle(DBRep_Face) F = new DBRep_Face(face, IsoBuild.NbDomains());
    IsoBuild.LoadIsos(F);
    //
    BRepAdaptor_Surface S(F->Face(), false);
    GeomAbs_SurfaceType SurfType = S.GetType();
    GeomAbs_CurveType   CurvType = GeomAbs_OtherCurve;
    //
    const int N = F->NbIsos();
    //
    int myDiscret = 100;
    //
    int Intrv, nbIntv;
    int nbUIntv = S.NbUIntervals(GeomAbs_CN);
    int nbVIntv = S.NbVIntervals(GeomAbs_CN);
    TColStd_Array1OfReal TI(1, Max(nbUIntv, nbVIntv) + 1);
    //
    for ( i = 1; i <= N; ++i )
    {
      std::vector<gp_XYZ> polyline;

      F->GetIso(i,T,Par,T1,T2);
      if (T == GeomAbs_IsoU) {
        S.VIntervals(TI, GeomAbs_CN);
        V1 = Max(T1, TI(1));
        V2 = Min(T2, TI(2));
        U1 = Par;
        U2 = Par;
        stepU = 0;
        nbIntv = nbVIntv;
      }
      else {
        S.UIntervals(TI, GeomAbs_CN);
        U1 = Max(T1, TI(1));
        U2 = Min(T2, TI(2));
        V1 = Par;
        V2 = Par;
        stepV = 0;
        nbIntv = nbUIntv;
      }  
  
      S.D0(U1,V1,P);
      polyline.push_back( P.XYZ() );

       for (Intrv = 1; Intrv <= nbIntv; Intrv++) {

        if (TI(Intrv) <= T1 && TI(Intrv + 1) <= T1)
          continue;
        if (TI(Intrv) >= T2 && TI(Intrv + 1) >= T2)
           continue;
        if (T == GeomAbs_IsoU) {
          V1 = Max(T1, TI(Intrv));
          V2 = Min(T2, TI(Intrv + 1));
          stepV = (V2 - V1) / myDiscret;
        }
        else {
          U1 = Max(T1, TI(Intrv));
          U2 = Min(T2, TI(Intrv + 1));
          stepU = (U2 - U1) / myDiscret;
        }

        switch (SurfType) {
    //-------------GeomAbs_Plane---------------
        case GeomAbs_Plane :
          break;
    //----GeomAbs_Cylinder   GeomAbs_Cone------
        case GeomAbs_Cylinder :
        case GeomAbs_Cone :
          if (T == GeomAbs_IsoV) {
            for (j = 1; j < myDiscret; j++) {
        U1 += stepU;
        V1 += stepV;
        S.D0(U1,V1,P);
        polyline.push_back( P.XYZ() );
            }
          }
          break;
    //---GeomAbs_Sphere   GeomAbs_Torus--------
    //GeomAbs_BezierSurface GeomAbs_BezierSurface
        case GeomAbs_Sphere :
        case GeomAbs_Torus :
        case GeomAbs_OffsetSurface :
        case GeomAbs_OtherSurface :
          for (j = 1; j < myDiscret; j++) {
            U1 += stepU;
            V1 += stepV;
            S.D0(U1,V1,P);
            polyline.push_back( P.XYZ() );
          }
          break;
    //-------------GeomAbs_BSplineSurface------
        case GeomAbs_BezierSurface :
        case GeomAbs_BSplineSurface :
          for (j = 1; j <= myDiscret/2; j++) {
            Handle(DBRep_Face) aLocalFace = F;

            PlotCount = 0;

            PlotIso (aLocalFace , S, T, U1, V1, (T == GeomAbs_IsoV) ? stepU*2. : stepV*2., polyline);
            U1 += stepU*2.;
            V1 += stepV*2.;
          }
          break;
    //-------------GeomAbs_SurfaceOfExtrusion--
    //-------------GeomAbs_SurfaceOfRevolution-
        case GeomAbs_SurfaceOfExtrusion :
        case GeomAbs_SurfaceOfRevolution :
          if ((T == GeomAbs_IsoV && SurfType == GeomAbs_SurfaceOfRevolution) ||
        (T == GeomAbs_IsoU && SurfType == GeomAbs_SurfaceOfExtrusion)) {
            if (SurfType == GeomAbs_SurfaceOfExtrusion) break;
            for (j = 1; j < myDiscret; j++) {
        U1 += stepU;
        V1 += stepV;
        S.D0(U1,V1,P);
        polyline.push_back( P.XYZ() );
            }
          } else {
            CurvType = (S.BasisCurve())->GetType();
            switch (CurvType) {
            case GeomAbs_Line :
        break;
            case GeomAbs_Circle :
            case GeomAbs_Ellipse :
        for (j = 1; j < myDiscret; j++) {
          U1 += stepU;
          V1 += stepV;
          S.D0(U1,V1,P);
          polyline.push_back( P.XYZ() );
        }
        break;
            case GeomAbs_Parabola :
            case GeomAbs_Hyperbola :
            case GeomAbs_BezierCurve :
            case GeomAbs_BSplineCurve :
            case GeomAbs_OffsetCurve :
            case GeomAbs_OtherCurve :
        for (j = 1; j <= myDiscret/2; j++) {
          Handle(DBRep_Face) aLocalFace = F;

          PlotCount = 0;

          PlotIso (aLocalFace, S, T, U1, V1,
             (T == GeomAbs_IsoV) ? stepU*2. : stepV*2., polyline);
          U1 += stepU*2.;
          V1 += stepV*2.;
        }
        break;
            }
          }
        }
      }
      S.D0(U2,V2,P);
      polyline.push_back( P.XYZ() );

      isos.push_back(polyline);
    }
  }
}

//-----------------------------------------------------------------------------

//! Creates new Hatching Pipeline initialized by default VTK mapper and actor.
asiVisu_HatchingPipeline::asiVisu_HatchingPipeline()
//
: asiVisu_Pipeline( vtkSmartPointer<vtkPolyDataMapper>::New(),
                    vtkSmartPointer<vtkActor>::New() )
{
}

//-----------------------------------------------------------------------------

//! Sets input data for the pipeline.
//! \param[in] DP Data Provider.
void asiVisu_HatchingPipeline::SetInput(const Handle(asiVisu_DataProvider)& DP)
{
  Handle(asiVisu_HatchingDataProvider)
    faceProvider = Handle(asiVisu_HatchingDataProvider)::DownCast(DP);

  /* =================
   *  Validate inputs
   * ================= */

  TopoDS_Face face = faceProvider->GetFace();
  if ( face.IsNull() )
  {
    // Pass empty data set in order to have valid pipeline
    vtkSmartPointer<vtkPolyData> dummyDS = vtkSmartPointer<vtkPolyData>::New();
    this->SetInputData(dummyDS);
    this->Modified(); // Update modification timestamp
    return; // Do nothing
  }

  /* ============================
   *  Prepare polygonal data set
   * ============================ */

  if ( faceProvider->MustExecute( this->GetMTime() ) )
  {
    std::vector< std::vector<gp_XYZ> > isos;
    ::BuildIsos( faceProvider->GetFace(), isos );

    // Append filter
    vtkSmartPointer<vtkAppendPolyData>
      appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();

    for ( const auto& iso : isos )
    {
      // Allocate Data Source
      vtkSmartPointer<asiVisu_CurveSource>
        curveSource = vtkSmartPointer<asiVisu_CurveSource>::New();

      // Curve representation
      Handle(Geom_Curve) isoCurve = asiAlgo_Utils::PolylineAsSpline(iso);

      // Set geometry to be converted to VTK polygonal DS
      if ( !curveSource->SetInputCurve( isoCurve, isoCurve->FirstParameter(), isoCurve->LastParameter() ) )
        continue; // No poly data produced

      // Append poly data
      appendFilter->AddInputConnection( curveSource->GetOutputPort() );
    }

    // Chain pipeline
    this->SetInputConnection( appendFilter->GetOutputPort() );
  }

  // Update modification timestamp
  this->Modified();
}

//-----------------------------------------------------------------------------

//! Callback for AddToRenderer() routine. Good place to adjust visualization
//! properties of the pipeline's actor.
void asiVisu_HatchingPipeline::callback_add_to_renderer(vtkRenderer*)
{}

//! Callback for RemoveFromRenderer() routine.
void asiVisu_HatchingPipeline::callback_remove_from_renderer(vtkRenderer*)
{}

//! Callback for Update() routine.
void asiVisu_HatchingPipeline::callback_update()
{}
