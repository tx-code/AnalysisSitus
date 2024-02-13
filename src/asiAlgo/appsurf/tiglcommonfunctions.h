/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-06-01 Martin Siggel <Martin.Siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef TIGLCOMMONFUNCTIONS_H
#define TIGLCOMMONFUNCTIONS_H

#include "Standard.hxx"
#include "Standard_values.h"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "gp_Pln.hxx"
#include "TopoDS_Shape.hxx"
#include <TopoDS_Edge.hxx>
#include "CTiglIntersectionPoint.h"
#include <TopoDS_Edge.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopTools_ListOfShape.hxx>
#include "TColgp_HArray1OfPnt.hxx"
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <Bnd_Box.hxx>
#include <CTiglPoint.h>

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

//! \ingroup ASI_MODELING
//!
enum TiglContinuity
{
    C0 = 0,
    C1 = 1,
    C2 = 2
};

//! \ingroup ASI_MODELING
//!
// helper function for std::find
struct IsInsideTolerance
{
    IsInsideTolerance(double value, double tolerance = 1e-15)
        : _a(value), _tol(tolerance)
    {}

    bool operator()(double v)
    {
        return (fabs(_a - v) <= _tol);
    }

    double _a;
    double _tol;
};

// calculates a wire's circumfence
asiAlgo_EXPORT Standard_Real GetLength(const TopoDS_Wire& wire);

asiAlgo_EXPORT Standard_Real GetLength(const TopoDS_Edge& edge);

// returns a point on the wire (0 <= alpha <= 1)
asiAlgo_EXPORT gp_Pnt WireGetPoint(const TopoDS_Wire& wire, double alpha);
asiAlgo_EXPORT void WireGetPointTangent(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal);

// returns the starting point of the wire/edge
asiAlgo_EXPORT gp_Pnt GetFirstPoint(const TopoDS_Shape& wireOrEdge);
asiAlgo_EXPORT gp_Pnt GetFirstPoint(const TopoDS_Wire& w);
asiAlgo_EXPORT gp_Pnt GetFirstPoint(const TopoDS_Edge& e);

// returns the end point of the wire/edge
asiAlgo_EXPORT gp_Pnt GetLastPoint(const TopoDS_Shape& wireOrEdge);
asiAlgo_EXPORT gp_Pnt GetLastPoint(const TopoDS_Wire& w);
asiAlgo_EXPORT gp_Pnt GetLastPoint(const TopoDS_Edge& e);

asiAlgo_EXPORT gp_Pnt EdgeGetPoint(const TopoDS_Edge& edge, double alpha);
asiAlgo_EXPORT void EdgeGetPointTangent(const TopoDS_Edge& edge, double alpha, gp_Pnt& point, gp_Vec& normal);

// calculates the alpha value for a given point on a wire
//asiAlgo_EXPORT Standard_Real ProjectPointOnWire(const TopoDS_Wire& wire, gp_Pnt p);

// projects a point onto the line (lineStart<->lineStop) and returns the projection parameter
asiAlgo_EXPORT Standard_Real ProjectPointOnLine(gp_Pnt p, gp_Pnt lineStart, gp_Pnt lineStop);

// calculates the alpha value for a given point on a wire
//asiAlgo_EXPORT Standard_Real ProjectPointOnWireAtAngle(const TopoDS_Wire& wire, gp_Pnt p, gp_Dir rotationAxisAroundP, double angle);

// projects a point onto a plane and returns the point in parameters of the plane
asiAlgo_EXPORT gp_Pnt2d ProjectPointOnPlane(gp_Pln pln, gp_Pnt p);

asiAlgo_EXPORT gp_Pnt ProjectPointOnShape(const TopoDS_Shape& shape, const gp_Pnt& point, const gp_Vec& direction);

/**
 * @brief The UVResult struct is used as an output for GetFaceAndUV
 */
