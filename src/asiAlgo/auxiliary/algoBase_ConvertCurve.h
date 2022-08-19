//-----------------------------------------------------------------------------
// Created on: September 2021
// Created by: Sergey KISELEV
//-----------------------------------------------------------------------------

#ifndef algoBase_ConvertCurve_HeaderFile
#define algoBase_ConvertCurve_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OCCT includes
#include <Geom_BSplineCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_SequenceOfShape.hxx>

//-----------------------------------------------------------------------------

namespace pb
{
  //! Function to perform conversion of spline and ellipse curves to arcs and lines.
  namespace algoBase_ConvertCurve
  {
    asiAlgo_EXPORT bool
      Perform(const Handle(Geom_Curve)& curve,
              double                    first,
              double                    last,
              TopoDS_Wire&              wire,
              double                    tolerance = 0.001);
  }; // namespace algoBase_ConvertCurve

} // namespace pb

#endif // algoBase_ConvertCurve_HeaderFile
