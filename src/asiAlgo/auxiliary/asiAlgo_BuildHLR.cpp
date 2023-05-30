//-----------------------------------------------------------------------------
// Created on: 24 December 2020
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
#include <asiAlgo_BuildHLR.h>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepLib.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

asiAlgo_BuildHLR::asiAlgo_BuildHLR(const TopoDS_Shape&  shape,
                                   ActAPI_ProgressEntry progress,
                                   ActAPI_PlotterEntry  plotter)
//
: ActAPI_IAlgorithm (progress, plotter),
  m_input           (shape)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildHLR::Perform(const gp_Dir&        projectionDir,
                               const Mode           mode,
                               const t_outputEdges& visibility)
{
  switch ( mode )
  {
    case Mode_Precise:
      return this->performPrecise(projectionDir, visibility);
    case Mode_Discrete:
      return this->performDiscrete(projectionDir);
    default:
      break;
  }
  return false;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildHLR::performPrecise(const gp_Dir&       direction,
                                      const t_outputEdges visibility)
{
  Handle(HLRBRep_Algo) brep_hlr = new HLRBRep_Algo;
  brep_hlr->Add(m_input);

  gp_Ax2 transform(gp::Origin(), direction);
  HLRAlgo_Projector projector(transform);
  brep_hlr->Projector(projector);
  brep_hlr->Update();
  brep_hlr->Hide();

  // Extract the result sets.
  HLRBRep_HLRToShape shapes(brep_hlr);

  // V -- visible
  // H -- hidden
  TopoDS_Shape V  = this->build3dCurves(shapes.VCompound       ()); // hard edge visibly
  TopoDS_Shape V1 = this->build3dCurves(shapes.Rg1LineVCompound()); // smooth edges visibly
  TopoDS_Shape VN = this->build3dCurves(shapes.RgNLineVCompound()); // contour edges visibly
  TopoDS_Shape VO = this->build3dCurves(shapes.OutLineVCompound()); // contours apparents visibly
  TopoDS_Shape VI = this->build3dCurves(shapes.IsoLineVCompound()); // isoparamtriques visibly
  TopoDS_Shape H  = this->build3dCurves(shapes.HCompound       ()); // hard edge invisibly
  TopoDS_Shape H1 = this->build3dCurves(shapes.Rg1LineHCompound()); // smooth edges invisibly
  TopoDS_Shape HN = this->build3dCurves(shapes.RgNLineHCompound()); // contour edges invisibly
  TopoDS_Shape HO = this->build3dCurves(shapes.OutLineHCompound()); // contours apparents invisibly
  TopoDS_Shape HI = this->build3dCurves(shapes.IsoLineHCompound()); // isoparamtriques invisibly

  TopoDS_Compound C;
  BRep_Builder().MakeCompound(C);
  //
  if ( !V.IsNull() && visibility.OutputVisibleSharpEdges)
    BRep_Builder().Add(C, V);
  //
  if ( !V1.IsNull() && visibility.OutputVisibleSmoothEdges)
    BRep_Builder().Add(C, V1);
  //
  if ( !VN.IsNull() && visibility.OutputVisibleOutlineEdges)
    BRep_Builder().Add(C, VN);
  //
  if ( !VO.IsNull() && visibility.OutputVisibleSewnEdges)
    BRep_Builder().Add(C, VO);
  //
  if ( !VI.IsNull() && visibility.OutputVisibleIsoLines)
    BRep_Builder().Add(C, VI);
  //
  if ( !H.IsNull() && visibility.OutputHiddenSharpEdges)
    BRep_Builder().Add(C, H);
  //
  if ( !H1.IsNull() && visibility.OutputHiddenSmoothEdges)
    BRep_Builder().Add(C, H1);
  //
  if ( !HN.IsNull() && visibility.OutputHiddenOutlineEdges)
    BRep_Builder().Add(C, HN);
  //
  if ( !HO.IsNull() && visibility.OutputHiddenSewnEdges)
    BRep_Builder().Add(C, HO);
  
  if ( !HI.IsNull() && visibility.OutputHiddenIsoLines)
    BRep_Builder().Add(C, HI);

  gp_Trsf T;
  T.SetTransformation( gp_Ax3(transform) );
  T.Invert();

  m_result = C.Moved(T);
  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildHLR::performDiscrete(const gp_Dir& direction)
{
  gp_Ax2 transform(gp::Origin(), direction);

  // Prepare projector.
  HLRAlgo_Projector projector(transform);

  // Prepare polygonal HLR algorithm which is known to be more reliable than
  // the "curved" version of HLR.
  Handle(HLRBRep_PolyAlgo) polyAlgo = new HLRBRep_PolyAlgo;
  //
  polyAlgo->Projector(projector);
  polyAlgo->Load(m_input);
  polyAlgo->Update();

  // Create topological entities.
  HLRBRep_PolyHLRToShape HLRToShape;
  HLRToShape.Update(polyAlgo);

  // Prepare one compound shape to store HLR results.
  TopoDS_Compound C;
  BRep_Builder().MakeCompound(C);

  // Add visible edges.
  TopoDS_Shape vcompound = HLRToShape.VCompound();
  if ( !vcompound.IsNull() )
    BRep_Builder().Add(C, vcompound);
  //
  vcompound = HLRToShape.OutLineVCompound();
  if ( !vcompound.IsNull() )
    BRep_Builder().Add(C, vcompound);

  gp_Trsf T;
  T.SetTransformation( gp_Ax3(transform) );
  T.Invert();

  m_result = C.Moved(T);
  return true;
}

//-----------------------------------------------------------------------------

const TopoDS_Shape& asiAlgo_BuildHLR::GetResult() const
{
  return m_result;
}

//-----------------------------------------------------------------------------

const TopoDS_Shape& asiAlgo_BuildHLR::build3dCurves(const TopoDS_Shape& shape)
{
  for ( TopExp_Explorer it(shape, TopAbs_EDGE); it.More(); it.Next() )
    BRepLib::BuildCurve3d( TopoDS::Edge( it.Current() ) );

  return shape;
}
