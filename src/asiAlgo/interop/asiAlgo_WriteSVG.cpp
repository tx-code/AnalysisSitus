//-----------------------------------------------------------------------------
// Created on: 07 March 2022
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Kiselev
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
#include <asiAlgo_WriteSVG.h>

// asiAlgo includes
#include <asiAlgo_BuildHLR.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <gp_Circ.hxx>
#include <Poly_Polygon3D.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>

// Standard includes
#include <algorithm>

//-----------------------------------------------------------------------------

namespace svg
{
  void printCircle(const BRepAdaptor_Curve& c,
                   std::ostream&            out,
                   ActAPI_PlotterEntry      plotter = nullptr)
  {
    // Note: we'll flip Y-direction down the code as
    //       Y direction of image goes from top to down what is opposite for geometry,
    //       so by flipping makes the coordinate systemes coplanar.

    gp_Circ       circ = c.Circle();
    const gp_Pnt& p    = circ.Location();

    const double r = circ.Radius();
    const double f = c.FirstParameter();
    const double l = c.LastParameter();

    gp_Pnt s = c.Value( f );
    gp_Pnt m = c.Value( ( l + f ) / 2.0 );
    gp_Pnt e = c.Value( l );

    gp_Vec v1( m, s );
    gp_Vec v2( m, e );
    v2.Reverse();
    gp_Vec v3( 0, 0, 1 );

    double a = v3.DotCross( v1, v2 );

    // Full circle.
    if ( fabs( l - f ) > 1.0 && s.SquareDistance( e ) < 0.001 )
    {
      out << "  <circle cx =\""
          << p.X()  << "\" cy =\""
          << -p.Y() << "\" r =\""
          << r      << "\" />";
    }
    // Arc of circle.
    else
    {
      out << "  <path d=\"M"
          << s.X() << " "
          << -s.Y()
          << " A"
          << r << " "
          << r << " "
          << "0 " // X-axis-rotation.
          << ( ( l - f > M_PI ) ? '1' : '0' ) << " " // Large-arc-flag.
          << ( ( a < 0 ) ? '1' : '0' ) << " " // Sweep-flag, i.e. clockwise (0) or counter-clockwise (1).
          << e.X() << " "
          << -e.Y() << "\" />";
    }
  }

  //-----------------------------------------------------------------------------

  void printGeneric(const BRepAdaptor_Curve& bac,
                    int                      id,
                    const double             angDef,
                    const double             linDef,
                    std::ostream&            out,
                    ActAPI_PlotterEntry      plotter = nullptr)
  {
    TopLoc_Location location;
    Handle(Poly_Polygon3D) polygon =
      BRep_Tool::Polygon3D( bac.Edge(), location );

    // Note: we'll flip Y-direction down the code as
    //       Y direction of image goes from top to down what is opposite for geometry,
    //       so by flipping makes the coordinate systemes coplanar.

    if ( !polygon.IsNull() )
    {
      const TColgp_Array1OfPnt& nodes = polygon->Nodes();
      char c = 'M';

      out << "  <path id= \"" /*<< ViewName*/ << id << "\" d=\" ";

      for ( int i = nodes.Lower(); i <= nodes.Upper(); i++ )
      {
        gp_Pnt placedNode = nodes(i).Transformed(location);
        out << c << " " << placedNode.X() << " " << -placedNode.Y() << " ";
        c = 'L';
      }

      out << "\" />" << std::endl;

    }
    else if ( bac.GetType() == GeomAbs_Line )
    {
      // BRep_Tool::Polygon3D assumes the edge has polygon representation,
      // i.e. has already been "tessellated", and this is not true for all edges,
      // especially dangling ones.
      double f = bac.FirstParameter();
      double l = bac.LastParameter();

      gp_Pnt s = bac.Value( f );
      gp_Pnt e = bac.Value( l );

      char c = 'M';

      out << "  <path id= \"" /*<< ViewName*/ << id << "\" d=\" ";

      out << c << " "
          << s.X() << " "
          << -s.Y()<< " ";

      c = 'L';

      out << c << " "
          << e.X() << " "
          << -e.Y()<< " ";

      out << "\" />" << std::endl;
    }
    else
    {
      GCPnts_TangentialDeflection pntGen(bac, angDef, linDef);
      const int nbPnt = pntGen.NbPoints();
      if ( nbPnt > 1 )
      {
        char c = 'M';

        out << "  <path id= \"" /*<< ViewName*/ << id << "\" d=\" ";

        for ( int index = 1; index <= nbPnt; ++index )
        {
          gp_Pnt pnt = bac.Value(pntGen.Parameter(index));
          out << c << " " << pnt.X() << " " << -pnt.Y() << " ";
          c = 'L';
        }

        out << "\" />" << std::endl;
      }
    }
  }

  //-----------------------------------------------------------------------------

