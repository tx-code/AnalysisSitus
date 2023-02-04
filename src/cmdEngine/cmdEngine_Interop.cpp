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
#include <asiEngine_IVTopoItemSTEPWriterInput.h>
#include <asiEngine_Model.h>
#include <asiEngine_Part.h>
#include <asiEngine_STEPReaderOutput.h>
#include <asiEngine_STEPWriterInput.h>
#include <asiEngine_Triangulation.h>

// asiVisu includes
#include <asiVisu_MeshEScalarFilter.h>
#include <asiVisu_MeshEScalarPipeline.h>
#include <asiVisu_ThicknessPrs.h>
#include <asiVisu_TriangulationSource.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiAlgo includes
#include <asiAlgo_FileFormat.h>
#include <asiAlgo_MeshMerge.h>
#include <asiAlgo_ReadSTEPWithMeta.h>
#include <asiAlgo_STEP.h>
#include <asiAlgo_STEPReduce.h>
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>
#include <asiAlgo_WriteDXF.h>
#include <asiAlgo_WriteSTEPWithMeta.h>
#include <asiAlgo_WriteSVG.h>
//
#if defined USE_MOBIUS
  #include <asiAlgo_MobiusProgressNotifier.h>
#endif

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// asiUI includes
#include <asiUI_XdeBrowser.h>

// glTF includes
#include <asiAsm_GLTFWriter.h>
#include <asiAsm_GLTFXdeDataSourceProvider.h>

// DF Browser includes
#include <DFBrowser.hxx>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <UnitsMethods.hxx>

// VTK includes
#pragma warning(push, 0)
#include <vtkCamera.h>
#include <vtkXMLPolyDataWriter.h>
#pragma warning(pop)

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QMainWindow>
#include <QTextStream>
#include <QVBoxLayout>
#pragma warning(pop)

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/geom_ReadAstra.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

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
#if defined USE_MOBIUS
  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    cmdEngine::model->GetTriangulationNode()->SetTriangulation( cascade::GetMobiusMesh(loadedMesh) );
  }
  cmdEngine::model->CommitCommand();

  if ( cmdEngine::cf )
  {
    // Update viewer.
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetTriangulationNode() );
  }
#else
  cmdEngine::cf->Progress.SendLogMessage(LogErr(Normal) << "Mobius is not available.");
#endif
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

  Handle(asiData_PartNode)
    partNode = cmdEngine::model->GetPartNode();

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

    partNode->SetFilenameIn    ( filename );
    partNode->SetOriginalUnits ( reader.GetUnitString() ); // Units as defined in the original file.
  }
  cmdEngine::model->CommitCommand();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Load STEP file")

  // Update viewer.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

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

  Handle(asiAlgo_WriteSTEPWithMetaInput) input;

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
    input = new asiEngine_STEPWriterInput(cmdEngine::model);
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
    input = new asiEngine_IVTopoItemSTEPWriterInput(topoItem, cmdEngine::model);
  }

  asiAlgo_WriteSTEPWithMeta writer(interp->GetProgress(), interp->GetPlotter());
  writer.SetInput(input);
  if ( !writer.Perform(filename) )
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

