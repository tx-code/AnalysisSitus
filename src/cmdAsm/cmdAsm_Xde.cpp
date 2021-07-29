//-----------------------------------------------------------------------------
// Created on: 18 December 2020
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

// cmdAsm includes
#include <cmdAsm_XdeModel.h>

// asiAlgo includes
#include <asiAlgo_FacetQuality.h>
#include <asiAlgo_FindVisibleFaces.h>
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_Timer.h>

// asiAsm includes
#include <asiAsm_XdeDocIterator.h>

// asiEngine includes
#include <asiEngine_Part.h>

// glTF includes
#include <gltf_Writer.h>
#include <gltf_XdeDataSourceProvider.h>

// FBX includes
#include <fbx_XdeWriter.h>

// asiUI includes
#include <asiUI_DialogXdeSummary.h>
#include <asiUI_XdeBrowser.h>

// DF Browser includes
#include <DFBrowser.hxx>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopExp_Explorer.hxx>
#include <UnitsMethods.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QMainWindow>
#pragma warning(pop)

using namespace asiAsm;
using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

void SelectFaceterOptions(const Handle(Doc)&         model,
                          const PartIds&             parts,
                          const asiAlgo_FacetQuality quality,
                          double&                    linDefl,
                          double&                    angDeflDeg,
                          ActAPI_ProgressEntry       progress)
{
  linDefl    = asiAlgo_LINDEFL_MIN;
  angDeflDeg = asiAlgo_ANGDEFL_MIN;

  // Initialize progress indicator.
  progress.Init( parts.Length() );
  progress.SetMessageKey("Select faceter options");

  // Select deflections for each part and then take the max values.
  for ( PartIds::Iterator pit(parts); pit.More(); pit.Next() )
  {
    const PartId& pid = pit.Value();

    double partLinDefl = asiAlgo_LINDEFL_MIN;
    double partAngDefl = asiAlgo_ANGDEFL_MIN;
    double partMinLinDefl;

    // Get shape for the part.
    TopoDS_Shape partShape = model->GetShape(pid);

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

    progress.StepProgress(1);
  }

  // Progress indication.
  progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);
}

//-----------------------------------------------------------------------------

