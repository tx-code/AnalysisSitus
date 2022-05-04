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

#ifndef asiAlgo_DiscrEdge_HeaderFile
#define asiAlgo_DiscrEdge_HeaderFile

#ifdef DEB
#include <Standard_NullObject.hxx>
#endif

// asiAlgo includes
#include <asiAlgo_DiscrFace.h>
#include <asiAlgo_DiscrCurve.h>
#include <asiAlgo_DiscrCurve2d.h>
#include <asiAlgo_DiscrSequenceOfPointer.h>

// OpenCascade includes
#include <TColStd_SequenceOfReal.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {
namespace discr {

class Wire;
class Vertex;

//! Discrete edge consisting of a discrete 3D curve and one or two discrete pcurves for a face (2 pcurves
//! for a seam edge on a closed surface). Each pcurve of a seam edge is associated with the edge's orientation.
//! All the pcurves are synchronized with the 3D curve by number of points and their geometric placement.
//! If an edge is DEGENERATED, its 3D curve consists of two equal points and it has one pcurve on one face only.
//! A non-degenerated edge may participate in several FRONTs. A front is a wire representing a free boundary of
//! a shell ora sharp edges sequence or whatever user defines.
class Edge
{
public:
  // ---------- CONSTRUCTORS ----------

  Edge () :
    myFirstVertex(0),
    myLastVertex(0),
    myDegenerated(false)
  {}
  // Empty constructor

  asiAlgo_EXPORT virtual ~Edge () {DestroyCurves();}

  // ---------- INITIALISATION ---------

  asiAlgo_EXPORT void Init ();
  // Init edge as having non-zero 3D length

  asiAlgo_EXPORT void InitDegenerated (const Face& aFace);
  // Init edge as degenerated

  asiAlgo_EXPORT int AddPCurve
    (const Face& aFace, const bool isEforward);
  // Add pcurve on a given face; returns index of new pcurve

  void SetFirstVertex (const Vertex& theVertex)
  { myFirstVertex = (Vertex*)&theVertex; }
  // Sets the first vertex

  void SetLastVertex (const Vertex& theVertex)
  { myLastVertex = (Vertex*)&theVertex; }
  // Sets the last vertex

  asiAlgo_EXPORT void AddFront (const Wire& aFront);
  // Add a front this edge participate in.

  void Nullify () { DestroyCurves(); }
  // Brings me to be empty

  // ---------- ACCESS TO CURVES ---------

  const Curve& GetCurve () const {return myCurve;}
  // Read-only access to 3D curve

  Curve& ChangeCurve () {return myCurve;}
  // Read-write access to 3D curve

  const TColStd_SequenceOfReal& GetParams () const {return myParams;}
  // Read-only access to parameters of points on curve from shape

  TColStd_SequenceOfReal& ChangeParams () {return myParams;}
  // Read-write access to parameters of points on curve from shape

  int GetNbPCurves () const {
    return myPCurves.Size();
  }
  // Returns the overall number of pcurves

  const Curve2d& GetPCurve (const int theIndex) const
  {
    #if !defined No_Exception && !defined No_Standard_OutOfRange
      if (theIndex < 1 || theIndex > GetNbPCurves())
        throw Standard_OutOfRange("asiAlgo_DiscrEdge::PCurve");
    #endif

    ListOfPointer::Iterator anIt(myPCurves);
    for(int i = 1; i != theIndex; anIt.Next(), ++i);

    return *(Curve2d * const &) anIt.Value();
  }
  // Read-only access to pcurve by index

  Curve2d& ChangePCurve (const int theIndex)
  {
    #if !defined No_Exception && !defined No_Standard_OutOfRange
      if (theIndex < 1 || theIndex > GetNbPCurves())
        throw Standard_OutOfRange("asiAlgo_DiscrEdge::ChangePCurve");
    #endif

    ListOfPointer::Iterator anIt(myPCurves);
    for(int i = 1; i != theIndex; anIt.Next(), ++i);

    return *(Curve2d *) anIt.Value();
  }
  // Read-write access to pcurve by index

