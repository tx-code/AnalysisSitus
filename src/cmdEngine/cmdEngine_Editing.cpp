//-----------------------------------------------------------------------------
// Created on: 24 August 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

// cmdEngine includes
#include <cmdEngine.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiAlgo includes
#include <asiAlgo_AppSurfUtils.h>
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_DivideByContinuity.h>
#include <asiAlgo_EulerKEF.h>
#include <asiAlgo_EulerKEV.h>
#include <asiAlgo_EulerKFMV.h>
#include <asiAlgo_InterpolateSurfMesh.h>
#include <asiAlgo_InvertShells.h>
#include <asiAlgo_Join3dEdges.h>
#include <asiAlgo_JoinEdges.h>
#include <asiAlgo_MeshConvert.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_OrientCnc.h>
#include <asiAlgo_PlateOnEdges.h>
#include <asiAlgo_PlateOnPoints.h>
#include <asiAlgo_RebuildEdge.h>
#include <asiAlgo_RecognizeBlends.h>
#include <asiAlgo_RelievePointCloud.h>
#include <asiAlgo_RepatchFaces.h>
#include <asiAlgo_SmallEdges.h>
#include <asiAlgo_SuppressBlendChain.h>
#include <asiAlgo_SuppressBlendsInc.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_TopoAttrOrientation.h>
#include <asiAlgo_TopoKill.h>
#include <asiAlgo_Utils.h>
#include <asiAlgo_ConvertToBezier.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <BRepAlgoAPI_BuilderAlgo.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepTools.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom_BoundedSurface.hxx>
#include <GeomConvert.hxx>
#include <GeomLib.hxx>
#include <gp_Quaternion.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeCustom.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <ShapeUpgrade_ShapeDivideClosed.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Solid.hxx>

// Mobius includes
#if defined USE_MOBIUS
  #include <mobius/cascade_BSplineCurve.h>
  #include <mobius/cascade_BSplineSurface.h>
  #include <mobius/cascade_Triangulation.h>
  #include <mobius/geom_FairBCurve.h>
  #include <mobius/geom_FairBSurf.h>

  using namespace mobius;
#endif

namespace
{
  //! Improves the continuity of the passed surface by knot removal.
  static void SimplifySurface(Handle(Geom_BSplineSurface)& BS,
                              const double                 Tol,
                              const int                    MultMin)

  {
    int  multU, multV, ii;
    bool Ok;

    const TColStd_Array1OfReal&    U  = BS->UKnots();
    const TColStd_Array1OfReal&    V  = BS->VKnots();
    const TColStd_Array1OfInteger& UM = BS->UMultiplicities();
    const TColStd_Array1OfInteger& VM = BS->VMultiplicities();

    for ( ii = U.Length() - 1; ii > 1; ii-- )
    {
      Ok    = true;
      multU = UM.Value(ii);
      for  ( ; Ok && multU > MultMin; multU-- )
      {
        Ok = BS->RemoveUKnot(ii, 1, Tol);
      }
    }

    for ( ii = V.Length() - 1; ii > 1; ii-- )
    {
      Ok    = true;
      multV = VM.Value(ii);
      for  ( ; Ok && multV > MultMin; multV-- )
      {
        Ok = BS->RemoveVKnot(ii, 1, Tol);
      }
    }
  }

  //! Improves the continuity of the passed curve by knot removal.
  static void SimplifyCurve(Handle(Geom_BSplineCurve)& BC,
                            const double               Tol,
                            const int                  MultMin)

  {
    int  mult, ii;
    bool Ok;

    const TColStd_Array1OfReal&    U = BC->Knots();
    const TColStd_Array1OfInteger& M = BC->Multiplicities();

    for ( ii = U.Length() - 1; ii > 1; ii-- )
    {
      Ok   = true;
      mult = M.Value(ii);
      for  ( ; Ok && mult > MultMin; mult-- )
      {
        Ok = BC->RemoveKnot(ii, 1, Tol);
      }
    }
  }
}

//-----------------------------------------------------------------------------

bool FindNamedArg(const Handle(asiTcl_Interp)&  interp,
                  const int                     argc,
                  const char**                  argv,
                  const Handle(asiAlgo_Naming)& naming,
                  const char*                   ssKey,
                  const TopAbs_ShapeEnum        ssType,
                  TopTools_ListOfShape&         result)
{
  // Loop over the arguments to find a named sub-shape of the requested type.
  TopTools_ListOfShape subShapes;
  //
  for ( int k = 1; k < argc - 1; k += 2 )
  {
    if ( interp->IsKeyword(argv[k], ssKey) )
    {
      // Get named sub-shape.
      TopoDS_Shape subshape = naming->GetShape(argv[k + 1]);
      //
      if ( subshape.IsNull() || subshape.ShapeType() != ssType )
        continue;

      // Set result and return.
      result.Append(subshape);
    }
  }

  return !result.IsEmpty();
}

//-----------------------------------------------------------------------------

int ENGINE_KEV(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  if ( argc != 3 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Get naming service.
  Handle(asiAlgo_Naming) naming = part_n->GetNaming();

  // Check whether naming service is active.
  const bool hasNaming = !naming.IsNull();
  //
  if ( !hasNaming )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is available "
                                                           "for named shapes only.");
    return TCL_ERROR;
  }

  // Find edge to kill.
  TopTools_ListOfShape edges;
  //
  if ( !FindNamedArg(interp, argc, argv, naming, "edge", TopAbs_EDGE, edges) &&
       !FindNamedArg(interp, argc, argv, naming, "name", TopAbs_EDGE, edges) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find edge to kill.");
    return TCL_ERROR;
  }
  //
  const TopoDS_Edge& E = TopoDS::Edge( edges.First() );

  // Find vertex to kill.
  TopoDS_Vertex V;
  //
  if ( argc == 5 )
  {
    TopTools_ListOfShape vertices;
    //
    if ( !FindNamedArg(interp, argc, argv, naming, "vertex", TopAbs_VERTEX, vertices) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find vertex to kill.");
      return TCL_ERROR;
    }
    //
    V = TopoDS::Vertex( vertices.First() );
  }

  // Prepare KEV utility.
  Handle(asiAlgo_EulerKEV) KEV;
  //
  if ( V.IsNull() )
    KEV = new asiAlgo_EulerKEV( part_n->GetShape(),
                                E,
                                interp->GetProgress(),
                                interp->GetPlotter() );
  else
    KEV = new asiAlgo_EulerKEV( part_n->GetShape(),
                                E,
                                V,
                                false,
                                interp->GetProgress(),
                                interp->GetPlotter() );
  //
  KEV->SetHistory( naming->GetHistory() );
  //
  if ( !KEV->Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "KEV failed.");
    return TCL_ERROR;
  }
  //
  const TopoDS_Shape& result = KEV->GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_KEF(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  if ( argc < 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Get naming service.
  Handle(asiAlgo_Naming) naming = part_n->GetNaming();

  // Check whether naming service is active.
  const bool hasNaming = !naming.IsNull();
  //
  if ( !hasNaming )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is available "
                                                           "for named shapes only.");
    return TCL_OK;
  }

  // Find face to kill.
  TopTools_ListOfShape faces;
  //
  if ( !FindNamedArg(interp, argc, argv, naming, "face", TopAbs_FACE, faces) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find face to kill.");
    return TCL_OK;
  }

  // Find edge to kill. The edge to kill may be specified with 'edge' or 
  // 'kedge' keyword. In case of 'edge', it means that no edges-to-survive
  // are defined.
  TopTools_ListOfShape edges2Kill;
  //
  if ( !FindNamedArg(interp, argc, argv, naming, "kedge", TopAbs_EDGE, edges2Kill) &&
       !FindNamedArg(interp, argc, argv, naming, "edge", TopAbs_EDGE, edges2Kill) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find edge to kill.");
    return TCL_OK;
  }

  // Find edge to survive.
  TopTools_ListOfShape edges2Survive;
  //
  if ( !FindNamedArg(interp, argc, argv, naming, "sedge", TopAbs_EDGE, edges2Survive) )
  {
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "No edge-to-survive is defined.");
  }

  Handle(asiAlgo_EulerKEF) KEF;

  // Get topological elements.
  const TopoDS_Face& F  = TopoDS::Face( faces.First() );
  const TopoDS_Edge& EK = TopoDS::Edge( edges2Kill.First() );
  //
  if ( edges2Survive.Extent() )
  {
    const TopoDS_Edge& ES = TopoDS::Edge( edges2Survive.First() );

    // Prepare KEF utility.
    KEF = new asiAlgo_EulerKEF( part_n->GetShape(),
                                F,
                                EK,
                                ES,
                                interp->GetProgress(),
                                interp->GetPlotter() );
  }
  else
  {
    // Prepare KEF utility.
    KEF = new asiAlgo_EulerKEF( part_n->GetShape(),
                                F,
                                EK,
                                interp->GetProgress(),
                                interp->GetPlotter() );
  }

  // Set history and perform.
  KEF->SetHistory( naming->GetHistory() );
  //
  if ( !KEF->Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "KEF failed.");
    return TCL_OK;
  }
  //
  const TopoDS_Shape& result = KEF->GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_KFMV(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Get naming service.
  Handle(asiAlgo_Naming) naming = part_n->GetNaming();

  // Check whether naming service is active.
  const bool hasNaming = !naming.IsNull();
  //
  if ( !hasNaming )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is available "
                                                           "for named shapes only.");
    return TCL_OK;
  }

  // Find face to kill.
  TopTools_ListOfShape faces;
  //
  if ( !FindNamedArg(interp, argc, argv, naming, "name", TopAbs_FACE, faces) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find face to kill.");
    return TCL_OK;
  }
  //
  const TopoDS_Face& F = TopoDS::Face( faces.First() );

  Handle(asiAlgo_EulerKFMV)
    KFMV = new asiAlgo_EulerKFMV( part_n->GetShape(),
                                  F,
                                  interp->GetProgress(),
                                  interp->GetPlotter() );

  // Set history and perform.
  KFMV->SetHistory( naming->GetHistory() );
  //
  if ( !KFMV->Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "KFMV failed.");
    return TCL_OK;
  }
  //
  const TopoDS_Shape& result = KFMV->GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int _MergeSubShapes(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv,
                    const TopAbs_ShapeEnum       ssType)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Get naming service.
  Handle(asiAlgo_Naming) naming = part_n->GetNaming();

  // Check whether naming service is active.
  const bool hasNaming = !naming.IsNull();
  //
  if ( !hasNaming )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is available "
                                                           "for named shapes only.");
    return TCL_OK;
  }

  // Loop over the arguments to collect sub-shapes to merge.
  TopTools_ListOfShape subShapes;
  //
  for ( int k = 1; k < argc - 1; k += 2 )
  {
    if ( !interp->IsKeyword(argv[k], "name") )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Keyword '-name' is expected.");
      return TCL_ERROR;
    }

    // Get named sub-shape.
    TopoDS_Shape subshape = naming->GetShape(argv[k + 1]);
    //
    if ( subshape.IsNull() || subshape.ShapeType() != ssType )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed sub-shape is null "
                                                             "or not of a proper type.");
      return TCL_OK;
    }

    subShapes.Append(subshape);
  }

  // Prepare a replacement shape as the first shape.
  const TopoDS_Shape& replacement = subShapes.First();

  // Prepare killer.
  asiAlgo_TopoKill killer( cmdEngine::model->GetPartNode()->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );
  //
  killer.SetHistory( part_n->GetNaming()->GetHistory() );

  // Put requests on replacement.
  for ( TopTools_ListIteratorOfListOfShape lit(subShapes); lit.More(); lit.Next() )
  {
    const TopoDS_Shape& operand = lit.Value();

    // Notice that the substitution sub-shape is oriented like the operand.
    if ( !killer.AskReplace( operand, replacement.Oriented( operand.Orientation() ) ) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on replacement rejected.");
      return TCL_OK;
    }
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_OK;
  }

  // Get result.
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int _KillSubShape(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv,
                  const TopAbs_ShapeEnum       ssType)
{
  if ( argc != 2 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Check whether naming service is active.
  const bool hasNaming = !part_n->GetNaming().IsNull();

  // Check if the global identifier is passed.
  const bool isGlobal = interp->HasKeyword(argc, argv, "gid");

  // Sub-shape in question.
  TopoDS_Shape subshape;

  // Check if naming service is active. If so, the user may ask to access
  // a sub-shape in question by its unique name.
  if ( hasNaming && !isGlobal )
  {
    if ( !interp->IsKeyword(argv[1], "name") )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Keyword '-name' is expected.");
      return TCL_ERROR;
    }
    else
    {
      TCollection_AsciiString name(argv[2]);
      //
      subshape = part_n->GetNaming()->GetShape(name);
      //
      if ( subshape.IsNull() || subshape.ShapeType() != ssType )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed sub-shape is null "
                                                               "or not of a proper type.");
        return TCL_OK;
      }
    }
  }
  else if ( isGlobal && argc == 3 )
  {
    const int gssidx = atoi(argv[2]);
    //
    if ( gssidx < 1 )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Sub-shape global index should be 1-based.");
      return TCL_OK;
    }

    // Get map of sub-shapes with respect to those the passed index is relevant.
    const TopTools_IndexedMapOfShape&
      subShapes = part_n->GetAAG()->RequestMapOfSubShapes();

    // Get sub-shape in question.
    subshape = subShapes(gssidx);
  }
  else if ( !isGlobal ) // Naming is not used to access the argument.
  {
    const int ssidx = atoi(argv[1]);
    //
    if ( ssidx < 1 )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Sub-shape index should be 1-based.");
      return TCL_OK;
    }

    // Get map of sub-shapes with respect to those the passed index is relevant.
    TopTools_IndexedMapOfShape subShapesOfType;
    part_n->GetAAG()->RequestMapOf(ssType, subShapesOfType);

    // Get sub-shape in question.
    subshape = subShapesOfType(ssidx);
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Inconsistent arguments.");
    return TCL_ERROR;
  }

  // Prepare killer.
  asiAlgo_TopoKill killer( cmdEngine::model->GetPartNode()->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );
  //
  if ( part_n->HasNaming() )
    killer.SetHistory( part_n->GetNaming()->GetHistory() );
  //
  if ( !killer.AskRemove(subshape) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on removal rejected.");
    return TCL_OK;
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_OK;
  }

  // Get result.
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MergeEdges(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  return _MergeSubShapes(interp, argc, argv, TopAbs_EDGE);
}

