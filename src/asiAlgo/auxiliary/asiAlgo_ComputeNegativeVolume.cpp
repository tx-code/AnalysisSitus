//-----------------------------------------------------------------------------
// Created on: 07 September 2022
// Author: Andrey Voevodin
//-----------------------------------------------------------------------------

// Own include
#include "asiAlgo_ComputeNegativeVolume.h"

// asiAlgo includes
#include <asiAlgo_ClassifyPt.h>
#include <asiAlgo_Cloudify.h>
#include <asiAlgo_BuildConvexHull.h>
#include <asiAlgo_InvertFaces.h>
#include <asiAlgo_InvertShells.h>

// OCCT includes
#include <BOPAlgo_Builder.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepClass_FaceExplorer.hxx>
#include <BRepClass_FClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRepLib.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRep_Tool.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GProp_GProps.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>

//-----------------------------------------------------------------------------

namespace {

//=======================================================================
// function: MakeSolid
// purpose: Make solid.
//=======================================================================
bool MakeSolid(const TopTools_ListOfShape&          listOfShape,
               TopoDS_Shape&                        result,
               TopTools_IndexedDataMapOfShapeShape& modifiedShapesInv,
               TopTools_IndexedDataMapOfShapeShape& modifiedShapes)
{
  modifiedShapesInv.Clear();
  TopoDS_Shape negativeVolumeShape;
  std::vector<TopoDS_Shell> shells;
  try
  {
    TopoDS_Shell shell;
    BRep_Builder bb;
    bb.MakeShell(shell);

    TopTools_ListOfShape::Iterator itFaces(listOfShape);
    for (; itFaces.More(); itFaces.Next())
    {
      bb.Add(shell, itFaces.Value());
    }

    if (shell.IsNull())
    {
      return false;
    }

    BRepBuilderAPI_Sewing tool;
    tool.Init(1.0e-4, true, true, true, true);
    tool.Add(shell);
    tool.Perform();
    TopoDS_Shape sewedShape = tool.SewedShape();
    if (sewedShape.IsNull())
    {
      return false;
    }

    itFaces.Initialize(listOfShape);
    for (; itFaces.More(); itFaces.Next())
    {
      if (tool.IsModified(itFaces.Value()))
      {
        modifiedShapesInv.Add(tool.Modified(itFaces.Value()), itFaces.Value());
        modifiedShapes.Add(itFaces.Value(), tool.Modified(itFaces.Value()));
      }
      else if (tool.IsModifiedSubShape(itFaces.Value()))
      {
        modifiedShapesInv.Add(tool.ModifiedSubShape(itFaces.Value()), itFaces.Value());
        modifiedShapes.Add(itFaces.Value(), tool.ModifiedSubShape(itFaces.Value()));
      }
      else
      {
        modifiedShapesInv.Add(itFaces.Value(), itFaces.Value());
        modifiedShapes.Add(itFaces.Value(), itFaces.Value());
      }
    }

    TopExp_Explorer exp(sewedShape, TopAbs_SHELL);
    for (; exp.More(); exp.Next())
    {
      if (!exp.Value().IsNull())
      {
        shells.push_back(TopoDS::Shell(exp.Value()));
      }
    }

    if (shells.empty())
    {
      return false;
    }

    negativeVolumeShape = sewedShape;
  }
  catch (...)
  {
    return false;
  }

  try
  {
    TopoDS_Solid solid;
    BRep_Builder solidBuilder;
    solidBuilder.MakeSolid(solid);
    std::vector<TopoDS_Shell>::const_iterator itShells = shells.cbegin();
    for (; itShells != shells.cend(); ++itShells)
    {
      solidBuilder.Add(solid, *itShells);
    }

    negativeVolumeShape = solid;
    if (negativeVolumeShape.IsNull())
    {
      return false;
    }

    try
    {
      BRepClass3d_SolidClassifier classifier(negativeVolumeShape);
      classifier.PerformInfinitePoint(Precision::Confusion());
      if (classifier.State() == TopAbs_IN)
      {
        asiAlgo_InvertShells inverter(negativeVolumeShape);
        if (!inverter.Perform() || inverter.GetResult().IsNull())
        {
          return false;
        }
        negativeVolumeShape = inverter.GetResult();
      }
    }
    catch (...)
    {
      return false;
    }
  }
  catch (...)
  {
    return false;
  }

  result = negativeVolumeShape;

  return true;
}

//=======================================================================
// function: TakeModified
// purpose: Stores the modified object into the list
//=======================================================================
void TakeModified(const TopoDS_Shape&   shape,
                  BOPAlgo_Builder&      builder,
                  TopTools_ListOfShape& listOfShape)
{
  const TopTools_ListOfShape& modified = builder.Modified(shape);
  if ( modified.IsEmpty() && !builder.IsDeleted(shape) )
  {
    listOfShape.Append(shape);
  }
  else
  {
    TopTools_ListIteratorOfListOfShape itM(modified);
    for ( ; itM.More(); itM.Next() )
    {
      listOfShape.Append(itM.Value());
    }
  }
}

//=======================================================================
// function: FindInternals
// purpose: Looks for internal shapes inside the face or solid
//=======================================================================
void FindInternals(const TopoDS_Shape&   shape,
                   TopTools_ListOfShape& internalShapes)
{
  TopoDS_Iterator itS(shape);
  for (; itS.More(); itS.Next())
  {
    const TopoDS_Shape& aSS = itS.Value();
    if (aSS.Orientation() == TopAbs_INTERNAL)
      internalShapes.Append(aSS);
    else
    {
      TopoDS_Iterator itSS(aSS);
      for (; itSS.More(); itSS.Next())
      {
        if (itSS.Value().Orientation() == TopAbs_INTERNAL)
        {
          internalShapes.Append(aSS);
          break;
        }
      }
    }
  }
}

//=======================================================================
// function: RemoveInternalWires
// purpose: Removes internal wires from the faces
//=======================================================================
void RemoveInternalWires(const TopTools_ListOfShape& theShapes,
                         TopTools_ListOfShape *theRemoved = NULL)
{
  TopTools_ListIteratorOfListOfShape itLS(theShapes);
  for (; itLS.More(); itLS.Next())
  {
    const TopoDS_Shape& aShape = itLS.Value();
    TopExp_Explorer anExpF(aShape, TopAbs_FACE);
    for (; anExpF.More(); anExpF.Next())
    {
      TopoDS_Face& aF = *(TopoDS_Face*)&anExpF.Current();
      TopTools_ListOfShape aLWToRemove;
      FindInternals(aF, aLWToRemove);
      if (aLWToRemove.Extent())
      {
        aF.Free(Standard_True);
        TopTools_ListIteratorOfListOfShape itR(aLWToRemove);
        for (; itR.More(); itR.Next())
        {
          if (theRemoved)
            theRemoved->Append(itR.Value());
          BRep_Builder().Remove(aF, itR.Value());
        }
        aF.Free(Standard_False);
      }
    }
  }
}

//=======================================================================
// function: FindShape
// purpose: Find the shape in the other shape
//=======================================================================
void FindShape(const TopoDS_Shape& theSWhat,
               const TopoDS_Shape& theSWhere,
               TopoDS_Shape& theSFound)
{
  TopExp_Explorer anExp(theSWhere, theSWhat.ShapeType());
  for (; anExp.More(); anExp.Next())
  {
    const TopoDS_Shape& aS = anExp.Current();
    if (aS.IsSame(theSWhat))
    {
      theSFound = aS;
      break;
    }
  }
}

//=======================================================================
// function: FindExtraShape
// purpose: Find shapes possibly filling the holes in the original shape
//=======================================================================
void FindExtraShapes(const TopTools_IndexedDataMapOfShapeListOfShape& theConnectionMap,
                     const TopTools_MapOfShape& theShapesToCheckOri,
                     BOPAlgo_Builder& theBuilder,
                     TopTools_MapOfShape& theShapesToAvoid,
                     TopTools_MapOfShape* theValidShapes = NULL)
{
  Handle(IntTools_Context) aCtx = theBuilder.Context();
  TopTools_MapOfShape aValidShapes;
  TopTools_MapOfShape* pValidShapes = theValidShapes ? theValidShapes : &aValidShapes;
  TopTools_MapIteratorOfMapOfShape itM(theShapesToCheckOri);
  for (; itM.More(); itM.Next())
  {
    const TopoDS_Shape& aSToCheckOri = itM.Value();
    // Check modification of the shape during intersection
    TopTools_ListOfShape aLSIm;
    TakeModified(aSToCheckOri, theBuilder, aLSIm);

    TopTools_ListIteratorOfListOfShape itLSIm(aLSIm);
    for (; itLSIm.More(); itLSIm.Next())
    {
      const TopoDS_Shape& aSIm = itLSIm.Value();

      const TopTools_ListOfShape* pShapesToValidate = theConnectionMap.Seek(aSIm);
      if (!pShapesToValidate)
        continue;

      TopTools_ListIteratorOfListOfShape itSV(*pShapesToValidate);
      for (; itSV.More(); itSV.Next())
      {
        const TopoDS_Shape& aShapeToValidate = itSV.Value();
        if (pValidShapes->Contains(aShapeToValidate))
          continue;

        TopoDS_Edge aSInShape;
        FindShape(aSIm, aShapeToValidate, aSInShape);

        Standard_Boolean bSameOri =
          !BOPTools_AlgoTools::IsSplitToReverse(aSInShape, aSToCheckOri, aCtx);

        if (bSameOri)
          pValidShapes->Add(aShapeToValidate);
        else
          theShapesToAvoid.Add(aShapeToValidate);
      }
    }
  }

  itM.Initialize(*pValidShapes);
  for (; itM.More(); itM.Next())
    theShapesToAvoid.Remove(itM.Value());
}

//=======================================================================
// class: FillGaps
// purpose: Auxiliary class for creation of the faces for filling the gap
//          created by removal of the single feature
//=======================================================================
class FillGap
{
public: //! @name Constructors

