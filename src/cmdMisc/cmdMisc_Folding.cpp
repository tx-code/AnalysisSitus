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

// asiAlgo includes
#include <asiAlgo_RTCD.h>

// asiEngine includes
#include <asiEngine_Part.h>

// OpenCascade includes
#include <BOPAlgo_Builder.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <gp_Quaternion.hxx>
#include <NCollection_UBTree.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

#define ApplyHalfAngleRotation
#define OutPlaneToler 0.1

//-----------------------------------------------------------------------------

struct BoxStock
{
  Bnd_Box bbox;
  double  scaleCoeff;

  //! Ctor with initialization.
  BoxStock(const TopoDS_Shape& shape)
  {
    asiAlgo_Utils::Bounds(shape, true, true, bbox);

    double xMin, yMin, zMin, xMax, yMax, zMax;
    bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

    scaleCoeff = Max( Max(xMax - xMin, yMax - yMin), zMax - zMin )*0.5;
  }

  //! Computes an anchor point on the stock shape for visualizing
  //! setup axis.
  gp_Pnt ComputeAnchorPoint(const gp_Dir& dir)
  {
    gp_Pnt res;

    RTCD::AABB aabb(bbox);

    double xMin, yMin, zMin, xMax, yMax, zMax;
    bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

    // Compose a probe point.
    RTCD::Point  p( (xMin + xMax)/2., (yMin + yMax)/2., (zMin + zMax)/2. );
    RTCD::Vector d( dir.X(), dir.Y(), dir.Z() );

    // Test w.r.t. AABB.
    double tmin, tmax;
    RTCD::Point q;
    //
    if ( RTCD::IntersectRayAABB(p, d, aabb, tmin, tmax) )
    {
      double t = (tmin < 0) ? tmax : tmin; // Negative `t` is in reversed direction.
      q = p + d * t;

      res.SetX(q.x);
      res.SetY(q.y);
      res.SetZ(q.z);
    }

    return res;
  }

  //! Returns axis color.
  ActAPI_Color AxisColor()
  {
    return Color_Red;
  }
};

//-----------------------------------------------------------------------------

enum FoldDir
{
  FoldDir_Undefined =  0,
  FoldDir_Up        =  1,
  FoldDir_Down      = -1
};

//-----------------------------------------------------------------------------

struct t_shapeOnSide
{
  TopoDS_Shape S;
  bool         isLeft;

  t_shapeOnSide(const TopoDS_Shape& sh, const bool left) : S(sh), isLeft(left)  {}
  t_shapeOnSide()                                        :        isLeft(false) {}
};

//-----------------------------------------------------------------------------

class BendLineSelector : public NCollection_UBTree<int, Bnd_Box>::Selector
{
public:

  BendLineSelector(const std::vector<t_shapeOnSide>& allSeparatedFaces)
  {
    m_faces = allSeparatedFaces;
    m_bLeft = false;
  }

public:

  void Init(const gp_Pnt& P)
  {
    m_pnt   = P;
    m_bLeft = false;
  }

  bool Reject(const Bnd_Box& box) const
  {
    return box.IsOut(m_pnt);
  }

  bool Accept(const int& index)
  {
    // Point is known to be inside the bbox of `index`-th object.

    BRepAdaptor_Surface bas(TopoDS::Face(m_faces[index].S), false);
    //
    if ( bas.GetType() != GeomAbs_Plane )
      return false;

    gp_Pln       pln = bas.Plane();
    const double d   = pln.Distance(m_pnt);

    if ( d > OutPlaneToler )
      return false;

    m_bLeft = m_faces[index].isLeft;
    return true;
  }

  bool IsLeft() const
  {
    return m_bLeft;
  }

private:

  std::vector<t_shapeOnSide> m_faces; //!< All faces as a universum.
  gp_Pnt                     m_pnt;   //!< Probe point at a bend line.
  bool                       m_bLeft; //!< Whether the currently tested point is on the left.

};

//-----------------------------------------------------------------------------

