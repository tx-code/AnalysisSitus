/***************************************************************************
 *   Copyright (c) 2011 Thomas Anderson <blobfish[at]gmx.com>              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#ifndef MODELREFINE_H
#define MODELREFINE_H

#include <vector>
#include <map>
#include <list>
#include <GeomAbs_SurfaceType.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>

 //! \ingroup ASIEXT_SheetMetal
 //!
 //! ModelRefine namespace.
namespace ModelRefine
{
//! \ingroup ASIEXT_SheetMetal
//!
//! Vector of faces.
typedef std::vector<TopoDS_Face>  FaceVectorType;

//! \ingroup ASIEXT_SheetMetal
//!
//! Vector of edges.
typedef std::vector<TopoDS_Edge>  EdgeVectorType;

//! \ingroup ASIEXT_SheetMetal
//!
//! Vector of shapes.
typedef std::vector<TopoDS_Shape> ShapeVectorType;

//! \ingroup ASIEXT_SheetMetal
//!
//! Pair <shape, shape>.
typedef std::pair<TopoDS_Shape, TopoDS_Shape> ShapePairType;

//! Gets edges from face.
//! \param [in]  face  face.
//! \param [out] edges edges.
void getFaceEdges(const TopoDS_Face &face, EdgeVectorType &edges);

//! Finds all the boundary edges.
//! \param [in]  faces    faces.
//! \param [out] edgesOut edges.
void boundaryEdges(const FaceVectorType &faces, EdgeVectorType &edgesOut);

//! Remove faces from shell.
//! \param [in] shell shell.
//! \param [in] faces faces to be removed.
//! \return rebuilt shell.
TopoDS_Shell removeFaces(const TopoDS_Shell &shell, const FaceVectorType &faces);

//! \ingroup ASIEXT_SheetMetal
//!
//! Base type of face.
class FaceTypedBase
{
private:

  //! Constructor.
  FaceTypedBase(){}

protected:

  //! Constructor.
  //! \param [in] typeIn surface type.
  FaceTypedBase(const GeomAbs_SurfaceType &typeIn){surfaceType = typeIn;}

public:

  //! Geometric comparison of faces.
  //! \param [in] faceOne first face.
  //! \param [in] faceTwo second face.
  //! \return true in the case of equality, false -- otherwise.
  virtual bool isEqual(const TopoDS_Face &faceOne, const TopoDS_Face &faceTwo) const = 0;

  //! Gets type of surface.
  virtual GeomAbs_SurfaceType getType() const = 0;

  //! Builds one face from several faces.
  //! \param [in] faces faces.
  //! \return built face.
  virtual TopoDS_Face buildFace(const FaceVectorType &faces) const = 0;

  //! Gets face type.
  //! \param [in] faceIn face.
  //! \return face type.
  static GeomAbs_SurfaceType getFaceType(const TopoDS_Face &faceIn);

protected:

  //! Finds boundary edges.
  //! \param [in]  facesIn       faces.
  //! \param [out] boundariesOut edges.
  virtual void boundarySplit(const FaceVectorType &facesIn, std::vector<EdgeVectorType> &boundariesOut) const;

  GeomAbs_SurfaceType surfaceType; //!< Type of surface.
};

//! \ingroup ASIEXT_SheetMetal
//!
//! Planar face.
class FaceTypedPlane : public FaceTypedBase
{
private:

  //! Constructor.
  FaceTypedPlane();

public:

  //! Geometric comparison of faces.
  //! \param [in] faceOne first face.
  //! \param [in] faceTwo second face.
  //! \return true in the case of equality, false -- otherwise.
  virtual bool isEqual(const TopoDS_Face &faceOne, const TopoDS_Face &faceTwo) const;

  //! Gets type of surface.
  virtual GeomAbs_SurfaceType getType() const;

  //! Builds one face from several faces.
  //! \param [in] faces faces.
  //! \return built face.
  virtual TopoDS_Face buildFace(const FaceVectorType &faces) const;

  //! Gets planar object.
  //! \return planar object.
  friend FaceTypedPlane& getPlaneObject();
};

//! Gets planar object.
//! \return planar object.
FaceTypedPlane& getPlaneObject();

//! \ingroup ASIEXT_SheetMetal
//!
//! Cylindrical face.
class FaceTypedCylinder : public FaceTypedBase
{
private:

  //! Constructor.
  FaceTypedCylinder();

public:

  //! Geometric comparison of faces.
  //! \param [in] faceOne first face.
  //! \param [in] faceTwo second face.
  //! \return true in the case of equality, false -- otherwise.
  virtual bool isEqual(const TopoDS_Face &faceOne, const TopoDS_Face &faceTwo) const;

  //! Gets type of surface.
  virtual GeomAbs_SurfaceType getType() const;

  //! Builds one face from several faces.
  //! \param [in] faces faces.
  //! \return built face.
  virtual TopoDS_Face buildFace(const FaceVectorType &faces) const;

  //! Gets cylindrical object.
  //! \return cylindrical object.
  friend FaceTypedCylinder& getCylinderObject();

protected:

  //! Finds boundary edges.
  //! \param [in]  facesIn       faces.
  //! \param [out] boundariesOut edges.
  virtual void boundarySplit(const FaceVectorType &facesIn, std::vector<EdgeVectorType> &boundariesOut) const;
};

//! Gets cylindrical object.
//! \return cylindrical object.
FaceTypedCylinder& getCylinderObject();

//! \ingroup ASIEXT_SheetMetal
//!
//! BSpline face.
class FaceTypedBSpline : public FaceTypedBase
{
private:

  //! Constructor.
  FaceTypedBSpline();

public:

  //! Geometric comparison of faces.
  //! \param [in] faceOne first face.
  //! \param [in] faceTwo second face.
  //! \return true in the case of equality, false -- otherwise.
  virtual bool isEqual(const TopoDS_Face &faceOne, const TopoDS_Face &faceTwo) const;

  //! Gets type of surface.
  virtual GeomAbs_SurfaceType getType() const;

  //! Builds one face from several faces.
  //! \param [in] faces faces.
  //! \return built face.
  virtual TopoDS_Face buildFace(const FaceVectorType &faces) const;

  //! Gets BSpline object.
  //! \return BSpline object.
  friend FaceTypedBSpline& getBSplineObject();
};

//! Gets BSpline object.
//! \return BSpline object.
FaceTypedBSpline& getBSplineObject();

//! \ingroup ASIEXT_SheetMetal
//!
//! Splitter face.
class FaceTypeSplitter
{
  //! Map <surface type, faces>.
  typedef std::map<GeomAbs_SurfaceType, FaceVectorType> SplitMapType;

public:

  //! Constructor.
  FaceTypeSplitter(){}

  //! Adds shell.
  //! \param [in] shellIn shell.
  void addShell(const TopoDS_Shell &shellIn);

  //! Registers type of surface.
  //! \param [in] type type of surface.
  void registerType(const GeomAbs_SurfaceType &type);

  // Checks if the specified type is among the registered types.
  //! \param [in] type type of surface.
  //! \return true in the case face is found, false -- otherwise.
  bool hasType(const GeomAbs_SurfaceType &type) const;

  //! Splits shell into faces and builds typeMap.
  void split();

  // Gets faces of specified type.
  //! \param [in] type type of surface.
  //! \return faces of specified type.
  const FaceVectorType& getTypedFaceVector(const GeomAbs_SurfaceType &type) const;

private:

  SplitMapType typeMap; //!< Map <surface type, faces>.
  TopoDS_Shell shell;   //!< Shell.
};

//! \ingroup ASIEXT_SheetMetal
//!
//! Adjacency splitter face.
class FaceAdjacencySplitter
{
public:

  //! Constructor.
  //! \param [in] shell shell.
  FaceAdjacencySplitter(const TopoDS_Shell &shell);

  //! Builds adjacency array from faces.
  //! \param [in] facesIn faces.
  void split(const FaceVectorType &facesIn);

  // Gets size of adjacency array.
  //! \return group count.
  std::size_t getGroupCount() const {return adjacencyArray.size();}

  //! Gets group.
  //! \param [in] index index.
  //! return found faces.
  const FaceVectorType& getGroup(const std::size_t &index) const {return adjacencyArray[index];}

private:

  //! Constructor.
  FaceAdjacencySplitter(){}

  //! Recursive search for face neighbors.
  //! \param [in]  face      face.
  //! \param [out] outVector neighbors.
  void recursiveFind(const TopoDS_Face &face, FaceVectorType &outVector);

private:

  std::vector<FaceVectorType>               adjacencyArray; //!< Adjacency array.
  TopTools_MapOfShape                       processedMap;   //!< Processed map.
  TopTools_MapOfShape                       facesInMap;     //!< Faces in map.
  TopTools_IndexedDataMapOfShapeListOfShape faceToEdgeMap;  //!< Face-edge map.
  TopTools_IndexedDataMapOfShapeListOfShape edgeToFaceMap;  //!< Edge-face map.
};

//! \ingroup ASIEXT_SheetMetal
//!
//! Equality splitter face.
class FaceEqualitySplitter
{
public:

  //! Constructor.
  FaceEqualitySplitter(){}

  //! Builds equality groups.
  //! \param [in] faces  faces.
  //! \param [in] object comparison object.
  void split(const FaceVectorType &faces,  FaceTypedBase *object);

  // Gets count of equality groups.
  //! \return group count.
  std::size_t getGroupCount() const {return equalityVector.size();}

  // Gets equality group.
  //! \param [in] index index.
  //! \return group.
  const FaceVectorType& getGroup(const std::size_t &index) const {return equalityVector[index];}

private:

  std::vector<FaceVectorType> equalityVector; //!< Equality groups.
};

//! \ingroup ASIEXT_SheetMetal
//!
//! Uniter of faces.
class FaceUniter
{
private:

  //! Constructor.
  FaceUniter(){}

public:

  //! Constructor.
  //! \param [in] shellIn shell.
  FaceUniter(const TopoDS_Shell &shellIn);

  //! Process.
  //! \return true in case of success, false -- otherwise.
  bool process();

  //! Gets shell.
  //! \return shell.
  const TopoDS_Shell& getShell() const {return workShell;}

  //! Method to check if the shell has changed.
  //! \return true if shell has been changed.
  bool isModified(){return modifiedSignal;}

  //! Gets modified shapes.
  //! \return modified shapes.
  const std::vector<ShapePairType>& getModifiedShapes() const
  {return modifiedShapes;}

  //! Gets deleted shapes.
  //! \return deleted shapes.
  const ShapeVectorType& getDeletedShapes() const
  {return deletedShapes;}

private:

  TopoDS_Shell                workShell;      //!< Work shell.
  std::vector<FaceTypedBase*> typeObjects;    //!< Type of objects.
  std::vector<ShapePairType>  modifiedShapes; //!< Modified shapes.
  ShapeVectorType             deletedShapes;  //!< Deleted shapes.
  bool                        modifiedSignal; //!< Modified signal.
};
}

/* excerpt from GeomAbs_SurfaceType.hxx
enum GeomAbs_SurfaceType {
GeomAbs_Plane,
GeomAbs_Cylinder,
GeomAbs_Cone,
GeomAbs_Sphere,
GeomAbs_Torus,
GeomAbs_BezierSurface,
GeomAbs_BSplineSurface,
GeomAbs_SurfaceOfRevolution,
GeomAbs_SurfaceOfExtrusion,
GeomAbs_OffsetSurface,
GeomAbs_OtherSurface
};
*/