int ASMXDE_DFBrowse(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  DFBrowser::DFBrowserCall( doc->GetDocument() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_New(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  TIMER_NEW
  TIMER_GO

  // Create a new empty XDE document.
  Handle(Doc) doc = new Doc( interp->GetProgress(),
                             interp->GetPlotter() );

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-new")

  // Set as variable.
  interp->SetVar( name, new cmdAsm_XdeModel(doc) );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_Load(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not specified. "
                                                           "Did you forget the '-filename' key?");
    return TCL_ERROR;
  }

  // Create a new empty XDE document.
  Handle(Doc) doc = new Doc( interp->GetProgress(),
                             interp->GetPlotter() );

  TIMER_NEW
  TIMER_GO

  // Load data from file.
  if ( !doc->Load( filename.c_str() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot load XDE document from file '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-load")

  // Set as variable.
  interp->SetVar( name, new cmdAsm_XdeModel(doc) );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_Save(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get filename.
  std::string filename;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filename) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not specified. "
                                                           "Did you forget the '-filename' key?");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  TIMER_NEW
  TIMER_GO

  // Save document to file.
  if ( !doc->SaveAs( filename.c_str() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save XDE document to file '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-save")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_Release(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  TIMER_NEW
  TIMER_GO

  // Close the document.
  doc->Release();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-release")

  // Unset the corresponding Tcl variable.
  interp->UnSetVar(name);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_Browse(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  if ( cmdAsm::cf.IsNull() )
    return TCL_OK; // Contract check: this function is UI-mode only.

  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);

  // Get optional title.
  std::string title;
  //
  interp->GetKeyValue(argc, argv, "title", title);

  TIMER_NEW
  TIMER_GO

  // Prepare browser.
  asiUI_XdeBrowser*
    pBrowser = new asiUI_XdeBrowser( xdeModel->GetDocument(),
                                     cmdAsm::cf,
                                     nullptr );
  //
  pBrowser->Populate();

  // Open UI dialog.
  QWidget* pDlg = new QDialog(cmdAsm::cf->MainWindow);
  //
  pDlg->setWindowTitle( title.empty() ? "XDE Browser" : title.c_str() );
  //
  QVBoxLayout* pDlgLayout = new QVBoxLayout;
  pDlgLayout->setAlignment(Qt::AlignTop);
  pDlgLayout->setContentsMargins(10, 10, 10, 10);
  //
  pDlgLayout->addWidget(pBrowser);
  pDlg->setLayout(pDlgLayout);
  //
  pDlg->show();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-browse")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_XCompounds(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(Doc) xdeDoc = Handle(cmdAsm_XdeModel)::DownCast(var)->GetDocument();

  // Get items.
  AssemblyItemIds items, leaves;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( AssemblyItemId(argv[ii]) );
    }

    xdeDoc->GetLeafAssemblyItems(items, leaves);
  }
  else
  {
    xdeDoc->GetLeafAssemblyItems(leaves);
  }

  TIMER_NEW
  TIMER_GO

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. items to expand: %1."
                                                        << items.Length() );

  // Expand compounds.
  xdeDoc->ExpandCompounds(leaves);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-xcompounds")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_PrintStructure(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(Doc) xdeDoc = Handle(cmdAsm_XdeModel)::DownCast(var)->GetDocument();

  // Read the level of hierarchy.
  int level = INT_MAX;
  //
  interp->GetKeyValue<int>(argc, argv, "level", level);

  TIMER_NEW
  TIMER_GO

  // Use assembly iterator to traverse structure.
  for ( DocIterator it(xdeDoc, level); it.More(); it.Next() )
  {
    AssemblyItemId id = it.Current();
    //
    interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Next item: %1." << id.ToString() );
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-print-structure")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_ShowSummary(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if ( cmdAsm::cf.IsNull() )
    return TCL_OK; // Contract check: this function is UI-mode only.

  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);

  TIMER_NEW
  TIMER_GO

  // Open UI widget for consulting the summary info.
  asiUI_DialogXdeSummary*
    pSummaryDlg = new asiUI_DialogXdeSummary(xdeModel->GetDocument(), cmdAsm::cf->Progress);
  //
  pSummaryDlg->show();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-show-summary")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_GetParts(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             xdeDoc   = xdeModel->GetDocument();

  // Get items (if any).
  AssemblyItemIds items;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( AssemblyItemId(argv[ii]) );
    }
  }

  TIMER_NEW
  TIMER_GO

  // If the collection of items is empty, all parts of the model will
  // be gathered.
  PartIds pids;
  //
  xdeDoc->GetParts(items, pids);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-get-parts")

  // Add part IDs to the interpreter.
  int pid = 1;
  //
  for ( PartIds::Iterator pit(pids); pit.More(); pit.Next(), ++pid )
  {
    *interp << pit.Value().Entry.ToCString();

    if ( pid < pids.Length() )
      *interp << " ";
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_GetLeaves(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             xdeDoc   = xdeModel->GetDocument();

  // Get items (if any).
  AssemblyItemIds items;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( AssemblyItemId(argv[ii]) );
    }
  }

  TIMER_NEW
  TIMER_GO

  // If the collection of items is empty, all leaves of the model will
  // be gathered.
  AssemblyItemIds leaves;
  //
  xdeDoc->GetLeafAssemblyItems(items, leaves);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-get-leaves")

  // Add items IDs to the interpreter.
  int aiid = 1;
  //
  for ( AssemblyItemIds::Iterator aiit(leaves);
        aiit.More();
        aiit.Next(), ++aiid )
  {
    *interp << aiit.Value().ToString();

    if ( aiid < leaves.Length() )
      *interp << " ";
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_FindItems(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  // Get model name.
  std::string modelName;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", modelName) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(modelName);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << modelName);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             xdeDoc   = xdeModel->GetDocument();

  // Get item name.
  std::string itemName;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", itemName) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Assembly item's name is not provided.");
    return TCL_ERROR;
  }

  // Find unique items.
  Handle(HAssemblyItemIdsMap) items;
  //
  xdeDoc->FindItems(itemName, items);

  // Add items IDs to the interpreter.
  int aiid = 1;
  //
  for ( HAssemblyItemIdsMap::Iterator aiit(*items); aiit.More(); aiit.Next(), ++aiid )
  {
    *interp << aiit.Value().ToString();

    if ( aiid < items->Extent() )
      *interp << " ";
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_AddPart(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  // Get Part Node.
  Handle(asiData_PartNode) partNode = cmdAsm::model->GetPartNode();
  //
  if ( partNode.IsNull() || !partNode->IsWellFormed() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part Node is null or ill-defined.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape shape = partNode->GetShape();

  // Get model name.
  std::string modelName;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", modelName) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(modelName);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << modelName);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             xdeDoc   = xdeModel->GetDocument();

  // Get part name.
  std::string partName;
  //
  interp->GetKeyValue(argc, argv, "name", partName);

  TIMER_NEW
  TIMER_GO

  // Add part.
  PartId pid = xdeDoc->AddPart(shape, partName);

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Part was added with id %1."
                                                       << pid);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-add-part")

  *interp << pid;

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_SaveGLTF(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get filename.
  std::string filenameArg;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filenameArg) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not specified. "
                                                           "Did you forget the '-filename' key?");
    return TCL_ERROR;
  }
  //
  TCollection_AsciiString filename( filenameArg.c_str() );
  TCollection_AsciiString ext = filename;
  ext.LowerCase();

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  TIMER_NEW
  TIMER_GO

  gltf_Writer cafWriter( filename,
                        ext.EndsWith(".glb"),
                        interp->GetProgress(),
                        interp->GetPlotter() );
  //
  cafWriter.SetTransformationFormat(gltf_WriterTrsfFormat_TRS);
  cafWriter.SetForcedUVExport(false);
  //
  //const double systemUnitFactor = UnitsMethods::GetCasCadeLengthUnit() * 0.001;
  //cafWriter.ChangeCoordinateSystemConverter().SetInputLengthUnit(systemUnitFactor);
  cafWriter.ChangeCoordinateSystemConverter().SetInputCoordinateSystem(gltf_CoordinateSystem_Zup);

  Handle(gltf_XdeDataSourceProvider) dataProvider = new gltf_XdeDataSourceProvider(doc->GetDocument());
  if ( !cafWriter.Perform(dataProvider) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "glTF export failed.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-save-gltf")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_SaveFBX(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get filename.
  std::string filenameArg;
  //
  if ( !interp->GetKeyValue(argc, argv, "filename", filenameArg) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not specified. "
                                                           "Did you forget the '-filename' key?");
    return TCL_ERROR;
  }
  //
  TCollection_AsciiString filename( filenameArg.c_str() );
  TCollection_AsciiString ext = filename;
  ext.LowerCase();

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  TIMER_NEW
  TIMER_GO

  fbxWriter cafWriter( filename,
                       interp->GetProgress(),
                       interp->GetPlotter() );

  if ( !cafWriter.Perform(doc) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "FBX export failed.");
    return TCL_ERROR;
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-save-fbx")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_GenerateFacets(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  // Get items (if any).
  AssemblyItemIds items;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( AssemblyItemId(argv[ii]) );
    }
  }

  // Get parts (if any).
  PartIds parts;
  int partsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "parts", partsIdx) )
  {
    for ( int ii = partsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      parts.Append( PartId(argv[ii]) );
    }
  }

  if ( !items.IsEmpty() )
  {
    AssemblyItemIds leaves;
    doc->GetParts(items, leaves, parts);
  }

  // If we still have no parts, let's take just them all.
  if ( parts.IsEmpty() )
  {
    doc->GetParts(parts);
  }

  // Check if facet quality level is defined.
  asiAlgo_FacetQuality fq = asiAlgo_FacetQuality::UNDEFINED;
  //
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
  //
  if ( fq == asiAlgo_FacetQuality::UNDEFINED )
  {
    interp->GetKeyValue<double>(argc, argv, "lin", linDefl);
    interp->GetKeyValue<double>(argc, argv, "ang", angDeflDeg);
  }

  if ( !cmdAsm::cf.IsNull() )
    cmdAsm::cf->ProgressListener->SetProcessEvents(true);

  if ( (fq != asiAlgo_FacetQuality::UNDEFINED) ||
       (linDefl < asiAlgo_LINDEFL_MIN) ||
       (angDeflDeg < asiAlgo_ANGDEFL_MIN) )
  {
    if ( fq == asiAlgo_FacetQuality::UNDEFINED )
      fq = asiAlgo_FacetQuality::Rough;

    SelectFaceterOptions( doc, parts, fq, linDefl, angDeflDeg, interp->GetProgress() );
  }

  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Chosen linear/angular deflection values: %1/%2"
                                                       << linDefl << angDeflDeg);

  // Initialize progress indicator.
  interp->GetProgress().Init( parts.Length() );
  interp->GetProgress().SetMessageKey("Generate visualization facets");

  TIMER_NEW
  TIMER_GO

  for ( PartIds::Iterator pit(parts); pit.More(); pit.Next() )
  {
    // Get part.
    const PartId& pid       = pit.Value();
    TopoDS_Shape  partShape = doc->GetShape(pid);

    // Get name.
    TCollection_ExtendedString partName;
    doc->GetObjectName(pid, partName);

    interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Processing part %1 ('%2')..."
                                                         << pid << partName);

    asiAlgo_MeshInfo meshInfo;
    //
    if ( !asiAlgo_MeshGen::DoNative(partShape, linDefl, angDeflDeg, meshInfo) )
    {
      interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Failed to generate facets for the part %1."
                                                           << pid);
    }

    if ( interp->GetProgress().IsCancelling() )
    {
      interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);

      if ( !cmdAsm::cf.IsNull() )
        cmdAsm::cf->ProgressListener->SetProcessEvents(false);

      return TCL_OK;
    }

    interp->GetProgress().StepProgress(1);
  }

  // Progress indication.
  interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);

  if ( !cmdAsm::cf.IsNull() )
    cmdAsm::cf->ProgressListener->SetProcessEvents(false);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-generate-facets")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_SetAsVar(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  // Get the item in question.
  std::string itemIdStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "item", itemIdStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Assembly item is not defined.");
    return TCL_ERROR;
  }
  //
  AssemblyItemId aiid( itemIdStr.c_str() );

  // Get variable name.
  std::string varName;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", varName) )
    varName = aiid.ToString().ToCString();

  // Get color RGB components as unsigned integer values.
  TCollection_AsciiString colorStr;
  ActAPI_Color            color;
  //
  if ( interp->GetKeyValue(argc, argv, "color", colorStr) )
  {
    // Get color components.
    std::vector<unsigned int> colorComponents;
    std::vector<std::string>  colorComponentsStr;
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

    color = ActAPI_Color(colorComponents[0]/255.,
                         colorComponents[1]/255.,
                         colorComponents[2]/255.,
                         Quantity_TOC_RGB);
  }
  else
  {
    color = Color_Default;
  }

  TIMER_NEW
  TIMER_GO

  interp->GetPlotter().REDRAW_SHAPE(varName.c_str(), doc->GetShape(aiid), color);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-set-as-var")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

