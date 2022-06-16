//-----------------------------------------------------------------------------
// Created on: 16 June 2022
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

// Own include
#include <asiAlgo_CheckInsertion.h>

// OCCT includes
#include <ShapeAnalysis.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

//-----------------------------------------------------------------------------

asiAlgo_CheckInsertion::asiAlgo_CheckInsertion(ActAPI_ProgressEntry progress,
                                               ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm(progress, plotter)
{}

//-----------------------------------------------------------------------------

asiAlgo_InsertionType
  asiAlgo_CheckInsertion::Check(const TopoDS_Face& featureFace,
                                const TopoDS_Face& masterFace) const
{
  // Extract common edges for faces
  TopTools_IndexedMapOfShape EdgesFeature, EdgesMaster, EdgesCommon;
  TopExp::MapShapes(featureFace, TopAbs_EDGE, EdgesFeature);
  TopExp::MapShapes(masterFace,  TopAbs_EDGE, EdgesMaster);
  //
  for ( int ef = 1; ef <= EdgesFeature.Extent(); ++ef )
  {
    for ( int eg = 1; eg <= EdgesMaster.Extent(); ++eg )
    {
      if ( EdgesFeature(ef).IsSame( EdgesMaster(eg) ) )
        EdgesCommon.Add( EdgesFeature(ef) );
    }
  }

  // Soft insertion is realized if all common edges are used in the
  // internal wires of master face
  TopoDS_Wire masterOuterWire = ShapeAnalysis::OuterWire(masterFace);
  TopTools_IndexedMapOfShape outerWireVert;
  TopExp::MapShapes(masterOuterWire, TopAbs_VERTEX, outerWireVert);
  //
  for ( int e = 1; e <= EdgesCommon.Extent(); ++e )
  {
    const TopoDS_Shape& featureEdge = EdgesCommon(e);

    // Try to find this edge among internal wires of the master face
    bool isFound = false;
    for ( TopoDS_Iterator wit(masterFace); wit.More(); wit.Next() )
    {
      if ( wit.Value().ShapeType() != TopAbs_WIRE || wit.Value().IsPartner(masterOuterWire) )
        continue; // Skip strange topologies and outer wire

      // Take inner wire
      const TopoDS_Wire& masterInnerWire = TopoDS::Wire( wit.Value() );

      for ( TopoDS_Iterator eit(masterInnerWire); eit.More(); eit.Next() )
      {
        if ( eit.Value().ShapeType() != TopAbs_EDGE )
          continue;

        if ( eit.Value().IsPartner(featureEdge) )
        {
          isFound = true;
          break;
        }
      }

      // Check that there is no intersection between inner and outer wire.
      // If such intersection is found, "hard" state will be returned.
      TopTools_IndexedMapOfShape innerWireVert;
      TopExp::MapShapes(masterInnerWire, TopAbs_VERTEX, innerWireVert);
      TopTools_IndexedMapOfShape::Iterator innerVertIter(innerWireVert);
      for ( ; innerVertIter.More() ; innerVertIter.Next())
      {
        if (outerWireVert.Contains(innerVertIter.Value()))
        {
          return InsertionType_Hard;
        }
      }
      if ( isFound ) break;
    }

    if ( !isFound )
      return InsertionType_Hard;

  }

  return InsertionType_Soft;
}