struct UVResult
{
    TopoDS_Face face;
    double u, v;
};
//
///**
// * @brief Given a point on a shape, GetFaceAndUV finds all faces
// * which contain the point and determines its (u,v)-coordinates on that face.
// *
// * It is assumes that the point is on the shape. Typically this function would
// * be used on the output of ProjectPointOnShape.
// *
// * @param shape Input shape
// * @param pnt Input point
// * @param tol a tolerance for the squared distance of the point
// * @return a boost::optional<UVResult> instance containing the face together with the
// * (u,v) coordinates of the point on that face, if the algorithm succeeded
// */
//asiAlgo_EXPORT boost::optional<UVResult> GetFaceAndUV(TopoDS_Shape const& shape,
//                                   gp_Pnt const& pnt,
//                                   double tol = 1e-3);


/**
 * @brief TrimFace trims a face given new minimum and maximum values for the (u,v)-
 * coordinates
 * @param face The face to be trimmed
 * @param umin new minimum u value
 * @param umax new maximum u value
 * @param vmin new minimum v value
 * @param vmax new maximum v value
 * @return the trimmed face
 */
asiAlgo_EXPORT TopoDS_Face TrimFace(TopoDS_Face const& face,
                                 double umin,
                                 double umax,
                                 double vmin,
                                 double vmax);


/**
 * @brief ReplaceFaceInShape returns a new shape that corresponds to
 * the input shape, except that old_face is replaced by new_face
 * @param shape The input shape
 * @param new_face the new face that shall take the place of the old shape
 * @param old_face the old face to be replaced
 * @return the shape with the replaced faces
 */
asiAlgo_EXPORT TopoDS_Shape ReplaceFaceInShape(TopoDS_Shape const& shape,
                                            TopoDS_Face const& new_face,
                                            TopoDS_Face const& old_face);

// checks, whether a face is in between two points
asiAlgo_EXPORT bool IsFaceBetweenPoints(const TopoDS_Face& face, gp_Pnt p1, gp_Pnt p2);

enum IntStatus
{
    BetweenPoints, // The intersection point lies between p1 and p2
    OutsideBefore, // The intersection point lies before p1
    OutsideAfter,  // The intersection point lies after p2
    NoIntersection // the plane and the line are parallel to each other
};

// returns the intersection point between a line (p1-p2) and the plane
asiAlgo_EXPORT IntStatus IntersectLinePlane(gp_Pnt p1, gp_Pnt p2, gp_Pln plane, gp_Pnt& result);

// returns the number of edges of the current shape
asiAlgo_EXPORT unsigned int GetNumberOfEdges(const TopoDS_Shape& shape);

// returns the number of faces of the current shape
asiAlgo_EXPORT unsigned int GetNumberOfFaces(const TopoDS_Shape& shape);

asiAlgo_EXPORT TopoDS_Edge GetEdge(const TopoDS_Shape& shape, int iEdge);

asiAlgo_EXPORT TopoDS_Face GetFace(const TopoDS_Shape& shape, int iFace);

asiAlgo_EXPORT Handle(Geom_BSplineCurve) GetBSplineCurve(const TopoDS_Edge& e);

// Returns the number of subshapes, if the shape is a compound
asiAlgo_EXPORT unsigned int GetNumberOfSubshapes(const TopoDS_Shape& shape);

// returns the central point of the face
asiAlgo_EXPORT gp_Pnt GetCentralFacePoint(const class TopoDS_Face& face);

//// puts all faces with the same origin to one TopoDS_Compound
//// Maps all compounds with its name in the map
//asiAlgo_EXPORT ListPNamedShape GroupFaces(const PNamedShape shape, tigl::ShapeGroupMode groupType);

//asiAlgo_EXPORT TopoDS_Shape GetFacesByName(const PNamedShape shape, const std::string& name);

// Returns the coordinates of the bounding box of the shape
asiAlgo_EXPORT void GetShapeExtension(const TopoDS_Shape& shape,
                                   double& minx, double& maxx,
                                   double& miny, double& maxy,
                                   double& minz, double& maxz);