  std::string exportEdges(const TopoDS_Shape& input,
                          const double        angDef,
                          const double        linDef,
                          ActAPI_PlotterEntry plotter  = nullptr)
  {
    std::stringstream result;

    TopExp_Explorer edges( input, TopAbs_EDGE );

    for ( int i = 1 ; edges.More(); edges.Next(), i++ )
    {
      const TopoDS_Edge& edge = TopoDS::Edge( edges.Current() );

      BRepAdaptor_Curve adapt( edge );
      if ( adapt.GetType() == GeomAbs_Circle )
      {
        printCircle( adapt, result, plotter);
      }
      else
      {
        printGeneric( adapt, i, angDef, linDef, result, plotter );
      }
    }

    return result.str();
  }

  //-----------------------------------------------------------------------------

  const TopoDS_Shape& build3dCurves(const TopoDS_Shape &shape)
  {
    TopExp_Explorer it;
    for ( it.Init( shape, TopAbs_EDGE ); it.More(); it.Next() )
    {
      BRepLib::BuildCurve3d( TopoDS::Edge( it.Current() ) );
    }

    return shape;
  }

  //-----------------------------------------------------------------------------

  void printEdges(const TopoDS_Shape& shape,
                  const double        lineWidth,
                  const double        angDef,
                  const double        linDef,
                  std::stringstream&  result,
                  ActAPI_PlotterEntry plotter = nullptr)
  {
    if ( shape.IsNull() )
    {
      return;
    }

    std::string style =
      "<g fill = \"none\""
      " stroke=\"rgb(0, 0, 0)\""
      " stroke-linecap=\"round\""
      " stroke-linejoin=\"round\""
      " stroke-width=\"" + std::to_string( lineWidth ) + "\">\n";

    BRepMesh_IncrementalMesh(shape, linDef);

    result << style.c_str()
           << exportEdges(shape, angDef, linDef, plotter)
           << "</g>"
           << std::endl;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_WriteSVG::WriteWithHLR(const TopoDS_Shape&            shape,
                                    const gp_Dir&                  dir,
                                    const TCollection_AsciiString& path,
                                    const double                   tol,
                                    const t_drawingStyle&          style,
                                    ActAPI_ProgressEntry           progress,
                                    ActAPI_PlotterEntry            plotter)
{
  TIMER_NEW
  TIMER_GO

  // Build HLR.
  asiAlgo_BuildHLR buildHLR(shape);
  //
  if ( !buildHLR.Perform(dir, asiAlgo_BuildHLR::Mode_Precise) )
  {
    std::cout << "Error: cannot build HLR presentation." << std::endl;
    return false;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("HLR discrete projection")

  TopoDS_Shape hlrResult = buildHLR.GetResult();

  /* Relocate the projection to XOY plane */
  {
    gp_Ax3 toCS  ( gp::XOY() );
    gp_Ax3 fromCS( gp::Origin(), dir );

    // Relocation transformation.
    gp_Trsf T = asiAlgo_Utils::GetAlignmentTrsf( toCS, fromCS );

    // Transform.
    hlrResult = BRepBuilderAPI_Transform( hlrResult, T, true );
  }

  TopoDS_Shape V = svg::build3dCurves(hlrResult);

  return Write(V, path, tol, style, plotter);
}

//-----------------------------------------------------------------------------

bool asiAlgo_WriteSVG::Write(const TopoDS_Shape&            shape,
                             const TCollection_AsciiString& path,
                             const double                   tol,
                             const t_drawingStyle&          style,
                             ActAPI_PlotterEntry            plotter)
{
  double xMin, yMin, zMin, xMax, yMax, zMax;
  asiAlgo_Utils::Bounds(shape, xMin, yMin, zMin, xMax, yMax, zMax, tol, true);

  // Compute line width.
  const double width        = std::abs(xMax - xMin);
  const double height       = std::abs(yMax - yMin);
  const double maxDimension = Max(width, height);

  const double scaledPadding = maxDimension * style.CanvasPadding * style.PaddingScaleCoeff;
  const int    canvasWidth   = (int)std::round(width  + scaledPadding * 2);
  const int    canvasHeight  = (int)std::round(height + scaledPadding * 2);

  const double maxDimensionCanvas = Max(canvasWidth, canvasHeight);
  //
  const double scaledLineWidth = maxDimension / maxDimensionCanvas * style.LineWidthScaleCoeff;

  // Get results.
  std::stringstream result;

  svg::printEdges(shape, 
                  scaledLineWidth, 
                  style.DiscrCurveAngDefl, style.DiscrCurveLinDefl,
                  result,
                  plotter);

  // Save results to file.
  std::ofstream FILE;
  FILE.open( path.ToCString(), std::ios::out | std::ios::trunc );
  //
  if ( !FILE.is_open() )
  {
    std::cout << "Error: cannot open file for SVG export." << std::endl;
    return false;
  }

  TCollection_AsciiString head = "<svg width=\"";
  head += width;
  head += "\" height=\"";
  head += height;
  head += "\" saveAspectRatio=\"";
  head += "xMinYMin meet";
  head += "\" viewBox=\"";
  head += xMin - scaledPadding;
  head += " ";
  head += -yMax - scaledPadding;
  head += " ";
  head += canvasWidth;
  head += " ";
  head += canvasHeight;
  head += "\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n\n";

  FILE << head
       << result.rdbuf()
       << "\n\n</svg>\n";

  return true;
}
