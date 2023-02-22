//-----------------------------------------------------------------------------
// Created on: 11 April 2019
//-----------------------------------------------------------------------------
// Copyright (c) 2019-present, Sergey Slyadnev
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
#include <asiTest_Utils.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRepPrimAPI_MakeBox.hxx>

//-----------------------------------------------------------------------------

outcome asiTest_Utils::testMapShapes1(const int funcID)
{
  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Prepare outcome.
  outcome res(DescriptionFn(), funcID);

  /* Make a unit box and compare indexation by two different methods. */

  TopoDS_Shape box = BRepPrimAPI_MakeBox(1, 1, 1);

  // Use data map.
  asiAlgo_DataMapOfShape M;
  asiAlgo_Utils::MapShapes(box, M);

  // Use indexed map.
  TopTools_IndexedMapOfShape IM;
  TopExp::MapShapes(box, IM);

  // Compare indices.
  for ( int k = 1; k <= IM.Extent(); ++k )
  {
    // Get sub-shape from the data map by the current index.
    const TopoDS_Shape& M_sh = M(k);

    // Get sub-shape from the indexed map by the current index.
    const TopoDS_Shape& IM_sh = IM(k);

    // Compare.
    if ( !M_sh.IsEqual(IM_sh) )
    {
      cf->Progress.SendLogMessage(LogErr(Normal) << "Shape %1 from an indexed map is different "
                                                    "from shape from a data map." << k);
      return res.failure();
    }
  }

  return res.success();
}

//-----------------------------------------------------------------------------

outcome asiTest_Utils::testRanges1(const int funcID)
{
  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Prepare outcome.
  outcome res(DescriptionFn(), funcID);

  // Set ranges to merge.
  asiAlgo_Utils::Range::t_rangesByFaces ranges, merged;
  //
  ranges.push_back( {0, {0.0, 1.0}} );
  ranges.push_back( {0, {0.5, 1.5}} );
  ranges.push_back( {0, {1.5, 2.5}} );
  ranges.push_back( {0, {2.7, 3.5}} );
  ranges.push_back( {0, {2.9, 3.7}} );

  // Reference ranges.
  asiAlgo_Utils::Range::t_rangesByFaces refRanges;
  refRanges.push_back( {0, {0.0, 2.5}} );
  refRanges.push_back( {0, {2.7, 3.7}} );

  // Merge.
  asiAlgo_Utils::Range::MergeRanges(ranges, merged);

  // Verify.
  if ( !asiAlgo_Utils::Range::AreEqual(merged, refRanges) )
    return res.failure();

  return res.success();
}

//-----------------------------------------------------------------------------

outcome asiTest_Utils::testRanges2(const int funcID)
{
  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Prepare outcome.
  outcome res(DescriptionFn(), funcID);

  // Set ranges to merge.
  asiAlgo_Utils::Range::t_rangesByFaces ranges, merged;
  //
  ranges.push_back( {0, {0.0, 1.0}} );
  ranges.push_back( {0, {0.5, 1.5}} );
  ranges.push_back( {0, {1.5, 2.5}} );
  ranges.push_back( {0, {2.7, 3.5}} );
  ranges.push_back( {0, {2.9, 3.7}} );

  // Reference ranges.
  asiAlgo_Utils::Range::t_rangesByFaces refRanges;
  refRanges.push_back( {0, {0.0, 2.5}} );
  refRanges.push_back( {0, {2.7, 3.7}} );

  // Merge.
  asiAlgo_Utils::Range::MergeRanges(ranges, merged);

  // Verify.
  if ( !asiAlgo_Utils::Range::AreEqual(merged, refRanges) )
    return res.failure();

  return res.success();
}

//-----------------------------------------------------------------------------

outcome asiTest_Utils::testRanges3(const int funcID)
{
  // Get common facilities.
  Handle(asiTest_CommonFacilities) cf = asiTest_CommonFacilities::Instance();

  // Prepare outcome.
  outcome res(DescriptionFn(), funcID);

  // Set ranges to merge.
  asiAlgo_Utils::Range::t_rangesByFaces ranges, merged;
  //
  ranges.push_back( {1,   {72.001207470360271, 167.50435320031025}} );
  ranges.push_back( {1,   {71.828747480850268, 167.50435324604024}} );
  ranges.push_back( {1,   {178.20111561631023, 273.87672138149821}} );
  ranges.push_back( {1,   {328.99210905624022, 335.01394622599622}} );
  ranges.push_back( {1,   {328.99210905624022, 335.01394622599622}} );
  ranges.push_back( {1,   {328.99210905624022, 335.01394622599622}} );
  ranges.push_back( {1,   {258.30078895377324, 259.21467038880724}} );
  ranges.push_back( {1,   {121.34702115485615, 259.21467038880724}} );
  ranges.push_back( {1,   {121.34702115485615, 334.75947720156125}} );
  ranges.push_back( {1,   {121.34702115485615, 334.75947720156125}} );
  ranges.push_back( {1,   {121.34702115485615, 334.75947720156125}} );
  ranges.push_back( {1,   {121.34702115485615, 334.75947720156125}} );
  ranges.push_back( {1,   {78.042915430580251, 334.75947720156125}} );
  ranges.push_back( {1,   {25.741039056240254, 334.75947720156125}} );
  ranges.push_back( {1,   {25.741039056240254, 334.75947720156125}} );
  ranges.push_back( {1,   {25.741039056240254, 334.75947720156125}} );
  ranges.push_back( {1,   {25.741039056240254, 334.75947720156125}} );
  ranges.push_back( {472, {176.66987725072025, 177.30487725072027}} );
  ranges.push_back( {472, {176.66987725072025, 177.30487725072027}} );
  ranges.push_back( {473, {167.11423830260026, 167.36823830260025}} );
  ranges.push_back( {473, {167.11423830260026, 167.36823830260028}} );
  ranges.push_back( {473, {167.11423830260026, 167.36823830260028}} );
  ranges.push_back( {474, {179.02254343174025, 179.27654343174024}} );
  ranges.push_back( {474, {179.02254343174025, 179.27654343174024}} );
  ranges.push_back( {474, {179.02254343174025, 179.27654343174024}} );

  // Reference ranges.
  asiAlgo_Utils::Range::t_rangesByFaces refRanges;
  refRanges.push_back( {0, {25.741039056240254, 335.01394622599622}} );

  // Merge.
  asiAlgo_Utils::Range::MergeRanges(ranges, merged);

  // Verify.
  if ( !asiAlgo_Utils::Range::AreEqual(merged, refRanges) )
    return res.failure();

  return res.success();
}