void propagate(const int                  seedFaceId,
               const Handle(asiAlgo_AAG)& aag,
               asiAlgo_Feature&           exclude,
               asiAlgo_Feature&           neighbors)
{
  asiAlgo_Feature nids = aag->GetNeighbors(seedFaceId);
  nids.Subtract(exclude);

  for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
  {
    const int nextSeedFaceId = nit.Key();
    neighbors.Add(nextSeedFaceId);
    exclude.Add(nextSeedFaceId);

    propagate(nextSeedFaceId, aag, exclude, neighbors);
  }
}

struct t_foldedState
{
  TopoDS_Shape S;

  operator TopoDS_Shape()
  {
    return S;
  }

  void Extract(const TopoDS_Shape& fp,
               TopoDS_Shape&       blankSheetShape,
               TopoDS_Shape&       bendLinesShape)
  {
    // COMPOUND
    // |
    // o-> FACE (SHELL)
    // |
    // o-> COMPOUND of EDGEs
    TopoDS_Iterator it(fp);
    //
    blankSheetShape = it.Value();
    it.Next();
    //
    bendLinesShape = it.Value();
  }

  void Imprint(const TopoDS_Shape&        blankSheetShape,
               const TopoDS_Shape&        foldEdge,
               TopoDS_Shell&              fusedSheetShell,
               Handle(BRepTools_History)& history,
               ActAPI_PlotterEntry        plotter)
  {
    // Compose the arguments for the Boolean operation.
    TopTools_ListOfShape bopArgs;
    bopArgs.Append(blankSheetShape);
    bopArgs.Append(foldEdge);

    plotter.REDRAW_SHAPE("blankSheetShape", blankSheetShape);
    plotter.REDRAW_SHAPE("foldEdge", foldEdge);

    BRep_Builder    bbuilder;
    BOPAlgo_Builder bopAlgo;
    TopoDS_Shape    fusedSheet = asiAlgo_Utils::BooleanGeneralFuse(bopArgs, 0.1, bopAlgo);
    //
    bbuilder.MakeShell(fusedSheetShell);
    //
    for ( TopExp_Explorer exp(fusedSheet, TopAbs_FACE); exp.More(); exp.Next() )
    {
      bbuilder.Add( fusedSheetShell, exp.Current() );
    }

    // Use the history of modification to find the imprinted edge.
    history = bopAlgo.History();

    plotter.REDRAW_SHAPE("imprinted", fusedSheetShell);
  }

  void Compose(const std::vector<t_shapeOnSide>& faces,
               const std::vector<t_shapeOnSide>& bls)
  {
    BRep_Builder bbuilder;

    // Make a shell.
    TopoDS_Shell shell;
    bbuilder.MakeShell(shell);
    //
    for ( const auto& face : faces )
    {
      bbuilder.Add(shell, face.S);
    }

    // Make sure that these faces are topologically connected.
    BRepBuilderAPI_Sewing sewing;
    sewing.Add(shell);
    sewing.Perform();
    shell = TopoDS::Shell( sewing.SewedShape() );

    // Make a compound of bend lines.
    TopoDS_Compound bendsComp;
    bbuilder.MakeCompound(bendsComp);
    //
    for ( const auto& bl : bls )
    {
      bbuilder.Add(bendsComp, bl.S);
    }

    // Make the root compound.
    TopoDS_Compound rootComp;
    bbuilder.MakeCompound(rootComp);
    bbuilder.Add(rootComp, shell);
    bbuilder.Add(rootComp, bendsComp);

    // Initialize the folded state shape.
    this->S = rootComp;
  }
};