int ENGINE_SaveGLTF(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  // Get Part Node to access shape.
  asiEngine_Part partApi(cmdEngine::model);
  //
  auto partNode = partApi.GetPart();
  if (partNode.IsNull() || !partNode->IsWellFormed())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_ERROR;
  }

  // Get the output filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not provided.");
    return TCL_ERROR;
  }
  //
  TCollection_AsciiString ext = filename.c_str();
  ext.LowerCase();

  // Get solid shape as currently glTF works only for solids.
  TopoDS_Shape partShape = partApi.GetShape();
  //
  TopTools_IndexedMapOfShape partSolids;
  TopExp::MapShapes(partShape, TopAbs_SOLID, partSolids);
  //
  if ( partSolids.IsEmpty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There are no solids in the part to export.");
    return TCL_ERROR;
  }

  // Prepare XDE document.
  Handle(asiAsm::xde::Doc) xdeDoc = new asiAsm::xde::Doc;

  // Add the shape's color. It is necessary to do because partApi.GetMetadata()
  // does not store solids, so we need to add part node to the doc separately.
  xdeDoc->GetShapeTool()->AddShape(partShape);
  //
  TDF_Label label;
  xdeDoc->GetShapeTool()->FindShape(partShape, label);
  //
  auto colorInt = partNode->GetColor();
  auto color = asiVisu_Utils::IntToColor(colorInt);
  auto colorRGB = Quantity_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB);
  xdeDoc->SetColor(label, colorRGB);

  // Add colors that are stored in metadata.
  Handle(asiData_MetadataNode) meta_n = partApi.GetMetadata();
  // Get shapes with colors.
  asiData_MetadataAttr::t_shapeColorMap map;
  meta_n->GetShapeColorMap(map);
  // Add all colors from meta.
  asiData_MetadataAttr::t_shapeColorMap::Iterator iter(map);
  for (; iter.More(); iter.Next())
  {
      colorInt = iter.Value();
      auto shape = iter.Key();
      bool found = xdeDoc->GetShapeTool()->FindShape(shape, label);
      if (!found)
      {
          xdeDoc->GetShapeTool()->AddShape(shape);
          xdeDoc->GetShapeTool()->FindShape(shape, label);
      }
      color = asiVisu_Utils::IntToColor(colorInt);
      colorRGB = Quantity_Color(color.Red(), color.Green(), color.Blue(), Quantity_TOC_RGB);
      xdeDoc->SetColor(label, colorRGB);
  }

  // Browse XDE document if requested.
  if ( interp->HasKeyword(argc, argv, "browse") )
  {
    // Prepare browser.
    asiUI_XdeBrowser*
      pBrowser = new asiUI_XdeBrowser( xdeDoc,
                                       cmdEngine::cf,
                                       nullptr );
    //
    pBrowser->Populate();

    // Open UI dialog.
    QWidget* pDlg = new QDialog(cmdEngine::cf->MainWindow);
    //
    pDlg->setWindowTitle( "XDE Browser" );
    //
    QVBoxLayout* pDlgLayout = new QVBoxLayout;
    pDlgLayout->setAlignment(Qt::AlignTop);
    pDlgLayout->setContentsMargins(10, 10, 10, 10);
    //
    pDlgLayout->addWidget(pBrowser);
    pDlg->setLayout(pDlgLayout);
    //
    pDlg->show();

    // DFBrowse
    DFBrowser::DFBrowserCall( xdeDoc->GetDocument() );
  }

  // Export to glTF.
  asiAsm::xde::glTFWriter cafWriter(TCollection_AsciiString( filename.c_str() ),
                                     ext.EndsWith(".glb"), nullptr, nullptr);
  //
  cafWriter.SetTransformationFormat(asiAsm::xde::glTFWriterTrsfFormat_TRS);
  cafWriter.SetForcedUVExport(false);
  //
  //const double systemUnitFactor = UnitsMethods::GetCasCadeLengthUnit() * 0.001;
  //cafWriter.ChangeCoordinateSystemConverter().SetInputLengthUnit(systemUnitFactor);
  cafWriter.ChangeCoordinateSystemConverter().SetInputCoordinateSystem(asiAsm::xde::glTFCoordinateSystem_Zup);
  //
  TColStd_IndexedDataMapOfStringString fileInfo;
  fileInfo.Add("Author", "Analysis Situs");
  fileInfo.Add("Organization", "Analysis Situs");

  Handle(asiAsm::xde::glTFXdeDataSourceProvider) dataProvider = new asiAsm::xde::glTFXdeDataSourceProvider(xdeDoc->GetDocument());
  if ( !cafWriter.Perform(dataProvider, fileInfo) )
  {
    xdeDoc->Release();
    return false;
  }

  xdeDoc->Release();
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveSVG(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
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

  // Get the output filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not provided.");
    return TCL_ERROR;
  }

  // Get the direction of projection.
  if ( cmdEngine::cf.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Visualization facilities are not available.");
    return TCL_ERROR;
  }

  vtkCamera*
    pCamera = cmdEngine::cf->ViewerPart->PrsMgr()->GetRenderer()->GetActiveCamera();

  // Read orientation.
  gp_Vec dir;
  double dX, dY, dZ;
  pCamera->GetViewPlaneNormal(dX, dY, dZ);
  //
  dir.SetX(dX);
  dir.SetY(dY);
  dir.SetZ(dZ);

  if ( !asiAlgo_WriteSVG::WriteWithHLR(shape, dir, filename.c_str(), 0.1) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to save SVG.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveFacetsStl(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  const bool isBinary = interp->HasKeyword(argc, argv, "binary");

  // Get Part Node to access shape.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_ERROR;
  }

  // Get part shape.
  TopoDS_Shape shape = partNode->GetShape();
  //
  if ( shape.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part shape is null.");
    return TCL_ERROR;
  }

  // Get the output filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not provided.");
    return TCL_ERROR;
  }

  asiAlgo_MeshMerge meshMerge(shape);

  // Convert shape's inherent mesh to a storable mesh.
  if ( meshMerge.GetResultPoly().IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot create mesh from shape.");
    return TCL_ERROR;
  }

  // Save mesh to STL file.
  if ( !asiAlgo_Utils::WriteStl( meshMerge.GetResultPoly()->GetTriangulation(),
                                 filename.c_str(), isBinary ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save facets to STL file '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Saved to '%1'."
                                                       << filename);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveSTL(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
#ifndef USE_MOBIUS
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "MOBIUS is unavailable.");
  return TCL_ERROR;
#else

  if ( argc != 3 && argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const bool isBinary = interp->HasKeyword(argc, argv, "binary");

  Handle(asiData_TriangulationNode) triangulationNode = cmdEngine::model->GetTriangulationNode();
  //
  if ( triangulationNode.IsNull() || !triangulationNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is not initialized.");
    return TCL_ERROR;
  }

  Handle(Poly_Triangulation) mesh = cascade::GetOpenCascadeMesh(triangulationNode->GetTriangulation());
  //
  if ( mesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh is null.");
    return TCL_ERROR;
  }

  // Get the output filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not provided.");
    return TCL_ERROR;
  }

  // Save mesh to STL file.
  if ( !asiAlgo_Utils::WriteStl(mesh, filename.c_str(), isBinary) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save triangulation to STL file '%1'."
      << filename);
    return TCL_ERROR;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Saved to '%1'."
                                                       << filename);
  return TCL_OK;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_SavePLY(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
#ifndef USE_MOBIUS
  interp->GetProgress().SendLogMessage(LogErr(Normal) << "MOBIUS is unavailable.");
  return TCL_ERROR;
#else

  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiData_TriangulationNode) triangulationNode = cmdEngine::model->GetTriangulationNode();
  //
  if ( triangulationNode.IsNull() || !triangulationNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Triangulation is not initialized.");
    return TCL_ERROR;
  }

  Handle(Poly_Triangulation) mesh = cascade::GetOpenCascadeMesh(triangulationNode->GetTriangulation());
  //
  if ( mesh.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mesh is null.");
    return TCL_ERROR;
  }

  // Get the output filename.
  std::string filename = argv[1];

  // Save mesh to STL file.
  if ( !asiAlgo_Utils::WritePly(mesh, filename.c_str(), nullptr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save triangulation to PLY file '%1'."
      << filename);
    return TCL_ERROR;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Saved to '%1'."
                                                       << filename);
  return TCL_OK;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_LoadBRep(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const bool isAdd = interp->HasKeyword(argc, argv, "add");

  TCollection_AsciiString filename(argv[1]);

  // Get Part Node and its AAG.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part is not initialized.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape partShape = partNode->GetShape();

  // Read BREP
  TopoDS_Shape shape;
  if ( !asiAlgo_Utils::ReadBRep(filename, shape) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read BREP file.");
    return TCL_ERROR;
  }

  if ( !isAdd )
  {
    partShape = shape;
  }
  else
  {
    if ( partShape.IsNull() )
    {
      partShape = shape;
    }
    else
    {
      if ( partShape.ShapeType() == TopAbs_COMPOUND )
      {
        BRep_Builder().Add(partShape, shape);
      }
      else
      {
        TopoDS_Compound comp;
        BRep_Builder().MakeCompound(comp);
        BRep_Builder().Add(comp, partShape);
        BRep_Builder().Add(comp, shape);
        //
        partShape = comp;
      }
    }
  }

  onModelLoaded(partShape);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_LoadIGES(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc < 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename(argv[1]);

  const bool withNames = interp->HasKeyword(argc, argv, "names");

  if ( withNames )
  {
    Handle(asiAsm::xde::Doc)
      doc = new asiAsm::xde::Doc( interp->GetProgress(),
                                  interp->GetPlotter() );

    // Read IGES file into a document.
    if ( !doc->LoadIGES(filename) )
    {
      doc->Release();
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "IGES file '%1' cannot be loaded."
                                                          << filename);
      return TCL_ERROR;
    }

    // Get all unique parts.
    asiAsm::xde::PartIds partIds;
    doc->GetParts(partIds);

    // Add all parts.
    for ( asiAsm::xde::PartIds::Iterator pit(partIds); pit.More(); pit.Next() )
    {
      const asiAsm::xde::PartId& pid = pit.Value();

      TCollection_ExtendedString name  = doc->GetPartName(pid);
      TopoDS_Shape               shape = doc->GetShape(pid);
      //
      interp->GetPlotter().REDRAW_SHAPE(name, shape, Color_Default);
    }
  }
  else
  {
    // Read B-rep from IGES.
    TopoDS_Shape shape;
    if ( !asiAlgo_Utils::ReadIGES(filename, shape) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read IGES file.");
      return TCL_ERROR;
    }

    onModelLoaded(shape);
  }

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

  asiAlgo_FileFormat
    format = asiAlgo_FileFormatTool::FormatFromFileContent(filename);
  //
  if ( format == FileFormat_Unknown )
  {
    // Recognize file format from file extension.
    format = asiAlgo_FileFormatTool::FormatFromFileExtension(filename);
  }

  if ( asiAlgo_FileFormatTool::IsMeshFormat(format) )
  {
    cmdEngine::model->OpenCommand();
    {
      if ( !asiEngine_Triangulation(cmdEngine::model, interp->GetProgress()).Import(filename) )
      {
        cmdEngine::model->AbortCommand();
        return TCL_ERROR;
      }
    }
    cmdEngine::model->CommitCommand();

    if ( cmdEngine::cf )
    {
      // Update viewer.
      cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetTriangulationNode() );
    }
  }
  else
  {
    // Modify Data Model.
    cmdEngine::model->OpenCommand();
    {
      if ( !asiEngine_Part( cmdEngine::model, interp->GetProgress() ).Import(filename) )
      {
        cmdEngine::model->AbortCommand();
        return TCL_ERROR;
      }
    }
    cmdEngine::model->CommitCommand();

    if ( cmdEngine::cf )
    {
      // Update viewer.
      cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );
    }
  }

  if ( cmdEngine::cf )
  {
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

int ENGINE_LoadOBJ(const Handle(asiTcl_Interp)& interp,
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
  if ( !asiAlgo_Utils::ReadObj( filename, mesh, interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read OBJ file.");
    return TCL_ERROR;
  }

  onModelLoaded(mesh);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_LoadPLY(const Handle(asiTcl_Interp)& interp,
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
  if ( !asiAlgo_Utils::ReadPly( filename, mesh, interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read PLY file.");
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
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "File '%1' cannot be opened for writing."
                                                        << argv[1]);
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
  asiVisu_MeshEScalarFilter* pFilter = pl->GetScalarFilter();

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

int ENGINE_DumpAutoread(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  (void) argc;
  (void) argv;

  if ( cmdEngine::cf.IsNull() || !cmdEngine::cf->Console )
    return TCL_OK;

  // Get the contents of Active Script.
  QString txt = cmdEngine::cf->Console->toPlainText();

  // Save to file.
  QFile qFile(asiTcl_AutoLogFilename);
  //
  if ( qFile.open(QIODevice::WriteOnly) )
  {
    QTextStream out(&qFile);
    out << txt;
    qFile.close();
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot write autoread file.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveXYZ(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString filename = argv[2];

  Handle(asiData_IVPointSetNode)
    ptsNode = Handle(asiData_IVPointSetNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
  //
  if ( ptsNode.IsNull() || !ptsNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find points with name %1." << argv[1]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = ptsNode->GetPoints();
  //
  if ( pts.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Selected point cloud is empty.");
    return TCL_ERROR;
  }

  // Save points.
  if ( !pts->SaveAs( filename.ToCString() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save point cloud.");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

static void SimplifyCurve(Handle(Geom_BSplineCurve)& BS,
                          const double               Tol,
                          const int                  MultMin)

{
  double    tol = Tol;
  int       Mult, ii;
  const int NbK = BS->NbKnots();

  for ( Mult = BS->Degree(); Mult > MultMin; Mult-- )
  {
    for ( ii = NbK; ii > 1; ii-- )
    {
      if ( BS->Multiplicity(ii) == Mult )
        BS->RemoveKnot(ii, Mult - 1, tol);
    }
  }
}

//-----------------------------------------------------------------------------

static void SimplifySurface(Handle(Geom_BSplineSurface)& BS,
                            const double                 Tol,
                            const int                    MultMin)

{
  int  multU, multV, ii;
  bool Ok;

  const TColStd_Array1OfReal&    U  = BS->UKnots();
  const TColStd_Array1OfReal&    V  = BS->VKnots();
  const TColStd_Array1OfInteger& UM = BS->UMultiplicities();
  const TColStd_Array1OfInteger& VM = BS->VMultiplicities();

  for ( ii = U.Length() - 1; ii > 1; ii-- )
  {
    Ok    = true;
    multU = UM.Value(ii) - 1;
    for  ( ; Ok && multU > MultMin; multU-- )
    {
      Ok = BS->RemoveUKnot(ii, multU, Tol);
    }
  }

  for ( ii = V.Length() - 1; ii > 1; ii-- )
  {
    Ok    = true;
    multV = VM.Value(ii) - 1;
    for  ( ; Ok && multV > MultMin; multV-- )
    {
      Ok = BS->RemoveVKnot(ii, multV, Tol);
    }
  }
}

//-----------------------------------------------------------------------------

int ENGINE_LoadAstra(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  std::string filename(argv[1]);

  // Read ASTRA file.
  geom_ReadAstra readAstra( MobiusProgress( interp->GetProgress() ) );
  //
  if ( !readAstra.Perform(filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read ASTRA file.");
    return TCL_ERROR;
  }

  int nbLoaded = 0;

  // Get all curves.
  const std::vector< t_ptr<t_bcurve> >& bcurves = readAstra.GetResultCurves();
  //
  for ( const auto& bcurve : bcurves )
  {
    Handle(Geom_BSplineCurve) c3d = cascade::GetOpenCascadeBCurve(bcurve);

    SimplifyCurve(c3d, Precision::Confusion(), 1);

    QString qstr = QString::fromLocal8Bit( bcurve->GetName().c_str() );

    interp->GetPlotter().REDRAW_CURVE( bcurve->HasName() ? QStr2ExtStr(qstr) : "astraCurve",
                                       c3d, Color_Red, true );

    nbLoaded++;
  }

  // Get all surfaces.
  const std::vector< t_ptr<t_surf> >& surfs = readAstra.GetResultSurfaces();
  //
  for ( const auto& surf : surfs )
  {
    Handle(Geom_Surface) s3d;

    // Spline surface.
    t_ptr<t_bsurf> bsurf = t_ptr<t_bsurf>::DownCast(surf);
    //
    if ( !bsurf.IsNull() )
    {
      Handle(Geom_BSplineSurface) occBSurf = cascade::GetOpenCascadeBSurface(bsurf);

      SimplifySurface(occBSurf, Precision::Confusion(), 1);

      s3d = occBSurf;
    }

    // Surface of revolution.
    t_ptr<t_surfRevol> revolSurf = t_ptr<t_surfRevol>::DownCast(surf);
    //
    if ( !revolSurf.IsNull() )
    {
      s3d = cascade::GetOpenCascadeRevolSurf(revolSurf);
    }

    QString qstr = QString::fromLocal8Bit( surf->GetName().c_str() );

    interp->GetPlotter().REDRAW_SURFACE( surf->HasName() ? QStr2ExtStr(qstr) : "astraSurface",
                                         s3d, Color_DarkGray );

    nbLoaded++;
  }

  // Return the number of loaded entities.
  *interp << nbLoaded;

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
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
  interp->AddCommand("save-gltf",
    //
    "save-gltf -filename <filename>\n"
    "\t Exports the part shape to glTF file <filename> with all assigned colors.",
    //
    __FILE__, group, ENGINE_SaveGLTF);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-svg",
    //
    "save-svg [-var <var-name>] -filename <filename>\n"
    "\t Exports the part shape or the specified variable to SVG file <filename>.",
    //
    __FILE__, group, ENGINE_SaveSVG);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-facets-stl",
    //
    "save-facets-stl -filename <filename> [-binary]\n"
    "\t Exports the part shape's facets to STL file <filename>.",
    //
    __FILE__, group, ENGINE_SaveFacetsStl);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-stl",
    //
    "save-stl -filename <filename> [-binary]\n"
    "\t Exports data from triangulation node to STL file <filename>.",
    //
    __FILE__, group, ENGINE_SaveSTL);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-ply",
    //
    "save-ply <filename>\n"
    "\t Exports data from triangulation node to PLY file <filename>.",
    //
    __FILE__, group, ENGINE_SavePLY);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-brep",
    //
    "load-brep <filename> [-add]\n"
    "\t Loads BREP file to the active part. If the '-add' flag is passed,\n"
    "\t the loaded geometry is appended to the active part.",
    //
    __FILE__, group, ENGINE_LoadBRep);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-iges",
    //
    "load-iges <filename> [-names]\n"
    "\t Loads IGES file to the active part or to the imperative plotter's\n"
    "\t section if the '-names' keyword is passed. In the latter cases, all\n"
    "\t IGES entities are imported with their original names instead of putting\n"
    "\t all geometries into a single compound.",
    //
    __FILE__, group, ENGINE_LoadIGES);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-part",
    //
    "load-part <filename>\n"
    "\t Loads CAD file of any supported format to the active part.\n"
    "\t If the model is coming in a mesh format, the data is loaded into the Triangulation Node.",
    //
    __FILE__, group, ENGINE_LoadPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-obj",
    //
    "load-obj <filename>\n"
    "\t Loads OBJ file to the active triangulation.",
    //
    __FILE__, group, ENGINE_LoadOBJ);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-ply",
    //
    "load-ply <filename>\n"
    "\t Loads PLY file to the active triangulation.",
    //
    __FILE__, group, ENGINE_LoadPLY);

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

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump-autoread",
    //
    "dump-autoread\n"
    "\t Dumps the currently entered Active Script commands to a specific file\n"
    "\t named 'autoread.log' that is located in the working directory of\n"
    "\t Analysis Situs. If exists, this file will be automatically loaded\n"
    "\t without execution on the next launch.\n"
    "\n"
    "\t Use this option to make Analysis Situs a kind of a \"notebook\" with your\n"
    "\t recorded commands and comments to get back on each launch.",
    //
    __FILE__, group, ENGINE_DumpAutoread);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-xyz",
    //
    "save-xyz <varName> <filename>\n"
    "\t Save (without metadata) selected points to a XYZ file with the\n"
    "\t given name.",
    //
    __FILE__, group, ENGINE_SaveXYZ);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load-astra",
    //
    "load-astra <filename>\n"
    "\t Loads ASTRA file with curves and surfaces. Returns the number of loaded\n"
    "\t curves and surfaces to the interpreter.",
    //
    __FILE__, group, ENGINE_LoadAstra);
}