//// Returns a unique Hashcode for a specific geometric component based on its loft
//asiAlgo_EXPORT int GetComponentHashCode(tigl::ITiglGeometricComponent&);

// Creates an Edge from the given Points by B-Spline interpolation
asiAlgo_EXPORT TopoDS_Edge EdgeSplineFromPoints(const std::vector<gp_Pnt>& points);

// Computes the intersection point of a face and an edge
asiAlgo_EXPORT bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst, double tolerance = Precision::Confusion());

// Computes the intersection point of a face and a wire
asiAlgo_EXPORT bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Wire& wire, gp_Pnt& dst, double tolerance = Precision::Confusion());

// Comuptes the intersection points of two wires
asiAlgo_EXPORT bool GetIntersectionPoint(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, intersectionPointList& intersectionPoints, const double tolerance=Precision::SquareConfusion());

// Checks, whether a points lies inside a given shape, which must be a solid.
// An optional bounding box can be passed to include a bounding box test as a prephase
asiAlgo_EXPORT bool IsPointInsideShape(const TopoDS_Shape& solid, gp_Pnt point, Bnd_Box const* bounding_box = nullptr);

// Checks, whether a point lies inside a given face
asiAlgo_EXPORT bool IsPointInsideFace(const TopoDS_Face& face, gp_Pnt point);

// Checks whether a point lies above or below a plane (determined by direction of normal)
asiAlgo_EXPORT bool IsPointAbovePlane(const gp_Pln& pln, gp_Pnt point);

// Returns the single face contained in the passed shape
// Throws an exception when number of faces != 1
asiAlgo_EXPORT TopoDS_Face GetSingleFace(const TopoDS_Shape& shape);

// Builds a face out of 4 points
asiAlgo_EXPORT TopoDS_Face BuildFace(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Pnt& p4);

// Returns true, if a path is relative
//asiAlgo_EXPORT bool IsPathRelative(const std::string&);

// Returns true, if a file is readable
asiAlgo_EXPORT bool IsFileReadable(const std::string& filename);

asiAlgo_EXPORT std::string FileExtension(const std::string& filename);

// get the continuity of two edges which share a common vertex
asiAlgo_EXPORT TiglContinuity getEdgeContinuity(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2);

asiAlgo_EXPORT Standard_Boolean IsEqual(const TopoDS_Shape& s1, const TopoDS_Shape& s2);

// Method for building a face out of two wires
asiAlgo_EXPORT TopoDS_Face BuildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

// Method for building a face out of two wires and a direction vector
asiAlgo_EXPORT TopoDS_Face BuildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir);

// Method for building a face out of closed wire.
// The algorithm tries to build a face using a plane as surface, in case this fails
// the BRepFill_Filling class is used.
asiAlgo_EXPORT TopoDS_Face BuildFace(const TopoDS_Wire& wire);

// Method for building a ruled face between the two wires. The method
// approximates the two wires by curves, and generates a single face
// between these curves.
// This can be used for generating non-planar faces. For planar faces
// see buildFace method
//asiAlgo_EXPORT TopoDS_Face BuildRuledFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

// Method for building a wire out of two points
asiAlgo_EXPORT TopoDS_Wire BuildWire(const gp_Pnt& p1, const gp_Pnt& p2);

// Method for building a wire out of the edges from the passed geometry
asiAlgo_EXPORT TopoDS_Wire BuildWireFromEdges(const TopoDS_Shape& edges);

// Returns a list of wires built from all connected edges in the passed shape
asiAlgo_EXPORT void BuildWiresFromConnectedEdges(const TopoDS_Shape& shape, TopTools_ListOfShape& wireList);

// Method for creating a face from an opened wire
asiAlgo_EXPORT TopoDS_Wire CloseWire(const TopoDS_Wire& wire);

