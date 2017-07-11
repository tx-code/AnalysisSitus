//-----------------------------------------------------------------------------
// Created on: 19 September 2016
//-----------------------------------------------------------------------------
// Copyright (c) 2017 Sergey Slyadnev
// Code covered by the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_GeomContourPrs.h>

// asiVisu includes
#include <asiVisu_ContourDataProvider.h>
#include <asiVisu_ContourPointsDataProvider.h>
#include <asiVisu_PointsPipeline.h>
#include <asiVisu_ShapePipeline.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>

//-----------------------------------------------------------------------------

//! Creates a Presentation object for the passed Geometry Contour Node.
//! \param theNode [in] Geometry Contour Node to create a Presentation for.
asiVisu_GeomContourPrs::asiVisu_GeomContourPrs(const Handle(ActAPI_INode)& theNode)
: asiVisu_Prs(theNode)
{
  Handle(asiData_ContourNode) contour_n = Handle(asiData_ContourNode)::DownCast(theNode);

  // Create Data Provider for polyline
  Handle(asiVisu_ContourDataProvider)
    DP_contour = new asiVisu_ContourDataProvider(contour_n);

  // Create Data Provider for points
  Handle(asiVisu_ContourPointsDataProvider)
    DP_points = new asiVisu_ContourPointsDataProvider(contour_n);

  // Pipeline for contour
  Handle(asiVisu_ShapePipeline) shape_pl = new asiVisu_ShapePipeline(false);
  shape_pl->GetDisplayModeFilter()->SetDisplayMode(ShapeDisplayMode_Wireframe);
  shape_pl->Actor()->GetProperty()->SetColor(1.0, 0.0, 0.0);
  //
  this->addPipeline        ( Pipeline_Main, shape_pl );
  this->assignDataProvider ( Pipeline_Main, DP_contour );

  // Pipeline for points
  Handle(asiVisu_PointsPipeline) points_pl = new asiVisu_PointsPipeline;
  //
  this->addPipeline        ( Pipeline_Points, points_pl );
  this->assignDataProvider ( Pipeline_Points, DP_points );

  // Configure presentation
  shape_pl  -> Actor()->GetProperty()->SetLineWidth(2.0f);
  shape_pl  -> Actor()->GetProperty()->SetLighting(0);
  shape_pl  -> Mapper()->SetScalarVisibility(0);
  points_pl -> Actor()->GetProperty()->SetColor(0.0, 0.0, 1.0);
  points_pl -> Actor()->GetProperty()->SetPointSize(5.0f);

  // Make contour be visualized always on top of the scene
  shape_pl->Mapper()->SetRelativeCoincidentTopologyLineOffsetParameters(0,-66000);
  shape_pl->Mapper()->SetRelativeCoincidentTopologyPolygonOffsetParameters(0,-66000);
  shape_pl->Mapper()->SetRelativeCoincidentTopologyPointOffsetParameter(-66000);
  //
  points_pl->Mapper()->SetRelativeCoincidentTopologyLineOffsetParameters(0,-66000);
  points_pl->Mapper()->SetRelativeCoincidentTopologyPolygonOffsetParameters(0,-66000);
  points_pl->Mapper()->SetRelativeCoincidentTopologyPointOffsetParameter(-66000);
}

//! Factory method for Presentation.
//! \param theNode [in] Node to create a Presentation for.
//! \return new Presentation instance.
Handle(asiVisu_Prs) asiVisu_GeomContourPrs::Instance(const Handle(ActAPI_INode)& theNode)
{
  return new asiVisu_GeomContourPrs(theNode);
}

//! Returns true if the Presentation is visible, false -- otherwise.
//! \return true/false.
bool asiVisu_GeomContourPrs::IsVisible() const
{
  return true;
}

//-----------------------------------------------------------------------------

//! Callback for initialization of Presentation pipelines.
void asiVisu_GeomContourPrs::beforeInitPipelines()
{
  // Do nothing...
}

//! Callback for initialization of Presentation pipelines.
void asiVisu_GeomContourPrs::afterInitPipelines()
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked before the
//! kernel update routine starts.
void asiVisu_GeomContourPrs::beforeUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for updating of Presentation pipelines invoked after the
//! kernel update routine completes.
void asiVisu_GeomContourPrs::afterUpdatePipelines() const
{
  // Do nothing...
}

//! Callback for highlighting.
//! \param theRenderer  [in] renderer.
//! \param thePickRes   [in] picking results.
//! \param theSelNature [in] selection nature (picking or detecting).
void asiVisu_GeomContourPrs::highlight(vtkRenderer*                  theRenderer,
                                       const asiVisu_PickResult&     thePickRes,
                                       const asiVisu_SelectionNature theSelNature) const
{
  asiVisu_NotUsed(theRenderer);
  asiVisu_NotUsed(thePickRes);
  asiVisu_NotUsed(theSelNature);
}

//! Callback for highlighting reset.
//! \param theRenderer [in] renderer.
void asiVisu_GeomContourPrs::unHighlight(vtkRenderer*                  theRenderer,
                                         const asiVisu_SelectionNature theSelNature) const
{
  asiVisu_NotUsed(theRenderer);
  asiVisu_NotUsed(theSelNature);
}

//! Callback for rendering.
//! \param theRenderer [in] renderer.
void asiVisu_GeomContourPrs::renderPipelines(vtkRenderer* theRenderer) const
{
  asiVisu_NotUsed(theRenderer);
}

//! Callback for de-rendering.
//! \param theRenderer [in] renderer.
void asiVisu_GeomContourPrs::deRenderPipelines(vtkRenderer* theRenderer) const
{
  asiVisu_NotUsed(theRenderer);
}
