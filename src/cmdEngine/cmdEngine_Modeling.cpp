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

// asiUI includes
#include <asiUI_DialogGeomDef.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiAlgo includes
#include <asiAlgo_BuildGordonSurf.h>
#include <asiAlgo_BuildHLR.h>
#include <asiAlgo_BuildOBB.h>
#include <asiAlgo_JoinSurf.h>
#include <asiAlgo_MeshOBB.h>
#include <asiAlgo_MeshOffset.h>
#include <asiAlgo_BuildOptBoundingCyl.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_UntrimSurf.h>
#include <asiAlgo_Utils.h>

#if defined USE_MOBIUS
  // Mobius includes
  #include <mobius/cascade_BSplineCurve.h>
  #include <mobius/geom_FairBCurve.h>

  using namespace mobius;
#endif

// OCCT includes
#include <BOPAlgo_Splitter.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffset.hxx>
#include <BRepOffset_MakeOffset.hxx>
#include <BRepOffset_MakeSimpleOffset.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <Geom_Plane.hxx>
#include <gp_Pln.hxx>
#include <Precision.hxx>
#include <ShapeCustom.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

// VTK includes
#pragma warning(push, 0)
#include <vtkCamera.h>
#pragma warning(pop)

//-----------------------------------------------------------------------------

bool MakeOffset(const TopoDS_Shape&  shape,
                const double         offsetVal,
                const bool           isSimple,
                const bool           isSolid,
                const double         toler,
                ActAPI_ProgressEntry progress,
                TopoDS_Shape&        offsetShape)
{
  if ( isSimple )
  {
    // Initialize
    BRepOffset_MakeSimpleOffset mkOffset;
    mkOffset.Initialize(shape, offsetVal);
    mkOffset.SetBuildSolidFlag(isSolid);
    //
    if ( toler )
      mkOffset.SetTolerance(toler);

    // Perform
    mkOffset.Perform();
    //
    if ( !mkOffset.IsDone() )
    {
      progress.SendLogMessage(LogErr(Normal) << "Simple offset not done.");
      return false;
    }
    offsetShape = mkOffset.GetResultShape();
  }
  else
  {
    BRepOffset_MakeOffset mkOffset;
    mkOffset.Initialize(shape, offsetVal, 1.0e-3, BRepOffset_Skin, true, false, GeomAbs_Arc, isSolid);
    //
    if ( isSolid )
      mkOffset.MakeThickSolid();
    else
      mkOffset.MakeOffsetShape();
    //
    if ( !mkOffset.IsDone() )
    {
      progress.SendLogMessage(LogErr(Normal) << "Offset not done.");
      return false;
    }
    offsetShape = mkOffset.Shape();
  }

  return true;
}

//-----------------------------------------------------------------------------