//! \ingroup ASIEXT_SheetMetal
//!
//! Part namespace.
namespace Part {

//! \ingroup ASIEXT_SheetMetal
//!
//! Refinement of model.
class BRepBuilderAPI_RefineModel : public BRepBuilderAPI_MakeShape
{
public:

  //! Constructor.
  //! \param [in] shape shape.
  Standard_EXPORT
    BRepBuilderAPI_RefineModel(const TopoDS_Shape& shape);

  //! Refine model.
  Standard_EXPORT
    void Build();

  //! Gets list of modified shapes.
  //! \param [in] S shape before modification.
  //! \return list of modified shapes.
  Standard_EXPORT
    const TopTools_ListOfShape& Modified(const TopoDS_Shape& S);

  //! Method that checks if the shape was removed during processing.
  //! \param [in] S shape.
  //! \return true if shape has been removed.
  Standard_EXPORT
    Standard_Boolean IsDeleted(const TopoDS_Shape& S);

private:

  //! Logs modifications.
  //! \param [in] uniter uniter.
  void LogModifications(const ModelRefine::FaceUniter& uniter);

private:

  TopTools_DataMapOfShapeListOfShape myModified;  //!< List of modified shapes.
  TopTools_ListOfShape               myEmptyList; //!< Empty list.
  TopTools_ListOfShape               myDeleted;   //!< List of removed shapes.
};
}

#endif // MODELREFINE_H
