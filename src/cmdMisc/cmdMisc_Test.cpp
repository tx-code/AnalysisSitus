//-----------------------------------------------------------------------------
// Created on: 14 May 2022 (*)
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

// cmdMisc includes
#include <cmdMisc.h>

// OpenCascade includes
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <TopExp_Explorer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <IntTools_EdgeFace.hxx>

#include <asiEngine_Part.h>

#include <Geom2d_Circle.hxx>
#include <GCE2d_MakeCircle.hxx>
#include <GeomAPI.hxx>
#include <gp_Pln.hxx>
#include <gp_Quaternion.hxx>
#include <BRepTools_WireExplorer.hxx>

//-----------------------------------------------------------------------------

#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <asiAlgo_DivideByContinuity.h>

#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_BuildConvexHull.h>
#include <asiAlgo_QuickHull2d.h>

#if defined USE_MOBIUS
  #include <mobius/poly_Mesh.h>
  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          cmdMisc_NotUsed(argc),
              const char**                 argv)
{
  t_ptr<poly_Mesh> mobMesh = new poly_Mesh;
  mobMesh->AddVertex(0,0,0);

  //Handle(Geom_Plane) drawingPlane = new Geom_Plane( gp::Origin(), gp::DZ() );

  //std::vector<gp_XY> pts = {
  //  gp_XY(1108.8858916194, 355),
  //  gp_XY(1258.8858916194, 355),
  //  gp_XY(1183.8858916194, 382.5),
  //  gp_XY(1258.8858916194, 382.5),
  //  gp_XY(1258.9377598936, 115.5),
  //  gp_XY(1058.8858916194, 231),
  //  gp_XY(1108.8858916194, 382.5),
  //  gp_XY(1258.8858916194, 350),
  //  gp_XY(1108.8858916194, 350),
  //  gp_XY(858.8340233452, 115.5),
  //  gp_XY(836.3858916194, 615),
  //  gp_XY(858.8340233452, 0),
  //  gp_XY(836.3858916194, 130),
  //  gp_XY(836.3858916194, 392.5),
  //  gp_XY(831.3858916194, 50),
  //  gp_XY(808.8858916194, 0),
  //  gp_XY(808.8858916194, 650),
  //  gp_XY(1008.8858916194, 630),
  //  gp_XY(1108.8858916194, 630),
  //  gp_XY(1308.8858916194, 650),
  //  gp_XY(1308.8858916194, 0),
  //  gp_XY(1258.9377598936, 0) };

  //Handle(asiAlgo_BaseCloud<double>) pcloud = new asiAlgo_BaseCloud<double>;
  ////
  //Handle(asiAlgo_PointWithAttrCloud<gp_XY>)
  //  cloud = new asiAlgo_PointWithAttrCloud<gp_XY>;
  ////
  //for ( const auto& pt : pts )
  //{
  //  cloud ->AddElement( pt );
  //  pcloud->AddElement( drawingPlane->Value( pt.X(), pt.Y() ) );
  //}

  //interp->GetPlotter().REDRAW_POINTS("pcloud", pcloud->GetCoordsArray(), Color_White);

  //asiAlgo_QuickHull2d<gp_XY> qHull( cloud,
  //                                  interp->GetProgress(),
  //                                  interp->GetPlotter() );

  //if ( !qHull.Perform() )
  //{
  //  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot build convex hull.");
  //  return TCL_ERROR;
  //}

  //const Handle(TColStd_HSequenceOfInteger)& hull = qHull.GetHull();

  //// Make polygon.
  //BRepBuilderAPI_MakePolygon mkPolygon;
  ////
  //for ( int hidx = hull->Lower(); hidx <= hull->Upper(); ++hidx )
  //{
  //  const int index = hull->Value(hidx);
  //  gp_XY     xy    = cloud->GetElement(index).Coord;

  //  mkPolygon.Add( drawingPlane->Value( xy.X(), xy.Y() ) );
  //}
  ////
  //mkPolygon.Close(); // Polygon should be closed
  //mkPolygon.Build();
  ////
  //const TopoDS_Wire& W = mkPolygon.Wire();

  //interp->GetPlotter().REDRAW_SHAPE("convexHull", W, Color_Green, 1., true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int MISC_TestBottle(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  /* OpenCascade tutorial from
     https://dev.opencascade.org/doc/overview/html/occt__tutorial.html
   */

  Standard_Real myWidth = 10;
  interp->GetKeyValue(argc, argv, "w", myWidth);

  Standard_Real myHeight = 15;
  interp->GetKeyValue(argc, argv, "h", myHeight);

  Standard_Real myThickness = 5;
  interp->GetKeyValue(argc, argv, "t", myThickness);

  // Profile : Define Support Points
  gp_Pnt aPnt1(-myWidth / 2., 0, 0);        
  gp_Pnt aPnt2(-myWidth / 2., -myThickness / 4., 0);
  gp_Pnt aPnt3(0, -myThickness / 2., 0);
  gp_Pnt aPnt4(myWidth / 2., -myThickness / 4., 0);
  gp_Pnt aPnt5(myWidth / 2., 0, 0);
 
  // Profile : Define the Geometry
  Handle(Geom_TrimmedCurve) anArcOfCircle = GC_MakeArcOfCircle(aPnt2,aPnt3,aPnt4);
  Handle(Geom_TrimmedCurve) aSegment1 = GC_MakeSegment(aPnt1, aPnt2);
  Handle(Geom_TrimmedCurve) aSegment2 = GC_MakeSegment(aPnt4, aPnt5);
 
  // Profile : Define the Topology
  TopoDS_Edge anEdge1 = BRepBuilderAPI_MakeEdge(aSegment1);
  TopoDS_Edge anEdge2 = BRepBuilderAPI_MakeEdge(anArcOfCircle);
  TopoDS_Edge anEdge3 = BRepBuilderAPI_MakeEdge(aSegment2);
  TopoDS_Wire aWire  = BRepBuilderAPI_MakeWire(anEdge1, anEdge2, anEdge3);
 
  // Complete Profile
  gp_Ax1 xAxis = gp::OX();
  gp_Trsf aTrsf;
 
  aTrsf.SetMirror(xAxis);
  BRepBuilderAPI_Transform aBRepTrsf(aWire, aTrsf);
  TopoDS_Shape aMirroredShape = aBRepTrsf.Shape();
  TopoDS_Wire aMirroredWire = TopoDS::Wire(aMirroredShape);
 
  BRepBuilderAPI_MakeWire mkWire;
  mkWire.Add(aWire);
  mkWire.Add(aMirroredWire);
  TopoDS_Wire myWireProfile = mkWire.Wire();
 
  // Body : Prism the Profile
  TopoDS_Face myFaceProfile = BRepBuilderAPI_MakeFace(myWireProfile);
  gp_Vec aPrismVec(0, 0, myHeight);
  TopoDS_Shape myBody = BRepPrimAPI_MakePrism(myFaceProfile, aPrismVec);
 
  // Body : Apply Fillets
  BRepFilletAPI_MakeFillet mkFillet(myBody);
  TopExp_Explorer anEdgeExplorer(myBody, TopAbs_EDGE);
  while(anEdgeExplorer.More()){
      TopoDS_Edge anEdge = TopoDS::Edge(anEdgeExplorer.Current());
      //Add edge to fillet algorithm
      mkFillet.Add(myThickness / 12., anEdge);
      anEdgeExplorer.Next();
  }
 
  myBody = mkFillet.Shape();
 
  // Body : Add the Neck
  gp_Pnt neckLocation(0, 0, myHeight);
  gp_Dir neckAxis = gp::DZ();
  gp_Ax2 neckAx2(neckLocation, neckAxis);
 
  Standard_Real myNeckRadius = myThickness / 4.;
  Standard_Real myNeckHeight = myHeight / 10.;
 
  BRepPrimAPI_MakeCylinder MKCylinder(neckAx2, myNeckRadius, myNeckHeight);
  TopoDS_Shape myNeck = MKCylinder.Shape();
 
  myBody = BRepAlgoAPI_Fuse(myBody, myNeck);
 
  // Body : Create a Hollowed Solid
  TopoDS_Face   faceToRemove;
  Standard_Real zMax = -1;
 
  for(TopExp_Explorer aFaceExplorer(myBody, TopAbs_FACE); aFaceExplorer.More(); aFaceExplorer.Next()){
      TopoDS_Face aFace = TopoDS::Face(aFaceExplorer.Current());
      // Check if <aFace> is the top face of the bottle's neck 
      Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
      if(aSurface->DynamicType() == STANDARD_TYPE(Geom_Plane)){
          Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aSurface);
          gp_Pnt aPnt = aPlane->Location();
          Standard_Real aZ   = aPnt.Z();
          if(aZ > zMax){
              zMax = aZ;
              faceToRemove = aFace;
          }
      }
  }
 
  TopTools_ListOfShape facesToRemove;
  facesToRemove.Append(faceToRemove);
  BRepOffsetAPI_MakeThickSolid aSolidMaker;
  aSolidMaker.MakeThickSolidByJoin(myBody, facesToRemove, -myThickness / 50, 1.e-3);
  myBody = aSolidMaker.Shape();
  // Threading : Create Surfaces
  Handle(Geom_CylindricalSurface) aCyl1 = new Geom_CylindricalSurface(neckAx2, myNeckRadius * 0.99);
  Handle(Geom_CylindricalSurface) aCyl2 = new Geom_CylindricalSurface(neckAx2, myNeckRadius * 1.05);
 
  // Threading : Define 2D Curves
  gp_Pnt2d aPnt(2. * M_PI, myNeckHeight / 2.);
  gp_Dir2d aDir(2. * M_PI, myNeckHeight / 4.);
  gp_Ax2d anAx2d(aPnt, aDir);
 
  Standard_Real aMajor = 2. * M_PI;
  Standard_Real aMinor = myNeckHeight / 10;
 
  Handle(Geom2d_Ellipse) anEllipse1 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor);
  Handle(Geom2d_Ellipse) anEllipse2 = new Geom2d_Ellipse(anAx2d, aMajor, aMinor / 4);
  Handle(Geom2d_TrimmedCurve) anArc1 = new Geom2d_TrimmedCurve(anEllipse1, 0, M_PI);
  Handle(Geom2d_TrimmedCurve) anArc2 = new Geom2d_TrimmedCurve(anEllipse2, 0, M_PI);
  gp_Pnt2d anEllipsePnt1 = anEllipse1->Value(0);
  gp_Pnt2d anEllipsePnt2 = anEllipse1->Value(M_PI);
 
  Handle(Geom2d_TrimmedCurve) aSegment = GCE2d_MakeSegment(anEllipsePnt1, anEllipsePnt2);
  // Threading : Build Edges and Wires
  TopoDS_Edge anEdge1OnSurf1 = BRepBuilderAPI_MakeEdge(anArc1, aCyl1);
  TopoDS_Edge anEdge2OnSurf1 = BRepBuilderAPI_MakeEdge(aSegment, aCyl1);
  TopoDS_Edge anEdge1OnSurf2 = BRepBuilderAPI_MakeEdge(anArc2, aCyl2);
  TopoDS_Edge anEdge2OnSurf2 = BRepBuilderAPI_MakeEdge(aSegment, aCyl2);
  TopoDS_Wire threadingWire1 = BRepBuilderAPI_MakeWire(anEdge1OnSurf1, anEdge2OnSurf1);
  TopoDS_Wire threadingWire2 = BRepBuilderAPI_MakeWire(anEdge1OnSurf2, anEdge2OnSurf2);
  BRepLib::BuildCurves3d(threadingWire1);
  BRepLib::BuildCurves3d(threadingWire2);
 
  // Create Threading 
  BRepOffsetAPI_ThruSections aTool(Standard_True);
  aTool.AddWire(threadingWire1);
  aTool.AddWire(threadingWire2);
  aTool.CheckCompatibility(Standard_False);
 
  TopoDS_Shape myThreading = aTool.Shape();
 
  // Building the Resulting Compound 
  TopoDS_Compound aRes;
  BRep_Builder aBuilder;
  aBuilder.MakeCompound (aRes);
  aBuilder.Add (aRes, myBody);
  aBuilder.Add (aRes, myThreading);

  interp->GetPlotter().REDRAW_SHAPE("bottle", aRes);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Test(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("test",        "Test anything.", __FILE__, group, MISC_Test);
  interp->AddCommand("test-bottle", "Test bottle.",   __FILE__, group, MISC_TestBottle);
}
