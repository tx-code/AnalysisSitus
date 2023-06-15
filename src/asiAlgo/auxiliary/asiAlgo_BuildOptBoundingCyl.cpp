//-----------------------------------------------------------------------------
// Created on: 25 May 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023, Julia Slyadneva
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
#include "asiAlgo_BuildOptBoundingCyl.h"

// asiAlgo includes
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_OrientCnc.h>

// OpenCascade includes
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBndLib.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

namespace aux
{
  //! Calculates bounding cylinder for given points and direction.
  void calculateCylinder(const Handle(TColgp_HArray1OfPnt)&     nodes,
                         const gp_Dir&                          orientation,
                         const Bnd_Box&                         aabb,
                         asiAlgo_BuildOptBoundingCyl::t_optBnd& result)
  {
    // Get center of bounding volume.
    gp_Pnt center((aabb.CornerMax().XYZ() + aabb.CornerMin().XYZ()) / 2.);

    // Calculates cylinder's radius as a distance between
    // the center point and triangulation nodes. All the points
    // are projected on a plane orthogonal to the target direction.
    gp_Pnt projCenter(center.X() * (1. - orientation.X()),
                      center.Y() * (1. - orientation.Y()),
                      center.Z() * (1. - orientation.Z()));
    //
    result.radius = DBL_MIN;
    for (int i = 1; i <= nodes->Size(); ++i)
    {
      gp_Pnt projNode = gp_Pnt(nodes->Value(i).X() * (1. - orientation.X()),
                               nodes->Value(i).Y() * (1. - orientation.Y()),
                               nodes->Value(i).Z() * (1. - orientation.Z()));

      double dist = projNode.Distance(projCenter);
      result.radius = Max(dist, result.radius);
    }

    // Calculates cylinder's height as a maximal size of 
    // a given bounding box along the given direction.
    result.height = Max((aabb.CornerMax().X() - aabb.CornerMin().X()) * orientation.X(),
                        (aabb.CornerMax().Y() - aabb.CornerMin().Y()) * orientation.Y());

    result.height = Max(result.height,
                        (aabb.CornerMax().Z() - aabb.CornerMin().Z()) * orientation.Z());

    // Calculates cylinder's volume.
    result.volume = M_PI * result.height * pow(result.radius, 2);

    // Get a reference plane for cylinder's base. The plane is
    // orthogonal to the given direction and placed at the center of 
    // a minimal side of bounding box which is orthogonal to the given direction.

    gp_Ax2 plane(gp_Pnt(center.X() * (1. - orientation.X()) + aabb.CornerMin().X() * orientation.X(),
                        center.Y() * (1. - orientation.Y()) + aabb.CornerMin().Y() * orientation.Y(),
                        center.Z() * (1. - orientation.Z()) + aabb.CornerMin().Z() * orientation.Z()),
                 orientation);

    // Transform cylinder to the initial position of part
    BRepPrimAPI_MakeCylinder mkCyl(plane,
                                   result.radius,
                                   result.height);
    result.shape = mkCyl.Solid();
  }  
};

//-----------------------------------------------------------------------------

asiAlgo_BuildOptBoundingCyl::asiAlgo_BuildOptBoundingCyl(ActAPI_ProgressEntry progress,
                                                         ActAPI_PlotterEntry  plotter)
:ActAPI_IAlgorithm(progress, plotter)
{
}

//-----------------------------------------------------------------------------
bool asiAlgo_BuildOptBoundingCyl::Perform(const Handle(asiAlgo_AAG)& aag,
                                          const bool                 forceTriangulate)
{
  m_cylinder.clear();
  TopoDS_Shape shape = aag->GetMasterShape();

  // Check if the shape should be meshed beforehand.
  asiAlgo_MeshInfo meshInfo = asiAlgo_MeshInfo::Extract(shape);
  //
  if ( !meshInfo.nFacets || forceTriangulate )
  {
    const double linDefl = asiAlgo_MeshGen::AutoSelectLinearDeflection(shape);
    const double angDefl = asiAlgo_MeshGen::AutoSelectAngularDeflection(shape);

    if ( !asiAlgo_MeshGen::DoNative(shape,
                                    linDefl,
                                    angDefl,
                                    meshInfo) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Failed to mesh body shape.");
      return false;
    }

    m_progress.SendLogMessage(LogInfo(Normal) << "The body shape was tessellated with "
                                                 "linear deflection %1 and angular deflection %2."
                                              << linDefl << angDefl);
  }

  // Orient the part so that it aligns the Z-direction (machining axis).
  asiAlgo_OrientCnc orient(aag,
                           m_progress,
                           m_plotter);
  //
  if (!orient.Perform())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Re-orientation failed.");
    return false;
  }

  TopoDS_Shape orientedShape = shape.Moved(orient.GetTrsf());

  // Get nodes of triangulation
  asiAlgo_MeshMerge meshMerge(orientedShape, asiAlgo_MeshMerge::Mode::Mode_Mesh);
  //
  Handle(Poly_Triangulation)
    inputTris = meshMerge.GetResultPoly()->GetTriangulation();

  if (inputTris.IsNull())
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Input triangulation is null.");
    return false;
  }

  // Get the center of bounding box.
  Bnd_Box aabb;
  BRepBndLib::Add(orientedShape, aabb);


  // Calculates bounding cylinders for X, Y and Z directions.
  t_optBnd Xcyl;
  aux::calculateCylinder(inputTris->MapNodeArray(),
                         gp_Dir(1., 0., 0.),
                         aabb,
                         Xcyl);

  t_optBnd Ycyl;
  aux::calculateCylinder(inputTris->MapNodeArray(),
                         gp_Dir(0., 1., 0.),
                         aabb,
                         Ycyl);

  t_optBnd Zcyl;
  aux::calculateCylinder(inputTris->MapNodeArray(),
                         gp_Dir(0., 0., 1.),
                         aabb,
                         Zcyl);

  // Get the minimal bounding cylinder.
  t_optBnd minBnd = Xcyl.min(Ycyl);
  minBnd = minBnd.min(Zcyl);

  // Transform the result cylinder to the initial position of part
  minBnd.shape.Move(orient.GetTrsf().Inverted());
  m_cylinder = minBnd;

  if (orient.GetAxes())
  {
    m_cylinder.placement = *orient.GetAxes();
  }
  m_cylinder.trsf      = orient.GetTrsf();
  m_progress.SendLogMessage(LogInfo(Normal) << "Cylinder volume: %1." << m_cylinder.volume);
 
  //m_plotter.DRAW_SHAPE(Xcyl.shape.Moved(orient.GetTrsf().Inverted()), Color_Red,   "Xcyl");
  //m_plotter.DRAW_SHAPE(Ycyl.shape.Moved(orient.GetTrsf().Inverted()), Color_Green, "Ycyl");
  //m_plotter.DRAW_SHAPE(Zcyl.shape.Moved(orient.GetTrsf().Inverted()), Color_Blue,  "Zcyl");

  return true;
}
