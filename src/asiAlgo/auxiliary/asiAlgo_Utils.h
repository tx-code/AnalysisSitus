//-----------------------------------------------------------------------------
// Created on: 20 November 2015
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

#ifndef asiAlgo_Utils_h
#define asiAlgo_Utils_h

// asiAlgo includes
#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_BorderTrihedron.h>
#include <asiAlgo_Collections.h>
#include <asiAlgo_ConvertCanonicalSummary.h>
#include <asiAlgo_FeatureAngleType.h>
#include <asiAlgo_FeatureFaces.h>
#include <asiAlgo_Naming.h>
#include <asiAlgo_TopoSummary.h>

// Active Data (API) includes
#include <ActAPI_ILogger.h>
#include <ActAPI_IProgressNotifier.h>

// Active Data includes
#include <ActData_Mesh.h>

// OCCT includes
#include <Bnd_Box.hxx>
#include <BRep_Tool.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_SweptSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <gp_Trsf.hxx>
#include <math_BullardGenerator.hxx>
#include <math_Function.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

// Standard includes
#include <limits>
#include <unordered_set>

//-----------------------------------------------------------------------------

#define asiAlgo_TooSmallValue 1.0e-4
#define asiAlgo_SlashStr      "/"

//-----------------------------------------------------------------------------

//! Auxiliary functions facilitating working with OCCT topological shapes.
namespace asiAlgo_Utils
{
  //! Functions to work with strings.
  namespace Str
  {
    //! Derives filename and extension out of the passed full path.
    //! \param[in]  fullPath  the input full path.
    //! \param[out] name      the output filename.
    //! \param[out] extension the output extension.
    asiAlgo_EXPORT void
      FileNameAndExtension(const TCollection_AsciiString& fullPath,
                           TCollection_AsciiString&       name,
                           TCollection_AsciiString&       extension);

    //! Splits the passed string by the given delimiter. Note that the
    //! passed output vector is not cleaned up beforehand.
    //! \param source_str [in]  input string to split.
    //! \param delim_str  [in]  delimiter string.
    //! \param result     [out] resulting collection of tokens after split.
    asiAlgo_EXPORT void
      Split(const std::string&        source_str,
            const std::string&        delim_str,
            std::vector<std::string>& result);

    //! Splits the passed string by the given delimiter. Note that the
    //! passed output vector is not cleaned up beforehand.
    //! \param source_str [in]  input string to split.
    //! \param delim      [in]  delimiter.
    //! \param result     [out] resulting collection of tokens after split.
    asiAlgo_EXPORT void
      Split(const TCollection_AsciiString&        source_str,
            const char*                           delim,
            std::vector<TCollection_AsciiString>& result);

    //! Replaces all occurrences of {what} with {with} in string {str}.
    //! \param str [in/out] target string.
    //! \param what [in] sub-string to replace.
    //! \param with [in] string to replace with.
    asiAlgo_EXPORT void
      ReplaceAll(std::string&       str,
                 const std::string& what,
                 const std::string& with);

    //! Replaces all occurrences of `what` with `with` in string `str`.
    //! \param[in,out] str  target string.
    //! \param[in]     what sub-string to replace.
    //! \param[in]     with string to replace with.
    asiAlgo_EXPORT void
      ReplaceAll(TCollection_AsciiString&       str,
                 const TCollection_AsciiString& what,
                 const TCollection_AsciiString& with);

    //! Extracts substring from the passed source.
    //! \param source [in] input string to extract substring from.
    //! \param idx_F  [in] 0-based index to start extraction from (inclusively).
    //! \param length [in] number of characters to extract.
    //! \return resulting substring.
    asiAlgo_EXPORT std::string
      SubStr(const std::string& source,
             const int          idx_F,
             const int          length);

    //! Returns the passed string ensuring that it has a trailing slash.
    //! \param[in] strIN input string.
    //! \return modified string.
    asiAlgo_EXPORT std::string
      Slashed(const char* strIN);

    //! Returns the passed string ensuring that it has a trailing slash.
    //! \param[in] strIN input string.
    //! \return modified string.
    asiAlgo_EXPORT std::string
      Slashed(const std::string& strIN);

    //! Returns the passed string ensuring that it has a trailing slash.
    //! \param[in] strIN input string.
    //! \return modified string.
    asiAlgo_EXPORT TCollection_AsciiString
      Slashed(const TCollection_AsciiString& strIN);

    //! Checks whether the passed string is number or not.
    //! \param str [in] string to check.
    //! \return true/false.
    asiAlgo_EXPORT bool
      IsNumber(const std::string& str);

    //! Converts the passed value to string. This function is used to
    //! substitute std::to_string() for compilers incompatible with
    //! C++ 11.
    //! \param value [in] value to convert.
    //! \return string.
    template <typename T>
    std::string ToString(T value)
    {
      std::ostringstream os;
      os << std::setprecision( std::numeric_limits<double>::max_digits10 );
      os << value;
      return os.str();
    }

    //! Converts the passed string to number.
    //! \param str           [in] string to convert.
    //! \param default_value [in] default value to use.
    //! \return string.
    template <typename T>
    static T ToNumber(const std::string& str,
                      const T            default_value = 0)
    {
      std::istringstream is(str);
      T result;
      (is >> result) ? result : default_value;
      return result;
    }

    //! Extracts the name of the last directory from the
    //! given path.
    asiAlgo_EXPORT std::string
      LastDirname(const std::string& filename);

    //! Extracts base filename with our without extension.1
    asiAlgo_EXPORT TCollection_AsciiString
      BaseFilename(const TCollection_AsciiString& filename,
                   const bool                     doKeepExt);

    //! Extracts base filename with our without extension.
    asiAlgo_EXPORT std::string
      BaseFilename(const std::string& filename,
                   const bool         doKeepExt);

    //! Given the full filename, this function transforms it to the
    //! reference data filename as `<path>/<basename>.ref`.
    //!
    //! \param[in] filename input filename.
    //! \param[in] refExt   extension of the reference file (normally it is "ref").
    //! \return full filename for the reference data file.
    asiAlgo_EXPORT std::string
      RefFilename(const std::string& filename,
                  const std::string& refExt);
  } // Str namespace.

  //! Functions for working with environment.
  namespace Env
  {
    asiAlgo_EXPORT
      std::string AsiTestData();

    asiAlgo_EXPORT
      std::string AsiTestDumping();

    asiAlgo_EXPORT
      std::string AsiTestScripts();

    asiAlgo_EXPORT
      std::string AsiDocs();

    asiAlgo_EXPORT
      std::string GetVariable(const char* varName);
  } // Env namespace.

  //! Functions to work with JSON-formatted strings.
  namespace Json
  {
    //! Dumps an iterable to JSON.
    //! \param[in] iterable the iterable to dump
    //! \return a JSON representation of the given iterable, e.g. [1, 2, 3].
    template <typename T>
    std::string FromIterable(const T& iterable)
    {
      std::stringstream out;

      out << "[";
      if ( !iterable.empty() )
      {
        auto it = iterable.begin();
        out << *it++;

        for ( auto end = iterable.end(); it != end; ++it )
        {
          out << ", " << *it;
        }
      }
      out << "]";

      return out.str();
    }

    //! Dumps the passed elements as a set in JSON.
    //! \param[in] elements the elements to dump.
    //! \return the outcome string representation of the elements.
    template <class T>
    std::string FromUnorderedSet(const std::unordered_set<T>& elements)
    {
      std::ostringstream result;

      result << "{ ";
      bool first = true;
      for ( const auto& elem : elements )
      {
        if (first) first = false;
        else       result << ", ";
        result << elem;
      }
      result << " }";

      return result.str();
    }

    //! Reads the passed JSON block as an array of integers and
    //! populates the second argument to compose a feature out
    //! of that array.
    //! \param[in]  jsonBlock the JSON block to interpret.
    //! \param[out] feature   the feature to compose.
    asiAlgo_EXPORT void
      ReadFeature(void*            pJsonBlock,
                  asiAlgo_Feature& feature);

    //! Reads the passed JSON block as an array of integer arrays.
    //! \param[in]  pJsonBlock the JSON block to interpret.
    //! \param[out] features   the container of features to compose.
    asiAlgo_EXPORT void
      ReadFeatures(void*                         pJsonBlock,
                   std::vector<asiAlgo_Feature>& features);

    //! Reads the passed JSON block as a pair of integers.
    //! \param[in]  jsonBlock the JSON block to interpret.
    //! \param[out] pair      the pair to compose.
    asiAlgo_EXPORT void
      ReadPair(void*                                pJsonBlock,
               tl::optional< std::pair<int, int> >& pair);