TopoDS_Shape BuildToolPrism(const Handle(asiAlgo_AAG)& aag,
                            const int                  fidFrom,
                            ActAPI_ProgressEntry       progress)
{
  const TopoDS_Face& faceFrom = aag->GetFace(fidFrom);
  //
  if ( !asiAlgo_Utils::IsPlanar(faceFrom) )
    return TopoDS_Shape();

  int    fidTo = 0;
  double dist  = 0.;
  gp_Vec norm;

  // Measure the distance between 'from' and 'to'.
  if ( !asiEngine_Part::ComputeMateFace<Geom_Plane>(aag,
                                                    fidFrom,
                                                    asiAlgo_Feature(),
                                                    3,
                                                    false,
                                                    fidTo,
                                                    dist,
                                                    norm) )
  {
    progress.SendLogMessage(LogErr(Normal) << "Cannot find a mate face for the face %1."
                                           << fidFrom);
    return TopoDS_Shape();
  }

  if ( norm.Magnitude() < Precision::Confusion() || Abs(dist) < Precision::Confusion() )
    return TopoDS_Shape();

  // Make a tool object.
  TopoDS_Shape result;
  try
  {
    BRepPrimAPI_MakePrism mkPrism(faceFrom,
                                  norm.Normalized()*dist,
                                  true);
    //
    result = mkPrism.Shape();
  }
  catch ( ... )
  {
    progress.SendLogMessage(LogErr(Normal) << "Failed to build prism from face %1."
                                           << fidFrom);
    return TopoDS_Shape();
  }

  return result;
}