  //! Empty constructor
  FillGap(const Handle(asiAlgo_AAG)& aag,
          const asiAlgo_Feature      faceIDs,
          const bool                 isOneSolid) :
    myAAG(aag),
    myFaceIDs(faceIDs),
    myIsOneSolid(isOneSolid),
    myHasAdjacentFaces(Standard_False)
  {
    BRep_Builder bb;
    TopoDS_Compound comp;
    bb.MakeCompound(comp);
    asiAlgo_Feature::Iterator it(myFaceIDs);
    for (; it.More(); it.Next())
    {
      bb.Add(comp, myAAG->GetFace(it.Key()));
    }

    myFeature = comp;
  }

public: //! @name Setters/Getters

  //! Sets the feature to remove
  void SetFeature(const TopoDS_Shape& theFeature) { myFeature = theFeature; }

  //! Returns the feature
  const TopoDS_Shape& Feature() const { return myFeature; }

  const TopoDS_Shape& Shape() const { return myResultShape; }

public: //! @name Perform the operation

  //! Performs the extension of the adjacent faces and
  //! then trims the extended faces to fill the gaps
  void Perform()
  {
    OCC_CATCH_SIGNALS

    try
    {
      // Find the faces adjacent to the faces of the feature
      TopTools_IndexedMapOfShape aMFAdjacent;
      FindAdjacentFaces(aMFAdjacent);

      myHasAdjacentFaces = (aMFAdjacent.Extent() > 0);
      if (!myHasAdjacentFaces)
        return;

      // Extend the adjacent faces keeping the connection to the original faces
      TopTools_IndexedDataMapOfShapeShape aFaceExtFaceMap;
      ExtendAdjacentFaces(aMFAdjacent, aFaceExtFaceMap);

      TopExp_Explorer itLF1(myFeature, TopAbs_FACE);
      for (; itLF1.More(); itLF1.Next())
      {
        aFaceExtFaceMap.Add(itLF1.Value(), itLF1.Value());
      }

      // Trim the extended faces
      TrimExtendedFaces(aFaceExtFaceMap);
    }
    catch (Standard_Failure const&)
    {
      // Make sure the warning will be given on the higher level
      myHasAdjacentFaces = Standard_True;
      myFaces.Clear();
      myUniqueFaces.Clear();
    }
  }

public: //! @name Obtain the result

  //! Shows whether the adjacent faces have been found for the feature
  Standard_Boolean HasAdjacentFaces() const { return myHasAdjacentFaces; }

  //! Returns the Images map of the adjacent faces
  const TopTools_IndexedDataMapOfShapeListOfShape& Faces() const { return myFaces; }

  const TopTools_ListOfShape& UniqueFaces() const { return myUniqueFaces; }

private: //! @name Private methods performing the operation

  //! Finds the faces adjacent to the feature and stores them into outgoing map.
  void FindAdjacentFaces(TopTools_IndexedMapOfShape& theMFAdjacent)
  {
    asiAlgo_Feature featureNeighbors;
    asiAlgo_Feature::Iterator it(myFaceIDs);
    for (; it.More(); it.Next())
    {
      asiAlgo_Feature neighbors = myAAG->GetNeighbors(it.Key());
      featureNeighbors.Unite(neighbors);
    }

    featureNeighbors.Subtract(myFaceIDs);
    it.Initialize(featureNeighbors);
    for (; it.More(); it.Next())
    {
      theMFAdjacent.Add(myAAG->GetFace(it.Key()));
    }
  }