    //! Reads the passed JSON block as a triple of coordinates.
    //! \param[in]  pJsonBlock the JSON block to interpret.
    //! \param[out] coords     the outcome coords.
    asiAlgo_EXPORT void
      ReadCoords(void*   pJsonBlock,
                 gp_XYZ& coords);

    //! Dumps the passed feature as a JSON array.
    //! \param[in] map the map to dump.
    //! \return a JSON array containing the elements of the map.
    asiAlgo_EXPORT std::string
      FromFeature(const asiAlgo_Feature& map);

    //! Dumps the passed direction as a JSON array.
    //! \param[in] gp_Dir the direction to dump.
    //! \return `[X(), Y(), Z()]`.
    asiAlgo_EXPORT std::string
      FromDirAsTuple(const gp_Dir& dir);

    //! Dumps the passed pair as a JSON array.
    //! \param[in] pair the pair to dump.
    //! \return a JSON array containing the elements of the pair.
    asiAlgo_EXPORT std::string
      FromPair(const tl::optional< std::pair<int, int> >& pair);
  }

  //! Functions for verification of results.
  namespace Verify
  {
    //! Compares the two passed vectors by counting and matching occurrences
    //! of each element.
    //! \param[in] V1   the first vector.
    //! \param[in] V2   the second vector.
    //! \param[in] prec the precision to use.
    //! \return true for matching vector, false -- otherwise.
    asiAlgo_EXPORT bool
      CompareOccurrences(const std::vector<double>& V1,
                         const std::vector<double>& V2,
                         const double               prec);

    //! This method compares two optionals given that they can have different states, i.e. initialized (has value)
    //! versus uninitialized. While on Windows direct comparison of the dereferenced values was quite Ok, on Linux
    //! we have to take special care of the optionals' states to avoid surprises at runtime.
    //! \param[in] a the first optional to compare.
    //! \param[in] b the second optional to compare.
    //! \return true in case of equality, false -- otherwise.
    asiAlgo_EXPORT bool
      AreEqual(const tl::optional<bool>& a,
               const tl::optional<bool>& b);

    //! This method compares two optionals given that they can have different states, i.e. initialized (has value)
    //! versus uninitialized. While on Windows direct comparison of the dereferenced values was quite Ok, on Linux
    //! we have to take special care of the optionals' states to avoid surprises at runtime.
    //! \param[in] a    the first optional to compare.
    //! \param[in] b    the second optional to compare.
    //! \param[in] prec the numerical precision to use in the case when both optionals are initialized.
    //! \return true in case of equality, false -- otherwise.
    asiAlgo_EXPORT bool
      AreEqual(const tl::optional<double>& a,
               const tl::optional<double>& b,
               const double                prec);

    //! This method compares two optional axes given that they can have different states, i.e. initialized (has value)
    //! versus uninitialized. While on Windows direct comparison of the dereferenced values was quite Ok, on Linux
    //! we have to take special care of the optionals' states to avoid surprises at runtime.
    //! \param[in] a           the first optional to compare.
    //! \param[in] b           the second optional to compare.
    //! \param[in] angTolerDeg the angular precision to use in the case when both optionals are initialized.
    //! \return true in case of equality, false -- otherwise.
    asiAlgo_EXPORT bool
      AreEqualAxes(const tl::optional<gp_Ax1>& a,
                   const tl::optional<gp_Ax1>& b,
                   const double                angTolerDeg);

    //! This method compares two optional dirs given that they can have different states, i.e. initialized (has value)
    //! versus uninitialized. While on Windows direct comparison of the dereferenced values was quite Ok, on Linux
    //! we have to take special care of the optionals' states to avoid surprises at runtime.
    //! \param[in] a           the first optional to compare.
    //! \param[in] b           the second optional to compare.
    //! \param[in] angTolerDeg the angular precision to use in the case when both optionals are initialized.
    //! \return true in case of equality, false -- otherwise.
    asiAlgo_EXPORT bool
      AreEqualDirs(const tl::optional<gp_Dir>& a,
                   const tl::optional<gp_Dir>& b,
                   const double                angTolerDeg);
  }

  //! Returns geometry of a face as a string label.
  //! \param face [in] face to inspect.
  //! \return label.
  asiAlgo_EXPORT std::string
    FaceGeometryName(const TopoDS_Face& face);

  //! Converts the passed feature angle to string,
  asiAlgo_EXPORT TCollection_AsciiString
    FeatureAngleToString(const asiAlgo_FeatureAngleType angle);

  //! Converts the passed named shape to string.
  asiAlgo_EXPORT TCollection_AsciiString
    NamedShapeToString(const TopoDS_Shape&           subShape,
                       const int                     pedigreeId,
                       const int                     globalId,
                       const Handle(asiAlgo_Naming)& naming);

  //! Converts orientation of the given shape to string.
  //! \param shape [in] shape to access orientation.
  //! \return string representation of orientation.
  asiAlgo_EXPORT TCollection_AsciiString
    OrientationToString(const TopoDS_Shape& shape);

  //! Converts the passed orientation to string.
  //! \param ori [in] orientation to convert.
  //! \return string representation of orientation.
  asiAlgo_EXPORT TCollection_AsciiString
    OrientationToString(const TopAbs_Orientation ori);

  //! Converts the passed continuity to string.
  //! \param[in] cont continuity.
  //! \return string representation of continuity.
  asiAlgo_EXPORT TCollection_AsciiString
    ContinuityToString(const GeomAbs_Shape cont);

  //! Converts the passed location to string.
  //! \param loc [in] location to convert.
  //! \return string representation of location.
  asiAlgo_EXPORT TCollection_AsciiString
    LocationToString(const TopLoc_Location& loc);

  //! Returns human-readable curve name.
  //! \param curve [in] curve to inspect.
  //! \return curve name.
  asiAlgo_EXPORT std::string
    CurveName(const Handle(Geom_Curve)& curve);

  //! Returns human-readable surface name.
  //! \param surf [in] surface to inspect.
  //! \return surface name.
  asiAlgo_EXPORT std::string
    SurfaceName(const Handle(Geom_Surface)& surf);

  //! Returns shape type as string.
  //! \param shapeType [in] shape type in question.
  //! \return type as string.
  asiAlgo_EXPORT std::string
    ShapeTypeStr(const TopAbs_ShapeEnum& shapeType);

  //! Returns shape type as string.
  //! \param shape [in] shape to get type string for.
  //! \return type string.
  asiAlgo_EXPORT std::string
    ShapeTypeStr(const TopoDS_Shape& shape);

  //! Returns shape address as string with a prefix indicating its type,
  //! e.g. "e05FB0" is an edge and "f780A1" is a face.
  //! \param shape [in] shape to extract the address for.
  //! \return address of TShape as string prefixed by type indicator.
  asiAlgo_EXPORT std::string
    ShapeAddrWithPrefix(const TopoDS_Shape& shape);

  //! Returns TShape address as string.
  //! \param tshape [in] TShape pointer to extract the address for.
  //! \return address of TShape as string.
  asiAlgo_EXPORT std::string
    ShapeAddr(const Handle(TopoDS_TShape)& tshape);

  //! Returns TShape address as string.
  //! \param shape [in] shape to extract the address for.
  //! \return address of TShape as string.
  asiAlgo_EXPORT std::string
    ShapeAddr(const TopoDS_Shape& shape);

  //! Checks edge type.
  //! \param[in]  edge      edge to check.
  //! \param[out] basecurve base curve.
  //! \return true/false.
  template<typename TCurve>
  bool IsTypeOf(const TopoDS_Edge& edge,
                Handle(TCurve)&    basecurve)
  {
    if ( edge.IsNull() )
      return false;

    double f, l;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
    //
    if ( curve.IsNull() )
      return false;

    // Check host curve directly.
    if ( curve->IsInstance( STANDARD_TYPE(TCurve) ) )
    {
      basecurve = Handle(TCurve)::DownCast(curve);
      return true;
    }

    // Check trimmed curve which may encapsulate the curve type we're looking for.
    if ( curve->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
    {
      Handle(Geom_TrimmedCurve)
        TC = Handle(Geom_TrimmedCurve)::DownCast(curve);

      // Check basis curve.
      basecurve = Handle(TCurve)::DownCast( TC->BasisCurve() );
      //
      if ( !basecurve.IsNull() )
        return true;
    }

    return false;
  }

  //! Checks edge type.
  //! \param[in] edge edge to check.
  //! \return true/false.
  template<typename TCurve>
  bool IsTypeOf(const TopoDS_Edge& edge)
  {
    Handle(TCurve) basecurve;
    return IsTypeOf<TCurve>(edge, basecurve);
  }

  //! Checks face type.
  //! \param[in]  face     face to check.
  //! \param[out] basesurf base surface.
  //! \return true/false.
  template<typename TSurf>
  bool IsTypeOf(const TopoDS_Face& face,
                Handle(TSurf)&     basesurf)
  {
    if ( face.IsNull() )
      return false;

    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

    // Check host surface directly.
    if ( surf->IsInstance( STANDARD_TYPE(TSurf) ) )
    {
      basesurf = Handle(TSurf)::DownCast(surf);
      return true;
    }

    // Check trimmed surface which may encapsulate the surface type we're looking for.
    if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
    {
      Handle(Geom_RectangularTrimmedSurface)
        RT = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);

      // Check basis surface.
      basesurf = Handle(TSurf)::DownCast( RT->BasisSurface() );
      //
      if ( !basesurf.IsNull() )
        return true;
    }

    return false;
  }

