//-----------------------------------------------------------------------------
// Created on: 24 August 2018
//-----------------------------------------------------------------------------
// Copyright (c) 2018-present, Sergey Slyadnev
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
#include <asiAlgo_RepatchFaces.h>

// asiAlgo includes
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_PlateOnEdges.h>
#include <asiAlgo_TopoKill.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepGProp.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <GProp_GProps.hxx>
#include <GeomProjLib.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_Face.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_HSequenceOfShape.hxx>

// STL includes
#include <vector>

#undef DRAW_DEBUG
#if defined DRAW_DEBUG
  #pragma message("===== warning: DRAW_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------

namespace
{
  Handle(BRepTools_History)
    BuildHistorySewing(const std::vector<TopoDS_Shape>& shapes,
                       const BRepBuilderAPI_Sewing&     tool)
  {
    Handle(BRepTools_History) history = new BRepTools_History();

    std::vector<TopoDS_Shape>::const_iterator shapeIt = shapes.cbegin();
    for ( ; shapeIt != shapes.cend(); ++shapeIt )
    {
      const TopoDS_Shape& shape = *shapeIt;

      // Shells, compounds and wires are not supported by BRepTools_History.
      if ( BRepTools_History::IsSupportedType(shape) && tool.IsModified(shape) )
      {
        history->AddModified(shape.Located(TopLoc_Location()), tool.Modified(shape));
      }

      if ( shape.ShapeType() >= TopAbs_FACE )
        continue;

      for ( TopExp_Explorer faceIt(shape, TopAbs_FACE); faceIt.More(); faceIt.Next() )
      {
        const TopoDS_Shape& oldShape = faceIt.Current();

        // Find modified face.
        TopoDS_Shape newShape = oldShape;
        if ( tool.IsModifiedSubShape(oldShape) )
          newShape = tool.ModifiedSubShape(oldShape);
        if ( !newShape.IsNull() )
          history->AddModified(oldShape.Located(TopLoc_Location()), newShape);
      }
    }

    return history;
  }
}

//-----------------------------------------------------------------------------

