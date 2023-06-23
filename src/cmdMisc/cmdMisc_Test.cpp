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

//-----------------------------------------------------------------------------

#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <asiAlgo_DivideByContinuity.h>

#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_BuildConvexHull.h>
#include <asiAlgo_QuickHull2d.h>

#if defined USE_MOBIUS
  #include <mobius/poly_Mesh.h>
  using namespace mobius;
#endif

#include <BRepTools.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <GeomConvert.hxx>
#include <ShapeFix_Wire.hxx>
#include <BOPAlgo_Builder.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>

#include <IGESControl_Reader.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Builder.hxx>
#include <ShapeAnalysis_Curve.hxx>

  #include <stdio.h>

void fixprint(char *s, const int len)
{
  if ( s[0] == '-' )
  {
    for ( int j = 2; j <= len; ++j  )
      s[j-1] = s[j];

    s[len] = '\0'; // Zero-trailing.
  }
}

char* format_fortran_float(
  char*    result,  // where to write the formatted number. Must have
  unsigned width,   // room for width + 1 characters.
  double   number
  ) {

   // 31.415926535 -> 0.314159E+02

  // First, we'll learn the exponent and adjust the number to the range [0.0,1.0]
  int exponent = 0;
  for (; Abs(number) > 1.0; exponent++) number /= 10;
  //for (; number < 0.0; exponent--) number *= 10;

  // Next, we'll print the number as mantissa in [0,1] and exponent
  //if ( number > 0 )
    sprintf( result, "%.*fE%+03d", 7, number, exponent );
  /*else
   printf( result, "%.10e", (width - 4), number, exponent ); */

  fixprint(result, width);

  // Finally, we'll return the new string
  return result;
  }

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          argc,
              const char**                 argv)
{
  char buf[ 14 ];
  printf(
    "The number is %s\n",
    format_fortran_float( buf, 12, 31.415926535 )
    );
  printf(
    "The number is %s\n",
    format_fortran_float( buf, 12, -31.415926535 )
    );
  return 0;
}

//-----------------------------------------------------------------------------

//int MISC_Test(const Handle(asiTcl_Interp)& interp,
//              int                          argc,
//              const char**                 argv)
//{
//  float x = 31.415926535;
//        char s[14];
//        sprintf(s,"% 8.2f",x);
//        fixprint(s);
//        printf("%s\n",s);
//  return 0;
//}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Test(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("test", "Test anything.", __FILE__, group, MISC_Test);
}
