//-----------------------------------------------------------------------------
// Created on: 24 August 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017, Sergey Slyadnev
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

// cmdEngine includes
#include <cmdEngine.h>

// asiEngine includes
#include <asiEngine_Part.h>
#include <asiEngine_STEPReaderOutput.h>

// asiVisu includes
#include <asiVisu_MeshEScalarFilter.h>
#include <asiVisu_MeshEScalarPipeline.h>
#include <asiVisu_ThicknessPrs.h>
#include <asiVisu_TriangulationSource.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiAlgo includes
#include <asiAlgo_ReadSTEPWithMeta.h>
#include <asiAlgo_STEP.h>
#include <asiAlgo_STEPReduce.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>
#include <asiAlgo_WriteDXF.h>

// OpenCascade includes
#include <BRepTools.hxx>

// VTK includes
#include <vtkXMLPolyDataWriter.h>

//-----------------------------------------------------------------------------

void onModelLoaded(const TopoDS_Shape& loadedShape)
{
  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    asiEngine_Part(cmdEngine::model).Update(loadedShape);
  }
  cmdEngine::model->CommitCommand();

  if ( cmdEngine::cf )
  {
    // Update viewer.
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

    // Update object browser.
    cmdEngine::cf->ObjectBrowser->Populate();
  }
}

//-----------------------------------------------------------------------------

void onModelLoaded(const Handle(Poly_Triangulation)& loadedMesh)
{
  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    cmdEngine::model->GetTriangulationNode()->SetTriangulation(loadedMesh);
  }
  cmdEngine::model->CommitCommand();

  if ( cmdEngine::cf )
  {
    // Update viewer.
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetTriangulationNode() );
  }
}

//-----------------------------------------------------------------------------