  //! Extends the found adjacent faces and binds them to the original faces.
  void ExtendAdjacentFaces(const TopTools_IndexedMapOfShape& theMFAdjacent,
                           TopTools_IndexedDataMapOfShapeShape& theFaceExtFaceMap)
  {
    // Get the extension value for the faces - half of the diagonal of bounding box of the feature
    Bnd_Box aFeatureBox;
    BRepBndLib::Add(myFeature, aFeatureBox);

    const Standard_Real anExtLength = sqrt(aFeatureBox.SquareExtent());

    const Standard_Integer aNbFA = theMFAdjacent.Extent();
    for (Standard_Integer i = 1; i <= aNbFA; ++i)
    {
      const TopoDS_Face& aF = TopoDS::Face(theMFAdjacent(i));
      // Extend the face
      TopoDS_Face aFExt;
      BRepLib::ExtendFace(aF, anExtLength,
                          Standard_True, Standard_True,
                          Standard_True, Standard_True,
                          aFExt);
      theFaceExtFaceMap.Add(aF, aFExt);
    }
  }

  //! Trims the extended adjacent faces by intersection with each other
  //! and following intersection with the bounds of original faces.
  void TrimExtendedFaces(const TopTools_IndexedDataMapOfShapeShape& theFaceExtFaceMap)
  {
    // Intersect the extended faces first
    BOPAlgo_Builder aGFInter;
    // Add faces for intersection
    const Standard_Integer aNbF = theFaceExtFaceMap.Extent();
    for (Standard_Integer i = 1; i <= aNbF; ++i)
      aGFInter.AddArgument(theFaceExtFaceMap(i));

    aGFInter.SetRunParallel(false);

    // Intersection result
    TopoDS_Shape anIntResult;
    if (aGFInter.Arguments().Extent() > 1)
    {
      aGFInter.Perform();
      if (aGFInter.HasErrors())
        return;

      anIntResult = aGFInter.Shape();
    }
    else
      anIntResult = aGFInter.Arguments().First();

    // Prepare the EF map of the extended faces after intersection
    // to select from them only boundary edges
    TopTools_IndexedDataMapOfShapeListOfShape anEFExtMap;
    TopExp::MapShapesAndAncestors(anIntResult, TopAbs_EDGE, TopAbs_FACE, anEFExtMap);

    // Get the splits of the extended faces after intersection
    // and trim them by the edges of the original faces

    // Map the edges of the Feature to avoid them during the trim
    TopTools_IndexedMapOfShape aFeatureEdgesMap;
    TopExp::MapShapes(myFeature, TopAbs_EDGE, aFeatureEdgesMap);

    TopTools_IndexedDataMapOfShapeListOfShape notDefinedFacesMap;

    for (Standard_Integer i = 1; i <= aNbF; ++i)
    {
      const TopoDS_Face& aFOriginal = TopoDS::Face(theFaceExtFaceMap.FindKey(i));
      const TopoDS_Face& aFExt = TopoDS::Face(theFaceExtFaceMap(i));
      TrimFace(aFExt, aFOriginal, aFeatureEdgesMap, anEFExtMap, aGFInter, notDefinedFacesMap);
    }

    FilterShape();
  }

