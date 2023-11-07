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
#include <asiAlgo_STEP.h>
#include <asiAlgo_Timer.h>

// asiAsm includes
#include <asiAsm_SceneTree.h>
#include <asiAsm_XdeDocIterator.h>

// asiEngine includes
#include <asiEngine_Part.h>

// glTF includes
#include <asiAsm_GLTFWriter.h>
#include <asiAsm_GLTFXdeDataSourceProvider.h>

// FBX includes
#include <fbx_XdeReader.h>
#include <fbx_XdeWriter.h>

// asiUI includes
#include <asiUI_DialogDump.h>
#include <asiUI_DialogXdeSummary.h>
#include <asiUI_XdeBrowser.h>

// DF Browser includes
#include <DFBrowser.hxx>

// OpenCascade includes
#include <BRep_Builder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <OSD_OpenFile.hxx>
#include <TopExp_Explorer.hxx>
#include <UnitsMethods.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QDialog>
#include <QDir>
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

int ASMXDE_ResetColors(const Handle(asiTcl_Interp)& interp,
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

  TIMER_NEW
  TIMER_GO

  // Clean up colors.
  xdeDoc->ResetColors();

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-reset-colors")

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

  // If the collection of items is empty and '-items' keyword hasn't been passed,
  // then all parts of the model will be gathered.
  PartIds pids;
  //
  if ( !items.IsEmpty() || (itemsIdx == -1) )
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

  glTFWriter cafWriter( filename,
                        ext.EndsWith(".glb"),
                        interp->GetProgress(),
                        interp->GetPlotter() );
  //
  cafWriter.SetTransformationFormat(glTFWriterTrsfFormat_TRS);
  cafWriter.SetForcedUVExport(false);
  //
  //const double systemUnitFactor = UnitsMethods::GetCasCadeLengthUnit() * 0.001;
  //cafWriter.ChangeCoordinateSystemConverter().SetInputLengthUnit(systemUnitFactor);
  cafWriter.ChangeCoordinateSystemConverter().SetInputCoordinateSystem(glTFCoordinateSystem_Zup);

  Handle(glTFXdeDataSourceProvider) dataProvider = new glTFXdeDataSourceProvider(doc->GetDocument());
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

int ASMXDE_LoadFBX(const Handle(asiTcl_Interp)& interp,
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

  fbxReader cafReader( filename.c_str(),
                       interp->GetProgress(),
                       interp->GetPlotter() );

  if ( !cafReader.Perform(doc) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "FBX load failed.");
    return TCL_ERROR;
  }
  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-load-fbx")

  // Set as variable.
  interp->SetVar( name, new cmdAsm_XdeModel(doc) );

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

int ASMXDE_SaveSTEP(const Handle(asiTcl_Interp)& interp,
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

  // Get optional part.
  PersistentId partId;
  //
  if ( interp->GetKeyValue(argc, argv, "part", partId) )
  {
    TopoDS_Shape partShape = doc->GetShape( PartId(partId) );

    // Write with a plain STEP translator.
    asiAlgo_STEP writer( interp->GetProgress() );
    //
    if ( !writer.Write(partShape, filename) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save part '%1' to '%2'."
                                                          << partId << filename);
      return TCL_ERROR;
    }
  }
  else
  {
    if ( !doc->SaveSTEP(filename) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "STEP export failed.");
      return TCL_ERROR;
    }
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-save-step")

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

int ASMXDE_Transform(const Handle(asiTcl_Interp)& interp,
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

  // Get transformation coefficients.
  std::vector<double> coeffs;
  int tIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "t", tIdx) )
  {
    for ( int ii = tIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      coeffs.push_back( Atof(argv[ii]) );
    }
  }
  //
  if ( coeffs.size() != 6 )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "You are supposed to pass 6 values for transformation: "
                                                        << "<x> <y> <z> <a> <b> <c>. Check help for details.");
    return TCL_ERROR;
  }

  // Apply transformation and update compounds (hell, yes, compounds).
  xdeDoc->TransformItem(aiid,
                        coeffs[0], coeffs[1], coeffs[2],
                        coeffs[3], coeffs[4], coeffs[5],
                        true);
  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_DumpJson(const Handle(asiTcl_Interp)& interp,
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

  // Get model name.
  bool doDumpShapes = interp->HasKeyword(argc, argv, "shapes");

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

  // Construct scene tree.
  asiAsm_SceneTree SceneTree;
  //
  SceneTree.Build(xdeDoc, doDumpShapes);

  // Dump to JSON.
  std::stringstream sstream;
  asiAsm_SceneTree::ToJSON(SceneTree, 0, true, sstream);
  //
  asiUI_DialogDump* pDumpDlg = new asiUI_DialogDump( "Scene Tree" );
  pDumpDlg->Populate( sstream.str() );
  pDumpDlg->show();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_DisplayJson(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  if (argc != 2)
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get file name containig scene tree description.
  std::string filename = argv[1];
  //
  if (filename.empty())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot find JSON with the name '%1'." << argv[1]);
    return TCL_ERROR;
  }

  // Load json file.
  std::ifstream stream;
  OSD_OpenStream(stream, filename.c_str(), std::ios::in);
  //
  if (!stream.is_open() || stream.fail())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot open file '%1'." << filename);
    return TCL_ERROR;
  }

  // Construct scene tree.
  asiAsm_SceneTree sceneTree;
  asiAsm_SceneTree::FromJSON(stream, sceneTree);

  // Visualize the scene tree content in 3D viewer.
  sceneTree.Dislay(interp->GetPlotter());

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_Unload(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  // Read model.
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

  // Read path.
  t_asciiString path;
  //
  if ( !interp->GetKeyValue(argc, argv, "path", path) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Output path is not specified.");
    return TCL_ERROR;
  }

  // Check the output directory.
  QDir qPath( path.ToCString() );
  //
  if ( !qPath.exists() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "The specified output directory '%1' does not exist."
                                                        << path);
    return TCL_ERROR;
  }

  // BOM filename.
  std::string bomFilename = asiAlgo_Utils::Str::Slashed( path.ToCString() );
  bomFilename += "bom.csv";

  // Filenames used for unique parts.
  NCollection_DataMap<PartId, std::string, PartId::Hasher> partFilenames;

  // Get unique parts.
  PartIds pids;
  xdeDoc->GetParts(pids);

  // Count parts.
  NCollection_DataMap<PartId, int, PartId::Hasher> partQuantities;
  xdeDoc->CountParts(partQuantities);

  // Create file for BOM output.
  std::ofstream bomFile;
  bomFile.open(bomFilename, std::ios::out | std::ios::trunc);

  // Iterate over the unique parts.
  for ( PartIds::Iterator pit(pids); pit.More(); pit.Next() )
  {
    int nextUniqueId = 1;

    // Next part.
    const PartId& pid       = pit.Value();
    TopoDS_Shape  partShape = xdeDoc->GetShape(pid);
    t_extString   partName  = xdeDoc->GetPartName(pid);

    // Remove unacceptable characters.
    partName.RemoveAll( '<' );
    partName.RemoveAll( '>' );

    // Prepare a filename.
    std::string filename = asiAlgo_Utils::Str::Slashed( path.ToCString() );
    filename += ExtStr2StdStr(partName);
    filename += ".stp";

    // Make sure that such a file does not exist yet.
    while ( QFile::exists( filename.c_str() ) )
    {
      interp->GetProgress().SendLogMessage(LogWarn(Normal) << "Filename '%1' has been already used."
                                                            << filename);

      // Recompose the filename with unique index.
      filename = asiAlgo_Utils::Str::Slashed( path.ToCString() );
      filename += ExtStr2StdStr(partName);
      filename += asiAlgo_Utils::Str::ToString(nextUniqueId++);
      filename += ".stp";
    }

    // Write STEP file.
    asiAlgo_STEP stepWriter( interp->GetProgress() );
    //
    if ( !stepWriter.Write( partShape, filename.c_str() ) )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot export part %1 to '%2'."
                                                          << pid << filename);
      continue;
    }

    // Keep track of filenames.
    partFilenames.Bind( pid, asiAlgo_Utils::Str::BaseFilename(filename, true) );

    // Add to the BOM file.
    bomFile << pid.ToString() << ", " << partFilenames(pid) << ", " << partQuantities(pid) << "\n";
  }

  bomFile.close();

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
    "\t Opens DF Browser to inspect the internals of the OCAF document\n"
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
    "\t items <item_1>, ... <item_k> (if passed), or all leaves (if\n"
    "\t not passed).",
    //
    __FILE__, group, ASMXDE_XCompounds);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-reset-colors",
    //
    "asm-xde-reset-colors -model <M>\n"
    "\t Cleans up all colors in the model with the name <M>.",
    //
    __FILE__, group, ASMXDE_ResetColors);

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
  interp->AddCommand("asm-xde-load-fbx",
    //
    "asm-xde-load-fbx -model <M> -filename <filename>\n"
    "\t Loads FBX file to fill XDE model.",
    //
    __FILE__, group, ASMXDE_LoadFBX);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-save-fbx",
    //
    "asm-xde-save-fbx -model <M> -filename <filename>\n"
    "\t Exports the passed XDE model to Autodesk FBX format.",
    //
    __FILE__, group, ASMXDE_SaveFBX);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-save-step",
    //
    "asm-xde-save-step -model <M> [-part <id>] -filename <filename>\n"
    "\t Exports the passed XDE model or its individual part to a STEP file.",
    //
    __FILE__, group, ASMXDE_SaveSTEP);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-generate-facets",
    //
    "asm-xde-generate-facets -model <M> [{-parts <id> ... | -items <id> ...}] "
    " {[-lin <val>] [-ang <deg>] | [-very-rough | -rough | -normal | -fine | -very-fine]}\n"
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

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-transform",
    //
    "asm-xde-transform -model <M> -item <id> [-t <x> <y> <z> <a> <b> <c>]\n"
    "\t Applies transformation to the given assembly item. The angles <a>, <b> and <c>\n"
    "\t are specified in degrees.",
    //
    __FILE__, group, ASMXDE_Transform);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-dump-json",
    //
    "asm-xde-dump-json -model <M> [-shapes]\n"
    "\t Dumps the passed model to JSON as a scene tree. If the '-shapes' flag\n"
    "\t is passed, the B-rep geometry of unique parts will be added as base64-encoded\n"
    "\t BLOBs in the extra 'shape' properties. This would make the dumped JSON file\n"
    "\t self-contained.",
    //
    __FILE__, group, ASMXDE_DumpJson);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-display-json",
    //
    "asm-xde-display-json <filename>\n"
    "\t Displays shapes out of JSON representing a scene tree.",
    //
    __FILE__, group, ASMXDE_DisplayJson);

  //-------------------------------------------------------------------------//
  interp->AddCommand("asm-xde-unload",
    //
    "asm-xde-unload -model <M> -path <dir>\n"
    "\n"
    "\t Unloads all unique parts from the model <M> to the directory specified\n"
    "\t with the '-path' keyword. Together with a plain list of parts, a BOM\n"
    "\t file (bom.csv) is generated in the CSV format to indicate how many occurrences\n"
    "\t each extracted part has got."
    "\n"
    "\t The default format for the exported part files is STEP.",
    //
    __FILE__, group, ASMXDE_Unload);
}