int ENGINE_LoadStep(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename(argv[1]);

  // Prepare output
  Handle(asiEngine_STEPReaderOutput)
    output = new asiEngine_STEPReaderOutput(cmdEngine::model);

  // Prepare translator
  asiAlgo_ReadSTEPWithMeta reader( interp->GetProgress(),
                                   interp->GetPlotter() );
  reader.SetOutput(output);

  TIMER_NEW
  TIMER_GO

  // Load from STEP
  cmdEngine::model->OpenCommand(); // tx start
  {
    if ( !reader.Perform(filename) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "STEP reader failed.");
      //
      cmdEngine::model->AbortCommand();
      return TCL_ERROR;
    }
  }
  cmdEngine::model->CommitCommand();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Load STEP file")

  // Update viewer.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

  // Update object browser.
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveStep(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename = ( argc == 2 ? argv[1] : argv[2] );

  TopoDS_Shape shape;
  if ( argc == 2 )
  {
    // Get Part Node to access shape.
    Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
    //
    if ( partNode.IsNull() || !partNode->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
      return TCL_ERROR;
    }
    //
    shape = partNode->GetShape();
  }
  else
  {
    // Get topological variable.
    Handle(asiData_IVTopoItemNode)
      topoItem = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
    //
    if ( topoItem.IsNull() || !topoItem->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
      return TCL_OK;
    }
    //
    shape = topoItem->GetShape();
  }

  // Save STEP.
  if ( !asiAlgo_STEP( interp->GetProgress() ).Write(shape, filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save STEP file.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveBrep(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 && argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename = ( argc == 2 ? argv[1] : argv[2] );

  TopoDS_Shape shape;
  if ( argc == 2 )
  {
    // Get Part Node to access shape.
    Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
    //
    if ( partNode.IsNull() || !partNode->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
      return TCL_ERROR;
    }
    //
    shape = partNode->GetShape();
  }
  else
  {
    // Get topological variable.
    Handle(asiData_IVTopoItemNode)
      topoItem = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
    //
    if ( topoItem.IsNull() || !topoItem->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
      return TCL_OK;
    }
    //
    shape = topoItem->GetShape();
  }

  // Save BREP.
  if ( !BRepTools::Write( shape, filename.ToCString() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save BREP file.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveDxf(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  // Check arguments.
  if ( argc < 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get shape to export.
  TopoDS_Shape shape;
  std::string  varName;
  const bool   isVar = interp->GetKeyValue(argc, argv, "var", varName);
  //
  if ( !isVar )
  {
    // Get Part Node to access shape.
    Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
    //
    if ( partNode.IsNull() || !partNode->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
      return TCL_ERROR;
    }
    //
    shape = partNode->GetShape();
  }
  else
  {
    // Get topological variable.
    Handle(asiData_IVTopoItemNode)
      topoItem = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName( varName.c_str() ) );
    //
    if ( topoItem.IsNull() || !topoItem->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << varName);
      return TCL_ERROR;
    }
    //
    shape = topoItem->GetShape();
  }

  // Get filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify filename with -filename key.");
    return TCL_ERROR;
  }

  // Get the segment length.
  double seglen = 0.;
  //
  if ( !interp->GetKeyValue<double>(argc, argv, "seglen", seglen) || (seglen < 1e-6) )
    seglen = 1.0; // The default arc length for freeform curves.

  // Get the format version.
  int ver = 0;
  //
  if ( !interp->GetKeyValue<int>(argc, argv, "ver", ver) )
    ver = 14;

  TIMER_NEW
  TIMER_GO

  // Export to DXF.
  asiAlgo_WriteDXF exportDxf( filename.c_str(),
                              interp->GetProgress(),
                              interp->GetPlotter() );
  //
  exportDxf.SetAutoOrient( interp->HasKeyword(argc, argv, "orient") );
  //
  if ( !exportDxf.CanOpen() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Could not open file for writing '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

  // Set props.
  exportDxf.SetSegmentLength(seglen);
  exportDxf.SetDxfVersion(ver);

  // Translate and write.
  if ( !exportDxf.Perform(shape) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Could not export DXF into '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "DXF export")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_LoadBRep(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename(argv[1]);

  // Read BREP
  TopoDS_Shape shape;
  if ( !asiAlgo_Utils::ReadBRep(filename, shape) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read BREP file.");
    return TCL_ERROR;
  }

  onModelLoaded(shape);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_LoadPart(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename(argv[1]);

   // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    if ( !asiEngine_Part(cmdEngine::model).Import(filename) )
    {
      return TCL_ERROR;
    }
  }
  cmdEngine::model->CommitCommand();

  if ( cmdEngine::cf )
  {
    // Update viewer.
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );

    // Update object browser.
    cmdEngine::cf->ObjectBrowser->Populate();
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_LoadSTL(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename(argv[1]);

  // Read STL
  Handle(Poly_Triangulation) mesh;
  if ( !asiAlgo_Utils::ReadStl( filename, mesh, interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read STL file.");
    return TCL_ERROR;
  }

  onModelLoaded(mesh);

  return TCL_OK;
}


//-----------------------------------------------------------------------------

int ENGINE_DumpAAGJSON(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node and its AAG.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_ERROR;
  }
  //
  Handle(asiAlgo_AAG) aag = partNode->GetAAG();
  //
  if ( aag.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "AAG is null.");
    return TCL_ERROR;
  }

  // Dump to file.
  std::ofstream filestream(argv[1]);
  //
  if ( !filestream.is_open() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "FILE_DEBUG: file cannot be opened.");
    return TCL_ERROR;
  }
  //
  aag->DumpJSON(filestream);
  filestream << "\n";
  //
  filestream.close();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_LoadPoints(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename(argv[2]);

  // Load point cloud
  Handle(asiAlgo_BaseCloud<double>) cloud = new asiAlgo_BaseCloud<double>;
  //
  if ( !cloud->Load( filename.ToCString() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot load point cloud.");
    return TCL_ERROR;
  }
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Point cloud was loaded successfully.");

  interp->GetPlotter().REDRAW_POINTS(argv[1], cloud->GetCoordsArray(), Color_White);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_ReduceSTEP(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  std::string inFilename(argv[1]);
  std::string outFilename(argv[2]);

  TIMER_NEW
  TIMER_GO

  // Run the compression tool.
  asiAlgo_STEPReduce ReduceTool( interp->GetProgress(),
                                 interp->GetPlotter() );
  //
  if ( !ReduceTool.Peform(inFilename, outFilename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "STEP reduction failed.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Reduce STEP")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DumpThicknessVTP(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  // ID of the Node to dump.
  TCollection_AsciiString nodeId;
  if ( !interp->GetKeyValue(argc, argv, "id", nodeId) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify the ID of "
                                                           "the Thickness Node to dump.");
    return TCL_ERROR;
  }

  // Find the Thickness Node.
  Handle(asiData_ThicknessNode)
    TN = Handle(asiData_ThicknessNode)::DownCast( cmdEngine::cf->Model->FindNode(nodeId) );
  //
  if ( TN.IsNull() || !TN->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The Node %1 is not a Thickness Node."
                                                        << nodeId);
    return TCL_ERROR;
  }

  // Output filename.
  TCollection_AsciiString outFilename;
  if ( !interp->GetKeyValue(argc, argv, "filename", outFilename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, specify output filename.");
    return TCL_ERROR;
  }

  if ( !cmdEngine::cf->ViewerPart )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part viewer is not available.");
    return TCL_ERROR;
  }

  // Get pipeline to access the data source.
  Handle(asiVisu_ThicknessPrs)
    prs = Handle(asiVisu_ThicknessPrs)::DownCast( cmdEngine::cf->ViewerPart->PrsMgr()->GetPresentation(TN) );
  //
  Handle(asiVisu_MeshEScalarPipeline)
    pl = Handle(asiVisu_MeshEScalarPipeline)::DownCast( prs->GetPipeline(asiVisu_ThicknessPrs::Pipeline_Main) );
  //
  asiVisu_TriangulationSource* pSource = pl->GetSource();
  asiVisu_MeshEScalarFilter*   pFilter = pl->GetScalarFilter();

  // Update and dump to file.
  pFilter->Update();
  //
  vtkSmartPointer<vtkXMLPolyDataWriter>
    writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  //
  writer->SetFileName( outFilename.ToCString() );
  writer->SetInputConnection( pFilter->GetOutputPort() );
  writer->Write();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdEngine::Commands_Interop(const Handle(asiTcl_Interp)&      interp,
                                 const Handle(Standard_Transient)& cmdEngine_NotUsed(data))
{
  static const char* group = "cmdEngine";

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-step",
    //
    "load-step <filename>\n"
    "\t Loads STEP file to the active part.",
    //
    __FILE__, group, ENGINE_LoadStep);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-step",
    //
    "save-step [<varName>] <filename>\n"
    "\t Save active part or variable (if specified) to a STEP file with the\n"
    "\t given name.",
    //
    __FILE__, group, ENGINE_SaveStep);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-brep",
    //
    "save-brep [<varName>] <filename>\n"
    "\t Save active part or variable (if specified) to a BREP file with the\n"
    "\t given name.",
    //
    __FILE__, group, ENGINE_SaveBrep);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-dxf",
    //
    "save-dxf [-var <var-name>] -filename <filename> [-seglen <seglen>] [-ver <ver>] [-orient]\n"
    "\t Exports the shape variable named <var-name> or the part shape to DXF file <filename>.\n"
    "\t Pass the <seglen> optional value to control the discretization of splines.\n"
    "\t Pass the <ver> optional value to specify the format version of DXF (14 is the default).\n"
    "\t If the '-orient' flag is passed, Analysis Situs will attempt to relocate the shape to\n"
    "\t the XOY plane.",
    //
    __FILE__, group, ENGINE_SaveDxf);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-brep",
    //
    "load-brep <filename>\n"
    "\t Loads BREP file to the active part.",
    //
    __FILE__, group, ENGINE_LoadBRep);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-part",
    //
    "load-part <filename>\n"
    "\t Loads CAD file of any supported format to the active part.",
    //
    __FILE__, group, ENGINE_LoadPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-stl",
    //
    "load-stl <filename>\n"
    "\t Loads STL file to the active triangulation.",
    //
    __FILE__, group, ENGINE_LoadSTL);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump-aag-json",
    //
    "dump-aag-json <filename>\n"
    "\t Dumps AAG of the active part to JSON file.",
    //
    __FILE__, group, ENGINE_DumpAAGJSON);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-points",
    //
    "load-points <name> <filename>\n"
    "\t Loads points from file to the point cloud with the given name.",
    //
    __FILE__, group, ENGINE_LoadPoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("reduce-step",
    //
    "reduce-step <inFlename> <outFilename>\n"
    "\t Applies STEP reduction procedure developed by Seth Hillbrand for KICAD.",
    //
    __FILE__, group, ENGINE_ReduceSTEP);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump-thickness-vtp",
    //
    "dump-thickness-vtp -id <nodeId> -filename <filename>\n"
    "\t Dumps thickness field to the VTP file.",
    //
    __FILE__, group, ENGINE_DumpThicknessVTP);
}
