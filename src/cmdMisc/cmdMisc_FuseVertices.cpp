//-----------------------------------------------------------------------------
// Created on: 12 December 2020
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2020-present, Sergey Slyadnev
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
#include <cmdMisc_FuseVertices.h>

// OpenCascade includes
#include <BRep_Builder.hxx>

//-----------------------------------------------------------------------------

cmdMisc_FuseVertices::cmdMisc_FuseVertices(const double tol)
{
  m_fToler = tol;
}

//-----------------------------------------------------------------------------

TopoDS_Shape
  cmdMisc_FuseVertices::operator()(const TopoDS_Shape& shape) const
{
  BRep_Builder               brepBuilder;
  TopTools_IndexedMapOfShape allVertices, allEdges;

  // Get all vertices/edges out of the input shape.
  TopExp::MapShapes(shape, TopAbs_VERTEX, allVertices);
  TopExp::MapShapes(shape, TopAbs_EDGE,   allEdges);

  // Collect shapes to fuse.
  TopoDS_Compound allVerticesComp, allEdgesComp;
  //
  brepBuilder.MakeCompound(allVerticesComp);
  brepBuilder.MakeCompound(allEdgesComp);
  //
  for ( int k = 1; k <= allVertices.Extent(); ++k )
  {
    brepBuilder.Add( allVerticesComp, allVertices(k) );
  }
  //
  for ( int k = 1; k <= allEdges.Extent(); ++k )
  {
    brepBuilder.Add( allEdgesComp, allEdges(k) );
  }

  // Prepare arguments.
  TopTools_ListOfShape args;
  //
  args.Append(allVerticesComp);
  args.Append(allEdgesComp);

  // Fuse vertices into edges.
  return asiAlgo_Utils::BooleanGeneralFuse(args, m_fToler);
}