int ASMXDE_KEA(const Handle(asiTcl_Interp)& interp,
               int                          argc,
               const char**                 argv)
{
  // Get model name.
  std::string modelName;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", modelName) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(modelName);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << modelName);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             xdeDoc   = xdeModel->GetDocument();

  TIMER_NEW
  TIMER_GO

  /* ======================
   *  Find invisible faces.
   * ====================== */

  // Prepare visibility checker. We initialize it with the entire model
  // to check for collisions with every part of an assembly.
  TopoDS_Shape asmShape = xdeDoc->GetOneShape();
  //
  asiAlgo_FindVisibleFaces FindVisible( asmShape,
                                        interp->GetProgress(),
                                        interp->GetPlotter() );

  Handle(asiAlgo_AAG) asmAag = new asiAlgo_AAG(asmShape);

  // Map faces to be able to derive subdomains.
  TopTools_IndexedMapOfShape allFaces = asmAag->GetMapOfFaces();

  // Get all leaves of the model.
  AssemblyItemIds leaves;
  xdeDoc->GetLeafAssemblyItems(leaves);

  TopoDS_Compound allTools;
  BRep_Builder().MakeCompound(allTools);

  // Tool prisms grouped by their originating parts.
  NCollection_DataMap<PartId,
                      TopoDS_Compound,
                      PartId::Hasher> partTools;

  // AAGs of the part shapes.
  NCollection_DataMap<PartId,
                      Handle(asiAlgo_AAG),
                      PartId::Hasher> partAAGs;

  // Iterate over all parts of the model.
  for ( AssemblyItemIds::Iterator aiit(leaves); aiit.More(); aiit.Next() )
  {
    // Get item.
    const AssemblyItemId& aiid = aiit.Value();

    // Get part.
    PartId pid = xdeDoc->GetPart(aiid);

    // Get shape.
    TopoDS_Shape itemShape = xdeDoc->GetShape(aiid);

    // Part shape.
    TopoDS_Shape partShape = xdeDoc->GetShape(pid);

    // Check if AAG for a part is available.
    Handle(asiAlgo_AAG) partAAG;
    //
    if ( partAAGs.IsBound(pid) )
    {
      partAAG = partAAGs(pid);
    }
    else
    {
      partAAG = new asiAlgo_AAG(partShape, true);
      partAAGs.Bind(pid, partAAG);
    }

    // Get all faces of the item shape to define its subdomain.
    TopTools_IndexedMapOfShape itemFaces;
    TopExp::MapShapes(itemShape, TopAbs_FACE, itemFaces);

    // Define subdomain for analysis.
    asiAlgo_Feature itemSubdomain;
    //
    for ( int kk = 1; kk <= itemFaces.Extent(); ++kk )
    {
      const int fid = allFaces.FindIndex( itemFaces(kk) );
      itemSubdomain.Add(fid);
    }
    //
    if ( itemSubdomain.IsEmpty() )
    {
      interp->GetProgress().SendLogMessage( LogWarn(Normal) << "The item %1 does not have subdomain faces."
                                                            << aiid.ToString() );
      continue;
    }

    // Find visible faces.
    FindVisible.SetSubdomain(itemSubdomain);
    //
    if ( !FindVisible.Perform() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find visible faces.");
      return TCL_ERROR;
    }

    // Get visible faces.
    asiAlgo_Feature visibleIndices;
    FindVisible.GetResultFaces(visibleIndices);

    // Invert to get the invisible faces.
    asiAlgo_Feature resIndices = itemSubdomain;
    resIndices.Subtract(visibleIndices);

    TopoDS_Compound comp, tools;
    BRep_Builder().MakeCompound(comp);
    BRep_Builder().MakeCompound(tools);
    //
    for ( asiAlgo_Feature::Iterator rit(resIndices); rit.More(); rit.Next() )
    {
      const int   fid      = rit.Key();
      TopoDS_Face faceFrom = TopoDS::Face( allFaces(fid).Located( TopLoc_Location() ) );

      BRep_Builder().Add(comp, faceFrom);

      TopoDS_Shape prism = BuildToolPrism( partAAG,
                                           partAAG->GetFaceId(faceFrom),
                                           interp->GetProgress() ).Located( itemShape.Location() );
      //
      if ( !prism.IsNull() )
        BRep_Builder().Add( tools, prism );
    }

    // Add to the result.
    TopoDS_Compound* pPartToolsComp = partTools.ChangeSeek(pid);
    //
    if ( pPartToolsComp == nullptr )
    {
      partTools.Bind(pid, tools);
    }
    else
    {
      pPartToolsComp->TShape()->Free(true);

      for ( TopExp_Explorer solExp(tools, TopAbs_SOLID); solExp.More(); solExp.Next() )
      {
        BRep_Builder().Add( *pPartToolsComp, solExp.Current() );
      }
    }

    interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Found %1 invisible out of %2 subdomain faces."
                                                          << resIndices.Extent() << itemSubdomain.Extent() );

    //TCollection_AsciiString groupName("invisible "), toolName("tools ");
    //groupName += aiid.ToString();
    //toolName  += aiid.ToString();
    ////
    //interp->GetPlotter().REDRAW_SHAPE(groupName, comp,  Color_Red);
    //interp->GetPlotter().REDRAW_SHAPE(toolName,  tools, Color_White);

    BRep_Builder().Add(allTools, tools);
  }

  //interp->GetPlotter().REDRAW_SHAPE("allTools", allTools, Color_White);

  // Output the results.
  for ( NCollection_DataMap<PartId,
                            TopoDS_Compound,
                            PartId::Hasher>::Iterator resIt(partTools);
        resIt.More(); resIt.Next() )
  {
    TCollection_AsciiString groupName("tools_");
    groupName += resIt.Key().ToString();
    //
    interp->GetPlotter().REDRAW_SHAPE(groupName, resIt.Value(), Color_White);
  }

  /* ==========================================
   *  Classify tools: keep only invisible ones.
   * ========================================== */

  // Construct one shape with all tools and parts.
  TopoDS_Compound asmShapeWithTools;
  //
  BRep_Builder().MakeCompound(asmShapeWithTools);
  BRep_Builder().Add(asmShapeWithTools, asmShape);
  //
  for ( NCollection_DataMap<PartId,
                            TopoDS_Compound,
                            PartId::Hasher>::Iterator resIt(partTools);
        resIt.More(); resIt.Next() )
  {
    BRep_Builder().Add( asmShapeWithTools, resIt.Value() );
  }

  // Compose another map with invisible prisms only.
  NCollection_DataMap<PartId,
                      TopoDS_Compound,
                      PartId::Hasher> invisibleTools;

  asiAlgo_FindVisibleFaces FindVisible2( asmShapeWithTools,
                                         interp->GetProgress(),
                                         interp->GetPlotter() );

  Handle(asiAlgo_AAG) asmShapeWithToolsAag = new asiAlgo_AAG(asmShapeWithTools);

  // Map faces to be able to derive subdomains.
  allFaces = asmShapeWithToolsAag->GetMapOfFaces();

  for ( NCollection_DataMap<PartId,
                            TopoDS_Compound,
                            PartId::Hasher>::Iterator resIt(partTools);
        resIt.More(); resIt.Next() )
  {
    const PartId&       pid            = resIt.Key();
    const TopoDS_Shape& subdomainShape = resIt.Value();

    for ( TopExp_Explorer exp(subdomainShape, TopAbs_SOLID); exp.More(); exp.Next() )
    {
      const TopoDS_Shape& subdomainSolid = exp.Current();

      // Get all faces of the item shape to define its subdomain.
      TopTools_IndexedMapOfShape itemFaces;
      TopExp::MapShapes(subdomainSolid, TopAbs_FACE, itemFaces);

      // Define subdomain for analysis.
      asiAlgo_Feature itemSubdomain;
      //
      for ( int kk = 1; kk <= itemFaces.Extent(); ++kk )
      {
        const int fid = allFaces.FindIndex( itemFaces(kk) );
        itemSubdomain.Add(fid);
      }
      //
      if ( itemSubdomain.IsEmpty() )
      {
        continue;
      }

      // Find visible faces.
      FindVisible2.SetSubdomain(itemSubdomain);
      //
      if ( !FindVisible2.Perform() )
      {
        interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find visible faces.");
        return TCL_ERROR;
      }

      // Get visible faces.
      asiAlgo_Feature visibleIndices;
      FindVisible2.GetResultFaces(visibleIndices, 0.01);

      if ( visibleIndices.Extent() )
        continue;

      // Add to the result.
      TopoDS_Compound* pPartToolsComp = invisibleTools.ChangeSeek(pid);
      //
      if ( pPartToolsComp == nullptr )
      {
        TopoDS_Compound tools;
        BRep_Builder().MakeCompound(tools);
        BRep_Builder().Add(tools, subdomainSolid);
        //
        invisibleTools.Bind(pid, tools);
      }
      else
      {
        pPartToolsComp->TShape()->Free(true);

        BRep_Builder().Add( *pPartToolsComp, subdomainSolid );
      }
    }
  }

  // Output the results.
  for ( NCollection_DataMap<PartId,
                            TopoDS_Compound,
                            PartId::Hasher>::Iterator resIt(invisibleTools);
        resIt.More(); resIt.Next() )
  {
    TCollection_AsciiString groupName("invtools_");
    groupName += resIt.Key().ToString();
    //
    interp->GetPlotter().REDRAW_SHAPE(groupName, resIt.Value(), Color_Green);
  }

  /* ==============================================
   *  Fuse remaining tools with their owning parts.
   * ============================================== */

  for ( NCollection_DataMap<PartId,
                            TopoDS_Compound,
                            PartId::Hasher>::Iterator resIt(invisibleTools);
        resIt.More(); resIt.Next() )
  {
    TopTools_ListOfShape args;
    //
    for ( TopExp_Explorer exp(resIt.Value(), TopAbs_SOLID); exp.More(); exp.Next() )
    {
      args.Append( exp.Current().Located( TopLoc_Location() ) );
    }
    //
    args.Append( xdeDoc->GetShape( resIt.Key() ) );

    Handle(BRepTools_History) history;
    TopoDS_Shape res = asiAlgo_Utils::BooleanFuse(args, history);

    //TCollection_AsciiString groupName("res_");
    //groupName += resIt.Key().ToString();
    ////
    //interp->GetPlotter().REDRAW_SHAPE(groupName, res, Color_White);

    xdeDoc->UpdatePartShape( xdeDoc->GetLabel( resIt.Key() ), res, history, false );
  }

  xdeDoc->UpdateAssemblies();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-kea")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_SetName(const Handle(asiTcl_Interp)& interp,
                   int                          argc,
                   const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(cmdAsm_XdeModel) xdeModel = Handle(cmdAsm_XdeModel)::DownCast(var);
  Handle(Doc)             doc      = xdeModel->GetDocument();

  // Whether to set names for instances.
  const bool is4Instance = interp->HasKeyword(argc, argv, "instance");

  // Get the item in question.
  std::string itemIdStr;
  //
  if ( !interp->GetKeyValue(argc, argv, "item", itemIdStr) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Assembly item is not defined.");
    return TCL_ERROR;
  }
  //
  AssemblyItemId aiid( itemIdStr.c_str() );

  // Get new name.
  std::string itemName;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", itemName) )
    itemName = aiid.ToString().ToCString();

  TDF_Label targetLabel;
  //
  if ( is4Instance )
  {
    targetLabel = doc->GetLabel(aiid);
  }
  else
  {
    targetLabel = doc->GetOriginal(aiid);
  }

  doc->SetObjectName( targetLabel, itemName.c_str() );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_RemoveParts(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  // Get model name.
  std::string name;
  //
  if ( !interp->GetKeyValue(argc, argv, "model", name) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model name is not specified.");
    return TCL_ERROR;
  }

  // Get the XDE document.
  Handle(asiTcl_Variable) var = interp->GetVar(name);
  //
  if ( var.IsNull() || !var->IsKind( STANDARD_TYPE(cmdAsm_XdeModel) ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no XDE model named '%1'."
                                                        << name);
    return TCL_ERROR;
  }
  //
  Handle(Doc) xdeDoc = Handle(cmdAsm_XdeModel)::DownCast(var)->GetDocument();

  // Get items.
  AssemblyItemIds items, leaves;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( AssemblyItemId(argv[ii]) );
    }

    xdeDoc->GetLeafAssemblyItems(items, leaves);
  }
  else
  {
    xdeDoc->GetLeafAssemblyItems(leaves);
  }

  // Get parts to remove.
  PartIds partsPassed, parts2Remove;
  xdeDoc->GetParts(leaves, partsPassed, true);

  // Check whether the selection of parts to remove should be inverted.
  if ( interp->HasKeyword(argc, argv, "invert") )
  {
    // Get all parts in the model.
    PartIds allParts;
    xdeDoc->GetParts(allParts);

    // Keep only those parts that have not been passed.
    for ( PartIds::Iterator allPartsIt(allParts); allPartsIt.More(); allPartsIt.Next() )
    {
      const PartId& pid1 = allPartsIt.Value();

      // Check if the part is passed as an input.
      bool isPassed = false;
      for ( PartIds::Iterator passedPartsIt(partsPassed); passedPartsIt.More(); passedPartsIt.Next() )
      {
        const PartId& pid2 = passedPartsIt.Value();
        //
        if ( pid1.IsEqual(pid2) )
        {
          isPassed = true;
          break;
        }
      }

      if ( !isPassed )
        parts2Remove.Append(pid1);
    }
  }
  else
  {
    parts2Remove = partsPassed;
  }

  TIMER_NEW
  TIMER_GO

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "Num. parts to remove: %1."
                                                        << parts2Remove.Length() );

  xdeDoc->RemoveParts(parts2Remove, false);
  xdeDoc->RemoveAllEmptyAssemblies();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-remove-parts")

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdAsm::Commands_XDE(const Handle(asiTcl_Interp)&      interp,
                          const Handle(Standard_Transient)& cmdAsm_NotUsed(data))
{
  static const char* group = "cmdAsm";

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-dfbrowse",
    //
    "asm-xde-dfbrowse -model <M>\n"
    "\t Opens up a DF Browser to inspect the internals of the OCAF document\n"
    "\t for the model <M>.",
    //
    __FILE__, group, ASMXDE_DFBrowse);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-new",
    //
    "asm-xde-new -model <M>\n"
    "\t Creates a new XDE document named <M>.",
    //
    __FILE__, group, ASMXDE_New);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-load",
    //
    "asm-xde-load -model <M> -filename <filename>\n"
    "\t Loads assembly from file <filename> to the XDE document named <M>.",
    //
    __FILE__, group, ASMXDE_Load);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-save",
    //
    "asm-xde-save -model <M> -filename <filename>\n"
    "\t Saves the XDE document named <M> to the file <filename>.",
    //
    __FILE__, group, ASMXDE_Save);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-release",
    //
    "asm-xde-release -model <M>\n"
    "\t Releases memory consumed by the XDE document named <M>.",
    //
    __FILE__, group, ASMXDE_Release);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-browse",
    //
    "asm-xde-browse -model <M> [-title <title>]\n"
    "\t Opens a dialog to browse the structure of the XDE document named <M>.\n"
    "\t If the '-title' argument is passed, the browser gets the specified title\n"
    "\t in its window.",
    //
    __FILE__, group, ASMXDE_Browse);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-xcompounds",
    //
    "asm-xde-xcompounds -model <M> [-items <item_1> ... <item_k>]\n"
    "\t Expands the compound-type parts in the model <M> for the assembly\n"
    "\t items <item_1>, ... <item_k> (if passed), or for the all leaves (if\n"
    "\t not passed).",
    //
    __FILE__, group, ASMXDE_XCompounds);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-print-structure",
    //
    "asm-xde-print-structure -model <M> [-level <zero-based-level>]\n"
    "\t Prints assembly hierarchy for the passed model <M> down to the\n"
    "\t given hierarchical level <zero-based-level>.",
    //
    __FILE__, group, ASMXDE_PrintStructure);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-show-summary",
    //
    "asm-xde-show-summary -model <M>\n"
    "\t Shows a GUI dialog with assembly summary.",
    //
    __FILE__, group, ASMXDE_ShowSummary);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-get-parts",
    //
    "asm-xde-get-parts -model <M> [-items <item_1> ... <item_k>]\n"
    "\t Returns IDs of all parts of the model <M> in the case when no parent items\n"
    "\t are specified. If the item IDs are passed through the '-item' keyword, only\n"
    "\t the children parts of those items are returned.",
    //
    __FILE__, group, ASMXDE_GetParts);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-get-leaves",
    //
    "asm-xde-get-leaves -model <M> [-items <item_1> ... <item_k>]\n"
    "\t Returns leaf assembly items for the passed items.",
    //
    __FILE__, group, ASMXDE_GetLeaves);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-find-items",
    //
    "asm-xde-find-items -model <M> -name <name>\n"
    "\t Finds assembly items having the passed name.",
    //
    __FILE__, group, ASMXDE_FindItems);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-add-part",
    //
    "asm-xde-add-part -model <M> [-name <name>]\n"
    "\t Adds the active part to the XDE document as an assembly part.",
    //
    __FILE__, group, ASMXDE_AddPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-save-gltf",
    //
    "asm-xde-save-gltf -model <M> -filename <filename>\n"
    "\t Exports the passed XDE model to glTF format.",
    //
    __FILE__, group, ASMXDE_SaveGLTF);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-save-fbx",
    //
    "asm-xde-save-fbx -model <M> -filename <filename>\n"
    "\t Exports the passed XDE model to Autodesk FBX format.",
    //
    __FILE__, group, ASMXDE_SaveFBX);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-generate-facets",
    //
    "asm-xde-generate-facets -model <M> [{-parts <id> ... | -items <id> ...}]\n"
    "\t {[-lin <val>] [-ang <deg>] | [-very-rough | -rough | -normal | -fine | -very-fine]}\n"
    "\n"
    "\t Generates visualization facets for all parts in the passed XDE document.\n"
    "\t You can specify the linear and angular deflection values using the '-lin'\n"
    "\t and '-ang' keywords. The linear deflection is specified in the model units.\n"
    "\t The angular deflection is specified in degrees.\n"
    "\n"
    "\t Alternatively, you can pass one of the predefined quality levels: '-very-rough',\n"
    "\t '-rough', etc. In such a case, the algorithm will select the linear and angular\n"
    "\t deflections automatically.\n"
    "\n"
    "\t If neither assembly items nor parts are specified via the corresponding '-items' and\n"
    "\t '-parts' keywords, the visualization facets will be generated for all B-rep shapes\n"
    "\t available in the XDE document.",
    //
    __FILE__, group, ASMXDE_GenerateFacets);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-set-as-var",
    //
    "asm-xde-set-as-var -model <M> -item <id> [-name <varName>] [-color rgb(<ured>,<ugreen>,<ublue>)]\n"
    "\t Sets the passed assembly item as a project variable.",
    //
    __FILE__, group, ASMXDE_SetAsVar);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-kea",
    //
    "asm-xde-kea -model <M>\n"
    "\t Performs KEA test.",
    //
    __FILE__, group, ASMXDE_KEA);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-set-name",
    //
    "asm-xde-set-name -model <M> -item <id> -name <name> [-instance]\n"
    "\t Sets new name for the passed assembly item. If the '-instance' flag is\n"
    "\t specified, the name will be set for the instances of the given prototype.",
    //
    __FILE__, group, ASMXDE_SetName);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-remove-parts",
    //
    "asm-xde-remove-parts -model <M> [-items <item_1> ... <item_k>] [-invert]\n"
    "\t Removes parts corresponding to the passed assembly items with all\n"
    "\t their occurrences in the model. If the '-invert' flag is passed, the\n"
    "\t passed items along with all their children will remain in the model,\n"
    "\t while all other parts will be removed instead.",
    //
    __FILE__, group, ASMXDE_RemoveParts);
}