int MISC_Fold(const Handle(asiTcl_Interp)& interp,
              int                          argc,
              const char**                 argv)
{
  /* ================
   *  Prepare inputs.
   * ================ */

  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  // Get rotation angle.
  double angleDeg = 0.;
  interp->GetKeyValue(argc, argv, "angle", angleDeg);

  // Folding direction.
  FoldDir foldDir = FoldDir_Undefined;
  //
  if ( interp->HasKeyword(argc, argv, "up") )
  {
    foldDir = FoldDir_Up;
  }
  else if ( interp->HasKeyword(argc, argv, "down") )
  {
    foldDir = FoldDir_Down;
  }
  //
  if ( foldDir == FoldDir_Undefined )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify the folding direction "
                                                           "as '-up' or '-down'.");
    return TCL_ERROR;
  }

  /* ===============
   *  Get bend line.
   * =============== */

  // Part API.
  asiEngine_Part PartAPI( M,
                          cmdMisc::cf->ViewerPart->PrsMgr(),
                          interp->GetProgress(),
                          interp->GetPlotter() );

  // Folded state structure.
  t_foldedState fs;

  // Get bend lines and blank sheet separately.
  TopoDS_Shape blankSheetComp = PartAPI.GetShape();
  TopoDS_Shape blankSheetShape, bendLinesShape;
  //
  fs.Extract(blankSheetComp, blankSheetShape, bendLinesShape);

  // Attempt to get the highlighted sub-shapes.
  TColStd_PackedMapOfInteger selectedEdgeIds;
  //
  if ( cmdMisc::cf && cmdMisc::cf->ViewerPart )
  {
    PartAPI.GetHighlightedEdges(selectedEdgeIds);
  }

  TopoDS_Edge foldEdge;
  //
  if ( selectedEdgeIds.Extent() != 1 )
  {
    int blId = 0;
    interp->GetKeyValue(argc, argv, "id", blId);

    if ( !blId )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, select a bend line.");
      return TCL_ERROR;
    }

    int idx = 0;
    for ( TopoDS_Iterator blIt(bendLinesShape); blIt.More(); blIt.Next() )
    {
      idx++;
      if ( idx == blId )
      {
        foldEdge = TopoDS::Edge( blIt.Value() );
        break;
      }
    }
  }

  // Grab the fold line edge selected interactively.
  if ( foldEdge.IsNull() )
  {
    for ( TColStd_PackedMapOfInteger::Iterator eit(selectedEdgeIds); eit.More(); eit.Next() )
    {
      const int edgeId = eit.Key();

      // Get edge.
      const TopoDS_Shape&
        edge = M->GetPartNode()->GetAAG()->RequestMapOfEdges()(edgeId);

      foldEdge = TopoDS::Edge(edge);
    }
  }

  /* ========================================
   *  Imprint fold line onto the blank sheet.
   * ======================================== */

  // Imprint.
  TopoDS_Shell              fusedSheetShell;
  Handle(BRepTools_History) history;
  //
  fs.Imprint( blankSheetShape, foldEdge, fusedSheetShell, history, interp->GetPlotter() );

  // Get images.
  const TopTools_ListOfShape& foldEdgeImages = history->Modified(foldEdge);
  TopoDS_Edge                 foldEdgeImage  = TopoDS::Edge( foldEdgeImages.First() );

  interp->GetPlotter().REDRAW_SHAPE("foldEdgeImage", foldEdgeImage);

  // Compose a new compound.
  BRep_Builder    bbuilder;
  TopoDS_Compound resCompound;
  //
  bbuilder.MakeCompound(resCompound);
  bbuilder.Add(resCompound, fusedSheetShell);
  bbuilder.Add(resCompound, bendLinesShape); // Add the remaining bend lines.

  /* ==============================
   *  Collect left and right faces.
   * ============================== */

  TopTools_IndexedDataMapOfShapeListOfShape edgesFacesMap;
  TopExp::MapShapesAndAncestors(resCompound, TopAbs_EDGE, TopAbs_FACE, edgesFacesMap);
  //
  const TopTools_ListOfShape* facesPtr = edgesFacesMap.Seek(foldEdgeImage);
  //
  if ( !facesPtr )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find neighbor faces for an "
                                                           "imprinted edge.");
    return TCL_ERROR;
  }

  const TopoDS_Face& leftFace  = TopoDS::Face( facesPtr->First() );
  const TopoDS_Face& rightFace = TopoDS::Face( facesPtr->Last() );

  /*interp->GetPlotter().REDRAW_SHAPE("leftFace", leftFace);
  interp->GetPlotter().REDRAW_SHAPE("rightFace", rightFace);*/

  Handle(asiAlgo_AAG)
    G = new asiAlgo_AAG(resCompound, true);
  //
  const int leftFaceId  = G->GetFaceId(leftFace);
  const int rightFaceId = G->GetFaceId(rightFace);

  asiAlgo_Feature allLeftFaces, allRightFaces;
  //
  allLeftFaces.Add(leftFaceId);
  allRightFaces.Add(rightFaceId);
  //
  {
    asiAlgo_Feature exclude;
    exclude.Add(rightFaceId);
    propagate(leftFaceId, G, exclude, allLeftFaces);
  }
  {
    asiAlgo_Feature exclude;
    exclude.Add(leftFaceId);
    propagate(rightFaceId, G, exclude, allRightFaces);
  }

  TopoDS_Compound allLeftFacesComp, allRightFacesComp;
  //
  bbuilder.MakeCompound(allLeftFacesComp);
  bbuilder.MakeCompound(allRightFacesComp);
  //
  for ( asiAlgo_Feature::Iterator fit(allLeftFaces); fit.More(); fit.Next() )
  {
    const int fid = fit.Key();
    const TopoDS_Face& F = G->GetFace(fid);

    bbuilder.Add(allLeftFacesComp, F);
  }
  //
  for ( asiAlgo_Feature::Iterator fit(allRightFaces); fit.More(); fit.Next() )
  {
    const int fid = fit.Key();
    const TopoDS_Face& F = G->GetFace(fid);

    bbuilder.Add(allRightFacesComp, F);
  }

  interp->GetPlotter().REDRAW_SHAPE("allLeftFacesComp", allLeftFacesComp);
  interp->GetPlotter().REDRAW_SHAPE("allRightFacesComp", allRightFacesComp);

  std::vector<t_shapeOnSide> allSeparatedFaces;

  // Index all faces.
  for ( TopExp_Explorer exp(allLeftFacesComp, TopAbs_FACE); exp.More(); exp.Next() )
  {
    allSeparatedFaces.push_back( t_shapeOnSide( exp.Current(), true ) );
  }
  //
  for ( TopExp_Explorer exp(allRightFacesComp, TopAbs_FACE); exp.More(); exp.Next() )
  {
    allSeparatedFaces.push_back( t_shapeOnSide( exp.Current(), false ) );
  }

  /* ================================
   *  Find left and right bend lines.
   * ================================ */

  NCollection_UBTree<int, Bnd_Box>       ubTree;
  NCollection_UBTreeFiller<int, Bnd_Box> ubTreeFiller(ubTree);

  // Add bounding boxes of the tentative faces to the filler.
  for ( int fi = 0; fi < int( allSeparatedFaces.size() ); ++fi )
  {
    Bnd_Box bbox;
    BRepBndLib::Add(allSeparatedFaces[fi].S, bbox);
    bbox.Enlarge(0.01);

    ubTreeFiller.Add(fi, bbox);
  }
  //
  ubTreeFiller.Fill();
  ubTreeFiller.CheckTree(std::cout);

  // Prepare selector initialized with the indexed objects.
  BendLineSelector blSelector(allSeparatedFaces);

  // Index bend lines.
  TopTools_IndexedMapOfShape allBendEdges;
  TopExp::MapShapes(bendLinesShape, TopAbs_EDGE, allBendEdges);
  std::vector<t_shapeOnSide> allSeparatedBendLines;
  //
  for ( int bi = 1; bi <= allBendEdges.Extent(); ++bi )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( allBendEdges(bi) );

    BRepAdaptor_Curve bac(edge);
    gp_Pnt P = bac.Value( ( bac.FirstParameter() + bac.LastParameter() )*0.5 );

    blSelector.Init(P);
    ubTree.Select(blSelector);

    allSeparatedBendLines.push_back( t_shapeOnSide( edge, blSelector.IsLeft() ) );
  }

  /* =============================================
   *  Compute border trihedron over the bend edge.
   * ============================================= */

  asiAlgo_BorderTrihedron btriLeft, btriRight;
  asiAlgo_Utils::ComputeBorderTrihedron(leftFace, foldEdge, btriLeft);
  asiAlgo_Utils::ComputeBorderTrihedron(rightFace, foldEdge, btriRight);

  gp_Dir refDir = 0.5*( btriLeft.V_z.XYZ() + btriLeft.V_z.XYZ() );

  // Stock props.
  BoxStock boxStock( PartAPI.GetShape() );
  //
  interp->GetPlotter().REDRAW_VECTOR_AT( "referenceAxis",
                                         boxStock.ComputeAnchorPoint(refDir),
                                         gp_Vec(refDir) * boxStock.scaleCoeff,
                                         boxStock.AxisColor() );

  interp->GetPlotter().REDRAW_AXES( "btriLeft",
                                     btriLeft.V_origin,
                                     btriLeft.V_x,
                                     btriLeft.V_y,
                                     btriLeft.V_z,
                                     boxStock.scaleCoeff*0.33 );

  interp->GetPlotter().REDRAW_AXES( "btriRight",
                                     btriRight.V_origin,
                                     btriRight.V_x,
                                     btriRight.V_y,
                                     btriRight.V_z,
                                     boxStock.scaleCoeff*0.33 );

  /* ================
   *  Apply rotation.
   * ================ */

  //gp_Lin straightLine;
  ////
  //if ( !asiAlgo_Utils::IsStraight(foldEdgeImage, straightLine, true) )
  //{
  //  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Folding edge is not straight.");
  //  return TCL_ERROR;
  //}

  //const gp_Dir& foldLineDir = straightLine.Direction();

  //// Build local axes for the part along the fold line.
  //gp_Ax3 smAxes( straightLine.Location(), refDir, foldLineDir );

  // Anchor axes at the global origin.
  gp_Ax3 anchorAxesLeft ( gp::Origin(), gp::DZ(),  gp::DX() );
  gp_Ax3 anchorAxesRight( gp::Origin(), gp::DZ(), -gp::DX() );

  // Compute placement transforms.
  gp_Trsf placementTleft  = asiAlgo_Utils::GetAlignmentTrsf(anchorAxesLeft,  btriLeft);
  gp_Trsf placementTright = asiAlgo_Utils::GetAlignmentTrsf(anchorAxesRight, btriRight);

  //// Draw relocated placement axes.
  //smAxes.Transform(placementT);
  //
  //interp->GetPlotter().REDRAW_AXES( "sheetMetalAxes_after",
  //                                   smAxes.Location(),
  //                                   smAxes.XDirection(),
  //                                   smAxes.YDirection(),
  //                                   smAxes.Direction(),
  //                                   boxStock.scaleCoeff*0.33 );

  // Prepare transformation by aligning the bend axis with origin
  // and applying half-angle rotation.
  gp_Trsf T_left, T_right;
  {
#if defined ApplyHalfAngleRotation
    gp_Quaternion qn(gp::DX(), (foldDir)*0.5*angleDeg/180.*M_PI);
#else
    gp_Quaternion qn(gp::DX(), (foldDir)*1.0*angleDeg/180.*M_PI);
#endif
    gp_Trsf R;
    R.SetRotation(qn);

    T_left = placementTleft.Inverted()*R*placementTleft;
  }
  {
#if defined ApplyHalfAngleRotation
    gp_Quaternion qn(gp::DX(), -(foldDir)*0.5*angleDeg/180.*M_PI);
#else
    gp_Quaternion qn;
#endif
    gp_Trsf R;
    R.SetRotation(qn);

    T_right = placementTright.Inverted()*R*placementTright;
  }

  // Apply transformations to all shapes.
  for ( auto& face : allSeparatedFaces )
  {
    face.S.Move( face.isLeft ? T_left : T_right );

    //interp->GetPlotter().DRAW_SHAPE(face.S, "rotated");
  }
  //
  for ( auto& bl : allSeparatedBendLines )
  {
    bl.S.Move( bl.isLeft ? T_left : T_right );
  }

  fs.Compose(allSeparatedFaces, allSeparatedBendLines);

  /* ==========
   *  Finalize.
   * ========== */

  // Update Data Model.
  cmdMisc::model->OpenCommand();
  {
    asiEngine_Part(cmdMisc::model).Update(fs);
  }
  cmdMisc::model->CommitCommand();

  // Actualize.
  if ( cmdMisc::cf->ViewerPart )
    cmdMisc::cf->ViewerPart->PrsMgr()->Actualize( cmdMisc::model->GetPartNode() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Folding(const Handle(asiTcl_Interp)&      interp,
                               const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("fold", "Folds a plat pattern.", __FILE__, group, MISC_Fold);
}
