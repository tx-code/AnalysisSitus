//-----------------------------------------------------------------------------
// Created on: 13 February 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

#ifndef asiAlgo_NurbsConvertModification_HeaderFile
#define asiAlgo_NurbsConvertModification_HeaderFile

#include <asiAlgo.h>

#include <TopTools_ListOfShape.hxx>
#include <TColStd_ListOfTransient.hxx>
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <BRepTools_Modification.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Real.hxx>
#include <GeomAbs_Shape.hxx>
class TopoDS_Face;
class Geom_Surface;
class TopLoc_Location;
class TopoDS_Edge;
class Geom_Curve;
class TopoDS_Vertex;
class gp_Pnt;
class Geom2d_Curve;


class asiAlgo_NurbsConvertModification;
DEFINE_STANDARD_HANDLE(asiAlgo_NurbsConvertModification, BRepTools_Modification)

//! Defines a modification of the  geometry by a  Trsf
//! from gp. All methods return True and transform the
//! geometry.
class asiAlgo_NurbsConvertModification : public BRepTools_Modification
{

public:

  
  Standard_EXPORT asiAlgo_NurbsConvertModification();
  
  //! Returns Standard_True  if  the face  <F> has  been
  //! modified.  In this  case, <S> is the new geometric
  //! support of  the  face, <L> the  new location,<Tol>
  //! the new  tolerance.<RevWires> has  to  be set   to
  //! Standard_True   when the modification reverses the
  //! normal of  the   surface.(the wires   have  to  be
  //! reversed).   <RevFace>   has   to   be   set    to
  //! Standard_True if  the orientation  of the modified
  //! face changes in the  shells which contain  it.  --
  //! Here, <RevFace>  will  return Standard_True if the
  //! -- gp_Trsf is negative.
  Standard_EXPORT Standard_Boolean NewSurface (const TopoDS_Face& F, Handle(Geom_Surface)& S, TopLoc_Location& L, Standard_Real& Tol, Standard_Boolean& RevWires, Standard_Boolean& RevFace) Standard_OVERRIDE;
  
  //! Returns Standard_True  if  the edge  <E> has  been
  //! modified.  In this case,  <C> is the new geometric
  //! support of the  edge, <L> the  new location, <Tol>
  //! the         new    tolerance.   Otherwise, returns
  //! Standard_False,    and  <C>,  <L>,   <Tol> are not
  //! significant.
  Standard_EXPORT Standard_Boolean NewCurve (const TopoDS_Edge& E, Handle(Geom_Curve)& C, TopLoc_Location& L, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns  Standard_True if the  vertex <V> has been
  //! modified.  In this  case, <P> is the new geometric
  //! support of the vertex,   <Tol> the new  tolerance.
  //! Otherwise, returns Standard_False, and <P>,  <Tol>
  //! are not significant.
  Standard_EXPORT Standard_Boolean NewPoint (const TopoDS_Vertex& V, gp_Pnt& P, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns Standard_True if  the edge  <E> has a  new
  //! curve on surface on the face <F>.In this case, <C>
  //! is the new geometric support of  the edge, <L> the
  //! new location, <Tol> the new tolerance.
  //! Otherwise, returns  Standard_False, and <C>,  <L>,
  //! <Tol> are not significant.
  Standard_EXPORT Standard_Boolean NewCurve2d (const TopoDS_Edge& E, const TopoDS_Face& F, const TopoDS_Edge& NewE, const TopoDS_Face& NewF, Handle(Geom2d_Curve)& C, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns Standard_True if the Vertex  <V> has a new
  //! parameter on the  edge <E>. In  this case,  <P> is
  //! the parameter,    <Tol>  the     new    tolerance.
  //! Otherwise, returns Standard_False, and <P>,  <Tol>
  //! are not significant.
  Standard_EXPORT Standard_Boolean NewParameter (const TopoDS_Vertex& V, const TopoDS_Edge& E, Standard_Real& P, Standard_Real& Tol) Standard_OVERRIDE;
  
  //! Returns the  continuity of  <NewE> between <NewF1>
  //! and <NewF2>.
  //!
  //! <NewE> is the new  edge created from <E>.  <NewF1>
  //! (resp. <NewF2>) is the new  face created from <F1>
  //! (resp. <F2>).
  Standard_EXPORT GeomAbs_Shape Continuity (const TopoDS_Edge& E, const TopoDS_Face& F1, const TopoDS_Face& F2, const TopoDS_Edge& NewE, const TopoDS_Face& NewF1, const TopoDS_Face& NewF2) Standard_OVERRIDE;

  Standard_EXPORT const TopTools_ListOfShape& GetUpdatedEdges() const;



  DEFINE_STANDARD_RTTIEXT(asiAlgo_NurbsConvertModification,BRepTools_Modification)

protected:




private:


  TopTools_ListOfShape myled;
  TColStd_ListOfTransient mylcu;
  TColStd_IndexedDataMapOfTransientTransient myMap;
  TopTools_ListOfShape myUpdatedEdges;


};

#endif