  void FilterShape()
  {
    TopoDS_Shape negativeVolumeShape;
    TopTools_IndexedDataMapOfShapeShape modifiedShapesInv;
    TopTools_IndexedDataMapOfShapeShape modifiedShapes;
    {
      modifiedShapesInv.Clear();
      modifiedShapes.Clear();
      TopTools_ListOfShape listOfShape;
      TopTools_ListOfShape::Iterator itFaces(myUniqueFaces);
      for (; itFaces.More(); itFaces.Next())
      {
        listOfShape.Append(itFaces.Value());
      }

      if (!MakeSolid(listOfShape, negativeVolumeShape, modifiedShapesInv, modifiedShapes))
      {
        return;
      }
    }

    // Remove faces into initial shape.
    {
      TopExp_Explorer exp(negativeVolumeShape, TopAbs_FACE);
      for (; exp.More(); exp.Next())
      {
        gp_Pnt checkedPnt;
        bool isFound = false;
        BRepAdaptor_Surface   checkedfaceAdapt(TopoDS::Face(exp.Value()));
        math_BullardGenerator RNG;
        int numSamples = 10;
        for (int i = 0; i < numSamples; ++i)
        {
          // Get a random sample point.
          gp_Pnt2d uv;
          if (!asiAlgo_Utils::GetRandomPoint(checkedfaceAdapt.Face(), RNG, uv))
          {
            continue;
          }

          checkedfaceAdapt.D0(uv.X(), uv.Y(), checkedPnt);
          isFound = true;
          break;
        }

        if (!isFound)
        {
          if (modifiedShapesInv.Contains(exp.Value()))
          {
            const TopoDS_Shape& faceToRemove = modifiedShapesInv.FindFromKey(exp.Value());

            myUniqueFaces.Remove(faceToRemove);
          }
          continue;
        }

        BRepClass3d_SolidClassifier solidClassifier(myAAG->GetMasterShape());
        solidClassifier.Perform(checkedPnt, 1.0e-4);
        TopAbs_State state = solidClassifier.State();
        if (state == TopAbs_IN)
        {
          if (modifiedShapesInv.Contains(exp.Value()))
          {
            const TopoDS_Shape& faceToRemove = modifiedShapesInv.FindFromKey(exp.Value());

            myUniqueFaces.Remove(faceToRemove);
          }
          continue;
        }
      }
    }

    {
      modifiedShapesInv.Clear();
      modifiedShapes.Clear();
      TopTools_ListOfShape listOfShape;
      TopTools_ListOfShape::Iterator itFaces(myUniqueFaces);
      for (; itFaces.More(); itFaces.Next())
      {
        listOfShape.Append(itFaces.Value());
      }

      if (!MakeSolid(listOfShape, negativeVolumeShape, modifiedShapesInv, modifiedShapes))
      {
        return;
      }
    }

    // Remove faces with open edges.
    int maxNbStep = 100;
    int nbTopoRemove = 0;
    int nbTopoStep = 0;
    do
    {
      ++nbTopoStep;
      nbTopoRemove = 0;

      TopTools_MapOfShape removedFaces;
      TopTools_IndexedDataMapOfShapeListOfShape edgesFacesMap;
      TopExp::MapShapesAndAncestors(negativeVolumeShape, TopAbs_EDGE, TopAbs_FACE, edgesFacesMap);
      for (int index = 1; index <= edgesFacesMap.Extent(); ++index)
      {
        const TopTools_ListOfShape& faces = edgesFacesMap(index);
        const TopoDS_Edge& edge = TopoDS::Edge(edgesFacesMap.FindKey(index));
        if (!BRep_Tool::Degenerated(edge) && faces.Extent() == 1 && modifiedShapesInv.Contains(faces.First()))
        {
          const TopoDS_Shape& faceToRemove = modifiedShapesInv.FindFromKey(faces.First());
          myUniqueFaces.Remove(faceToRemove);
          ++nbTopoRemove;
        }
      }

      {
        modifiedShapesInv.Clear();
        modifiedShapes.Clear();
        TopTools_ListOfShape listOfShape;
        TopTools_ListOfShape::Iterator itFaces(myUniqueFaces);
        for (; itFaces.More(); itFaces.Next())
        {
          listOfShape.Append(itFaces.Value());
        }

        if (!MakeSolid(listOfShape, negativeVolumeShape, modifiedShapesInv, modifiedShapes))
        {
          return;
        }
      }
    } while (nbTopoRemove != 0 && nbTopoStep < maxNbStep);


    // Remove inner faces.
    {
      Bnd_Box bndBox;
      BRepBndLib::AddOptimal(negativeVolumeShape, bndBox, false, false);
      bndBox.Enlarge(1.0);
      TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(bndBox.CornerMin(), bndBox.CornerMax());
      TopTools_IndexedMapOfShape boxShapeMap;
      TopExp::MapShapes(boxShape, TopAbs_FACE, boxShapeMap);

      BRepAlgoAPI_Cut API;
      asiAlgo_Utils::BooleanCut(boxShape, negativeVolumeShape, false, 0.0, API);
      TopoDS_Shape cutShape = API.Shape();

      Handle(Poly_Triangulation) hull;
      asiAlgo_BuildConvexHull chBuilder;
      bool isUsedHull = chBuilder.Perform(myFeature, hull, true);

      std::vector<TopoDS_Shape> solidsToRecheck;
      bool isFromStock = false;
      Handle(BRepTools_History) history = API.History();
      if (!history.IsNull() && !cutShape.IsNull())
      {

        TopTools_MapOfShape newFeaturesFacesTmp;
        TopTools_MapOfShape::Iterator itFF(myFeaturesFaces);
        for (; itFF.More(); itFF.Next())
        {
          if (modifiedShapes.Contains(itFF.Value()))
          {
            const TopoDS_Shape& face = modifiedShapes.FindFromKey(itFF.Value());
            newFeaturesFacesTmp.Add(face);
          }
          else
          {
            newFeaturesFacesTmp.Add(itFF.Value());
          }
        }

        TopTools_MapOfShape newFeaturesFaces;
        TopTools_MapOfShape::Iterator itFFTMP(newFeaturesFacesTmp);
        for (; itFFTMP.More(); itFFTMP.Next())
        {
          if (history->IsRemoved(itFFTMP.Value()))
          {
            continue;
          }
          const TopTools_ListOfShape& generatedShape = history->Generated(itFFTMP.Value());
          TopTools_ListOfShape::Iterator itG(generatedShape);
          for (; itG.More(); itG.Next())
          {
            const TopoDS_Shape& locGen = itG.Value();
            TopExp_Explorer expLG(locGen, TopAbs_FACE);
            for (; expLG.More(); expLG.Next())
            {
              newFeaturesFaces.Add(expLG.Value());
            }
          }

          const TopTools_ListOfShape& modifiedShape = history->Modified(itFFTMP.Value());
          TopTools_ListOfShape::Iterator itM(modifiedShape);
          for (; itM.More(); itM.Next())
          {
            const TopoDS_Shape& locMod = itM.Value();
            TopExp_Explorer expLM(locMod, TopAbs_FACE);
            for (; expLM.More(); expLM.Next())
            {
              newFeaturesFaces.Add(expLM.Value());
            }
          }

          if (modifiedShape.IsEmpty() && generatedShape.IsEmpty())
          {
            newFeaturesFaces.Add(itFFTMP.Value());
          }
        }

        TopoDS_Compound comp;
        BRep_Builder bb;
        bb.MakeCompound(comp);
        {
          TopExp_Explorer expSolids(cutShape, TopAbs_SOLID);
          for (; expSolids.More(); expSolids.Next())
          {
            TopoDS_Shape solid = expSolids.Value();

            bool isStock = false;
            bool isFound = false;
            TopExp_Explorer expFaces(expSolids.Value(), TopAbs_FACE);
            for (; expFaces.More(); expFaces.Next())
            {
              if (boxShapeMap.Contains(expFaces.Value()))
              {
                isStock = true;
                isFound = false;
                break;
              }

              if (newFeaturesFaces.Contains(expFaces.Value()))
              {
                isFound = true;
              }
            }
            if (isFound)
            {
              bb.Add(comp, expSolids.Value());
              continue;
            }
            if (isStock)
            {
              continue;
            }

            if (isUsedHull)
            {
              bool isIn = false;

              Handle(asiAlgo_BaseCloud<double>) point_cloud;
              asiAlgo_Cloudify cloudify;
              cloudify.Sample_Faces(solid, point_cloud);

              ClassifyPt classifyPt(hull);
              for (int index = 0; index < point_cloud->GetNumberOfElements(); ++index)
              {
                gp_XYZ point = point_cloud->GetElement(index);
                if (classifyPt.IsIn(point, Precision::Confusion()))
                {
                  isIn = true;
                  break;
                }
              }

              if (isIn)
              {
                bb.Add(comp, expSolids.Value());
                continue;
              }
            }

            solidsToRecheck.push_back(expSolids.Value());

          }
          cutShape = comp;
        }

        TopExp_Explorer expCS(cutShape, TopAbs_SOLID);
        if (!expCS.More())
        {
          cutShape = API.Shape();
          {
            bool isFound = false;
            TopExp_Explorer expSolids(cutShape, TopAbs_SOLID);
            for (; expSolids.More(); expSolids.Next())
            {
              TopExp_Explorer expFaces(expSolids.Value(), TopAbs_FACE);
              for (; expFaces.More(); expFaces.Next())
              {
                if (boxShapeMap.Contains(expFaces.Value()))
                {
                  isFound = true;
                  break;
                }
              }
              if (isFound)
              {
                isFromStock = true;
                cutShape = expSolids.Value();
                break;
              }
            }
          }
        }
        else
        {
          TopoDS_Compound compCutShape;
          BRep_Builder bbCutShape;
          bbCutShape.MakeCompound(compCutShape);
          bbCutShape.Add(compCutShape, cutShape);

          Handle(Poly_Triangulation) hullCutShape;
          asiAlgo_BuildConvexHull chBuilderCutShape;
          if (chBuilderCutShape.Perform(compCutShape, hullCutShape, true))
          {
            std::vector<TopoDS_Shape>::const_iterator itSolids = solidsToRecheck.cbegin();
            for (; itSolids != solidsToRecheck.cend(); ++itSolids)
            {
              Handle(asiAlgo_BaseCloud<double>) point_cloud;
              asiAlgo_Cloudify cloudify;
              cloudify.Sample_Faces(*itSolids, point_cloud);

              ClassifyPt classifyPt(hullCutShape);
              for (int index = 0; index < point_cloud->GetNumberOfElements(); ++index)
              {
                gp_XYZ point = point_cloud->GetElement(index);
                if (classifyPt.IsIn(point, 1.0))
                {
                  bbCutShape.Add(compCutShape, *itSolids);
                  break;
                }
              }
            }
          }

          cutShape = compCutShape;
        }

        TopTools_IndexedMapOfShape cutFaceMap;
        TopExp::MapShapes(cutShape, TopAbs_FACE, cutFaceMap);

        TopExp_Explorer exp(negativeVolumeShape, TopAbs_FACE);
        for (; exp.More(); exp.Next())
        {
          bool hasShape = cutFaceMap.Contains(exp.Value());

          if (!hasShape && history->HasRemoved() && history->IsRemoved(exp.Value()))
          {
            hasShape = false;
          }

          if (!hasShape && history->HasGenerated())
          {
            const TopTools_ListOfShape& generatedShape = history->Generated(exp.Value());
            TopTools_ListOfShape::Iterator itG(generatedShape);
            for (; itG.More(); itG.Next())
            {
              const TopoDS_Shape& locGen = itG.Value();
              TopExp_Explorer expLG(locGen, TopAbs_FACE);
              for (; expLG.More(); expLG.Next())
              {
                if (cutFaceMap.Contains(expLG.Value()))
                {
                  hasShape = true;
                  break;
                }
              }
            }
          }

          if (!hasShape && history->HasModified())
          {
            const TopTools_ListOfShape& modifiedShape = history->Modified(exp.Value());
            TopTools_ListOfShape::Iterator itM(modifiedShape);
            for (; itM.More(); itM.Next())
            {
              const TopoDS_Shape& locMod = itM.Value();
              TopExp_Explorer expLM(locMod, TopAbs_FACE);
              for (; expLM.More(); expLM.Next())
              {
                if (cutFaceMap.Contains(expLM.Value()))
                {
                  hasShape = true;
                  break;
                }
              }
            }
          }

          if (!hasShape)
          {
            if (modifiedShapesInv.Contains(exp.Value()))
            {
              const TopoDS_Shape& faceToRemove = modifiedShapesInv.FindFromKey(exp.Value());

              myUniqueFaces.Remove(faceToRemove);
            }
          }
        }
      }

      if (!isFromStock)
      {
        myResultShape = cutShape;
      }
      else
      {
        modifiedShapesInv.Clear();
        modifiedShapes.Clear();
        TopTools_ListOfShape listOfShape;
        TopTools_ListOfShape::Iterator itFaces(myUniqueFaces);
        for (; itFaces.More(); itFaces.Next())
        {
          listOfShape.Append(itFaces.Value());
        }

        if (!MakeSolid(listOfShape, negativeVolumeShape, modifiedShapesInv, modifiedShapes))
        {
          return;
        }

        myResultShape = negativeVolumeShape;
      }
    } // Remove inner faces.

    // Final filter.
    if (myIsOneSolid)
    {
      TopTools_IndexedMapOfShape finalShapesMap;
      TopExp::MapShapes(myResultShape, TopAbs_SOLID, finalShapesMap);
      if (finalShapesMap.Size() == 1)
      {
        myResultShape = TopoDS::Solid(finalShapesMap(1));
      }
      else if (finalShapesMap.Size() > 1)
      {
        {
          modifiedShapesInv.Clear();
          modifiedShapes.Clear();
          TopTools_ListOfShape listOfShape;
          TopTools_ListOfShape::Iterator itFaces(myUniqueFaces);
          for (; itFaces.More(); itFaces.Next())
          {
            listOfShape.Append(itFaces.Value());
          }

          if (!MakeSolid(listOfShape, negativeVolumeShape, modifiedShapesInv, modifiedShapes))
          {
            return;
          }
        }

        Bnd_Box bndBox;
        BRepBndLib::AddOptimal(negativeVolumeShape, bndBox, false, false);
        bndBox.Enlarge(1.0);
        TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(bndBox.CornerMin(), bndBox.CornerMax());
        TopTools_IndexedMapOfShape boxShapeMap;
        TopExp::MapShapes(boxShape, TopAbs_FACE, boxShapeMap);

        BRepAlgoAPI_Cut API;
        asiAlgo_Utils::BooleanCut(boxShape, negativeVolumeShape, false, 0.0, API);
        TopoDS_Shape cutShape = API.Shape();

        Handle(BRepTools_History) history = API.History();
        if (!history.IsNull() && !cutShape.IsNull())
        {
          bool isFound = false;
          TopExp_Explorer expSolids(cutShape, TopAbs_SOLID);
          for (; expSolids.More(); expSolids.Next())
          {
            TopExp_Explorer expFaces(expSolids.Value(), TopAbs_FACE);
            for (; expFaces.More(); expFaces.Next())
            {
              if (boxShapeMap.Contains(expFaces.Value()))
              {
                isFound = true;
                break;
              }
            }
            if (isFound)
            {
              cutShape = expSolids.Value();
              break;
            }
          }

          TopTools_IndexedMapOfShape cutFaceMap;
          TopExp::MapShapes(cutShape, TopAbs_FACE, cutFaceMap);

          TopExp_Explorer exp(negativeVolumeShape, TopAbs_FACE);
          for (; exp.More(); exp.Next())
          {
            bool hasShape = cutFaceMap.Contains(exp.Value());

            if (!hasShape && history->HasRemoved() && history->IsRemoved(exp.Value()))
            {
              hasShape = false;
            }

            if (!hasShape && history->HasGenerated())
            {
              const TopTools_ListOfShape& generatedShape = history->Generated(exp.Value());
              TopTools_ListOfShape::Iterator itG(generatedShape);
              for (; itG.More(); itG.Next())
              {
                const TopoDS_Shape& locGen = itG.Value();
                TopExp_Explorer expLG(locGen, TopAbs_FACE);
                for (; expLG.More(); expLG.Next())
                {
                  if (cutFaceMap.Contains(expLG.Value()))
                  {
                    hasShape = true;
                    break;
                  }
                }
              }
            }

            if (!hasShape && history->HasModified())
            {
              const TopTools_ListOfShape& modifiedShape = history->Modified(exp.Value());
              TopTools_ListOfShape::Iterator itM(modifiedShape);
              for (; itM.More(); itM.Next())
              {
                const TopoDS_Shape& locMod = itM.Value();
                TopExp_Explorer expLM(locMod, TopAbs_FACE);
                for (; expLM.More(); expLM.Next())
                {
                  if (cutFaceMap.Contains(expLM.Value()))
                  {
                    hasShape = true;
                    break;
                  }
                }
              }
            }

            if (!hasShape)
            {
              if (modifiedShapesInv.Contains(exp.Value()))
              {
                const TopoDS_Shape& faceToRemove = modifiedShapesInv.FindFromKey(exp.Value());

                myUniqueFaces.Remove(faceToRemove);
              }
            }
          }

          {
            modifiedShapesInv.Clear();
            modifiedShapes.Clear();
            TopTools_ListOfShape listOfShape;
            TopTools_ListOfShape::Iterator itFaces(myUniqueFaces);
            for (; itFaces.More(); itFaces.Next())
            {
              listOfShape.Append(itFaces.Value());
            }

            if (!MakeSolid(listOfShape, negativeVolumeShape, modifiedShapesInv, modifiedShapes))
            {
              return;
            }

            myResultShape = negativeVolumeShape;
          }

        }

      }

      finalShapesMap.Clear();
      TopExp::MapShapes(myResultShape, TopAbs_SOLID, finalShapesMap);
      if (finalShapesMap.Size() != 1)
      {
        myResultShape = TopoDS_Shape();
        return;
      }

    } // Final filter.

  }

