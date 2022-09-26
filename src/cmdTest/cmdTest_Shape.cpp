//-----------------------------------------------------------------------------
// Created on: 27 June 2022
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

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// OCCT includes
#include <BRepBndLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>

// STL includes
#include <vector>

//-----------------------------------------------------------------------------

int TEST_CheckNumberOfShapeEntities(const Handle(asiTcl_Interp)& interp,
                                    int                          argc,
                                    const char**                 argv)
{
  if ( argc != 17 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  int nbVerticesRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "vertex", nbVerticesRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-vertex' was found.");
    return TCL_ERROR;
  }
  //
  int nbEdgesRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "edge", nbEdgesRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-edge' was found.");
    return TCL_ERROR;
  }
  //
  int nbWiresRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "wire", nbWiresRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-wire' was found.");
    return TCL_ERROR;
  }
  //
  int nbFacesRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "face", nbFacesRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-face' was found.");
    return TCL_ERROR;
  }
  //
  int nbShellsRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "shell", nbShellsRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-shell' was found.");
    return TCL_ERROR;
  }
  //
  int nbSolidsRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "solid", nbSolidsRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-solid' was found.");
    return TCL_ERROR;
  }
  //
  int nbCompsolidsRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "compsolid", nbCompsolidsRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-compsolid' was found.");
    return TCL_ERROR;
  }
  //
  int nbCompoundsRef = 0;
  if ( !interp->GetKeyValue(argc, argv, "compound", nbCompoundsRef) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No key '-compound' was found.");
    return TCL_ERROR;
  }

  // Get shape to analyze.
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( M.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model is NULL.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape partShape = M->GetPartNode()->GetShape();
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }
  //
  asiAlgo_TopoSummary summary;
  asiAlgo_Utils::ShapeSummary(partShape, summary);

  if ( nbVerticesRef   != summary.nbVertexes   ||
       nbEdgesRef      != summary.nbEdges      ||
       nbWiresRef      != summary.nbWires      ||
       nbFacesRef      != summary.nbFaces      ||
       nbShellsRef     != summary.nbShells     ||
       nbSolidsRef     != summary.nbSolids     ||
       nbCompsolidsRef != summary.nbCompsolids ||
       nbCompoundsRef  != summary.nbCompounds )
  {
    std::string message = "";
    message += "Real nbVertices = "    + std::to_string(summary.nbVertexes)   + ".";
    message += " Real nbEdges = "      + std::to_string(summary.nbEdges)      + ".";
    message += " Real nbWires = "      + std::to_string(summary.nbWires)      + ".";
    message += " Real nbFaces = "      + std::to_string(summary.nbFaces)      + ".";
    message += " Real nbShells = "     + std::to_string(summary.nbShells)     + ".";
    message += " Real nbSolids = "     + std::to_string(summary.nbSolids)     + ".";
    message += " Real nbCompsolids = " + std::to_string(summary.nbCompsolids) + ".";
    message += " Real nbCompounds = "  + std::to_string(summary.nbCompounds)  + ".";
    interp->GetProgress().SendLogMessage(LogErr(Normal) << message.c_str());
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int TEST_CheckAABBOfShape(const Handle(asiTcl_Interp)& interp,
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

  Handle(asiEngine_Model) model = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No model was found.");
    return TCL_ERROR;
  }

  TopoDS_Shape partShape = model->GetPartNode()->GetShape();
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }

  Bnd_Box bbox;
  BRepBndLib::AddOptimal(partShape, bbox, false, false);

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
}

//-----------------------------------------------------------------------------

int TEST_CheckSolidsVolumes(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const double tol = std::atof(argv[1]);

  std::vector<double> volumes;
  for ( int index = 2; index < argc; ++index )
  {
    volumes.push_back(std::atof(argv[index]));
  }

  // Get shape to analyze.
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( M.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model is NULL.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape partShape = M->GetPartNode()->GetShape();
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }

  bool isOK = true;
  std::vector<double>::const_iterator itVolumes = volumes.cbegin();
  TopExp_Explorer exp(partShape, TopAbs_SOLID);
  if (!exp.More())
  {
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "No solids");
    return TCL_ERROR;
  }

  for (; itVolumes != volumes.cend() && exp.More(); ++itVolumes, exp.Next())
  {
    GProp_GProps props;
    BRepGProp::VolumeProperties(exp.Value(), props);

    if (abs(props.Mass() - *itVolumes) > tol)
    {
      isOK = false;
    }
  }

  if ( itVolumes != volumes.cend() || exp.More() || !isOK )
  {
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Volumes: ");
    exp.Init(partShape, TopAbs_SOLID);
    for (; exp.More(); exp.Next())
    {
      GProp_GProps props;
      BRepGProp::VolumeProperties(exp.Value(), props);
      interp->GetProgress().SendLogMessage(LogInfo(Normal) << "%1" << props.Mass());
    }

    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int TEST_CheckPartShape(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get shape to analyze.
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if ( M.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model is NULL.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape partShape = M->GetPartNode()->GetShape();
  if ( partShape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }

  try
  {
    BRepCheck_Analyzer analyzer(partShape, true);
    if ( !analyzer.IsValid() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is not valid.");
      return TCL_ERROR;
    }
  }
  catch ( ... )
  {
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdTest::Commands_Shape(const Handle(asiTcl_Interp)&      interp,
                             const Handle(Standard_Transient)& cmdTest_NotUsed(data))
{
  static const char* group = "cmdTest";

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-check-number-shape-entities",
    //
    "test-check-number-shape-entities -vertex <nbVertices> -edge <nbEdges> -wire <nbWires>"
    " -face <nbFaces> -shell <nbShells> -solid <nbsolids> -compsolid <nbCompsolids> -compound <nbCompound>\n"
    "\t Check number of shape entities.",
    //
    __FILE__, group, TEST_CheckNumberOfShapeEntities);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-check-shape-aabb-dim",
    //
    "test-check-shape-aabb-dim -xDim <xDim> -yDim <yDim> -zDim <zDim> -tol <tolerance>\n"
    "\t Check dimensions of AABB of mesh.",
    //
    __FILE__, group, TEST_CheckAABBOfShape);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-check-solids-volumes",
    //
    "test-check-solids-volumes <tolerance> <vol1> <vol2> <vol3> ...\n"
    "\t Checks volumes of solids.",
    //
    __FILE__, group, TEST_CheckSolidsVolumes);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-check-part-shape",
    //
    "test-check-part-shape\n"
    "\t Checks part shape.",
    //
    __FILE__, group, TEST_CheckPartShape);
}
