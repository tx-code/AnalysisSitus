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

// asiAlgo includes
#include <asiAlgo_AttrFaceColor.h>
#include <asiAlgo_FacetQuality.h>
#include <asiAlgo_FileDumper.h>
#include <asiAlgo_MeshConvert.h>
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_Utils.h>

// asiData includes
#include <asiData_IVTessItemNode.h>

// asiUI includes
#include <asiUI_DialogOCAFDump.h>

// asiVisu includes
#include <asiVisu_PartPipeline.h>
#include <asiVisu_PartPrs.h>
#include <asiVisu_Utils.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// Active Data includes
#include <ActData_GraphToDot.h>

// DF Browser includes
#include <DFBrowser.hxx>

// VTK includes
#pragma warning(push, 0)
#include <vtkXMLPolyDataWriter.h>
#pragma warning(pop)

#if defined USE_MOBIUS
  #include <mobius/cascade.h>
  #include <mobius/poly_Mesh.h>

  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

void SelectFaceterOptions(const asiAlgo_FacetQuality quality,
                          double&                    linDefl,
                          double&                    angDeflDeg,
                          ActAPI_ProgressEntry       progress)
{
  linDefl    = asiAlgo_LINDEFL_MIN;
  angDeflDeg = asiAlgo_ANGDEFL_MIN;

  // Initialize progress indicator.
  progress.SendLogMessage(LogInfo(Normal) << "Select faceter options for active part");

  asiEngine_Part partApi(cmdEngine::model,
                         (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr);

  double partLinDefl = asiAlgo_LINDEFL_MIN;
  double partAngDefl = asiAlgo_ANGDEFL_MIN;
  double partMinLinDefl;

  // Get shape for the part.
  TopoDS_Shape partShape = partApi.GetShape();

  if ( !asiAlgo_Utils::IsEmptyShape(partShape) &&
        asiAlgo_MeshGen::AutoSelectLinearDeflection(partShape, partMinLinDefl, 0.01) )
  {
    asiAlgo_SelectFaceterOptions(quality, partMinLinDefl, partLinDefl, partAngDefl);
  }

  linDefl    = Max(linDefl,    partLinDefl);
  angDeflDeg = Max(angDeflDeg, partAngDefl);

  if ( progress.IsCancelling() )
  {
    progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
    return;
  }
}


//-----------------------------------------------------------------------------

void onUndoRedo(const Handle(ActAPI_TxRes)& txRes,
                ActAPI_ProgressEntry        progress)
{
  if ( txRes.IsNull() )
    return;

  if ( !cmdEngine::cf )
    return;

  // Loop over the affected Parameters to get the affected Nodes. These Nodes
  // are placed into a map to have them unique.
  ActAPI_NodeIdMap modifiedNodes, deletedNodes;
  //
  for ( int k = 1; k <= txRes->parameterRefs.Extent(); ++k )
  {
    const ActAPI_TxRes::t_parameterRef& objRef = txRes->parameterRefs(k);

    // Get ID of the Node.
    ActAPI_NodeId nodeId = ActData_Common::NodeIdByParameterId(objRef.id);

    // Add Node ID.
    if ( objRef.isAlive )
    {
      if ( !modifiedNodes.Contains(nodeId) )
      {
        modifiedNodes.Add(nodeId);
        //
        progress.SendLogMessage(LogInfo(Normal) << "Modified Node: %1." << nodeId);
      }
    }
    else
    {
      if ( !deletedNodes.Contains(nodeId) )
      {
        deletedNodes.Add(nodeId);
        //
        progress.SendLogMessage(LogInfo(Normal) << "Deleted Node: %1." << nodeId);
      }
    }
  }

  // Get all presentation managers
  const vtkSmartPointer<asiVisu_PrsManager>& partPM   = cmdEngine::cf->ViewerPart->PrsMgr();
  const vtkSmartPointer<asiVisu_PrsManager>& hostPM   = cmdEngine::cf->ViewerHost->PrsMgr();
  const vtkSmartPointer<asiVisu_PrsManager>& domainPM = cmdEngine::cf->ViewerDomain->PrsMgr();

  // Loop over the deleted Nodes to derender them
  for ( int k = 1; k <= deletedNodes.Extent(); ++k )
  {
    const ActAPI_DataObjectId& id = deletedNodes(k);

    if ( partPM->IsPresented(id) )
      partPM->DeRenderPresentation(id);
    //
    if ( hostPM->IsPresented(id) )
      hostPM->DeRenderPresentation(id);
    //
    if ( domainPM->IsPresented(id) )
      domainPM->DeRenderPresentation(id);
  }

  // Loop over the modified Nodes to actualize them
  for ( int k = 1; k <= modifiedNodes.Extent(); ++k )
  {
    const ActAPI_DataObjectId& id = modifiedNodes(k);

    // Actualize
    if ( partPM->IsPresented(id) )
      partPM->Actualize( cmdEngine::model->FindNode(id) );
    //
    if ( hostPM->IsPresented(id) )
      hostPM->Actualize( cmdEngine::model->FindNode(id) );
    //
    if ( domainPM->IsPresented(id) )
      domainPM->Actualize( cmdEngine::model->FindNode(id) );
  }

  // Update object browser
  cmdEngine::cf->ObjectBrowser->Populate();
}

//-----------------------------------------------------------------------------

int ENGINE_WhatIs(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  std::string label;

  Handle(asiTcl_Variable) var = interp->GetVar(argv[1]);
  //
  if ( var.IsNull() )
    label = "nothing";
  else
    label = var->WhatIs();

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "'%1' is '%2'."
                                                       << argv[1] << label);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Dump(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  std::stringstream out;

  Handle(asiTcl_Variable) var = interp->GetVar(argv[1]);
  //
  if ( !var.IsNull() )
  {
    var->Dump(out);

    interp->GetProgress().SendLogMessage( LogInfo(Normal) << out.str() );
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SaveAs(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Print format for information.
  TCollection_AsciiString docFormat = cmdEngine::model->Document()->StorageFormat();
  //
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Document format is %1."
                                                       << docFormat);

  // Save.
  if ( !cmdEngine::model->SaveAs( argv[1], interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Saving failed.");
    return TCL_ERROR;
  }
  //
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Model was saved to %1."
                                                       << argv[1]);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Load(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Release current project.
  cmdEngine::model->Release();

  // Open.
  if ( !cmdEngine::model->Open( argv[1], interp->GetProgress() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Loading failed.");
    return TCL_ERROR;
  }
  //
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Model was loaded from %1."
                                                       << argv[1]);

  // Find all presentable Nodes.
  Handle(ActAPI_HNodeList)
    nodes = asiEngine_Base(cmdEngine::model).FindPresentableNodes();

  // Update UI.
  cmdEngine::cf->ObjectBrowser->Populate();
  //
  cmdEngine::cf->ViewerPart   ->PrsMgr()->DeleteAllPresentations();
  cmdEngine::cf->ViewerDomain ->PrsMgr()->DeleteAllPresentations();
  cmdEngine::cf->ViewerHost   ->PrsMgr()->DeleteAllPresentations();
  //
  cmdEngine::cf->ViewerPart->PrsMgr()->ActualizeCol(nodes);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Undo(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Undo and process the affected Parameters
  onUndoRedo( cmdEngine::model->Undo(), interp->GetProgress() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Redo(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Redo and process the affected Parameters
  onUndoRedo( cmdEngine::model->Redo(), interp->GetProgress() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetAsVar(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) part_n = cmdEngine::model->GetPartNode();

  // Erase Part Node for convenience.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->DeRenderPresentation(part_n);

  // Draw.
  interp->GetPlotter().REDRAW_SHAPE( argv[1], part_n->GetShape(true), Color_Default, 1. );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetAsPart(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  if ( argc != 2 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Faceter parameters to copy to Part Node.
  double linDefl = 0.;
  double angDefl = 0.;

  // Extract shape to set as a part.
  TopoDS_Shape shapeToSet;
  //
  if ( argc == 2 )
  {
    Handle(asiData_IVTopoItemNode)
      node = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
    //
    if ( node.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << argv[1]);
      return TCL_OK;
    }

    // Get shape to convert.
    shapeToSet = node->GetShape();
    linDefl    = node->GetLinearDeflection();
    angDefl    = node->GetAngularDeflection();

    // It is usually convenient to erase the source Node.
    if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
      cmdEngine::cf->ViewerPart->PrsMgr()->DeRenderPresentation(node);
  }
  else
  {
    TCollection_AsciiString nodeIdStr, paramIdStr;
    //
    if ( !interp->GetKeyValue(argc, argv, "node", nodeIdStr) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected format of arguments.");
      return TCL_OK;
    }
    //
    if ( !interp->GetKeyValue(argc, argv, "param", paramIdStr) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Unexpected format of arguments.");
      return TCL_OK;
    }

    // Get Data Node.
    Handle(ActAPI_INode) node = cmdEngine::model->FindNode(nodeIdStr);
    //
    if ( node.IsNull() || !node->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Data object with ID %1 does not exist."
                                                          << nodeIdStr);
      return TCL_OK;
    }

    // Get Parameter.
    if ( !paramIdStr.IsIntegerValue() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Parameter ID %1 is not numerical."
                                                          << paramIdStr);
      return TCL_OK;
    }
    //
    const int                     PID   = paramIdStr.IntegerValue();
    Handle(ActAPI_IUserParameter) param = node->Parameter(PID);
    //
    if ( param.IsNull() || !param->IsWellFormed() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Parameter with ID %1 does not exist."
                                                          << PID);
      return TCL_OK;
    }

    // Convert to Shape Parameter.
    Handle(ActData_ShapeParameter)
      shapeParam = Handle(ActData_ShapeParameter)::DownCast(param);
    //
    if ( shapeParam.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Parameter with ID %1 is not of Shape type."
                                                          << PID);
      return TCL_OK;
    }

    // Set shape to convert.
    shapeToSet = shapeParam->GetShape();
  }

  // Modify Data Model.
  Handle(asiData_PartNode) part_n;
  //
  cmdEngine::model->OpenCommand();
  {
    part_n = asiEngine_Part(cmdEngine::model).Update(shapeToSet);
    //
    part_n->SetLinearDeflection(linDefl);
    part_n->SetAngularDeflection(angDefl);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
  {
    // Disable notifier to avoid async events.
    cmdEngine::cf->ViewerPart->PrsMgr()->SetDiagnosticTools(nullptr, nullptr);
    //
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(part_n);
    //
    cmdEngine::cf->ViewerPart->PrsMgr()->SetDiagnosticTools(interp->GetProgress(),
                                                            interp->GetPlotter());
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetAsTriangulation(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
#if defined USE_MOBIUS
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiData_IVTessItemNode)
    node = Handle(asiData_IVTessItemNode)::DownCast( cmdEngine::model->FindNodeByName(argv[1]) );
  //
  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find tessellation with name %1." << argv[1]);
    return TCL_ERROR;
  }

  // Get triangulation to convert.
  Handle(Poly_Triangulation) trisToSet;
  //
  if ( !asiAlgo_MeshConvert::FromPersistent(node->GetMesh(), trisToSet) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot convert tessellation to triangulation.");
    return TCL_ERROR;
  }

  // It is usually convenient to erase the source Node.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->DeRenderPresentation(node);

  // Modify Data Model.
  cmdEngine::model->OpenCommand();
  {
    cmdEngine::model->GetTriangulationNode()->SetTriangulation( cascade::GetMobiusMesh(trisToSet) );
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetTriangulationNode() );

  return TCL_OK;
#else
  (void) argc;
  (void) argv;

  interp->GetProgress().SendLogMessage(LogErr(Normal) << "Mobius is not available.");
  return TCL_ERROR;
#endif
}

//-----------------------------------------------------------------------------

int ENGINE_GetId(const Handle(asiTcl_Interp)& interp,
                 int                          argc,
                 const char**                 argv)
{
  if ( (argc != 2) && (argc != 3) )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get name of the target Node as a path.
  std::string namePath(argv[1]);

  Handle(ActAPI_INode) node;

  // Split by delimeter.
  std::vector<std::string> names;
  //
  if ( !interp->HasKeyword(argc, argv, "nosplit") )
  {
    asiAlgo_Utils::Str::Split(namePath, "/", names);

    // Prepare a collection of object names for Active Data.
    std::vector<TCollection_ExtendedString> adNames;
    //
    for ( size_t k = 0; k < names.size(); ++k )
    {
      TCollection_AsciiString adName( names[k].c_str() );
      adName.LeftAdjust();
      adName.RightAdjust();
      //
      adNames.push_back(adName);
    }

    // Find Node.
    node = cmdEngine::model->FindNodeByNames(adNames);
  }
  else
  {
    node = cmdEngine::model->FindNodeByName( namePath.c_str() );
  }

  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with name '%1'."
                                                        << namePath);
    return TCL_OK;
  }

  // Get ID of the object.
  ActAPI_DataObjectId id = node->GetId();
  //
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Object ID: %1." << id);

  // Send to interpreter.
  *interp << id;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_GetName(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  ActAPI_DataObjectId id(argv[1]);

  // Find Node.
  Handle(ActAPI_INode) node = cmdEngine::model->FindNode(id);
  //
  if ( node.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find object with ID %1." << id);
    return TCL_OK;
  }

  TCollection_ExtendedString name;

  // Name is not necessarily a Parameter of a Data Node, so Active Data does not
  // come with any default `GetName()/SetName()` methods for us to call here.
  Handle(ActAPI_IParamIterator) pit = node->GetParamIterator();
  //
  for ( ; pit->More(); pit->Next() )
  {
    Handle(ActData_NameParameter) NP = ActParamTool::AsName( pit->Value() );
    //
    if ( NP.IsNull() )
      continue;

    name = NP->GetValue();
    break;
  }

  // Get name of the object.
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Object name: '%1'." << name);

  // Send to interpreter.
  *interp << name;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Clear(const Handle(asiTcl_Interp)& interp,
                 int                          argc,
                 const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Clear viewers
  cmdEngine::ClearViewers();

  // Clear data
  cmdEngine::model->Clear();

  // Update object browser
  if ( cmdEngine::cf )
    cmdEngine::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DumpProject(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  asiUI_DialogOCAFDump*
    pDumpProject = new asiUI_DialogOCAFDump( cmdEngine::model, interp->GetProgress() );
  //
  pDumpProject->Populate();
  //
  pDumpProject->show();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DFBrowse(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  (void) argc;
  (void) argv;

  Handle(ActData_BaseModel)
    M = Handle(ActData_BaseModel)::DownCast( interp->GetModel() );

  DFBrowser::DFBrowserCall( M->Document() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetFaceColorAAG(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 3 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get face ID (if passed).
  int fid = 0;
  TCollection_AsciiString fidStr;
  //
  if ( interp->GetKeyValue(argc, argv, "fid", fidStr) )
    fid = fidStr.IntegerValue();

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  // Get Part Node to access AAG and optionally the selected faces.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  Handle(asiAlgo_AAG) aag = partNode->GetAAG();
  //
  if ( aag.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "AAG is null.");
    return TCL_ERROR;
  }

  // If the face ID is not passed, get the selected face.
  TColStd_PackedMapOfInteger fids;
  //
  if ( !fid )
  {
    asiEngine_Part partAPI( cmdEngine::model, cmdEngine::cf->ViewerPart->PrsMgr() );
    partAPI.GetHighlightedFaces(fids);
  }
  else
    fids.Add(fid);

  // Add color attributes to AAG.
  for ( TColStd_MapIteratorOfPackedMapOfInteger fit(fids); fit.More(); fit.Next() )
  {
    const int currFid = fit.Key();

    Handle(asiAlgo_FeatureAttr)
      attrBase = aag->GetNodeAttribute( currFid, asiAlgo_AttrFaceColor::GUID() );

    // Create a new color attribute or get the existing one.
    Handle(asiAlgo_AttrFaceColor) attrColor;
    //
    if ( attrBase.IsNull() )
    {
      attrColor = new asiAlgo_AttrFaceColor;

      if ( !aag->SetNodeAttribute(currFid, attrColor) )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot set color attribute to AAG.");
        return TCL_ERROR;
      }
    }
    else
      attrColor = Handle(asiAlgo_AttrFaceColor)::DownCast(attrBase);

    // Adjust color.
    attrColor->SetColor(colorComponents[0],
                        colorComponents[1],
                        colorComponents[2]);
  }

  // Touch Parameter and actualize.
  // TODO: that's an overkill !!!
  cmdEngine::model->OpenCommand();
  {
    partNode->Parameter(asiData_PartNode::PID_AAG)->SetModified();
  }
  cmdEngine::model->CommitCommand();
  //
  cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(partNode);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetFaceColor(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 3 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get face ID (if passed).
  int fid = 0;
  TCollection_AsciiString fidStr;
  //
  if ( interp->GetKeyValue(argc, argv, "fid", fidStr) )
    fid = fidStr.IntegerValue();

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  asiEngine_Part partApi( cmdEngine::model,
                          (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );

  // If the face ID is not passed, get the selected face.
  TColStd_PackedMapOfInteger fids;
  //
  if ( !fid )
    partApi.GetHighlightedFaces(fids);
  else
    fids.Add(fid);

  Handle(asiData_MetadataNode) meta_n = partApi.GetMetadata();

  cmdEngine::model->OpenCommand();
  {
    // Add metadata.
    for ( TColStd_MapIteratorOfPackedMapOfInteger fit(fids); fit.More(); fit.Next() )
    {
      TopoDS_Shape shape = partApi.GetFace( fit.Key() );
      //
      const int icolor = asiVisu_Utils::ColorToInt(colorComponents[0],
                                                   colorComponents[1],
                                                   colorComponents[2]);
      //
      interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Setting face color to %1." << icolor);
      //
      meta_n->SetColor(shape, icolor);
    }
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_GetFaceColor(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get face ID.
  int fid = 0;
  TCollection_AsciiString fidStr;
  //
  if ( interp->GetKeyValue(argc, argv, "fid", fidStr) )
  {
    fid = fidStr.IntegerValue();
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "'-fid' is not specified.");
    return TCL_ERROR;
  }

  asiEngine_Part partApi( cmdEngine::model,
                         (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );

  Handle(asiData_MetadataNode) meta_n = partApi.GetMetadata();

  asiData_MetadataAttr::t_shapeColorMap map;
  meta_n->GetShapeColorMap(map);

  int          colorInt = 0;
  TopoDS_Shape shape    = partApi.GetFace(fid);
  bool         isFound  = map.Contains(shape);
  //
  if ( isFound )
    colorInt = map.FindFromKey(shape);

  if ( !isFound )
  {
    colorInt = cmdEngine::model->GetPartNode()->GetColor();
    isFound  = true;
  }

  ActAPI_Color color = asiVisu_Utils::IntToColor(colorInt);
  int red   = color.Red()   * MAX_COLOR_SCALE;
  int green = color.Green() * MAX_COLOR_SCALE;
  int blue  = color.Blue()  * MAX_COLOR_SCALE;
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Color: (%1, %2, %3)." << red << green << blue);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_CheckFaceColor(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get face ID.
  int fid = 0;
  TCollection_AsciiString fidStr;
  //
  if ( interp->GetKeyValue(argc, argv, "fid", fidStr) )
  {
    fid = fidStr.IntegerValue();
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "'-fid' is not specified.");
    return TCL_ERROR;
  }

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  asiEngine_Part partApi( cmdEngine::model,
                          (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );

  TopoDS_Shape shape = partApi.GetFace(fid);

  // Get metadata.
  Handle(asiData_MetadataNode) metadata_n = partApi.GetMetadata();

  asiData_MetadataAttr::t_shapeColorMap map;
  metadata_n->GetShapeColorMap(map);

  bool isFound = map.Contains(shape);
  int colorInt = 0;
  //
  if ( isFound )
    colorInt = map.FindFromKey(shape);

  if ( !isFound )
  {
    colorInt = cmdEngine::model->GetPartNode()->GetColor();
    isFound = true;
  }

  ActAPI_Color color = asiVisu_Utils::IntToColor(colorInt);
  int red   = color.Red()   * MAX_COLOR_SCALE;
  int green = color.Green() * MAX_COLOR_SCALE;
  int blue  = color.Blue()  * MAX_COLOR_SCALE;

  Quantity_Color checkedColor(colorComponents[0]/MAX_COLOR_SCALE,
                              colorComponents[1]/MAX_COLOR_SCALE,
                              colorComponents[2]/MAX_COLOR_SCALE,
                              Quantity_TOC_RGB);

  if ( !color.IsEqual(checkedColor) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color of face: (%1, %2, %3)." << red << green << blue);
    return TCL_ERROR;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Color matched.");

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetPartColor(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  asiEngine_Part partApi( cmdEngine::model,
                          (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );

  cmdEngine::model->OpenCommand();
  {
    const int icolor = asiVisu_Utils::ColorToInt(colorComponents[0],
                                                 colorComponents[1],
                                                 colorComponents[2]);
    cmdEngine::model->GetPartNode()->SetColor(icolor);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_GetPartColor(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  asiEngine_Part partApi( cmdEngine::model,
                          (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );


  int colorInt = cmdEngine::model->GetPartNode()->GetColor();

  ActAPI_Color color = asiVisu_Utils::IntToColor(colorInt);
  int red   = color.Red()   * MAX_COLOR_SCALE;
  int green = color.Green() * MAX_COLOR_SCALE;
  int blue  = color.Blue()  * MAX_COLOR_SCALE;
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Color: (%1, %2, %3)." << red << green << blue);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_CheckPartColor(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  asiEngine_Part partApi( cmdEngine::model,
                          (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );

  int colorInt = cmdEngine::model->GetPartNode()->GetColor();

  ActAPI_Color color = asiVisu_Utils::IntToColor(colorInt);
  int red   = color.Red()   * MAX_COLOR_SCALE;
  int green = color.Green() * MAX_COLOR_SCALE;
  int blue  = color.Blue()  * MAX_COLOR_SCALE;

  Quantity_Color checkedColor(colorComponents[0]/MAX_COLOR_SCALE,
                              colorComponents[1]/MAX_COLOR_SCALE,
                              colorComponents[2]/MAX_COLOR_SCALE,
                              Quantity_TOC_RGB);

  if ( !color.IsEqual(checkedColor) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color of part: (%1, %2, %3)." << red << green << blue);
    return TCL_ERROR;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Color matched.");

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetTopoItemColor(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get name.
  TCollection_AsciiString nameOfTopoItem;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", nameOfTopoItem) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Name of topological object is not specified.");
    return TCL_ERROR;
  }

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  Handle(asiData_IVTopoItemNode)
  topoItem = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(nameOfTopoItem) );
  //
  if ( topoItem.IsNull() || !topoItem->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << nameOfTopoItem.ToCString());
    return TCL_ERROR;
  }

  cmdEngine::model->OpenCommand();
  {
    const int icolor = asiVisu_Utils::ColorToInt(colorComponents[0],
                                                 colorComponents[1],
                                                 colorComponents[2]);
    topoItem->SetHasColor(true);
    topoItem->SetColor(icolor);
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( topoItem );
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_GetTopoItemColor(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if ( argc != 3 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get name.
  TCollection_AsciiString nameOfTopoItem;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", nameOfTopoItem) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Name of topological object is not specified.");
    return TCL_ERROR;
  }

  Handle(asiData_IVTopoItemNode)
  topoItem = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(nameOfTopoItem) );
  //
  if ( topoItem.IsNull() || !topoItem->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << nameOfTopoItem.ToCString());
    return TCL_ERROR;
  }

  if ( !topoItem->HasColor() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological object does not have color.");
    return TCL_ERROR;
  }

  int colorInt = topoItem->GetColor();

  ActAPI_Color color = asiVisu_Utils::IntToColor(colorInt);
  int red   = color.Red()   * MAX_COLOR_SCALE;
  int green = color.Green() * MAX_COLOR_SCALE;
  int blue  = color.Blue()   * MAX_COLOR_SCALE;
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Color: (%1, %2, %3)." << red << green << blue);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_CheckTopoItemColor(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
  if ( argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get name.
  TCollection_AsciiString nameOfTopoItem;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", nameOfTopoItem) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Name of topological object is not specified.");
    return TCL_ERROR;
  }

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  Handle(asiData_IVTopoItemNode)
  topoItem = Handle(asiData_IVTopoItemNode)::DownCast( cmdEngine::model->FindNodeByName(nameOfTopoItem) );
  //
  if ( topoItem.IsNull() || !topoItem->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find topological object with name %1." << nameOfTopoItem.ToCString());
    return TCL_ERROR;
  }

  if (!topoItem->HasColor())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Topological object does not have color.");
    return TCL_ERROR;
  }

  int colorInt = topoItem->GetColor();

  ActAPI_Color color = asiVisu_Utils::IntToColor(colorInt);
  int red   = color.Red()   * MAX_COLOR_SCALE;
  int green = color.Green() * MAX_COLOR_SCALE;
  int blue  = color.Blue()  * MAX_COLOR_SCALE;

  Quantity_Color checkedColor(colorComponents[0]/MAX_COLOR_SCALE,
                              colorComponents[1]/MAX_COLOR_SCALE,
                              colorComponents[2]/MAX_COLOR_SCALE,
                              Quantity_TOC_RGB);

  if ( !color.IsEqual(checkedColor) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color of topological object: (%1, %2, %3)." << red << green << blue);
    return TCL_ERROR;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Color matched.");

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_SetEdgeColor(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 3 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get edge ID (if passed).
  int eid = 0;
  TCollection_AsciiString eidStr;
  //
  if ( interp->GetKeyValue(argc, argv, "eid", eidStr) )
    eid = eidStr.IntegerValue();

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Color components are not specified.");
    return TCL_ERROR;
  }

  // Get color components.
  std::vector<unsigned int> colorComponents;
  std::vector<std::string> colorComponentsStr;
  //
  asiAlgo_Utils::Str::Split(colorStr.ToCString(), "(,)", colorComponentsStr);
  //
  for ( size_t k = 0; k < colorComponentsStr.size(); ++k )
  {
    TCollection_AsciiString compStr( colorComponentsStr[k].c_str() );
    //
    if ( compStr.IsIntegerValue() )
      colorComponents.push_back( compStr.IntegerValue() );
  }

  if ( colorComponents.size() != 3 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Three color components expected.");
    return TCL_ERROR;
  }

  asiEngine_Part partApi( cmdEngine::model,
                          (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr );

  // Get Metadata Node.
  Handle(asiData_MetadataNode) metadata_n = partApi.GetMetadata();

  // If the edge ID is not passed, get the selected edge.
  TColStd_PackedMapOfInteger eids;
  //
  if ( !eid )
    partApi.GetHighlightedEdges(eids);
  else
    eids.Add(eid);

  cmdEngine::model->OpenCommand();
  {
    // Add metadata.
    for ( TColStd_MapIteratorOfPackedMapOfInteger eit(eids); eit.More(); eit.Next() )
    {
      TopoDS_Shape edge = partApi.GetEdge( eit.Key() );

      const int icolor = asiVisu_Utils::ColorToInt(colorComponents[0],
                                                   colorComponents[1],
                                                   colorComponents[2]);
      //
      interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Setting edge color to %1." << icolor);
      //
      metadata_n->SetColor(edge, icolor);
    }
  }
  cmdEngine::model->CommitCommand();

  // Update UI.
  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize( cmdEngine::model->GetPartNode() );
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_GetParamShape(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 4 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Node by its ID.
  Handle(ActAPI_INode) N = cmdEngine::model->FindNode(argv[2]);
  //
  if ( N.IsNull() || !N->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Data Node %1 is null or invalid."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get Parameter.
  Handle(ActAPI_IUserParameter) P = N->Parameter( atoi(argv[3]) );
  //
  if ( P.IsNull() || !P->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Parameter %1 is null or invalid."
                                                        << argv[3]);
    return TCL_ERROR;
  }

  // Convert Parameter to shape type.
  Handle(ActData_ShapeParameter)
    SP = Handle(ActData_ShapeParameter)::DownCast(P);
  //
  if ( SP.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The requested Parameter is not of shape type.");
    return TCL_ERROR;
  }

  // Get shape and set it as a result.
  interp->GetPlotter().REDRAW_SHAPE( argv[1], SP->GetShape() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DumpExecutionGraphDot(const Handle(asiTcl_Interp)& interp,
                                 int                          argc,
                                 const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  TCollection_AsciiString graphRepr = ActData_GraphToDot::Convert(cmdEngine::model);

  // Dump to file.
  asiAlgo_FileDumper FILE;
  //
  if ( !FILE.Open(argv[1]) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save to file %1."
                                                        << argv[1]);
    return TCL_ERROR;
  }
  //
  FILE.Dump(graphRepr);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DumpVTP(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdEngine::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }

  // Get pipeline to access the data source.
  Handle(asiVisu_PartPrs)
    partPrs = Handle(asiVisu_PartPrs)::DownCast( cmdEngine::cf->ViewerPart->PrsMgr()->GetPresentation(partNode) );
  //
  Handle(asiVisu_PartPipeline)
    partPl = Handle(asiVisu_PartPipeline)::DownCast( partPrs->GetPipeline(asiVisu_PartPrs::Pipeline_Main) );
  //
  const vtkSmartPointer<asiVisu_ShapeRobustSource>& partSource = partPl->GetSource();

  // Update and dump to file.
  partSource->Update();
  //
  vtkSmartPointer<vtkXMLPolyDataWriter>
    writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  //
  writer->SetFileName( argv[1] );
  writer->SetInputConnection( partSource->GetOutputPort() );
  writer->Write();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_GenerateFacets(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 1 && argc != 2 && argc != 3 && argc != 5 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Check if facet quality level is defined.
  asiAlgo_FacetQuality fq = asiAlgo_FacetQuality::UNDEFINED;
  if ( interp->HasKeyword(argc, argv, FQ_Name_VeryRough) )
  {
    fq = asiAlgo_FacetQuality::VeryRough;
  }
  else if ( interp->HasKeyword(argc, argv, FQ_Name_Rough) )
  {
    fq = asiAlgo_FacetQuality::Rough;
  }
  else if ( interp->HasKeyword(argc, argv, FQ_Name_Normal) )
  {
    fq = asiAlgo_FacetQuality::Normal;
  }
  else if ( interp->HasKeyword(argc, argv, FQ_Name_Fine) )
  {
    fq = asiAlgo_FacetQuality::Fine;
  }
  else if ( interp->HasKeyword(argc, argv, FQ_Name_VeryFine) )
  {
    fq = asiAlgo_FacetQuality::VeryFine;
  }

  // If the quality level is not defined, let's try to find the deflection
  // values specified explicitly.
  double linDefl = 0., angDeflDeg = 0.;
  if ( fq == asiAlgo_FacetQuality::UNDEFINED )
  {
    interp->GetKeyValue<double>(argc, argv, "lin", linDefl);
    interp->GetKeyValue<double>(argc, argv, "ang", angDeflDeg);
  }

  if ( (fq != asiAlgo_FacetQuality::UNDEFINED) ||
       (linDefl < asiAlgo_LINDEFL_MIN) ||
       (angDeflDeg < asiAlgo_ANGDEFL_MIN) )
  {
    if ( fq == asiAlgo_FacetQuality::UNDEFINED )
      fq = asiAlgo_FacetQuality::Rough;

    SelectFaceterOptions(fq, linDefl, angDeflDeg, interp->GetProgress() );
  }
  if ( interp->GetProgress().IsCancelling() )
  {
    interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
    return TCL_OK;
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Chosen linear/angular deflection values: %1/%2"
                                                       << linDefl << angDeflDeg);

  // Generate facets.
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Generate facets");
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Processing active part ...");

  asiEngine_Part partApi(cmdEngine::model,
                         (cmdEngine::cf && cmdEngine::cf->ViewerPart) ? cmdEngine::cf->ViewerPart->PrsMgr() : nullptr);

  cmdEngine::model->OpenCommand();
  {
    partApi.GetPart()->SetLinearDeflection(linDefl);
    partApi.GetPart()->SetAngularDeflection(angDeflDeg);
  }
  cmdEngine::model->CommitCommand();

  if ( cmdEngine::cf && cmdEngine::cf->ViewerPart )
  {
    // There is already a facet generator inside.
    cmdEngine::cf->ViewerPart->PrsMgr()->Actualize(cmdEngine::model->GetPartNode());
  }
  else
  {
    // This is done for the CLI.
    TopoDS_Shape partShape = partApi.GetShape();

    asiAlgo_MeshInfo meshInfo;
    if ( !asiAlgo_MeshGen::DoNative(partShape, linDefl, angDeflDeg, meshInfo) )
    {
      interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Failed to generate facets for active part.");
      return TCL_ERROR;
    }
    if ( interp->GetProgress().IsCancelling() )
    {
      interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
      return TCL_OK;
    }
    interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);
  }
  //
  if ( cmdEngine::cf && cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate(); // To sync metadata.

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdEngine::Commands_Data(const Handle(asiTcl_Interp)&      interp,
                              const Handle(Standard_Transient)& cmdEngine_NotUsed(data))
{
  static const char* group = "cmdEngine";

  //-------------------------------------------------------------------------//
  interp->AddCommand("whatis",
    //
    "whatis <name>\n"
    "\t Explains what is <name>.",
    //
    __FILE__, group, ENGINE_WhatIs);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump",
    //
    "dump <name>\n"
    "\t Dumps variable <name> to the logger.",
    //
    __FILE__, group, ENGINE_Dump);

  //-------------------------------------------------------------------------//
  interp->AddCommand("save-as",
    //
    "save-as <filename>\n"
    "\t Saves model to file with the given name.",
    //
    __FILE__, group, ENGINE_SaveAs);

  //-------------------------------------------------------------------------//
  interp->AddCommand("load",
    //
    "load <filename>\n"
    "\t Loads model from file with the given name. The current project data\n"
    "\t will be lost.",
    //
    __FILE__, group, ENGINE_Load);

  //-------------------------------------------------------------------------//
  interp->AddCommand("undo",
    //
    "undo\n"
    "\t Undoes model changes.",
    //
    __FILE__, group, ENGINE_Undo);

  //-------------------------------------------------------------------------//
  interp->AddCommand("redo",
    //
    "redo\n"
    "\t Redoes model changes.",
    //
    __FILE__, group, ENGINE_Redo);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-as-var",
    //
    "set-as-var <varName>\n"
    "\t Copies part shape to a topological variable.",
    //
    __FILE__, group, ENGINE_SetAsVar);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-as-part",
    //
    "set-as-part [<varName> | -node <id> -param <id>]\n"
    "\t Sets the object with the given name as a part for analysis.\n"
    "\t The object is expected to exist as a topological item in\n"
    "\t the imperative plotter. Alternatively, you can specify a custom Node ID\n"
    "\t with its source Parameter ID storing the shape to transfer to the\n"
    "\t Part Node. The latter option is useful if your shape is not stored\n"
    "\t in the section of imperative plotter data (i.e., you use custom Node\n"
    "\t as storage).",
    //
    __FILE__, group, ENGINE_SetAsPart);


  //-------------------------------------------------------------------------//
  interp->AddCommand("set-as-triangulation",
    //
    "set-as-triangulation <varName>\n"
    "\t Sets the object with the given name as the active triangulation.",
    //
    __FILE__, group, ENGINE_SetAsTriangulation);

  //-------------------------------------------------------------------------//
  interp->AddCommand("get-id",
    //
    "get-id [<parentObjectName> / [<parentObjectName> / [...]]] <objectName> [-nosplit]\n"
    "\t Finds a data object with the given name and returns its persistent ID.\n"
    "\t If the object name is not unique, you may specify a list of parents\n"
    "\t to narrow down your request. The names of the parent objects are separated\n"
    "\t by direct slash. You should always specify a direct parent of an object.\n"
    "\t It is not allowed to leave intermediate parents unspecified. If the '-nosplit'\n"
    "\t keyword is used, the direct slashes will be treated as parts of the single\n"
    "\t object's name.",
    //
    __FILE__, group, ENGINE_GetId);

  //-------------------------------------------------------------------------//
  interp->AddCommand("get-name",
    //
    "get-name <objectId>\n"
    "\t Returns object name by its passed ID.",
    //
    __FILE__, group, ENGINE_GetName);

  //-------------------------------------------------------------------------//
  interp->AddCommand("clear",
    //
    "clear\n"
    "\t Cleans up project data.",
    //
    __FILE__, group, ENGINE_Clear);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump-project",
    //
    "dump-project\n"
    "\t Dumps the active project's OCAF contents as text into a specific UI dialog.",
    //
    __FILE__, group, ENGINE_DumpProject);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dfbrowse",
    //
    "dfbrowse\n"
    "\t Opens DF Browser to inspect the internals of the OCAF document\n"
    "\t for the active project.",
    //
    __FILE__, group, ENGINE_DFBrowse);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-face-color-aag",
    //
    "set-face-color-aag [-fid <id>] -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Sets color for the given face as AAG attribute.",
    //
    __FILE__, group, ENGINE_SetFaceColorAAG);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-face-color",
    //
    "set-face-color [-fid id] -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Sets color for the given face.",
    //
    __FILE__, group, ENGINE_SetFaceColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("get-face-color",
    //
    "get-face-color -fid <id>\n"
    "\t Gets color for the given face.",
    //
    __FILE__, group, ENGINE_GetFaceColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("check-face-color",
    //
    "check-face-color -fid <id> -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Checks color for the given face.",
    //
    __FILE__, group, ENGINE_CheckFaceColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-part-color",
    //
    "set-part-color -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Sets color for the part.",
    //
    __FILE__, group, ENGINE_SetPartColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("get-part-color",
    //
    "get-part-color\n"
    "\t Gets color for the part.",
    //
    __FILE__, group, ENGINE_GetPartColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("check-part-color",
    //
    "check-part-color -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Checks color for the part.",
    //
    __FILE__, group, ENGINE_CheckPartColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-topo-item-color",
    //
    "set-topo-item-color -name <name> -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Sets color for the given topoItem.",
    //
    __FILE__, group, ENGINE_SetTopoItemColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("get-topo-item-color",
    //
    "get-topo-item-color -name <name>\n"
    "\t Gets color for the given topoItem.",
    //
    __FILE__, group, ENGINE_GetTopoItemColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("check-topo-item-color",
    //
    "check-topo-item-color -name <name> -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Checks color for the given topoItem.",
    //
    __FILE__, group, ENGINE_CheckTopoItemColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("set-edge-color",
    //
    "set-edge-color [-eid id] -color rgb(<ured>, <ugreen>, <ublue>)\n"
    "\t Sets color for the given edge.",
    //
    __FILE__, group, ENGINE_SetEdgeColor);

  //-------------------------------------------------------------------------//
  interp->AddCommand("get-param-shape",
    //
    "get-param-shape <resName> <nodeId> <paramId>\n"
    "\t Returns shape from the Parameter <paramId> of the Data Node <nodeId>.",
    //
    __FILE__, group, ENGINE_GetParamShape);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump-execution-graph-dot",
    //
    "dump-execution-graph-dot <filename>\n"
    "\t Dumps Tree Function execution graph of the active project to a DOT file (Graphviz).\n"
    "\t Read more about Active Data framework to familiarize yourself with Tree Functions.",
    //
    __FILE__, group, ENGINE_DumpExecutionGraphDot);

  //-------------------------------------------------------------------------//
  interp->AddCommand("dump-vtp",
    //
    "dump-vtp <filename>\n"
    "\t Dumps the polygonal data of the active part to a file in the VTP format\n"
    "\t of VTK. The data to dump is taken out from the part's pipelines.",
    //
    __FILE__, group, ENGINE_DumpVTP);

  //-------------------------------------------------------------------------//
  interp->AddCommand("generate-facets",
    //
    "generate-facets {[-lin <val>] [-ang <deg>] | [-very-rough | -rough | -normal | -fine | -very-fine]}\n"
    "\n"
    "\t Generates visualization facets for the active part.\n"
    "\t You can specify the linear and angular deflection values using the '-lin'\n"
    "\t and '-ang' keywords. The linear deflection is specified in the model units.\n"
    "\t The angular deflection is specified in degrees.\n"
    "\n"
    "\t Alternatively, you can pass one of the predefined quality levels: '-very-rough',\n"
    "\t '-rough', etc. In such a case, the algorithm will select the linear and angular\n"
    "\t deflections automatically.",
    //
    __FILE__, group, ENGINE_GenerateFacets);
}