asiAlgo_RepatchFaces::asiAlgo_RepatchFaces(const TopoDS_Shape&  shape,
                                           ActAPI_ProgressEntry progress,
                                           ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm(progress, plotter), m_input(shape)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_RepatchFaces::Perform(const TColStd_PackedMapOfInteger& faceIds)
{
  const double invalidityCoeff = 5.;

  /* ================
   *  Pre-processing.
   * ================ */

  // Get all faces.
  TopTools_IndexedMapOfShape facesMap;
  TopExp::MapShapes(m_input, TopAbs_FACE, facesMap);

  // Area calculation parameters.
  const double EPS = 1.0e-5;
  double origArea = 0.0, resultArea = 0.0;

  // Check presence of inner wires and calculate total area of input faces.
  for ( TColStd_PackedMapOfInteger::Iterator iter(faceIds); iter.More(); iter.Next() )
  {
    const int          fid  = iter.Key();
    const TopoDS_Face& face = TopoDS::Face( facesMap(fid) );

    // Check for inner wires.
    TopoDS_Wire outerWire = ShapeAnalysis::OuterWire(face);
    //
    for ( TopExp_Explorer expFW(face, TopAbs_WIRE); expFW.More(); expFW.Next() )
    {
      const TopoDS_Shape& candidate = expFW.Current();
      //
      if ( candidate.IsSame(outerWire) )
        continue;

      m_progress.SendLogMessage(LogErr(Normal) << "Faces with inner contours could not be repatched.");
      return false;
    }

    // Face area.
    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props, EPS);
    origArea += props.Mass();
  }

  // Prepare history.
  m_history = new BRepTools_History;

  // Get faces adjacency map.
  TopTools_IndexedDataMapOfShapeListOfShape edgeFacesMap;
  TopExp::MapShapesAndAncestors(m_input, TopAbs_EDGE, TopAbs_FACE, edgeFacesMap);

  // Construct used faces group.
  std::vector<bool> usedFaces(facesMap.Extent(), true);
  for ( TColStd_PackedMapOfInteger::Iterator iter(faceIds); iter.More(); iter.Next() )
    usedFaces[iter.Key() - 1] = false;

  // We need to construct adjacency groups; each group is processed independently.
  // Result is stored in a special array to keep indexes valid.
  std::vector<TColStd_PackedMapOfInteger> groups;
  std::vector<TopoDS_Shape> newFaces;
  for ( int i = 1; i <= facesMap.Size(); ++i )
  {
    if ( usedFaces[i - 1] )
      continue; // Skip used faces.

    // New group is found.
    usedFaces[i - 1] = true;
    TColStd_PackedMapOfInteger localGroup;
    localGroup.Add(i);

    // Explore current group.
    TopTools_IndexedMapOfShape edgesMap;
    TopExp::MapShapes(facesMap(i), TopAbs_EDGE, edgesMap);
    //
    for ( int edgeIdx = 1; edgeIdx <= edgesMap.Size(); ++edgeIdx )
    {
      const TopoDS_Shape& edge = edgesMap(edgeIdx);

      const TopoDS_ListOfShape& candidateFaces = edgeFacesMap.FindFromKey(edge);
      for ( TopoDS_ListOfShape::Iterator iter(candidateFaces); iter.More() ; iter.Next() )
      {
        const TopoDS_Shape& candidateFace = iter.Value();

        int index = facesMap.FindIndex(candidateFace);
        if ( faceIds.Contains(index) && !usedFaces[index - 1] )
        {
          usedFaces[index - 1] = true;
          localGroup.Add(index);

          // Add face edges.
          for ( TopExp_Explorer expFE(TopoDS::Face(candidateFace), TopAbs_EDGE); expFE.More(); expFE.Next() )
            edgesMap.Add(expFE.Current());
        }
      }
    }

    TopoDS_Shape repatchF;
    groups.push_back(localGroup);
    bool isOK = this->repatchGroup(localGroup, facesMap, repatchF);
    if ( !isOK )
      return false;

    // New face area.
    GProp_GProps props;
    BRepGProp::SurfaceProperties(repatchF, props, EPS);
    resultArea += props.Mass();

    newFaces.push_back(repatchF);
  }

  if ( int( groups.size() ) == faceIds.Extent() )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "Faces are not stitched.");
  }

  // Check area difference.
  if ( origArea * invalidityCoeff < resultArea )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "The resulting surface oscillates too badly.");
    return false;
  }

  Handle(asiAlgo_TopoKill)
    killer = new asiAlgo_TopoKill(m_input, NULL, NULL);

  // Kill old faces.
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(faceIds); fit.More(); fit.Next() )
  {
    const int fid = fit.Key();

    if ( !killer->AskRemove(facesMap(fid)) )
    {
      return false;
    }
  }
  //
  if ( !killer->Apply() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Face killer failed.");
    return false;
  }
  //
  const TopoDS_Shape& shape = killer->GetResult();
  m_history->Merge( killer->GetHistory()->ConvertToOcc()) ;

  /* ==============
   *  Stitch faces
   * ============== */

  // Faces from groups[] cannot be in list of deleted and in list of modified at the same time.
  Handle(BRepTools_History)
    historyWithoutRemovedFaces = new BRepTools_History();
  //
  for ( int fid = 1; fid <= facesMap.Size(); ++fid )
  {
    const TopTools_ListOfShape& listM = m_history->Modified(facesMap(fid));
    const TopTools_ListOfShape& listG = m_history->Generated(facesMap(fid));

    TopTools_ListOfShape::Iterator modItM(listM);
    for ( ; modItM.More(); modItM.Next() )
    {
      historyWithoutRemovedFaces->AddModified(facesMap(fid), modItM.Value());
    }

    TopTools_ListOfShape::Iterator modItG(listG);
    for ( ; modItG.More(); modItG.Next() )
    {
      historyWithoutRemovedFaces->AddGenerated(facesMap(fid), modItG.Value());
    }
  }
  //
  m_history->Clear();
  m_history = historyWithoutRemovedFaces;
  //
  for ( int i = 0; i < int( groups.size() ); ++i )
  {
    const TColStd_PackedMapOfInteger& groupInd = groups[i];
    for ( TColStd_MapIteratorOfPackedMapOfInteger fit(groupInd); fit.More(); fit.Next() )
    {
      const int fid = fit.Key();
      m_history->AddModified(facesMap(fid), newFaces[i]);
    }
  }

  // Sew.
  std::vector<TopoDS_Shape> shapes;
  shapes.push_back(shape);
  //
  BRepBuilderAPI_Sewing Sewer(asiAlgo_CheckValidity().MaxTolerance(shape), true, false, false, false);
  Sewer.Add(shape);
  for ( int i = 0; i < int( newFaces.size() ); ++i )
  {
    shapes.push_back(newFaces[i]);
    Sewer.Add(newFaces[i]);
  }
  //
  Sewer.Perform();
  m_result = Sewer.SewedShape();
  //
  Handle(BRepTools_History)
    historySewing = ::BuildHistorySewing(shapes, Sewer);
  //
  m_history->Merge(historySewing);

  // Sewing likes to "reduce" solids to shells, so let's get back our solid.
  if ( (shape.ShapeType() == TopAbs_SOLID) && (m_result.ShapeType() == TopAbs_SHELL) )
  {
    TopoDS_Solid solid;
    BRep_Builder bbuilder;
    bbuilder.MakeSolid(solid);
    bbuilder.Add(solid, m_result);
    //
    m_result = solid;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RepatchFaces::repatchGroup(const TColStd_PackedMapOfInteger& faceIds,
                                        const TopTools_IndexedMapOfShape& facesMap,
                                        TopoDS_Shape&                     newfFace)
{
#ifdef DRAW_DEBUG
  TIMER_NEW
  TIMER_GO
#endif

  // Prepare a compound of faces in question.
  TopoDS_Compound compFaces;
  BRep_Builder().MakeCompound(compFaces);
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(faceIds); fit.More(); fit.Next() )
  {
    const int fid = fit.Key();

    BRep_Builder().Add( compFaces, facesMap(fid) );
  }

  // Build map of edges to extract free ones.
  TopTools_IndexedDataMapOfShapeListOfShape edgesFaces;
  TopExp::MapShapesAndAncestors(compFaces, TopAbs_EDGE, TopAbs_FACE, edgesFaces);

  // Get free edges.
  Handle(TopTools_HSequenceOfShape) freeEdgesSeq = new TopTools_HSequenceOfShape;
  Handle(TopTools_HSequenceOfShape) allEdgesSeq = new TopTools_HSequenceOfShape;
  //
  for ( int k = 1; k <= edgesFaces.Extent(); ++k )
  {
    allEdgesSeq->Append(edgesFaces.FindKey(k));

    const TopTools_ListOfShape& faces = edgesFaces(k);
    //
    if ( faces.Extent() == 1 )
    {
      const TopoDS_Edge& E = TopoDS::Edge( edgesFaces.FindKey(k) );
      //
      if ( BRep_Tool::Degenerated(E) )
        continue;

      freeEdgesSeq->Append(E);

#if defined DRAW_DEBUG
      m_plotter.DRAW_SHAPE(freeEdges[freeEdges.size() - 1], Color_Red, 1.0, true, "freeEdge");
#endif
    }
  }

  // Compose a new wire from the free edges.
  Handle(TopTools_HSequenceOfShape) freeWires;
  ShapeAnalysis_FreeBounds::ConnectEdgesToWires(freeEdgesSeq, 1e-3, 0, freeWires);
  //
  TopoDS_Wire repatchW;
  try
  {
    repatchW = TopoDS::Wire( freeWires->First() );
  }
  catch ( ... )
  {
    return false;
  }

#if defined DRAW_DEBUG
  m_plotter.REDRAW_SHAPE("repatchW", repatchW, Color_Red, 1.0, true);
#endif

#ifdef DRAW_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("algoRepair_RepatchFaces: preparation")
#endif

  /* =================
   *  Surface fitting
   * ================= */

#ifdef DRAW_DEBUG
  TIMER_RESET
  TIMER_GO
#endif

  // Prepare interpolation tool.
  asiAlgo_PlateOnEdges interpAlgo(m_input, m_progress, m_plotter);

  // Interpolate.
  Handle(Geom_BSplineSurface) repatchSurf;
  //
  if ( !interpAlgo.BuildSurf(allEdgesSeq, GeomAbs_C0, repatchSurf) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Interpolation failed.");
    return false;
  }

#if defined DRAW_DEBUG
  m_plotter.DRAW_SURFACE(repatchSurf, Color_White, "repatch");
#endif

#ifdef DRAW_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("algoRepair_RepatchFaces: surface fitting")
#endif

  /* ===========================
   *  Prepare model and a patch
   * =========================== */

#ifdef DRAW_DEBUG
  TIMER_RESET
  TIMER_GO
#endif

  // Build new face.
  TopoDS_Face repatchF = BRepBuilderAPI_MakeFace(repatchSurf, repatchW, false);
  const double maxTol = BRep_Tool::MaxTolerance(repatchW, TopAbs_VERTEX);

  // Build pcurves manually since the result is better than result created by shapefix.
  for ( TopExp_Explorer expWE(repatchW, TopAbs_EDGE); expWE.More(); expWE.Next() )
  {
    double f, l, tol(maxTol);
    const TopoDS_Edge& edge = TopoDS::Edge(expWE.Current());
    Handle(Geom_Curve) c3d = BRep_Tool::Curve(edge, f, l);

    Handle(Geom2d_Curve) c2d = GeomProjLib::Curve2d(c3d, f, l, repatchSurf, tol);

    BRep_Builder bb;
    bb.UpdateEdge(edge, c2d, repatchF, maxTol);
  }

  // Heal defects.
  ShapeFix_Face ShapeHealer(repatchF);
  ShapeHealer.Perform();
  repatchF = ShapeHealer.Face();

#ifdef DRAW_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("algoRepair_RepatchFaces: healing")

  TIMER_RESET
  TIMER_GO
#endif

  // Classify point to invert wire if necessary.
  BRepTopAdaptor_FClass2d FClass(repatchF, 0.);
  if ( FClass.PerformInfinitePoint() == TopAbs_IN)
  {
    BRep_Builder B;
    TopoDS_Shape S = repatchF.EmptyCopied();
    TopoDS_Iterator it(repatchF);
    while ( it.More() )
    {
      B.Add( S, it.Value().Reversed() );
      it.Next();
    }
    repatchF = TopoDS::Face(S);
  }

#if defined DRAW_DEBUG
  m_plotter.DRAW_SHAPE(repatchF, Color_White, "repatchF");
#endif

  newfFace = repatchF;
  return true;
}
