//-----------------------------------------------------------------------------
// Created on: 07 September 2022
// Author: Andrey Voevodin
//-----------------------------------------------------------------------------

// Own include
#include "asiAlgo_ComputeNegativeVolume.h"

// asiAlgo includes
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
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepClass_FaceExplorer.hxx>
#include <BRepClass_FClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRepLib.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GProp_GProps.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>

#include <STEPControl_Writer.hxx>
#include <BRepTools.hxx>

//-----------------------------------------------------------------------------

namespace {

//=======================================================================
// function: MakeSolid
// purpose: Make solid.
//=======================================================================
bool MakeSolid(const TopTools_ListOfShape&          listOfShape,
               TopoDS_Shape&                        result,
               TopTools_IndexedDataMapOfShapeShape& modifiedShapesInv)
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
      }
      else if (tool.IsModifiedSubShape(itFaces.Value()))
      {
        modifiedShapesInv.Add(tool.ModifiedSubShape(itFaces.Value()), itFaces.Value());
      }
      else
      {
        modifiedShapesInv.Add(itFaces.Value(), itFaces.Value());
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
// function: TakeModified
// purpose: Stores the modified object into the map
//=======================================================================
void TakeModified(const TopoDS_Shape&  shape,
                  BOPAlgo_Builder&     builder,
                  TopTools_MapOfShape& mapOfShape)
{
  const TopTools_ListOfShape& modified = builder.Modified(shape);
  if ( modified.IsEmpty() && !builder.IsDeleted(shape) )
  {
    mapOfShape.Add(shape);
  }
  else
  {
    TopTools_ListIteratorOfListOfShape itM(modified);
    for ( ; itM.More(); itM.Next() )
    {
      mapOfShape.Add(itM.Value());
    }
  }
}

//=======================================================================
// function: MakeRemoved
// purpose: Makes the shapes in the list removed in the history.
//          Keeps the shapes contained in the map.
//=======================================================================
void MakeRemoved(const TopTools_ListOfShape&       shapes,
                 BRepTools_History&                history,
                 const TopTools_IndexedMapOfShape& keepShapes)
{
  TopTools_IndexedMapOfShape shapesMap;
  TopTools_ListIteratorOfListOfShape it(shapes);
  for ( ; it.More(); it.Next() )
  {
    TopExp::MapShapes(it.Value(), shapesMap);
  }

  const Standard_Integer nbShapes = shapesMap.Extent();
  for ( Standard_Integer i = 1; i <= nbShapes; ++i )
  {
    const TopoDS_Shape& shape = shapesMap(i);
    if ( !keepShapes.Contains(shape) &&
         BRepTools_History::IsSupportedType(shape) )
    {
      history.Remove(shape);
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
          const asiAlgo_Feature      faceIDs) :
    myAAG(aag),
    myFaceIDs(faceIDs)
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

public:

  //! Sets the feature to remove
  void SetFeature(const TopoDS_Shape& theFeature) { myFeature = theFeature; }

  //! Returns the feature
  const TopoDS_Shape& Feature() const { return myFeature; }

public:

  //! Performs the extension of the adjacent faces and
  //! then trims the extended faces to fill the gaps
  void Perform()
  {
    OCC_CATCH_SIGNALS

    try
    {
      // Find the faces adjacent to the faces of the feature
      TopTools_IndexedMapOfShape aMFAdjacent;
      FindAdjacentAndFeatureFaces(aMFAdjacent);

      if (!(aMFAdjacent.Extent() > 0))
        return;

      // Extend the adjacent faces keeping the connection to the original faces
      TopTools_IndexedDataMapOfShapeShape aFaceExtFaceMap;
      ExtendAdjacentAndFeatureFaces(aMFAdjacent, aFaceExtFaceMap);

      // Trim the extended faces
      TopoDS_Shape intShape;
      IntersectAdjacentFacesWithFeature(aFaceExtFaceMap, intShape);

      FilterShape(intShape, myNegativeVolume);
    }
    catch (Standard_Failure const&)
    {}
  }

public:

  const TopoDS_Shape& NegativeVolume() const { return myNegativeVolume; }

private: //! @name Private methods performing the operation

  //! Finds the faces adjacent to the feature and stores them into outgoing map.
  void FindAdjacentAndFeatureFaces(TopTools_IndexedMapOfShape& theMFAdjacent)
  {
    asiAlgo_Feature neighborsOfCluster;
    asiAlgo_Feature::Iterator it(myFaceIDs);
    for (; it.More(); it.Next())
    {
      asiAlgo_Feature neighbors = myAAG->GetNeighbors(it.Key());
      neighborsOfCluster.Unite(neighbors);
      theMFAdjacent.Add(myAAG->GetFace(it.Key()).Reversed());
    }

    neighborsOfCluster.Subtract(myFaceIDs);
    it.Initialize(neighborsOfCluster);
    for (; it.More(); it.Next())
    {
      theMFAdjacent.Add(myAAG->GetFace(it.Key()));
    }
  }

  //! Extends the found adjacent faces and binds them to the original faces.
  void ExtendAdjacentAndFeatureFaces(const TopTools_IndexedMapOfShape&    theMFAdjacent,
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

  //! 
  void IntersectAdjacentFacesWithFeature(const TopTools_IndexedDataMapOfShapeShape& theFaceExtFaceMap,
                                         TopoDS_Shape&                              resultShape)
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

    resultShape = anIntResult;
  }

  void FilterShape(const TopoDS_Shape& filteredShape,
                   TopoDS_Shape&       resultShape)
  {
    TopoDS_Shape negativeVolumeShape = filteredShape;

    // Remove faces into initial shape.
    TopTools_ListOfShape keeppedFaces;
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
          continue;
        }

        BRepClass3d_SolidClassifier solidClassifier(myAAG->GetMasterShape());
        solidClassifier.Perform(checkedPnt, 1.0e-4);
        TopAbs_State state = solidClassifier.State();
        if (state == TopAbs_IN)
          continue;

        keeppedFaces.Append(exp.Value());
      }
    }

    TopTools_IndexedDataMapOfShapeShape modifiedShapesInv;
    if (!MakeSolid(keeppedFaces, negativeVolumeShape, modifiedShapesInv))
    {
      return;
    }

    // Remove faces with open edges.
    int maxNbStep = 100;
    int nbTopoRemove = 0;
    int nbTopoStep = 0;
    do
    {
      ++nbTopoStep;
      nbTopoRemove = 0;
      keeppedFaces.Clear();
      modifiedShapesInv.Clear();

      TopTools_MapOfShape removedFaces;
      TopTools_IndexedDataMapOfShapeListOfShape edgesFacesMap;
      TopExp::MapShapesAndAncestors(negativeVolumeShape, TopAbs_EDGE, TopAbs_FACE, edgesFacesMap);
      for (int index = 1; index <= edgesFacesMap.Extent(); ++index)
      {
        const TopTools_ListOfShape& faces = edgesFacesMap(index);
        if (faces.Extent() == 1)
        {
          removedFaces.Add(faces.First());
          ++nbTopoRemove;
        }
      }

      {
        TopExp_Explorer exp(negativeVolumeShape, TopAbs_FACE);
        for (; exp.More(); exp.Next())
        {
          if (!removedFaces.Contains(exp.Value()))
          {
            keeppedFaces.Append(exp.Value());
          }
        }

        if (!MakeSolid(keeppedFaces, negativeVolumeShape, modifiedShapesInv))
        {
          return;
        }
      }
    } while (nbTopoRemove != 0 && nbTopoStep < maxNbStep);



    int nbRemoveFaces = 0;
    int nbStep = 0;
    do
    {
      keeppedFaces.Clear();
      modifiedShapesInv.Clear();
      nbRemoveFaces = 0;
      ++nbStep;

      TopExp_Explorer exp(negativeVolumeShape, TopAbs_FACE);
      for (; exp.More(); exp.Next())
      {
        gp_Pnt checkedPnt;
        gp_Vec D1U, D1V;
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

          checkedfaceAdapt.D1(uv.X(), uv.Y(), checkedPnt, D1U, D1V);
          isFound = true;
          break;
        }

        if (!isFound)
        {
          ++nbRemoveFaces;
          continue;
        }

        gp_Vec normal = (D1U ^ D1V).Normalized();
        //
        if (TopoDS::Face(exp.Value()).Orientation() == TopAbs_REVERSED)
          normal *= -1.0;

        checkedPnt = checkedPnt.XYZ() + 2.0 * normal.XYZ();

        //gp_Lin sampleRay(checkedPnt, -normal);
        //
        //IntCurvesFace_ShapeIntersector intersector;
        //intersector.Load(negativeVolumeShape, 1.0e-4);
        //intersector.Perform(sampleRay, 0.0, 1e100);
        //
        //if (!intersector.IsDone() || intersector.NbPnt() % 2 == 1 || intersector.NbPnt() == 0)
        //{
        //  ++nbRemoveFaces;
        //  continue;
        //}

        BRepClass3d_SolidClassifier solidClassifier(negativeVolumeShape);
        solidClassifier.Perform(checkedPnt, 1.0e-4);
        TopAbs_State state = solidClassifier.State();
        if (state == TopAbs_IN)
        {
          ++nbRemoveFaces;
          continue;
        }

        keeppedFaces.Append(exp.Value());
      }

      if (!MakeSolid(keeppedFaces, negativeVolumeShape, modifiedShapesInv))
      {
        return;
      }

    } while (nbRemoveFaces != 0 && nbStep < maxNbStep);

    resultShape = negativeVolumeShape;

  }

private: //! @name Fields

  // Inputs
  TopoDS_Shape        myFeature;        //!< Feature to remove.
  Handle(asiAlgo_AAG) myAAG;            //!< AAG.
  asiAlgo_Feature     myFaceIDs;        //!< FaceIds.

  // Results
  TopoDS_Shape        myNegativeVolume; //!< Negative volume.
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

    FillGap aFG(m_aag, *itClusters);
    aFG.Perform();
    TopoDS_Shape negativeVolumeShape = aFG.NegativeVolume();

    TopTools_IndexedDataMapOfShapeListOfShape edgesFacesMap;
    TopExp::MapShapesAndAncestors(negativeVolumeShape, TopAbs_EDGE, TopAbs_FACE, edgesFacesMap);
    bool isNotValid = false;
    for ( int index = 1; index <= edgesFacesMap.Extent(); ++index )
    {
      const TopTools_ListOfShape& faces = edgesFacesMap(index);
      if ( faces.Extent() != 2 )
      {
        isNotValid = true;
        break;
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