//-----------------------------------------------------------------------------

int ENGINE_MergeVertices(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  return _MergeSubShapes(interp, argc, argv, TopAbs_VERTEX);
}

//-----------------------------------------------------------------------------

int ENGINE_KillSubShape(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  return _KillSubShape(interp, argc, argv, TopAbs_SHAPE);
}

//-----------------------------------------------------------------------------

int ENGINE_KillVertex(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  return _KillSubShape(interp, argc, argv, TopAbs_VERTEX);
}

//-----------------------------------------------------------------------------

int ENGINE_KillEdge(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  return _KillSubShape(interp, argc, argv, TopAbs_EDGE);
}

//-----------------------------------------------------------------------------

int ENGINE_KillFace(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( !interp->HasKeyword(argc, argv, "defeat") )
    return _KillSubShape(interp, argc, argv, TopAbs_FACE);

  /* ========================
   *  Smart face suppression
   * ======================== */

  // Get face indices.
  TColStd_PackedMapOfInteger fids;
  //
  for ( int i = 1; i < argc; ++i )
  {
    if ( interp->IsKeyword(argv[i], "defeat") ) continue;

    // Process next face index.
    const int fidx = atoi(argv[i]);
    //
    if ( fidx < 1 )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face index should be 1-based.");
      return TCL_ERROR;
    }
    //
    fids.Add(fidx);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  TopoDS_Shape             part   = part_n->GetShape();
  Handle(asiAlgo_AAG)      aag    = part_n->GetAAG();

  // Collect faces to kill.
  TopTools_ListOfShape faces2Kill;
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(fids); fit.More(); fit.Next() )
  {
    const int          fid       = fit.Key();
    const TopoDS_Face& face2Kill = aag->GetFace(fid);

    faces2Kill.Append(face2Kill);
  }

  TIMER_NEW
  TIMER_GO

  // Perform Boolean operation for face removal.
  TopoDS_Shape result;
  //
  if ( !asiAlgo_Utils::BooleanRemoveFaces( part,
                                           faces2Kill,
                                           false, // Parallel mode.
                                           true, // Whether to track history.
                                           result,
                                           interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Smart face removal failed.");
    return TCL_OK;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Smart face removal")

  // Modify Data Model
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_KillFaces(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Prepare killer.
  asiAlgo_TopoKill killer( part_n->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );

  // Get map of sub-shapes with respect to those the passed index is relevant.
  TopTools_IndexedMapOfShape subShapesOfType;
  part_n->GetAAG()->RequestMapOf(TopAbs_FACE, subShapesOfType);

  for ( int k = 0; k < argc; ++k )
  {
    const int fid = atoi(argv[k]);
    //
    if ( fid < 1 || fid > subShapesOfType.Extent() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face ID %1 is out of range. Skipping..."
                                                          << fid);
      continue;
    }

    const TopoDS_Shape& subshape = subShapesOfType(fid);

    if ( !killer.AskRemove(subshape) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on removal of face %1 is rejected."
                                                          << fid);
      continue;
    }
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_ERROR;
  }

  // Get result.
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_KillSolidByFace(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const int fidx = atoi(argv[1]);
  //
  if ( fidx < 1 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face index should be 1-based.");
    return TCL_OK;
  }

  // Get Part Node
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Get map of faces with respect to those the passed index is relevant
  const TopTools_IndexedMapOfShape& allFaces = part_n->GetAAG()->GetMapOfFaces();

  // Get face in question
  TopoDS_Face face = TopoDS::Face( allFaces(fidx) );

  // Get owner solid
  TopTools_IndexedDataMapOfShapeListOfShape faceOwners;
  TopExp::MapShapesAndAncestors(part_n->GetShape(), TopAbs_FACE, TopAbs_SOLID, faceOwners);
  //
  if ( faceOwners.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find parent solid for face %1." << fidx);
    return TCL_OK;
  }

  // Get owner shapes
  TopTools_ListOfShape owners = faceOwners.FindFromKey(face);
  //
  if ( owners.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There are no parents for face %1. Cannot proceed." << fidx);
    return TCL_OK;
  }

  // Get solid
  TopoDS_Solid ownerSolid = TopoDS::Solid( owners.First() );
  //
  if ( ownerSolid.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Owner solid is null. Cannot proceed.");
    return TCL_OK;
  }

  // Prepare killer
  asiAlgo_TopoKill killer( cmdEngine::model->GetPartNode()->GetShape(),
                           interp->GetProgress(),
                           interp->GetPlotter() );
  //
  if ( part_n->HasNaming() )
    killer.SetHistory( part_n->GetNaming()->GetHistory() );
  //
  if ( !killer.AskRemove(ownerSolid) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Request on removal of solid was rejected.");
    return TCL_OK;
  }

  if ( !killer.Apply() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological killer failed.");
    return TCL_OK;
  }

  // Get result
  const TopoDS_Shape& result = killer.GetResult();

  // Modify Data Model
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MoveByFace(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get face index
  const int fidx = atoi(argv[1]);
  //
  if ( fidx < 1 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face index should be 1-based.");
    return TCL_OK;
  }

  // Get offset value
  const double offset = atof(argv[2]);

  // Get part
  TopoDS_Shape
    partShape = Handle(asiEngine_Model)::DownCast(interp->GetModel())->GetPartNode()->GetShape();

  // Get face
  TopTools_IndexedMapOfShape M;
  TopExp::MapShapes(partShape, TopAbs_FACE, M);
  TopoDS_Face faceShape = TopoDS::Face( M.FindKey(fidx) );

  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(faceShape, uMin, uMax, vMin, vMax);

  double uMid = (uMin + uMax)*0.5;
  double vMid = (vMin + vMax)*0.5;

  BRepLProp_SLProps lprops( BRepAdaptor_Surface(faceShape), uMid, vMid, 1, Precision::Confusion() );
  //
  if ( !lprops.IsNormalDefined() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Normal undefined.");
    return TCL_OK;
  }
  //
  gp_Dir n = lprops.Normal();
  //
  if ( faceShape.Orientation() == TopAbs_REVERSED )
    n.Reverse();

  // Prepare transformation
  gp_Trsf T;
  T.SetTranslation(gp_Vec(n)*offset);

  // Move
  TopoDS_Shape moved = BRepBuilderAPI_Transform(partShape, T, true);

  // Modify Data Model
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(moved);
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Imprint(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Get topological item to imprint.
  Handle(asiData_IVTopoItemNode)
    topoItem_n = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
  //
  if ( topoItem_n.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
    return TCL_OK;
  }

  // Put all arguments to the list.
  TopTools_ListOfShape arguments;
  arguments.Append( part_n->GetShape() );
  arguments.Append( topoItem_n->GetShape() );

  // Initialize General Fuse tool for imprinting.
  BRepAlgoAPI_BuilderAlgo GeneralFuse;
  //
  GeneralFuse.SetArguments(arguments);
  GeneralFuse.SetRunParallel(true);
  //
  GeneralFuse.Build();
  //
  if ( GeneralFuse.HasErrors() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Boolean operation failed.");
    return TCL_OK;
  }

  // Take the result
  const TopoDS_Shape& fused = GeneralFuse.Shape();

  // Modify Data Model
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(fused);
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Repair(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_OK;
  }

  // Get part shape.
  TopoDS_Shape partShape = part_n->GetShape();
  //
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part contains no B-Rep.");
    return TCL_OK;
  }

  // Fix shape.
  ShapeFix_Shape fix( part_n->GetShape() );
  fix.Perform();
  TopoDS_Shape result = fix.Shape();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_RepairShifted(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_OK;
  }

  // Get part shape.
  TopoDS_Shape partShape = part_n->GetShape();
  //
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part contains no B-Rep.");
    return TCL_OK;
  }

  // Access selected faces (if any).
  asiAlgo_Feature selected;
  //
  if ( !cmdEngine::cf.IsNull() )
  {
    asiEngine_Part( cmdEngine::cf->Model,
                    cmdEngine::cf->ViewerPart->PrsMgr() ).GetHighlightedFaces(selected);
  }

  // Get the face in question.
  int fid = 0;
  interp->GetKeyValue<int>(argc, argv, "fid", fid);
  //
  if ( fid ) selected.Add(fid);

  TopTools_IndexedMapOfShape M;
  TopExp::MapShapes(partShape, TopAbs_FACE, M);

  for ( asiAlgo_Feature::Iterator fit(selected); fit.More(); fit.Next() )
  {
    TopoDS_Face faceShape = TopoDS::Face( M.FindKey( fit.Key() ) );

    // Fix shape.
    Handle(ShapeFix_Face) sff = new ShapeFix_Face(faceShape);
    //
    sff->FixWireTool()->FixShiftedMode() = true;
    sff->Perform();
    //
    TopoDS_Shape result = sff->Result();
    interp->GetPlotter().DRAW_SHAPE(result, "fixed");
  }

  //// Modify Data Model.
  //cmdEngine::model->OpenCommand();
  //{
  //  asiEngine_Part(cmdEngine::model).Update(result);
  //}
  //cmdEngine::model->CommitCommand();

  //// Update UI.
  //if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
  //  cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetFaceTolerance(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_OK;
  }

  // Get part shape.
  TopoDS_Shape partShape = part_n->GetShape();
  //
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part contains no B-Rep.");
    return TCL_OK;
  }

  const int    faceId = atoi(argv[1]); // 1-based
  const double toler  = atof(argv[2]);

  TopTools_IndexedMapOfShape allFaces;
  TopExp::MapShapes(partShape, TopAbs_FACE, allFaces);
  TopoDS_Face faceShape = TopoDS::Face( allFaces.FindKey(faceId) );

  ShapeFix_ShapeTolerance fixToler;
  fixToler.SetTolerance(faceShape, toler);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetTolerance(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  //
  if ( part_n.IsNull() || !part_n->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_OK;
  }

  // Get part shape.
  TopoDS_Shape partShape = part_n->GetShape();
  //
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part contains no B-Rep.");
    return TCL_OK;
  }

  const double toler = atof(argv[1]);

  // Update tolerance.
  ShapeFix_ShapeTolerance fixToler;
  fixToler.SetTolerance(partShape, toler);

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(partShape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI (tolerance may affect visualization).
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_RebuildEdge(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 2 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node and shape.
  Handle(asiData_PartNode) part_n    = cmdEngine::model->GetPartNode();
  TopoDS_Shape             partShape = part_n->GetShape();

  // Check whether naming service is active.
  const bool hasNaming = !part_n->GetNaming().IsNull();

  // Edge to rebuild.
  TopoDS_Edge edge;

  // Check if naming service is active. If so, the user may ask to access
  // a sub-shape in question by its unique name.
  if ( hasNaming && argc == 3 )
  {
    if ( !interp->IsKeyword(argv[1], "name") )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Keyword '-name' is expected.");
      return TCL_ERROR;
    }
    else
    {
      TCollection_AsciiString name(argv[2]);
      //
      TopoDS_Shape subshape = part_n->GetNaming()->GetShape(name);
      //
      if ( subshape.IsNull() || subshape.ShapeType() != TopAbs_EDGE )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed sub-shape is null "
                                                               "or not of a proper type.");
        return TCL_OK;
      }
      //
      edge = TopoDS::Edge(subshape);
    }
  }
  else // Naming is not used to access the argument.
  {
    const int ssidx = atoi(argv[1]);
    //
    if ( ssidx < 1 )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Sub-shape index should be 1-based.");
      return TCL_OK;
    }

    // Get map of sub-shapes with respect to those the passed index is relevant.
    TopTools_IndexedMapOfShape subShapesOfType;
    part_n->GetAAG()->RequestMapOf(TopAbs_EDGE, subShapesOfType);

    // Get sub-shape in question.
    edge = TopoDS::Edge( subShapesOfType(ssidx).Oriented(TopAbs_EXTERNAL) );
  }

  /* ======================
   *  Perform modification
   * ====================== */

  TIMER_NEW
  TIMER_GO

  // Prepare algorithm.
  asiAlgo_RebuildEdge oper( part_n->GetShape(),
                            interp->GetProgress()/*,
                            interp->GetPlotter()*/ );
  //
  if ( hasNaming )
    oper.SetHistory( part_n->GetNaming()->GetHistory() );

  // Apply geometric operator.
  if ( !oper.Perform(edge) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "BRepTools_Modifier is not done.");
    return false;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Rebuild edge")

  // Get result.
  const TopoDS_Shape&            result  = oper.GetResult();
  const Handle(asiAlgo_History)& history = oper.GetHistory();

  /* =======================
   *  Finalize modification
   * ======================= */

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part partApi(cmdEngine::model);
    //
    partApi.Update(result, history);

    // Store history if naming is not active (otherwise, the naming will
    // be overridden which is not good).
    if ( !partApi.HasNaming() )
      partApi.StoreHistory(history);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_FairCurve(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a curve."
                                                        << argv[1]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast(node);

  // Get B-curve.
  double f, l;
  Handle(Geom_BSplineCurve)
    occtBCurve = Handle(Geom_BSplineCurve)::DownCast( curveNode->GetCurve(f, l) );
  //
  if ( occtBCurve.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The curve in question is not a B-spline curve.");
    return TCL_OK;
  }

  // Perform fairing.
  Handle(Geom_BSplineCurve) result;
  //
#if defined USE_MOBIUS
  // Get fairing coefficient.
  const double lambda = Atof(argv[3]);

  // Convert to Mobius curve.
  cascade_BSplineCurve toMobius(occtBCurve);
  toMobius.DirectConvert();
  const t_ptr<t_bcurve>& mobCurve = toMobius.GetMobiusCurve();

  TIMER_NEW
  TIMER_GO

  // Perform fairing from Mobius.
  geom_FairBCurve fairing(mobCurve, lambda, nullptr, nullptr);
  //
  if ( !fairing.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Fairing failed.");
    return TCL_OK;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Mobius B-curve fairing")

  // Get the faired curve.
  const t_ptr<t_bcurve>& mobResult = fairing.GetResult();

  // Convert to OpenCascade curve.
  cascade_BSplineCurve toOpenCascade(mobResult);
  toOpenCascade.DirectConvert();
  result = toOpenCascade.GetOpenCascadeCurve();

  // Draw faired curve.
  interp->GetPlotter().REDRAW_CURVE(argv[1], result, Color_Green, true);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_FairSurf(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 4 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[1]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfNode = Handle(asiData_IVSurfaceNode)::DownCast(node);

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_OK;
  }

  Handle(Geom_BSplineSurface) result;

#if defined USE_MOBIUS
  bool isRelax = interp->HasKeyword(argc, argv, "relax");

  // Get fairing coefficient.
  const double lambda = Atof(argv[3]);

  // Convert to Mobius surface.
  cascade_BSplineSurface converter(occtBSurface);
  converter.DirectConvert();
  //
  const t_ptr<t_bsurf>& mobSurf = converter.GetMobiusSurface();

  // Print bending energy.
  const double initEnergy = mobSurf->ComputeBendingEnergy();
  //
  interp->GetProgress().SendLogMessage( LogNotice(Normal) << "Initial bending energy: %1"
                                                          << initEnergy );

  TIMER_NEW
  TIMER_GO

  // Initialize fairing tool.
  geom_FairBSurf fairing(mobSurf, lambda, nullptr, nullptr);
  //
  if ( !isRelax )
  {
    const int nPolesU = int( mobSurf->GetPoles().size() );
    const int nPolesV = int( mobSurf->GetPoles()[0].size() );
    //
    for ( int i = 0; i < nPolesU; ++i )
    {
      fairing.AddPinnedPole( i, 0 );
      fairing.AddPinnedPole( i, nPolesV - 1 );
    }
    //
    for ( int j = 0; j < nPolesV; ++j )
    {
      fairing.AddPinnedPole( 0, j );
      fairing.AddPinnedPole( nPolesU - 1, j );
    }
  }

  // Perform fairing.
  if ( !fairing.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Fairing failed.");
    return TCL_OK;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Mobius B-surface fairing")

  // Get the faired surface.
  const t_ptr<t_bsurf>& mobResult = fairing.GetResult();

  // Print bending energy.
  const double resEnergy = mobResult->ComputeBendingEnergy();
  //
  interp->GetProgress().SendLogMessage( LogNotice(Normal) << "Resulting bending energy: %1"
                                                          << resEnergy );

  // Convert to OpenCascade surface.
  cascade_BSplineSurface toOpenCascade(mobResult);
  toOpenCascade.DirectConvert();
  result = toOpenCascade.GetOpenCascadeSurface();

  // Draw result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1], result, (resEnergy < initEnergy) ? Color_Default : Color_Red);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_SplitByContinuity(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
  if ( argc != 2 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  /* ================
   *  Prepare inputs
   * ================ */

  const double tolerance = ( argc == 3 ? Atof(argv[2]) : Precision::Confusion() );

  // Get Part Node and shape.
  Handle(asiData_PartNode) part_n    = cmdEngine::model->GetPartNode();
  TopoDS_Shape             partShape = part_n->GetShape();

  // Read the requested continuity.
  GeomAbs_Shape           continuity;
  TCollection_AsciiString continuityStr(argv[1]);
  continuityStr.LowerCase();
  //
  if ( continuityStr == "c1" )
    continuity = GeomAbs_C1;
  else
    continuity = GeomAbs_C0;

  /* ======================
   *  Perform modification
   * ====================== */

  // Divide by continuity.
  asiAlgo_DivideByContinuity tool( interp->GetProgress(),
                                   interp->GetPlotter() );
  //
  if ( !tool.Perform(partShape, continuity, tolerance) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot split by continuity.");
    return TCL_OK;
  }

  /* =======================
   *  Finalize modification
   * ======================= */

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(partShape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SplitClosed(const Handle(asiTcl_Interp)& interp,
                       int,
                       const char**)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Split faces.
  cmdEngine::model->OpenCommand();
  {
    ShapeUpgrade_ShapeDivideClosed divider(shape);
    divider.Perform();
    //
    asiEngine_Part(cmdEngine::model).Update( divider.Result() );
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ExtendSurf(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  // Find Surface Node by name.
  Handle(ActAPI_INode)          node     = cmdEngine::model->FindNodeByName(argv[2]);
  Handle(asiData_IVSurfaceNode) surfNode = Handle(asiData_IVSurfaceNode)::DownCast(node);
  //
  if ( surfNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[2]);
    return TCL_OK;
  }

  // Get surface.
  Handle(Geom_Surface) occtSurface = surfNode->GetSurface();
  //
  if ( occtSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is null.");
    return TCL_OK;
  }

  // Convert to bounded surface.
  Handle(Geom_BoundedSurface)
    BS = Handle(Geom_BoundedSurface)::DownCast( occtSurface->Copy() );
  //
  if ( BS.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not bounded.");
    return TCL_OK;
  }

  // Get length.
  const double L = atof(argv[3]);

  // Get extension qualifier.
  TCollection_AsciiString qualifier(argv[4]);

  if ( interp->HasKeyword(argc, argv, "umin") )
    GeomLib::ExtendSurfByLength(BS, L, 1, 1, 0);
  if ( interp->HasKeyword(argc, argv, "umax") )
    GeomLib::ExtendSurfByLength(BS, L, 1, 1, 1);
  if ( interp->HasKeyword(argc, argv, "vmin") )
    GeomLib::ExtendSurfByLength(BS, L, 1, 0, 0);
  if ( interp->HasKeyword(argc, argv, "vmax") )
    GeomLib::ExtendSurfByLength(BS, L, 1, 0, 1);

  // Create surface.
  interp->GetPlotter().REDRAW_SURFACE(argv[1], BS, Color_White);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_TrimCurve(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a curve."
                                                        << argv[2]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast(node);

  // Get B-curve.
  double f, l;
  Handle(Geom_BSplineCurve)
    occtBCurve = Handle(Geom_BSplineCurve)::DownCast( curveNode->GetCurve(f, l) );
  //
  if ( occtBCurve.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The curve in question is not a B-spline curve.");
    return TCL_OK;
  }

  // Read parameter range.
  const double u0 = atof(argv[3]);
  const double u1 = atof(argv[4]);

  // Trim curve.
  Handle(Geom_Curve)
    trimmedCurve = new Geom_TrimmedCurve(occtBCurve, u0, u1);

  // Create object.
  interp->GetPlotter().REDRAW_CURVE(argv[1], trimmedCurve, Color_White, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_InvertShells(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  TopoDS_Shape             part   = part_n->GetShape();

  // Invert all contained shells.
  asiAlgo_InvertShells algo( part, interp->GetProgress(), interp->GetPlotter() );
  //
  if ( !algo.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shell inversion failed.");
    return TCL_OK;
  }
  //
  const TopoDS_Shape& result = algo.GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_HealSmallEdges(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();
  TopoDS_Shape             part   = part_n->GetShape();

  // Get parameters.
  const double len     = atof(argv[1]);
  const double ang_deg = atof(argv[2]);

  // Heal small edges.
  asiAlgo_SmallEdges algo( part, interp->GetProgress(), interp->GetPlotter() );
  //
  if ( !algo.Perform(len, ang_deg) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Healing operation failed.");
    return TCL_OK;
  }
  //
  const TopoDS_Shape& result = algo.GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Repatch(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  // Get Part Node to access the selected faces.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_OK;
  }
  Handle(asiAlgo_AAG) aag   = partNode->GetAAG();
  TopoDS_Shape        shape = partNode->GetShape();

  // Get indices of faces to repatch.
  TColStd_PackedMapOfInteger fids;
  //
  asiEngine_Part partAPI( cmdEngine::model, cmdEngine::cf->ViewerPart->PrsMgr() );
  partAPI.GetHighlightedFaces(fids);

  // Get fairing coeff.
  double lambda = 0.;
  interp->GetKeyValue(argc, argv, "lambda", lambda);

  // Repatch.
  asiAlgo_RepatchFaces repatcher( shape,
                                  interp->GetProgress(),
                                  interp->GetPlotter() );
  //
  repatcher.SetFairingCoeff(lambda);
  //
  if ( !repatcher.Perform(fids) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Repatching failed.");
    return TCL_OK;
  }
  //
  const TopoDS_Shape& result = repatcher.GetResult();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_KillBlend(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  const bool isInteractive = (argc == 1);

  // Get Part Node to access the selected faces.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  Handle(asiAlgo_AAG) aag   = partNode->GetAAG();
  TopoDS_Shape        shape = partNode->GetShape();

  // Get index of the base face.
  int fid = 0;
  //
  if ( isInteractive )
  {
    TColStd_PackedMapOfInteger fids;
    //
    asiEngine_Part partAPI( cmdEngine::model, cmdEngine::cf->ViewerPart->PrsMgr() );
    partAPI.GetHighlightedFaces(fids);

    if ( fids.Extent() != 1 )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, select a single 'guess face'.");
      return TCL_ERROR;
    }
    //
    fid = fids.GetMinimalMapped();
  }
  else
  {
    TCollection_AsciiString argStr(argv[1]);
    //
    if ( !argStr.IsIntegerValue() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed face ID is not an integer value.");
      return TCL_ERROR;
    }

    fid = atoi(argv[1]);
    //
    if ( !partNode->GetAAG()->HasFace(fid) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face %1 does not exist in the working part."
                                                          << fid);
      return TCL_ERROR;
    }
  }

  TIMER_NEW
  TIMER_GO

  // Perform recognition starting from the guess face.
  asiAlgo_RecognizeBlends recognizer( partNode->GetAAG(),
                                      interp->GetProgress()/*,
                                      interp->GetPlotter()*/ );
  //
  if ( !recognizer.Perform(fid) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Recognition failed.");

    *interp << false;
    return TCL_OK;
  }

  // Perform suppression.
  asiAlgo_SuppressBlendChain suppressor( recognizer.GetAAG(),
                                         interp->GetProgress()/*,
                                         interp->GetPlotter()*/ );
  //
  if ( !suppressor.Perform(fid) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Suppression failed.");

    *interp << false;
    return TCL_OK;
  }
  //
  const TopoDS_Shape&            result  = suppressor.GetResult();
  const Handle(asiAlgo_History)& history = suppressor.GetHistory();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "kill-blend")

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part partApi(cmdEngine::model);
    //
    partApi.Update(result, history);
    partApi.StoreHistory(history);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  *interp << true;
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_KillBlendsInc(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 1 && argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get radius.
  const double maxRadius = ( (argc == 2) ? atof(argv[1]) : 1.e100 );

  // Get Part Node to access the selected faces.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  Handle(asiAlgo_AAG) aag   = partNode->GetAAG();
  TopoDS_Shape        shape = partNode->GetShape();

  TIMER_NEW
  TIMER_GO

  Handle(asiAlgo_History) history;
  TopoDS_Shape result;

  // Perform suppression incrementally.
  int numSuppressedChains = 0;
  //
  asiAlgo_SuppressBlendsInc incSuppress( interp->GetProgress(), nullptr );
  //
  if ( !incSuppress.Perform(aag, maxRadius, result, history,
                            numSuppressedChains) )
  {
    if ( !interp->GetProgress().IsCancelling() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Incremental suppression failed.");
      return TCL_ERROR;
    }
    else
    {
      interp->GetProgress().SendLogMessage(LogNotice(Normal) << "Incremental suppression cancelled.");
      return TCL_OK;
    }
  }
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Number of suppressed blend chains: %1."
                                                       << numSuppressedChains);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "kill-blends-inc")

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part partApi(cmdEngine::model);
    //
    partApi.Update(result, history);
    partApi.StoreHistory(history);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  *interp << numSuppressedChains;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_InsertKnotCurve(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 4 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a curve."
                                                        << argv[2]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast(node);

  // Get B-curve.
  double f, l;
  Handle(Geom_BSplineCurve)
    occtBCurve = Handle(Geom_BSplineCurve)::DownCast( curveNode->GetCurve(f, l) );
  //
  if ( occtBCurve.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The curve in question is not a B-spline curve.");
    return TCL_OK;
  }

  // Get knot value.
  const double u = atof(argv[3]);

  // Get multiplicity.
  const int times = ( (argc == 5) ? atoi(argv[4]) : 1 );

  // Make a copy of the initial curve.
  t_ptr<t_bcurve>
    mobBCurve = cascade::GetMobiusBCurve(occtBCurve);
  //
  t_ptr<t_bcurve> mobResult = mobBCurve->Copy();

  // Insert knot.
  if ( !mobResult->InsertKnot(u, times) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Knot insertion failed for knot %1 to be inserted %2 time(s)."
                                                        << u << times);
    return TCL_ERROR;
  }

  // Draw result.
  interp->GetPlotter().REDRAW_CURVE(argv[1],
                                    cascade::GetOpenCascadeBCurve(mobResult),
                                    Color_Default, true);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_InsertKnotSurfU(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 4 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[2]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfaceNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_OK;
  }

  // Get knot value.
  const double u = atof(argv[3]);

  // Get multiplicity.
  const int times = ( (argc == 5) ? atoi(argv[4]) : 1 );

  // Make a copy of the initial surface.
  t_ptr<t_bsurf>
    mobBSurface = cascade::GetMobiusBSurface(occtBSurface);
  //
  t_ptr<t_bsurf> mobResult = mobBSurface->Copy();

  // Insert knot.
  if ( !mobResult->InsertKnot_U(u, times) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Knot insertion failed for knot %1 to be inserted %2 time(s)."
                                                        << u << times);
    return TCL_ERROR;
  }

  // Draw result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1],
                                      cascade::GetOpenCascadeBSurface(mobResult),
                                      Color_Default);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_InsertKnotsSurfU(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[2]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfaceNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_OK;
  }

  // Get number of knots to insert.
  const double num = atoi(argv[3]);

  // Make a copy of the initial surface.
  t_ptr<t_bsurf>
    mobBSurface = cascade::GetMobiusBSurface(occtBSurface);
  //
  t_ptr<t_bsurf> mobResult = mobBSurface->Copy();

  const double firstKnot = mobResult->GetKnots_U()[0];
  const double lastKnot  = mobResult->GetKnots_U()[mobResult->GetNumOfKnots_U() - 1];
  const double delta     = (lastKnot - firstKnot) / (num + 1);

  // Insert knots.
  for ( int i = 1; i <= num; ++i )
  {
    const double u = firstKnot + i*delta;
    if ( !mobResult->InsertKnot_U(u, 1) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Knot insertion failed for knot %1."
                                                          << u);
      return TCL_ERROR;
    }
  }

  // Draw result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1],
                                      cascade::GetOpenCascadeBSurface(mobResult),
                                      Color_Default);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_InsertKnotSurfV(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 4 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[2]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfaceNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_OK;
  }

  // Get knot value.
  const double v = atof(argv[3]);

  // Get multiplicity.
  const int times = ( (argc == 5) ? atoi(argv[4]) : 1 );

  // Make a copy of the initial surface.
  t_ptr<t_bsurf>
    mobBSurface = cascade::GetMobiusBSurface(occtBSurface);
  //
  t_ptr<t_bsurf> mobResult = mobBSurface->Copy();

  // Insert knot.
  if ( !mobResult->InsertKnot_V(v, times) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Knot insertion failed for knot %1 to be inserted %2 time(s)."
                                                        << v << times);
    return TCL_ERROR;
  }

  // Draw result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1],
                                      cascade::GetOpenCascadeBSurface(mobResult),
                                      Color_Default);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_InsertKnotsSurfV(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVSurfaceNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[2]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfaceNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_OK;
  }

  // Get number of knots to insert.
  const double num = atoi(argv[3]);

  // Make a copy of the initial surface.
  t_ptr<t_bsurf>
    mobBSurface = cascade::GetMobiusBSurface(occtBSurface);
  //
  t_ptr<t_bsurf> mobResult = mobBSurface->Copy();

  const double firstKnot = mobResult->GetKnots_U()[0];
  const double lastKnot  = mobResult->GetKnots_U()[mobResult->GetNumOfKnots_U() - 1];
  const double delta     = (lastKnot - firstKnot) / (num + 1);

  // Insert knots.
  for ( int i = 1; i <= num; ++i )
  {
    const double v = firstKnot + i*delta;
    if ( !mobResult->InsertKnot_V(v, 1) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Knot insertion failed for knot %1."
                                                          << v);
      return TCL_ERROR;
    }
  }

  // Draw result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1],
                                      cascade::GetOpenCascadeBSurface(mobResult),
                                      Color_Default);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_ExchangeUV(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[1]);
  //
  Handle(asiData_IVSurfaceNode)
    surfaceNode = Handle(asiData_IVSurfaceNode)::DownCast(node);
  //
  if ( surfaceNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a surface."
                                                        << argv[1]);
    return TCL_OK;
  }

  // Get B-surface.
  Handle(Geom_BSplineSurface)
    occtBSurface = Handle(Geom_BSplineSurface)::DownCast( surfaceNode->GetSurface() );
  //
  if ( occtBSurface.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The surface in question is not a B-spline surface.");
    return TCL_OK;
  }

  // Exchange UV.
  t_ptr<t_bsurf>
    mobBSurface = cascade::GetMobiusBSurface(occtBSurface);
  //
  mobBSurface->ExchangeUV();

  // Draw result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1],
                                      cascade::GetOpenCascadeBSurface(mobBSurface),
                                      Color_Default);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_SplitCurveBezier(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS
  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[2]);
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast(node);
  //
  if ( curveNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a curve."
                                                        << argv[2]);
    return TCL_OK;
  }

  // Get B-curve.
  double f, l;
  Handle(Geom_BSplineCurve)
    occtBCurve = Handle(Geom_BSplineCurve)::DownCast( curveNode->GetCurve(f, l) );
  //
  if ( occtBCurve.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The curve in question is not a B-spline curve.");
    return TCL_OK;
  }

  // Convert to Mobius curve.
  t_ptr<t_bcurve>
    mobBCurve = cascade::GetMobiusBCurve(occtBCurve);

  // Split.
  std::vector< t_ptr<t_bcurve> > segments;
  //
  if ( !mobBCurve->SplitToBezier(segments) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Splitting to Bezier failed.");
    return TCL_ERROR;
  }

  // Draw results.
  for ( size_t ii = 0; ii < segments.size(); ++ii )
    interp->GetPlotter().DRAW_CURVE(cascade::GetOpenCascadeBCurve(segments[ii]),
                                    Color_Default, true,
                                    argv[1]);

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "This function is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_MovePointCurve(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 6 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Find Node by name.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[1]);
  //
  if ( node.IsNull() || !node->IsKind( STANDARD_TYPE(asiData_IVCurveNode) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a curve."
                                                        << argv[1]);
    return TCL_OK;
  }
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast(node);

  // Get B-curve.
  double f, l;
  Handle(Geom_BSplineCurve)
    occtBCurve = Handle(Geom_BSplineCurve)::DownCast( curveNode->GetCurve(f, l) );
  //
  if ( occtBCurve.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The curve in question is not a B-spline curve.");
    return TCL_OK;
  }

  // Get parameter.
  const double u = atof(argv[2]);

  // Get coordinates of the offset.
  const double dx = atof(argv[3]);
  const double dy = atof(argv[4]);
  const double dz = atof(argv[5]);
  //
  gp_Vec D(dx, dy, dz);

  gp_Pnt orig = occtBCurve->Value(u);
  gp_Pnt dest = orig.XYZ() + D.XYZ();

  interp->GetPlotter().REDRAW_POINT("move-P", orig, Color_Yellow);
  interp->GetPlotter().REDRAW_VECTOR_AT("move-D", orig, D, Color_Yellow);

  const int index1 = 2, index2 = occtBCurve->Poles().Length() - 1;
  int resIndex1, resIndex2;
  //
  Handle(Geom_BSplineCurve) resCurve = Handle(Geom_BSplineCurve)::DownCast( occtBCurve->Copy() );
  //
  resCurve->MovePoint(u, dest, index1, index2, resIndex1, resIndex2);
  //
  interp->GetPlotter().REDRAW_CURVE(argv[1], resCurve, Color_Default, true);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MoveTriangulation(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 7 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get mesh from the Triangulation Node.
  Handle(Poly_Triangulation)
    poly = cascade::GetOpenCascadeMesh( cmdEngine::model->GetTriangulationNode()->GetTriangulation() );
  //
  if ( poly.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is empty.");
    return TCL_ERROR;
  }

  // Read input transformation.
  const double tx = atof(argv[1]);
  const double ty = atof(argv[2]);
  const double tz = atof(argv[3]);
  const double rx = atof(argv[4]);
  const double ry = atof(argv[5]);
  const double rz = atof(argv[6]);

  // Prepare transformation.
  gp_Vec Translation(tx, ty, tz);
  gp_Quaternion RX(gp::DX(), rx/180.*M_PI);
  gp_Quaternion RY(gp::DY(), ry/180.*M_PI);
  gp_Quaternion RZ(gp::DZ(), rz/180.*M_PI);
  //
  gp_Trsf T;
  T.SetRotation(RZ*RY*RX);
  T.SetTranslationPart(Translation);

  // Create transformed vertices.
  Handle(TColgp_HArray1OfPnt) oldNodes = poly->MapNodeArray();
  TColgp_Array1OfPnt newNodes( oldNodes->Lower(), oldNodes->Upper() );
  //
  for ( int nidx = oldNodes->Lower(); nidx <= oldNodes->Upper(); ++nidx )
    newNodes.SetValue( nidx, oldNodes->Value(nidx).Transformed(T) );

  // Create new triangulation.
  Handle(Poly_Triangulation)
    newPoly = new Poly_Triangulation( newNodes, poly->MapTriangleArray()->Array1() );

  // Update Data Model.
  cmdEngine::model->OpenCommand();
  {
    cmdEngine::model->GetTriangulationNode()->SetTriangulation( cascade::GetMobiusMesh(newPoly) );
  }
  cmdEngine::model->CommitCommand();

  // Actualize.
  if ( cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetTriangulationNode() );

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_MovePart(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 7 && argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get shape from the Part Node.
  TopoDS_Shape
    shape = cmdEngine::model->GetPartNode()->GetShape();
  //
  if ( shape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is empty.");
    return TCL_ERROR;
  }

  bool isGeometric = false;
  if ( interp->HasKeyword(argc, argv, "isGeometric") )
  {
    isGeometric = true;
  }

  // Read input transformation.
  const double tx = atof(argv[1]);
  const double ty = atof(argv[2]);
  const double tz = atof(argv[3]);
  const double rx = atof(argv[4]);
  const double ry = atof(argv[5]);
  const double rz = atof(argv[6]);

  // Prepare transformation.
  gp_Vec Translation(tx, ty, tz);
  gp_Quaternion RX(gp::DX(), rx/180.*M_PI);
  gp_Quaternion RY(gp::DY(), ry/180.*M_PI);
  gp_Quaternion RZ(gp::DZ(), rz/180.*M_PI);
  //
  gp_Trsf T;
  T.SetRotation(RZ*RY*RX);
  T.SetTranslationPart(Translation);

  if ( isGeometric )
  {
    // Create a new transformed shape.
    TopoDS_Shape newShape = BRepBuilderAPI_Transform(shape, T, true);

    // Update Data Model.
    cmdEngine::model->OpenCommand();
    {
      asiEngine_Part(cmdEngine::model).Update(newShape);
    }
    cmdEngine::model->CommitCommand();
  }
  else
  {
    cmdEngine::model->OpenCommand();
    {
      asiEngine_Part(cmdEngine::model).GetPart()->SetTransformation(tx, ty, tz, rx, ry, rz);
    }
    cmdEngine::model->CommitCommand();
  }

  // Actualize.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  // To sync metadata.
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConvertToBSurf(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc > 2 )
  {
    Handle(asiData_IVSurfaceNode)
      node = Handle(asiData_IVSurfaceNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find surface object with name %1." << argv[2]);
      return TCL_ERROR;
    }

    double uMin, uMax, vMin, vMax;
    node->GetLimits(uMin, uMax, vMin, vMax);

    // Get shape to convert.
    Handle(Geom_Surface) surf = node->GetSurface();
    //
    if ( surf.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target surface is null.");
      return TCL_ERROR;
    }

    // Apply limits.
    Handle(Geom_RectangularTrimmedSurface)
      tsurf = new Geom_RectangularTrimmedSurface(surf, uMin, uMax, vMin, vMax);

    // Convert.
    Handle(Geom_BSplineSurface) bsurf = GeomConvert::SurfaceToBSplineSurface(tsurf);

    if ( interp->HasKeyword(argc, argv, "cubic") )
      bsurf->IncreaseDegree(3, 3);

    // Set as result.
    interp->GetPlotter().REDRAW_SURFACE(argv[1], bsurf, Color_Default);
  }
  else if ( argc == 1 )
  {
    // Get shape from the Part Node.
    TopoDS_Shape
      shape = cmdEngine::model->GetPartNode()->GetShape();
    //
    if ( shape.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part shape is empty.");
      return TCL_ERROR;
    }

    TopoDS_Shape
      newShape = ShapeCustom::ConvertToBSpline(shape, true, true, true, true);

    // Update Data Model.
    cmdEngine::model->OpenCommand();
    {
      asiEngine_Part(cmdEngine::model).Update(newShape);
    }
    cmdEngine::model->CommitCommand();

    // Actualize.
    if ( cmdEngine::cf->ViewerPart )
      cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConvertPlaneToBSurf(const Handle(asiTcl_Interp)& interp,
                               int                          argc,
                               const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiData_IVSurfaceNode)
    node = Handle(asiData_IVSurfaceNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
  //
  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find surface object with name %1." << argv[2]);
    return TCL_ERROR;
  }

  double uMin, uMax, vMin, vMax;
  node->GetLimits(uMin, uMax, vMin, vMax);

  // Get surface to convert.
  Handle(Geom_Plane) surf = Handle(Geom_Plane)::DownCast( node->GetSurface() );
  //
  if ( surf.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target surface is null or not a plane.");
    return TCL_ERROR;
  }

  // Convert to Mobius plane.
  t_ptr<t_plane> plane = cascade::GetMobiusPlane(surf);

  // Apply limits.
  plane->SetLimits(uMin, uMax, vMin, vMax);

  // Convert to B-surface.
  t_ptr<t_bsurf>
    bsurf = plane->ToBSurface( atoi(argv[3]), atoi(argv[4]) );
  //
  if ( bsurf.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot convert a plane to a B-surface.");
    return TCL_ERROR;
  }

  // Set as result.
  interp->GetPlotter().REDRAW_SURFACE(argv[1], cascade::GetOpenCascadeBSurface(bsurf), Color_Default);

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_RotateQn(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const double Vx     = atof(argv[1]);
  const double Vy     = atof(argv[2]);
  const double Vz     = atof(argv[3]);
  const double angDeg = atof(argv[4]);

  // Create a quaternion representing the rotation.
  core_Quaternion Q;
  Q.SetRotation(t_xyz(Vx, Vy, Vz), angDeg*M_PI/180.);
  //
  interp->GetPlotter().REDRAW_VECTOR_AT("qn_axis", gp::Origin(), gp_Vec(Vx, Vy, Vz), Color_Yellow);

  // Get matrix representation.
  double qnMx[3][3] = { {0., 0., 0.},
                        {0., 0., 0.},
                        {0., 0., 0.} };
  Q.AsMatrix3x3(qnMx);

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Passed quaternion q = (q0, qx, qy, qz) = (%1, %2, %3, %4)."
                                                        << Q.Q0()
                                                        << Q.Qx()
                                                        << Q.Qy()
                                                        << Q.Qz() );

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  TopoDS_Shape shape = partNode->GetShape();

  // Prepare transformation matrix.
  gp_Trsf T;
  T.SetValues(qnMx[0][0], qnMx[0][1], qnMx[0][2], 0.,
              qnMx[1][0], qnMx[1][1], qnMx[1][2], 0.,
              qnMx[2][0], qnMx[2][1], qnMx[2][2], 0.);

  // Perform transformation.
  TopoDS_Shape transformedShape = BRepBuilderAPI_Transform(shape, T, true);

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(transformedShape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_ResetLocation(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Reset location.
  shape.Location( TopLoc_Location() );

  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
  {
    // Access selected faces (if any).
    TColStd_PackedMapOfInteger selected;
    asiEngine_Part( cmdEngine::model, cmdEngine::cf->ViewerPart->PrsMgr() ).GetHighlightedFaces(selected);

    if ( !selected.IsEmpty() )
    {
      const int          fid  = selected.GetMinimalMapped();
      const TopoDS_Face& face = partNode->GetAAG()->GetFace(fid);
      //
      if ( asiAlgo_Utils::IsPlanar(face) )
      {
        gp_Ax3 faceAx;
        asiAlgo_Utils::GetLocalFrame(face, true, faceAx);

        // B goes to global origin.
        gp_Trsf T_B;
        T_B.SetTransformation(faceAx);

        // Global origin goes to A.
        gp_Trsf T_A;
        T_A.SetTransformation( gp::XOY() );
        T_A.Invert();

        // Final transformation from B to A.
        gp_Trsf T = T_A * T_B;

        // Transform.
        BRepBuilderAPI_Transform mkTransform(shape, T, true);
        shape = mkTransform.Shape();
      }
    }
  }

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(shape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Sew(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  if ( argc != 1 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Get the sewing tolerance.
  double toler = Precision::Confusion();
  //
  interp->GetKeyValue(argc, argv, "toler", toler);

  // Sew part
  cmdEngine::model->OpenCommand();
  {
    // Perform sewing
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Sewing tolerance: %1." << toler);
    //
    TopoDS_Shape sewnPart;
    //
    if ( !asiAlgo_Utils::Sew(shape, toler, sewnPart) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Sewing failed.");
      //
      cmdEngine::model->AbortCommand();
      return TCL_ERROR;
    }
    //
    asiEngine_Part(cmdEngine::model).Update(sewnPart);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MaximizeFaces(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Modify shape.
  cmdEngine::model->OpenCommand();
  {
    if ( !asiAlgo_Utils::MaximizeFaces(shape) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face maximization failed.");
      //
      cmdEngine::model->AbortCommand();
      return TCL_ERROR;
    }
    //
    asiEngine_Part(cmdEngine::model).Update(shape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConvertToBRep(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get mesh from the Triangulation Node.
  Handle(Poly_Triangulation)
    poly = cascade::GetOpenCascadeMesh( cmdEngine::model->GetTriangulationNode()->GetTriangulation() );
  //
  if ( poly.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is empty.");
    return TCL_ERROR;
  }

  TIMER_NEW
  TIMER_GO

  // Convert.
  TopoDS_Shape result;
  //
  if ( !asiAlgo_MeshConvert::ToBRep(poly, result) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Conversion failed.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Mesh to B-rep")

  // Modify shape.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_ConvertToCanonical(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Use the default tolerance or the provided one.
  double tol = asiAlgo_CheckValidity().MaxTolerance(shape);
  //
  interp->GetKeyValue(argc, argv, "tol", tol);
  interp->GetProgress().SendLogMessage(LogNotice(Normal) << "Canonical conversion tolerance: %1."
                                                         << tol);

  // Modify shape.
  cmdEngine::model->OpenCommand();
  {
    asiAlgo_ConvertCanonicalSummary summary;
    //
    if ( !asiAlgo_Utils::ConvertCanonical( shape, tol, true, summary, interp->GetProgress() ) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face maximization failed.");
      //
      cmdEngine::model->AbortCommand();
      return TCL_ERROR;
    }
    //
    asiEngine_Part(cmdEngine::model).Update(shape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_RebuildBounds(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  (void) argc;
  (void) argv;

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();
  //
  if ( shape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is empty.");
    return TCL_ERROR;
  }

  // Rebuild bounds.
  asiAlgo_Utils::RebuildBounds(shape);

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(shape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConnectEdgesToWires(const Handle(asiTcl_Interp)& interp,
                               int                          argc,
                               const char**                 argv)
{
  (void) argc;
  (void) argv;

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();
  //
  if ( shape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is empty.");
    return TCL_ERROR;
  }

  // Get all edges from the Part and collect them to a sequence expected by ShapeAnalysis.
  const TopTools_IndexedMapOfShape& edgesMap = partNode->GetAAG()->RequestMapOfEdges();
  Handle(TopTools_HSequenceOfShape) edges    = new TopTools_HSequenceOfShape;
  //
  for ( int e = 1; e <= edgesMap.Extent(); ++e )
    edges->Append( edgesMap(e) );

  // Connect to wires.
  Handle(TopTools_HSequenceOfShape) wires;
  ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, 1e-3, 0, wires);

  // Add to the scene.
  if ( !wires.IsNull() )
  {
    for ( TopTools_HSequenceOfShape::Iterator wit(*wires); wit.More(); wit.Next() )
      interp->GetPlotter().DRAW_SHAPE( wit.Value(), Color_Red, "wire" );
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SparsePointCloud(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiData_IVPointSetNode)
    ptsNode = Handle(asiData_IVPointSetNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
  //
  if ( ptsNode.IsNull() || !ptsNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find points with the name '%1'." << argv[1]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = ptsNode->GetPoints();
  //
  if ( pts.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Selected point cloud is empty.");
    return TCL_ERROR;
  }

  // Read the tolerance.
  const double tol = Atof(argv[2]);
  //
  if ( tol < Precision::Confusion() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Too small tolerance passed.");
    return TCL_ERROR;
  }

  // Sparse point cloud.
  asiAlgo_RelievePointCloud sparse;
  Handle(asiAlgo_BaseCloud<double>) res = sparse(pts, tol);

  // Update cloud.
  interp->GetPlotter().REDRAW_POINTS(argv[1], res->GetCoordsArray(), Color_White);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ClearPart(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update( TopoDS_Shape() );
  }
  cmdEngine::model->CommitCommand();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_OrientPart(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  const bool apply = interp->HasKeyword(argc, argv, "apply");

  // Find a better orientation.
  asiAlgo_OrientCnc orient( partNode->GetAAG(),
                            interp->GetProgress(),
                            interp->GetPlotter() );
  //
  if ( !orient.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Reorientation failed.");
    return TCL_ERROR;
  }

  if ( apply )
  {
    // Make another shape.
    auto transform = BRepBuilderAPI_Transform( orient.GetTrsf() );
    transform.Perform(partNode->GetShape(), true);
    //
    const TopoDS_Shape& orientedCopy = transform.Shape();

    // Modify Data Model.
    cmdEngine::model->OpenCommand();
    {
      asiEngine_Part(cmdEngine::model).Update(orientedCopy);
    }
    cmdEngine::model->CommitCommand();

    // Update UI.
    if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
      cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);
  }

  tl::optional<gp_Ax3> ax = orient.GetAxes();

  if ( ax.has_value() )
  {
    if ( apply )
      ax->Transform( orient.GetTrsf() );

    interp->GetPlotter().REDRAW_AXES( "oriAxes",
                                       ax->Location(),
                                       ax->XDirection(),
                                       ax->YDirection(),
                                       ax->Direction(),
                                       orient.GetExtents() );
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConcatPCurves(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  // Get AAG and the active part.
  Handle(asiAlgo_AAG) aag  = partNode->GetAAG();
  TopoDS_Shape        part = partNode->GetShape();

  /* =============
   *  Read inputs.
   * ============= */

  // Read face ID.
  int fid = 0;
  //
  if ( !interp->GetKeyValue(argc, argv, "face", fid) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify the face of interest "
                                                           "using the '-face' keyword.");
    return TCL_ERROR;
  }

  // Get the number of edges in the face.
  TopTools_IndexedMapOfShape faceEdges, selectedEdges;
  TopExp::MapShapes(aag->GetFace(fid), TopAbs_EDGE, faceEdges);
  //
  const int numEdges = faceEdges.Extent();

  // Read local indices of edges.
  std::vector<int> eids;
  int eIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "edges", eIdx) )
  {
    int k = eIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int eid = atoi(argv[k]);

      if ( eid < 1 || eid > numEdges )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed edge ID %1 is out of range [1, %2]."
                                                            << eid << numEdges);
        return TCL_ERROR;
      }

      eids.push_back(eid);
    }
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify the edges to concatenate "
                                                           "using the '-edges' keyword.");
    return TCL_ERROR;
  }

  // Get edges.
  for ( const auto eid : eids )
  {
    selectedEdges.Add( faceEdges(eid).Located( TopLoc_Location() ) );
  }

  /* ===================
   *  Concatenate edges.
   * =================== */

  TopoDS_Face face = aag->GetFace(fid);

  asiAlgo_JoinEdges joiner( part, interp->GetProgress(), interp->GetPlotter() );
  //
  if ( !joiner.Perform(selectedEdges, face) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Edge concatenation failed.");
    return TCL_ERROR;
  }

  const TopoDS_Shape& result = joiner.Result();

  /* =============
   *  Update part.
   * ============= */

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part partApi(cmdEngine::model);
    //
    partApi.Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConcatCurves(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  // Get AAG and the active part.
  Handle(asiAlgo_AAG) aag  = partNode->GetAAG();
  TopoDS_Shape        part = partNode->GetShape();

  /* =============
   *  Read inputs.
   * ============= */

  // Read local indices of edges.
  std::vector<int> eids;
  int eIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "edges", eIdx) )
  {
    int k = eIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int eid = atoi(argv[k]);

      /*if ( eid < 1 || eid > 2 )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed edge ID %1 is out of range [1, %2]."
                                                            << eid);
        return TCL_ERROR;
      }*/

      eids.push_back(eid);
    }
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify the edges to concatenate "
                                                           "using the '-edges' keyword.");
    return TCL_ERROR;
  }

  // Get edges.
  TopTools_IndexedMapOfShape selectedEdges;
  auto shape = aag->GetMasterShape();
  TopTools_IndexedMapOfShape faceEdges;
  TopExp::MapShapes(shape, TopAbs_EDGE, faceEdges);
  for ( const auto eid : eids )
  {
    selectedEdges.Add( faceEdges(eid).Located( TopLoc_Location() ) );
  }

  /* ===================
   *  Concatenate edges.
   * =================== */

  asiAlgo_Join3dEdges joiner( part, interp->GetProgress(), interp->GetPlotter() );
  //
  if ( !joiner.Perform(selectedEdges) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Edge concatenation failed.");
    return TCL_ERROR;
  }

  const TopoDS_Shape& result = joiner.Result();

  /* =============
   *  Update part.
   * ============= */

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part partApi(cmdEngine::model);
    //
    partApi.Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConcatAndCheck(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  // Get AAG and the active part.
  Handle(asiAlgo_AAG) aag  = partNode->GetAAG();
  TopoDS_Shape        part = partNode->GetShape();

  /* =============
  *  Read inputs.
  * ============= */

  // Read local indices of edges.
  std::vector<int> eids;
  int eIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "edges", eIdx) )
  {
    int k = eIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int eid = atoi(argv[k]);

      /*if ( eid < 1 || eid > 2 )
      {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed edge ID %1 is out of range [1, %2]."
      << eid);
      return TCL_ERROR;
      }*/

      eids.push_back(eid);
    }
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify the edges to concatenate "
      "using the '-edges' keyword.");
    return TCL_ERROR;
  }

  // Get edges.
  TopTools_IndexedMapOfShape selectedEdges;
  auto shape = aag->GetMasterShape();
  TopTools_IndexedMapOfShape faceEdges;
  TopExp::MapShapes(shape, TopAbs_EDGE, faceEdges);
  for ( const auto eid : eids )
  {
    selectedEdges.Add( faceEdges(eid).Located( TopLoc_Location() ) );
  }

  /* ===================
  *  Concatenate edges.
  * =================== */

  asiAlgo_Join3dEdges joiner( part, interp->GetProgress(), interp->GetPlotter() );
  //
  if ( !joiner.Perform(selectedEdges) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Edge concatenation failed.");
    return TCL_ERROR;
  }

  const TopoDS_Shape& result = joiner.Result();

  /* =============
  *  Check part.
  * ============= */

  Handle(asiAlgo_AAG) newAag = new asiAlgo_AAG(result);
  if (aag->RequestMapOfEdges().Size() - 1 != newAag->RequestMapOfEdges().Size()) {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Wrong number of edges.");
    return TCL_ERROR;
  }

  /* =============
  *  Update part.
  * ============= */

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part partApi(cmdEngine::model);
    //
    partApi.Update(result);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConvertToC2(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Assuming that `argv[1]` is UTF-8.
  TCollection_ExtendedString name(argv[1], true);

  // Get node.
  Handle(asiData_IVSurfaceNode)
    surfNode = Handle(asiData_IVSurfaceNode)::DownCast( cmdEngine::model->FindNodeByName(name) );
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast( cmdEngine::model->FindNodeByName(name) );
  //
  if ( surfNode.IsNull() && curveNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find geometric object with the name %1." << name);
    return TCL_ERROR;
  }

  // Read conversion tolerance.
  double tol = 1e10;
  interp->GetKeyValue(argc, argv, "tol", tol);

  // Whether to add knots.
  const bool addKnots = interp->HasKeyword(argc, argv, "add-knots");

  // Max achieved deviation.
  double maxDev = 0;

  /* ====================
   *  Surface conversion.
   * ==================== */

  if ( !surfNode.IsNull() )
  {
    // Get parametric surface.
    Handle(Geom_Surface) surf = surfNode->GetSurface();
    //
    if ( surf.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target surface is null.");
      return TCL_ERROR;
    }

    // The input surface has to be a B-spline surface.
    Handle(Geom_BSplineSurface) bsurfInit = Handle(Geom_BSplineSurface)::DownCast( surf );
    Handle(Geom_BSplineSurface) bsurfCopy = Handle(Geom_BSplineSurface)::DownCast( surf->Copy() );
    //
    if ( bsurfCopy.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target surface is not a B-spline surface.");
      return TCL_ERROR;
    }

    if ( addKnots )
    {
      const TColStd_Array1OfReal& U  = bsurfInit->UKnots();
      const TColStd_Array1OfReal& V  = bsurfInit->VKnots();

      for ( int ii = 1; ii < U.Length(); ++ii )
      {
        const double u_prev = U(ii);
        const double u_next = U(ii + 1);

        bsurfCopy->InsertUKnot( (3*u_prev +   u_next)*0.25, 1, Precision::PConfusion() );
        bsurfCopy->InsertUKnot( (  u_prev + 3*u_next)*0.25, 1, Precision::PConfusion() );
      }

      for ( int ii = 1; ii < V.Length(); ++ii )
      {
        const double v_prev = V(ii);
        const double v_next = V(ii + 1);

        bsurfCopy->InsertVKnot( (3*v_prev +   v_next)*0.25, 1, Precision::PConfusion() );
        bsurfCopy->InsertVKnot( (  v_prev + 3*v_next)*0.25, 1, Precision::PConfusion() );
      }
    }

    // Simplify.
    ::SimplifySurface(bsurfCopy, tol, 1);
    //
    interp->GetPlotter().REDRAW_SURFACE(name + "_C2", bsurfCopy, Color_Default);

    // Measure deviation.
    asiAlgo_AppSurfUtils::MeasureDeviation( bsurfCopy, bsurfInit, maxDev, interp->GetPlotter() );
  }

  /* ==================
   *  Curve conversion.
   * ================== */

  if ( !curveNode.IsNull() )
  {
    // Get parametric curve.
    Handle(Geom_Curve) curve = curveNode->GetCurve();
    //
    if ( curve.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target curve is null.");
      return TCL_ERROR;
    }

    // The input curve has to be a B-spline curve.
    Handle(Geom_BSplineCurve) bcurveInit = Handle(Geom_BSplineCurve)::DownCast( curve );
    Handle(Geom_BSplineCurve) bcurveCopy = Handle(Geom_BSplineCurve)::DownCast( curve->Copy() );
    //
    if ( bcurveCopy.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target curve is not a B-spline curve.");
      return TCL_ERROR;
    }

    if ( addKnots )
    {
      const TColStd_Array1OfReal& U = bcurveInit->Knots();

      for ( int ii = 1; ii < U.Length(); ++ii )
      {
        const double u_prev = U(ii);
        const double u_next = U(ii + 1);

        bcurveCopy->InsertKnot( (3*u_prev +   u_next)*0.25, 1, Precision::PConfusion() );
        bcurveCopy->InsertKnot( (  u_prev + 3*u_next)*0.25, 1, Precision::PConfusion() );
      }
    }

    // Simplify.
    ::SimplifyCurve(bcurveCopy, tol, 1);
    //
    interp->GetPlotter().REDRAW_CURVE(name + "_C2", bcurveCopy, Color_Default, true);

    // Measure deviation.
    asiAlgo_AppSurfUtils::MeasureDeviation( bcurveCopy, bcurveInit, maxDev, interp->GetPlotter() );
  }

  (*interp) << maxDev;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ConvertToBezier(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Assuming that `argv[1]` is UTF-8.
  TCollection_ExtendedString name(argv[1], true);

  // Get node.
  Handle(asiData_IVSurfaceNode)
    surfNode = Handle(asiData_IVSurfaceNode)::DownCast( cmdEngine::model->FindNodeByName(name) );
  //
  Handle(asiData_IVCurveNode)
    curveNode = Handle(asiData_IVCurveNode)::DownCast( cmdEngine::model->FindNodeByName(name) );
  //
  if ( surfNode.IsNull() && curveNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find geometric object with the name %1." << name);
    return TCL_ERROR;
  }

  // Whether to approximate.
  const bool toApprox = interp->HasKeyword(argc, argv, "approx");

  // Prepare the converter.
  asiAlgo_ConvertToBezier converter( interp->GetProgress(),
                                     interp->GetPlotter() );

  /* ====================
   *  Surface conversion.
   * ==================== */

  if ( !surfNode.IsNull() )
  {
    // Get parametric surface.
    Handle(Geom_Surface) surf = surfNode->GetSurface();
    //
    if ( surf.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target surface is null.");
      return TCL_ERROR;
    }

    // Check if the surface is untrimmed and make sure to trim it on a finite rectangle,
    // so that NURBS conversion can handle it.
    // Check for infinite bounds.
    double uMin, uMax, vMin, vMax;
    surf->Bounds(uMin, uMax, vMin, vMax);
    //
    if ( Precision::IsInfinite(uMin) || Precision::IsInfinite(uMax) ||
         Precision::IsInfinite(vMin) || Precision::IsInfinite(vMax) )
    {
      surfNode->GetLimits(uMin, uMax, vMin, vMax);
      //
      surf = new Geom_RectangularTrimmedSurface(surf, uMin, uMax, vMin, vMax);
    }

    // Convert.
    Handle(Geom_Surface) converted = converter.Perform(surf, toApprox);
    //
    if ( converted.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Conversion failed.");
      return TCL_ERROR;
    }

    interp->GetPlotter().REDRAW_SURFACE(name + "_B", converted, Color_Default);
    return TCL_OK;
  }

  /* ==================
   *  Curve conversion.
   * ================== */

  if ( !curveNode.IsNull() )
  {
    // Get parametric curve.
    Handle(Geom_Curve) curve = curveNode->GetCurve();
    //
    if ( curve.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Target curve is null.");
      return TCL_ERROR;
    }

    // Convert.
    Handle(Geom_Curve) converted = converter.Perform(curve);
    //
    if ( converted.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Conversion failed.");
      return TCL_ERROR;
    }

    interp->GetPlotter().REDRAW_CURVE(name + "_B", converted, Color_Default, true);
    return TCL_OK;
  }

  return TCL_ERROR; // Should never happen.
}

//-----------------------------------------------------------------------------

void cmdEngine::Commands_Editing(const Handle(asiTcl_Interp)&      interp,
                                 const Handle(Standard_Transient)& cmdEngine_NotUsed(data))
{
  static const char* group = "cmdEngine";

  //-------------------------------------------------------------------------//
  interp->AddCommand("kev",
    //
    "kev {<edgeIndex> <vertexIndex> | -edge <edgeName> -vertex <vertexName> | -name <edgeName>}\n"
    "\t KEV (Kill-Edge-Vertex) Euler operator.",
    //
    __FILE__, group, ENGINE_KEV);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kef",
    //
    "kef {<edgeIndex> <faceIndex> | -edge <edgeName> -face <faceName>}\n"
    "\t KEF (Kill-Edge-Face) Euler operator.",
    //
    __FILE__, group, ENGINE_KEF);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kfmv",
    //
    "kfmv {<faceIndex> | -name <faceName>}\n"
    "\t KFMV (Kill-Face-Make-Vertex) Euler operator.",
    //
    __FILE__, group, ENGINE_KFMV);

  //-------------------------------------------------------------------------//
  interp->AddCommand("merge-vertices",
    //
    "merge-vertices -name <vertexName1> -name <vertexName2> [... -name <vertexNameK>]\n"
    "\t Merges several vertices into one.",
    //
    __FILE__, group, ENGINE_MergeVertices);

  //-------------------------------------------------------------------------//
  interp->AddCommand("merge-edges",
    //
    "merge-edges -name <edgeName1> -name <edgeName2> [... -name <edgeNameK>]\n"
    "\t Merges several edges into one.",
    //
    __FILE__, group, ENGINE_MergeEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-vertex",
    //
    "kill-vertex {<vertexIndex> | -name <vertexName>}\n"
    "\t Kills vertex with the passed 1-based index from the active part.\n"
    "\t This is a pure topological operation which does not attempt to\n"
    "\t modify geometry. Moreover, unlike Euler operator, this function\n"
    "\t does not preserve the topological consistency of the CAD part.\n"
    "\t We have introduced this function to ground Euler operators on it.",
    //
    __FILE__, group, ENGINE_KillVertex);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-edge",
    //
    "kill-edge {<edgeIndex> | -name <edgeName>}\n"
    "\t Kills edge with the passed 1-based index from the active part.\n"
    "\t This is a pure topological operation which does not attempt to\n"
    "\t modify geometry. Moreover, unlike Euler operator, this function\n"
    "\t does not preserve the topological consistency of the CAD part.\n"
    "\t We have introduced this function to ground Euler operators on it.",
    //
    __FILE__, group, ENGINE_KillEdge);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-face",
    //
    "kill-face {<fid> | -name <faceName>} [-defeat]\n"
    "\t Kills the face with the passed 1-based index from the active part.\n"
    "\t This is a pure topological operation which does not attempt to\n"
    "\t modify geometry. Moreover, unlike Euler operators, this function\n"
    "\t does not preserve the topological consistency of the CAD part.\n"
    "\t We have introduced this function to ground Euler operators on it.\n"
    "\n"
    "\t If the '-defeat' key is passed, another algorithm of smart face removal\n"
    "\t will be used. The 'smart' algorithm not only removes a face but also\n"
    "\t stitches the neighbor faces to produce a watertight solid model as a result.",
    //
    __FILE__, group, ENGINE_KillFace);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-faces",
    //
    "kill-faces <fid1> [<fid2> ...]\n"
    "\t Kills the faces with the given 1-based indices.",
    //
    __FILE__, group, ENGINE_KillFaces);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-solid-by-face",
    //
    "kill-solid-by-face <faceIndex>\n"
    "\t Kills a solid which contains a face with the passed 1-based index.",
    //
    __FILE__, group, ENGINE_KillSolidByFace);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-subshape",
    //
    "kill-subshape -gid <num>\n"
    "\t Kills a subshape having the passed global identifier.",
    //
    __FILE__, group, ENGINE_KillSubShape);

  //-------------------------------------------------------------------------//
  interp->AddCommand("move-by-face",
    //
    "move-by-face <faceIndex> <offset>\n"
    "\t Moves part in direction determined by orientation of the given face\n."
    "\t The passed offset value can be positive or negative. In the case of\n"
    "\t positive offset, the movement is done along the face normal. In the\n"
    "\t case of negative offset, the direction of movement is reversed.\n"
    "\n"
    "\t CAUTION: this command makes deep copy of the part. Our experience shows\n"
    "\t that simply assigning location may cause very weird behaviour on subsequent\n"
    "\t modeling operations.",
    //
    __FILE__, group, ENGINE_MoveByFace);

  //-------------------------------------------------------------------------//
  interp->AddCommand("imprint",
    //
    "imprint <varName>\n"
    "\t Imprints the passed topological object to the working part.\n"
    "\t This operation is essenially the General Fuse algorithm of OpenCascde.",
    //
    __FILE__, group, ENGINE_Imprint);

  //-------------------------------------------------------------------------//
  interp->AddCommand("repair",
    //
    "repair\n"
    "\t Performs automatic shape repair on the active part.",
    //
    __FILE__, group, ENGINE_Repair);

  //-------------------------------------------------------------------------//
  interp->AddCommand("repair-shifted",
    //
    "repair-shifted\n"
    "\t Performs automatic repair of shifted pcurves on the active part.",
    //
    __FILE__, group, ENGINE_RepairShifted);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-face-tolerance",
    //
    "set-face-tolerance <faceId> <toler>\n"
    "\t Forces the face with the given index to have the passed tolerance.\n"
    "\t In OpenCascade, there is a rule that a tolerance of a face should not be\n"
    "\t greater than the tolerances of its edges (the same rule applies for\n"
    "\t edges and vertices). Therefore, this function updates tolerance not\n"
    "\t only for the face in question, but also for its sub-shapes.",
    //
    __FILE__, group, ENGINE_SetFaceTolerance);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-tolerance",
    //
    "set-tolerance <toler>\n"
    "\t Forces the part to have the passed tolerance in all its sub-shapes.",
    //
    __FILE__, group, ENGINE_SetTolerance);

  //-------------------------------------------------------------------------//
  interp->AddCommand("rebuild-edge",
    //
    "rebuild-edge {<edgeIndex> | -name <edgeName>}\n"
    "\t Rebuilds edge with the given ID or name.",
    //
    __FILE__, group, ENGINE_RebuildEdge);

  //-------------------------------------------------------------------------//
  interp->AddCommand("fair-curve",
    //
    "fair-curve <resName> <curveName> <fairingCoeff>\n"
    "\t Fairs curve with the given name <curveName>. The passed fairing coefficient\n"
    "\t is a weight of a fairing term in the goal function.",
    //
    __FILE__, group, ENGINE_FairCurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("fair-surf",
    //
    "fair-surf <resName> <surfName> <fairingCoeff> [-relax]\n"
    "\t Fairs surface with the given name <surfName>. The passed fairing coefficient\n"
    "\t is a weight of a fairing term in the goal function. If the '-relax' key is passed,\n"
    "\t the natural boundaries of the surface will remain unconstrained.",
    //
    __FILE__, group, ENGINE_FairSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("split-by-continuity",
    //
    "split-by-continuity {c0 | c1} [<toler>]\n"
    "\t Splits part by continuity.",
    //
    __FILE__, group, ENGINE_SplitByContinuity);

  //-------------------------------------------------------------------------//
  interp->AddCommand("split-closed",
    //
    "split-closed\n"
    "\t Splits closed faces in the active part.",
    //
    __FILE__, group, ENGINE_SplitClosed);

  //-------------------------------------------------------------------------//
  interp->AddCommand("extend-surf",
    //
    "extend-surf <resName> <surfName> <length> {-umax | -umin | -vmax | -vmin}\n"
    "\t Extends surface <surfName> by length <length> in the given\n"
    "\t parameter's direction.",
    //
    __FILE__, group, ENGINE_ExtendSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("trim-curve",
    //
    "trim-curve <resName> <curveName> <u0> <u1>\n"
    "\t Trims curve with the given name <curveName> by the passed\n"
    "\t parameter values <u0> and <u1>.",
    //
    __FILE__, group, ENGINE_TrimCurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("invert-shells",
    //
    "invert-shells\n"
    "\t Inverts orientations of all shells in the working part.",
    //
    __FILE__, group, ENGINE_InvertShells);

  //-------------------------------------------------------------------------//
  interp->AddCommand("heal-small-edges",
    //
    "heal-small-edges <len> <ang_deg>\n"
    "\t Heals (merges) small edges in the part. The argument <len> specifies\n"
    "\t the linear threshold for merge. The argument <ang_deg> specifies the\n"
    "\t the angular threshold for merge (in degrees).",
    //
    __FILE__, group, ENGINE_HealSmallEdges);

  //-------------------------------------------------------------------------//
  interp->AddCommand("repatch",
    //
    "repatch [<fid1> [<fid2> [...]]] [-lambda <coeff>]\n"
    "\t Repatches faces which are selected interactively or specified as\n"
    "\t arguments (1-based face IDs).",
    //
    __FILE__, group, ENGINE_Repatch);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-blend",
    //
    "kill-blend [<fid>]\n"
    "\t Attempts to defeature a fillet starting from the face selected\n"
    "\t interactively or specified as 1-based face ID.",
    //
    __FILE__, group, ENGINE_KillBlend);

  //-------------------------------------------------------------------------//
  interp->AddCommand("kill-blends-inc",
    //
    "kill-blends-inc [<radius>]\n"
    "\t Attempts to defeature all blends incrementally.",
    //
    __FILE__, group, ENGINE_KillBlendsInc);

  //-------------------------------------------------------------------------//
  interp->AddCommand("insert-knot-curve",
    //
    "insert-knot-curve <resName> <curveName> <u> [<r>]\n"
    "\t Inserts the passed knot value <u> <r> times to the\n"
    "\t B-curve <curveName>. A new curve <resName> is created as a result.",
    //
    __FILE__, group, ENGINE_InsertKnotCurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("insert-knot-surf-u",
    //
    "insert-knot-surf-u <resName> <surfName> <u> [<r>]\n"
    "\t Inserts the passed knot value <u> <r> times to the\n"
    "\t B-surface <surfName> in its U parametric direction.\n"
    "\t A new surface <resName> is created as a result.",
    //
    __FILE__, group, ENGINE_InsertKnotSurfU);

  //-------------------------------------------------------------------------//
  interp->AddCommand("insert-knots-surf-u",
    //
    "insert-knots-surf-u <resName> <surfName> <num>\n"
    "\t Inserts the <num> uniformly distributed knots to the surface <surfName>.\n"
    "\t The new surface is created with the <resName> name.",
    //
    __FILE__, group, ENGINE_InsertKnotsSurfU);

  //-------------------------------------------------------------------------//
  interp->AddCommand("insert-knot-surf-v",
    //
    "insert-knot-surf-v <resName> <surfName> <v> [<r>]\n"
    "\t Inserts the passed knot value <v> <r> times to the\n"
    "\t B-surface <surfName> in its V parametric direction.\n"
    "\t A new surface <resName> is created as a result.",
    //
    __FILE__, group, ENGINE_InsertKnotSurfV);

  //-------------------------------------------------------------------------//
  interp->AddCommand("insert-knots-surf-v",
    //
    "insert-knots-surf-v <resName> <surfName> <num>\n"
    "\t Inserts the <num> uniformly distributed knots to the surface <surfName>.\n"
    "\t The new surface is created with the <resName> name.",
    //
    __FILE__, group, ENGINE_InsertKnotsSurfV);

  //-------------------------------------------------------------------------//
  interp->AddCommand("exchange-uv",
    //
    "exchange-uv <surfName>\n"
    "\t Flips parameterization of the given B-surface.",
    //
    __FILE__, group, ENGINE_ExchangeUV);

  //-------------------------------------------------------------------------//
  interp->AddCommand("split-curve-bezier",
    //
    "split-curve-bezier <resName> <curveName>\n"
    "\t Splits the passed B-curve <curveName> to a series of Bezier curves.",
    //
    __FILE__, group, ENGINE_SplitCurveBezier);

  //-------------------------------------------------------------------------//
  interp->AddCommand("move-point-curve",
    //
    "move-point-curve <curveName> <u> <dx> <dy> <dz>\n"
    "\t Moves point on B-curve with parameter <u> by vector (<dx>, <dy>, <dz>).",
    //
    __FILE__, group, ENGINE_MovePointCurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("move-triangulation",
    //
    "move-triangulation <tx> <ty> <tz> <rx> <ry> <rz>\n"
    "\t Moves triangulation by applying the passed transformation. The values\n"
    "\t <tx>, <ty>, <tx> define the translation vector. The values <rx>, <ry>, <rz>\n"
    "\t define the rotation angles in degrees. Rotation is performed with respect\n"
    "\t to the global X, Y, Z axes.",
    //
    __FILE__, group, ENGINE_MoveTriangulation);

  //-------------------------------------------------------------------------//
  interp->AddCommand("move-part",
    //
    "move-part <tx> <ty> <tz> <rx> <ry> <rz> [-isGeometric]\n"
    "\t In case the '-isGeometric' key is not set, moves part by applying the passed\n"
    "\t transformation to the location of part. In case the '-isGeometric' key is set,\n"
    "\t moves part by applying the passed transformation on its deep copy. The values\n"
    "\t <tx>, <ty>, <tx> define the translation vector. The values <rx>, <ry>, <rz>\n"
    "\t define the rotation angles in degrees. Rotation is performed with respect\n"
    "\t to the global X, Y, Z axes.",
    //
    __FILE__, group, ENGINE_MovePart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("convert-to-bsurf",
    //
    "convert-to-bsurf [<res> <surfName> [-cubic]]\n"
    "\t Converts the surface with the given name to B-surface. The trimming\n"
    "\t parameters are taken from the corresponding Data Node. If no surface\n"
    "\t is specified, the entire part is converted.",
    //
    __FILE__, group, ENGINE_ConvertToBSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("convert-plane-to-bsurf",
    //
    "convert-plane-to-bsurf <res> <surfName> <uDeg> <vDeg>\n"
    "\t Converts the planar surface with the given name to a B-spline surface. The trimming\n"
    "\t parameters are taken from the corresponding Data Node. The desired degrees\n"
    "\t are specified via <uDeg> and <vDeg> arguments.",
    //
    __FILE__, group, ENGINE_ConvertPlaneToBSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("rotate-qn",
    //
    "rotate-qn <Vx> <Vy> <Vz> <angleDeg>\n"
    "\t Rotates CAD part applying the passed quaternion defined by the given\n"
    "\t axis of rotation (Vx, Vy, Vz) and the angle in degrees.",
    //
    __FILE__, group, ENGINE_RotateQn);

  //-------------------------------------------------------------------------//
  interp->AddCommand("reset-location",
    //
    "reset-location\n"
    "\t Resets location associated with the part shape. If any planar face is\n"
    "\t selected, the part will be moved to the global origin, so that the local\n"
    "\t axes of the selected face match the global scene's axes.",
    //
    __FILE__, group, ENGINE_ResetLocation);

  //-------------------------------------------------------------------------//
  interp->AddCommand("sew",
    //
    "sew [-toler <toler>]\n"
    "\t Stitches faces of the active part using the passed tolerance <toler>\n"
    "\t that indicates the max allowed gaps between the edges. If no tolerance\n"
    "\t is passed, the default value 1.e-7 is used.",
    //
    __FILE__, group, ENGINE_Sew);

  //-------------------------------------------------------------------------//
  interp->AddCommand("maximize-faces",
    //
    "maximize-faces\n"
    "\t Maximizes canonical faces.",
    //
    __FILE__, group, ENGINE_MaximizeFaces);

  //-------------------------------------------------------------------------//
  interp->AddCommand("convert-to-brep",
    //
    "convert-to-brep\n"
    "\t Converts triangulation to B-rep. The triangles are mapped one-by-one\n"
    "\t to the faces of the resulting model.",
    //
    __FILE__, group, ENGINE_ConvertToBRep);

  //-------------------------------------------------------------------------//
  interp->AddCommand("convert-to-canonical",
    //
    "convert-to-canonical [-tol <tol>]\n"
    "\t Attempts to convert a shape to a canonical form.",
    //
    __FILE__, group, ENGINE_ConvertToCanonical);

  //-------------------------------------------------------------------------//
  interp->AddCommand("rebuild-bounds",
    //
    "rebuild-bounds\n"
    "\t Rebuilds 3D representations of edges from their pcurves.",
    //
    __FILE__, group, ENGINE_RebuildBounds);

  //-------------------------------------------------------------------------//
  interp->AddCommand("connect-edges-to-wires",
    //
    "connect-edges-to-wires\n"
    "\t Constructs as many wires as possible from the active set of edges taken\n"
    "\t from the Part Node.",
    //
    __FILE__, group, ENGINE_ConnectEdgesToWires);

  //-------------------------------------------------------------------------//
  interp->AddCommand("sparse-point-cloud",
    //
    "sparse-point-cloud <name> <tol>\n"
    "\t Sparses the point cloud with the name <name> using the given spatial tolerance <tol>.",
    //
    __FILE__, group, ENGINE_SparsePointCloud);

  //-------------------------------------------------------------------------//
  interp->AddCommand("clear-part",
    //
    "clear-part\n"
    "\t Resets part shape to empty state.",
    //
    __FILE__, group, ENGINE_ClearPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("orient-part",
    //
    "orient-part [-apply]\n"
    "\t Reorients the active part so it gets more suitable axes frame.\n"
    "\t Pass the '-apply' flag to change the transformation of the shape.\n"
    "\t Otherwise, only local axes are detected but the shape is not changed.",
    //
    __FILE__, group, ENGINE_OrientPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("concat-pcurves",
    //
    "concat-pcurves -face <fid> -edges <e1> <e2>\n"
    "\t Concatenates edges <e1> and <e2> belonging to the face <fid> into\n"
    "\t a single edge. The indices <e1> and <e2> are defined locally and thereby\n"
    "\t range from 1 to the number of edges in the face. The concatenation process\n"
    "\t is done at the level of pcurves.",
    //
    __FILE__, group, ENGINE_ConcatPCurves);

  //-------------------------------------------------------------------------//
  interp->AddCommand("concat-curves",
    //
    "concat-curves -edges <e1> <e2>\n"
    "\t Concatenates edges <e1> and <e2> belonging to the face <fid> into\n"
    "\t a single edge. The indices <e1> and <e2> are defined locally and thereby\n"
    "\t range from 1 to the number of edges in the face. The concatenation process\n"
    "\t is done at the level of pcurves.",
    //
    __FILE__, group, ENGINE_ConcatCurves);

  //-------------------------------------------------------------------------//
  interp->AddCommand("concat-curves-and-check",
    //
    "concat-curves-and-check -edges <e1> <e2>\n"
    "\t Concatenates edges <e1> and <e2> belonging to the face <fid> into\n"
    "\t a single edge. The indices <e1> and <e2> are defined locally and thereby\n"
    "\t range from 1 to the number of edges in the face. The concatenation process\n"
    "\t is done at the level of pcurves.",
    //
    __FILE__, group, ENGINE_ConcatAndCheck);

  //-------------------------------------------------------------------------//
  interp->AddCommand("convert-to-c2",
    //
    "convert-to-c2 <name> [-tol <tol>] [-add-knots]\n"
    "\t Converts the curve or surface named <name> to C2 continuity class if possible.\n"
    "\t You can pass the tolerance value via the '-tol' key to protect knot removal\n"
    "\t operation from distorting the input geometry too much.",
    //
    __FILE__, group, ENGINE_ConvertToC2);

  //-------------------------------------------------------------------------//
  interp->AddCommand("convert-to-bezier",
    //
    "convert-to-c2 <name> [-toApprox]\n"
    "\t Converts the curve or surface named <name> to C2 continuity class if possible.\n"
    "\t You can pass the tolerance value via the '-tol' key to protect knot removal\n"
    "\t operation from distorting the input geometry too much.",
    //
    __FILE__, group, ENGINE_ConvertToBezier);
}