  //! Trim the extended faces by the bounds of the original face,
  //! except those contained in the feature to remove.
  void TrimFace(const TopoDS_Face&                               theFExt,
                const TopoDS_Face&                               theFOriginal,
                const TopTools_IndexedMapOfShape&                theFeatureEdgesMap,
                const TopTools_IndexedDataMapOfShapeListOfShape& theEFExtMap,
                BOPAlgo_Builder&                                 theGFInter,
                TopTools_IndexedDataMapOfShapeListOfShape&       notDefinedFaces)
  {
    TopTools_MapOfShape featureFaces;
    {
      TopExp_Explorer itLF1(myFeature, TopAbs_FACE);
      for (; itLF1.More(); itLF1.Next())
      {
        featureFaces.Add(itLF1.Value());
      }
    }

    // Map all edges of the extended face, to filter the result of trim
    // from the faces containing these edges
    TopTools_MapOfShape aMExtEdges;
    TopExp_Explorer anExpE(theFExt, TopAbs_EDGE);
    for (; anExpE.More(); anExpE.Next())
    {
      const TopoDS_Edge& aE = TopoDS::Edge(anExpE.Current());
      // skip degenerated and seam edges
      if (BRep_Tool::Degenerated(aE) || BRep_Tool::IsClosed(aE, theFExt))
        continue;
      TopTools_ListOfShape aLEIm;
      TakeModified(aE, theGFInter, aLEIm);
      TopTools_ListIteratorOfListOfShape itLEIm(aLEIm);
      for (; itLEIm.More(); itLEIm.Next())
      {
        const TopoDS_Shape& aEIm = itLEIm.Value();
        if (theEFExtMap.FindFromKey(aEIm).Extent() == 1)
          aMExtEdges.Add(aEIm);
      }
    }

    // Trimming tool
    BOPAlgo_Builder aGFTrim;

    // Get the splits of the face and add them for trimming
    TopTools_ListOfShape anExtSplits;
    TakeModified(theFExt, theGFInter, anExtSplits);
    aGFTrim.SetArguments(anExtSplits);

    // Add edges of the original faces
    TopTools_MapOfShape aMEdgesToCheckOri;
    anExpE.Init(theFOriginal, TopAbs_EDGE);
    for (; anExpE.More(); anExpE.Next())
    {
      if (!featureFaces.Contains(theFOriginal))
      {
        const TopoDS_Edge& aE = TopoDS::Edge(anExpE.Current());
        if (!theFeatureEdgesMap.Contains(aE))
        {
          aGFTrim.AddArgument(aE);
          if (!BRep_Tool::Degenerated(aE) &&
            !BRep_Tool::IsClosed(aE, theFOriginal))
          {
            if (!aMEdgesToCheckOri.Add(aE))
              aMEdgesToCheckOri.Remove(aE);
          }
        }
      }
      else
      {
        const TopoDS_Edge& aE = TopoDS::Edge(anExpE.Current());
          aGFTrim.AddArgument(aE);
          if (!BRep_Tool::Degenerated(aE) &&
            !BRep_Tool::IsClosed(aE, theFOriginal))
          {
            if (!aMEdgesToCheckOri.Add(aE))
              aMEdgesToCheckOri.Remove(aE);
          }
      }

    }

    //! Add edges from feature.
    for (TopTools_IndexedMapOfShape::Iterator itFEM(theFeatureEdgesMap); itFEM.More(); itFEM.Next())
    {
      const TopoDS_Edge& aE = TopoDS::Edge(itFEM.Value());
      aGFTrim.AddArgument(aE);
      aMEdgesToCheckOri.Add(aE);
    }

    // Avoid faces intersection
    aGFTrim.SetGlue(BOPAlgo_GlueShift);
    aGFTrim.SetRunParallel(false);
    aGFTrim.SetNonDestructive(Standard_True);

    aGFTrim.Perform();
    if (aGFTrim.HasErrors())
      return;

    // Get all splits
    const TopoDS_Shape& aSplits = aGFTrim.Shape();

    // Remove dangling faces.
    TopTools_IndexedDataMapOfShapeListOfShape splitEFMap;
    TopExp::MapShapesAndAncestors(aSplits, TopAbs_EDGE, TopAbs_FACE, splitEFMap);
    for (int index = 1; index <= splitEFMap.Extent(); ++index)
    {
      const TopTools_ListOfShape& faces = splitEFMap(index);
      if (faces.Extent() == 0)
      {
        aGFTrim.History()->Remove(TopoDS::Edge(splitEFMap.FindKey(index)));
      }
    }

    TopTools_IndexedMapOfShape theFeatureEdgesMapNew;
    for (TopTools_IndexedMapOfShape::Iterator itFEM(theFeatureEdgesMap); itFEM.More(); itFEM.Next())
    {
      for (TopTools_ListOfShape::Iterator itGen(aGFTrim.History()->Generated(itFEM.Value())); itGen.More(); itGen.Next())
      {
        theFeatureEdgesMapNew.Add(itGen.Value());
      }

      for (TopTools_ListOfShape::Iterator itMod(aGFTrim.History()->Modified(itFEM.Value())); itMod.More(); itMod.Next())
      {
        theFeatureEdgesMapNew.Add(itMod.Value());
      }
    }

    // Filter the trimmed faces and save the valid ones into result map
    TopTools_ListOfShape aLFTrimmed;

    TopExp_Explorer anExpF(aSplits, TopAbs_FACE);
    for (; anExpF.More(); anExpF.Next())
    {
      const TopoDS_Shape& aSp = anExpF.Current();
      anExpE.Init(aSp, TopAbs_EDGE);
      for (; anExpE.More(); anExpE.Next())
      {
        if (aMExtEdges.Contains(anExpE.Current()))
          break;
      }
      if (!anExpE.More())
        aLFTrimmed.Append(aSp);
    }

    TopTools_ListOfShape notDefinedFacesLoc;
    if (aLFTrimmed.Extent() > 1)
    {
      // Chose the correct faces - the ones that contains edges with proper
      // bi-normal direction
      TopTools_IndexedDataMapOfShapeListOfShape anEFMap;
      TopTools_ListIteratorOfListOfShape itLF(aLFTrimmed);
      for (; itLF.More(); itLF.Next())
        TopExp::MapShapesAndAncestors(itLF.Value(), TopAbs_EDGE, TopAbs_FACE, anEFMap);

      // Check edges orientations
      TopTools_MapOfShape aFacesToAvoid, aValidFaces;
      FindExtraShapes(anEFMap, aMEdgesToCheckOri, aGFTrim, aFacesToAvoid, &aValidFaces);

      if (aLFTrimmed.Extent() - aFacesToAvoid.Extent() > 1)
      {
        // Keep faces that are in contact with feature edges
        // (excluding overlaps on faces adjacent to the feature).
        {
          Bnd_Box bndBoxOrig;
          BRepBndLib::AddOptimal(theFOriginal, bndBoxOrig, false, false);

          if (!featureFaces.Contains(theFOriginal))
          {
            TopTools_MapOfShape::Iterator itVF(aValidFaces);
            for (; itVF.More(); itVF.Next())
            {
              const TopoDS_Shape& checkedValidFace = itVF.Value();
              TopoDS_Edge commonEdge;
              bool isSkipped = true;
              for (TopExp_Explorer exp(checkedValidFace, TopAbs_EDGE); exp.More(); exp.Next())
              {
                if (theFeatureEdgesMapNew.Contains(exp.Value()))
                {
                  commonEdge = TopoDS::Edge(exp.Value());
                  isSkipped = false;
                }
              }

              if (isSkipped)
              {
                continue;
              }

              Bnd_Box bndBox;
              BRepBndLib::AddOptimal(checkedValidFace, bndBox, false, false);

              if (!bndBoxOrig.IsOut(bndBox))
              {

                ShapeAnalysis_ShapeTolerance tolerCheckerOrig;
                double maxToler = tolerCheckerOrig.Tolerance(theFOriginal, 1);

                ShapeAnalysis_ShapeTolerance tolerChecker;
                maxToler = std::max(tolerChecker.Tolerance(checkedValidFace, 1), maxToler);
                maxToler = std::max(1.0e-4, maxToler);


                BRepAdaptor_Surface adaptSurf(TopoDS::Face(checkedValidFace));

                double f, l;
                Handle(Geom2d_Curve) c2d =
                  BRep_Tool::CurveOnSurface(commonEdge, TopoDS::Face(checkedValidFace), f, l);
                Geom2dAdaptor_Curve adap2d(c2d);
                gp_Pnt2d uvBasis;
                gp_Vec2d uvTangent;
                adap2d.D1(f + 0.25 * (l - f), uvBasis, uvTangent);
                if (uvTangent.SquareMagnitude() > Precision::SquarePConfusion())
                  uvTangent.Normalize();
                else
                  continue;

                gp_Vec2d uvNormal(-uvTangent.Y(), uvTangent.X());

                gp_Pnt2d firstCheckedPnt = uvBasis.XY() + (1 + maxToler) * uvNormal.XY();
                gp_Pnt2d secondCheckedPnt = uvBasis.XY() - (1 + maxToler) * uvNormal.XY();

                ShapeAnalysis_Surface analisysSurf(adaptSurf.Surface().Surface());
                analisysSurf.SetDomain(adaptSurf.FirstUParameter(), adaptSurf.LastUParameter(),
                  adaptSurf.FirstVParameter(), adaptSurf.LastVParameter());

                BRepClass_FaceExplorer faceExp(TopoDS::Face(checkedValidFace));

                BRepClass_FClassifier fClassifier1;
                fClassifier1.Perform(faceExp, firstCheckedPnt, 1.0e-4);

                BRepClass_FClassifier fClassifier2;
                fClassifier2.Perform(faceExp, secondCheckedPnt, 1.0e-4);

                gp_Pnt checkedShapePnt;
                if (fClassifier1.State() == TopAbs_IN)
                {
                  checkedShapePnt = analisysSurf.Value(firstCheckedPnt);
                }
                else if (fClassifier2.State() == TopAbs_IN)
                {
                  checkedShapePnt = analisysSurf.Value(secondCheckedPnt);
                }
                else
                {
                  continue;
                }

                BRepAdaptor_Surface adaptSurfOrig(TopoDS::Face(theFOriginal));
                ShapeAnalysis_Surface analisysSurfOrig(adaptSurfOrig.Surface().Surface());
                analisysSurfOrig.SetDomain(adaptSurfOrig.FirstUParameter(), adaptSurfOrig.LastUParameter(),
                  adaptSurfOrig.FirstVParameter(), adaptSurfOrig.LastVParameter());
                gp_Pnt2d pnt2dOrig = analisysSurfOrig.ValueOfUV(checkedShapePnt, Precision::Confusion());
                gp_Pnt checkedOrigPnt = adaptSurfOrig.Value(pnt2dOrig.X(), pnt2dOrig.Y());
                BRepClass_FaceExplorer faceExpOrig(TopoDS::Face(theFOriginal));
                BRepClass_FClassifier fClassifierOrig;
                fClassifierOrig.Perform(faceExpOrig, pnt2dOrig, 1.0e-4);
                if (fClassifierOrig.State() == TopAbs_IN)
                {
                  if (checkedOrigPnt.Distance(checkedShapePnt) < Precision::Confusion())
                  {
                    aFacesToAvoid.Add(checkedValidFace);
                    continue;
                  }
                }
              }
            }
          }

        }

        // It is possible that the splits are forming the different blocks.
        // Take only those containing the valid faces.
        TopoDS_Compound aCF;
        BRep_Builder().MakeCompound(aCF);
        itLF.Initialize(aLFTrimmed);
        for (; itLF.More(); itLF.Next())
        {
          if (!aFacesToAvoid.Contains(itLF.Value()))
            BRep_Builder().Add(aCF, itLF.Value());
        }

        TopTools_ListOfShape aLCB;
        BOPTools_AlgoTools::MakeConnexityBlocks(aCF, TopAbs_EDGE, TopAbs_FACE, aLCB);
        if (aLCB.Extent() > 1)
        {
          TopTools_ListIteratorOfListOfShape itLCB(aLCB);
          for (; itLCB.More(); itLCB.Next())
          {
            // Check if the block contains any valid faces
            const TopoDS_Shape& aCB = itLCB.Value();
            TopoDS_Iterator itF(aCB);
            for (; itF.More(); itF.Next())
            {
              if (aValidFaces.Contains(itF.Value()))
                break;
            }
            if (!itF.More())
            {
              // Invalid block
              for (itF.Initialize(aCB); itF.More(); itF.Next())
                aFacesToAvoid.Add(itF.Value());
            }
          }
        }
      }

      itLF.Initialize(aLFTrimmed);
      for (; itLF.More(); itLF.Next())
      {
        if (!aFacesToAvoid.Contains(itLF.Value()) &&
            !aValidFaces.Contains(itLF.Value()) &&
            !notDefinedFacesLoc.Contains(itLF.Value()))
        {
          notDefinedFacesLoc.Append(itLF.Value());
        }
      }

      itLF.Initialize(aLFTrimmed);
      for (; itLF.More();)
      {
        if (aFacesToAvoid.Contains(itLF.Value()))
          aLFTrimmed.Remove(itLF);
        else
          itLF.Next();
      }

    }
    else if (aLFTrimmed.IsEmpty())
    {
      // Use all splits, including those having the bounds of extended face
      anExpF.ReInit();
      for (; anExpF.More(); anExpF.Next())
        aLFTrimmed.Append(anExpF.Current());
    }


    if (aLFTrimmed.Extent())
    {
      // Remove the internal edges and vertices from the faces
      RemoveInternalWires(aLFTrimmed);

      myFaces.Add(theFOriginal, aLFTrimmed);
    }

    if (featureFaces.Contains(theFOriginal))
    {
      TopTools_ListIteratorOfListOfShape itLF1(aLFTrimmed);
      for (; itLF1.More(); itLF1.Next())
      {
        myFeaturesFaces.Add(itLF1.Value());
      }
    }

    if (notDefinedFacesLoc.Extent())
    {
      notDefinedFaces.Add(theFOriginal, notDefinedFacesLoc);
    }

    // Update history with all removed shapes
    BRepTools_History aHistRem;

    // Map of the result splits
    TopTools_IndexedMapOfShape aResMap;
    TopTools_ListIteratorOfListOfShape itLF(aLFTrimmed);
    for (; itLF.More(); itLF.Next())
    {
      TopExp::MapShapes(itLF.Value(), aResMap);

      // For adjacent faces that form a single surface, only one
      // extension needs to be kept (otherwise, we have an overlay).
      gp_Pnt checkedPnt;
      bool isFound = false;
      BRepAdaptor_Surface   checkedfaceAdapt(TopoDS::Face(itLF.Value()));
      math_BullardGenerator RNG;
      int numSamples = 10;
      for (int i = 0; i < numSamples; ++i)
      {
        // Get a random sample point.
        gp_Pnt2d uv;
        if (!asiAlgo_Utils::GetRandomPoint(checkedfaceAdapt.Face(), RNG, uv))
        {
          continue;
        }

        checkedfaceAdapt.D0(uv.X(), uv.Y(), checkedPnt);
        isFound = true;
        break;
      }

      if (!isFound)
      {
        continue;
      }

      isFound = false;

      TopTools_ListOfShape::Iterator itUF(myUniqueFaces);
      for (; itUF.More(); itUF.Next())
      {
        BRepAdaptor_Surface adaptSurf(TopoDS::Face(itUF.Value()));
        ShapeAnalysis_Surface analisysSurf(adaptSurf.Surface().Surface());
        analisysSurf.SetDomain(adaptSurf.FirstUParameter(), adaptSurf.LastUParameter(),
                               adaptSurf.FirstVParameter(), adaptSurf.LastVParameter());
        gp_Pnt2d pnt2d = analisysSurf.ValueOfUV(checkedPnt, Precision::Confusion());
        gp_Pnt checkedUFPnt = adaptSurf.Value(pnt2d.X(), pnt2d.Y());
        BRepClass_FaceExplorer faceExp(TopoDS::Face(itUF.Value()));
        BRepClass_FClassifier fClassifier;
        fClassifier.Perform(faceExp, pnt2d, 1.0e-4);
        if (fClassifier.State() == TopAbs_IN && checkedPnt.Distance(checkedUFPnt) < Precision::Confusion())
        {
          isFound = true;
          break;
        }
      }

      if (!isFound)
      {
        myUniqueFaces.Append(itLF.Value());
      }
    }
  }

private:

