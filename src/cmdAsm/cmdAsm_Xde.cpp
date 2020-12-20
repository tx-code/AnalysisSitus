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
#include <asiAsm_XdeDoc.h>

// asiUI includes
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

  // Load data from file.
  if ( !doc->Load( filename.c_str() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot load XDE document from file '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

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

  // Save document to file.
  if ( !doc->SaveAs( filename.c_str() ) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot save XDE document to file '%1'."
                                                        << filename);
    return TCL_ERROR;
  }

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

  // Open UI widget for browsing the assembly structure.
  asiUI_XdeBrowser* pBrowser = new asiUI_XdeBrowser(xdeModel->GetDocument(), cmdAsm::cf);
  //
  pBrowser->Populate();
  //
  pBrowser->show();

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
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Expand XDE compounds")

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
}
