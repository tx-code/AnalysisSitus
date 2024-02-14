//-----------------------------------------------------------------------------
// Created on: 20 November 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2015-present, Sergey Slyadnev
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

// Rapidjson includes (should be hidden in cpp)
#if defined USE_RAPIDJSON
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;
typedef rapidjson::Document::Object    t_jsonObject;
#endif

// OS-dependent
#ifdef _WIN32
  #include <Windows.h>
  #include <WNT_Window.hxx>
  #include <WNT_WClass.hxx>
#else
  #include <Xw_Window.hxx>
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>

  // Get rid of X11 macro conflicting with Eigen.
  #ifdef Success
    #undef Success
  #endif
#endif

// Own include
#include <asiAlgo_Utils.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_BuildCoonsSurf.h>
#include <asiAlgo_CheckValidity.h>
#include <asiAlgo_ClassifyPointFace.h>
#include <asiAlgo_ConvertCanonical.h>
#include <asiAlgo_IGES.h>
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_FeatureAttrArea.h>
#include <asiAlgo_FeatureAttrAxialRange.h>
#include <asiAlgo_FeatureAttrUVBounds.h>
#include <asiAlgo_FeatureFaces.h>
#include <asiAlgo_PLY.h>
#include <asiAlgo_RecognizeCanonical.h>
#include <asiAlgo_RelievePointCloud.h>
#include <asiAlgo_Timer.h>

#if defined USE_MOBIUS
  #include <mobius/bspl_UnifyKnots.h>
  #include <mobius/cascade.h>
  #include <mobius/cascade_Triangulation.h>
  #include <mobius/geom_CoonsSurfaceLinear.h>
  #include <mobius/geom_SkinSurface.h>
  #include <mobius/poly_ReadOBJ.h>
  #include <mobius/poly_ReadPLY.h>
  #include <mobius/poly_ReadSTL.h>

  using namespace mobius;
#endif

// OCCT includes
#include <BinTools.hxx>
#include <Bnd_Box.hxx>
#include <BOPAlgo_BOP.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BRep_Builder.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Defeaturing.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_ListIteratorOfListOfStatus.hxx>
#include <BRepCheck_Result.hxx>
#include <BRepCheck_Status.hxx>
#include <BRepCheck_Wire.hxx>
#include <BRepClass_FClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ShapeSet.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GC_MakeCircle.hxx>
#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_Line.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomConvert.hxx>
#include <GeomFill_ConstrainedFilling.hxx>
#include <GeomFill_SimpleBound.hxx>
#include <GeomLProp_CLProps.hxx>
#include <GeomPlate_BuildPlateSurface.hxx>
#include <GeomPlate_MakeApprox.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Vec.hxx>
#include <GProp_GProps.hxx>
#include <math_Matrix.hxx>
#include <NCollection_IncAllocator.hxx>
#include <OSD_Environment.hxx>
#include <OSD_OpenFile.hxx>
#include <Poly_CoherentTriangulation.hxx>
#include <Precision.hxx>
#include <RWStl.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeBuild_Edge.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>

// For offscreen rendering
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Image_AlienPixMap.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_View.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <BRepBuilderAPI_Transform.hxx>

// Eigen includes
#pragma warning(push, 0)
#pragma warning(disable : 4702 4701)
#include <Eigen/Dense>
#pragma warning(default : 4702 4701)
#pragma warning(pop)

// Standard includes
#include <map>

//-----------------------------------------------------------------------------

#undef DUMP_COUT
#undef DUMP_FILES

#define dump_filename_N  "D:\\N_interp_log_OCCT.log"
#define dump_filename_Bx "D:\\Bx_interp_log_OCCT.log"
#define dump_filename_By "D:\\By_interp_log_OCCT.log"
#define dump_filename_Bz "D:\\Bz_interp_log_OCCT.log"

#define BUFSIZE              1000
#define NUM_INTEGRATION_BINS 500
#define ASI_FORTRAN_BUFSIZE  15

#undef COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

// Microsoft's *_s functions are unportable, so we adapt here.
#ifdef __unix
#define fopen_s(pFile, filename, mode) ((*(pFile))=fopen((filename), (mode)))
#endif

//-----------------------------------------------------------------------------

void appendSurfaceDetails(const Handle(Geom_Surface)& surf,
                          TCollection_AsciiString&    msg)
{
  // B-surface.
  if ( surf->IsInstance( STANDARD_TYPE(Geom_BSplineSurface) ) )
  {
    Handle(Geom_BSplineSurface)
      bsurf = Handle(Geom_BSplineSurface)::DownCast(surf);
    //
    msg += "\n\t Degree U: ";
    msg += bsurf->UDegree();
    msg += "\n\t Degree V: ";
    msg += bsurf->VDegree();
    msg += "\n\t Num poles U: ";
    msg += bsurf->NbUPoles();
    msg += "\n\t Num poles V: ";
    msg += bsurf->NbVPoles();
  }

  // Cylindrical surface.
  if ( surf->IsInstance( STANDARD_TYPE(Geom_CylindricalSurface) ) )
  {
    Handle(Geom_CylindricalSurface)
      cylsurf = Handle(Geom_CylindricalSurface)::DownCast(surf);
    //
    msg += "\n\t Radius: ";
    msg += cylsurf->Radius();
  }

  // Conical surface.
  if ( surf->IsInstance( STANDARD_TYPE(Geom_ConicalSurface) ) )
  {
    Handle(Geom_ConicalSurface)
      conesurf = Handle(Geom_ConicalSurface)::DownCast(surf);
    //
    msg += "\n\t Angle: ";
    msg += ( 2*conesurf->SemiAngle() )*180./M_PI;
  }

  // For trimmed surfaces, we extract the basis surface for detalisation.
  if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
  {
    Handle(Geom_RectangularTrimmedSurface)
      tsurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);

    // Proceed with the basis surface.
    Handle(Geom_Surface) basisSurf = tsurf->BasisSurface();

    // Get parametric bounds.
    double uMin, uMax, vMin, vMax;
    basisSurf->Bounds(uMin, uMax, vMin, vMax);

    // Dump.
    msg += "\n\t Basis surface: ";
    msg += basisSurf->DynamicType()->Name();
    msg += "\n\t Min U parameter: ";
    msg += uMin;
    msg += "\n\t Max U parameter: ";
    msg += uMax;
    msg += "\n\t Min V parameter: ";
    msg += vMin;
    msg += "\n\t Max V parameter: ";
    msg += vMax;
    //
    if ( basisSurf->IsKind( STANDARD_TYPE(Geom_BSplineSurface) ) )
    {
      Handle(Geom_BSplineSurface)
        bsplSurf = Handle(Geom_BSplineSurface)::DownCast(basisSurf);
      //
      msg += "\n\t Is U periodic: ";
      msg += bsplSurf->IsUPeriodic();
      msg += "\n\t Is U closed: ";
      msg += bsplSurf->IsUClosed();
      msg += "\n\t Is V periodic: ";
      msg += bsplSurf->IsVPeriodic();
      msg += "\n\t Is V closed: ";
      msg += bsplSurf->IsVClosed();
    }

    // Recursive call for the basis surface.
    appendSurfaceDetails(basisSurf, msg);
  }
}

//-----------------------------------------------------------------------------

#ifdef USE_MOBIUS

void asiAlgo_Utils_DrawSurfPts(const t_ptr<geom_Surface>&     surface,
                               const TCollection_AsciiString& name,
                               ActAPI_PlotterEntry            plotter)
{
  // Sample patch.
  Handle(asiAlgo_BaseCloud<double>) pts = new asiAlgo_BaseCloud<double>;
  //
  const double uMin = surface->GetMinParameter_U();
  const double uMax = surface->GetMaxParameter_U();
  const double vMin = surface->GetMinParameter_V();
  const double vMax = surface->GetMaxParameter_V();
  //
  const double deltaU = (uMax - uMin)/100.;
  const double deltaV = (vMax - vMin)/100.;
  //
  double u = uMin;
  //
  bool stopU = false;
  do
  {
    if ( u > uMax )
      stopU = true;
    else
    {
      double v = vMin;
      //
      bool stopV = false;
      do
      {
        if ( v > vMax )
          stopV = true;
        else
        {
          t_xyz P;
          surface->Eval(u, v, P);

          pts->AddElement( P.X(), P.Y(), P.Z() );

          v += deltaV;
        }
      }
      while ( !stopV );

      u += deltaU;
    }
  }
  while ( !stopU );
  //
  plotter.REDRAW_POINTS(name, pts->GetCoordsArray(), Color_White);
}

#endif

//-----------------------------------------------------------------------------

bool IsASCII(const TCollection_AsciiString& filename)
{
  FILE* FILE;
  fopen_s(&FILE, filename.ToCString(), "rb");
  //
  if ( FILE == nullptr )
    return false;

  bool isAscii = true;
  char buffer[128] = {};
  //
  if ( fread(buffer, 1, 84, FILE) != 84 )
  {
    fclose(FILE);
    return false;
  }

  size_t readLen = fread(buffer, 1, 128, FILE);
  for ( size_t byteIter = 0; byteIter < readLen; ++byteIter )
  {
    if ( (unsigned char) buffer[byteIter] > (unsigned char )'~' )
    {
      isAscii = false;
      break;
    }
  }

  fclose(FILE);
  return isAscii;
}

//-----------------------------------------------------------------------------

//! Univariate function representing the squared second derivative of
//! a parametric curve.
class asiAlgo_CuuSquared : public math_Function
{
public:

  //! ctor.
  //! \param[in] curve parametric curve in question.
  asiAlgo_CuuSquared(const Handle(Geom_Curve)& curve) : math_Function()
  {
    m_curve = curve;
  }

public:

  //! Evaluates the second derivative squared.
  //! \param[in]  u    parameter value.
  //! \param[out] Cuu2 evaluated function.
  //! \return true in case of success, false -- otherwise.
  virtual bool Value(const double u, double& Cuu2)
  {
    gp_Pnt P;
    gp_Vec D1, D2;
    m_curve->D2(u, P, D1, D2);

    Cuu2 = D2*D2;
    return true;
  }

public:

  //! \return curve in question.
  const Handle(Geom_Curve)& GetCurve() const
  {
    return m_curve;
  }

protected:

  Handle(Geom_Curve) m_curve; //!< Curve.

};

namespace
{

  void fixprint(char *s, const int len)
  {
    if ( s[0] == '-' )
    {
      for ( int j = 2; j < len; ++j  )
        s[j-1] = s[j];

      s[len-1] = '\0'; // Zero-trailing.
    }
  }

  char* format_fortran_float(char*    result,
                             unsigned width,
                             double   number)
  {
    int exponent = 0;
    for (; fabs(number) > 1.0; exponent++) number /= 10;

    sprintf( result, "%.*fE%+03d", 7, number, exponent );

    fixprint(result, width);

    return result;
  }