// Method for closing two wires to a single one, 
// The method determines a direction vector based on the end vertices of wire1
// and calls the second closeWires method
asiAlgo_EXPORT TopoDS_Wire CloseWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

// Method for closing two wires to a single one, 
// the passed vector is used to define the upper and lower end vertices of the wires
asiAlgo_EXPORT TopoDS_Wire CloseWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir);

// Method for sorting the edges of a wire
asiAlgo_EXPORT TopoDS_Wire SortWireEdges(const TopoDS_Wire& wire);

// Returns the first and last vertex of the passed shape along the passed 
// direction
asiAlgo_EXPORT bool GetMinMaxPoint(const TopoDS_Shape& shape, const gp_Vec& dir, gp_Pnt& minPnt, gp_Pnt& maxPnt);

// Returns the list of shapes of the passed type from the passed shape
asiAlgo_EXPORT void GetListOfShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, TopTools_ListOfShape& result);

// Returns all shapes with the given type contained in the given shape
asiAlgo_EXPORT std::vector<TopoDS_Shape> GetSubShapes(const TopoDS_Shape& shape, TopAbs_ShapeEnum type);

// Cuts two shapes and returns the common geometry (e.g. intersection edges)
// Throws an exception in case the intersection failed
asiAlgo_EXPORT TopoDS_Shape CutShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);

// Helper for splitting a shape by another shape
asiAlgo_EXPORT TopoDS_Shape SplitShape(const TopoDS_Shape& src, const TopoDS_Shape& tool);

// Method for finding all directly and indirectly connected edges
// The method loops over the passed edgeList and checks for each element if it
// is connected to the passed edge. When an edge is found it is removed from 
// the edgeList and added to the targetList. Additionally for this edge all
// connected edges are also added to the targetList by recursively calling this
// method. Finally all directly or indirectly connected edges to the passed
// edge are moved from the edgeList to the targetList
asiAlgo_EXPORT void FindAllConnectedEdges(const TopoDS_Edge& edge, TopTools_ListOfShape& edgeList, TopTools_ListOfShape& targetList);

// Method for checking if two edges have a common vertex (same position)
asiAlgo_EXPORT bool CheckCommonVertex(const TopoDS_Edge& e1, const TopoDS_Edge& e2);

// Method for searching all vertices which are only connected to a single edge
asiAlgo_EXPORT void GetEndVertices(const TopoDS_Shape& shape, TopTools_ListOfShape& endVertices);

// Method for finding the face which has the lowest distance to the passed point
asiAlgo_EXPORT TopoDS_Face GetNearestFace(const TopoDS_Shape& src, const gp_Pnt& pnt);

// Method for finding the center of mass of a shape
asiAlgo_EXPORT gp_Pnt GetCenterOfMass(const TopoDS_Shape& shape);

// Method for finding the area of a shape or the area that is framed by the shape
asiAlgo_EXPORT double GetArea(const TopoDS_Shape &shape);

// Method for checking for duplicate edges in the passed shape.
// The method returns a shape with only unique edges
// NOTE: THIS METHOD ONLY CHECKS THE VERTEX POSITIONS, AND THE MIDDLE POINT 
//       OF THE EDGES, BUT DOES NOT COMPARE THE CURVES EXACTLY
asiAlgo_EXPORT TopoDS_Shape RemoveDuplicateEdges(const TopoDS_Shape& shape);

inline double Radians(double degree)
{
    return degree / 180. * M_PI;
}

inline double Degrees(double radians)
{
    return 180.*radians / M_PI;
}

// Clamps val between min and max
asiAlgo_EXPORT int Clamp(int val, int min, int max);
asiAlgo_EXPORT double Clamp(double val, double min, double max);
asiAlgo_EXPORT size_t Clamp(size_t val, size_t min, size_t max);

// linearly interpolate between two values result = x*(1−a)+y*a.
asiAlgo_EXPORT double Mix(double x, double y, double a);

// Normalizes the input angle into the range [0, 360)
asiAlgo_EXPORT double NormalizeAngleDeg(double angleDeg);