  // Inputs
  TopoDS_Shape        myFeature;    //!< Feature to remove.
  Handle(asiAlgo_AAG) myAAG;        //!< AAG.
  asiAlgo_Feature     myFaceIDs;    //!< FaceIds.
  bool                myIsOneSolid; //!< Indicator showing whether there
                                    //!  should be a "one solid - one feature"
                                    //!  configuration.

  // Results
  Standard_Boolean                          myHasAdjacentFaces; //!< Flag to show whether the adjacent faces have been found or not.
  TopTools_ListOfShape                      myUniqueFaces;      //!< Extensions of adjacent faces.
  TopTools_MapOfShape                       myFeaturesFaces;    //!< Feature faces after extension.
  TopTools_IndexedDataMapOfShapeListOfShape myFaces;            //!< Reconstructed adjacent faces.
  TopoDS_Shape                              myResultShape;      //!< Result shape.
};

}

//-----------------------------------------------------------------------------

bool ComputeNegativeVolumeAlgo::Perform()
{
  if ( m_aag.IsNull() || m_faceIDs.IsEmpty() )
  {
    return false;
  }

  if ( !m_negativeVolumes.empty() )
  {
    m_negativeVolumes.clear();
  }

  return perform();
}

//-----------------------------------------------------------------------------

bool ComputeNegativeVolumeAlgo::perform()
{
  std::vector<asiAlgo_Feature> clusters;
  m_aag->GetConnectedComponents(m_faceIDs, clusters);

  std::vector<asiAlgo_Feature>::const_iterator itClusters = clusters.cbegin();
  for ( ; itClusters != clusters.cend(); ++itClusters )
  {
    TopTools_ListOfShape listOfShape;

    FillGap aFG(m_aag, *itClusters, m_isOneSolid);
    aFG.Perform();

    TopoDS_Shape negativeVolumeShape = aFG.Shape();

    TopTools_IndexedDataMapOfShapeListOfShape edgesFacesMap;
    TopExp::MapShapesAndAncestors(negativeVolumeShape, TopAbs_EDGE, TopAbs_FACE, edgesFacesMap);
    bool isNotValid = false;
    bool recheckValidity = false;
    for ( int index = 1; index <= edgesFacesMap.Extent(); ++index )
    {
      const TopoDS_Edge& edge = TopoDS::Edge(edgesFacesMap.FindKey(index));
      const TopTools_ListOfShape& faces = edgesFacesMap(index);
      if ( !BRep_Tool::Degenerated(edge) && faces.Extent() < 2 )
      {
        isNotValid = true;
        break;
      }
      else if (BRep_Tool::Degenerated(edge))
      {
        recheckValidity = true;
      }
    }

    if ( recheckValidity )
    {
      try
      {
        BRepCheck_Analyzer analyzer(negativeVolumeShape, true);
        if (!analyzer.IsValid())
        {
          isNotValid = true;
        }
        else
        {
          isNotValid = false;
        }
      }
      catch (...)
      {
        isNotValid = true;
      }
    }

    if (isNotValid)
    {
      continue;
    }

    GProp_GProps props;
    BRepGProp::VolumeProperties(negativeVolumeShape, props);

    std::tuple<asiAlgo_Feature,
               TopoDS_Shape,
               double> negativeVolume(*itClusters, negativeVolumeShape, props.Mass());

    m_negativeVolumes.push_back(negativeVolume);
  }

  return !m_negativeVolumes.empty();
}