  char* fortranize(const double val,
                   char*        buff)
  {
    return format_fortran_float(buff, ASI_FORTRAN_BUFSIZE, val);
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Str::FileNameAndExtension(const TCollection_AsciiString& fullPath,
                                              TCollection_AsciiString&       name,
                                              TCollection_AsciiString&       extension)
{
  const int EXT_MAX_LEN = 20;
  const int len         = fullPath.Length();
  //
  for ( int extLen = 1; extLen < len && extLen < EXT_MAX_LEN; ++extLen )
  {
    if ( fullPath.Value(len - extLen) == '.' )
    {
      const int nameUpper = len - extLen - 1;
      //
      if ( nameUpper < 1 )
        break;

      name      = fullPath.SubString(1, nameUpper);
      extension = fullPath.SubString(len - extLen + 1, len);
      extension.LowerCase();
      return;
    }
  }

  name = fullPath;
  extension.Clear();
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Str::Split(const std::string&        source_str,
                               const std::string&        delim_str,
                               std::vector<std::string>& result)
{
  // Initialize collection of strings to split
  std::vector<std::string> chunks;
  chunks.push_back(source_str);

  // Split by each delimiter consequently
  for ( size_t delim_idx = 0; delim_idx < delim_str.length(); ++delim_idx )
  {
    std::vector<std::string> new_chunks;
    const char delim = delim_str[delim_idx];

    // Split each chunk
    for ( size_t chunk_idx = 0; chunk_idx < chunks.size(); ++chunk_idx )
    {
      const std::string& source = chunks[chunk_idx];
      std::string::size_type currPos = 0, prevPos = 0;
      while ( (currPos = source.find(delim, prevPos) ) != std::string::npos )
      {
        std::string item = source.substr(prevPos, currPos - prevPos);
        if ( item.size() > 0 )
        {
          new_chunks.push_back(item);
        }
        prevPos = currPos + 1;
      }
      new_chunks.push_back( source.substr(prevPos) );
    }

    // Set new collection of chunks for splitting by the next delimiter
    chunks = new_chunks;
  }

  // Set result
  result = chunks;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Str::Split(const TCollection_AsciiString&        source_str,
                               const char*                           delim,
                               std::vector<TCollection_AsciiString>& result)
{
  int tt = 1;
  TCollection_AsciiString chunk;
  bool stop = false;

  do
  {
    chunk = source_str.Token(delim, tt++);
    //
    if ( !chunk.IsEmpty() )
      result.push_back(chunk);
    else
      stop = true;
  }
  while ( !stop );
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Str::ReplaceAll(std::string&       str,
                                    const std::string& what,
                                    const std::string& with)
{
  for ( size_t pos = 0; ; pos += with.length() )
  {
    pos = str.find(what, pos); // Locate the substring to replace
    if ( pos == std::string::npos )
      break; // Not found

    // Replace by erasing and inserting
    str.erase  ( pos, what.length() );
    str.insert ( pos, with );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Str::ReplaceAll(TCollection_AsciiString&       str,
                                    const TCollection_AsciiString& what,
                                    const TCollection_AsciiString& with)
{
  std::string stdStr  ( str.ToCString() );
  std::string stdWhat ( what.ToCString() );
  std::string stdWith ( with.ToCString() );

  ReplaceAll(stdStr, stdWhat, stdWith);

  str = stdStr.c_str();
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Str::SubStr(const std::string& source,
                                       const int          idx_F,
                                       const int          length)
{
  return source.substr(idx_F, length);
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Str::Quoted(const char* strIN)
{
  return Quoted( std::string(strIN) );
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Str::Quoted(const std::string& strIN)
{
  if ( !strIN.length() )
    return "\"\"";

  char cf = strIN.at(0);
  char ce = strIN.at(strIN.length() - 1);

  if ( (cf == *asiAlgo_QuoteStr) && (ce == *asiAlgo_QuoteStr) )
    return strIN;

  std::string strOUT;

  if ( cf != *asiAlgo_QuoteStr )
    strOUT += asiAlgo_QuoteStr;
  //
  strOUT += strIN;
  //
  if ( ce != *asiAlgo_QuoteStr )
    strOUT += asiAlgo_QuoteStr;

  return strOUT;
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Str::Slashed(const char* strIN)
{
  return Slashed( std::string(strIN) );
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Str::Slashed(const std::string& strIN)
{
  if ( !strIN.length() )
    return strIN;

  char c = strIN.at(strIN.length() - 1);
  if ( c == *asiAlgo_SlashStr )
    return strIN;

  std::string strOUT(strIN);
  strOUT.append(asiAlgo_SlashStr);
  return strOUT;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::Str::Slashed(const TCollection_AsciiString& strIN)
{
  std::string str( strIN.ToCString() );
  //
  if ( !str.length() )
    return str.c_str();

  char c = str.at(str.length() - 1);
  if ( c == *asiAlgo_SlashStr )
    return str.c_str();

  std::string strOUT(str);
  strOUT.append(asiAlgo_SlashStr);
  return strOUT.c_str();
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Str::IsNumber(const std::string& str)
{
  char* cnv;
  strtod(str.c_str(), &cnv);
  return cnv != str.data();
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Str::FileDirectory(const std::string& filename)
{
  // Split path to directory names.
  std::vector<std::string> chunks;
  Split(filename, "\\/", chunks);

  std::string resPath;

  // Add leading slash for the unix systems.
  if (filename[0] == '/') // unix
    resPath += "/";

  // Compose full path.
  for (size_t k = 0; k < chunks.size() - 1; ++k)
  {
    resPath += chunks[k];
    resPath += "/";
  }

  return resPath;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::Str::FileDirectory(const TCollection_AsciiString& filename)
{
  return FileDirectory(std::string(filename.ToCString())).c_str();
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Str::LastDirname(const std::string& filename)
{
  std::vector<std::string> chunks;
  Split(filename, "\\/", chunks);

  if ( chunks.size() > 1 )
    return chunks[chunks.size() - 2];

  return filename;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::Str::BaseFilename(const TCollection_AsciiString& filename,
                                   const bool                     doKeepExt)
{
  std::string
    basefilename = BaseFilename(std::string( filename.ToCString() ), doKeepExt);

  return basefilename.c_str();
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Str::BaseFilename(const std::string& filename,
                                   const bool         doKeepExt)
{
  // Split path to directory names.
  std::vector<std::string> chunks;
  Split(filename, "\\/", chunks);

  // Get base filename and change its extension.
  std::string baseFn = chunks[chunks.size() - 1];
  std::vector<std::string> baseFnChunks;
  Split(baseFn, ".", baseFnChunks);

  if ( baseFnChunks.size() == 1 )
  {
    // No extension
    return baseFnChunks[0];
  }
  //
  baseFn = "";
  for ( size_t k = 0; k < baseFnChunks.size() - 1; ++k )
  {
    baseFn += baseFnChunks[k]; // Base filename without extension.

    if ( (k == baseFnChunks.size() - 2) && doKeepExt )
      baseFn += ".";
  }
  if ( doKeepExt )
    baseFn += baseFnChunks[baseFnChunks.size() - 1];

  return baseFn;
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Str::RefFilename(const std::string& filename,
                                  const std::string& refExt)
{
  // Split path to directory names.
  std::vector<std::string> chunks;
  Split(filename, "\\/", chunks);

  // Get base filename and change its extension.
  std::string baseFn = chunks[chunks.size() - 1];
  std::vector<std::string> baseFnChunks;
  Split(baseFn, ".", baseFnChunks);
  //
  baseFn = "";
  for ( size_t k = 0; k < baseFnChunks.size() - 1; ++k )
  {
    baseFn += baseFnChunks[k]; // Base filename without extension.
    baseFn += ".";
  }
  baseFn += refExt;

  std::string resFilename;

  // Add leading slash for the unix systems.
  if ( filename[0] == '/' ) // unix
    resFilename += "/";

  // Compose full name.
  for ( size_t k = 0; k < chunks.size() - 1; ++k )
  {
    resFilename += chunks[k];
    resFilename += "/";
  }
  //
  resFilename += baseFn;

  return resFilename;
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Str::AmendedFilename(const std::string& filename,
                                      const std::string& suffix)
{
  // Split path to directory names.
  std::vector<std::string> chunks;
  Split(filename, "\\/", chunks);

  // Get base filename and take its extension.
  std::string baseFn = chunks[chunks.size() - 1];

  std::size_t dotPos = baseFn.find_last_of(".");
  baseFn = baseFn.insert(dotPos, suffix);

  std::string resFilename;

  // Add leading slash for the unix systems.
  if ( filename[0] == '/' ) // unix
    resFilename += "/";

  // Compose full name.
  for ( size_t k = 0; k < chunks.size() - 1; ++k )
  {
    resFilename += chunks[k];
    resFilename += "/";
  }
  //
  resFilename += baseFn;

  return resFilename;
}

//-----------------------------------------------------------------------------

//! Returns value of ASI_TEST_DATA environment variable. This variable is used to
//! refer to the directory containing all data files playing as inputs for
//! unit tests.
//! \return value of ASI_TEST_DATA variable.
std::string asiAlgo_Utils::Env::AsiTestData()
{
  return GetVariable(ASI_TEST_DATA);
}

//-----------------------------------------------------------------------------

//! Returns value of ASI_TEST_DUMPING environment variable. This variable is
//! used to refer to the directory containing all temporary files utilized by
//! unit tests.
//! \return value of ASI_TEST_DUMPING variable.
std::string asiAlgo_Utils::Env::AsiTestDumping()
{
  return GetVariable(ASI_TEST_DUMPING);
}

//-----------------------------------------------------------------------------

//! Returns value of ASI_TEST_SCRIPTS environment variable. This variable is used
//! to refer to the directory containing all Tcl scripts.
//! \return value of ASI_TEST_SCRIPTS variable.
std::string asiAlgo_Utils::Env::AsiTestScripts()
{
  return GetVariable(ASI_TEST_SCRIPTS);
}

//-----------------------------------------------------------------------------

//! Returns value of ASI_DOCS environment variable.
//! \return value of ASI_DOCS variable.
std::string asiAlgo_Utils::Env::AsiDocs()
{
  return GetVariable(ASI_DOCS);
}

//-----------------------------------------------------------------------------

//! Returns value of the environment variable with the passed name.
//! \param varName [in] variable name.
//! \return value of the variable.
std::string asiAlgo_Utils::Env::GetVariable(const char* varName)
{
  return OSD_Environment(varName).Value().ToCString();
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Json::ReadFeature(void*            pJsonBlock,
                                      asiAlgo_Feature& feature)
{
#if defined USE_RAPIDJSON
  t_jsonArray*
    jsonBlock = reinterpret_cast<t_jsonArray*>(pJsonBlock);

  for ( t_jsonValue::ValueIterator vit = jsonBlock->Begin();
        vit != jsonBlock->End(); vit++ )
  {
    feature.Add( vit->GetInt() );
  }
#else
  (void) pJsonBlock;
  (void) feature;
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Json::ReadFeatures(void*                         pJsonBlock,
                                       std::vector<asiAlgo_Feature>& features)
{
#if defined USE_RAPIDJSON
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonBlock);

  for ( t_jsonValue::ValueIterator it = pJsonObj->Begin();
        it != pJsonObj->End(); it++ )
  {
    asiAlgo_Feature feature;
    t_jsonArray arr = it->GetArray();
    ReadFeature(&arr, feature);
    features.push_back(feature);
  }
#else
  (void) pJsonBlock;
  (void) features;
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Json::ReadPair(void*                                pJsonBlock,
                                   tl::optional< std::pair<int, int> >& pair)
{
#if defined USE_RAPIDJSON
  pair = tl::make_optional< std::pair<int, int> >();

  t_jsonArray*
    jsonBlock = reinterpret_cast<t_jsonArray*>(pJsonBlock);

  int idx = 0;

  for ( t_jsonValue::ValueIterator vit = jsonBlock->Begin();
        vit != jsonBlock->End(); vit++, idx++ )
  {
    if ( idx == 0 )
      pair->first = vit->GetInt();
    else if ( idx == 1 )
      pair->second = vit->GetInt();
    else
      break;
  }
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Json::ReadCoords(void*   pJsonBlock,
                                     gp_XYZ& coords)
{
#if defined USE_RAPIDJSON
  t_jsonArray*
    pJsonArr = reinterpret_cast<t_jsonArray*>(pJsonBlock);

  int i = 1;
  for ( t_jsonArray::ValueIterator it = pJsonArr->Begin();
        it != pJsonArr->End(); it++ )
  {
    coords.SetCoord( i++, it->GetDouble() );
  }
#endif
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Json::FromFeature(const asiAlgo_Feature& feature)
{
  std::stringstream out;

  out << "[";
  int i = 0;
  for ( asiAlgo_Feature::Iterator fit(feature); fit.More(); fit.Next(), ++i )
  {
    out << fit.Key();
    if ( i != feature.Extent() - 1 )
    {
      out << ", ";
    }
  }
  out << "]";

  return out.str();
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Json::FromDirAsTuple(const gp_Dir& dir)
{
  std::stringstream out;

  out << "[";
  out << dir.X();
  out << ", " << dir.Y();
  out << ", " << dir.Z();
  out << "]";

  return out.str();
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Json::FromPointAsTuple(const gp_Pnt& pnt)
{
  std::stringstream out;

  out << "[";
  out << pnt.X();
  out << ", " << pnt.Y();
  out << ", " << pnt.Z();
  out << "]";

  return out.str();
}

//-----------------------------------------------------------------------------

std::string
  asiAlgo_Utils::Json::FromPair(const tl::optional< std::pair<int, int> >& pair)
{
  std::stringstream out;
  //
  if ( pair.has_value() )
    out << "[" << pair->first << ", " << pair->second << "]";
  else
    out << "null";

  return out.str();
}

//-----------------------------------------------------------------------------

// https://stackoverflow.com/a/33799784/2058753
std::string
  asiAlgo_Utils::Json::EscapeJson(const std::string& s)
{
  std::ostringstream o;
  for ( auto c = s.cbegin(); c != s.cend(); c++ )
  {
    switch ( *c )
    {
      case '"':  o << "\\\""; break;
      case '\\': o << "\\\\"; break;
      case '\b': o << "\\b";  break;
      case '\f': o << "\\f";  break;
      case '\n': o << "\\n";  break;
      case '\r': o << "\\r";  break;
      case '\t': o << "\\t";  break;
      default:
      {
        if ( '\x00' <= *c && *c <= '\x1f' )
        {
          o << "\\u"
            << std::hex << std::setw(4) << std::setfill('0') << (int)* c;
        }
        else
        {
          o << *c;
        }
      }
    }
  }

  return o.str();
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Json::ReadVector(void*             pJsonBlock,
                                     std::vector<int>& vector)
{
#if defined USE_RAPIDJSON
  t_jsonArray*
    jsonBlock = reinterpret_cast<t_jsonArray*>(pJsonBlock);

  for ( t_jsonValue::ValueIterator vit = jsonBlock->Begin();
        vit != jsonBlock->End(); vit++ )
  {
    vector.push_back( vit->GetInt() );
  }
#else
  (void) pJsonBlock;
  (void) vector;
#endif
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::Json::FromVector(const std::vector<int>& v)
{
  std::stringstream out;

  out << "[";
  int i = 0;
  for ( const auto& value : v )
  {
    out << value;
    if ( i != int( v.size() ) - 1 )
    {
      out << ", ";
    }

    ++i;
  }
  out << "]";

  return out.str();
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Verify::CompareOccurrences(const std::vector<double>& V1,
                                               const std::vector<double>& V2,
                                               const double               prec)
{
  if ( V1.size() != V2.size() )
    return false;

  std::map<double, int> uniques1, uniques2;

  // Unique values in the first vector.
  std::for_each(V1.begin(), V1.end(), [&](const double val) {
    bool found = false;
    for ( auto u : uniques1 )
    {
      if ( Abs(u.first - val) < prec )
      {
        uniques1[u.first]++;
        found = true;
        break;
      }
    }
    //
    if ( !found ) uniques1[val]++;
  });

  // Unique values in the second vector.
  std::for_each(V2.begin(), V2.end(), [&](const double val) {
    bool found = false;
    for ( auto u : uniques2 )
    {
      if ( Abs(u.first - val) < prec )
      {
        uniques2[u.first]++;
        found = true;
        break;
      }
    }
    //
    if ( !found ) uniques2[val]++;
  });

  // Compare maps.
  for ( auto u1 : uniques1 )
  {
    bool found = false;
    for ( auto u2 : uniques2 )
    {
      if ( (u1.first - u2.first) < prec && (u1.second == u2.second) )
      {
        found = true;
        break;
      }
    }

    if ( !found )
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Verify::AreEqual(const tl::optional<bool>& a,
                                     const tl::optional<bool>& b)
{
  if ( !a.has_value() && !b.has_value() )
    return true;

  if ( !a.has_value() && b.has_value() )
    return false;

  if ( a.has_value() && !b.has_value() )
    return false;

  if ( *a != *b )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Verify::AreEqual(const tl::optional<double>& a,
                                     const tl::optional<double>& b,
                                     const double                prec)
{
  if ( !a.has_value() && !b.has_value() )
    return true;

  if ( !a.has_value() && b.has_value() )
    return false;

  if ( a.has_value() && !b.has_value() )
    return false;

  if ( Abs(*a - *b) > prec )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Verify::AreEqualAxes(const tl::optional<gp_Ax1>& a,
                                         const tl::optional<gp_Ax1>& b,
                                         const double                angTolerDeg)
{
  if ( !a.has_value() && !b.has_value() )
    return true;

  if ( !a.has_value() && b.has_value() )
    return false;

  if ( a.has_value() && !b.has_value() )
    return false;

  if ( a->IsParallel(*b, angTolerDeg) || a->IsOpposite(*b, angTolerDeg) )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Verify::AreEqualDirs(const tl::optional<gp_Dir>& a,
                                         const tl::optional<gp_Dir>& b,
                                         const double                angTolerDeg)
{
  if ( !a.has_value() && !b.has_value() )
    return true;

  if ( !a.has_value() && b.has_value() )
    return false;

  if ( a.has_value() && !b.has_value() )
    return false;

  const double angDeg = a->Angle(*b)*180./M_PI;

  if ( Abs(angDeg) < angTolerDeg )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::FaceGeometryName(const TopoDS_Face& face)
{
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
  return SurfaceName(surf);
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::DirectionName(const gp_Dir& dir)
{
  if ( dir.IsEqual( gp::DX(), Precision::Angular() ) )
  {
    return "X+";
  }

  if ( dir.IsEqual( -gp::DX(), Precision::Angular() ) )
  {
    return "X-";
  }

  if ( dir.IsEqual( gp::DY(), Precision::Angular() ) )
  {
    return "Y+";
  }

  if ( dir.IsEqual( -gp::DY(), Precision::Angular() ) )
  {
    return "Y-";
  }

  if ( dir.IsEqual( gp::DZ(), Precision::Angular() ) )
  {
    return "Z+";
  }

  if ( dir.IsEqual( -gp::DZ(), Precision::Angular() ) )
  {
    return "Z-";
  }

  char buff[ASI_FORTRAN_BUFSIZE];
  std::string xStr = ::fortranize(dir.X(), buff);
  std::string yStr = ::fortranize(dir.Y(), buff);
  std::string zStr = ::fortranize(dir.Z(), buff);
  //
  std::string res = xStr;
  res += "_";
  res += yStr;
  res += "_";
  res += zStr;

  return res;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::NamedShapeToString(const TopoDS_Shape&           subShape,
                                    const int                     pedigreeId,
                                    const int                     globalId,
                                    const Handle(asiAlgo_Naming)& naming)
{
  TCollection_AsciiString msg("Sub-shape: ");
  msg += ShapeTypeStr(subShape).c_str();
  msg += "\n\t ID: ";        msg += pedigreeId;
  msg += "\n\t Global ID: "; msg += globalId;
  msg += "\n\t Address: ";   msg += ShapeAddr(subShape).c_str();
  //
  if ( !naming.IsNull() )
  {
    msg += "\n\t Name: ";

    TCollection_AsciiString label;
    naming->FindName(subShape, label);
    //
    if ( !label.IsEmpty() )
      msg += label;
    else
      msg += "<empty>";
  }
  //
  if ( subShape.ShapeType() == TopAbs_EDGE )
  {
    const TopoDS_Edge& edge = TopoDS::Edge(subShape);

    // Get host curve.
    double f, l;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
    //
    if ( curve.IsNull() )
      msg += "\n\t Curve: null";
    else
    {
      msg += "\n\t Curve: ";
      msg += curve->DynamicType()->Name();
      msg += "\n\t Min parameter: ";
      msg += curve->FirstParameter();
      msg += "\n\t Max parameter: ";
      msg += curve->LastParameter();

      if ( curve->IsInstance( STANDARD_TYPE(Geom_BSplineCurve) ) )
      {
        Handle(Geom_BSplineCurve)
          bcurve = Handle(Geom_BSplineCurve)::DownCast(curve);
        //
        msg += "\n\t Degree: ";
        msg += bcurve->Degree();
        msg += "\n\t Num poles: ";
        msg += bcurve->NbPoles();
      }
    }
  }
  //
  else if ( subShape.ShapeType() == TopAbs_FACE )
  {
    const TopoDS_Face& face = TopoDS::Face(subShape);

    // Get host surface.
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

    // Get parametric bounds.
    double uMin, uMax, vMin, vMax;
    surf->Bounds(uMin, uMax, vMin, vMax);

    // Dump.
    msg += "\n\t Surface: ";
    msg += surf->DynamicType()->Name();
    msg += "\n\t Natural bounds: ";
    msg += (BRep_Tool::NaturalRestriction(face) ? "true" : "false");
    msg += "\n\t Min U parameter: ";
    msg += uMin;
    msg += "\n\t Max U parameter: ";
    msg += uMax;
    msg += "\n\t Min V parameter: ";
    msg += vMin;
    msg += "\n\t Max V parameter: ";
    msg += vMax;
    //
    appendSurfaceDetails(surf, msg);
  }

  return msg;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::OrientationToString(const TopoDS_Shape& shape)
{
  return OrientationToString( shape.Orientation() );
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::OrientationToString(const TopAbs_Orientation ori)
{
  TCollection_AsciiString oriStr;

  // Check orientation.
  if ( ori == TopAbs_FORWARD )
    oriStr = "forward";
  else if ( ori == TopAbs_REVERSED )
    oriStr = "reversed";
  else if ( ori == TopAbs_INTERNAL )
    oriStr = "internal";
  else if ( ori == TopAbs_EXTERNAL )
    oriStr = "external";

  return oriStr;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::ContinuityToString(const GeomAbs_Shape cont)
{
  TCollection_AsciiString contStr;

  // Check continuity.
  switch ( cont )
  {
    case GeomAbs_C0 : contStr = "C0"; break;
    case GeomAbs_C1 : contStr = "C1"; break;
    case GeomAbs_C2 : contStr = "C2"; break;
    case GeomAbs_C3 : contStr = "C3"; break;
    case GeomAbs_CN : contStr = "CN"; break;
    case GeomAbs_G1 : contStr = "G1"; break;
    case GeomAbs_G2 : contStr = "G2"; break;
    default: break;
  }

  return contStr;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::
  ConnectEdgesToWires(const std::vector<TopoDS_Edge>& edges,
                      const bool                      isShared,
                      std::vector<TopoDS_Wire>&       contours,
                      const double                    tolerance)
{
  Handle(TopTools_HSequenceOfShape) edgesSeq = new TopTools_HSequenceOfShape();
  //
  std::vector<TopoDS_Edge>::const_iterator itE = edges.cbegin();
  for ( ; itE != edges.cend(); ++itE )
  {
    if ( BRep_Tool::Degenerated(*itE) )
      continue;
    //
    edgesSeq->Append(*itE);
  }
  //
  return ConnectEdgesToWires(isShared, edgesSeq, contours, tolerance);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::
  ConnectEdgesToWires(const bool                         isShared,
                      Handle(TopTools_HSequenceOfShape)& edges,
                      std::vector<TopoDS_Wire>&          contours,
                      const double                       tolerance)
{
  if ( edges.IsNull() )
    return false;
  //
  // Compose border wires from the naked edges.
  Handle(TopTools_HSequenceOfShape) borderWires;
  ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance, isShared, borderWires);
  //
  if ( borderWires.IsNull() )
    return false;
  //
  for ( TopTools_HSequenceOfShape::Iterator wit(*borderWires); wit.More(); wit.Next() )
  {
    TopoDS_Wire wire = TopoDS::Wire( wit.Value() );
    contours.push_back(wire);
  }
  //
  return true;
}

//-----------------------------------------------------------------------------

TCollection_AsciiString
  asiAlgo_Utils::LocationToString(const TopLoc_Location& loc)
{
  const gp_Trsf& T      = loc.Transformation();
  gp_Mat         T_roto = T.VectorialPart();
  const gp_XYZ&  T_move = T.TranslationPart();
  gp_TrsfForm    T_form = T.Form();

  TCollection_AsciiString result;
  result += "\n---\nTransformation: ";
  if ( T_form == gp_Identity )
    result += "identity";
  else if ( T_form == gp_Rotation )
    result += "rotation";
  else if ( T_form == gp_Translation )
    result += "translation";
  else if ( T_form == gp_PntMirror )
    result += "point mirror (central symmetry)";
  else if ( T_form == gp_Ax1Mirror )
    result += "axis mirror (rotational symmetry)";
  else if ( T_form == gp_Ax2Mirror )
    result += "plane mirror (bilateral symmetry)";
  else if ( T_form == gp_Scale )
    result += "scaling";
  else if ( T_form == gp_CompoundTrsf )
    result += "combination of orthogonal transformations";
  else
    result += "non-orthogonal transformation";
  //
  if ( T_form != gp_Identity )
  {
    result += "\n---\n";
    result += T_roto(1, 1); result += " "; result += T_roto(1, 2); result += " "; result += T_roto(1, 3); result += "\n";
    result += T_roto(2, 1); result += " "; result += T_roto(2, 2); result += " "; result += T_roto(2, 3); result += "\n";
    result += T_roto(3, 1); result += " "; result += T_roto(3, 2); result += " "; result += T_roto(3, 3);
    result += "\n---\n";
    result += T_move.X(); result += " "; result += T_move.Y(); result += " "; result += T_move.Z();
  }

  return result;
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::CurveName(const Handle(Geom_Curve)& curve)
{
  if ( curve->IsInstance( STANDARD_TYPE(Geom_Line) ) )
    return "line";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_Circle) ) )
    return "circle";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_BezierCurve) ) )
    return "bezier";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_BSplineCurve) ) )
    return "b-curve";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_Ellipse) ) )
    return "ellipse";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_Parabola) ) )
    return "parabola";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_Hyperbola) ) )
    return "hyperbola";
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_OffsetCurve) ) )
  {
    Handle(Geom_OffsetCurve) oc = Handle(Geom_OffsetCurve)::DownCast(curve);
    return std::string("offset on ") + CurveName( oc->BasisCurve() );
  }
  //
  if ( curve->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
  {
    Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(curve);
    return std::string("trimmed on ") + CurveName( tc->BasisCurve() );
  }

  return "unknown";
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::SurfaceName(const Handle(Geom_Surface)& surf)
{
  if ( surf->IsInstance( STANDARD_TYPE(Geom_Plane) ) )
    return "plane";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_CylindricalSurface) ) )
    return "cylinder";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_BezierSurface) ) )
    return "bezier";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_BSplineSurface) ) )
    return "b-surface";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_ConicalSurface) ) )
    return "cone";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_OffsetSurface) ) )
  {
    Handle(Geom_OffsetSurface) os = Handle(Geom_OffsetSurface)::DownCast(surf);
    return std::string("offset on ") + SurfaceName( os->BasisSurface() );
  }
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
  {
    Handle(Geom_RectangularTrimmedSurface) ts = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);
    return std::string("rect trimmed on ") + SurfaceName( ts->BasisSurface() );
  }
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_SphericalSurface) ) )
    return "sphere";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion) ) )
    return "extrusion";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_SurfaceOfRevolution) ) )
    return "revolution";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_SweptSurface) ) )
    return "sweep";
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_ToroidalSurface) ) )
    return "torus";

  return "unknown";
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::ShapeTypeStr(const TopAbs_ShapeEnum& shapeType)
{
  std::string name;
  if ( shapeType == TopAbs_COMPOUND )
    name = "COMPOUND";
  else if ( shapeType == TopAbs_COMPSOLID )
    name = "COMPSOLID";
  else if ( shapeType == TopAbs_SOLID )
    name = "SOLID";
  else if ( shapeType == TopAbs_SHELL )
    name = "SHELL";
  else if ( shapeType == TopAbs_FACE )
    name = "FACE";
  else if ( shapeType == TopAbs_WIRE )
    name = "WIRE";
  else if ( shapeType == TopAbs_EDGE )
    name = "EDGE";
  else if ( shapeType == TopAbs_VERTEX )
    name = "VERTEX";
  else
    name = "SHAPE";

  return name;
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::ShapeTypeStr(const TopoDS_Shape& shape)
{
  return ShapeTypeStr( shape.ShapeType() );
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::ShapeAddrWithPrefix(const TopoDS_Shape& shape)
{
  std::string addr_str = ShapeAddr    ( shape.TShape() );
  std::string prefix   = ShapeTypeStr ( shape );

  // Notice extra spacing for better visualization
  return prefix + " [" + addr_str + "]";
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::ShapeAddr(const Handle(TopoDS_TShape)& tshape)
{
  std::string addr_str;
  std::ostringstream ost;
  ost << tshape.get();
  addr_str = ost.str();

  size_t pos = 0;
  while ( addr_str[pos] == '0' )
    pos++;

  if ( pos )
    addr_str = Str::SubStr( addr_str, (int) pos, (int) (addr_str.size() - pos) );

  return addr_str;
}

//-----------------------------------------------------------------------------

std::string asiAlgo_Utils::ShapeAddr(const TopoDS_Shape& shape)
{
  return ShapeAddr( shape.TShape() );
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsPlanar(const TopoDS_Face& face)
{
  return IsTypeOf<Geom_Plane>(face);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsPlanar(const TopoDS_Face&  face,
                             Handle(Geom_Plane)& plane)
{
  if ( !IsTypeOf<Geom_Plane>(face) )
    return false;

  plane = Handle(Geom_Plane)::DownCast( BRep_Tool::Surface(face) );
  return true;
}


//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCylindrical(const TopoDS_Face& face)
{
  gp_Ax1 ax;
  double radius, angle_min, angle_max, h_min, h_max;
  return IsCylindrical(face, radius, ax, false, angle_min, angle_max, h_min, h_max);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCylindrical(const TopoDS_Face& face,
                                  gp_Cylinder&       cyl)
{
  double radius;
  gp_Ax1 ax;
  double angle_min, angle_max, h_min, h_max;

  if ( IsCylindrical(face, radius, ax, false, angle_min, angle_max, h_min, h_max) )
  {
    cyl = gp_Cylinder(gp_Ax3( ax.Location(), ax.Direction() ), radius);
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCylindrical(const TopoDS_Face& face,
                                  gp_Ax1&            ax)
{
  double radius, angle_min, angle_max, h_min, h_max;
  return IsCylindrical(face, radius, ax, false, angle_min, angle_max, h_min, h_max);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCylindrical(const TopoDS_Face& face,
                                  double&            radius)
{
  gp_Ax1 ax;
  double angle_min, angle_max, h_min, h_max;
  return IsCylindrical(face, radius, ax, false, angle_min, angle_max, h_min, h_max);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCylindrical(const TopoDS_Face& face,
                                  double&            radius,
                                  gp_Ax1&            ax,
                                  double&            angle_min,
                                  double&            angle_max)
{
  double h_min, h_max;
  return IsCylindrical(face, radius, ax, true, angle_min, angle_max, h_min, h_max);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCylindrical(const TopoDS_Face& face,
                                  double&            radius,
                                  gp_Ax1&            ax,
                                  const bool         computeBounds,
                                  double&            angle_min,
                                  double&            angle_max,
                                  double&            h_min,
                                  double&            h_max)
{
  bool isCylindrical = false;
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
  Handle(Geom_CylindricalSurface) cylsurf;
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_CylindricalSurface) ) )
  {
    isCylindrical = true;

    // Get host surface.
    cylsurf = Handle(Geom_CylindricalSurface)::DownCast(surf);
  }
  else if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
  {
    Handle(Geom_RectangularTrimmedSurface)
      RT = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);

    if ( RT->BasisSurface()->IsInstance( STANDARD_TYPE(Geom_CylindricalSurface) ) )
    {
      isCylindrical = true;

      // Get host surface.
      cylsurf = Handle(Geom_CylindricalSurface)::DownCast( RT->BasisSurface() );
    }
  }

  if ( isCylindrical && !cylsurf.IsNull() )
  {
    radius = cylsurf->Radius();
    ax     = cylsurf->Axis();

    if ( computeBounds )
      BRepTools::UVBounds(face, angle_min, angle_max, h_min, h_max);
  }

  return isCylindrical;
}
//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsConical(const TopoDS_Face& face)
{
  gp_Ax1 ax;
  return IsConical(face, ax);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsConical(const TopoDS_Face& face,
                              gp_Ax1&            ax)
{
  Handle(Geom_ConicalSurface) surf;
  //
  if ( IsTypeOf<Geom_ConicalSurface>(face, surf) )
  {
    ax = surf->Axis();
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsConical(const TopoDS_Face& face,
                              gp_Ax1&            ax,
                              const bool         computeBounds,
                              double&            angle_min,
                              double&            angle_max,
                              double&            h_min,
                              double&            h_max,
                              double&            minRadius,
                              double&            maxRadius)
{
  bool isConical = false;
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
  Handle(Geom_ConicalSurface) conesurf;
  //
  if ( surf->IsInstance( STANDARD_TYPE(Geom_ConicalSurface) ) )
  {
    isConical = true;

    // Get host surface.
    conesurf = Handle(Geom_ConicalSurface)::DownCast(surf);
  }
  else if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
  {
    Handle(Geom_RectangularTrimmedSurface)
      RT = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);

    if ( RT->BasisSurface()->IsInstance( STANDARD_TYPE(Geom_ConicalSurface) ) )
    {
      isConical = true;

      // Get host surface.
      conesurf = Handle(Geom_ConicalSurface)::DownCast( RT->BasisSurface() );
    }
  }

  if ( isConical && !conesurf.IsNull() )
  {
    ax = conesurf->Axis();

    if ( computeBounds )
    {
      BRepTools::UVBounds(face, angle_min, angle_max, h_min, h_max);

      Handle(Geom_Circle) isos[2] = { Handle(Geom_Circle)::DownCast( conesurf->VIso(h_min) ),
                                      Handle(Geom_Circle)::DownCast( conesurf->VIso(h_max) ) };

      const double radii[2] = { isos[0]->Radius(), isos[1]->Radius() };

      // Select the extremity radii.
      minRadius = ( (radii[0] < radii[1]) ? radii[0] : radii[1] );
      maxRadius = ( (radii[0] > radii[1]) ? radii[0] : radii[1] );
    }
  }

  return isConical;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsSpherical(const TopoDS_Face& face)
{
  Handle(Geom_SphericalSurface) surf;
  //
  if ( IsTypeOf<Geom_SphericalSurface>(face, surf) )
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsToroidal(const TopoDS_Face& face)
{
  Handle(Geom_ToroidalSurface) surf;
  //
  if ( IsTypeOf<Geom_ToroidalSurface>(face, surf) )
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsToroidal(const TopoDS_Face& face,
                               double&            rmax)
{
  double rmin = 0.;
  gp_Ax1 ax;
  //
  return IsToroidal(face, rmin, rmax, ax);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsToroidal(const TopoDS_Face& face,
                               gp_Ax1&            ax)
{
  double rmin = 0.;
  double rmax = 0.;
  //
  return IsToroidal(face, rmin, rmax, ax);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsToroidal(const TopoDS_Face& face,
                               double&            rmin,
                               double&            rmax,
                               gp_Ax1&            ax)
{
  if ( !asiAlgo_Utils::IsToroidal(face) )
    return false;

  bool isToroidal = false;
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
  Handle(Geom_ToroidalSurface) torsurf;

  if ( surf->IsInstance( STANDARD_TYPE(Geom_ToroidalSurface) ) )
  {
    isToroidal = true;

    // Get host surface.
    torsurf = Handle(Geom_ToroidalSurface)::DownCast(surf);
  }
  else if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
  {
    Handle(Geom_RectangularTrimmedSurface)
      RT = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);

    if ( RT->BasisSurface()->IsInstance( STANDARD_TYPE(Geom_ToroidalSurface) ) )
    {
      isToroidal = true;

      // Get host surface.
      torsurf = Handle(Geom_ToroidalSurface)::DownCast( RT->BasisSurface() );
    }
  }

  if ( isToroidal && !torsurf.IsNull() )
  {
    ax   = torsurf->Axis();
    rmin = torsurf->MinorRadius();
    rmax = torsurf->MajorRadius();
  }

  return isToroidal;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCircular(const TopoDS_Edge& edge)
{
  double f, l;
  Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

  return IsCircular(curve);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCircular(const TopoDS_Edge& edge,
                               gp_Circ&           circ)
{
  double f, l;
  Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);

  return IsCircular(curve, circ);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCircular(const Handle(Geom_Curve)& curve)
{
  gp_Circ circ;
  return IsCircular(curve, circ);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCircular(const Handle(Geom_Curve)& curve,
                               gp_Circ&                  circ)
{
  if ( curve->IsKind( STANDARD_TYPE(Geom_Circle) ) )
  {
    circ = Handle(Geom_Circle)::DownCast(curve)->Circ();
    return true;
  }

  if ( curve->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
  {
    Handle(Geom_TrimmedCurve) tcurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
    //
    if ( tcurve->BasisCurve()->IsKind( STANDARD_TYPE(Geom_Circle) ) )
    {
      circ = Handle(Geom_Circle)::DownCast( tcurve->BasisCurve() )->Circ();
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsCircular(const TopoDS_Edge& edge,
                               gp_Circ&           circ,
                               const bool         canrec)
{
  // Get curve.
  double f = 0.0, l = 0.0;
  Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
  if ( curve.IsNull() )
  {
    return false;
  }

  if ( IsCircular(curve, circ) )
  {
    return true;
  }

  // If canonical conversion is allowed, let's do the last-chance check for splines.
  if ( canrec )
  {
    if ( curve->IsKind( STANDARD_TYPE(Geom_BSplineCurve) ) )
    {
      Handle(Geom_BSplineCurve)
        spl = Handle(Geom_BSplineCurve)::DownCast(curve);

      double dev = 0.0, cf = 0.0, cl = 0.0;
      Handle(Geom_Curve) newCurve = asiAlgo_RecognizeCanonical::FitCircle(curve, 1.e-2, f, l, cf, cl, dev);
      if ( !newCurve.IsNull() )
      {
        circ = Handle(Geom_Circle)::DownCast(newCurve)->Circ();
        return true;
      }
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsStraight(const TopoDS_Edge& edge)
{
  Handle(Geom_Line) line;
  return IsStraight(edge, line);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsStraight(const TopoDS_Edge& edge,
                               Handle(Geom_Line)& line)
{
  double f, l;
  Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
  //
  if ( curve.IsNull() )
    return false;

  if ( curve->IsKind( STANDARD_TYPE(Geom_Line) ) )
  {
    line = Handle(Geom_Line)::DownCast(curve);
    return true;
  }

  if ( curve->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
  {
    Handle(Geom_TrimmedCurve) tcurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
    //
    if ( tcurve->BasisCurve()->IsKind( STANDARD_TYPE(Geom_Line) ) )
    {
      line = Handle(Geom_Line)::DownCast( tcurve->BasisCurve() );
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsStraight(const TopoDS_Edge& edge,
                               gp_Lin&            lin,
                               const bool         canrec)
{
  Handle(Geom_Line) hostLine;
  //
  if ( IsStraight(edge, hostLine) )
  {
    lin = hostLine->Lin();
    return true;
  }

  // Get curve.
  double f, l;
  Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
  //
  if ( curve.IsNull() )
    return false;

  // If canonical conversion is allowed, let's do the last-chance check for splines.
  if ( canrec )
  {
    if ( curve->IsKind( STANDARD_TYPE(Geom_BSplineCurve) ) )
    {
      Handle(Geom_BSplineCurve)
        spl = Handle(Geom_BSplineCurve)::DownCast(curve);

      gp_Lin linFit;
      double dev = 0.;
      //
      if ( asiAlgo_RecognizeCanonical::IsLinear(spl->Poles(), 1.e-2, dev, linFit) )
      {
        lin = linFit;
        return true;
      }
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsStraightPCurve(const Handle(Geom2d_Curve)& pcu,
                                     const bool                  canrec)
{
  gp_Lin2d dummy;
  return IsStraightPCurve(pcu, dummy, canrec);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsStraightPCurve(const Handle(Geom2d_Curve)& pcu,
                                     gp_Lin2d&                   lin,
                                     const bool                  canrec)
{
  if ( pcu.IsNull() )
    return false;

  if ( pcu->IsKind( STANDARD_TYPE(Geom2d_Line) ) )
  {
    lin = Handle(Geom2d_Line)::DownCast(pcu)->Lin2d();
    return true;
  }

  if ( canrec )
  {
    if ( pcu->IsKind( STANDARD_TYPE(Geom2d_BSplineCurve) ) )
    {
      Handle(Geom2d_BSplineCurve)
        spl = Handle(Geom2d_BSplineCurve)::DownCast(pcu);

      gp_Lin2d linFit;
      double   dev = 0.;
      //
      if ( asiAlgo_RecognizeCanonical::IsLinear(spl->Poles(), 1.e-2, dev, linFit) )
      {
        lin = linFit;
        return true;
      }
    }
  }

  if ( pcu->IsInstance( STANDARD_TYPE(Geom2d_TrimmedCurve) ) )
  {
    Handle(Geom2d_TrimmedCurve) tcurve = Handle(Geom2d_TrimmedCurve)::DownCast(pcu);
    //
    if ( tcurve->BasisCurve()->IsKind( STANDARD_TYPE(Geom2d_Line) ) )
    {
      lin = Handle(Geom2d_Line)::DownCast( tcurve->BasisCurve() )->Lin2d();
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsEmptyShape(const TopoDS_Shape& shape)
{
  if ( shape.IsNull() )
    return true;

  if ( shape.ShapeType() >= TopAbs_FACE )
    return false;

  // If a shape contains just nested compounds, it's still empty. If there's at least
  // something of non-compound type, the shape is considered as non-empty.
  TopTools_IndexedMapOfShape map;
  TopExp::MapShapes(shape, map);
  //
  for ( TopTools_IndexedMapOfShape::const_iterator it = map.cbegin(); it != map.cend(); it++ )
  {
    if ( (*it).ShapeType() != TopAbs_COMPOUND )
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsIdentity(const TopLoc_Location& loc)
{
  if ( loc.IsIdentity() )
    return true;

  // Nested locations.
  if ( !loc.NextLocation().IsIdentity() )
    return false;

  /* =============================
   *  Check transformation matrix.
   * ============================= */

  gp_Trsf T = loc.Transformation();

  if ( T.Form() == gp_Identity )
    return true;

  const double prec = Precision::Confusion();

  // Check translation part.
  if ( !T.TranslationPart().IsEqual( gp_XYZ(0., 0., 0.), prec ) )
    return false;

  // Check scaling.
  if ( Abs(T.ScaleFactor() - 1.0) > prec )
    return false;

  // Check whether the matrix components define exactly the identity.
  if ( Abs( T.Value(1, 1) - 1.0 ) > prec || Abs( T.Value(1, 2) )       > prec || Abs( T.Value(1, 3) )       > prec ||
       Abs( T.Value(2, 1) )       > prec || Abs( T.Value(2, 2) - 1.0 ) > prec || Abs( T.Value(2, 3) )       > prec ||
       Abs( T.Value(3, 1) )       > prec || Abs( T.Value(3, 2) )       > prec || Abs( T.Value(3, 3) - 1.0 ) > prec )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsBasisCircular(const Handle(Geom_Curve)& curve)
{
  if ( curve->IsKind( STANDARD_TYPE(Geom_Circle) ) )
    return true;

  if ( curve->IsInstance( STANDARD_TYPE(Geom_TrimmedCurve) ) )
  {
    Handle(Geom_TrimmedCurve) tcurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
    //
    if ( tcurve->BasisCurve()->IsKind( STANDARD_TYPE(Geom_Circle) ) )
      return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::AreParallel(const Handle(Geom_Plane)& S1,
                                const Handle(Geom_Plane)& S2,
                                const double              angPrec)
{
  const gp_Dir& planeDirS1 = S1->Pln().Axis().Direction();
  const gp_Dir& planeDirS2 = S2->Pln().Axis().Direction();

  if ( planeDirS1.IsParallel(planeDirS2, angPrec) ||
       planeDirS1.IsOpposite(planeDirS2, angPrec) )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::ApplyTransformation(const TopoDS_Shape& theShape,
                                                const double        theXPos,
                                                const double        theYPos,
                                                const double        theZPos,
                                                const double        theAngleA,
                                                const double        theAngleB,
                                                const double        theAngleC,
                                                const bool          doCopy)
{
  gp_Trsf T = Transformation(theXPos, theYPos, theZPos,
                             theAngleA, theAngleB, theAngleC);

  return ApplyTransformation(theShape, T, doCopy);
}

//-----------------------------------------------------------------------------

gp_Trsf asiAlgo_Utils::Transformation(const double theXPos,
                                      const double theYPos,
                                      const double theZPos,
                                      const double theAngleA,
                                      const double theAngleB,
                                      const double theAngleC)
{
  gp_Vec aTranslation(theXPos, theYPos, theZPos);

  gp_Quaternion aRotationX(gp::DX(), theAngleA);
  gp_Quaternion aRotationY(gp::DY(), theAngleB);
  gp_Quaternion aRotationZ(gp::DZ(), theAngleC);

  gp_Trsf aTrsf;
  aTrsf.SetRotation(aRotationZ * aRotationY * aRotationX);
  aTrsf.SetTranslationPart(aTranslation);

  return aTrsf;
}

//-----------------------------------------------------------------------------

TopoDS_Shape
  asiAlgo_Utils::ApplyTransformation(const TopoDS_Shape& theShape,
                                     const gp_Trsf&      theTransform,
                                     const bool          doCopy)
  {
  if ( doCopy )
  {
    TopoDS_Shape copy = BRepBuilderAPI_Copy(theShape, 1).Shape();
    return copy.Moved(theTransform);
  }

  return theShape.Moved(theTransform);
}

//-----------------------------------------------------------------------------

TopoDS_Shape
  asiAlgo_Utils::AssembleShapes(const TopTools_ListOfShape& shapes)
{
  TopoDS_Shape result, singleShape;
  int numShapes = 0;

  for ( TopTools_ListIteratorOfListOfShape it(shapes); it.More(); it.Next() )
  {
    if ( !it.Value().IsNull() )
    {
      ++numShapes;
      singleShape = it.Value();
    }
  }

  if ( numShapes > 1)
  {
    TopoDS_Compound compound;
    BRep_Builder B;
    B.MakeCompound(compound);

    for ( TopTools_ListIteratorOfListOfShape it(shapes); it.More(); it.Next() )
    {
      const TopoDS_Shape& shape = it.Value();
      //
      if ( shape.IsNull() )
        continue;

      ++numShapes;
      B.Add(compound, shape);
    }

    result = compound;
  }
  else
    result = singleShape;

  return result;
}

//-----------------------------------------------------------------------------

TopoDS_Shape
  asiAlgo_Utils::AssembleShapes(const Handle(TopTools_HSequenceOfShape)& shapes)
{
  TopTools_ListOfShape shapeList;

  // Repack as list for unification.
  if ( !shapes.IsNull() )
    for ( TopTools_SequenceOfShape::Iterator it(*shapes); it.More(); it.Next() )
      shapeList.Append( it.Value() );

  return AssembleShapes(shapeList);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Bounds(const TopoDS_Shape& shape,
                           double& XMin, double& YMin, double& ZMin,
                           double& XMax, double& YMax, double& ZMax,
                           const double tolerance,
                           const bool isPrecise)
{
  Bnd_Box bndBox;

  try
  {
    if ( isPrecise )
      BRepBndLib::AddOptimal(shape, bndBox, false, false);
    else
      BRepBndLib::Add(shape, bndBox);
  }
  catch ( ... )
  {
    return false;
  }
  //
  if ( bndBox.IsVoid() )
    return false;

  bndBox.Get(XMin, YMin, ZMin, XMax, YMax, ZMax);
  bndBox.Enlarge(tolerance);
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Bounds(const TopoDS_Shape& shape,
                           const bool          useFacets,
                           const bool          keepGap,
                           Bnd_Box&            bndBox)
{
  if ( shape.IsNull() )
    return false;

  BRepBndLib::Add(shape, bndBox, useFacets);

  if ( !keepGap )
    bndBox.SetGap( Precision::Confusion() );

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Bounds(const Handle(Poly_Triangulation)& tris,
                           double& XMin, double& YMin, double& ZMin,
                           double& XMax, double& YMax, double& ZMax,
                           const double tolerance)
{
  TopoDS_Face fictiveFace;
  BRep_Builder().MakeFace(fictiveFace);
  BRep_Builder().UpdateFace(fictiveFace, tris);

  return Bounds(fictiveFace, XMin, YMin, ZMin, XMax, YMax, ZMax, tolerance);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Bounds(const Handle(Poly_Triangulation)& mesh,
                           const TopLoc_Location&            loc,
                           Bnd_Box&                          bndBox)
{
  if ( mesh.IsNull() )
    return false;

  const int nbNodes = mesh->NbNodes();

  TColgp_Array1OfPnt nodes(1, mesh->NbNodes());
  for (int i = 1; i <= mesh->NbNodes(); i++)
    nodes(i) = mesh->Node(i);
  //
  for ( int i = 1; i <= nbNodes; ++i )
  {
    if ( loc.IsIdentity() )
      bndBox.Add( nodes(i) );
    else
      bndBox.Add( nodes(i).Transformed(loc) );
  }
  bndBox.Enlarge( mesh->Deflection() );
  return true;
}

//-----------------------------------------------------------------------------

tl::optional<gp_Ax3>
  asiAlgo_Utils::GetBboxSideFrame(const gp_Dir& dir,
                                  const double  xMin,
                                  const double  yMin,
                                  const double  zMin,
                                  const double  xMax,
                                  const double  yMax,
                                  const double  zMax)
{
  tl::optional<gp_Ax3> T_A;

  // Find local axes for the bbox side.
  if ( dir.IsEqual( gp::DX(), Precision::Angular() ) ) // RIGHT
  {
    T_A = gp_Ax3( gp_Pnt(xMax, (yMin + yMax)*0.5, (zMin + zMax)*0.5), gp::DX(), gp::DZ() );
  }
  else if ( dir.IsEqual( -gp::DX(), Precision::Angular() ) ) // LEFT
  {
    T_A = gp_Ax3( gp_Pnt(xMin, (yMin + yMax)*0.5, (zMin + zMax)*0.5), -gp::DX(), -gp::DZ() );
  }
  else if ( dir.IsEqual( gp::DY(), Precision::Angular() ) ) // TOP
  {
    T_A = gp_Ax3( gp_Pnt((xMin + xMax)*0.5, yMax, (zMin + zMax)*0.5), gp::DY(), gp::DZ() );
  }
  else if ( dir.IsEqual( -gp::DY(), Precision::Angular() ) ) // BOTTOM
  {
    T_A = gp_Ax3( gp_Pnt((xMin + xMax)*0.5, yMin, (zMin + zMax)*0.5), -gp::DY(), -gp::DZ() );
  }
  else if ( dir.IsEqual( gp::DZ(), Precision::Angular() ) ) // FRONT
  {
    T_A = gp_Ax3( gp_Pnt((xMin + xMax)*0.5, (yMin + yMax)*0.5, zMax), gp::DZ(), gp::DX() );
  }
  else if ( dir.IsEqual( -gp::DZ(), Precision::Angular() ) ) // BACK
  {
    T_A = gp_Ax3( gp_Pnt((xMin + xMax)*0.5, (yMin + yMax)*0.5, zMin), -gp::DZ(), -gp::DX() );
  }

  return T_A;
}

//-----------------------------------------------------------------------------

double asiAlgo_Utils::ComputeAABBVolume(const TopoDS_Shape& shape,
                                        const double        tolerance,
                                        const bool          isPrecise)
{
  double xMin, yMin, zMin, xMax, yMax, zMax;
  Bounds(shape, xMin, yMin, zMin, xMax, yMax, zMax, tolerance, isPrecise);

  return Abs(xMax - xMin) * Abs(yMax - yMin) * Abs(zMax - zMin);
}

//-----------------------------------------------------------------------------

double asiAlgo_Utils::CacheFaceArea(const int                  fid,
                                    const Handle(asiAlgo_AAG)& aag)
{
  double area = 0.;

  // Access the AAG attribute.
  Handle(asiAlgo_FeatureAttrArea)
    areaAttr = aag->ATTR_NODE<asiAlgo_FeatureAttrArea>(fid);

  // Compute or use the cached value.
  if ( areaAttr.IsNull() )
  {
    area = ComputeArea( aag->GetFace(fid) );
    aag->SetNodeAttribute( fid, new asiAlgo_FeatureAttrArea(area) );
  }
  else
  {
    if ( !areaAttr->GetArea() )
    {
      areaAttr->SetArea( ComputeArea( aag->GetFace(fid) ) );
    }
    area = areaAttr->GetArea();
  }
  return area;
}

//-----------------------------------------------------------------------------

double asiAlgo_Utils::ComputeArea(const TopoDS_Shape& shape)
{
  GProp_GProps props;
  BRepGProp::SurfaceProperties(shape, props, 1.0e-2);
  //
  return props.Mass();
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::CacheFaceUVBounds(const int                  fid,
                                      const Handle(asiAlgo_AAG)& aag,
                                      double&                    umin,
                                      double&                    umax,
                                      double&                    vmin,
                                      double&                    vmax)
{
  // Access the AAG attribute.
  Handle(asiAlgo_FeatureAttrUVBounds)
    uvAttr = aag->ATTR_NODE<asiAlgo_FeatureAttrUVBounds>(fid);

  // Compute or use the cached value.
  if ( uvAttr.IsNull() )
  {
    BRepTools::UVBounds(aag->GetFace(fid), umin, umax, vmin, vmax);

    aag->SetNodeAttribute( fid, new asiAlgo_FeatureAttrUVBounds(umin, umax, vmin, vmax) );
  }
  else
  {
    umin = uvAttr->uMin;
    umax = uvAttr->uMax;
    vmin = uvAttr->vMin;
    vmax = uvAttr->vMax;
  }
}

//-----------------------------------------------------------------------------

double asiAlgo_Utils::ComputeFaceLength(const TopoDS_Face& face,
                                        const gp_Ax1&      axis,
                                        const bool         useTriangulation,
                                        double&            hmin,
                                        double&            hmax)
{
  // Get all vertices here
  std::vector<gp_XYZ> pts;
  GetFacePoints(face, true, useTriangulation, pts);

  // Project to axis.
  double dotMin =  DBL_MAX;
  double dotMax = -DBL_MAX;
  for ( const auto& pnt : pts )
  {
    const double dot = ( pnt - axis.Location().XYZ() ).Dot( axis.Direction().XYZ() );

    if ( dot < dotMin )
      dotMin = dot;
    if ( dot > dotMax )
      dotMax = dot;
  }

  hmin = dotMin;
  hmax = dotMax;

  return Abs(dotMax - dotMin);
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::CacheFaceRange(const int                  fid,
                                   const Handle(asiAlgo_AAG)& aag,
                                   const gp_Ax1&              ax,
                                   double&                    hmin,
                                   double&                    hmax)
{
  // Access the AAG attribute.
  Handle(asiAlgo_FeatureAttrAxialRange)
    rangeAttr = aag->ATTR_NODE<asiAlgo_FeatureAttrAxialRange>(fid);

  // Compute or use the cached values.
  if ( rangeAttr.IsNull() )
  {
    // Compute axial range.
    ComputeFaceLength(aag->GetFace(fid), ax, false, hmin, hmax);

    // Store in AAG.
    aag->SetNodeAttribute( fid, new asiAlgo_FeatureAttrAxialRange(hmin, hmax) );
  }
  else
  {
    hmin = rangeAttr->hMin;
    hmax = rangeAttr->hMax;
  }
}

//-----------------------------------------------------------------------------

gp_Trsf asiAlgo_Utils::GetAlignmentTrsf(const gp_Ax3& A,
                                        const gp_Ax3& B)
{
  // B goes to global origin.
  gp_Trsf T_B;
  T_B.SetTransformation(B);

  // Global origin goes to A.
  gp_Trsf T_A;
  T_A.SetTransformation(A);
  T_A.Invert();

  // Final transformation from B to A.
  gp_Trsf T = T_A*T_B;

  return T;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::CleanFacets(TopoDS_Shape& shape)
{
  BRepTools::Clean(shape);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ReadBRep(const TCollection_AsciiString& filename,
                             TopoDS_Shape&                  shape)
{
  if ( IsASCII(filename) )
  {
    BRep_Builder BB;
    bool isOk;

    // We use try-catch as sometimes BREP reader crashes (if this happens in
    // batch, that's really annoying).
    try
    {
      isOk = BRepTools::Read(shape, filename.ToCString(), BB);
    }
    catch ( ... )
    {
      isOk = false;
    }

    return isOk;
  }

  // Try to read as binary
  return BinTools::Read( shape, filename.ToCString() );
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ReadIGES(const TCollection_AsciiString& filename,
                             TopoDS_Shape&                  shape,
                             ActAPI_ProgressEntry           progress,
                             ActAPI_PlotterEntry            plotter)
{
  asiAlgo_IGES reader(progress, plotter);
  if ( !reader.Read(filename, shape) )
  {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::WriteBRep(const TopoDS_Shape&            shape,
                              const TCollection_AsciiString& filename)
{
  return BRepTools::Write(shape, filename.ToCString(), false, false,
                          TopTools_FormatVersion_VERSION_2);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ReadStl(const TCollection_AsciiString& filename,
                            Handle(Poly_Triangulation)&    triangulation,
                            ActAPI_ProgressEntry           progress)
{
#if defined USE_MOBIUS
  progress.SendLogMessage(LogInfo(Normal) << "Use Mobius STL reader.");

  // Prepare reader.
  poly_ReadSTL reader;

  // Read STL.
  if ( !reader.Perform( filename.ToCString() ) )
    return false;

  // Get the constructed mesh.
  const t_ptr<t_mesh>& mesh = reader.GetResult();

  // Convert to OpenCascade's mesh.
  cascade_Triangulation<> converter(mesh);
  converter.DirectConvert();
  //
  triangulation = converter.GetOpenCascadeTriangulation();
#else
  progress.SendLogMessage(LogInfo(Normal) << "Use OpenCascade STL reader.");

  triangulation = RWStl::ReadFile(filename);
#endif

  if ( triangulation.IsNull() )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_Utils::ReadPly(const TCollection_AsciiString& filename,
                            Handle(ActData_Mesh)&          mesh,
                            ActAPI_ProgressEntry           progress)
{
  progress.SendLogMessage(LogInfo(Normal) << "Use Mobius PLY reader.");

  // Prepare reader.
  poly_ReadPLY reader;

  // Read PLY.
  if ( !reader.Perform( filename.ToCString() ) )
    return false;

  // Get the constructed mesh.
  const t_ptr<t_mesh>& mobMesh = reader.GetResult();

  // ...
  // Convert to Active Data mesh.
  // ...

  mesh = new ActData_Mesh;

  // Add mesh nodes.
  for ( t_mesh::VertexIterator vit(mobMesh); vit.More(); vit.Next() )
  {
    const poly_VertexHandle vh = vit.Current();

    // Get vertex.
    poly_Vertex mobVertex;
    if ( !mobMesh->GetVertex(vh, mobVertex) )
      continue;

    // Add node to Active Data structure.
    mesh->AddNode( mobVertex.X(), mobVertex.Y(), mobVertex.Z() );
  }

  // Add triangles.
  for ( t_mesh::TriangleIterator tit(mobMesh); tit.More(); tit.Next() )
  {
    const poly_TriangleHandle th = tit.Current();

    // Get triangle.
    poly_Triangle<> mobTriangle;
    if ( !mobMesh->GetTriangle(th, mobTriangle) )
      continue;

    // Get node indices.
    poly_VertexHandle vh[3];
    mobTriangle.GetVertices(vh[0], vh[1], vh[2]);

    // Add triangle to Active Data structure.
    mesh->AddFace(vh[0].GetIdx(), vh[1].GetIdx(), vh[2].GetIdx());
  }

  // Add quads.
  for ( t_mesh::QuadIterator qit(mobMesh); qit.More(); qit.Next() )
  {
    const poly_QuadHandle qh = qit.Current();

    // Get quad.
    poly_Quad mobQuad;
    if ( !mobMesh->GetQuad(qh, mobQuad) )
      continue;

    // Get node indices.
    poly_VertexHandle vh[4];
    mobQuad.GetVertices(vh[0], vh[1], vh[2], vh[3]);

    // Add quad to Active Data structure.
    mesh->AddFace(vh[0].GetIdx(), vh[1].GetIdx(), vh[2].GetIdx(), vh[3].GetIdx());
  }

  return true;
}
#else
bool asiAlgo_Utils::ReadPly(const TCollection_AsciiString&,
                            Handle(ActData_Mesh)&,
                            ActAPI_ProgressEntry progress)
{
  progress.SendLogMessage(LogErr(Normal) << "PLY reader is unavailable (USE_MOBIUS is off).");
  return false;
}
#endif

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_Utils::ReadPly(const TCollection_AsciiString& filename,
                            Handle(Poly_Triangulation)&    triangulation,
                            ActAPI_ProgressEntry           progress)
{
  progress.SendLogMessage(LogInfo(Normal) << "Use Mobius PLY reader.");

  // Prepare reader.
  poly_ReadPLY reader;

  // Read PLY.
  if ( !reader.Perform(filename.ToCString() ))
    return false;

  // Get the constructed mesh.
  const t_ptr<t_mesh>& mesh = reader.GetResult();

  // Convert to OpenCascade's mesh.
  cascade_Triangulation<> converter(mesh);
  converter.DirectConvert();
  //
  triangulation = converter.GetOpenCascadeTriangulation();

  if ( triangulation.IsNull() )
    return false;

  return true;
}
#else
bool asiAlgo_Utils::ReadPly(const TCollection_AsciiString&,
                            Handle(Poly_Triangulation)&,
                            ActAPI_ProgressEntry progress)
{
  progress.SendLogMessage(LogErr(Normal) << "PLY reader is unavailable (USE_MOBIUS is off).");
  return false;
}
#endif

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_Utils::ReadObj(const TCollection_AsciiString& filename,
                            Handle(ActData_Mesh)&          mesh,
                            ActAPI_ProgressEntry           progress)
{
  progress.SendLogMessage(LogInfo(Normal) << "Use Mobius OBJ reader.");

  // Prepare reader.
  poly_ReadOBJ reader;

  // Read OBJ.
  if ( !reader.Perform( filename.ToCString() ) )
    return false;

  // Get the constructed mesh.
  const t_ptr<t_mesh>& mobMesh = reader.GetResult();

  // ...
  // Convert to Active Data mesh.
  // ...

  mesh = new ActData_Mesh;

  // Add mesh nodes.
  for ( t_mesh::VertexIterator vit(mobMesh); vit.More(); vit.Next() )
  {
    const poly_VertexHandle vh = vit.Current();

    // Get vertex.
    poly_Vertex mobVertex;
    if ( !mobMesh->GetVertex(vh, mobVertex) )
      continue;

    // Add node to Active Data structure.
    mesh->AddNode( mobVertex.X(), mobVertex.Y(), mobVertex.Z() );
  }

  // Add triangles.
  for ( t_mesh::TriangleIterator tit(mobMesh); tit.More(); tit.Next() )
  {
    const poly_TriangleHandle th = tit.Current();

    // Get triangle.
    poly_Triangle<> mobTriangle;
    if ( !mobMesh->GetTriangle(th, mobTriangle) )
      continue;

    // Get node indices.
    poly_VertexHandle vh[3];
    mobTriangle.GetVertices(vh[0], vh[1], vh[2]);

    // Add triangle to Active Data structure.
    mesh->AddFace(vh[0].GetIdx(), vh[1].GetIdx(), vh[2].GetIdx());
  }

  // Add quads.
  for ( t_mesh::QuadIterator qit(mobMesh); qit.More(); qit.Next() )
  {
    const poly_QuadHandle qh = qit.Current();

    // Get quad.
    poly_Quad mobQuad;
    if ( !mobMesh->GetQuad(qh, mobQuad) )
      continue;

    // Get node indices.
    poly_VertexHandle vh[4];
    mobQuad.GetVertices(vh[0], vh[1], vh[2], vh[3]);

    // Add quad to Active Data structure.
    mesh->AddFace(vh[0].GetIdx(), vh[1].GetIdx(), vh[2].GetIdx(), vh[3].GetIdx());
  }

  return true;
}
#else
bool asiAlgo_Utils::ReadObj(const TCollection_AsciiString&,
                            Handle(ActData_Mesh)&,
                            ActAPI_ProgressEntry progress)
{
  progress.SendLogMessage(LogErr(Normal) << "OBJ reader is unavailable (USE_MOBIUS is off).");

  return false;
}
#endif

#if defined USE_MOBIUS
bool asiAlgo_Utils::ReadObj(const TCollection_AsciiString& filename,
                            Handle(Poly_Triangulation)&    triangulation,
                            ActAPI_ProgressEntry           progress)
{
  progress.SendLogMessage(LogInfo(Normal) << "Use Mobius OBJ reader.");

  // Prepare reader.
  poly_ReadOBJ reader;

  // Read OBJ.
  if ( !reader.Perform(filename.ToCString()) )
    return false;

  // Get the constructed mesh.
  const t_ptr<t_mesh>& mesh = reader.GetResult();

  // Convert to OpenCascade's mesh.
  cascade_Triangulation<> converter(mesh);
  converter.DirectConvert();
  //
  triangulation = converter.GetOpenCascadeTriangulation();

  if ( triangulation.IsNull() )
    return false;

  return true;
}
#else
bool asiAlgo_Utils::ReadObj(const TCollection_AsciiString&,
                            Handle(Poly_Triangulation)&,
                            ActAPI_ProgressEntry progress)
{
  progress.SendLogMessage(LogErr(Normal) << "OBJ reader is unavailable (USE_MOBIUS is off).");
  return false;
}
#endif

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::WriteStl(const Handle(Poly_Triangulation)& triangulation,
                             const TCollection_AsciiString&    filename,
                             const bool                        isBinary)
{
  bool isOK = true;

  if ( isBinary )
  {
    isOK = RWStl::WriteBinary(triangulation, filename);
  }
  else
  {
    isOK = RWStl::WriteAscii(triangulation, filename);
  }

  return isOK;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::WritePly(const Handle(Poly_Triangulation)& triangulation,
                             const TCollection_AsciiString&    filename,
                             ActAPI_ProgressEntry              progress)
{
  return asiAlgo_PLY(progress).Write(triangulation, filename);
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::ShapeSummary(const TopoDS_Shape&  shape,
                                 asiAlgo_TopoSummary& summary)
{
  if ( shape.IsNull() )
    return;

  // Get all outer wires in the model
  TopTools_IndexedMapOfShape outerWires;
  for ( TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next() )
  {
    const TopoDS_Face& F = TopoDS::Face( exp.Current() );
    outerWires.Add( BRepTools::OuterWire(F) );
  }

  // The following map is used to skip already traversed TShapes
  TopTools_IndexedMapOfShape M;

  // Traverse all sub-shapes
  TopTools_ListOfShape shapeList;
  shapeList.Append(shape);
  //
  for ( TopTools_ListIteratorOfListOfShape itL(shapeList); itL.More(); itL.Next() )
  {
    const TopoDS_Shape& currentShape = itL.Value();
    //
    if ( M.Contains( currentShape.Located( TopLoc_Location() ) ) )
      continue;
    else
      M.Add( currentShape.Located( TopLoc_Location() ) );

    if ( currentShape.ShapeType() == TopAbs_COMPSOLID )
      summary.nbCompsolids++;
    else if ( currentShape.ShapeType() == TopAbs_COMPOUND )
      summary.nbCompounds++;
    else if ( currentShape.ShapeType() == TopAbs_SOLID )
      summary.nbSolids++;
    else if ( currentShape.ShapeType() == TopAbs_SHELL )
      summary.nbShells++;
    else if ( currentShape.ShapeType() == TopAbs_FACE )
      summary.nbFaces++;
    else if ( currentShape.ShapeType() == TopAbs_WIRE )
    {
      summary.nbWires++;
      if ( outerWires.Contains(currentShape) )
        summary.nbOuterWires++;
      else
        summary.nbInnerWires++;
    }
    else if ( currentShape.ShapeType() == TopAbs_EDGE )
    {
      summary.nbEdges++;
      if ( BRep_Tool::Degenerated( TopoDS::Edge(currentShape) ) )
        summary.nbDegenEdges++;
    }
    else if ( currentShape.ShapeType() == TopAbs_VERTEX )
      summary.nbVertexes++;

    // Iterate over the direct children of a shape
    for ( TopoDS_Iterator sub_it(currentShape); sub_it.More(); sub_it.Next() )
    {
      const TopoDS_Shape& subShape = sub_it.Value();

      // Add sub-shape to iterate over
      shapeList.Append(subShape);
    }
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::ShapeSummary(const TopoDS_Shape&      shape,
                                 TCollection_AsciiString& info)
{
  // Extract summary
  asiAlgo_TopoSummary props;
  ShapeSummary(shape, props);

  // Prepare output string with the gathered summary
  info += "PART SUMMARY (EQ TSHAPE, ANY TRSF, ANY ORI):\n";
  info += "- nb compsolids: "; info += props.nbCompsolids; info += "\n";
  info += "- nb compounds: ";  info += props.nbCompounds;  info += "\n";
  info += "- nb solids: ";     info += props.nbSolids;     info += "\n";
  info += "- nb shells: ";     info += props.nbShells;     info += "\n";
  info += "- nb faces: ";      info += props.nbFaces;      info += "\n";
  info += "- nb wires: ";      info += props.nbWires;      info += "\n";
  info += "- nb edges: ";      info += props.nbEdges;      info += "\n";
  info += "- nb vertices: ";   info += props.nbVertexes;   info += "\n";
}

//-----------------------------------------------------------------------------

TopoDS_Wire asiAlgo_Utils::CreateCircularWire(const double radius)
{
  Handle(Geom_Circle) C = GC_MakeCircle(gp_Ax1( gp::Origin(), gp::DZ() ), radius);

  // Let's convert our circle to b-curve
  Handle(Geom_BSplineCurve) BC = GeomConvert::CurveToBSplineCurve(C, Convert_QuasiAngular);

  // Build a wire
  return BRepBuilderAPI_MakeWire( BRepBuilderAPI_MakeEdge(BC) );
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::MakeSkin(const TopTools_SequenceOfShape& wires)
{
  if ( wires.Length() < 2 )
  {
    std::cout << "Error: not enough sections" << std::endl;
    return TopoDS_Shape();
  }

  // Initialize and build
  BRepOffsetAPI_ThruSections ThruSections(false, false, 1.0e-1);
  ThruSections.SetSmoothing(false);
  ThruSections.SetMaxDegree(8);
  //
  for ( int k = 1; k <= wires.Length(); ++k )
  {
    if ( wires(k).ShapeType() != TopAbs_WIRE )
    {
      std::cout << "Warning: section " << k << " is not a wire" << std::endl;
      continue;
    }
    //
    ThruSections.AddWire( TopoDS::Wire( wires(k) ) );
  }
  try
  {
    ThruSections.Build();
  }
  catch ( ... )
  {
    std::cout << "Error: crash in BRepOffsetAPI_ThruSections" << std::endl;
    return TopoDS_Shape();
  }
  //
  if ( !ThruSections.IsDone() )
  {
    std::cout << "Error: IsDone() false in BRepOffsetAPI_ThruSections" << std::endl;
    return TopoDS_Shape();
  }

  // Get the result
  TopExp_Explorer exp(ThruSections.Shape(), TopAbs_SHELL);
  if ( !exp.More() )
    return TopoDS_Shape();
  //
  TopoDS_Shell ResultShell = TopoDS::Shell( exp.Current() );
  //
  std::cout << "Skinning was done successfully" << std::endl;
  return ResultShell;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Sew(const TopoDS_Shape& shape,
                        const double        tolerance,
                        TopoDS_Shape&       result)
{
  BRepBuilderAPI_Sewing Sewer(tolerance);
  Sewer.Load(shape);

  // Perform sewing
  Sewer.Perform();
  result = Sewer.SewedShape();
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::MaximizeFaces(TopoDS_Shape& shape,
                                  const double  linToler,
                                  const double  angToler)
{
  Handle(BRepTools_History) history;
  return MaximizeFaces(shape, history, linToler, angToler);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::MaximizeFaces(TopoDS_Shape&              shape,
                                  Handle(BRepTools_History)& history,
                                  const double               linToler,
                                  const double               angToler)
{
  ShapeUpgrade_UnifySameDomain Unify(shape);
  try
  {
    Unify.SetLinearTolerance  (linToler);
    Unify.SetAngularTolerance (angToler);
    //
    Unify.Build();
  }
  catch ( ... )
  {
    return false;
  }
  shape   = Unify.Shape();
  history = Unify.History();
  //
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ConvertCanonical(TopoDS_Shape&                    shape,
                                     const double                     tol,
                                     const bool                       checkValidity,
                                     asiAlgo_ConvertCanonicalSummary& summary,
                                     ActAPI_ProgressEntry             progress)
{
  TIMER_NEW
  TIMER_GO

  // Convert.
  asiAlgo_ConvertCanonical converter(progress);
  //
  shape   = converter.Perform(shape, tol);
  summary = converter.GetSummary();
  //
  summary.Print("Conversion results", progress);

  // Check validity.
  if ( checkValidity )
  {
    asiAlgo_CheckValidity checker(progress);
    //
    if ( !checker.CheckBasic(shape) )
    {
      progress.SendLogMessage(LogErr(Normal) << "Basic validity checker returns false "
                                                "after canonical conversion.");
      summary.isValid = false;
    }
    else
    {
      summary.isValid = true;
    }
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(progress, "CR")

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::InterpolatePoints(const std::vector<gp_XYZ>& points,
                                      const int                  p,
                                      Handle(Geom_BSplineCurve)& result)
{
  // Number of unknown control points
  const int numPoles = (int) points.size();
  const int n        = numPoles - 1;
  const int m        = n + p + 1;

  // Calculate total chord length
  double chordTotal = 0.0;
  for ( int k = 1; k < numPoles; ++k )
    chordTotal += Sqrt( ( points[k] - points[k-1] ).Modulus() );

  //-------------------------------------------------------------------------
  // Choose parameters
  double* pParams = new double[numPoles];
  pParams[0] = 0.0;
  for ( int k = 1; k < numPoles - 1; ++k )
  {
    pParams[k] = pParams[k - 1] + Sqrt( ( points[k]- points[k-1] ).Modulus() ) / chordTotal;
  }
  pParams[numPoles - 1] = 1.0;

  //-------------------------------------------------------------------------
  // Choose knots
  double* pKnots = new double[m + 1];
  //
  for ( int k = 0; k < p + 1; ++k )
    pKnots[k] = 0.0;
  for ( int k = m; k > m - p - 1; --k )
    pKnots[k] = 1.0;
  for ( int j = 1; j <= n - p; ++j )
  {
    pKnots[j + p] = 0.0;
    for ( int i = j; i <= j + p - 1; ++i )
    {
      pKnots[j + p] += pParams[i];
    }
    pKnots[j + p] /= p;
  }

  // Convert to flat knots
  TColStd_Array1OfReal flatKnots(1, m + 1);
  for ( int k = 0; k <= m; ++k )
    flatKnots(k + 1) = pKnots[k];

  // Initialize matrix from the passed row pointer
  const int dim = n + 1;
  Eigen::MatrixXd eigen_N_mx(dim, dim);

  // Calculate non-zero B-splines for each parameter value
  for ( int k = 0; k <= n; ++k )
  {
    int firstNonZeroIdx;
    math_Matrix N_mx(1, 1, 1, p + 1);
    BSplCLib::EvalBsplineBasis(0, p + 1, flatKnots, pParams[k], firstNonZeroIdx, N_mx);

#if defined DUMP_COUT
    // Dump
    std::cout << "\tFirst Non-Zero Index for " << pParams[k] << " is " << firstNonZeroIdx << std::endl;
    for ( int kk = 1; kk <= p + 1; ++kk )
    {
      std::cout << "\t" << N_mx(1, kk);
    }
    std::cout << std::endl;
#endif

    // Fill Eigen's matrix
    for ( int c = 0; c < firstNonZeroIdx - 1; ++c )
    {
      eigen_N_mx(k, c) = 0.0;
    }
    //
    for ( int c = firstNonZeroIdx - 1, kk = 1; c < firstNonZeroIdx + p; ++c, ++kk )
    {
      eigen_N_mx(k, c) = N_mx(1, kk);
    }
    //
    for ( int c = firstNonZeroIdx + p; c < dim; ++c )
    {
      eigen_N_mx(k, c) = 0.0;
    }
  }

#if defined DUMP_FILES
  // Dump Eigen matrix
  {
    std::ofstream FILE;
    FILE.open(dump_filename_N, std::ios::out | std::ios::trunc);
    //
    if ( !FILE.is_open() )
    {
      std::cout << "Error: cannot open file for dumping" << std::endl;
      return false;
    }
    //
    for ( int i = 0; i < dim; ++i )
    {
      for ( int j = 0; j < dim; ++j )
      {
        FILE << eigen_N_mx(i, j);
        if ( (i + j + 2) < (dim + dim) )
          FILE << "\t";
      }
      FILE << "\n";
    }
  }
#endif

  // Initialize vector of right hand side
  Eigen::MatrixXd eigen_B_mx(dim, 3);
  for ( int r = 0; r < dim; ++r )
  {
    eigen_B_mx(r, 0) = points[r].X();
    eigen_B_mx(r, 1) = points[r].Y();
    eigen_B_mx(r, 2) = points[r].Z();
  }

#if defined DUMP_FILES
  // Dump Eigen B [X]
  {
    std::ofstream FILE;
    FILE.open(dump_filename_Bx, std::ios::out | std::ios::trunc);
    //
    if ( !FILE.is_open() )
    {
      std::cout << "Error: cannot open file for dumping" << std::endl;
      return false;
    }
    //
    for ( int i = 0; i < dim; ++i )
    {
      FILE << eigen_B_mx(i, 0) << "\n";
    }
  }
#endif

  //---------------------------------------------------------------------------
  // BEGIN: solve linear system
  //---------------------------------------------------------------------------

  TIMER_NEW
  TIMER_GO

  // Solve
  Eigen::ColPivHouseholderQR<Eigen::MatrixXd> QR(eigen_N_mx);
  Eigen::MatrixXd eigen_X_mx = QR.solve(eigen_B_mx);

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("B-curve interpolation")

  //---------------------------------------------------------------------------
  // END: solve linear system
  //---------------------------------------------------------------------------

  // Fill poles
  TColgp_Array1OfPnt poles(1, dim);
  for ( int i = 0; i < dim; ++i )
  {
    gp_Pnt P( eigen_X_mx(i, 0), eigen_X_mx(i, 1), eigen_X_mx(i, 2) );
    poles(i + 1) = P;
  }

  // Fill knots
  TColStd_Array1OfReal knots(1, m + 1 - 2*p);
  TColStd_Array1OfInteger mults(1, m + 1 - 2*p);
  //
  knots(1) = 0; mults(1) = p + 1;
  //
  for ( int j = 2, jj = p + 1; j <= m - 2*p; ++j, ++jj )
  {
    knots(j) = pKnots[jj]; mults(j) = 1;
  }
  //
  knots( knots.Upper() ) = 1; mults( mults.Upper() ) = p + 1;

#if defined DUMP_COUT
  // Dump knots
  for ( int k = knots.Lower(); k <= knots.Upper(); ++k )
  {
    std::cout << "knots(" << k << ") = " << knots(k) << "\t" << mults(k) << std::endl;
  }
#endif

  // Create B-spline curve
  Handle(Geom_BSplineCurve) bcurve = new Geom_BSplineCurve(poles, knots, mults, p);

  // Save result
  result = bcurve;

  // Delete parameters
  delete[] pParams;
  delete[] pKnots;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::InterpolatePoints(const Handle(asiAlgo_BaseCloud<double>)& points,
                                      const int                                p,
                                      Handle(Geom_BSplineCurve)&               result)
{
  std::vector<gp_XYZ> occPts;

  // Repack data points.
  for ( int i = 0; i < points->GetNumberOfElements(); ++i )
  {
    double x, y, z;
    points->GetElement(i, x, y, z);
    //
    occPts.push_back( gp_XYZ(x, y, z) );
  }

  return InterpolatePoints(occPts, p, result);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ApproximatePoints(const std::vector<gp_XYZ>& points,
                                      const int                  degMin,
                                      const int                  degMax,
                                      const double               tol3d,
                                      Handle(Geom_BSplineCurve)& result)
{
  // Contract check.
  const int nPts = int( points.size() );
  //
  if ( nPts < 2 )
    return false;

  // Convert to OCCT collection.
  TColgp_Array1OfPnt occPts(1, nPts);
  //
  for ( int k = 0; k < nPts; ++k )
    occPts(k + 1) = points[k];

  // Approximate.
  GeomAPI_PointsToBSpline api(occPts, Approx_Centripetal, degMin, degMax, GeomAbs_C2, tol3d);
  //
  if ( !api.IsDone() )
    return false;

  result = api.Curve();
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Fill4Contour(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                                 Handle(Geom_BSplineSurface)&                  result)
{
  if ( curves.size() != 4 )
    return false;

  Handle(GeomFill_SimpleBound) b1 = new GeomFill_SimpleBound(new GeomAdaptor_Curve(curves[0]), 1e-3, 1e-2);
  Handle(GeomFill_SimpleBound) b2 = new GeomFill_SimpleBound(new GeomAdaptor_Curve(curves[1]), 1e-3, 1e-2);
  Handle(GeomFill_SimpleBound) b3 = new GeomFill_SimpleBound(new GeomAdaptor_Curve(curves[2]), 1e-3, 1e-2);
  Handle(GeomFill_SimpleBound) b4 = new GeomFill_SimpleBound(new GeomAdaptor_Curve(curves[3]), 1e-3, 1e-2);

  GeomFill_ConstrainedFilling filling(3, 100);
  filling.Init(b1, b2, b3, b4);
  //
  result = filling.Surface();
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::FillCoons(const Handle(Geom_BSplineCurve)& C0,
                              const Handle(Geom_BSplineCurve)& C1,
                              const Handle(Geom_BSplineCurve)& B0,
                              const Handle(Geom_BSplineCurve)& B1,
                              Handle(Geom_BSplineSurface)&     result,
                              ActAPI_ProgressEntry             progress,
                              ActAPI_PlotterEntry              plotter)
{
  asiAlgo_BuildCoonsSurf buildCoons(C0, C1, B0, B1, progress, plotter);
  //
  if ( !buildCoons.Perform() )
    return false;

  result = buildCoons.GetResult();
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::FillContourPlate(const std::vector<Handle(Geom_BSplineCurve)>& curves,
                                     Handle(Geom_BSplineSurface)&                  result)
{
  const int nbCurves = int( curves.size() );

  // Prepare working collection for support curves (used to create
  // pinpoint constraints).
  Handle(GeomPlate_HArray1OfHCurve)
    fronts = new GeomPlate_HArray1OfHCurve(1, nbCurves);

  // Prepare working collection for smoothness values associated with
  // each support curve.
  Handle(TColStd_HArray1OfInteger)
    tang = new TColStd_HArray1OfInteger(1, nbCurves);

  // Prepare working collection for discretization numbers associated with
  // each support curve.
  Handle(TColStd_HArray1OfInteger)
    nbPtsCur = new TColStd_HArray1OfInteger(1, nbCurves);

  // Loop over the curves to prepare constraints.
  int i = 0;
  for ( int cidx = 0; cidx < nbCurves; ++cidx )
  {
    i++;
    tang->SetValue(i, GeomAbs_C0);
    nbPtsCur->SetValue(i, 50); // Number of discretization points

    Handle(GeomAdaptor_Curve)
      curveAdt = new GeomAdaptor_Curve(curves[cidx]);

    fronts->SetValue(i, curveAdt);
  }

  TIMER_NEW
  TIMER_GO

  // Build plate
  GeomPlate_BuildPlateSurface BuildPlate(nbPtsCur, fronts, tang, 3);
  //
  try
  {
    BuildPlate.Perform();
  }
  catch ( ... )
  {
    return false;
  }
  //
  if ( !BuildPlate.IsDone() )
  {
    return false;
  }
  Handle(GeomPlate_Surface) plate = BuildPlate.Surface();

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("Build plate")

  // Approximate plate with NURBS.

  TIMER_RESET
  TIMER_GO

  GeomPlate_MakeApprox MKS(plate, Precision::Confusion(), 18, 14, 0.001, 0);
  result = MKS.Surface();

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("Approximate plate with B-surface")

  return true;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::BooleanCut(const TopoDS_Shape& object,
                                       const TopoDS_Shape& tool,
                                       const double        fuzzy)
{
  // Prepare the arguments
  TopTools_ListOfShape BOP_args;
  BOP_args.Append(object);
  BOP_args.Append(tool);

  // Prepare data structure (calculate interferences)
  Handle(NCollection_BaseAllocator) Alloc = new NCollection_IncAllocator;
  //
  BOPAlgo_PaveFiller DSFiller(Alloc);
  DSFiller.SetArguments(BOP_args);
  DSFiller.SetRunParallel(0);
  DSFiller.SetFuzzyValue(fuzzy);
  DSFiller.Perform();

  // Check data structure
  bool hasErr = DSFiller.HasErrors();
  if ( hasErr )
  {
    std::cout << "Error: cannot cut" << std::endl;
    return TopoDS_Shape();
  }

  // Run BOP
  BOPAlgo_BOP BOP(Alloc);
  BOP.AddArgument(object);
  BOP.AddTool(tool);
  BOP.SetRunParallel(0);
  BOP.SetOperation(BOPAlgo_CUT);
  BOP.PerformWithFiller(DSFiller);
  hasErr = BOP.HasErrors();
  if ( hasErr )
  {
    std::cout << "Error: cannot cut the part model from the stock model" << std::endl;
    return TopoDS_Shape();
  }

  return BOP.Shape();
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::BooleanCut(const TopoDS_Shape&         object,
                                       const TopTools_ListOfShape& tools,
                                       const bool                  isParallel,
                                       const double                fuzz)
{
  BRepAlgoAPI_Cut API;
  //
  return BooleanCut(object, tools, isParallel, fuzz, API);
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::BooleanCut(const TopoDS_Shape& object,
                                       const TopoDS_Shape& tool,
                                       const bool          isParallel,
                                       const double        fuzz,
                                       BRepAlgoAPI_Cut&    API)
{
  TopTools_ListOfShape tools;
  tools.Append(tool);

  return BooleanCut(object, tools, isParallel, fuzz, API);
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::BooleanCut(const TopoDS_Shape&         object,
                                       const TopTools_ListOfShape& tools,
                                       const bool                  isParallel,
                                       const double                fuzz,
                                       BRepAlgoAPI_Cut&            API)
{
  // Prepare the arguments
  TopTools_ListOfShape objects;
  objects.Append(object);

  // Set the arguments
  API.SetArguments(objects);
  API.SetTools(tools);
  API.SetRunParallel(isParallel);
  API.SetFuzzyValue(fuzz);

  // Run the algorithm
  API.Build();

  // Check the result
  const bool hasErr = API.HasErrors();
  //
  if ( hasErr )
  {
    std::cout << "Error: cannot cut the part model from the stock model" << std::endl;
    return TopoDS_Shape();
  }

  return API.Shape();
}

//-----------------------------------------------------------------------------

//! Performs Boolean fuse operation on the passed objects.
//! \param[in] objects shapes to fuse.
//! \return result of fuse.
TopoDS_Shape asiAlgo_Utils::BooleanFuse(const TopTools_ListOfShape& objects)
{
  TopTools_ListIteratorOfListOfShape it(objects);
  TopoDS_Shape result = it.Value();
  it.Next();

  for ( ; it.More(); it.Next() )
  {
    TopoDS_Shape arg = it.Value();
    result = BRepAlgoAPI_Fuse(result, arg);
  }

  return result;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::BooleanFuse(const TopTools_ListOfShape& objects,
                                        const double                fuzz,
                                        Handle(BRepTools_History)&  history)
{
  TopTools_ListIteratorOfListOfShape it(objects);
  TopoDS_Shape result = it.Value();
  it.Next();

  // Fuse one by one.
  history = new BRepTools_History;
  //
  for ( ; it.More(); it.Next() )
  {
    TopoDS_Shape arg = it.Value();

    // For pave filler.
    TopTools_ListOfShape ops;
    ops.Append(result);
    ops.Append(arg);

    // For the Boolean Operation filtering.
    TopTools_ListOfShape args;
    args.Append(result);
    //
    TopTools_ListOfShape tools;
    tools.Append(arg);

    // Use pave filler to pass the fuzzy value.
    BOPAlgo_PaveFiller DSFiller;
    DSFiller.SetArguments(ops);
    DSFiller.SetRunParallel(false);
    DSFiller.SetFuzzyValue(fuzz);
    DSFiller.Perform();
    bool hasErr = DSFiller.HasErrors();
    //
    if ( hasErr )
    {
      return TopoDS_Shape();
    }

    // Fuse with a pave filler.
    BRepAlgoAPI_Fuse API(DSFiller);
    API.SetArguments(args);
    API.SetTools(tools);
    API.Build();
    //
    result = API.Shape();
    history->Merge( API.History() );
  }

  return result;
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::BooleanFuse(const TopTools_ListOfShape& objects,
                                        const bool                  maximizeFaces,
                                        Handle(BRepTools_History)&  history)
{
  // Fuse.
  Handle(BRepTools_History) bopHistory;
  TopoDS_Shape              fused = BooleanFuse(objects, 0., bopHistory);

  // Check if face maximization was requested.
  if ( !maximizeFaces )
  {
    history = bopHistory;
    return fused;
  }

  // Maximize faces.
  Handle(BRepTools_History) maximizeHistory;
  //
  if ( !MaximizeFaces(fused, maximizeHistory) )
  {
    history = bopHistory;
    return fused;
  }

  // Merge history and return.
  bopHistory->Merge(maximizeHistory);
  history = bopHistory;
  //
  return fused;
}

//-----------------------------------------------------------------------------

//! Performs Boolean intersection operation on the passed objects.
//! \param[in] objects shapes to intersect.
//! \return result of intersection.
TopoDS_Shape asiAlgo_Utils::BooleanIntersect(const TopTools_ListOfShape& objects)
{
  TopTools_ListIteratorOfListOfShape it(objects);
  TopoDS_Shape result = it.Value();
  it.Next();

  for ( ; it.More(); it.Next() )
  {
    TopoDS_Shape arg = it.Value();
    result = BRepAlgoAPI_Common(result, arg);
  }

  return result;
}

//-----------------------------------------------------------------------------

//! Performs Boolean General Fuse for the passed objects.
//! \param[in]  objects objects to fuse in non-manifold manner.
//! \param[in]  fuzz    fuzzy value to control the tolerance.
//! \param[out] API     Boolean algorithm to access history.
//! \param[in]  glue    whether gluing is requested.
TopoDS_Shape asiAlgo_Utils::BooleanGeneralFuse(const TopTools_ListOfShape& objects,
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

//-----------------------------------------------------------------------------

//! Performs Boolean General Fuse for the passed objects.
//! \param[in] objects objects to fuse in non-manifold manner.
//! \param[in] fuzz    fuzzy value to control the tolerance.
//! \param[in] glue    whether gluing is requested.
TopoDS_Shape asiAlgo_Utils::BooleanGeneralFuse(const TopTools_ListOfShape& objects,
                                               const double                fuzz,
                                               const bool                  glue)
{
  BOPAlgo_Builder API;
  return BooleanGeneralFuse(objects, fuzz, API, glue);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::BooleanRemoveFaces(const TopoDS_Shape&  shape,
                                       const TopoDS_Shape&  face2Remove,
                                       const bool           runParallel,
                                       const bool           trackHistory,
                                       TopoDS_Shape&        result,
                                       ActAPI_ProgressEntry progress)
{
  TopTools_ListOfShape faces2Remove; faces2Remove.Append(face2Remove);

  return BooleanRemoveFaces(shape,
                            faces2Remove,
                            runParallel,
                            trackHistory,
                            result,
                            progress);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::BooleanRemoveFaces(const TopoDS_Shape&         shape,
                                       const TopTools_ListOfShape& faces2Remove,
                                       const bool                  runParallel,
                                       const bool                  trackHistory,
                                       TopoDS_Shape&               result,
                                       ActAPI_ProgressEntry        progress)
{
  // Prepare tool.
  BRepAlgoAPI_Defeaturing API;
  //
  API.SetShape         (shape);
  API.AddFacesToRemove (faces2Remove);
  API.SetRunParallel   (runParallel);
  API.SetToFillHistory (trackHistory);

  // Perform.
  API.Build();
  //
  if ( !API.IsDone() )
  {
    progress.SendLogMessage(LogErr(Normal) << "Smart face removal is not done.");
    return false;
  }
  //
  if ( API.HasWarnings() )
  {
    std::ostringstream out;
    API.DumpWarnings(out);

    progress.SendLogMessage( LogWarn(Normal) << "Smart face removal finished with warnings:\n\t %1"
                                             << out.str().c_str() );
  }

  // Set result.
  result = API.Shape();
  return true;
}

//-----------------------------------------------------------------------------

//! Explodes the passed shape by solids.
//! \param model  [in]  input CAD part.
//! \param solids [out] extracted solids.
void asiAlgo_Utils::ExplodeBySolids(const TopoDS_Shape&   model,
                                    TopTools_ListOfShape& solids)
{
  for ( TopExp_Explorer exp(model, TopAbs_SOLID); exp.More(); exp.Next() )
  {
    solids.Append( exp.Current() );
  }
}

//-----------------------------------------------------------------------------

//! Inverts the passed face so that all internal loops become individual
//! faces while the outer loop is simply erased.
//! \param face     [in]  face to invert.
//! \param inverted [out] inversion result (collection of faces).
//! \return true in case of success, false -- otherwise.
bool asiAlgo_Utils::InvertFace(const TopoDS_Face&    face,
                               TopTools_ListOfShape& inverted)
{
  TopTools_IndexedMapOfShape wires;
  TopExp::MapShapes(face, TopAbs_WIRE, wires);
  //
  if ( wires.Extent() < 2 )
    return false; // Cannot invert a face without holes

  // Get outer wire
  TopoDS_Wire outerWire = BRepTools::OuterWire(face);

  // Get host geometry
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

  // Search for internal loops: those which are not the outer wire
  for ( int w = 1; w <= wires.Extent(); ++w )
  {
    TopoDS_Wire wire = TopoDS::Wire( wires(w) );
    //
    if ( wire.IsSame(outerWire) )
      continue;

    // Construct another face on the same host with a different wire
    wire.Reverse();
    TopoDS_Face innerFace = BRepBuilderAPI_MakeFace(surf, wire, false);
    //
    ShapeFix_Face fix(innerFace);
    if ( fix.Perform() )
      innerFace = fix.Face();
    //
    inverted.Append(innerFace);
  }
  return true;
}

//-----------------------------------------------------------------------------

Handle(Geom_BSplineCurve)
  asiAlgo_Utils::PolylineAsSpline(const TColgp_Array1OfPnt& trace)
{
  std::vector<gp_XYZ> poles;
  //
  for ( int i = trace.Lower(); i <= trace.Upper(); ++i )
    poles.push_back( trace(i).XYZ() );

  return PolylineAsSpline(poles);
}

//-----------------------------------------------------------------------------

Handle(Geom_BSplineCurve)
  asiAlgo_Utils::PolylineAsSpline(const std::vector<gp_XYZ>& trace)
{
  // Initialize properties for spline trajectories
  TColgp_Array1OfPnt poles( 1, (int) trace.size() );
  //
  for ( size_t k = 0; k < trace.size(); ++k )
  {
    poles( int(k + 1) ) = gp_Pnt( trace[k] );
  }

  const int n = poles.Upper() - 1;
  const int p = 1;
  const int m = n + p + 1;
  const int k = m + 1 - (p + 1)*2;

  // Knots
  TColStd_Array1OfReal knots(1, k + 2);
  knots(1) = 0;
  //
  for ( int j = 2; j <= k + 1; ++j )
    knots(j) = knots(j-1) + 1.0 / (k + 1);
  //
  knots(k + 2) = 1;

  // Multiplicities
  TColStd_Array1OfInteger mults(1, k + 2);
  mults(1) = 2;
  //
  for ( int j = 2; j <= k + 1; ++j )
    mults(j) = 1;
  //
  mults(k + 2) = 2;

  return new Geom_BSplineCurve(poles, knots, mults, 1);
}

//-----------------------------------------------------------------------------

Handle(Geom2d_BSplineCurve)
  asiAlgo_Utils::PolylineAsSpline(const TColgp_Array1OfPnt2d& trace)
{
  std::vector<gp_XY> poles;
  //
  for ( int i = trace.Lower(); i <= trace.Upper(); ++i )
    poles.push_back( trace(i).XY() );

  return PolylineAsSpline(poles);
}

//-----------------------------------------------------------------------------

Handle(Geom2d_BSplineCurve)
  asiAlgo_Utils::PolylineAsSpline(const std::vector<gp_XY>& trace)
{
  // Initialize properties for spline trajectories
  TColgp_Array1OfPnt2d poles( 1, (int) trace.size() );
  //
  for ( int k = 0; k < int( trace.size() ); ++k )
  {
    poles(k + 1) = gp_Pnt2d( trace[k] );
  }

  const int n = poles.Upper() - 1;
  const int p = 1;
  const int m = n + p + 1;
  const int k = m + 1 - (p + 1)*2;

  // Knots
  TColStd_Array1OfReal knots(1, k + 2);
  knots(1) = 0;
  //
  for ( int j = 2; j <= k + 1; ++j )
    knots(j) = knots(j-1) + 1.0 / (k + 1);
  //
  knots(k + 2) = 1;

  // Multiplicities
  TColStd_Array1OfInteger mults(1, k + 2);
  mults(1) = 2;
  //
  for ( int j = 2; j <= k + 1; ++j )
    mults(j) = 1;
  //
  mults(k + 2) = 2;

  return new Geom2d_BSplineCurve(poles, knots, mults, 1);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Contains(const TopoDS_Shape& shape,
                             const TopoDS_Shape& subShape)
{
  TopTools_IndexedMapOfShape allSubShapes;
  TopExp::MapShapes(shape, allSubShapes);

  return allSubShapes.Contains(subShape);
}

//-----------------------------------------------------------------------------

TopoDS_Shape asiAlgo_Utils::GetImage(const TopoDS_Shape&       source,
                                     BRepBuilderAPI_MakeShape& API)
{
  const TopTools_ListOfShape& modified = API.Modified(source);
  //
  if ( modified.Extent() == 1 )
    return modified.First();
  //
  if ( modified.Extent() )
  {
    TopoDS_Compound C;
    BRep_Builder().MakeCompound(C);

    for ( TopTools_ListIteratorOfListOfShape lit(modified); lit.More(); lit.Next() )
    {
      const TopoDS_Shape& image = lit.Value();
      BRep_Builder().Add(C, image);
    }

    return C;
  }

  const TopTools_ListOfShape& generated = API.Generated(source);
  //
  if ( generated.Extent() == 1 )
    return generated.First();
  //
  if ( generated.Extent() )
  {
    TopoDS_Compound C;
    BRep_Builder().MakeCompound(C);

    for ( TopTools_ListIteratorOfListOfShape lit(generated); lit.More(); lit.Next() )
    {
      const TopoDS_Shape& image = lit.Value();
      BRep_Builder().Add(C, image);
    }

    return C;
  }

  // This check is placed after all other checks intentionally. The idea is
  // that for edges we may have GENERATED faces from them, while the edges
  // themselves are DELETED. Since our focus is on features (which are
  // essentially just sets of faces), we prefer returning an image face
  // rather than null shape for a deleted edge (like it happens in filleting).
  if ( API.IsDeleted(source) )
    return TopoDS_Shape();

  return source;
}

//-----------------------------------------------------------------------------

Handle(Poly_Triangulation)
  asiAlgo_Utils::CreateTriangle(const gp_Pnt& P0,
                                const gp_Pnt& P1,
                                const gp_Pnt& P2)
{
  // Create array of nodes
  TColgp_Array1OfPnt nodes(1, 3);
  nodes(1) = P0;
  nodes(2) = P1;
  nodes(3) = P2;

  // Prepare single triangle
  Poly_Array1OfTriangle tris(1, 1);
  tris(1) = Poly_Triangle(1, 2, 3);

  // Create triangulation
  return new Poly_Triangulation(nodes, tris);
}

//-----------------------------------------------------------------------------

//! Originally taken from http://www.redblobgames.com/grids/hexagons/.
void asiAlgo_Utils::HexagonPoles(const gp_XY& center,
                                 const double dist2Pole,
                                 gp_XY&       P1,
                                 gp_XY&       P2,
                                 gp_XY&       P3,
                                 gp_XY&       P4,
                                 gp_XY&       P5,
                                 gp_XY&       P6)
{
  std::vector<gp_XY*> res = {&P1, &P2, &P3, &P4, &P5, &P6};

  std::vector<gp_XY> poles;
  PolygonPoles(center, dist2Pole, 6, poles);
  //
  for ( int k = 0; k < 6; ++k )
    *res[k] = poles[k];
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::PolygonPoles(const gp_XY&        center,
                                 const double        dist2Pole,
                                 const int           numPoles,
                                 std::vector<gp_XY>& poles)
{
  const double angStep  = 360.0/numPoles;
  const double angDelta = angStep/2;

  for ( int i = 0; i < numPoles; ++i )
  {
    const double angle_deg = angStep*i + angDelta;
    const double angle_rad = angle_deg / 180.0 * M_PI;

    const double x = center.X() + dist2Pole * Cos(angle_rad);
    const double y = center.X() + dist2Pole * Sin(angle_rad);

    poles.push_back( gp_XY(x, y) );
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateFaceNormals(const TopoDS_Face&                 face,
                                         const double                       sampleRate,
                                         Handle(asiAlgo_BaseCloud<double>)& points,
                                         Handle(asiAlgo_BaseCloud<double>)& vectors,
                                         gp_Vec&                            average)
{
  if ( sampleRate < 1e-10 || sampleRate > 1 )
    return false;

  // Take surface.
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
  //
  if ( surf.IsNull() )
    return false;

  // Prepare a point cloud as a result
  points  = new asiAlgo_BaseCloud<double>;
  vectors = new asiAlgo_BaseCloud<double>;

  // Nullify average norm.
  average = {0, 0, 0};

  // Take face domain
  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);
  //
  const double uStep = (uMax - uMin)*sampleRate;
  const double vStep = (vMax - vMin)*sampleRate;

  // Prepare classifier
  asiAlgo_ClassifyPointFace classifier(face, BRep_Tool::Tolerance(face), 0.01, false);

  // Sample points
  double   u = uMin;
  bool uStop = false;
  int  nnorm = 0;
  //
  while ( !uStop )
  {
    if ( u > uMax )
    {
      u     = uMax;
      uStop = true;
    }

    double v = vMin;
    bool vStop = false;
    while ( !vStop )
    {
      if ( v > vMax )
      {
        v     = vMax;
        vStop = true;
      }

      // Perform point membership classification
      asiAlgo_Membership pmc = classifier( gp_Pnt2d(u, v) );
      //
      if ( pmc & Membership_InOn )
      {
        gp_Pnt P;
        gp_Vec D1U, D1V;
        surf->D1(u, v, P, D1U, D1V);

        gp_Vec N = D1U^D1V;
        //
        if ( N.Magnitude() > 1e-10 )
        {
          N.Normalize();
          //
          points->AddElement  ( P.X(), P.Y(), P.Z() );
          vectors->AddElement ( N.X(), N.Y(), N.Z() );

          // Compute average.
          average += N;
          nnorm++;
        }
      }

      v += vStep;
    }

    u += uStep;
  }

  average /= nnorm;
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::GetFaceAnyInteriorPoint(const TopoDS_Face& face,
                                            gp_Pnt2d&          uv,
                                            gp_Pnt&            xyz)
{
  // Take surface
  Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
  //
  if ( surf.IsNull() )
    return false;

  // Take face domain
  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);
  //
  const double uStep = (uMax - uMin)*0.05;
  const double vStep = (vMax - vMin)*0.05;

  // Prepare classifier
  asiAlgo_ClassifyPointFace classifier(face, BRep_Tool::Tolerance(face), 0.01, false);

  // Sample points
  double u = uMin;
  bool uStop = false;
  while ( !uStop )
  {
    if ( u > uMax )
    {
      u     = uMax;
      uStop = true;
    }

    double v = vMin;
    bool vStop = false;
    while ( !vStop )
    {
      if ( v > vMax )
      {
        v     = vMax;
        vStop = true;
      }

      // Perform point membership classification
      asiAlgo_Membership pmc = classifier( gp_Pnt2d(u, v) );
      //
      if ( pmc & Membership_In )
      {
        gp_Pnt P;
        gp_Vec D1U, D1V;
        surf->D0(u, v, P);

        uv.SetCoord( u, v );
        xyz.SetCoord( P.X(), P.Y(), P.Z() );

        return true;
      }

      v += vStep;
    }

    u += uStep;
  }
  return false;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::PrintSurfaceDetails(const Handle(Geom_Surface)& surf,
                                        Standard_OStream&           out)
{
  // Print common header.
  out << "Surface type: " << surf->DynamicType()->Name() << "\n";

  // Rectangular Trimmed Surface.
  if ( surf->IsInstance( STANDARD_TYPE(Geom_RectangularTrimmedSurface) ) )
  {
    Handle(Geom_RectangularTrimmedSurface)
      RS = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);

    // Let's check basis surface.
    Handle(Geom_Surface) BS = RS->BasisSurface();
    //
    out << "Basis surface: ";
    out << ( BS.IsNull() ? "NONE" : BS->DynamicType()->Name() ) << "\n";
  }

  // B-surface
  else if ( surf->IsKind( STANDARD_TYPE(Geom_BSplineSurface) ) )
  {
    // Downcast.
    Handle(Geom_BSplineSurface)
      bsurf = Handle(Geom_BSplineSurface)::DownCast(surf);

    out << "U degree: "   << bsurf->UDegree()                          << "\n";
    out << "V degree: "   << bsurf->VDegree()                          << "\n";
    out << "Continuity: " << ContinuityToString( bsurf->Continuity() ) << "\n";

    if ( bsurf->IsUPeriodic() )
      out << "U-periodic\n";
    else
      out << "Not U-periodic\n";

    if ( bsurf->IsVPeriodic() )
      out << "V-periodic\n";
    else
      out << "Not V-periodic\n";

    // Get number of control points.
    const int numPoles = bsurf->Poles().ColLength() * bsurf->Poles().RowLength();
    //
    out << "Num. of poles: " << numPoles << "\n";
  }

  // Cylindrical surface.
  else if ( surf->IsKind( STANDARD_TYPE(Geom_CylindricalSurface) ) )
  {
    Handle(Geom_CylindricalSurface)
      CS = Handle(Geom_CylindricalSurface)::DownCast(surf);

    const double r = CS->Radius();
    //
    out << "Radius: " << r << "\n";
  }

  // Spherical surface.
  else if ( surf->IsKind( STANDARD_TYPE(Geom_SphericalSurface) ) )
  {
    Handle(Geom_SphericalSurface)
      SS = Handle(Geom_SphericalSurface)::DownCast(surf);

    const double r = SS->Radius();
    //
    out << "Radius: " << r << "\n";
  }

  // Toroidal surface.
  else if ( surf->IsKind( STANDARD_TYPE(Geom_ToroidalSurface) ) )
  {
    Handle(Geom_ToroidalSurface)
      TS = Handle(Geom_ToroidalSurface)::DownCast(surf);

    const double minr = TS->MinorRadius();
    const double majr = TS->MajorRadius();
    //
    out << "Minor radius: " << minr << "\n";
    out << "Major radius: " << majr << "\n";
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateCurvatureComb(const Handle(Geom_Curve)& curve,
                                           const double              u,
                                           const double              curvAmpl,
                                           gp_Pnt&                   p,
                                           double&                   k,
                                           gp_Vec&                   c)
{
  p = curve->Value(u);

  GeomLProp_CLProps lProps( curve, u, 2, gp::Resolution() );
  //
  if ( !lProps.IsTangentDefined() )
    return false;

  // Calculate the first derivative.
  const gp_Vec& x_1 = lProps.D1();
  //
  if ( x_1.Magnitude() < 1.0e-7 )
    return false;

  // Calculate the second derivative.
  const gp_Vec& x_2 = lProps.D2();
  //
  if ( x_2.Magnitude() < 1.0e-7 )
    return false;

  // Calculate binormal.
  gp_Dir b = x_1 ^ x_2;

  // Calculate normal.
  gp_Dir n = b ^ x_1;

  // Calculate curvature.
  k = lProps.Curvature();

  // Calculate comb.
  c = p.XYZ() - curvAmpl*k*n.XYZ();

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateCurvatureCombs(const Handle(Geom_Curve)& curve,
                                            const double              f,
                                            const double              l,
                                            const int                 numPts,
                                            const double              curvAmpl,
                                            std::vector<gp_Pnt>&      points,
                                            std::vector<double>&      params,
                                            std::vector<double>&      curvatures,
                                            std::vector<gp_Vec>&      combs,
                                            std::vector<bool>&        combsOk)
{
  // Discretize with a uniform curvilinear step.
  GeomAdaptor_Curve gac(curve, f, l);
  GCPnts_QuasiUniformAbscissa Defl(gac, numPts);
  //
  if ( !Defl.IsDone() )
    return false;

  // Calculate combs at discretization points.
  for ( int i = 1; i <= numPts; ++i )
  {
    const double param = Defl.Parameter(i);

    gp_Pnt p;
    double curvature;
    gp_Vec comb;
    //
    const bool
      isOk = asiAlgo_Utils::CalculateCurvatureComb(curve,
                                                   param,
                                                   curvAmpl,
                                                   p,
                                                   curvature,
                                                   comb);

    // Fill result arrays.
    combsOk    .push_back( isOk );
    points     .push_back( p );
    params     .push_back( param );
    curvatures .push_back( curvature );
    combs      .push_back( comb.XYZ() );
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateStrainEnergy(const Handle(Geom_Curve)& curve,
                                          double&                   result)
{
  asiAlgo_CuuSquared Cuu2Func(curve);

  result = IntegralRect( Cuu2Func,
                         curve->FirstParameter(),
                         curve->LastParameter(),
                         NUM_INTEGRATION_BINS );
  return true;
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_Utils::CalculateBendingEnergy(const Handle(Geom_Surface)& surface,
                                           double&                     result)
{
  if ( !surface->IsInstance( STANDARD_TYPE(Geom_BSplineSurface) ) )
    return false;

  Handle(Geom_BSplineSurface)
    occtSurf = Handle(Geom_BSplineSurface)::DownCast(surface);

  // Convert to Mobius.
  t_ptr<t_bsurf>
    mobSurf = cascade::GetMobiusBSurface(occtSurf);

  // Evaluate bending energy.
  result = mobSurf->ComputeBendingEnergy();
  return true;
}
#else
bool asiAlgo_Utils::CalculateBendingEnergy(const Handle(Geom_Surface)&,
                                           double& result)
{
  result = 0.0;
  return false;
}
#endif

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ReparametrizeBSpl(const Handle(Geom2d_Curve)&  curve,
                                      const double                 newFirst,
                                      const double                 newLast,
                                      const bool                   toCopy,
                                      Handle(Geom2d_BSplineCurve)& result)
{
  Handle(Geom2d_BSplineCurve)
    curveBSpl = Handle(Geom2d_BSplineCurve)::DownCast(curve);
  //
  if ( curveBSpl.IsNull() )
    return false;

  // Prepare a copy if necessary.
  if ( toCopy )
    result = Handle(Geom2d_BSplineCurve)::DownCast( curveBSpl->Copy() );
  else
    result = curveBSpl;

  // Reparameterize.
  const int numKnots = result->NbKnots();
  //
  TColStd_Array1OfReal knots(1, numKnots);
  result->Knots(knots);
  BSplCLib::Reparametrize(newFirst, newLast, knots);
  result->SetKnots(knots);
  //
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::EvaluateAlongCurvature(const TopoDS_Face& face,
                                           const TopoDS_Edge& edge,
                                           const double       t,
                                           gp_Pnt2d&          UV,
                                           double&            k)
{
  // Get host geometries
  double f, l;
  Handle(Geom2d_Curve) c2d = BRep_Tool::CurveOnSurface(edge, face, f, l);
  BRepAdaptor_Surface surf(face);

  // Evaluate curve
  gp_Vec2d T;
  c2d->D1(t, UV, T);

  // Calculate differential properties
  BRepLProp_SLProps Props(surf, UV.X(), UV.Y(), 2, 1e-7);
  //
  if ( !Props.IsCurvatureDefined() )
  {
#if defined COUT_DEBUG
    std::cout << "Error: curvature is not defined" << std::endl;
#endif
    return false;
  }

  // Get differential properties
  const gp_Vec Xu  = Props.D1U();
  const gp_Vec Xv  = Props.D1V();
  const gp_Vec Xuu = Props.D2U();
  const gp_Vec Xuv = Props.DUV();
  const gp_Vec Xvv = Props.D2V();
  const gp_Vec n   = Props.Normal();

  // Coefficients of the FFF
  const double E = Xu.Dot(Xu);
  const double F = Xu.Dot(Xv);
  const double G = Xv.Dot(Xv);

  // Coefficients of the SFF
  const double L = n.Dot(Xuu);
  const double M = n.Dot(Xuv);
  const double N = n.Dot(Xvv);

  // Calculate curvature using the coefficients of both fundamental forms
  if ( Abs( T.X() ) < 1.0e-5 )
    k = N / G;
  else
  {
    const double lambda = T.Y() / T.X();
    k = (L + 2*M*lambda + N*lambda*lambda) / (E + 2*F*lambda + G*lambda*lambda);
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::EvaluateAlongCurvature(const TopoDS_Face& face,
                                           const TopoDS_Edge& edge,
                                           const double       t,
                                           double&            k)
{
  gp_Pnt2d UV;

  return EvaluateAlongCurvature(face, edge, t, UV, k);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::EvaluateAlongCurvature(const TopoDS_Face& face,
                                           const TopoDS_Edge& edge,
                                           double&            k)
{
  gp_Pnt2d UV;
  double f, l;
  BRep_Tool::Range(edge, f, l);
  const double t = (f + l)*0.5;

  return EvaluateAlongCurvature(face, edge, t, UV, k);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::EvaluateAlongCurvature(const TopoDS_Face& face,
                                           const TopoDS_Edge& edge,
                                           gp_Pnt2d&          UV,
                                           double&            k)
{
  double f, l;
  BRep_Tool::Range(edge, f, l);
  const double t = (f + l)*0.5;

  return EvaluateAlongCurvature(face, edge, t, UV, k);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateMidCurvature(const Handle(Geom_Curve)& curve,
                                          gp_Pnt&                   P,
                                          gp_Dir&                   T,
                                          double&                   k,
                                          double&                   r,
                                          gp_Pnt&                   center)
{
  const double uMid = ( curve->FirstParameter() + curve->LastParameter() )*0.5;

  GeomLProp_CLProps lProps( curve, uMid, 2, gp::Resolution() );
  //
  if ( !lProps.IsTangentDefined() )
    return false;

  // Calculate point and tangent.
  P = lProps.Value();
  //
  lProps.Tangent(T);

  // Calculate curvature.
  k = lProps.Curvature();
  //
  if ( Abs(k) < 1.e-5 )
    r = Precision::Infinite();
  else
  {
    r = Abs(1.0 / k);
    //
    lProps.CentreOfCurvature(center);
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateMidCurvature(const Handle(Geom_Curve)& curve,
                                          double&                   k,
                                          double&                   r,
                                          gp_Pnt&                   center)
{
  gp_Pnt P;
  gp_Dir T;
  return CalculateMidCurvature(curve, P, T, k, r, center);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::CalculateMidCurvature(const Handle(Geom_Curve)& curve,
                                          double&                   k,
                                          double&                   r)
{
  gp_Pnt center;
  return CalculateMidCurvature(curve, k, r, center);
}

//-----------------------------------------------------------------------------

double asiAlgo_Utils::IntegralRect(math_Function& F,
                                   const double   a,
                                   const double   b,
                                   const int      n)
{
  double step = (b - a) / n; // width of each small rectangle.
  double area = 0.0; // signed area.
  for ( int i = 0; i < n; ++i )
  {
    double val = 0.0;
    F.Value(a + (i + 0.5) * step, val);
    area +=  val*step; // sum up each small rectangle.
  }
  return area;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::RebuildBounds(TopoDS_Shape& shape)
{
  for ( TopExp_Explorer exp(shape, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    const TopoDS_Edge&                               E = TopoDS::Edge( exp.Current() );
    const BRep_TEdge*                               TE = static_cast<const BRep_TEdge*>( E.TShape().get() );
    const BRep_ListOfCurveRepresentation& listOfCurves = TE->Curves();

    // Check if there is at least one p-curve. If not, we cannot
    // reconstruct 3D curve
    bool hasAnyPCurves = false;
    for ( BRep_ListIteratorOfListOfCurveRepresentation cit(listOfCurves); cit.More(); cit.Next() )
    {
      const Handle(BRep_GCurve)&
        fromGC = Handle(BRep_GCurve)::DownCast( cit.Value() );
      //
      if ( fromGC.IsNull() ) continue;
      if ( fromGC->IsCurveOnSurface() )
      {
        hasAnyPCurves = true;
        break;
      }
    }

    if ( hasAnyPCurves )
    {
      // Rebuild 3D representation
      ShapeBuild_Edge().RemoveCurve3d(E);
      ShapeFix_Edge().FixAddCurve3d(E);
    }
  }
}

//-----------------------------------------------------------------------------

TopoDS_Edge asiAlgo_Utils::GetCommonEdge(const TopoDS_Shape&         F,
                                         const TopoDS_Shape&         G,
                                         TopTools_IndexedMapOfShape& allCommonEdges,
                                         const TopoDS_Vertex&        hint)
{
  TopoDS_Edge commonEdge;

  // Extract edges for faces.
  TopTools_IndexedMapOfShape EdgesF, EdgesG;
  TopExp::MapShapes(F, TopAbs_EDGE, EdgesF);
  TopExp::MapShapes(G, TopAbs_EDGE, EdgesG);

  // Collect common edges.
  for ( int ef = 1; ef <= EdgesF.Extent(); ++ef )
  {
    for ( int eg = 1; eg <= EdgesG.Extent(); ++eg )
    {
      if ( EdgesF(ef).IsSame( EdgesG(eg) ) )
      {
        allCommonEdges.Add( EdgesF(ef) );
        //
        if ( commonEdge.IsNull() )
        {
          const TopoDS_Edge& candidate = TopoDS::Edge( EdgesF(ef) );

          if ( !hint.IsNull() )
          {
            // Use hint vertex to select an edge to return.
            TopoDS_Vertex vf, vl;
            TopExp::Vertices(candidate, vf, vl);
            //
            if ( vf.IsPartner(hint) || vl.IsPartner(hint) )
              commonEdge = candidate;
          }
          else
            commonEdge = candidate; // Take just first one.
        }
      }
    }
  }
  return commonEdge;
}

//-----------------------------------------------------------------------------

TopoDS_Edge asiAlgo_Utils::GetCommonEdge(const TopoDS_Shape& F,
                                         const TopoDS_Shape& G)
{
  TopTools_IndexedMapOfShape commonEdges;
  return GetCommonEdge(F, G, commonEdges);
}

//-----------------------------------------------------------------------------

TopoDS_Edge asiAlgo_Utils::GetCommonEdge(const TopoDS_Shape&  F,
                                         const TopoDS_Shape&  G,
                                         const TopoDS_Vertex& hint)
{
  TopTools_IndexedMapOfShape commonEdges;
  return GetCommonEdge(F, G, commonEdges, hint);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::GetCommonEdges(const TopoDS_Shape&         F,
                                   const TopoDS_Vertex&        V,
                                   TopTools_IndexedMapOfShape& edges)
{
  TopTools_IndexedDataMapOfShapeListOfShape M;
  TopExp::MapShapesAndAncestors(F, TopAbs_VERTEX, TopAbs_EDGE, M);

  bool isAnyFound = false;
  for ( int k = 1; k <= M.Extent(); ++k )
  {
    const TopoDS_Shape& currV = M.FindKey(k);
    //
    if ( !currV.IsSame(V) )
      continue;

    // Add all edges sharing the vertex `V` within the face `F` to the result.
    const TopTools_ListOfShape& currEdges = M.FindFromIndex(k);
    //
    for ( TopTools_ListIteratorOfListOfShape lit(currEdges); lit.More(); lit.Next() )
    {
      edges.Add( lit.Value() );
      if ( !isAnyFound ) isAnyFound = true;
    }
  }

  return isAnyFound;
}

//-----------------------------------------------------------------------------

TopoDS_Vertex asiAlgo_Utils::GetCommonVertex(const TopoDS_Shape& F,
                                             const TopoDS_Shape& G,
                                             const TopoDS_Shape& H)
{
  TopoDS_Vertex commonVertex;

  // Extract vertices.
  TopTools_IndexedMapOfShape VerticesF, VerticesG, VerticesH;
  TopExp::MapShapes(F, TopAbs_VERTEX, VerticesF);
  TopExp::MapShapes(G, TopAbs_VERTEX, VerticesG);
  TopExp::MapShapes(H, TopAbs_VERTEX, VerticesH);

  // Collect common vertices.
  bool isDone = false;
  for ( int vf = 1; vf <= VerticesF.Extent(); ++vf )
  {
    for ( int vg = 1; vg <= VerticesG.Extent(); ++vg )
    {
      for ( int vh = 1; vh <= VerticesH.Extent(); ++vh )
      {
        if ( VerticesF(vf).IsSame( VerticesG(vg) ) &&
             VerticesG(vg).IsSame( VerticesH(vh) ) )
        {
          if ( commonVertex.IsNull() ) // A single common vertex is returned.
          {
            commonVertex = TopoDS::Vertex( VerticesF(vf) );
            isDone = true;
            break;
          }
        }
      }
      if ( isDone ) break;
    }
    if ( isDone ) break;
  }
  return commonVertex;
}

//-----------------------------------------------------------------------------

TopoDS_Vertex
  asiAlgo_Utils::GetCommonVertex(const TopoDS_Shape& F,
                                 const TopoDS_Shape& G)
{
  TopoDS_Vertex commonVertex;

  // Extract vertices.
  TopTools_IndexedMapOfShape VerticesF, VerticesG;
  TopExp::MapShapes(F, TopAbs_VERTEX, VerticesF);
  TopExp::MapShapes(G, TopAbs_VERTEX, VerticesG);

  // Collect common vertices.
  bool isDone = false;
  for ( int vf = 1; vf <= VerticesF.Extent(); ++vf )
  {
    for ( int vg = 1; vg <= VerticesG.Extent(); ++vg )
    {
      if ( VerticesF(vf).IsSame( VerticesG(vg) ) )
      {
        if ( commonVertex.IsNull() ) // A single common vertex is returned.
        {
          commonVertex = TopoDS::Vertex( VerticesF(vf) );
          isDone = true;
          break;
        }
      }
      if ( isDone ) break;
    }
    if ( isDone ) break;
  }
  return commonVertex;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::GetNeighborsThru(const TopoDS_Shape&         shape,
                                     const TopoDS_Face&          F,
                                     const TopoDS_Edge&          E,
                                     TopTools_IndexedMapOfShape& M)
{
  // Get all edges with their owner faces in the master shape.
  TopTools_IndexedDataMapOfShapeListOfShape edgesFacesMap;
  TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edgesFacesMap);

  // Get all faces owning the edge in question.
  if ( !edgesFacesMap.Contains(E) )
    return false;
  //
  const TopTools_ListOfShape& ownerFaces = edgesFacesMap.FindFromKey(E);

  // Find all faces except F.
  bool isFfound = false;
  //
  for ( TopTools_ListIteratorOfListOfShape fit(ownerFaces); fit.More(); fit.Next() )
  {
    const TopoDS_Shape& currFace = fit.Value();
    //
    if ( currFace.IsPartner(F) )
    {
      isFfound = true;
      continue;
    }

    M.Add(currFace);
  }

  return isFfound;
}

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_Utils::JoinCurves(Handle(Geom_BSplineCurve)& curve1,
                               Handle(Geom_BSplineCurve)& curve2,
                               const int                  order,
                               Handle(Geom_BSplineCurve)& result,
                               ActAPI_ProgressEntry       progress)
{
  if ( curve1->Degree() != curve2->Degree() )
  {
    progress.SendLogMessage(LogErr(Normal) << "Cannot join curves of different degrees.");
    return false;
  }
  //
  const int degree = curve1->Degree();

  // Convert curve 1 to Mobius form.
  const t_ptr<t_bcurve>&
    mobCurve1 = cascade::GetMobiusBCurve(curve1);

  // Convert curve 2 to Mobius form.
  const t_ptr<t_bcurve>&
    mobCurve2 = cascade::GetMobiusBCurve(curve2);

  // Get common vector of poles.
  std::vector<t_xyz> poles;
  //
  for ( int k = 0; k < mobCurve1->GetNumOfPoles(); ++k )
    poles.push_back( mobCurve1->GetPole(k) );
  //
  for ( int k = 1; k < mobCurve2->GetNumOfPoles(); ++k )
    poles.push_back( mobCurve2->GetPole(k) );

  // Prepare knots.
  std::vector<double> U;
  //
  for ( int k = 0; k < mobCurve1->GetNumOfKnots() - 1; ++k )
    U.push_back( mobCurve1->GetKnot(k) );
  //
  const double U1max = U[U.size() - 1];
  //
  for ( int k = degree + 1; k < mobCurve2->GetNumOfKnots(); ++k )
    U.push_back( U1max + mobCurve2->GetKnot(k) );

  if ( order )
  {
    // TODO: perform knot removal here...
  }

  t_ptr<t_bcurve>
    mobResult = new t_bcurve(poles, U, degree);

  // Convert result to OpenCascade form.
  result = cascade::GetOpenCascadeBCurve(mobResult);

  return true;
}
#else
bool asiAlgo_Utils::JoinCurves(Handle(Geom_BSplineCurve)&,
                               Handle(Geom_BSplineCurve)&,
                               const int,
                               Handle(Geom_BSplineCurve)&,
                               ActAPI_ProgressEntry progress)
{
  progress.SendLogMessage(LogErr(Normal) << "This function is not available (USE_MOBIUS is off).");
  return false;
}
#endif

//-----------------------------------------------------------------------------

#if defined USE_MOBIUS
bool asiAlgo_Utils::JoinCurves(std::vector<Handle(Geom_BSplineCurve)>& curves,
                               const int                               order,
                               Handle(Geom_BSplineCurve)&              result,
                               ActAPI_ProgressEntry                    progress)
{
  const size_t numCurves = curves.size();
  //
  if ( numCurves < 2 )
    return false;

  size_t k = 0;
  Handle(Geom_BSplineCurve) curve1 = curves[k++];
  //
  do
  {
    // Get next curve.
    Handle(Geom_BSplineCurve) curve2 = curves[k];

    if ( !JoinCurves(curve1, curve2, order, curve1, progress) )
      return false;

    ++k;
  }
  while ( k < numCurves );

  // Set the result.
  result = curve1;
  return true;
}
#else
bool asiAlgo_Utils::JoinCurves(std::vector<Handle(Geom_BSplineCurve)>&,
                               const int,
                               Handle(Geom_BSplineCurve)&,
                               ActAPI_ProgressEntry progress)
{
  progress.SendLogMessage(LogErr(Normal) << "This function is not available (USE_MOBIUS is off).");
  return false;
}
#endif

//-----------------------------------------------------------------------------

void asiAlgo_Utils::MapShapes(const TopoDS_Shape&         S,
                              asiAlgo_DataMapOfShape&     M,
                              int&                        startIdx,
                              TopTools_IndexedMapOfShape& IM)
{
  if ( !IM.Contains(S) )
  {
    IM.Add(S);
    M.Bind(startIdx++, S);
  }

  TopoDS_Iterator It(S);
  //
  while ( It.More() )
  {
    MapShapes(It.Value(), M, startIdx, IM);
    It.Next();
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::MapShapes(const TopoDS_Shape&     S,
                              asiAlgo_DataMapOfShape& M)
{
  int startIdx = 1;
  TopTools_IndexedMapOfShape IM;
  //
  MapShapes(S, M, startIdx, IM);
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::MapTShapes(const TopoDS_Shape&         S,
                               asiAlgo_IndexedMapOfTShape& M)
{
  M.Add(S);
  TopoDS_Iterator It(S);
  //
  while ( It.More() )
  {
    MapTShapes(It.Value(), M);
    It.Next();
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::MapTShapes(const TopoDS_Shape&         S,
                               const TopAbs_ShapeEnum      T,
                               asiAlgo_IndexedMapOfTShape& M)
{
  TopExp_Explorer Ex(S, T);
  while ( Ex.More() )
  {
    M.Add( Ex.Current() );
    Ex.Next();
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::MapTShapesAndAncestors(const TopoDS_Shape&                        S,
                                           const TopAbs_ShapeEnum                     TS,
                                           const TopAbs_ShapeEnum                     TA,
                                           asiAlgo_IndexedDataMapOfTShapeListOfShape& M)
{
  TopTools_ListOfShape empty;

  // Visit ancestors.
  TopExp_Explorer exa(S,TA);
  while ( exa.More() )
  {
    // Visit shapes.
    const TopoDS_Shape& anc = exa.Current();
    TopExp_Explorer exs(anc, TS);
    while ( exs.More() )
    {
      int index = M.FindIndex( exs.Current() );
      if ( index == 0 ) index = M.Add(exs.Current(), empty);
      M(index).Append(anc);
      exs.Next();
    }
    exa.Next();
  }

  // Visit shapes not under ancestors.
  TopExp_Explorer ex(S, TS, TA);
  while ( ex.More() )
  {
    int index = M.FindIndex( ex.Current() );
    if ( index == 0 ) index = M.Add(ex.Current(), empty);
    ex.Next();
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::HasInternalLocations(const TopoDS_Shape&    S,
                                         const TopAbs_ShapeEnum ST)
{
  // Iterate over the sub-shapes.
  for ( TopoDS_Iterator it(S, false, false); it.More(); it.Next() )
  {
    const TopoDS_Shape& subShape = it.Value();

    if ( ST == TopAbs_SHAPE || subShape.ShapeType() == ST )
    {
      if ( !subShape.Location().IsIdentity() )
        return true;
    }

    if ( HasInternalLocations(subShape, ST) ) // Go deeper.
      return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::IsolateRealParts(const TopoDS_Shape&   S,
                                     TopTools_ListOfShape& parts)
{
  for ( TopoDS_Iterator it(S, false, false); it.More(); it.Next() )
  {
    const TopoDS_Shape& subShape = it.Value();

    if ( subShape.Location().IsIdentity() )
      parts.Append( S.Located( TopLoc_Location() ) ); // Stop recursion and add the parent as a real part.
    else
      IsolateRealParts(subShape, parts); // Go deeper.
  }
}

//-----------------------------------------------------------------------------

TopoDS_Shape
  asiAlgo_Utils::FindBySubshape(const TopTools_ListOfShape& parts,
                                const TopoDS_Shape&         subshape)
{
  for ( TopTools_ListIteratorOfListOfShape lit(parts); lit.More(); lit.Next() )
  {
    const TopoDS_Shape& currPart = lit.Value();

    // Try to find the passed shape as a subshape.
    asiAlgo_IndexedMapOfTShape M;
    MapTShapes(currPart, subshape.ShapeType(), M);
    //
    if ( M.Contains(subshape) )
      return currPart;
  }

  return TopoDS_Shape(); // Not found.
}

//-----------------------------------------------------------------------------

Handle(Poly_Triangulation)
  asiAlgo_Utils::GetSubMesh(const Handle(Poly_Triangulation)& mesh,
                            const TColStd_PackedMapOfInteger& elems,
                            const bool                        oneBased)
{
  // Prepare triangulation.
  std::vector<Poly_Triangle> selectedTrisVec;
  std::vector<gp_Pnt>        selectedTrisNodesVec;
  //
  for ( TColStd_MapIteratorOfPackedMapOfInteger eit(elems); eit.More(); eit.Next() )
  {
    const int tid = eit.Key();
    //
    if ( tid == -1 )
      continue;

    const Poly_Triangle& triangle = mesh->Triangle(oneBased ? tid : tid + 1);

    int n1, n2, n3;
    triangle.Get(n1, n2, n3);

    const gp_Pnt& P1 = mesh->Node(n1);
    const gp_Pnt& P2 = mesh->Node(n2);
    const gp_Pnt& P3 = mesh->Node(n3);

    // Populate the new collections of nodes and triangles.
    int k = int( selectedTrisNodesVec.size() ) + 1;
    //
    selectedTrisNodesVec.push_back(P1);
    selectedTrisNodesVec.push_back(P2);
    selectedTrisNodesVec.push_back(P3);
    //
    selectedTrisVec.push_back( Poly_Triangle(k, k + 1, k + 2) );
  }

  const int numSelectedNodes = int( selectedTrisNodesVec.size() );
  const int numSelectedTris  = int( selectedTrisVec.size() );

  // Populate new array of mesh nodes.
  TColgp_Array1OfPnt selectedNodesArr(1, numSelectedNodes);
  for ( int i = 1; i <= numSelectedNodes; ++i )
    selectedNodesArr.ChangeValue(i) = selectedTrisNodesVec[i - 1];

  // Populate new array of mesh elements.
  Poly_Array1OfTriangle selectedTrisArr(1, numSelectedTris);
  for ( int i = 1; i <= numSelectedTris; ++i )
    selectedTrisArr.ChangeValue(i) = selectedTrisVec[i - 1];

  // Create new triangulation and return.
  Handle(Poly_Triangulation)
    selectedTris = new Poly_Triangulation(selectedNodesArr, selectedTrisArr);

  return selectedTris;
}

//-----------------------------------------------------------------------------

gp_XYZ asiAlgo_Utils::ComputeAveragePoint(const std::vector<gp_XYZ>& pts)
{
  // Get center point.
  gp_XYZ P_center;
  //
  for ( size_t k = 0; k < pts.size(); ++k )
  {
    P_center += pts[k];
  }
  //
  P_center /= int( pts.size() );

  return P_center;
}

//-----------------------------------------------------------------------------

TopoDS_Wire asiAlgo_Utils::OuterWire(const TopoDS_Face& face)
{
  const double prec = Precision::PConfusion();

  TopoDS_Wire Wres;
  TopExp_Explorer expw(face, TopAbs_WIRE);
  //
  if ( expw.More() )
  {
    Wres = TopoDS::Wire( expw.Current() );
    expw.Next();
    if ( expw.More() )
    {
      double UMin, UMax, VMin, VMax;
      double umin, umax, vmin, vmax;
      BRepTools::UVBounds(face, Wres, UMin, UMax, VMin, VMax);

      while ( expw.More() )
      {
        const TopoDS_Wire& W = TopoDS::Wire( expw.Current() );
        BRepTools::UVBounds(face, W, umin, umax, vmin, vmax);

        if ( (umin < UMin || Abs(umin - UMin) < prec) &&
             (umax > UMax || Abs(umax - UMax) < prec) &&
             (vmin < VMin || Abs(vmin - VMin) < prec) &&
             (vmax > VMax || Abs(vmax - VMax) < prec) )
        {
          Wres = W;
          UMin = umin;
          UMax = umax;
          VMin = vmin;
          VMax = vmax;
        }
        expw.Next();
      }
    }
  }
  return Wres;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::GetRandomPoint(const TopoDS_Face&     face,
                                   math_BullardGenerator& RNG,
                                   gp_Pnt2d&              uv)
{
  // Prepare classification utility.
  BRepClass_FClassifier faceClass2d;

  // Prepare face adaptor to initialize the classifier.
  BRepClass_FaceExplorer fe(face);

  // Sample face iteratively.
  gp_Pnt2d  sample;
  bool      isOk     = false;
  bool      stop     = false;
  int       numIters = 0;
  const int maxIters = 100;
  //
  do
  {
    ++numIters;
    //
    if ( numIters > maxIters )
    {
      stop = true;
      continue; // Escape from the loop.
    }

    // Get a random point from the face's bounded area.
    double umin, umax, vmin, vmax;
    BRepTools::UVBounds(face, umin, umax, vmin, vmax);
    //
    sample.SetX( umin + (umax - umin)*RNG.NextReal() );
    sample.SetY( vmin + (vmax - vmin)*RNG.NextReal() );

    faceClass2d.Perform(fe, sample, 1.0e-4);

    // Check if a point is inside the face.
    if ( faceClass2d.State() == TopAbs_IN )
      isOk = stop = true;

  }
  while ( !stop );

  // Initialize the result.
  if ( isOk )
    uv = sample;

  return isOk;
}


//-----------------------------------------------------------------------------

bool asiAlgo_Utils::GetLocalFrame(const TopoDS_Face& face,
                                  const bool         alongEdges,
                                  gp_Ax3&            axes)
{
  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);

  // Get midpoint.
  const double uMid = (uMin + uMax)*0.5;
  const double vMid = (vMin + vMax)*0.5;

  // Evaluate surface.
  gp_Pnt P;
  gp_Vec Du, Dv, DX, DY;
  BRepAdaptor_Surface bas(face);
  bas.D1(uMid, vMid, P, Du, Dv);

  const double prec = 1.e-3;

  if ( alongEdges )
  {
    // Take all straight edges.
    TopTools_IndexedMapOfShape faceEdges;
    TopExp::MapShapes(face, TopAbs_EDGE, faceEdges);
    //
    std::vector<TopoDS_Edge> straightEdges;
    std::vector<double>      straightEdgesLen;
    std::vector<int>         straightEdgesIds;
    //
    for ( int eidx = 1; eidx <= faceEdges.Extent(); ++eidx )
    {
      const TopoDS_Edge& faceEdge = TopoDS::Edge( faceEdges(eidx) );

      // Check curve type.
      double f, l;
      Handle(Geom_Curve)
        c3d = BRep_Tool::Curve(faceEdge, f, l);
      //
      if ( c3d.IsNull() || !c3d->IsKind( STANDARD_TYPE(Geom_Line) ) )
        continue;

      straightEdges    .push_back( faceEdge );
      straightEdgesLen .push_back( l-f );
      straightEdgesIds .push_back( int( straightEdgesIds.size() ) );
    }

    // Select the longest edge.
    bool isEdgeSelected = false;
    if ( straightEdgesIds.size() )
    {
      // Sort edges by descending lengths.
      std::sort( straightEdgesIds.begin(), straightEdgesIds.end(),
                 [&](const int a, const int b)
                 {
                   return straightEdgesLen[a] > straightEdgesLen[b];
                 } );

      double f, l;
      Handle(Geom_Curve)
        c3d = BRep_Tool::Curve(straightEdges[straightEdgesIds[0]], f, l);

      c3d->D1( (f + l)*0.5, P, DX );

      if ( DX.Magnitude() > prec )
        isEdgeSelected = true;
    }

    if ( !isEdgeSelected )
    {
      DX = Du;
      DY = Dv;
    }
    else
    {
      // Select the second direction to compute the vector product at the end.
      const double ang = Abs( DX.Angle(Du) );
      //
      if ( ang > prec && Abs(ang - M_PI) > prec )
        DY = Du;
      else
        DY = Dv;
    }
  }
  else
  {
    DX = Du;
    DY = Dv;
  }

  // Prepare axes.
  axes = gp_Ax3(P, DX^DY, DX);

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ComputeBorderTrihedron(const TopoDS_Face&       face,
                                           const TopoDS_Edge&       edge,
                                           const double             t,
                                           asiAlgo_BorderTrihedron& btri,
                                           ActAPI_ProgressEntry     progress)
{
  // Check whether the edge of interest possesses "same parameter" property.
  // If so, we may use its host spatial curves in conjunction with p-curves
  // to sample points on adjacent surfaces. If not, then direction evaluation
  // is not legal anymore, so point inversion problem has to be solved. The
  // latter is quite heavy operation, so it is good that we run it for
  // invalid model only.
  const bool isSameParam = BRep_Tool::SameParameter(edge) && BRep_Tool::SameRange(edge);

  // Get orientation of co-edge on the face of interest.
  TopAbs_Orientation CumulOriOnFace = TopAbs_EXTERNAL;
  //
  for ( TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    if ( exp.Current().IsSame(edge) )
    {
      CumulOriOnFace = exp.Current().Orientation();
      break;
    }
  }

  // Get host (shared) curve.
  double f, l;
  Handle(Geom_Curve) probeCurve = BRep_Tool::Curve(edge, f, l);
  //
  if ( probeCurve.IsNull() )
  {
    progress.SendLogMessage( LogErr(Normal) << "Null host curve in the border edge." );
    return false;
  }

  // Pick up two points on the curve.
  const double midParam  = t;
  const double paramStep = (l - f)*0.01;
  const double A_param   = midParam;
  const double B_param   = midParam + paramStep;
  //
  gp_Pnt A = probeCurve->Value(A_param);
  gp_Pnt B = probeCurve->Value(B_param);

  //this->Plotter().DRAW_POINT(A, Color_Red,  "A");
  //this->Plotter().DRAW_POINT(B, Color_Blue, "B");

  // Get host surface.
  Handle(Geom_Surface) S = BRep_Tool::Surface(face);
  Handle(Geom2d_Curve) c2d;

  gp_Pnt2d UV;
  gp_Vec TF; // Tangent for F.

  // Vx.
  gp_Vec VX = B.XYZ() - A.XYZ();
  if ( VX.Magnitude() < RealEpsilon() )
  {
    progress.SendLogMessage( LogErr(Normal) << "Zero Vx." );
    return false;
  }
  //
  if ( CumulOriOnFace == TopAbs_REVERSED )
    VX.Reverse();

  //this->Plotter().DRAW_VECTOR_AT(A, VX, Color_Red, "Vx");

  // Say (u, v) is the parametric space of S. Now we take parameters
  // of the point A on this surface.
  if ( isSameParam )
  {
    double cons_f, cons_l;
    c2d = BRep_Tool::CurveOnSurface(edge, face, cons_f, cons_l);
    //
    if ( c2d.IsNull() )
    {
      progress.SendLogMessage( LogErr(Normal) << "Null p-curve in the border edge." );
      return false; // Face is invalid.
    }

    UV = c2d->Value(A_param);
  }
  else
  {
    ShapeAnalysis_Surface SAS(S);
    UV = SAS.ValueOfUV(A, 1.0e-7);
  }

  // N (Vz).
  gp_Pnt P;
  gp_Vec D1U, D1V;
  S->D1(UV.X(), UV.Y(), P, D1U, D1V);
  //
  gp_Vec N = D1U.Crossed(D1V);
  if ( N.Magnitude() < RealEpsilon() )
  {
    progress.SendLogMessage( LogErr(Normal) << "Zero Vz." );
    return false;
  }
  //
  if ( face.Orientation() == TopAbs_REVERSED )
    N.Reverse();

  //this->Plotter().DRAW_VECTOR_AT(A, N, Color_Blue, "Vz");

  // Vy.
  gp_Vec VY = N.Crossed(VX);
  if ( VY.Magnitude() < RealEpsilon() )
  {
    progress.SendLogMessage( LogErr(Normal) << "Zero Vy." );
    return false;
  }

  // Set output.
  btri.V_origin = A;
  btri.V_x      = VX;
  btri.V_y      = VY;
  btri.V_z      = N;
  //
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ComputeBorderTrihedron(const TopoDS_Face&       face,
                                           const TopoDS_Edge&       edge,
                                           asiAlgo_BorderTrihedron& btri,
                                           ActAPI_ProgressEntry     progress)
{
  // Get host (shared) curve.
  double f, l;
  Handle(Geom_Curve) probeCurve = BRep_Tool::Curve(edge, f, l);
  //
  if ( probeCurve.IsNull() )
  {
    progress.SendLogMessage( LogErr(Normal) << "Null host curve in the border edge." );
    return false;
  }

  // Pick up two points on the curve.
  const double midParam  = (f + l)*0.5;
  return ComputeBorderTrihedron(face, edge, midParam, btri, progress);
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::GeomSummary(const TopoDS_Shape&  shape,
                                asiAlgo_GeomSummary& summary)
{
  summary.Reset();

  TopTools_IndexedMapOfShape allFaces, allEdges;
  //
  TopExp::MapShapes(shape, TopAbs_FACE, allFaces);
  TopExp::MapShapes(shape, TopAbs_EDGE, allEdges);

  // Check surfaces.
  for ( int fidx = 1; fidx <= allFaces.Extent(); ++fidx )
  {
    const TopoDS_Face& face = TopoDS::Face( allFaces(fidx) );

    if ( IsTypeOf<Geom_BezierSurface>(face) )
      summary.nbSurfBezier++;
    else if ( IsTypeOf<Geom_BSplineSurface>(face) )
      summary.nbSurfSpl++;
    else if ( IsTypeOf<Geom_ConicalSurface>(face) )
      summary.nbSurfConical++;
    else if ( IsTypeOf<Geom_CylindricalSurface>(face) )
      summary.nbSurfCyl++;
    else if ( IsTypeOf<Geom_OffsetSurface>(face) )
      summary.nbSurfOffset++;
    else if ( IsTypeOf<Geom_SphericalSurface>(face) )
      summary.nbSurfSph++;
    else if ( IsTypeOf<Geom_SurfaceOfLinearExtrusion>(face) )
      summary.nbSurfLinExtr++;
    else if ( IsTypeOf<Geom_SurfaceOfRevolution>(face) )
      summary.nbSurfOfRevol++;
    else if ( IsTypeOf<Geom_ToroidalSurface>(face) )
      summary.nbSurfToroidal++;
    else if ( IsTypeOf<Geom_Plane>(face) )
      summary.nbSurfPlane++;
  }

  // Check curves.
  for ( int eidx = 1; eidx <= allEdges.Extent(); ++eidx )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( allEdges(eidx) );

    if ( IsTypeOf<Geom_BezierCurve>(edge) )
      summary.nbCurveBezier++;
    else if ( IsTypeOf<Geom_BSplineCurve>(edge) )
      summary.nbCurveSpline++;
    else if ( IsTypeOf<Geom_Circle>(edge) )
      summary.nbCurveCircle++;
    else if ( IsTypeOf<Geom_Ellipse>(edge) )
      summary.nbCurveEllipse++;
    else if ( IsTypeOf<Geom_Hyperbola>(edge) )
      summary.nbCurveHyperbola++;
    else if ( IsTypeOf<Geom_Line>(edge) )
      summary.nbCurveLine++;
    else if ( IsTypeOf<Geom_OffsetCurve>(edge) )
      summary.nbCurveOffset++;
    else if ( IsTypeOf<Geom_Parabola>(edge) )
      summary.nbCurveParabola++;
  }
}

//-----------------------------------------------------------------------------

double asiAlgo_Utils::MinArcAngle(const std::vector<gp_Vec>& dirs,
                                  const gp_Dir&              norm)
{
  if ( dirs.empty() || dirs.size() == 1 )
    return 0.;

  const  gp_Dir& ref = dirs[0];
  double positiveAng = 0.;
  double negativeAng = 0.;

  for ( size_t k = 1; k < dirs.size(); ++k )
  {
    const double ang = ref.AngleWithRef(dirs[k], norm);

    // PI can be attributed as both positive and negative.
    bool isPi = false;
    //
    if ( Abs(Abs(ang) - M_PI) < Precision::Angular() )
      isPi = true;
    //
    if ( isPi )
    {
      positiveAng =  M_PI;
      negativeAng = -M_PI;
      break;
    }

    if ( ang > positiveAng )
      positiveAng = ang;

    if ( ang < negativeAng )
      negativeAng = ang;
  }

  return positiveAng - negativeAng;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::AreCoaxial(const gp_Ax1& a1,
                               const gp_Ax1& a2,
                               const double  angTolerDeg,
                               const double  linToler)
{
  if ( a1.IsCoaxial(a2, angTolerDeg, linToler) ||
       a1.IsCoaxial(a2.Reversed(), angTolerDeg, linToler) )
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsOnCylinder(const Handle(Geom_Curve)& curve,
                                 const gp_Ax1&             axis,
                                 const double              tol,
                                 double&                   r)
{
  gp_Lin axLin(axis);

  // Check each curve separately.
  std::vector<double> dists;
  //
  const double cf    = curve->FirstParameter();
  const double cl    = curve->LastParameter();
  const double cm    = (cf + cl)*0.5;
  const double delta = Abs(cl - cf)*0.1;

  // Discretisation of a curve. We take midpoint's vicinity as
  // the extreme points of a thread's helix might go outside the
  // cylindrical bore.
  std::vector<double>
    cParams = {cm - delta, cm, cm + delta};

  // Measure distances to the axis.
  for ( auto p : cParams )
  {
    dists.push_back( axLin.Distance( curve->Value(p) ) );
  }

  // Check distances.
  const double refDist = dists[0];
  for ( size_t k = 1; k < dists.size(); ++k )
  {
    if ( Abs(dists[k] - refDist) > tol )
    {
      return false;
    }
  }

  r = refDist;
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsOnCylinder(const std::vector<Handle(Geom_Curve)>& curves,
                                 const gp_Ax1&                          axis,
                                 const double                           tol,
                                 double&                                r)
{
  gp_Lin axLin(axis);

  // Check each curve separately.
  std::vector<double> dists;
  //
  for ( const auto& curve : curves )
  {
    const double cf    = curve->FirstParameter();
    const double cl    = curve->LastParameter();
    const double cm    = (cf + cl)*0.5;
    const double delta = Abs(cl - cf)*0.1;

    // Discretisation of a curve. We take midpoint's vicinity as
    // the extreme points of a thread's helix might go outside the
    // cylindrical bore.
    std::vector<double>
      cParams = {cm - delta, cm, cm + delta};

    // Measure distances to the axis.
    for ( auto p : cParams )
    {
      dists.push_back( axLin.Distance( curve->Value(p) ) );
    }
  } // loop over curves

  // Check distances.
  const double refDist = dists[0];
  for ( size_t k = 1; k < dists.size(); ++k )
  {
    if ( Abs(dists[k] - refDist) > tol )
    {
      return false;
    }
  }

  r = refDist;
  return true;
}

//-----------------------------------------------------------------------------

//! Checks whether the passed curve lies on the given cylindrical
//! surface `cyl`.
bool asiAlgo_Utils::IsOnCylinder(const Handle(Geom_Curve)& curve,
                                 const gp_Cylinder&        cyl,
                                 const double              tol,
                                 ActAPI_PlotterEntry       plotter)
{
  gp_Lin axLin( cyl.Axis() );

  const double cf    = curve->FirstParameter();
  const double cl    = curve->LastParameter();
  const double cm    = (cf + cl)*0.5;
  const double delta = Abs(cl - cf)*0.1;

  // Discretisation of a curve. We take midpoint's vicinity as
  // the extreme points of a thread's helix might go outside the
  // cylindrical bore.
  std::vector<double>
    cParams = {cm - delta, cm, cm + delta};

  // Measure distances to the cylinder.
  std::vector<double> dists;
  //
  for ( auto p : cParams )
  {
    dists.push_back( axLin.Distance( curve->Value(p) ) - cyl.Radius() );
  }

  // Check distances.
  for ( size_t k = 0; k < dists.size(); ++k )
  {
    if ( Abs(dists[k]) > tol )
    {
#if defined DRAW_DEBUG
      plotter.DRAW_POINT(curve->Value(cParams[k]), Color_Red, "deviationPoint");
#endif
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::GetFacePoints(const TopoDS_Face&   face,
                                  const bool           midPoints,
                                  const bool           triangPoints,
                                  std::vector<gp_XYZ>& pts)
{
  // Add triangulation points. Triangulation
  // also includes the original vertices.
  if ( triangPoints )
  {
    TopLoc_Location loc;

    const Handle(Poly_Triangulation)&
      poly = BRep_Tool::Triangulation(face, loc);

    if ( !poly.IsNull() )
    {
      for ( int iNode = 1; iNode <= poly->NbNodes(); ++iNode )
      {
        // Make sure to apply location.
        gp_XYZ P = poly->Node(iNode).Transformed(loc).XYZ();
        pts.push_back(P);
      }
    }
  }
  else
  {
    TopTools_IndexedMapOfShape faceVerts;
    TopExp::MapShapes(face, TopAbs_VERTEX, faceVerts);
    //
    for ( int v = 1; v <= faceVerts.Extent(); ++v )
      pts.push_back(BRep_Tool::Pnt(TopoDS::Vertex(faceVerts(v))).XYZ());
  }

  // Add midpoints from edges.
  if ( midPoints )
  {
    TopTools_IndexedMapOfShape faceEdges;
    TopExp::MapShapes(face, TopAbs_EDGE, faceEdges);
    //
    for ( int e = 1; e <= faceEdges.Extent(); ++e )
    {
      BRepAdaptor_Curve bac( TopoDS::Edge( faceEdges(e) ) );
      //
      const double f = bac.FirstParameter();
      const double l = bac.LastParameter();

      std::vector<double> params;
      params.push_back( (f + l) * 0.5 );

      if ( bac.GetType() == GeomAbs_Circle )
      {
        params.push_back( (3*f + l) * 0.25 );
        params.push_back( (f + 3*l) * 0.25 );
      }

      for ( const auto p : params )
        pts.push_back( bac.Value(p).XYZ() );
    }
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::GetFacePointsByFacets(const TopoDS_Face&                     face,
                                          const double                           tol,
                                          std::vector< std::pair<int, gp_Ax1> >& samples)
{
  TopLoc_Location                   L;
  const Handle(Poly_Triangulation)& T = BRep_Tool::Triangulation(face, L);
  //
  if ( T.IsNull() )
  {
    return false;
  }

  // Collect from mesh.
  for ( int itri = 1; itri <= T->NbTriangles(); ++itri )
  {
    const Poly_Triangle& tri = T->Triangle(itri);

    int nodes[3];
    tri.Get(nodes[0], nodes[1], nodes[2]);

    gp_XYZ P0 = T->Node(nodes[0]).Transformed(L).XYZ();
    gp_XYZ P1 = T->Node(nodes[1]).Transformed(L).XYZ();
    gp_XYZ P2 = T->Node(nodes[2]).Transformed(L).XYZ();
    gp_XYZ Pm = (P0 + P1 + P2)/3.;

    gp_Vec N;

    // Compute analytical norm whenever possible to avoid
    // mesh distortions.
    gp_Cylinder cyl;
    if ( asiAlgo_Utils::IsCylindrical(face, cyl) )
    {
      gp_Lin axLin( cyl.Axis() );
      //
      gp_XYZ       axLinDir = axLin.Direction().XYZ();
      gp_XYZ       O        = axLin.Location().XYZ();
      gp_XYZ       V1       = Pm - O;
      const double t        = V1.Dot(axLinDir);
      gp_XYZ       V2       = t*axLinDir;

      N = V1 - V2;
    }
    else
    {
      gp_Vec P0P1 = P1 - P0;
      gp_Vec P0P2 = P2 - P0;

      // Fallback on mesh.
      N = P0P1.Crossed(P0P2);
    }
    //
    if ( N.Magnitude() < gp::Resolution() )
      continue;

    N.Normalize();
    if ( face.Orientation() == TopAbs_REVERSED )
      N.Reverse();

    samples.push_back( {itri, gp_Ax1(Pm, N)} );
  }

  // Sparse the point cloud if requested.
  if ( tol > Precision::Confusion() )
  {
    std::vector< std::pair<int, gp_Ax1> > sparsed;
    asiAlgo_RelievePointCloud sparse;
    sparse(samples, tol, sparsed);
    //
    samples = sparsed;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsInternal(const TopoDS_Face& face,
                               const double       diameter,
                               const gp_Ax1&      ax)
{
  double uMin, uMax, vMin, vMax;
  BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);

  return IsInternal(face, diameter, (uMin + uMax)*0.5, (vMin + vMax)*0.5, ax);
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::IsInternal(const TopoDS_Face& face,
                               const double       diameter,
                               const double       u,
                               const double       v,
                               const gp_Ax1&      ax)
{
  BRepAdaptor_Surface bas(face);
  BRepLProp_SLProps lprops( bas, u, v, 1, Precision::Confusion() );
  //
  if ( !lprops.IsNormalDefined() )
    return false;

  const gp_Pnt& cylPt   = lprops.Value();
  gp_Dir        cylNorm = lprops.Normal();
  //
  if ( face.Orientation() == TopAbs_REVERSED )
    cylNorm.Reverse();

  // Take a probe point along the normal.
  gp_Pnt normProbe = cylPt.XYZ() + cylNorm.XYZ()*diameter*0.05;

  // Compute the distance to the axis.
  gp_Lin axisLin(ax);
  //
  const double probeDist = axisLin.Distance(normProbe);
  const double cylDist   = axisLin.Distance(cylPt);
  //
  if ( probeDist < cylDist )
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

Handle(Image_AlienPixMap)
  asiAlgo_Utils::Graphics::GeneratePixmap(const TopoDS_Shape&   shape,
                                          const int             width,
                                          const int             height,
                                          const Quantity_Color& shapeColor)
{
  Handle(Aspect_DisplayConnection)
    displayConnection = new Aspect_DisplayConnection();

  // don't waste the time waiting for VSync when window is not displayed on the screen
  OpenGl_Caps caps;
  caps.buffersNoSwap = true;

  Handle(OpenGl_GraphicDriver)
    graphicDriver = new OpenGl_GraphicDriver(displayConnection, false);
  //
  graphicDriver->ChangeOptions() = caps;
  graphicDriver->InitContext();

  //* Viewer setup
  Handle(V3d_Viewer) viewer = new V3d_Viewer(graphicDriver);
  Quantity_Color bgColor(255. / 255, 255. / 255, 255. / 255, Quantity_TOC_RGB);
  viewer->SetDefaultBackgroundColor(bgColor);

  // Lightning
  Handle(V3d_DirectionalLight) lightDir =
    new V3d_DirectionalLight(gp_Dir(0., 0., -1.),
                             Quantity_NOC_WHITE,
                             true);
  //
  viewer->AddLight(lightDir);
  viewer->SetLightOn(lightDir);

  Handle(V3d_View)
    view = new V3d_View(viewer, viewer->DefaultTypeOfView());
  view->SetImmediateUpdate(false);

#ifdef _WIN32
  /* Window - create a so called "virtual" WNT window that is a pure WNT window
     redefined to be never shown. */
  Handle(WNT_WClass) wClass = new WNT_WClass("GW3D_Class", (void*)DefWindowProcW,
                                             CS_VREDRAW | CS_HREDRAW, 0, 0,
                                             ::LoadCursor(NULL, IDC_ARROW));

  Handle(WNT_Window) win = new WNT_Window("",
                                          wClass,
                                          WS_POPUP,
                                          0, 0,
                                          width, height,
                                          Quantity_NOC_BLACK);

  win->SetVirtual(true);
  view->SetWindow(win);

#else // Linux
  Handle(Xw_Window) win = new Xw_Window(graphicDriver->GetDisplayConnection(),
                                        "",
                                        0, 0,
                                        width, height);
                                        win->SetVirtual(true);
                                        view->SetWindow(win);
#endif

  //* View setup
  view->SetWindow(win);
  view->SetComputedMode(false);
  view->SetProj(V3d_XposYnegZpos);
  view->AutoZFit();

  //* AIS context
  Handle(AIS_InteractiveContext) context = new AIS_InteractiveContext(viewer);
  context->SetDisplayMode(AIS_Shaded, false);
  context->DefaultDrawer()->SetFaceBoundaryDraw(true);

  // Render immediate structures into back buffer rather than front.
  view->View()->SetImmediateModeDrawToFront(false);

  //* Dump
  Handle(Image_AlienPixMap) pixmap = new Image_AlienPixMap;

  Handle(AIS_Shape) shapePrs = new AIS_Shape(shape);
  shapePrs->SetColor(shapeColor);

  context->Display(shapePrs, false);
  view->FitAll(0.1, true);

  bool isOk = view->ToPixMap(*pixmap, width, height);

  return isOk ? pixmap : nullptr;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Graphics::GeneratePicture(const TopoDS_Shape&   shape,
                                              const int             width,
                                              const int             height,
                                              const std::string&    filename,
                                              const Quantity_Color& shapeColor)
{
  Handle(Image_AlienPixMap)
    pixmap = GeneratePixmap(shape, width, height, shapeColor);

  if (pixmap.IsNull())
    return false;

  return pixmap->Save(filename.c_str());
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Range::Contains(const t_range& range1,
                                    const t_range& range2,
                                    const bool     strict,
                                    const double   tol)
{
  if ( strict )
  {
    if ( (range1.second > range2.second) && (range1.first < range2.first) )
    {
      return true;
    }
  }
  else // Not strict.
  {
    const bool le = ( Abs(range1.first  - range2.first)  < tol );
    const bool re = ( Abs(range1.second - range2.second) < tol );

    if ( ((range1.second > range2.second) || re) &&
         ((range1.first < range2.first)   || le) )
    {
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Range::Coincide(const t_range& range1,
                                    const t_range& range2)
{
  if ( (Abs(range1.first - range2.first) < asiAlgo_RangeLinPrec) &&
       (Abs(range1.second - range2.second) < asiAlgo_RangeLinPrec) )
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Range::Overlap(const t_range& range1,
                                   const t_range& range2)
{
  // Partial overlapping.
  if ( ( (range1.second > range2.first)  || ( Abs(range1.second - range2.first)  < asiAlgo_RangeLinPrec ) ) &&
       ( (range1.first  < range2.first)  || ( Abs(range1.first  - range2.first)  < asiAlgo_RangeLinPrec ) ) &&
       ( (range1.second < range2.second) || ( Abs(range1.second - range2.second) < asiAlgo_RangeLinPrec ) ) )
  {
    return true;
  }

  // Inclusion/coincidence.
  if ( ( (range1.second < range2.second) || ( Abs(range1.second - range2.second) < asiAlgo_RangeLinPrec ) ) &&
       ( (range1.first  > range2.first)  || ( Abs(range1.first  - range2.first)  < asiAlgo_RangeLinPrec ) ) )
  {
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

asiAlgo_Utils::Range::t_range
  asiAlgo_Utils::Range::Merge(const t_range& range1,
                              const t_range& range2)
{
  return t_range( Min(range1.first,  range2.first),
                  Max(range1.second, range2.second) );
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Range::Intersect(const t_range& range,
                                     const double   hmin,
                                     const double   hmax,
                                     t_range&       res)
{
  const double rmin = range.first;
  const double rmax = range.second;

  // Self degenerated.
  if ( Abs(range.first - range.second) < asiAlgo_RangeLinPrec )
    return false;

  // Entirely on the left.
  if ( (rmin < hmin) && ( (rmax < hmin) || Abs(rmax - hmin) < asiAlgo_RangeLinPrec) )
    return false;

  // Entirely on the right.
  if ( (rmax > hmax) && ( (rmin > hmax) || Abs(rmin - hmax) < asiAlgo_RangeLinPrec) )
    return false;

  // Left corner is inside the range and right corner is outside the range.
  if ( ( (rmin < hmax) && ( (rmin > hmin) || Abs(rmin - hmin) < asiAlgo_RangeLinPrec) ) &&
       ( (rmax > hmax) || Abs(rmax - hmax) < asiAlgo_RangeLinPrec ) )
  {
    res.first  = rmin;
    res.second = hmax;
    return true;
  }

  // Left corner is outside the range and right corner is inside the range.
  if ( ( (rmin < hmin) || Abs(rmin - hmin) < asiAlgo_RangeLinPrec ) &&
       ( (rmax > hmin) && ( (rmax < hmax) || Abs(rmax - hmax) < asiAlgo_RangeLinPrec) ) )
  {
    res.first  = hmin;
    res.second = rmax;
    return true;
  }

  // Both corners are in the range.
  if ( ( (rmin > hmin) || Abs(rmin - hmin) < asiAlgo_RangeLinPrec ) &&
       ( (rmax < hmax) || Abs(rmax - hmax) < asiAlgo_RangeLinPrec ) )
  {
    res.first  = rmin;
    res.second = rmax;
    return true;
  }

  // Both corners are outside the range.
  if ( ( (rmin < hmin) || Abs(rmin - hmin) < asiAlgo_RangeLinPrec ) &&
       ( (rmax > hmax) || Abs(rmax - hmax) < asiAlgo_RangeLinPrec ) )
  {
    res.first  = hmin;
    res.second = hmax;
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

void asiAlgo_Utils::Range::MergeRanges(const t_rangesByFaces& inputRanges,
                                       t_rangesByFaces&       merged)
{
  // Contract check.
  if ( inputRanges.empty() )
  {
    return;
  }

  t_rangesByFaces ranges = inputRanges;

  std::sort(ranges.begin(), ranges.end(), [&](const t_rangeByFace& a,
                                              const t_rangeByFace& b) {
    return a.second.first < b.second.first;
  });

  t_rangesByFaces result;

  int  i    = 0;
  bool stop = false;
  do
  {
    // Get the next range.
    t_range currRange = ranges[i].second;

    int  j   = i + 1;
    bool gap = false;

    if ( j == (int) ( ranges.size() ) )
    {
      stop = true;
      result.push_back({0, currRange});
    }
    else
    {
      do
      {
        // Get the next range.
        const t_range& nextRange = ranges[j].second;

        if ( Overlap(currRange, nextRange) || Overlap(nextRange, currRange) )
        {
          currRange = Merge(currRange, nextRange);
          j++;

          if ( j == (int) ( ranges.size() ) )
            stop = true;
        }
        else
        {
          gap = true;
          i   = j; // Proceed with the next range.
        }

        if ( gap || stop )
        {
          result.push_back({0, currRange});
        }
      }
      while ( !gap && !stop );

      if ( i == (int) ( ranges.size() ) )
        stop = true;
    }
  }
  while ( !stop );

  merged = result;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::Range::AreEqual(const t_rangesByFaces& ranges1,
                                    const t_rangesByFaces& ranges2)
{
  if ( ranges1.size() != ranges2.size() )
    return false;

  for ( size_t i = 0; i < ranges1.size(); ++i )
  {
    if ( ranges1[i].first != ranges2[i].first )
      return false;

    if ( Abs(ranges1[i].second.first - ranges2[i].second.first) > asiAlgo_RangeLinPrec )
      return false;

    if ( Abs(ranges1[i].second.second - ranges2[i].second.second) > asiAlgo_RangeLinPrec )
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_Utils::ProjectPointOnPlane(const Handle(Geom_Plane)& plane,
                                        const gp_Dir&             dir,
                                        const gp_Pnt&             point,
                                        gp_Pnt&                   proj)
{
  gp_Vec vec(point, plane->Location());

  Standard_Real alpha = vec * gp_Vec(plane->Axis().Direction());

  if ( Abs(dir * plane->Axis().Direction()) < Precision::SquareConfusion() )
  {
    return false;
  }

  alpha /= dir * plane->Axis().Direction();

  proj.SetXYZ(point.XYZ() + alpha * dir.XYZ());

  return true;
}