// Creates a linear spaces array but with some additional breaking points
// If the breaking points are very close to a point, the point will be replaced
// Else, the breaking point will be inserted
asiAlgo_EXPORT std::vector<double> LinspaceWithBreaks(double umin, double umax, size_t n_values, const std::vector<double>& breaks);

// Transforms a shape accourding to the given coordinate transformation
//asiAlgo_EXPORT TopoDS_Shape TransformedShape(const tigl::CTiglTransformation& transformationToGlobal, TiglCoordinateSystem cs, const TopoDS_Shape& shape);
//asiAlgo_EXPORT TopoDS_Shape TransformedShape(const tigl::CTiglRelativelyPositionedComponent& component, TiglCoordinateSystem cs, const TopoDS_Shape& shape);

/// Converters between std::vectors and opencascade vectors
asiAlgo_EXPORT Handle(TColgp_HArray1OfPnt) OccArray(const std::vector<gp_Pnt>& pnts);
asiAlgo_EXPORT Handle(TColgp_HArray1OfPnt) OccArray(const std::vector<tigl::CTiglPoint>& pnts);

asiAlgo_EXPORT Handle(TColStd_HArray1OfReal) OccFArray(const std::vector<double>& vector);
asiAlgo_EXPORT Handle(TColStd_HArray1OfInteger) OccIArray(const std::vector<int>& vector);

template <typename T>
size_t IndexFromUid(const std::vector<std::unique_ptr<T> >& vectorOfPointers, const std::string& uid)
{
    const auto found = std::find_if(vectorOfPointers.begin(), vectorOfPointers.end(), [&uid](const std::unique_ptr<T>& ptr) {
        return ptr->GetUID() == uid;
    });
    return found - vectorOfPointers.begin();
}

/**
 * @brief Searches for a entry in a range and returns its index
 *
 * This function can be used with std::arrays, std::vectors
 * and also possible other containers supported by std::distance
 *
 * If the index is not found, the size of the range will be returned instead
 */
template <typename ForwardIter, typename Compare>
size_t FindIndex(ForwardIter begin, ForwardIter end, Compare comp)
{
    const auto it = std::find_if(begin, end, comp);
    if (it != end) {
        return std::distance(begin, it);
    }
    else {
        return std::distance(begin, end);
    }
}

/**
 * @brief Checks, whether an array contains a value or not
 *
 * @param array The array to be searched in
 * @param val The value to be saerched for
 * @param tolerance This functions is typically used with floating point values. The tolerance allows
 *                  to search for a value within the specified tolerance.
 */
template <typename ArrayLike, typename ValueType>
bool Contains(const ArrayLike& array, ValueType val, ValueType tolerance)
{
    auto idx = FindIndex(std::begin(array), std::end(array), [val, tolerance](const typename ArrayLike::value_type& cval) {
        return fabs(cval - val) < tolerance;
    });

    return idx < array.size();
}

/**
 * Returns true, if all elements are the same
 */
template <typename ForwardIter>
bool AllSame(ForwardIter begin, ForwardIter end)
{
    return std::adjacent_find( begin, end, std::not_equal_to<typename std::iterator_traits<ForwardIter>::value_type>() ) == end;
}

template <class ArrayType, typename BinaryPredicate, typename BinaryMerge>
void ReplaceAdjacentWithMerged(ArrayType& list, BinaryPredicate is_adjacent, BinaryMerge merged)
{
    for (auto it = std::begin(list); it != std::end(list);) {
        auto nextIt = it;
        
        if (++nextIt == std::end(list)) {
            return;
        }
        
        if (is_adjacent(*it, *nextIt)) {
            const auto merged_val = merged(*it, *nextIt);
            it = list.erase(it, ++nextIt);
            it = list.insert(it, merged_val);
        }
        else {
            it = nextIt;
        }
    }
}

#endif // TIGLCOMMONFUNCTIONS_H