  //! Checks face type.
  //! \param[in] face face to check.
  //! \return true/false.
  template<typename TSurf>
  bool IsTypeOf(const TopoDS_Face& face)
  {
    Handle(TSurf) basesurf;
    return IsTypeOf<TSurf>(face, basesurf);
  }

  //! Checks surface type.
  //! \param[in] surface surface to check.
  //! \return true/false.
  template<typename TSurf>
  static bool IsTypeOf(const Handle(Geom_Surface)& surface)
  {
    if ( surface->IsInstance( STANDARD_TYPE(TSurf) ) )
      return true;

    return false;
  }

  //! Checks if the passed face has planar support.
  //! \param[in] face face to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsPlanar(const TopoDS_Face& face);

  //! Checks if the passed face has planar support.
  //! \param[in]  face  face to check.
  //! \param[out] plane planar support.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsPlanar(const TopoDS_Face&  face,
             Handle(Geom_Plane)& plane);

  //! Checks if the passed face has cylindrical support.
  //! \param[in] face face to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCylindrical(const TopoDS_Face& face);

  //! Checks if the passed face has cylindrical support and returns the
  //! extracted cylindrical primitive.
  //! \param[in]  face face to check.
  //! \param[out] cyl  extracted cylindrical primitive.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCylindrical(const TopoDS_Face& face,
                  gp_Cylinder&       cyl);

  //! Checks if the passed face has cylindrical support.
  //! \param[in]  face face to check.
  //! \param[out] ax   axis of the cylindrical surface.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCylindrical(const TopoDS_Face& face,
                  gp_Ax1&            ax);

  //! Checks if the passed face has cylindrical support and returns the
  //! cylinder's radius.
  //! \param[in]  face   face to check.
  //! \param[out] radius radius of the underlying cylinder.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCylindrical(const TopoDS_Face& face,
                  double&            radius);

  //! Checks if the passed face has cylindrical support and returns the
  //! basic properties of a cylinder.
  //! \param[in]  face      face to check.
  //! \param[out] radius    radius of the underlying cylinder.
  //! \param[out] ax        axis of the cylinder.
  //! \param[out] angle_min min angle.
  //! \param[out] angle_max max angle.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCylindrical(const TopoDS_Face& face,
                  double&            radius,
                  gp_Ax1&            ax,
                  double&            angle_min,
                  double&            angle_max);

  //! Checks if the passed face has cylindrical support and returns the
  //! basic properties of a cylinder.
  //! \param[in]  face          face to check.
  //! \param[out] radius        radius of the underlying cylinder.
  //! \param[out] ax            axis of the cylinder.
  //! \param[in]  computeBounds indicates whether to compute UV bounds of the cylindrical face.
  //!                           This parameter should be `true` in most cases. When it is `false`,
  //!                           the props `angle_min`, `angle_max`, `h_min`, and `h_max` remain
  //!                           uninitialized, but the computations get faster (as not bounding
  //!                           box is computed). Set this argument `false` if you are only
  //!                           interested in the radius and axis of the cylinder.
  //! \param[out] angle_min     min angle.
  //! \param[out] angle_max     max angle.
  //! \param[out] h_min         min longitudal parameter value.
  //! \param[out] h_max         max longitudal parameter value.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCylindrical(const TopoDS_Face& face,
                  double&            radius,
                  gp_Ax1&            ax,
                  const bool         computeBounds,
                  double&            angle_min,
                  double&            angle_max,
                  double&            h_min,
                  double&            h_max);

  //! Checks whether the passed face has conical support.
  //! \param[in] face face to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsConical(const TopoDS_Face& face);

  //! Checks whether the passed face has conical support.
  //! \param[in]  face face to check.
  //! \param[out] ax1  cone axis.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsConical(const TopoDS_Face& face,
              gp_Ax1&            ax1);

  //! Checks whether the passed face has spherical support.
  //! \param[in] face face to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsSpherical(const TopoDS_Face& face);

  //! Checks whether the passed face has toroidal support.
  //! \param[in] face face to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsToroidal(const TopoDS_Face& face);

  //! Checks if the passed edge is circular.
  //! \param[in] edge edge to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCircular(const TopoDS_Edge& edge);

  //! Checks if the passed curve is circular.
  //! \param[in] curve curve to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCircular(const Handle(Geom_Curve)& curve);

  //! Checks if the passed curve is circular.
  //! \param[in]  curve the curve to check.
  //! \param[out] circ  the extracted circle.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsCircular(const Handle(Geom_Curve)& curve,
               gp_Circ&                  circ);

  //! Checks if the passed edge is straight.
  //! \param[in] edge edge to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsStraight(const TopoDS_Edge& edge);

  //! Checks if the passed edge is straight.
  //! \param[in]  edge edge to check.
  //! \param[out] line host curve of the straight edge.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsStraight(const TopoDS_Edge& edge,
               Handle(Geom_Line)& line);

  //! Checks if the passed pcurve is a straight line.
  //! \param[in] pcu    pcurve to check.
  //! \param[in] canrec whether to try canonical recognition.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsStraightPCurve(const Handle(Geom2d_Curve)& pcu,
                     const bool                  canrec);

  //! Checks if the passed pcurve is a straight line.
  //! \param[in]  pcu    pcurve to check.
  //! \param[out] lin    straight line primitive.
  //! \param[in]  canrec whether to try canonical recognition.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsStraightPCurve(const Handle(Geom2d_Curve)& pcu,
                     gp_Lin2d&                   lin,
                     const bool                  canrec);

  //! Checks whether the passed shape is empty. A series of nested
  //! compounds is also considered an empty shape.
  //! \param[in] shape the shape to check.
  //! \return true for empty shape, false -- otherwise.
  asiAlgo_EXPORT bool
    IsEmptyShape(const TopoDS_Shape& shape);

  //! Checks whether the passed location (a transformation matrix) is
  //! identity (i.e., it defines no real transformation).
  //! \param[in] loc the location to check.
  //! \return true for the identity, false -- otherwise.
  asiAlgo_EXPORT bool
    IsIdentity(const TopLoc_Location& loc);

  //! Checks whether the curve or its basic curve is circular.
  //! \param[in] curve curve to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    IsBasisCircular(const Handle(Geom_Curve)& curve);

  //! Checks if the passed planar surfaces are parallel.
  //! \param[in] S1      the first surface to check.
  //! \param[in] S2      the second surface to check.
  //! \param[in] angPrec the angular precision to use.
  //! \return true/false.
  asiAlgo_EXPORT bool
    AreParallel(const Handle(Geom_Plane)& S1,
                const Handle(Geom_Plane)& S2,
                const double              angPrec);

  //! Applies the passed transformation to the given shape. Returns another shape
  //! as a result (no deep copy of geometry is performed, only location is
  //! changed).
  //! \param[in] shape  shape to apply transformation to.
  //! \param[in] XPos   X position.
  //! \param[in] YPos   Y position.
  //! \param[in] ZPos   Z position.
  //! \param[in] AngleA rotation angle A (radians).
  //! \param[in] AngleB rotation angle B (radians).
  //! \param[in] AngleC rotation angle C (radians).
  //! \param[in] doCopy whether to copy the shape.
  //! \return relocated shape.
  asiAlgo_EXPORT TopoDS_Shape
    ApplyTransformation(const TopoDS_Shape& shape,
                        const double        XPos,
                        const double        YPos,
                        const double        ZPos,
                        const double        AngleA,
                        const double        AngleB,
                        const double        AngleC,
                        const bool          doCopy);

  //! Returns OCCT transformation structure for the given elemental
  //! transformations.
  //! \param theXPos   [in] X position.
  //! \param theYPos   [in] Y position.
  //! \param theZPos   [in] Z position.
  //! \param theAngleA [in] rotation angle A (radians).
  //! \param theAngleB [in] rotation angle B (radians).
  //! \param theAngleC [in] rotation angle C (radians).
  //! \return OCCT transformation structure.
  asiAlgo_EXPORT gp_Trsf
    Transformation(const double theXPos,
                   const double theYPos,
                   const double theZPos,
                   const double theAngleA,
                   const double theAngleB,
                   const double theAngleC);

  //! Applies the passed transformation to the given shape.
  //! \param theShape     [in] shape to transform.
  //! \param theTransform [in] transformation to apply.
  //! \param doCopy       [in] indicates whether to construct a deep copy.
  //! \return transformed shape.
  asiAlgo_EXPORT TopoDS_Shape
    ApplyTransformation(const TopoDS_Shape& theShape,
                        const gp_Trsf&      theTransform,
                        const bool          doCopy);

  //! Creates a compound from the given list of shapes. If the list contains
  //! only one not-null shape, this single shape is returned as-is.
  //! \param[in] shapes source shapes.
  //! \return resulting compound.
  asiAlgo_EXPORT TopoDS_Shape
    AssembleShapes(const TopTools_ListOfShape& shapes);

  //! Creates a compound from the given sequence of shapes. If the sequence
  //! contains only one not-null shape, this single shape is returned as-is.
  //! If the sequence is NULL or empty, then null shape is returned.
  //! \param[in] shapes source shapes.
  //! \return resulting compound.
  asiAlgo_EXPORT TopoDS_Shape
    AssembleShapes(const Handle(TopTools_HSequenceOfShape)& shapes);

  //! Calculates bounding box for the given shape.
  //! \param shape     [in]  input shape.
  //! \param XMin      [out] min X.
  //! \param YMin      [out] min Y.
  //! \param ZMin      [out] min Z.
  //! \param XMax      [out] max X.
  //! \param YMax      [out] max Y.
  //! \param ZMax      [out] max Z.
  //! \param tolerance [in]  tolerance to enlarge the bounding box with.
  //! \param isPrecise [in]  whether to use precise bbox computation.
  //! \return false if bounding box is void.
  asiAlgo_EXPORT bool
    Bounds(const TopoDS_Shape& shape,
           double& XMin, double& YMin, double& ZMin,
           double& XMax, double& YMax, double& ZMax,
           const double tolerance = 0.0,
           const bool isPrecise = true);

  //! Computes the axis-aligned bounding box for the passed shape.
  //! \param[in]  shape     the shape of interest.
  //! \param[in]  useFacets indicates whether to compute bounding box on facets.
  //! \param[in]  keepGap   indicates whether to preserve "gap" of AABB set by OpenCascade.
  //! \param[out] bndBox    the computed axis-aligned bounding box.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Bounds(const TopoDS_Shape& shape,
           const bool          useFacets,
           const bool          keepGap,
           Bnd_Box&            bndBox);

  //! Calculates bounding box for the given triangulation.
  //! \param tris      [in]  input triangulation.
  //! \param XMin      [out] min X.
  //! \param YMin      [out] min Y.
  //! \param ZMin      [out] min Z.
  //! \param XMax      [out] max X.
  //! \param YMax      [out] max Y.
  //! \param ZMax      [out] max Z.
  //! \param tolerance [in]  tolerance to enlarge the bounding box with.
  //! \return false if bounding box is void.
  asiAlgo_EXPORT bool
    Bounds(const Handle(Poly_Triangulation)& tris,
           double& XMin, double& YMin, double& ZMin,
           double& XMax, double& YMax, double& ZMax,
           const double tolerance = 0.0);

  //! Cleans up polygonal data from the passed shape.
  asiAlgo_EXPORT void
    CleanFacets(TopoDS_Shape& shape);

  //! Reads CAD model from native OCCT b-rep file.
  //! \param filename [in]  filename.
  //! \param shape    [out] CAD model retrieved from file.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ReadBRep(const TCollection_AsciiString& filename,
             TopoDS_Shape&                  shape);

  //! Writes shape to B-Rep format.
  //! \param shape    [in] shape to write.
  //! \param filename [in] filename.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    WriteBRep(const TopoDS_Shape&            shape,
              const TCollection_AsciiString& filename);

  //! Reads STL triangulation from file.
  //! \param[in]  filename      source filename.
  //! \param[out] triangulation output triangulation.
  //! \param[in]  progress      progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ReadStl(const TCollection_AsciiString& filename,
            Handle(Poly_Triangulation)&    triangulation,
            ActAPI_ProgressEntry           progress);

  //! Reads PLY tessellation from file.
  //! \param[in]  filename source filename.
  //! \param[out] mesh     output tessellation.
  //! \param[in]  progress progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ReadPly(const TCollection_AsciiString& filename,
            Handle(ActData_Mesh)&          mesh,
            ActAPI_ProgressEntry           progress);

  //! Reads OBJ tessellation from file.
  //! \param[in]  filename source filename.
  //! \param[out] mesh     output tessellation.
  //! \param[in]  progress progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ReadObj(const TCollection_AsciiString& filename,
            Handle(ActData_Mesh)&          mesh,
            ActAPI_ProgressEntry           progress);

  //! Writes triangulation to STL file.
  //! \param[in] triangulation triangulation to write.
  //! \param[in] filename      target filename.
  //! \param[in] isBinary      indicator showing whether to save
  //!                          the mesh in Ascii format or binary
  //!                          format.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    WriteStl(const Handle(Poly_Triangulation)& triangulation,
             const TCollection_AsciiString&    filename,
             const bool                        isBinary = false);

  //! Writes triangulation to PLY file.
  //! \param[in] triangulation triangulation to write.
  //! \param[in] filename      target filename.
  //! \param[in] progress      progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    WritePly(const Handle(Poly_Triangulation)& triangulation,
             const TCollection_AsciiString&    filename,
             ActAPI_ProgressEntry              progress);

  //! Collects summary information for the given shape.
  //! \param shape   [in]  input shape.
  //! \param summary [out] shape summary.
  asiAlgo_EXPORT void
    ShapeSummary(const TopoDS_Shape&  shape,
                 asiAlgo_TopoSummary& summary);

  //! Collects summary information of the given shape: returns the number
  //! of sub-shapes of each type.
  //! \param shape [in]  shape to analyze.
  //! \param info  [out] shape summary as string.
  asiAlgo_EXPORT void
    ShapeSummary(const TopoDS_Shape&      shape,
                 TCollection_AsciiString& info);

  //! Creates a circular wire with the given radius.
  //! \param radius [in] radius of the host circle.
  //! \return created wire.
  asiAlgo_EXPORT TopoDS_Wire
    CreateCircularWire(const double radius);

  //! Skins a surface through the passed sections.
  //! \param wires [in] sections to skin.
  //! \return skinning result.
  asiAlgo_EXPORT TopoDS_Shape
    MakeSkin(const TopTools_SequenceOfShape& wires);

  //! Performs sewing.
  //! \param[in] shape     shape to sew.
  //! \param[in] tolerance sewing tolerance.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Sew(const TopoDS_Shape& shape,
        const double        tolerance,
        TopoDS_Shape&       result);

  //! Performs "same domain" expansion on faces and edges.
  //! \param[in,out] shape shape to modify.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    MaximizeFaces(TopoDS_Shape& shape);

  //! Performs "same domain" expansion on faces and edges.
  //! \param[in,out] shape   shape to modify.
  //! \param[out]    history modification history.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    MaximizeFaces(TopoDS_Shape&              shape,
                  Handle(BRepTools_History)& history);

  //! Converts the passed shape to a canonical form.
  //! \param[in,out] shape         the shape to convert.
  //! \param[in]     tol           the tolerance to use.
  //! \param[in]     checkValidity the Boolean flag indicating whether to check for validity
  //!                              after the conversion is done.
  //! \param[out]    summary       the conversion summary.
  //! \param[in]     progress      the progress notifier.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ConvertCanonical(TopoDS_Shape&                    shape,
                     const double                     tol,
                     const bool                       checkValidity,
                     asiAlgo_ConvertCanonicalSummary& summary,
                     ActAPI_ProgressEntry             progress = nullptr);

  //! Interpolates the given collection of points with B-curve of the
  //! desired degree.
  //! \param points [in]  points to interpolate.
  //! \param p      [in]  degree to use.
  //! \param result [out] interpolant.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    InterpolatePoints(const std::vector<gp_XYZ>& points,
                      const int                  p,
                      Handle(Geom_BSplineCurve)& result);

  asiAlgo_EXPORT bool
    InterpolatePoints(const Handle(asiAlgo_BaseCloud<double>)& points,
                      const int                                p,
                      Handle(Geom_BSplineCurve)&               result);

  //! Approximate the given vector of points with a B-curve of the
  //! desired degree.
  //! \param[in]  points points to approximate.
  //! \param[in]  degMin min degree to use.
  //! \param[in]  degMax max degree to limit polynomial order.
  //! \param[in]  tol3d  3D tolerance.
  //! \param[out] result interpolant.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ApproximatePoints(const std::vector<gp_XYZ>& points,
                      const int                  degMin,
                      const int                  degMax,
                      const double               tol3d,
                      Handle(Geom_BSplineCurve)& result);

  //! Fills the passed contour with a parametric surface using the
  //! constraint-filling algorithm of OpenCascade.
  //! \param[in]  curves curves defining the contour to fill.
  //! \param[out] result resulting patch.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Fill4Contour(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                 Handle(Geom_BSplineSurface)&                  result);

  //! Fills the passed contour with a Coons surface.
  //! \param[in]  C0       curve c0.
  //! \param[in]  C1       curve c1.
  //! \param[in]  B0       curve b0.
  //! \param[in]  B1       curve b1.
  //! \param[out] result   resulting patch as B-surface.
  //! \param[in]  progress progress entry.
  //! \param[in]  plotter  imperative plotter.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    FillCoons(const Handle(Geom_BSplineCurve)& C0,
              const Handle(Geom_BSplineCurve)& C1,
              const Handle(Geom_BSplineCurve)& B0,
              const Handle(Geom_BSplineCurve)& B1,
              Handle(Geom_BSplineSurface)&     result,
              ActAPI_ProgressEntry             progress,
              ActAPI_PlotterEntry              plotter);

  //! Fills the passed contour with a parametric surface using the Plate
  //! algorithm of OpenCascade to produce a class A surface.
  //! \param[in]  curves curves defining the contour to fill.
  //! \param[out] result resulting patch.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    FillContourPlate(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                     Handle(Geom_BSplineSurface)&                  result);

  //! Performs Boolean Cut of a tool shape from the given object shape.
  //! \param Object [in] object to cut from.
  //! \param Tool   [in] tool to cut out.
  //! \param fuzzy  [in] fuzzy tolerance.
  //! \return result shape.
  asiAlgo_EXPORT TopoDS_Shape
    BooleanCut(const TopoDS_Shape& object,
               const TopoDS_Shape& tool,
               const double        fuzz = 0.0);

  asiAlgo_EXPORT TopoDS_Shape
    BooleanCut(const TopoDS_Shape&         object,
               const TopTools_ListOfShape& tools,
               const bool                  isParallel,
               const double                fuzz = 0.0);

  asiAlgo_EXPORT TopoDS_Shape
    BooleanCut(const TopoDS_Shape&         object,
               const TopTools_ListOfShape& tools,
               const bool                  isParallel,
               const double                fuzz,
               BRepAlgoAPI_Cut&            API);

  //! Fuses the passed shapes.
  //! \param[in] objects shapes to fuse.
  //! \return result of the Boolean operation.
  asiAlgo_EXPORT TopoDS_Shape
    BooleanFuse(const TopTools_ListOfShape& objects);

  //! Fuses the passed shapes.
  //! \param[in]  objects shapes to fuse.
  //! \param[out] history accumulated modification history.
  //! \return result of the Boolean operation.
  asiAlgo_EXPORT TopoDS_Shape
    BooleanFuse(const TopTools_ListOfShape& objects,
                Handle(BRepTools_History)&  history);

  //! Fuses the passed shapes and optionally maximizes faces in the
  //! resulting shape.
  //! \param[in]  objects       shapes to fuse.
  //! \param[in]  maximizeFaces indicates whether to maximize faces of the
  //!                           result so that to reduce its complexity.
  //! \param[out] history       accumulated modification history.
  //! \return result of the Boolean operation.
  asiAlgo_EXPORT TopoDS_Shape
    BooleanFuse(const TopTools_ListOfShape& objects,
                const bool                  maximizeFaces,
                Handle(BRepTools_History)&  history);

  asiAlgo_EXPORT TopoDS_Shape
    BooleanIntersect(const TopTools_ListOfShape& objects);

  asiAlgo_EXPORT TopoDS_Shape
    BooleanGeneralFuse(const TopTools_ListOfShape& objects,
                       const double                fuzz,
                       BOPAlgo_Builder&            API,
                       const bool                  glue = false);

  asiAlgo_EXPORT TopoDS_Shape
    BooleanGeneralFuse(const TopTools_ListOfShape& objects,
                       const double                fuzz,
                       const bool                  glue = false);

  asiAlgo_EXPORT bool
    BooleanRemoveFaces(const TopoDS_Shape&  shape,
                       const TopoDS_Shape&  face2Remove,
                       const bool           runParallel,
                       const bool           trackHistory,
                       TopoDS_Shape&        result,
                       ActAPI_ProgressEntry progress);

  asiAlgo_EXPORT bool
    BooleanRemoveFaces(const TopoDS_Shape&         shape,
                       const TopTools_ListOfShape& faces2Remove,
                       const bool                  runParallel,
                       const bool                  trackHistory,
                       TopoDS_Shape&               result,
                       ActAPI_ProgressEntry        progress);

  asiAlgo_EXPORT void
    ExplodeBySolids(const TopoDS_Shape&   model,
                    TopTools_ListOfShape& solids);

  asiAlgo_EXPORT bool
    InvertFace(const TopoDS_Face&    face,
               TopTools_ListOfShape& inverted);

  //! Constructs B-spline curve of degree 1 from the passed poles.
  //! \param[in] trace poles of the curve to construct.
  //! \return 1-degree spline curve.
  asiAlgo_EXPORT Handle(Geom_BSplineCurve)
    PolylineAsSpline(const TColgp_Array1OfPnt& trace);

  //! Constructs B-spline curve of degree 1 from the passed poles.
  //! \param[in] trace poles of the curve to construct.
  //! \return 1-degree spline curve.
  asiAlgo_EXPORT Handle(Geom_BSplineCurve)
    PolylineAsSpline(const std::vector<gp_XYZ>& trace);

  //! Constructs 2D B-spline curve of degree 1 from the passed poles.
  //! \param[in] trace poles of the curve to construct.
  //! \return 1-degree 2D spline curve.
  asiAlgo_EXPORT Handle(Geom2d_BSplineCurve)
    PolylineAsSpline(const TColgp_Array1OfPnt2d& trace);

  //! Constructs 2D B-spline curve of degree 1 from the passed poles.
  //! \param[in] trace poles of the curve to construct.
  //! \return 1-degree 2D spline curve.
  asiAlgo_EXPORT Handle(Geom2d_BSplineCurve)
    PolylineAsSpline(const std::vector<gp_XY>& trace);

  //! Checks whether the given shape contains the given sub-shape. This method
  //! builds a map of sub-shapes, so it is quite slow.
  //! \param[in] shape    master shape to check.
  //! \param[in] subShape sub-shape in question.
  //! \return true/false.
  asiAlgo_EXPORT bool
    Contains(const TopoDS_Shape& shape,
             const TopoDS_Shape& subShape);

  asiAlgo_EXPORT TopoDS_Shape
    GetImage(const TopoDS_Shape&       source,
             BRepBuilderAPI_MakeShape& API);

  asiAlgo_EXPORT Handle(Poly_Triangulation)
    CreateTriangle(const gp_Pnt& P0,
                   const gp_Pnt& P1,
                   const gp_Pnt& P2);

  asiAlgo_EXPORT void
    HexagonPoles(const gp_XY& center,
                 const double dist2Pole,
                 gp_XY&       P1,
                 gp_XY&       P2,
                 gp_XY&       P3,
                 gp_XY&       P4,
                 gp_XY&       P5,
                 gp_XY&       P6);

  //! Constructs a two-dimensional regular polygon.
  //! \param[in]  center    center point.
  //! \param[in]  dist2Pole radius of circumcircle.
  //! \param[in]  numPoles  number of poles in the resulting regular polygon.
  //! \param[out] poles     calculated poles.
  asiAlgo_EXPORT void
    PolygonPoles(const gp_XY&        center,
                 const double        dist2Pole,
                 const int           numPoles,
                 std::vector<gp_XY>& poles);

  //! Calculates normal vectors at interior face points. This method uses
  //! point membership classification to ensure that inner points only are
  //! used.
  //!
  //! CAUTION: Face orientation is not taken into account. The returned
  //!          normal vectors represent geometric orientation of
  //!          the face (i.e., the cross product of its partials).
  //!
  //! \param[in]  face       topological face in question.
  //! \param[in]  sampleRate sampling rate from (0, 1) interval.
  //! \param[out] points     sample points (internal or lying on the boundary).
  //! \param[out] vectors    computed normal field.
  //! \param[out] average    computed average normal.
  //!
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    CalculateFaceNormals(const TopoDS_Face&                 face,
                         const double                       sampleRate,
                         Handle(asiAlgo_BaseCloud<double>)& points,
                         Handle(asiAlgo_BaseCloud<double>)& vectors,
                         gp_Vec&                            average);

  //! This function takes "just any" interior point on a face.
  //!
  //! CAUTION: This function is not super-fast as it follows generate-and-test
  //!          approach, i.e., it samples many points and selects the one lying
  //!          INSIDE the face. Therefore, do not use this method in heavy loops.
  //!
  //! \param[in]  face topological face in question.
  //! \param[out] uv   UV coordinates of the selected point.
  //! \param[out] xyz  Cartesian coordinates of the selected point in the
  //!                  modeling space.
  //!
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    GetFaceAnyInteriorPoint(const TopoDS_Face& face,
                            gp_Pnt2d&          uv,
                            gp_Pnt&            xyz);

  //! Prints some useful analytics for the passed surface to the given
  //! output stream.
  //! \param[in]  surf parametric surface in question.
  //! \param[out] out  output stream.
  asiAlgo_EXPORT void
    PrintSurfaceDetails(const Handle(Geom_Surface)& surf,
                        Standard_OStream&           out);

  //! Calculates curvature comb for the passed curve at the given parameter
  //! value. The method follows the paper "Farin, G. 2016. Curvature combs
  //! and curvature plots. CAD Computer Aided Design 80, 6�8."
  //!
  //! \param[in]  curve    parametric curve to analyze.
  //! \param[in]  u        parameter value.
  //! \param[in]  curvAmpl curvature amplification factor used to bring out
  //!                      salient features of the comb.
  //! \param[out] p        point on curve at the given parameter value.
  //! \param[out] k        curvature at the given parameter value.
  //! \param[out] c        curvature comb.
  //!
  //! \return false if a comb cannot be calculated. NOTE: the output argument
  //!         <p> will be calculated anyway.
  asiAlgo_EXPORT bool
    CalculateCurvatureComb(const Handle(Geom_Curve)& curve,
                           const double              u,
                           const double              curvAmpl,
                           gp_Pnt&                   p,
                           double&                   k,
                           gp_Vec&                   c);

  //! Calculates curvature combs along the given curve.
  //!
  //! \param[in]  curve      parametric curve to analyze.
  //! \param[in]  f          first parameter of the range of interest.
  //! \param[in]  l          last parameter of the range of interest.
  //! \param[in]  numPts     number of discretization points.
  //! \param[in]  curvAmpl   curvature amplification factor used to bring out
  //!                        salient features of the comb.
  //! \param[out] points     discretization points.
  //! \param[out] params     discretization parameters.
  //! \param[out] curvatures curvatures.
  //! \param[out] combs      curvature combs.
  //! \param[out] combsOk    statuses of combs calculation.
  //!
  //! \return false if combs calculation is not possible on the passed data.
  asiAlgo_EXPORT bool
    CalculateCurvatureCombs(const Handle(Geom_Curve)& curve,
                            const double              f,
                            const double              l,
                            const int                 numPts,
                            const double              curvAmpl,
                            std::vector<gp_Pnt>&      points,
                            std::vector<double>&      params,
                            std::vector<double>&      curvatures,
                            std::vector<gp_Vec>&      combs,
                            std::vector<bool>&        combsOk);

  //! Calculates approximate strain energy of the passed curve. The strain
  //! energy is calculated as an integral of squared second derivative
  //! (instead of squared curvature).
  //! \param[in]  curve  parametric curve to calculate strain energy for.
  //! \param[out] result calculated strain energy.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    CalculateStrainEnergy(const Handle(Geom_Curve)& curve,
                          double&                   result);

  //! Calculates approximate bending energy of the passed surface. The strain
  //! energy is calculated as an area integral of squared second derivatives.
  //! \param[in]  surface parametric surface to calculate bending energy for.
  //! \param[out] result  calculated strain energy.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    CalculateBendingEnergy(const Handle(Geom_Surface)& surface,
                           double&                     result);

  //! Reparametrizes the given B-spline curve applying the passed new range
  //! for its knot vector.
  //!
  //! \param[in]  curve    B-spline curve to reparametrize.
  //! \param[in]  newFirst new first parameter value.
  //! \param[in]  newLast  new last parameter value.
  //! \param[in]  toCopy   indicates whether to prepare a copy of the
  //!                      input curve instead of directly modifying it.
  //! \param[out] result   reparametrized curve.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ReparametrizeBSpl(const Handle(Geom2d_Curve)&  curve,
                      const double                 newFirst,
                      const double                 newLast,
                      const bool                   toCopy,
                      Handle(Geom2d_BSplineCurve)& result);

  //! This function is designed to evaluate curvature of the surface along the
  //! the given edge. It takes the tangency direction of the edge's curve for
  //! the passed parameter value and calculates the normal curvature at the
  //! corresponding point on surface.
  //! \param[in]  face face of interest.
  //! \param[in]  edge edge of interest.
  //! \param[in]  t    parameter on the edge's curve to evaluate curvature for.
  //! \param[out] UV   parametric coordinates of the sample point on the host
  //!                  surface.
  //! \param[out] k    evaluated curvature.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    EvaluateAlongCurvature(const TopoDS_Face& face,
                           const TopoDS_Edge& edge,
                           const double       t,
                           gp_Pnt2d&          UV,
                           double&            k);

  //! This function is designed to evaluate curvature of the surface along the
  //! the given edge. It takes the tangency direction of the edge's curve for
  //! the passed parameter value and calculates the normal curvature at the
  //! corresponding point on surface.
  //! \param[in]  face face of interest.
  //! \param[in]  edge edge of interest.
  //! \param[in]  t    parameter on the edge's curve to evaluate curvature for.
  //! \param[out] k    evaluated curvature.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    EvaluateAlongCurvature(const TopoDS_Face& face,
                           const TopoDS_Edge& edge,
                           const double       t,
                           double&            k);

  //! This function is designed to evaluate curvature of the surface along the
  //! the given edge. It takes the tangency direction of the edge's curve for
  //! the middle parameter value and calculates the normal curvature at the
  //! corresponding point on surface.
  //! \param[in]  face face of interest.
  //! \param[in]  edge edge of interest.
  //! \param[out] k    evaluated curvature.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    EvaluateAlongCurvature(const TopoDS_Face& face,
                           const TopoDS_Edge& edge,
                           double&            k);

  //! This function is designed to evaluate curvature of the surface along the
  //! the given edge. It takes the tangency direction of the edge's curve for
  //! the middle parameter value and calculates the normal curvature at the
  //! corresponding point on surface.
  //!
  //! \param[in]  face face of interest.
  //! \param[in]  edge edge of interest.
  //! \param[out] UV   parametric coordinates of the sample point on the host
  //!                  surface.
  //! \param[out] k    evaluated curvature.
  //!
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    EvaluateAlongCurvature(const TopoDS_Face& face,
                           const TopoDS_Edge& edge,
                           gp_Pnt2d&          UV,
                           double&            k);

  //! Calculates curvature in the middle of the passed curve.
  //! \param[in]  curve  curve in question.
  //! \param[out] P      point on curve.
  //! \param[out] T      tangent vector on curve.
  //! \param[out] k      evaluated curvature.
  //! \param[out] r      equivalent curvature radius.
  //! \param[out] center center of curvature.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    CalculateMidCurvature(const Handle(Geom_Curve)& curve,
                          gp_Pnt&                   P,
                          gp_Dir&                   T,
                          double&                   k,
                          double&                   r,
                          gp_Pnt&                   center);

  //! Calculates curvature in the middle of the passed curve.
  //! \param[in]  curve  curve in question.
  //! \param[out] k      evaluated curvature.
  //! \param[out] r      equivalent curvature radius.
  //! \param[out] center center of curvature.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    CalculateMidCurvature(const Handle(Geom_Curve)& curve,
                          double&                   k,
                          double&                   r,
                          gp_Pnt&                   center);

  //! Calculates curvature in the middle of the passed curve.
  //! \param[in]  curve curve in question.
  //! \param[out] k     evaluated curvature.
  //! \param[out] r     equivalent curvature radius.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    CalculateMidCurvature(const Handle(Geom_Curve)& curve,
                          double&                   k,
                          double&                   r);

  //! Integrates the passed function by midpoint (rectangle) rule.
  //! \param[in] F univariate function in question.
  //! \param[in] a lower bound.
  //! \param[in] a upper bound.
  //! \param[in] n number of bins.
  //! \return integral value.
  asiAlgo_EXPORT double
    IntegralRect(math_Function& F,
                 const double   a,
                 const double   b,
                 const int      n);

  //! Attempts to rebuild shape boundaries.
  //! \param[in] shape shape to refine.
  asiAlgo_EXPORT void
    RebuildBounds(TopoDS_Shape& shape);

  //! Gets common (shared) edge between the two passed faces.
  //! \param[in]  F              first face.
  //! \param[in]  G              second face.
  //! \param[out] allCommonEdges all common edges.
  //! \param[in]  hint           vertex to resolve ambiguity.
  //! \return first common edge.
  asiAlgo_EXPORT TopoDS_Edge
    GetCommonEdge(const TopoDS_Shape&         F,
                  const TopoDS_Shape&         G,
                  TopTools_IndexedMapOfShape& allCommonEdges,
                  const TopoDS_Vertex&        hint = TopoDS_Vertex());

  //! Gets common (shared) edge between the two passed faces.
  //! \param[in] F first face.
  //! \param[in] G second face.
  //! \return first common edge.
  asiAlgo_EXPORT TopoDS_Edge
    GetCommonEdge(const TopoDS_Shape& F,
                  const TopoDS_Shape& G);

  //! Gets common (shared) edge between the two passed faces.
  //! \param[in] F    first face.
  //! \param[in] G    second face.
  //! \param[in] hint vertex to resolve ambiguity.
  //! \return first common edge.
  asiAlgo_EXPORT TopoDS_Edge
    GetCommonEdge(const TopoDS_Shape&  F,
                  const TopoDS_Shape&  G,
                  const TopoDS_Vertex& hint);

  //! Gets common edges for the passed face and it vertex.
  //! \param[in]  F     face in question.
  //! \param[in]  V     vertex in question.
  //! \param[out] edges found common edges.
  //! \return false if no common edges exist.
  asiAlgo_EXPORT bool
    GetCommonEdges(const TopoDS_Shape&         F,
                   const TopoDS_Vertex&        V,
                   TopTools_IndexedMapOfShape& edges);

  //! Gets common (shared) vertex between the three passed shapes.
  //! \param[in] F first shape.
  //! \param[in] G second shape.
  //! \param[in] H third shape.
  //! \return first common vertex.
  asiAlgo_EXPORT TopoDS_Vertex
    GetCommonVertex(const TopoDS_Shape& F,
                    const TopoDS_Shape& G,
                    const TopoDS_Shape& H);

  //! Gets common (shared) vertex between the two passed shapes.
  //! \param[in] F first shape.
  //! \param[in] G second shape.
  //! \return first common vertex.
  asiAlgo_EXPORT TopoDS_Vertex
    GetCommonVertex(const TopoDS_Shape& F,
                    const TopoDS_Shape& G);

  //! Gathers all faces which are neighbors to `F` through the edge `E`.
  //! \param[in]  shape master shape.
  //! \param[in]  F     face to find the neighbors for.
  //! \param[in]  E     edge to check neighbors in.
  //! \param[out] M     gathered collection of neighbor faces.
  //! \return false if the passed entities are not related.
  asiAlgo_EXPORT bool
    GetNeighborsThru(const TopoDS_Shape&         shape,
                     const TopoDS_Face&          F,
                     const TopoDS_Edge&          E,
                     TopTools_IndexedMapOfShape& M);

  //! Joins the passed pair of B-spline curves into a single curve.
  //! The resulting curve will have the passed continuity at joints. The input
  //! curves should be properly oriented.
  //! \param[in]  curve1   first B-curve to join.
  //! \param[in]  curve2   second B-curve to join.
  //! \param[in]  order    order of continuity to ensure.
  //! \param[out] result   joint curve.
  //! \param[in]  pregress progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    JoinCurves(Handle(Geom_BSplineCurve)& curve1,
               Handle(Geom_BSplineCurve)& curve2,
               const int                  order,
               Handle(Geom_BSplineCurve)& result,
               ActAPI_ProgressEntry       progress);

  //! Joins the passed chain of B-spline curves into a single curve.
  //! The resulting curve will have the passed continuity at joints. The input
  //! curves should be properly oriented.
  //! \param[in]  curves   input B-curves to join.
  //! \param[in]  order    order of continuity to ensure.
  //! \param[out] result   joint curve.
  //! \param[in]  pregress progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    JoinCurves(std::vector<Handle(Geom_BSplineCurve)>& curves,
               const int                               order,
               Handle(Geom_BSplineCurve)&              result,
               ActAPI_ProgressEntry                    progress);

  //! This method is the analogue of TopExp::MapShapes() except the fact that
  //! it populates a data map instead of an indexed map. Data maps can be
  //! used to store the same shapes under different indices (indexed map will
  //! throw an exception if you try to use its Substitute() method passed
  //! the already contained shape).
  //! \param[in]     S        shape to decompose.
  //! \param[out]    M        populated map of sub-shapes.
  //! \param[in,out] startIdx starting index (1-based indexation is the default).
  //! \param[out]    IM       indexed map to avoid duplicates and maintain
  //!                         equal identifiers in the data map and indexed map.
  asiAlgo_EXPORT void
    MapShapes(const TopoDS_Shape&         S,
              asiAlgo_DataMapOfShape&     M,
              int&                        startIdx,
              TopTools_IndexedMapOfShape& IM);

  //! This method is the analogue of TopExp::MapShapes() except the fact that
  //! it populates a data map instead of an indexed map. Data maps can be
  //! used to store the same shapes under different indices (indexed map will
  //! throw an exception if you try to use its Substitute() method passed
  //! the already contained shape).
  //! \param[in]  S shape to decompose.
  //! \param[out] M populated map of sub-shapes.
  asiAlgo_EXPORT void
    MapShapes(const TopoDS_Shape&     S,
              asiAlgo_DataMapOfShape& M);

  //! This method is the analogue of TopExp::MapShapes() except the fact that
  //! it does not distinguish sub-shapes with different locations.
  //! \param[in]  S shape to decompose.
  //! \param[out] M populated map of sub-shapes.
  asiAlgo_EXPORT void
    MapTShapes(const TopoDS_Shape&         S,
               asiAlgo_IndexedMapOfTShape& M);

  //! This method is the analogue of TopExp::MapShapes() except the fact that
  //! it does not distinguish sub-shapes with different locations.
  //! \param[in]  S shape to decompose.
  //! \param[in]  T sub-shape's type of interest.
  //! \param[out] M populated map of sub-shapes.
  asiAlgo_EXPORT void
    MapTShapes(const TopoDS_Shape&         S,
               const TopAbs_ShapeEnum      T,
               asiAlgo_IndexedMapOfTShape& M);

  //! This method is the analogue of TopExp::MapShapesAndAncestors() except
  //! the fact that it does not distinguish sub-shapes with different
  //! locations.
  //! \param[in]  S  shape to decompose.
  //! \param[in]  TS sub-shape's type of interest.
  //! \param[in]  TA ancestor's type of interest.
  //! \param[out] M  populated map of sub-shapes.
  asiAlgo_EXPORT void
    MapTShapesAndAncestors(const TopoDS_Shape&                        S,
                           const TopAbs_ShapeEnum                     TS,
                           const TopAbs_ShapeEnum                     TA,
                           asiAlgo_IndexedDataMapOfTShapeListOfShape& M);

  //! Checks whether the passed shape has any internal non-identity TopLoc_Location
  //! structures. In some algorithm, such internal locations are prohibited
  //! because these algorithms make an assumption that all boundary elements
  //! of a shape are unique.
  //! \param[in] S  shape to check.
  //! \param[in] ST indicates the shape type to search for internal locations.
  //!               E.g., if TopAbs_FACE is passed, this function will look
  //!               for the located face instances.
  //! \return true if some internal locations exist, false -- otheriwse.
  asiAlgo_EXPORT bool
    HasInternalLocations(const TopoDS_Shape&    S,
                         const TopAbs_ShapeEnum ST = TopAbs_SHAPE);

  //! Often in practice a part is represented with a compound having some
  //! internal instances (i.e., the located occurrences of nested shapes).
  //! Such a data structure is a bad mix of part vs assembly concept. However,
  //! we have to deal with such complex part as they are not something
  //! exotic (to get one, it is enough to read a STEP assembly into a
  //! single TopoDS_Shape).
  //!
  //! The level of the topology graph where the instancing interrupts
  //! corresponds to the level of a real part definition. A part per se can
  //! be a solid, a shell or whatever entity not excluding another compound
  //! without a location. This function is used to extract real parts from
  //! the passed shape. The extracted shapes have no locations.
  //!
  //! \param[in]  S     shape in question.
  //! \param[out] parts extracted real parts (shapes without locations).
  asiAlgo_EXPORT void
    IsolateRealParts(const TopoDS_Shape&   S,
                     TopTools_ListOfShape& parts);

  //! Finds a shape in the given list which contains the passed subshape.
  //! \param[in] parts    list of shapes to check.
  //! \param[in] subshape subshape to find.
  //! \return found shape from the list or null shape if nothing was found.
  asiAlgo_EXPORT TopoDS_Shape
    FindBySubshape(const TopTools_ListOfShape& parts,
                   const TopoDS_Shape&         subshape);

  //! Extracts sub-triangulation from the given source by the passed indices.
  //! Only the mesh elements with the passed indices will be returned in a new
  //! triangulation.
  //! \param[in] mesh  source triangulation.
  //! \param[in] elems elements to keep.
  //! \return sub-triangulation.
  asiAlgo_EXPORT Handle(Poly_Triangulation)
    GetSubMesh(const Handle(Poly_Triangulation)& mesh,
               const TColStd_PackedMapOfInteger& elems);

  //! Computes the average point for the passed point set.
  //! \param[in] pts point set in question.
  //! \return average point.
  asiAlgo_EXPORT gp_XYZ
    ComputeAveragePoint(const std::vector<gp_XYZ>& pts);

  //! Fixed version of BRepTools::OuterWire (see #31172 in the OpenCascade
  //! bugtracker).
  //! \param[in] face face in question.
  //! \return outer wire.
  asiAlgo_EXPORT TopoDS_Wire
    OuterWire(const TopoDS_Face& face);

  //! Computes random inner point on a face.
  //! \param[in]     face the face of interest.
  //! \param[in,out] RNG  the random number generator.
  //! \param[out]    uv   the `(u,v)` coordinates of the random point in
  //!                     the bounded face's parametric space.
  //! \return false if an inner point cannot be queried (e.g., if the face is
  //!         very invalid).
  asiAlgo_EXPORT bool
    GetRandomPoint(const TopoDS_Face&     face,
                   math_BullardGenerator& RNG,
                   gp_Pnt2d&              uv);

  //! Computes local reference frame for the passed face.
  //! \param[in]  face       target face.
  //! \param[in]  alongEdges indicates whether the X direction of the
  //!                        reference frame should follow one of the edges.
  //! \param[out] axes       computed axes.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    GetLocalFrame(const TopoDS_Face& face,
                  const bool         alongEdges,
                  gp_Ax3&            axes);

  //! Calculates border trihedron for the given face.
  //! \param[in]  face     the face to analyze.
  //! \param[in]  edge     the border edge lying on the `face` to calculate
  //!                      the trihedron for.
  //! \param[in]           the parameter along the edge.
  //! \param[out] btri     the border axes.
  //! \param[in]  progress the progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ComputeBorderTrihedron(const TopoDS_Face&       face,
                           const TopoDS_Edge&       edge,
                           const double             t,
                           asiAlgo_BorderTrihedron& btri,
                           ActAPI_ProgressEntry     progress = nullptr);

  //! Calculates border trihedron for the given face.
  //! \param[in]  face     face to analyze.
  //! \param[in]  edge     border edge lying on the `face` to calculate
  //!                      the trihedron for.
  //! \param[out] btri     border axes.
  //! \param[in]  progress progress notifier.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    ComputeBorderTrihedron(const TopoDS_Face&       face,
                           const TopoDS_Edge&       edge,
                           asiAlgo_BorderTrihedron& btri,
                           ActAPI_ProgressEntry     progress = nullptr);

  //! Extract geometry summary for the passed shape.
  asiAlgo_EXPORT void
    GeomSummary(const TopoDS_Shape&  shape,
                asiAlgo_GeomSummary& summary);

  //! Computes the minimal-angle arc for the passed directions.
  //! \param[in] dirs the directions to check.
  //! \param[in] norm the normal vector to determine angle signs.
  //! \return angle of the minimal arc spanned by the passed vectors.
  asiAlgo_EXPORT double
    MinArcAngle(const std::vector<gp_Vec>& dirs,
                const gp_Dir&              norm);

  //! Checks whether the passed two axes are coaxial.
  //! \param[in] a1          the first axis.
  //! \param[in] a2          the second axis.
  //! \param[in] angTolerDeg the angular tolerance (in degrees).
  //! \param[in] linToler    the linear tolerance (in model units).
  //! \return true/false.
  asiAlgo_EXPORT bool
    AreCoaxial(const gp_Ax1& a1,
               const gp_Ax1& a2,
               const double  angTolerDeg,
               const double  linToler);

  //! Checks if the passed curve is equidistant from the given axis, i.e., it
  //! lies on a cylindrical surface.
  //! \param[in]  curve the curve to check.
  //! \param[in]  axis  the known axis to check the passed curve against.
  //! \param[in]  tol   the linear tolerance value to use for the deviation check.
  //! \param[out] r     the extracted radius value.
  //! \return true if the curve lies on a cylinder, false -- otherwise.
  asiAlgo_EXPORT bool
    IsOnCylinder(const Handle(Geom_Curve)& curves,
                 const gp_Ax1&             axis,
                 const double              tol,
                 double&                   r);

  //! Checks if the passed curves are equidistant from the given axis, i.e., they
  //! all lie on a cylindrical surface.
  //! \param[in]  curves the curves to check.
  //! \param[in]  axis   the known axis to check the curves against.
  //! \param[in]  tol    the linear tolerance value to use for the deviation check.
  //! \param[out] r      the extracted radius value.
  //! \return true if the curves lie on a cylinder, false -- otherwise.
  asiAlgo_EXPORT bool
    IsOnCylinder(const std::vector<Handle(Geom_Curve)>& curves,
                 const gp_Ax1&                          axis,
                 const double                           tol,
                 double&                                r);

  //! Checks whether the passed curve `curve` lies on the given cylindrical
  //! surface `cyl`.
  //! \param[in] curve   the curve to check.
  //! \param[in] cyl     the candidate cylinder.
  //! \param[in] tol     the linear tolerance value to use for the deviation check.
  //! \param[in] plotter the imperative plotter to use.
  //! \return true if the curve lies on a cylinder, false -- otherwise.
  asiAlgo_EXPORT bool
    IsOnCylinder(const Handle(Geom_Curve)& curve,
                 const gp_Cylinder&        cyl,
                 const double              tol,
                 ActAPI_PlotterEntry       plotter = nullptr);

  //! Returns a set of points presumably lying on the given face.
  //! \param[in]  face the face in question.
  //! \param[out] pts  the sampled points on a face.
  asiAlgo_EXPORT void
    GetFacePoints(const TopoDS_Face&   face,
                  std::vector<gp_XYZ>& pts);

  //! Checks if the passed face is internal or external, depending
  //! on its norm field orientation.
  //! \param[in] face     the face to check.
  //! \param[in] diameter the presumably known diameter.
  //! \param[in] ax       the presumably known axis.
  //! \return true for internal, false for external.
  asiAlgo_EXPORT bool
    IsInternal(const TopoDS_Face& face,
               const double       diameter,
               const gp_Ax1&      ax);

  //! Checks if the passed face is internal or external, depending
  //! on its norm field orientation.
  //! \param[in] face     the face to check.
  //! \param[in] diameter the presumably known diameter.
  //! \param[in] u        the U coordinate of the probe point on the face of interest.
  //! \param[in] v        the V coordinate of the probe point on the face of interest.
  //! \param[in] ax       the presumably known axis.
  //! \return true for internal, false for external.
  asiAlgo_EXPORT bool
    IsInternal(const TopoDS_Face& face,
               const double       diameter,
               const double       u,
               const double       v,
               const gp_Ax1&      ax);

} // asiAlgo_Utils namespace.

#endif
