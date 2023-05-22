//-----------------------------------------------------------------------------
// Created on: 12 May 2023
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

// Own include
#include <asiAlgo_IntersectCurves.h>

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>

// OpenCascade includes
#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_ListOfShape.hxx>

#define GeneralFuseTol 0.1

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

namespace
{
  TopoDS_Shape BooleanGeneralFuse(const TopTools_ListOfShape& objects,
                                  const double                fuzz,
                                  BOPAlgo_Builder&            API,
                                  const bool                  glue)
  {
    const bool bRunParallel = false;

    BOPAlgo_PaveFiller DSFiller;
    DSFiller.SetArguments(objects);
    DSFiller.SetRunParallel(bRunParallel);
    DSFiller.SetFuzzyValue(fuzz);
    DSFiller.Perform();
    bool hasErr = DSFiller.HasErrors();
    //
    if ( hasErr )
    {
      return TopoDS_Shape();
    }

    if ( glue )
      API.SetGlue(BOPAlgo_GlueFull);

    API.SetArguments(objects);
    API.SetRunParallel(bRunParallel);
    API.PerformWithFiller(DSFiller);
    hasErr = API.HasErrors();
    //
    if ( hasErr )
    {
      return TopoDS_Shape();
    }

    return API.Shape();
  }
}

//-----------------------------------------------------------------------------

asiAlgo_IntersectCurves::asiAlgo_IntersectCurves(ActAPI_ProgressEntry progress,
                                                 ActAPI_PlotterEntry   plotter)
: ActAPI_IAlgorithm(progress, plotter)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_IntersectCurves::Perform(const std::vector<Handle(Geom_BSplineCurve)>& uCurves,
                                      const std::vector<Handle(Geom_BSplineCurve)>& vCurves,
                                      math_Matrix&                                  uParams,
                                      math_Matrix&                                  vParams)
{
  std::vector<TopoDS_Edge> uEdges, vEdges;

  // Prepare arguments for a Boolean operation (General Fuse).
  TopTools_ListOfShape args;
  //
  for ( const auto& c : uCurves )
  {
    uEdges.push_back( BRepBuilderAPI_MakeEdge(c) );
  }
  //
  for ( const auto& c : vCurves )
  {
    vEdges.push_back( BRepBuilderAPI_MakeEdge(c) );
  }
  //
  for ( const auto& edge : uEdges )
  {
    args.Append(edge);
  }
  //
  for ( const auto& edge : vEdges )
  {
    args.Append(edge);
  }

  // Fuse edges to obtain vertices at their intersection points.
  BOPAlgo_Builder algo;
  TopoDS_Shape fused = BooleanGeneralFuse(args, GeneralFuseTol, algo, false);

  // Get parameters along U curves.
  for ( int i = 0; i < (int) uEdges.size(); ++i )
  {
    std::vector<double> params;
    this->getEdgeParameters( uEdges[i], algo.History(), params );

    if ( uParams.ColNumber() < params.size() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "The number of intersection parameters "
                                                  "exceeds the number of curves.");
      return false;
    }

    for ( int k = 0; k < (int) params.size(); ++k )
    {
      uParams(i, k) = params[k];
    }
  }

  // Get parameters along V curves.
  for ( int i = 0; i < (int) vEdges.size(); ++i )
  {
    std::vector<double> params;
    this->getEdgeParameters( vEdges[i], algo.History(), params );

    if ( vParams.RowNumber() < params.size() )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "The number of intersection parameters "
                                                  "exceeds the number of curves.");
      return false;
    }

    for ( int k = 0; k < (int) params.size(); ++k )
    {
      vParams(k, i) = params[k];
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_IntersectCurves::getEdgeParameters(const TopoDS_Edge&               edge,
                                                const Handle(BRepTools_History)& history,
                                                std::vector<double>&             params)
{
  double f, l;
  Handle(Geom_Curve) c3d = BRep_Tool::Curve(edge, f, l);

  // To construct topologies.
  BRep_Builder bbuilder;

  // Projection tool to get parameters of points by inversion.
  ShapeAnalysis_Curve sac;

  // Get all constructed edge splits.
  const TopTools_ListOfShape& modified = history->Modified(edge);

  // Special case when no splits are done: take extremities.
  if ( modified.IsEmpty() )
  {
    params.push_back( c3d->FirstParameter() );
    params.push_back( c3d->LastParameter() );
    return;
  }

  // Put all vertices in a compound.
  TopoDS_Compound modifiedComp;
  bbuilder.MakeCompound(modifiedComp);
  //
  for ( TopTools_ListIteratorOfListOfShape lit(modified); lit.More(); lit.Next() )
  {
    bbuilder.Add(modifiedComp, lit.Value());
  }

  // Get parameters.
  TopTools_IndexedMapOfShape splitVertices;
  TopExp::MapShapes(modifiedComp, TopAbs_VERTEX, splitVertices);
  //
  for ( int vidx = 1; vidx <= splitVertices.Extent(); ++vidx )
  {
    const TopoDS_Vertex& V = TopoDS::Vertex( splitVertices(vidx) );
    gp_Pnt               P = BRep_Tool::Pnt(V);
    gp_Pnt               Pproj;
    double               param;

    sac.Project(c3d, P, 1e-3, Pproj, param);

    bool isAlreadyThere = false;
    //
    for ( const auto p : params )
    {
      if ( Abs(p - param) < 0.001 )
      {
        isAlreadyThere = true;
        break;
      }
    }

    if ( !isAlreadyThere )
      params.push_back(param);
  }

  std::sort( params.begin(), params.end() );

  if ( !m_plotter.Access().IsNull() )
  {
    // Evaluate params back in 3D.
    Handle(asiAlgo_BaseCloud<double>) curvePts = new asiAlgo_BaseCloud<double>;
    //
    for ( const auto p : params )
    {
      curvePts->AddElement( c3d->Value(p) );
    }

    /*m_plotter.DRAW_SHAPE(modifiedComp, "images");
    m_plotter.DRAW_POINTS(curvePts->GetCoordsArray(), Color_Blue, "imagePts");*/
  }
}