int ENGINE_OffsetShell(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc > 7 || argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get part shape
  TopoDS_Shape
    partShape = Handle(asiEngine_Model)::DownCast( interp->GetModel() )->GetPartNode()->GetShape();

  // Get offset value
  const double offsetVal = atof(argv[1]);

  // Check whether topology of the result should be preserved
  bool isSimple = false, isSolid = false, isKeep = false;
  if ( argc > 2 )
  {
    for ( int k = 2; k < argc; ++k )
    {
      if ( !isSimple && interp->IsKeyword(argv[k], "simple") )
        isSimple = true;

      if ( !isSolid && interp->IsKeyword(argv[k], "solid") )
        isSolid = true;

      if ( !isKeep && interp->IsKeyword(argv[k], "keep") )
        isKeep = true;
    }
  }

  // Take tolerance for suppressing singularities
  double toler = 0.0;
  TCollection_AsciiString tolerStr;
  //
  if ( interp->GetKeyValue(argc, argv, "toler", tolerStr) )
  {
    toler = tolerStr.RealValue();

    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Passed tolerance value %1."
                                                         << toler);
  }

  const bool isByFaces = interp->HasKeyword(argc, argv, "faces");

  // Make offset
  TopoDS_Shape offsetShape;
  //
  if ( isByFaces )
  {
    TopoDS_Compound offsetShapeComp;
    BRep_Builder().MakeCompound(offsetShapeComp);
    //
    for ( TopExp_Explorer fexp(partShape, TopAbs_FACE); fexp.More(); fexp.Next() )
    {
      TopoDS_Shape offsetFace;
      TopoDS_Shape baseFace = BRepBuilderAPI_Copy( fexp.Current() ); // To detach face.

      if ( !MakeOffset(baseFace,
                       offsetVal,
                       isSimple,
                       isSolid,
                       toler,
                       interp->GetProgress(),
                       offsetFace) )
        continue;

      BRep_Builder().Add(offsetShapeComp, offsetFace);
    }
    //
    offsetShape = offsetShapeComp;
  }
  else
  {
    if ( !MakeOffset(partShape,
                     offsetVal,
                     isSimple,
                     isSolid,
                     toler,
                     interp->GetProgress(),
                     offsetShape) )
    {
      return TCL_ERROR;
    }
  }

  TopoDS_Shape resultShape;

  // If this flag is enabled, the initial geometry is not substituted
  if ( isKeep )
  {
    TopoDS_Compound resComp;
    BRep_Builder().MakeCompound(resComp);
    //
    BRep_Builder().Add(resComp, partShape);
    BRep_Builder().Add(resComp, offsetShape);
    //
    resultShape = resComp;
  }
  else
    resultShape = offsetShape;

  // Modify Data Model
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(resultShape);
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_OffsetTess(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get mesh.
  Handle(asiData_TessNode) tessNode = cmdEngine::model->GetTessellationNode();
  //
  if ( tessNode.IsNull() || !tessNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Tessellation Node is null or inconsistent.");
    return TCL_ERROR;
  }
  //
  Handle(ActData_Mesh) mesh = tessNode->GetMesh();

  // Get offset value.
  const double offsetVal = atof(argv[1]);

  // Perform mesh offset.
  asiAlgo_MeshOffset meshOffset( mesh, interp->GetProgress(), interp->GetPlotter() );
  meshOffset.Perform(offsetVal);

  // Update Data Model.
  cmdEngine::model->OpenCommand(); // tx start
  {
    tessNode->SetMesh( meshOffset.GetResult() );
  }
  cmdEngine::model->CommitCommand(); // tx commit

  // Update UI.
  cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(tessNode.get(), false, false);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeVertex(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  std::vector<TopoDS_Vertex> Vs;

  if ( argc == 3 )
  {
    // Get Points Node.
    Handle(asiData_IVPointSetNode)
      node = Handle(asiData_IVPointSetNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find Points object with name %1." << argv[2]);
      return TCL_OK;
    }

    Handle(asiAlgo_BaseCloud<double>) pts = node->GetPoints();

    // Make vertex.
    for ( int ipt = 0; ipt < pts->GetNumberOfElements(); ++ipt )
    {
      Vs.push_back( BRepBuilderAPI_MakeVertex( pts->GetElement(ipt) ) );
    }
  }

  // Draw in IV.
  for ( const auto& V : Vs )
  {
    interp->GetPlotter().DRAW_SHAPE(V, Color_Default, 1.0, true, argv[1]);
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeEdge(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 3 && argc != 6 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TopoDS_Edge E;

  if ( argc == 3 )
  {
    // Get Curve Node.
    Handle(asiData_IVCurveNode)
      node = Handle(asiData_IVCurveNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a Curve object with name %1." << argv[2]);
      return TCL_OK;
    }

    // Get geometry of a curve.
    double f, l;
    Handle(Geom_Curve) curve = node->GetCurve(f, l);
    //
    if ( curve.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Curve in question is null.");
      return TCL_OK;
    }

    // Make edge.
    E = BRepBuilderAPI_MakeEdge(curve);
  }
  else if ( argc == 6 )
  {
    std::string v1Name, v2Name;

    if ( !interp->GetKeyValue(argc, argv, "p1", v1Name) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "No -p1 argument is passed.");
      return TCL_OK;
    }

    if ( !interp->GetKeyValue(argc, argv, "p2", v2Name) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "No -p2 argument is passed.");
      return TCL_OK;
    }

    // Get Point Nodes.
    Handle(asiData_IVPointSetNode)
      p1Node = Handle(asiData_IVPointSetNode)::DownCast( cmdEngine::model->FindNodeByName( v1Name.c_str() ) );
    Handle(asiData_IVPointSetNode)
      p2Node = Handle(asiData_IVPointSetNode)::DownCast( cmdEngine::model->FindNodeByName( v2Name.c_str() ) );
    //
    if ( p1Node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a point with the name %1." << v1Name);
      return TCL_ERROR;
    }
    //
    if ( p2Node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a point with the name %1." << v2Name);
      return TCL_ERROR;
    }

    Handle(asiAlgo_BaseCloud<double>) p1Set = p1Node->GetPoints();
    Handle(asiAlgo_BaseCloud<double>) p2Set = p2Node->GetPoints();

    gp_XYZ P1 = p1Set->GetElement(0);
    gp_XYZ P2 = p2Set->GetElement(0);

    E = BRepBuilderAPI_MakeEdge(P1, P2);
  }

  // Draw in IV.
  if ( !E.IsNull() )
  {
    interp->GetPlotter().REDRAW_SHAPE(argv[1], E, Color_Red, 1.0, true);
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot create edge: invalid arguments.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeWire(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  BRepBuilderAPI_MakeWire mkWire;

  for ( int k = 2; k < argc; ++k )
  {
    // Get Topology Item Node.
    Handle(asiData_IVTopoItemNode)
      node = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[k]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[k]);
      return TCL_ERROR;
    }

    TopoDS_Shape sh = node->GetShape();
    //
    if ( sh.ShapeType() != TopAbs_EDGE )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "The shape %1 is not an edge." << argv[k]);
      return TCL_ERROR;
    }

    TopoDS_Edge E = TopoDS::Edge(sh);

    mkWire.Add(E);
  }

  interp->GetPlotter().REDRAW_SHAPE(argv[1], mkWire.Wire(), Color_Magenta, 1.0, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeFace(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TopoDS_Face F;

  if ( argc == 3 )
  {
    // Get Surface Node.
    Handle(asiData_IVSurfaceNode)
      node = Handle(asiData_IVSurfaceNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a Surface object with name %1." << argv[2]);
      return TCL_ERROR;
    }

    // Get geometry of a surface.
    Handle(Geom_Surface) surface = node->GetSurface();
    //
    if ( surface.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Surface in question is null.");
      return TCL_ERROR;
    }

    // Make face.
    F = BRepBuilderAPI_MakeFace( surface, Precision::Confusion() );
  }
  else if ( argc >= 4 )
  {
    int wIdx = 0;

    if ( !interp->HasKeyword(argc, argv, "w", wIdx) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "No -w argument is passed.");
      return TCL_ERROR;
    }

    // Collect wires.
    std::vector<TopoDS_Wire> wires;
    //
    for ( int w = wIdx + 1; w < argc; ++w )
    {
      // Get Topology Item Node.
      Handle(asiData_IVTopoItemNode)
        node = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[w]) );
      //
      if ( node.IsNull() )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[w]);
        return TCL_ERROR;
      }

      TopoDS_Shape sh = node->GetShape();
      //
      if ( sh.ShapeType() != TopAbs_WIRE )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "The shape %1 is not a wire." << argv[w]);
        return TCL_ERROR;
      }

      wires.push_back( TopoDS::Wire(sh) );
    }

    if ( wires.empty() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "No wires to construct a face from.");
      return TCL_ERROR;
    }

    // Build face from wires.
    BRepBuilderAPI_MakeFace mkFace(wires[0]);
    //
    for ( size_t w = 1; w < wires.size(); ++w )
    {
      mkFace.Add( TopoDS::Wire( wires[w].Reversed() ) );
    }
    F = mkFace.Face();
  }

  if ( !F.IsNull() )
  {
    interp->GetPlotter().REDRAW_SHAPE(argv[1], F, Color_White, 1.0, false);
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot create face: invalid arguments.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeShell(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TopoDS_Shell result;
  BRep_Builder BB;
  BB.MakeShell(result);

  // Add faces to the resulting shell.
  for ( int k = 2; k < argc; ++k )
  {
    // Get Topology Item Node.
    Handle(asiData_IVTopoItemNode)
      node = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[k]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[k]);
      return TCL_OK;
    }

    // Get item shape.
    TopoDS_Shape itemShape = node->GetShape();
    //
    if ( itemShape.ShapeType() != TopAbs_FACE )
    {
      interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Object %1 is not a face. Skipped." << argv[k]);
      continue;
    }
    //
    TopoDS_Face itemFace = TopoDS::Face(itemShape);

    // Add face to the shell being constructed.
    BB.Add(result, itemFace);
  }

  // Draw in IV.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], result, Color_White, 1.0, false);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeSolid(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TopoDS_Solid result;
  BRep_Builder BB;
  BB.MakeSolid(result);

  // Add shells to the resulting solid.
  for ( int k = 2; k < argc; ++k )
  {
    // Get Topology Item Node.
    Handle(asiData_IVTopoItemNode)
      node = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[k]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[k]);
      return TCL_OK;
    }

    // Get item shape.
    TopoDS_Shape itemShape = node->GetShape();
    //
    if ( itemShape.ShapeType() != TopAbs_SHELL )
    {
      interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Object %1 is not a shell. Skipped." << argv[k]);
      continue;
    }
    //
    TopoDS_Shell itemShell = TopoDS::Shell(itemShape);

    // Add shell to the solid being constructed.
    BB.Add(result, itemShell);
  }

  // Draw in IV.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], result, Color_White, 1.0, false);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeCompound(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TopoDS_Compound result;
  BRep_Builder BB;
  BB.MakeCompound(result);

  // Add shapes to the resulting compound.
  for ( int k = 2; k < argc; ++k )
  {
    // Get Topology Item Node.
    Handle(asiData_IVTopoItemNode)
      node = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[k]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[k]);
      return TCL_OK;
    }

    // Get item shape.
    TopoDS_Shape itemShape = node->GetShape();

    // Add face to the shell being constructed.
    BB.Add(result, itemShape);
  }

  // Draw in IV.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], result, Color_White, 1.0, false);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_AddSubShape(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get parent item Node.
  Handle(asiData_IVTopoItemNode)
    parentNode = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
  //
  if ( parentNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[1]);
    return TCL_OK;
  }

  // Get child item Node.
  Handle(asiData_IVTopoItemNode)
    childNode = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
  //
  if ( childNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name %1." << argv[2]);
    return TCL_OK;
  }

  // Get parent shape.
  TopoDS_Shape parentShape = parentNode->GetShape();

  // Get child shape.
  TopoDS_Shape childShape = childNode->GetShape();

  // Add sub-shape.
  BRep_Builder().Add(parentShape, childShape);

  // Draw in IV.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], parentShape, Color_White, 1.0, false);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakePoint(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  double px;
  double py;
  double pz;

  if ( argc == 5 )
  {
    px = atof(argv[2]);
    py = atof(argv[3]);
    pz = atof(argv[4]);
  }
  else
  {
    asiEngine_Part partApi( cmdEngine::cf->Model,
                            cmdEngine::cf->ViewerPart->PrsMgr() );

    // Get highlighted faces and edges.
    TColStd_PackedMapOfInteger vertIndices;
    //
    partApi.GetHighlightedVertices(vertIndices);

    // Take all vertices
    const TopTools_IndexedMapOfShape&
      allVertices = cmdEngine::cf->Model->GetPartNode()->GetAAG()->RequestMapOfVertices();

    // Extract the selected ones.
    TopoDS_Shape VS = allVertices.FindKey( vertIndices.GetMinimalMapped() );
    //
    gp_Pnt P = BRep_Tool::Pnt( TopoDS::Vertex(VS) );

    px = P.X();
    py = P.Y();
    pz = P.Z();
  }

  interp->GetPlotter().REDRAW_POINT(argv[1], gp_Pnt(px, py, pz), Color_Yellow);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeCurve(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node to access the selected edge.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_OK;
  }
  //
  const TopTools_IndexedMapOfShape&
    subShapes = partNode->GetAAG()->RequestMapOfSubShapes();

  // Curve Node is expected.
  Handle(asiData_CurveNode) curveNode = partNode->GetCurveRepresentation();
  //
  if ( curveNode.IsNull() || !curveNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Curve Node is null or ill-defined.");
    return TCL_OK;
  }

  TopoDS_Shape edgeShape;

  // Get ID of the selected edge.
  int edgeIdx = curveNode->GetSelectedEdge();
  //
  if ( edgeIdx > 0 )
  {
    edgeShape = subShapes(edgeIdx);
  }
  else
  {
    /* The edge might have been passed by ID */

    if ( !interp->GetKeyValue(argc, argv, "eid", edgeIdx) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Edge ID is not provided (use '-eid' keyword).");
      return TCL_ERROR;
    }

    // Get shape.
    edgeShape = partNode->GetAAG()->RequestMapOfEdges()(edgeIdx);
  }

  // Get host curve of the selected edge.
  if ( edgeShape.ShapeType() != TopAbs_EDGE )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected topological type of the selected edge.");
    return TCL_OK;
  }
  //
  double f, l;
  Handle(Geom_Curve) curve = BRep_Tool::Curve( TopoDS::Edge(edgeShape), f, l );

  // Set result.
  interp->GetPlotter().REDRAW_CURVE(argv[1], curve, Color_White, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeSurf(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TopoDS_Shape faceShape;

  // Get Part Node to access the selected face.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  const TopTools_IndexedMapOfShape&
    subShapes = partNode->GetAAG()->RequestMapOfSubShapes();

  // Surface Node is expected.
  Handle(asiData_SurfNode) surfNode = partNode->GetSurfaceRepresentation();
  //
  if ( surfNode.IsNull() || !surfNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Surface Node is null or ill-defined.");
    return TCL_ERROR;
  }

  // Get ID of the selected face.
  int faceIdx = surfNode->GetAnySelectedFace();
  //
  if ( faceIdx > 0 )
  {
    faceShape = subShapes(faceIdx);
  }
  else
  {
    /* The face might have been passed by ID */

    if ( !interp->GetKeyValue(argc, argv, "fid", faceIdx) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Face ID is not provided (use '-fid' keyword).");
      return TCL_ERROR;
    }

    // Get shape.
    faceShape = partNode->GetAAG()->GetMapOfFaces()(faceIdx);
  }

  if ( !faceIdx )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The target face is not specified.");
    return TCL_ERROR;
  }

  // Check type.
  if ( faceShape.ShapeType() != TopAbs_FACE )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected topological type of the selected face.");
    return TCL_OK;
  }

  if ( interp->HasKeyword(argc, argv, "spl") )
  {
    faceShape = ShapeCustom::ConvertToRevolution(faceShape);
    faceShape = ShapeCustom::ConvertToBSpline(faceShape, true, true, true, true);
  }

  TopoDS_Face          face = TopoDS::Face(faceShape);
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

  if ( surf->IsKind( STANDARD_TYPE(Geom_BoundedSurface) ) )
  {
    interp->GetPlotter().REDRAW_SURFACE(argv[1], surf, Color_Default);
  }
  else
  {
    // Take UV bounds from the original face.
    double uMin, uMax, vMin, vMax;
    BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);

    // Set result.
    interp->GetPlotter().REDRAW_SURFACE(argv[1], surf, uMin, uMax, vMin, vMax, Color_Default);
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_InterpolateCurve(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  // Get points to interpolate.
  Handle(asiAlgo_BaseCloud<double>) pts = new asiAlgo_BaseCloud<double>;
  //
  for ( int k = 2; k < argc; ++k )
  {
    if ( !interp->IsKeyword(argv[k], "points") )
      continue;

    bool stop = false;
    do
    {
      ++k;
      if ( (k == argc) || interp->IsKeyword(argv[k], "") )
      {
        stop = true;
      }
      else
      {
        // Get Points Node.
        Handle(asiData_IVPointSetNode)
          pointsNode = Handle(asiData_IVPointSetNode)::DownCast( cmdEngine::model->FindNodeByName(argv[k]) );
        //
        if ( pointsNode.IsNull() )
        {
          interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find Points Node with name %1." << argv[k]);
          return TCL_ERROR;
        }

        // Get Cartesian points to interpolate.
        pts->Merge( pointsNode->GetPoints() );
      }
    }
    while ( !stop );
  }

  // Get degree.
  int degree = 0;
  if ( !interp->GetKeyValue<int>(argc, argv, "degree", degree) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Curve degree is not specified.");
    return TCL_ERROR;
  }

  // Interpolate.
  Handle(Geom_BSplineCurve) interpolant;
  //
  if ( !asiAlgo_Utils::InterpolatePoints(pts, degree, interpolant) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Interpolation failed.");
    return TCL_OK;
  }

  // Set result.
  interp->GetPlotter().REDRAW_CURVE(argv[1], interpolant, Color_Red, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BOPSplit(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_OK;
  }
  //
  TopoDS_Shape partShape = partNode->GetShape();

  // Get cutting face.
  Handle(asiData_IVTopoItemNode)
    faceNode = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
  //
  if ( faceNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a face with name %1." << argv[1]);
    return TCL_OK;
  }
  //
  TopoDS_Face cuttingFace = TopoDS::Face( faceNode->GetShape() );

  // Prepare arguments.
  TopTools_ListOfShape arguments, tools;
  arguments.Append(partShape);
  tools.Append(cuttingFace);

  // Split.
  BOPAlgo_Splitter splitter;
  splitter.SetArguments(arguments);
  splitter.SetTools(tools);
  splitter.Perform();

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update( splitter.Shape() );
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BOPCut(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc != 4 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get topological items which are the operands.
  Handle(asiData_IVTopoItemNode)
    topoItem1 = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
  //
  Handle(asiData_IVTopoItemNode)
    topoItem2 = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[3]) );
  //
  if ( topoItem1.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[2]);
    return TCL_OK;
  }
  //
  if ( topoItem2.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[3]);
    return TCL_OK;
  }

  // Read fuzzy value.
  double fuzz = 0.0;
  if ( argc == 5 )
    fuzz = atof(argv[4]);

  // Cut.
  TopoDS_Shape
    result = asiAlgo_Utils::BooleanCut( topoItem1->GetShape(),
                                        topoItem2->GetShape(), fuzz );
  //
  interp->GetPlotter().REDRAW_SHAPE(argv[1], result);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BOPFuse(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 4 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Fuzzy value.
  const double fuzz = (argc > 4 ? Atof(argv[4]) : 0.0);

  // Get topological items which are the operands.
  Handle(asiData_IVTopoItemNode)
    topoItem1 = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
  //
  Handle(asiData_IVTopoItemNode)
    topoItem2 = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[3]) );
  //
  if ( topoItem1.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[2]);
    return TCL_OK;
  }
  //
  if ( topoItem2.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[3]);
    return TCL_OK;
  }

  // Put all arguments to the list.
  TopTools_ListOfShape arguments;
  arguments.Append( topoItem1->GetShape() );
  arguments.Append( topoItem2->GetShape() );

  // Fuse.
  Handle(BRepTools_History) history;
  TopoDS_Shape fused = asiAlgo_Utils::BooleanFuse(arguments, fuzz, history);
  //
  interp->GetPlotter().REDRAW_SHAPE(argv[1], fused);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BOPFuseGen(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 4 && argc != 5 && argc != 6 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get topological items which are the operands.
  Handle(asiData_IVTopoItemNode)
    topoItem1 = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[2]) );
  //
  Handle(asiData_IVTopoItemNode)
    topoItem2 = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[3]) );
  //
  if ( topoItem1.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[2]);
    return TCL_OK;
  }
  //
  if ( topoItem2.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[3]);
    return TCL_OK;
  }

  // Fuzzy value.
  const double fuzz = (argc > 4 ? Atof(argv[4]) : 0.0);

  // Put all arguments to the list.
  TopTools_ListOfShape arguments;
  arguments.Append( topoItem1->GetShape() );
  arguments.Append( topoItem2->GetShape() );

  TIMER_NEW
  TIMER_GO

  // Fuse.
  TopoDS_Shape fused = asiAlgo_Utils::BooleanGeneralFuse( arguments, fuzz, interp->HasKeyword(argc, argv, "glue") );

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "General fuse")

  // Set the result.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], fused);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DefineGeom(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 1 && argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  if ( !cmdEngine::cf )
    return TCL_OK;

  asiUI_DialogGeomDef* pDlg = new asiUI_DialogGeomDef( cmdEngine::model,
                                                       interp->GetProgress(),
                                                       interp->GetPlotter() );

  // Populate with data.
  if ( argc == 2 )
  {
    Handle(ActAPI_INode) node = cmdEngine::model->FindNodeByName(argv[1]);
    //
    if ( !node.IsNull() && node->IsWellFormed() )
      pDlg->Initialize(node);
  }

  pDlg->show();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakePlane(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc != 2 && argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_OK;
  }
  //
  TopoDS_Shape partShape = partNode->GetShape();

  // Init position.
  gp_Pnt pos;
  //
  if ( argc >= 5 )
  {
    pos.SetX( atof(argv[2]) );
    pos.SetY( atof(argv[3]) );
    pos.SetZ( atof(argv[4]) );
  }

  // Init norm.
  double nX = 0.0, nY = 0.0, nZ = 1.0;
  //
  if ( argc >= 8 )
  {
    nX = atof(argv[5]);
    nY = atof(argv[6]);
    nZ = atof(argv[7]);
  }

  // Create plane.
  Handle(Geom_Plane) plane = new Geom_Plane( pos, gp_Vec(nX, nY, nZ) );

  // Make face.
  double xMin, yMin, zMin, xMax, yMax, zMax;
  asiAlgo_Utils::Bounds(partShape, xMin, yMin, zMin, xMax, yMax, zMax);
  //
  const double d = Max( Abs(xMax - xMin), Max( Abs(yMax - yMin), Abs(zMax - zMin) ) );
  TopoDS_Face cuttingFace = BRepBuilderAPI_MakeFace(plane->Pln(), -d, d, -d, d);

  // Create shape.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], cuttingFace, Color_Default);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeBox(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 2 && argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Init position.
  gp_Pnt pos;
  //
  if ( argc >= 5 )
  {
    pos.SetX( atof(argv[2]) );
    pos.SetY( atof(argv[3]) );
    pos.SetZ( atof(argv[4]) );
  }

  // Init dimensions.
  double dX = 1.0, dY = 1.0, dZ = 1.0;
  //
  if ( argc >= 8 )
  {
    dX = atof(argv[5]);
    dY = atof(argv[6]);
    dZ = atof(argv[7]);
  }

  // Create and draw the box primitive.
  TopoDS_Shape box = BRepPrimAPI_MakeBox( pos, pos.XYZ() + gp_XYZ(dX, dY, dZ) );
  //
  interp->GetPlotter().REDRAW_SHAPE(argv[1], box);
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "The box '%1' was successfully created."
                                                       << argv[1]);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeCylinder(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 2 && argc != 7 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  gp_Pnt origin;
  if ( argc >= 5 )
  {
    origin.SetX( atof(argv[2]) );
    origin.SetY( atof(argv[3]) );
    origin.SetZ( atof(argv[4]) );
  }

  gp_Ax2 pos( origin, gp::DZ() );
  double radius = 1.;
  double height = 1.;
  if ( argc >= 7 )
  {
    radius = atof(argv[5]);
    height = atof(argv[6]);
  }

  // Create and draw the cylinder primitive.
  TopoDS_Shape cyl = BRepPrimAPI_MakeCylinder(pos, radius, height);
  //
  interp->GetPlotter().REDRAW_SHAPE(argv[1], cyl);
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "The cylinder '%1' was successfully created."
                                                       << argv[1]);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_HLR(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  if ( argc != 8 && argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const bool planeDefined = (argc == 8);

  /* =============
   *  Preparation
   * ============= */

  // Get Part Node to access the selected face.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_OK;
  }
  //
  TopoDS_Shape partShape = partNode->GetShape();

  // Get bounding box diagonal to trim projection plane for nice visualization.
  double xmin, ymin, zmin, xmax, ymax, zmax;
  asiAlgo_Utils::Bounds(partShape, xmin, ymin, zmin, xmax, ymax, zmax);
  //
  const double diag = gp_Pnt(xmax, ymax, zmax).Distance( gp_Pnt(xmin, ymin, zmin) );

  gp_Pnt pos;
  gp_Vec dir;
  gp_Ax2 axes;

  if ( planeDefined )
  {
    // Read position of the projection plane.
    pos.SetX( atof(argv[2]) );
    pos.SetY( atof(argv[3]) );
    pos.SetZ( atof(argv[4]) );

    // Read orientation of projection plane.
    const double dX = atof(argv[5]);
    const double dY = atof(argv[6]);
    const double dZ = atof(argv[7]);
    //
    dir.SetX(dX);
    dir.SetY(dY);
    dir.SetZ(dZ);
  }
  else
  {
    if ( cmdEngine::cf.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Visualization facilities are not available.");
      return TCL_ERROR;
    }

    vtkCamera*
      pCamera = cmdEngine::cf->ViewerPart->PrsMgr()->GetRenderer()->GetActiveCamera();

    // Read orientation.
    double dX, dY, dZ;
    pCamera->GetViewPlaneNormal(dX, dY, dZ);
    //
    dir.SetX(dX);
    dir.SetY(dY);
    dir.SetZ(dZ);

    // Read position.
    pos = gp::Origin();
  }

  // Prepare projection plane and its transformation.
  axes = gp_Ax2(pos, dir);
  gp_Pln projPlane(axes);
  //
  interp->GetPlotter().REDRAW_SHAPE("projPlane",
                                    BRepBuilderAPI_MakeFace(projPlane, -diag, diag, -diag, diag),
                                    Color_White, 0.25, false);

  /* =======================
   *  Perform HLR algorithm
   * ======================= */

  // Set a filter for the hidden edges.
  asiAlgo_BuildHLR::t_outputEdges filter;
  //
  if ( Handle(asiData_RootNode)::DownCast( cmdEngine::model->GetRootNode() )->IsEnabledHiddenInHlr() )
  {
    filter.OutputHiddenSharpEdges   = true;
    filter.OutputHiddenOutlineEdges = true;
    filter.OutputHiddenSmoothEdges  = true;
    filter.OutputHiddenIsoLines     = true;
    filter.OutputHiddenSewnEdges    = true;
  }

  asiAlgo_BuildHLR buildHLR( partShape,
                             interp->GetProgress(),
                             interp->GetPlotter() );
  //
  if ( !buildHLR.Perform(dir, asiAlgo_BuildHLR::Mode_Precise, filter) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot build HLR.");
    return TCL_ERROR;
  }

  const TopoDS_Shape& result = buildHLR.GetResult();

  // Draw the result.
  interp->GetPlotter().REDRAW_SHAPE(argv[1], result, Color_White);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeFillet(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get radius.
  const double R = atof(argv[1]);

  // Get part.
  Handle(asiData_PartNode) partNode = M->GetPartNode();
  TopoDS_Shape             partSh   = partNode->GetShape();

  // Attempt to get the highlighted sub-shapes.
  TColStd_PackedMapOfInteger edgeIds;
  //
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
  {
    asiEngine_Part PartAPI( M,
                            cmdEngine::cf->ViewerPart->PrsMgr(),
                            interp->GetProgress(),
                            interp->GetPlotter() );
    //
    PartAPI.GetHighlightedEdges(edgeIds);
  }

  // Initialize blending operator.
  BRepFilletAPI_MakeFillet mkFillet(partSh);
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger eit(edgeIds); eit.More(); eit.Next() )
  {
    const int edgeId = eit.Key();

    // Get edge to blend.
    const TopoDS_Edge&
      edge = TopoDS::Edge( partNode->GetAAG()->RequestMapOfEdges()(edgeId) );

    // Add edge to the blending operator.
    mkFillet.Add(R, edge);
  }

  // Perform blending.
  mkFillet.Build();
  //
  if ( !mkFillet.IsDone() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Blending not done.");
    return TCL_OK;
  }

  // Get result and update part.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update( mkFillet.Shape() );
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_MakeChamfer(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get size.
  const double s = atof(argv[1]);

  // Get part.
  Handle(asiData_PartNode) partNode = M->GetPartNode();
  TopoDS_Shape             partSh   = partNode->GetShape();

  // Attempt to get the highlighted sub-shapes.
  TColStd_PackedMapOfInteger edgeIds;
  //
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
  {
    asiEngine_Part PartAPI( M,
                            cmdEngine::cf->ViewerPart->PrsMgr(),
                            interp->GetProgress(),
                            interp->GetPlotter() );
    //
    PartAPI.GetHighlightedEdges(edgeIds);
  }

  // Initialize blending operator.
  BRepFilletAPI_MakeChamfer mkChamfer(partSh);
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger eit(edgeIds); eit.More(); eit.Next() )
  {
    const int edgeId = eit.Key();

    // Get edge to blend.
    const TopoDS_Edge&
      edge = TopoDS::Edge( partNode->GetAAG()->RequestMapOfEdges()(edgeId) );

    // Add edge to the blending operator.
    mkChamfer.Add(s, edge);
  }

  // Perform blending.
  mkChamfer.Build();
  //
  if ( !mkChamfer.IsDone() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Chamfering not done.");
    return TCL_OK;
  }

  // Get result and update part.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update( mkChamfer.Shape() );
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BuildTriangulationOBB(const Handle(asiTcl_Interp)& interp,
                                 int                          argc,
                                 const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Triangulation.
  Handle(asiData_TriangulationNode) tris_n = cmdEngine::model->GetTriangulationNode();
  //
  if ( tris_n.IsNull() || !tris_n->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is null.");
    return TCL_ERROR;
  }
  //
  Handle(Poly_Triangulation)
    triangulation = cascade::GetOpenCascadeMesh( tris_n->GetTriangulation() );

  // Build OBB.
  asiAlgo_MeshOBB mkOBB(triangulation);
  //
  if ( !mkOBB.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Operation failed.");
    return TCL_ERROR;
  }

  // Get result as a solid.
  TopoDS_Shape obb = mkOBB.GetResultBox();
  //
  interp->GetPlotter().REDRAW_SHAPE(argv[1], obb, Color_Yellow, 1.0, true);

  return TCL_OK;
#else
  (void) argc;
  (void) argv;
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, compile with USE_MOBIUS enabled to use this function.");

  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_BuildOBB(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Check if an equivalent cylinder or sphere is requested.
  const bool isCyl    = interp->HasKeyword(argc, argv, "cyl");
  const bool isSphere = interp->HasKeyword(argc, argv, "sphere");

  if (isCyl || isSphere)
  {
    if (argc < 3)
    {
      return interp->ErrorOnWrongArgs(argv[0]);
    }
  }

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get part.
  Handle(asiData_PartNode) partNode = M->GetPartNode();

  // Build OBB.
  asiAlgo_BuildOBB mkOBB(partNode->GetAAG());
  //
  if ( !mkOBB.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Operation failed.");
    return TCL_ERROR;
  }

  // Get OBB data structure.
  const asiAlgo_OBB& obb = mkOBB.GetResult();

  // Get result shape.
  TopoDS_Shape obbShape;
  //
  if ( isCyl )
  {
    obbShape = obb.BuildCircumscribedCylinder();
  }
  else if (isSphere)
  {
    obbShape = obb.BuildCircumscribedSphere();
  }
  else
  {
    obbShape = mkOBB.GetResultBox();
  }

  interp->GetPlotter().REDRAW_SHAPE(argv[1], obbShape, Color_Yellow, 1.0, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BuildOptBoundingCyl(const Handle(asiTcl_Interp)& interp,
                               int                          argc,
                               const char**                 argv)
{
  if (argc < 2)
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast(interp->GetModel());

  // Get part.
  Handle(asiData_PartNode) partNode = M->GetPartNode();

  // Build bounding cylinder.
  asiAlgo_BuildOptBoundingCyl mkBndCyl(interp->GetProgress(),
                                       interp->GetPlotter());
  if (!mkBndCyl.Perform(partNode->GetAAG()))
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Operation failed.");
    return TCL_ERROR;
  }

  interp->GetPlotter().REDRAW_SHAPE(argv[1], mkBndCyl.GetResult().shape, Color_Yellow, 1.0, true);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

#ifndef WIN32
  #ifdef USE_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
  #endif
#endif

namespace
{
  //! Computes the unsigned distance between the two passed planes.
  //! \param[in]  S1 the first plane.
  //! \param[in]  S2 the second plane.
  //! \param[out] d  the computed distance value.
  //! \return true in case of success, false if the passed surfaces
  //!         are not parallel.
  bool DistanceBetweenPlanes(const Handle(Geom_Plane)& S1,
                             const Handle(Geom_Plane)& S2,
                             double&                   d)
  {
    if ( !asiAlgo_Utils::AreParallel(S1, S2, 1.*M_PI/180.) )
      return false;

    d = S1->Pln().Distance( S2->Pln() );
    return true;
  }

  //! Contructs a Boolean common for the passed two planar faces.
  //! The faces `F` and `G` should have non-empty overlapping if
  //! translated onto each other along their normals.
  //! \param[in] F the first base face.
  //! \param[in] G the second base face.
  //! \return true if the intersection zone has been computed,
  //!         false -- otherwise.
  bool FindCommonBase(const TopoDS_Face&   F,
                      const TopoDS_Face&   G,
                      ActAPI_ProgressEntry progress)
  {
    // Contract check 1.
    Handle(Geom_Plane) FP;
    //
    if ( !asiAlgo_Utils::IsTypeOf<Geom_Plane>(F, FP) )
    {
      progress.SendLogMessage(LogErr(Normal) << "The first passed base face is not planar.");
      return false;
    }

    // Contract check 2.
    Handle(Geom_Plane) GP;
    //
    if ( !asiAlgo_Utils::IsTypeOf<Geom_Plane>(G, GP) )
    {
      progress.SendLogMessage(LogErr(Normal) << "The second passed base face is not planar.");
      return false;
    }

    // Compute the distance.
    double d = 0.;
    if ( !DistanceBetweenPlanes(FP, GP, d) )
    {
      progress.SendLogMessage(LogErr(Normal) << "Cannot compute plane-to-plane distance."
                                                "Are the selected planes parallel?");
      return false;
    }

    // TODO: NYI
    return false;
  }
}

#ifndef WIN32
  #ifdef USE_GCC
    #pragma GCC diagnostic pop
  #endif
#endif

int ENGINE_Fill(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( (argc != 1) && (argc != 2) && (argc != 3) )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  /* =====================
   *  Prepare for filling.
   * ===================== */

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get part.
  Handle(asiData_PartNode) partNode = M->GetPartNode();
  Handle(asiAlgo_AAG)      partAAG  = partNode->GetAAG();
  TopoDS_Shape             partSh   = partNode->GetShape();

  // Get the base face.
  bool isBetweenMode = false;
  int fidFrom = 0, fidTo = 0;
  //
  if ( argc == 2 )
  {
    fidFrom = atoi(argv[1]);
  }
  else if ( argc == 3 )
  {
    isBetweenMode = true;
    fidFrom       = atoi(argv[1]);
    fidTo         = atoi(argv[2]);
  }
  else
  {
    asiEngine_Part api( cmdEngine::model, cmdEngine::cf->ViewerPart->PrsMgr() );

    asiAlgo_Feature selectedFids;
    api.GetHighlightedFaces(selectedFids);
    //
    if ( selectedFids.Extent() )
      fidFrom = selectedFids.GetMinimalMapped();
  }

  // Check the base face.
  if ( (fidFrom == 0) || !partAAG->HasFace(fidFrom) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed face index %1 is invalid."
                                                        << fidFrom);
    return TCL_ERROR;
  }

  // Check the second face for the in-between filling mode.
  if ( isBetweenMode && ( (fidTo == 0) || !partAAG->HasFace(fidTo) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The passed face index %1 is invalid."
                                                        << fidTo);
    return TCL_ERROR;
  }

  // The first base face.
  const TopoDS_Face& faceFrom = partAAG->GetFace(fidFrom);

  // The second base face (optional).
  TopoDS_Face faceTo;
  if ( isBetweenMode )
    faceTo = partAAG->GetFace(fidTo);

  /* ==========================
   *  Construct the tool prism.
   * ========================== */

  // Variables to build the tool prism.
  TopoDS_Face prismBase;
  double dist = 0.;
  gp_Vec norm;

  if ( isBetweenMode )
  {
    // TODO: NYI
    return TCL_ERROR;
  }
  else
  {
    // Measure the distance between 'from' and 'to'.
    if ( !asiEngine_Part::ComputeMateFace<Geom_Plane>(partAAG,
                                                      fidFrom,
                                                      asiAlgo_Feature(),
                                                      3,
                                                      false,
                                                      fidTo,
                                                      dist,
                                                      norm) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find a mate face for the face %1."
                                                          << fidFrom);
      return TCL_ERROR;
    }

    prismBase = faceFrom;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Distance to fill: %1."
                                                       << dist);

  // Make a tool object.
  BRepPrimAPI_MakePrism mkPrism(prismBase, norm.Normalized()*dist, true);
  //
  const TopoDS_Shape& toolSh = mkPrism.Shape();

  /* ======================
   *  Perform Boolean fuse.
   * ====================== */

  // Fuse.
  Handle(BRepTools_History) history;
  //
  TopTools_ListOfShape objects;
  objects.Append(partSh);
  objects.Append(toolSh);
  //
  TopoDS_Shape resultSh = asiAlgo_Utils::BooleanFuse(objects, true, history);

  //interp->GetPlotter().REDRAW_SHAPE("tool", toolSh);

  // Get result and update part.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(resultSh);
  }
  cmdEngine::model->CommitCommand();

  // Update UI
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_BuildGordon(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get the part.
  asiEngine_Part partApi(M);

  // Read {p} ("profile") curves.
  std::vector<int> pIds;
  int pIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "p", pIdx) )
  {
    int k = pIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int eid = atoi(argv[k]);
      pIds.push_back(eid);
    }
  }

  // Read {g} ("guide") curves.
  std::vector<int> gIds;
  int gIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "g", gIdx) )
  {
    int k = gIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int eid = atoi(argv[k]);
      gIds.push_back(eid);
    }
  }

  // Get AAG.
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  //
  if ( aag.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "AAG is null.");
    return false;
  }
  //
  const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();

  // Collect edges.
  std::vector<TopoDS_Edge> pEdges, gEdges;
  //
  for ( const auto eid : pIds )
  {
    if ( eid < 1 || eid > allEdges.Extent() )
    {
      interp->GetProgress().SendLogMessage( LogErr(Normal) << "Edge %1 is out of range [1, %2]."
                                                           << eid << allEdges.Extent() );
      return false;
    }

    pEdges.push_back( TopoDS::Edge( allEdges(eid) ) );
  }
  //
  for ( const auto eid : gIds )
  {
    if ( eid < 1 || eid > allEdges.Extent() )
    {
      interp->GetProgress().SendLogMessage( LogErr(Normal) << "Edge %1 is out of range [1, %2]."
                                                           << eid << allEdges.Extent() );
      return false;
    }

    gEdges.push_back( TopoDS::Edge( allEdges(eid) ) );
  }

  Handle(Geom_BSplineSurface) resSurf;
  TopoDS_Face                 resFace;

  // Build Gordon surface.
  asiAlgo_BuildGordonSurf buildGordon( interp->GetProgress(),
                                       interp->GetPlotter() );
  //
  if ( !buildGordon.Build(pEdges, gEdges, resSurf, resFace) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot build Gordon surface.");
    return TCL_ERROR;
  }

  std::string surfName = "gordonSurf";
  interp->GetKeyValue(argc, argv, "name", surfName);

  interp->GetPlotter().REDRAW_SURFACE(surfName.c_str(), resSurf, Color_Default);

  const double maxError = buildGordon.GetMaxError();

  *interp << maxError;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_UntrimSurf(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get the part.
  asiEngine_Part partApi(M);

  // Read faces.
  std::vector<int> fIds;
  int fIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "f", fIdx) )
  {
    int k = fIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int fid = atoi(argv[k]);
      fIds.push_back(fid);
    }
  }

  // Read edges.
  std::vector<int> eIds;
  int eIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "e", eIdx) )
  {
    int k = eIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int eid = atoi(argv[k]);
      eIds.push_back(eid);
    }
  }

  // Read number of isos.
  int numUIsos = 2, numVIsos = 2;
  interp->GetKeyValue(argc, argv, "u", numUIsos);
  interp->GetKeyValue(argc, argv, "v", numVIsos);

  // Get AAG.
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  //
  if ( aag.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "AAG is null.");
    return false;
  }
  //
  const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();
  const TopTools_IndexedMapOfShape& allFaces = aag->GetMapOfFaces();

  // Collect edges.
  Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape;
  //
  for ( const auto eid : eIds )
  {
    if ( eid < 1 || eid > allEdges.Extent() )
    {
      interp->GetProgress().SendLogMessage( LogErr(Normal) << "Edge %1 is out of range [1, %2]."
                                                           << eid << allEdges.Extent() );
      return false;
    }

    edges->Append( allEdges(eid) );
  }

  // Collect faces.
  Handle(TopTools_HSequenceOfShape) faces = new TopTools_HSequenceOfShape;
  //
  for ( const auto fid : fIds )
  {
    if ( fid < 1 || fid > allFaces.Extent() )
    {
      interp->GetProgress().SendLogMessage( LogErr(Normal) << "Face %1 is out of range [1, %2]."
                                                           << fid << allFaces.Extent() );
      return false;
    }

    faces->Append( allFaces(fid) );
  }

  Handle(Geom_BSplineSurface) resSurf;
  TopoDS_Face                 resFace;

  // Untrim surface.
  asiAlgo_UntrimSurf UNTRIM( interp->GetProgress(),
                             nullptr );
  //
  UNTRIM.SetNumUIsos(numUIsos);
  UNTRIM.SetNumVIsos(numVIsos);
  //
  if ( !UNTRIM.Build(faces, edges, resSurf, resFace) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "UNTRIM operator failed.");
    return TCL_ERROR;
  }

  std::string surfName = "untrimSurf";
  interp->GetKeyValue(argc, argv, "name", surfName);
  interp->GetPlotter().REDRAW_SURFACE(surfName.c_str(), resSurf, Color_Default);

  // Visually dump guide and profile curves.
  BRep_Builder    bbuilder;
  TopoDS_Compound guidesComp, profilesComp;
  //
  bbuilder.MakeCompound(guidesComp);
  bbuilder.MakeCompound(profilesComp);
  //
  const std::vector<TopoDS_Edge>& guides   = UNTRIM.GetGuides();
  const std::vector<TopoDS_Edge>& profiles = UNTRIM.GetProfiles();
  //
  for ( const auto& guide : guides )
  {
    bbuilder.Add(guidesComp, guide);
  }
  //
  for ( const auto& profile : profiles )
  {
    bbuilder.Add(profilesComp, profile);
  }
  //
  interp->GetPlotter().REDRAW_SHAPE("guides",   guidesComp,   Color_Red);
  interp->GetPlotter().REDRAW_SHAPE("profiles", profilesComp, Color_Green);

  const double maxError = UNTRIM.GetMaxError();

  *interp << maxError;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_JoinSurf(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get the part.
  asiEngine_Part partApi(M);

  // Read faces.
  std::vector<int> fIds;
  int fIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "f", fIdx) )
  {
    int k = fIdx;
    //
    while ( (k + 1 < argc) && !interp->IsKeyword(argv[++k]) )
    {
      const int fid = atoi(argv[k]);
      fIds.push_back(fid);
    }
  }
  //
  if ( fIds.size() != 2 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Two faces should be passed.");
    return TCL_ERROR;
  }

  // Read number of isos.
  int numProfilesS1 = 2, numProfilesS2 = 2, numGuides = 2;
  interp->GetKeyValue(argc, argv, "profiles1", numProfilesS1);
  interp->GetKeyValue(argc, argv, "profiles2", numProfilesS2);
  interp->GetKeyValue(argc, argv, "guides",    numGuides);

  // Read boundary offset.
  double offset = 1.;
  interp->GetKeyValue(argc, argv, "offset", offset);

  // Get AAG.
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  //
  if ( aag.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "AAG is null.");
    return false;
  }
  //
  const TopTools_IndexedMapOfShape& allFaces = aag->GetMapOfFaces();

  // Collect faces.
  Handle(TopTools_HSequenceOfShape) faces = new TopTools_HSequenceOfShape;
  //
  for ( const auto fid : fIds )
  {
    if ( fid < 1 || fid > allFaces.Extent() )
    {
      interp->GetProgress().SendLogMessage( LogErr(Normal) << "Face %1 is out of range [1, %2]."
                                                           << fid << allFaces.Extent() );
      return false;
    }

    faces->Append( allFaces(fid) );
  }

  Handle(Geom_BSplineSurface) resSurf;
  TopoDS_Face                 resFace;

  // Join surfaces.
  asiAlgo_JoinSurf JOINSURF( interp->GetProgress(),
                             interp->HasKeyword(argc, argv, "draw") ? interp->GetPlotter() : nullptr );
  //
  JOINSURF.SetNumProfilesS1  (numProfilesS1);
  JOINSURF.SetNumProfilesS2  (numProfilesS2);
  JOINSURF.SetNumGuides      (numGuides);
  JOINSURF.SetBoundaryOffset (offset);
  //
  if ( !JOINSURF.Build(faces, resSurf, resFace) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "JOINSURF operator failed.");
    return TCL_ERROR;
  }

  std::string surfName = "jointSurf";
  interp->GetKeyValue(argc, argv, "name", surfName);
  interp->GetPlotter().REDRAW_SURFACE(surfName.c_str(), resSurf, Color_Default);

  // Visually dump guide and profile curves.
  BRep_Builder    bbuilder;
  TopoDS_Compound guidesComp, profilesComp;
  //
  bbuilder.MakeCompound(guidesComp);
  bbuilder.MakeCompound(profilesComp);
  //
  const std::vector<TopoDS_Edge>& guides   = JOINSURF.GetGuides();
  const std::vector<TopoDS_Edge>& profiles = JOINSURF.GetProfiles();
  //
  for ( const auto& guide : guides )
  {
    bbuilder.Add(guidesComp, guide);
  }
  //
  for ( const auto& profile : profiles )
  {
    bbuilder.Add(profilesComp, profile);
  }
  //
  interp->GetPlotter().REDRAW_SHAPE("guides",   guidesComp,   Color_Red);
  interp->GetPlotter().REDRAW_SHAPE("profiles", profilesComp, Color_Green);

  const double maxError = JOINSURF.GetMaxError();

  *interp << maxError;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdEngine::Commands_Modeling(const Handle(asiTcl_Interp)&      interp,
                                  const Handle(Standard_Transient)& cmdEngine_NotUsed(data))
{
  static const char* group = "cmdEngine";

  //-------------------------------------------------------------------------//
  interp->AddCommand("offset-shell",
    //
    "offset-shell <offset-positive-or-negative> [-simple] [-solid] [-keep] [-toler <val>] [-faces]\n"
    "\t Offsets part (it should be a topological shell) on the given offset\n"
    "\t value. Offsetting is performed in the direction of face normals. If the\n"
    "\t option '-simple' is passed, this operation will attempt to preserve\n"
    "\t the topology of the base shell. If the option '-solid' is passed, this\n"
    "\t operation will build a solid instead of an offset shell. If the option\n"
    "\t '-keep' is passed, the original part is not substituted with the offset\n"
    "\t shape, and the offset is added to the part. If the option '-toler' is\n"
    "\t passed and '-simple' key is used, an optional tolerance for suppressing\n"
    "\t singularities on triangular surface patches is used. If the key '-faces'\n"
    "\t is passed, the input shell will be broken down to faces, and each face\n"
    "\t will be offset individually.",
    //
    __FILE__, group, ENGINE_OffsetShell);

  //-------------------------------------------------------------------------//
  interp->AddCommand("offset-tess",
    //
    "offset-tess <offset>\n"
    "\t Offsets mesh nodes in directions of their norms.",
    //
    __FILE__, group, ENGINE_OffsetTess);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-vertex",
    //
    "make-vertex <result> <ptName>\n"
    "\n"
    "\t Creates a vertex from the point named <ptName>.",
    //
    __FILE__, group, ENGINE_MakeVertex);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-edge",
    //
    "make-edge <result> {<curveName> | -p1 <pointName> -p2 <pointName>}\n"
    "\n"
    "\t Creates an edge from a curve or a pair of points. The <curveName>/<pointName>\n"
    "\t variables should exist in the scene graph of the imperative plotter.",
    //
    __FILE__, group, ENGINE_MakeEdge);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-wire",
    //
    "make-wire <result> <edgeName1> [<edgeName2> [...]]\n"
    "\n"
    "\t Creates a wire from the passed series of edges.",
    //
    __FILE__, group, ENGINE_MakeWire);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-face",
    //
    "make-face <result> {<surfaceName> | -w <wireName1> [<wireName2> ...]}\n"
    "\n"
    "\t Creates a face from a surface or wires. The <surfaceName>/<wireName> variables\n"
    "\t should exist in the scene graph of the imperative plotter.",
    //
    __FILE__, group, ENGINE_MakeFace);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-shell",
    //
    "make-shell <result> <face1> [<face2> [...]]\n"
    "\t Creates a shell from the passed faces.",
    //
    __FILE__, group, ENGINE_MakeShell);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-solid",
    //
    "make-solid <result> <shell1> [<shell2> [...]]\n"
    "\t Creates a solid from the passed shells.",
    //
    __FILE__, group, ENGINE_MakeSolid);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-compound",
    //
    "make-compound <result> [<shape1> [<shape2> [...]]]\n"
    "\t Creates compound from the passed shapes.",
    //
    __FILE__, group, ENGINE_MakeCompound);

  //-------------------------------------------------------------------------//
  interp->AddCommand("add-subshape",
    //
    "add-subshape <parent> <child>\n"
    "\t Adds the <child> subshape to the <parent> shape.",
    //
    __FILE__, group, ENGINE_AddSubShape);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-point",
    //
    "make-point [<pointName> <x> <y> <z>]\n"
    "\t Creates a point with the passed coordinates. If nothing is passed, the\n"
    "\t point is created using the coordinates of the selected vertex.",
    //
    __FILE__, group, ENGINE_MakePoint);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-curve",
    //
    "make-curve <curveName> [-eid <eid>]\n"
    "\t Creates a curve from the selected edge or the edge with the passed ID.",
    //
    __FILE__, group, ENGINE_MakeCurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-surf",
    //
    "make-surf <surfName> [-fid <fid>] [-spl]\n"
    "\t Creates a surface from the selected face or a face with the given name.\n"
    "\t If the '-spl' flag is passed, the surface will be converted to spline.",
    //
    __FILE__, group, ENGINE_MakeSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("interpolate-curve",
    //
    "interpolate-curve <curveName> -points <pointsName> -degree <deg>\n"
    "\t Creates a curve from the passed point series by interpolation.",
    //
    __FILE__, group, ENGINE_InterpolateCurve);

  //-------------------------------------------------------------------------//
  interp->AddCommand("bop-split",
    //
    "bop-split <plane>\n"
    "\t Splits the active part by the passed plane.",
    //
    __FILE__, group, ENGINE_BOPSplit);

  //-------------------------------------------------------------------------//
  interp->AddCommand("bop-cut",
    //
    "bop-cut <result> <op1> <op2> [<fuzz>]\n"
    "\t Cuts <op2> from <op1> using Boolean Cut operation. Use <fuzz> value\n"
    "\t to control the 'fuzzy tolerance'.",
    //
    __FILE__, group, ENGINE_BOPCut);

  //-------------------------------------------------------------------------//
  interp->AddCommand("bop-fuse",
    //
    "bop-fuse <result> <op1> <op2> [<fuzz>]\n"
    "\t Fuses the passed two operands using Boolean Fuse operation.\n"
    "\t It is possible to affect the fusion tolerance with <fuzz> argument.\n",
    //
    __FILE__, group, ENGINE_BOPFuse);

  //-------------------------------------------------------------------------//
  interp->AddCommand("bop-fuse-gen",
    //
    "bop-fuse-gen <result> <op1> <op2> [<fuzz>] [-glue]\n"
    "\t Fuses the passed two operands using Boolean General Fuse operation.\n"
    "\t It is possible to affect the fusion tolerance with <fuzz> argument.\n"
    "\t In case if you have overlapping faces in your operands, you may want\n"
    "\t to try gluing option to speed up computations.",
    //
    __FILE__, group, ENGINE_BOPFuseGen);

  //-------------------------------------------------------------------------//
  interp->AddCommand("define-geom",
    //
    "define-geom [<name>]\n"
    "\t Opens geometry definition dialog.",
    //
    __FILE__, group, ENGINE_DefineGeom);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-plane",
    //
    "make-plane name [<posX> <posY> <posZ> <nX> <nY> <nZ>]\n"
    "\t Creates a plane with origin at <posX>, <posY>, <posZ>\n"
    "\t and the normal direction (<nX>, <nY>, <nZ>).",
    //
    __FILE__, group, ENGINE_MakePlane);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-box",
    //
    "make-box name [<posX> <posY> <posZ> <dX> <dY> <dZ>]\n"
    "\t Creates axis-aligned box solid with min corner at <posX>, <posY>, <posZ>\n"
    "\t and dimensions <dX>, <dY>, <dZ>.",
    //
    __FILE__, group, ENGINE_MakeBox);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-cylinder",
    //
    "make-cylinder name [<posX> <posY> <posZ> <r> <h>]\n"
    "\t Creates a cylindrical primitive oriented along the OZ axis. The <posX>,\n"
    "\t <posY>, <posZ> arguments define the origin point. The arguments <r> and\n"
    "\t <h> define the radius and the height.",
    //
    __FILE__, group, ENGINE_MakeCylinder);

  //-------------------------------------------------------------------------//
  interp->AddCommand("hlr",
    //
    "hlr <res> [<posX> <posY> <posZ> <nX> <nY> <nZ>]\n"
    "\t Performs HLR algorithm using <posX>, <posY>, <posZ> as a location of a\n"
    "\t projection plane with <nX>, <nY>, <nZ> as its normal direction.",
    //
    __FILE__, group, ENGINE_HLR);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-fillet",
    //
    "make-fillet <radius>\n"
    "\t Blends the selected edges with the given radius.",
    //
    __FILE__, group, ENGINE_MakeFillet);

  //-------------------------------------------------------------------------//
  interp->AddCommand("make-chamfer",
    //
    "make-chamfer <size>\n"
    "\t Chamfers the selected edges with the given size.",
    //
    __FILE__, group, ENGINE_MakeChamfer);

  //-------------------------------------------------------------------------//
  interp->AddCommand("build-triangulation-obb",
    //
    "build-triangulation-obb <res>\n"
    "\t Builds the oriented bounding box (OBB) for triangulation.",
    //
    __FILE__, group, ENGINE_BuildTriangulationOBB);

  //-------------------------------------------------------------------------//
  interp->AddCommand("build-obb",
    //
    "build-obb <res> [-cyl] [-sphere]\n"
    "\t Builds the oriented bounding box (OBB) for the active part.\n"
    "\t If the '-cyl' or '-sphere' flag is passed, the constructed OBB is turned\n"
    "\t into an circumscribed cylinder or sphere.",
    //
    __FILE__, group, ENGINE_BuildOBB);

  //-------------------------------------------------------------------------//
  interp->AddCommand("build-opt-bnd-cyl",
    //
    "build-opt-bnd-cyl <res>\n"
    "\t Builds an optimal bounding cylinder for the active part.",
    //
    __FILE__, group, ENGINE_BuildOptBoundingCyl);

  //-------------------------------------------------------------------------//
  interp->AddCommand("fill",
    //
    "fill [<fid>]\n"
    "\t Fills the cavity by fusing the part with a prismatic tool defined\n"
    "\t with the <fid> face. There should be a mate face to fill until.",
    //
    __FILE__, group, ENGINE_Fill);

  //-------------------------------------------------------------------------//
  interp->AddCommand("build-gordon",
    //
    "build-gordon -p <e1> <e2> [<e3> ...] -g <e1> <e2> [<e3> ...] [-name <surfName>]\n"
    "\t Builds a Gordon surface passing through the given {p} (\"profile\")\n"
    "\t and {g} (\"guide\") curves specified as edge indices in the active part.",
    //
    __FILE__, group, ENGINE_BuildGordon);

  //-------------------------------------------------------------------------//
  interp->AddCommand("untrim-surf",
    //
    "untrim-surf -f <f1> [<f2> ...] -e <e1> <e2> [<e3> ...] [-name <surfName>] [-u <numIsosU>] [-v <numIsosV>]\n"
    "\t Untrims surface.",
    //
    __FILE__, group, ENGINE_UntrimSurf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("join-surf",
    //
    "join-surf -f <f1> <f2> [-profiles1 <numProfilesS1>] [-profiles2 <numProfilesS2>]"
    "                       [-guides <numGuides>]"
    "                       [-offset <d>]"
    "                       [-draw]\n"
    "\t Joins surfaces.",
    //
    __FILE__, group, ENGINE_JoinSurf);
}