  bool IsForward (const int theIndex) const
  {
    if(theIndex < 2) return true;

    const Curve2d& aCurrent = GetPCurve(theIndex);
    const Curve2d& aPrevious = GetPCurve(theIndex - 1);

    return !(aCurrent.GetFace() == aPrevious.GetFace());
  }
  // Edge orientation by PCurve index

  asiAlgo_EXPORT void RemovePCurve (const int theIndex);
  // Removes pcurve pointed by index

  const Curve2d& GetPCurve(const Face& theFace,
                           const bool isEforward) const
  {
    return GetPCurve( FindPCurve(theFace,isEforward) );
  }
  // Returns pcurve on aFace (see comments to FindPCurve)

  Curve2d& ChangePCurve
    (const Face& theFace, const bool isEforward)
    { return ChangePCurve (FindPCurve (theFace,isEforward)); }
  // Non-const variant of the above method

  asiAlgo_EXPORT int FindPCurve
    (const Face& theFace, const bool isEforward) const;
  // Returns the index of pcurve on aFace.
  // If the edge is not a seem (has 1 pcurve on aFace) orientation does not
  // matter, otherwise returns the pcurve corresponding to the flag isEforward

  asiAlgo_EXPORT bool 
    FindAdjacentFace(const bool isEForward,
                     Face const*&    theOutFace,
                     const bool useFaceOri = true) const;
  // Finds a face that contains this edge with the given orientation.
  // If useFaceOri is True then the orientation of a checked face is
  // considered.
  // Returns True if a face is found, False otherwise

  // ---------- ACCESS TO VERTICES -------

  const Vertex& FirstVertex() const {
#ifdef DEB
    if (!myFirstVertex)
      throw Standard_NullObject("asiAlgo_DiscrEdge::FirstVertex");
#endif
    return *myFirstVertex;
  }
  // Returns the first vertex

  const Vertex& LastVertex() const {
#ifdef DEB
    if (!myLastVertex)
      throw Standard_NullObject("asiAlgo_DiscrEdge::LastVertex");
#endif
    return *myLastVertex;
  }
  // Returns the last vertex

  Vertex& ChangeFirstVertex() {
#ifdef DEB
    if (!myFirstVertex)
      throw Standard_NullObject("asiAlgo_DiscrEdge::FirstVertex");
#endif
    return *myFirstVertex;
  }
  // Returns the first vertex

  Vertex& ChangeLastVertex() {
#ifdef DEB
    if (!myLastVertex)
      throw Standard_NullObject("asiAlgo_DiscrEdge::LastVertex");
#endif
    return *myLastVertex;
  }
  // Returns the last vertex

  // ---------- ACCESS TO FRONTS -------

  int GetNbFronts () const {return myFronts.Length();}
  // Returns the number of fronts this edge participate in

  const Wire& GetFront (const int aIndex) const
  {return *(Wire*)myFronts(aIndex);}
  // Returns a front by index

  asiAlgo_EXPORT void RemoveFront (const int aIndex);
  // Removes a front by index

  // ---------- CONSULTING -------------

  bool IsNull () const {return myCurve.IsEmpty() && myPCurves.IsEmpty();}
  // Returns true if this edge has not been yet initialized

  bool IsDegenerated () const {return myDegenerated;}
  // Degenerated flag

  bool operator == (const Edge& aOther) const
  { return this == &aOther; }

 protected:

  asiAlgo_EXPORT void DestroyCurves ();

 private:

  Edge (const Edge&);
  // Hides copy constructor

  Edge& operator = (const Edge&);
  // Hides assignment operator

  // ---------- PRIVATE FIELDS ----------

  Vertex                *myFirstVertex;
  Vertex                *myLastVertex;
  SequenceOfPointer      myFronts;
  Curve                  myCurve;
  TColStd_SequenceOfReal myParams;
  bool                   myDegenerated;
  ListOfPointer          myPCurves;
};

}
}

#endif
