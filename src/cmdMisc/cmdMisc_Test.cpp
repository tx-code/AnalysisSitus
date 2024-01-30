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

#include <BRepTools.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <GeomConvert.hxx>
#include <ShapeFix_Wire.hxx>
#include <BOPAlgo_Builder.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>

#include <IGESControl_Reader.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Builder.hxx>
#include <ShapeAnalysis_Curve.hxx>

#include <GC_MakeArcOfCircle.hxx>

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          /*argc*/,
              const char**                 /*argv*/)
{
  gp_Pnt p1 (0, 0, 0);
  gp_Pnt p2(2, 2, 0);
  gp_Pnt center(2, 0, 0);
  gp_Circ circ(gp_Ax2(center, gp_Dir(0, 0, 1)), 2);
  gp_Circ circ2(gp_Ax2(center, gp_Dir(0, 0, 1)), 3);

  Handle(Geom_TrimmedCurve) anArcOfCircle =  GC_MakeArcOfCircle(circ, p2, p1, true);
  Handle(Geom_TrimmedCurve) anArcOfCircle2 = GC_MakeArcOfCircle(circ2, p2, p1, false);

  interp->GetPlotter().REDRAW_CURVE("c1", anArcOfCircle,  Color_Red, true);
  interp->GetPlotter().REDRAW_CURVE("c2", anArcOfCircle2, Color_Red, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Test(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("test", "Test anything.", __FILE__, group, MISC_Test);
}
