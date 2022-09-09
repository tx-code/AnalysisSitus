//-----------------------------------------------------------------------------
// Created on: 25 June 2022
// Created by: Andrey Voevodin
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

// cmdTest includes
#include <cmdTest.h>

// asiEngine includes
#include <asiEngine_Model.h>
#include <asiEngine_Triangulation.h>

// asiAlgo includes
#include <asiAlgo_MeshMerge.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// OCCT includes
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>

#if defined USE_MOBIUS
#include <mobius/cascade.h>
#include <mobius/poly_Mesh.h>

using namespace mobius;
#endif

//-----------------------------------------------------------------------------

int ENGINE_CheckNumberOfMeshEntities(const Handle(asiTcl_Interp)& interp,
                                     int                          argc,
                                     const char**                 argv)
{
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  int nbNodesRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "nodes", nbNodesRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-nodes' was found.");
    return TCL_ERROR;
  }

  int nbTrianglesRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "tri", nbTrianglesRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-tri' was found.");
    return TCL_ERROR;
  }

#if defined USE_MOBIUS

  Handle(asiEngine_Model) model = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No model was found.");
    return TCL_ERROR;
  }

  Handle(asiData_TriangulationNode) triNode = model->GetTriangulationNode();
  if ( triNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No triangulation was found.");
    return TCL_ERROR;
  }

  t_ptr<poly_Mesh> mesh = triNode->GetTriangulation();
  if ( mesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No triangulation was found.");
    return TCL_ERROR;
  }

  Handle(Poly_Triangulation) polyMesh = cascade::GetOpenCascadeMesh(mesh);
  if ( polyMesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh is NULL.");
    return TCL_ERROR;
  }

  int nbNodes     = polyMesh->NbNodes();
  int nbTriangles = polyMesh->NbTriangles();

  if ( nbNodes != nbNodesRef || nbTriangles != nbTrianglesRef )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Real nbNodes = '%1'. Real nbTri = '%2'"
                                                        << nbNodes
                                                        << nbTriangles);
    return TCL_ERROR;
  }

  return TCL_OK;

#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_CheckAABBOfMesh(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 9 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  double tolerance = 0;
  if ( !interp->GetKeyValue(argc, argv, "tol", tolerance) || tolerance <= -Precision::SquareConfusion() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-tol' was found.");
    return TCL_ERROR;
  }
  tolerance = abs(tolerance);

  double xDim = 0;
  if ( !interp->GetKeyValue(argc, argv, "xDim", xDim) || xDim <= -Precision::SquareConfusion() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-xDim' was found.");
    return TCL_ERROR;
  }
  xDim = abs(xDim);

  double yDim = 0;
  if ( !interp->GetKeyValue(argc, argv, "yDim", yDim) || yDim <= -Precision::SquareConfusion() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-yDim' was found.");
    return TCL_ERROR;
  }
  yDim = abs(yDim);

  double zDim = 0;
  if ( !interp->GetKeyValue(argc, argv, "zDim", zDim) || zDim <= -Precision::SquareConfusion() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-zDim' was found.");
    return TCL_ERROR;
  }
  zDim = abs(zDim);

#if defined USE_MOBIUS

  Handle(asiEngine_Model) model = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No model was found.");
    return TCL_ERROR;
  }

  Handle(asiData_TriangulationNode) triNode = model->GetTriangulationNode();
  if ( triNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No triangulation was found.");
    return TCL_ERROR;
  }

  t_ptr<poly_Mesh> mesh = triNode->GetTriangulation();
  if ( mesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No triangulation was found.");
    return TCL_ERROR;
  }

  Handle(Poly_Triangulation) polyMesh = cascade::GetOpenCascadeMesh(mesh);
  if ( polyMesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh is NULL.");
    return TCL_ERROR;
  }

  TopoDS_Face face;
  BRep_Builder().MakeFace(face);
  BRep_Builder().UpdateFace(face, polyMesh);

  Bnd_Box bbox;
  BRepBndLib::AddOptimal(face, bbox, true, false);

  if ( bbox.IsVoid() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Box is void.");
    return TCL_ERROR;
  }

  std::vector<double> dim = { fabs(bbox.CornerMax().X() - bbox.CornerMin().X()),
                              fabs(bbox.CornerMax().Y() - bbox.CornerMin().Y()),
                              fabs(bbox.CornerMax().Z() - bbox.CornerMin().Z()) };

  if ( abs(xDim - dim[0]) >= tolerance ||
       abs(yDim - dim[1]) >= tolerance ||
       abs(zDim - dim[2]) >= tolerance )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Real xDim = '%1'. Real yDim = '%2'. Real zDim = '%3'"
                                                        << dim[0]
                                                        << dim[1]
                                                        << dim[2]);
    return TCL_ERROR;
  }

  return TCL_OK;
#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_MakeTriangulationFromFacets(const Handle(asiTcl_Interp)& interp,
                                       int                          argc,
                                       const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

#if defined USE_MOBIUS

  Handle(asiEngine_Model) model = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No model was found.");
    return TCL_ERROR;
  }

  Handle(asiData_PartNode) partNode = model->GetPartNode();
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_ERROR;
  }
  TopoDS_Shape shape = partNode->GetShape();
  if ( shape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }

  asiAlgo_MeshMerge meshMerge(shape);
  if ( meshMerge.GetResultPoly().IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot create mesh from shape.");
    return TCL_ERROR;
  }

  Handle(Poly_Triangulation) polyMesh = meshMerge.GetResultPoly()->GetTriangulation();
  if ( polyMesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh is NULL.");
    return TCL_ERROR;
  }

  Handle(asiData_TriangulationNode) triNode = model->GetTriangulationNode();
  if ( triNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No triangulation was found.");
    return TCL_ERROR;
  }

  model->OpenCommand();
  triNode->SetTriangulation(cascade::GetMobiusMesh(polyMesh));
  model->CommitCommand();

  return TCL_OK;

#else
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

void cmdTest::Commands_Mesh(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdTest_NotUsed(data))
{
  static const char* group = "cmdTest";

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-check-number-mesh-entities",
    //
    "test-check-number-mesh-entities -nodes <nbNodes> -tri <nbTriangles>\n"
    "\t Check number of mesh entities.",
    //
    __FILE__, group, ENGINE_CheckNumberOfMeshEntities);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-check-mesh-aabb-dim",
    //
    "test-check-mesh-aabb-dim -xDim <xDim> -yDim <yDim> -zDim <zDim> -tol <tolerance>\n"
    "\t Check dimensions of AABB of mesh.",
    //
    __FILE__, group, ENGINE_CheckAABBOfMesh);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-make-triangulation-from-facets",
    //
    "test-make-triangulation-from-facets\n"
    "\t Make triangulation from facets.",
    //
    __FILE__, group, ENGINE_MakeTriangulationFromFacets);
}
