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
#include <asiAlgo_Timer.h>

// asiAsm includes
#include <asiAsm_XdeDocIterator.h>

// asiUI includes
#include <asiUI_DialogXdeSummary.h>
#include <asiUI_XdeBrowser.h>

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
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not specified.");
    return TCL_ERROR;
  }

  // Create a new empty XDE document.
  Handle(asiAsm_XdeDoc) doc = new asiAsm_XdeDoc( interp->GetProgress(),
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
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Filename is not specified.");
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
  Handle(asiAsm_XdeDoc)   doc      = xdeModel->GetDocument();

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
  Handle(asiAsm_XdeDoc)   doc      = xdeModel->GetDocument();

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

  TIMER_NEW
  TIMER_GO

  // Open UI widget for browsing the assembly structure.
  asiUI_XdeBrowser* pBrowser = new asiUI_XdeBrowser(xdeModel->GetDocument(), cmdAsm::cf);
  //
  pBrowser->Populate();
  //
  pBrowser->show();

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
  Handle(asiAsm_XdeDoc) xdeDoc = Handle(cmdAsm_XdeModel)::DownCast(var)->GetDocument();

  // Get items.
  asiAsm_XdeAssemblyItemIds items;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( asiAsm_XdeAssemblyItemId(argv[ii]) );
    }
  }
  else
  {
    xdeDoc->GetLeafAssemblyItems(items);
  }

  TIMER_NEW
  TIMER_GO

  // Expand compounds.
  xdeDoc->ExpandCompounds(items);

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
  Handle(asiAsm_XdeDoc) xdeDoc = Handle(cmdAsm_XdeModel)::DownCast(var)->GetDocument();

  // Read the level of hierarchy.
  int level = INT_MAX;
  //
  interp->GetKeyValue<int>(argc, argv, "level", level);

  TIMER_NEW
  TIMER_GO

  // Use assembly iterator to traverse structure.
  for ( asiAsm_XdeDocIterator it(xdeDoc, level); it.More(); it.Next() )
  {
    asiAsm_XdeAssemblyItemId id = it.Current();
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
  Handle(asiAsm_XdeDoc)   xdeDoc   = xdeModel->GetDocument();

  // Get items (if any).
  asiAsm_XdeAssemblyItemIds items;
  int itemsIdx = -1;
  //
  if ( interp->HasKeyword(argc, argv, "items", itemsIdx) )
  {
    for ( int ii = itemsIdx + 1; ii < argc; ++ii )
    {
      if ( interp->IsKeyword(argv[ii]) )
        break;

      items.Append( asiAsm_XdeAssemblyItemId(argv[ii]) );
    }
  }

  TIMER_NEW
  TIMER_GO

  // If the collection of items is empty, all parts of the model will
  // be gathered.
  asiAsm_XdePartIds pids;
  //
  xdeDoc->GetParts(items, pids);

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-get-parts")

  // Add part IDs to the interpreter.
  int pid = 1;
  //
  for ( asiAsm_XdePartIds::Iterator pit(pids); pit.More(); pit.Next(), ++pid )
  {
    *interp << pit.Value().Entry.ToCString();

    if ( pid < pids.Length() )
      *interp << " ";
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ASMXDE_FindItem(const Handle(asiTcl_Interp)& interp,
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
  Handle(asiAsm_XdeDoc)   xdeDoc   = xdeModel->GetDocument();

  // Get item name.
  std::string itemName;
  //
  if ( !interp->GetKeyValue(argc, argv, "name", itemName) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Assembly item's name is not provided.");
    return TCL_ERROR;
  }

  TIMER_NEW
  TIMER_GO

  asiAsm_XdeAssemblyItemIds items;

  // Find items.
  xdeDoc->FindItems(itemName, items);

  interp->GetProgress().SendLogMessage( LogInfo(Normal) << "%1 item(s) collected."
                                                        << items.Length() );

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "asm-xde-find-item")

  // Add items IDs to the interpreter.
  int aiid = 1;
  //
  for ( asiAsm_XdeAssemblyItemIds::Iterator aiit(items); aiit.More(); aiit.Next(), ++aiid )
  {
    *interp << aiit.Value().ToString();

    if ( aiid < items.Length() )
      *interp << " ";
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdAsm::Commands_XDE(const Handle(asiTcl_Interp)&      interp,
                          const Handle(Standard_Transient)& cmdAsm_NotUsed(data))
{
  static const char* group = "cmdAsm";

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
    "asm-xde-browse -model <M>\n"
    "\t Opens a dialog to browse the structure of the XDE document named <M>.",
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
  interp->AddCommand("asm-xde-find-item",
    //
    "asm-xde-find-item -model <M> -name <name>\n"
    "\t Finds assembly item having the passed name.",
    //
    __FILE__, group, ASMXDE_FindItem);
}
