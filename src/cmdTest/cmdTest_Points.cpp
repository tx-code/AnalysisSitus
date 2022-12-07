//-----------------------------------------------------------------------------
// Created on: 25 June 2022
// Created by: Andrey Voevodin
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

// cmdTest includes
#include <cmdTest.h>

// asiEngine includes
#include <asiEngine_Model.h>

// asiAlgo includes
#include <asiAlgo_PointWithAttr.h>
#include <asiAlgo_PurifyCloud.h>
#include <asiAlgo_QuickHull2d.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// OCCT includes
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <gp_XY.hxx>
#include <gp_XYZ.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopoDS_Wire.hxx>

//-----------------------------------------------------------------------------

int TEST_BuildQuickHull(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  gp_Pnt loc(std::atof(argv[2]), std::atof(argv[3]), std::atof(argv[4]));
  gp_Dir dir(std::atof(argv[5]), std::atof(argv[6]), std::atof(argv[7]));

  Handle(Geom_Plane) plane = new Geom_Plane(loc, dir);
  ShapeAnalysis_Surface sas(plane);

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast(interp->GetModel()->FindNodeByName(argv[1]));
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints();
  if ( pts.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No points.");
    return TCL_ERROR;
  }

  // Get output points.
  Handle(asiAlgo_PointWithAttrCloud<gp_XY>) cloud = new asiAlgo_PointWithAttrCloud<gp_XY>;
  //
  for ( int ipt = 0; ipt < pts->GetNumberOfElements(); ++ipt )
  {
    gp_Pnt pnt = pts->GetElement(ipt);

    gp_Pnt proj;
    if ( !asiAlgo_Utils::ProjectPointOnPlane(plane,
                                             dir, pnt, proj) )
    {
      continue;
    }

    gp_XY pnt2d = sas.ValueOfUV(proj, 1.0e-4).XY();

    cloud->AddElement(asiAlgo_PointWithAttr<gp_XY>(pnt2d, 0, ipt));
  }

  ///
  Handle(asiAlgo_PointWithAttrCloud<gp_XY>) sparsedCloud;

  // Sparse cloud using the appropriate type of inspector.
  asiAlgo_PurifyCloud sparser(interp->GetProgress(), interp->GetPlotter());
  //
  sparser.PerformCommon< asiAlgo_PointWithAttrCloud<gp_XY>,
    asiAlgo_PointWithAttrInspector2d<gp_XY> >(0.01, cloud, sparsedCloud);

  ///
  asiAlgo_QuickHull2d<gp_XY> qHull(sparsedCloud, interp->GetProgress(), interp->GetPlotter());
  //
  if ( !qHull.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Quick hull failed.");
    return TCL_ERROR;
  }

  // Get hull indices.
  const Handle(TColStd_HSequenceOfInteger)& hull = qHull.GetHull();

  // Make polygon
  std::vector<gp_XYZ> chPoints;
  BRepBuilderAPI_MakePolygon mkPolygon;
  for ( int hidx = hull->Lower(); hidx <= hull->Upper(); ++hidx )
  {
    gp_Pnt pnt = plane->Value(sparsedCloud->GetElement(hull->Value(hidx)).Coord.X(),
                              sparsedCloud->GetElement(hull->Value(hidx)).Coord.Y());

    mkPolygon.Add(pnt);

    chPoints.push_back(pnt.XYZ());
  }
  //
  mkPolygon.Close(); // Polygon should be closed
  mkPolygon.Build();
  //
  const TopoDS_Wire& W = mkPolygon.Wire();

  interp->GetPlotter().REDRAW_SHAPE("hull", W, Color_Green, 1., true);
  interp->GetPlotter().REDRAW_POINTS("hull-points", chPoints, Color_Green);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdTest::Commands_Points(const Handle(asiTcl_Interp)&      interp,
                              const Handle(Standard_Transient)& cmdTest_NotUsed(data))
{
  static const char* group = "cmdTest";

  interp->AddCommand("test-build-quick-hull",
    //
    "test-build-quick-hull <pointsName> <loc_x> <loc_y> <loc_z> <dir_x> <dir_y> <dir_z>\n"
    "\t Build quick hull.",
    //
    __FILE__, group, TEST_BuildQuickHull);
}
