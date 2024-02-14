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

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiUI includes
#include <asiUI_DialogCommands.h>
#include <asiUI_IV.h>

// STL includes
#include <regex>

//-----------------------------------------------------------------------------

Handle(asiEngine_Model)        cmdEngine::model = nullptr;
Handle(asiUI_CommonFacilities) cmdEngine::cf    = nullptr;

//-----------------------------------------------------------------------------

int ENGINE_EnableNotifier(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const bool wasSilent = !interp->IsNotifierOn();

  interp->SetNotifierOn();

  if ( wasSilent )
    interp->GetProgress().SendLogMessage(LogNotice(Normal) << "Interpretor exits silent mode.");

  return TCL_OK;
}


//-----------------------------------------------------------------------------

int ENGINE_Notifier(const Handle(asiTcl_Interp)& interp,
                    int                          argc,
                    const char**                 argv)
{
  if ( interp->HasKeyword(argc, argv, "init") )
  {
    interp->GetProgress().Init();
    interp->GetProgress().SetMessageKey("Running...");
  }

  if ( interp->HasKeyword(argc, argv, "step") )
    interp->GetProgress().StepProgress(1);

  if ( interp->HasKeyword(argc, argv, "finish") )
    interp->GetProgress().SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DisableNotifier(const Handle(asiTcl_Interp)& interp,
                           int                          argc,
                           const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  interp->GetProgress().SendLogMessage(LogNotice(Normal) << "Interpretor enters silent mode.");
  interp->SetNotifierOff();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_EnablePlotter(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiUI_IV)
    IV = Handle(asiUI_IV)::DownCast( interp->GetPlotter().Access() );
  //
  if ( !IV.IsNull() )
    IV->VISUALIZATION_ON();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DisablePlotter(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiUI_IV)
    IV = Handle(asiUI_IV)::DownCast( interp->GetPlotter().Access() );
  //
  if ( !IV.IsNull() )
    IV->VISUALIZATION_OFF();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_EnableTransactions(const Handle(asiTcl_Interp)& interp,
                              int                          argc,
                              const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  interp->GetModel()->EnableTransactions();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DisableTransactions(const Handle(asiTcl_Interp)& interp,
                               int                          argc,
                               const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  interp->GetModel()->DisableTransactions();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_EnableBrowser(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiUI_IV)
    IV = Handle(asiUI_IV)::DownCast( interp->GetPlotter().Access() );
  //
  if ( !IV.IsNull() )
    IV->BROWSER_ON();

  if ( cmdEngine::cf->ObjectBrowser )
    cmdEngine::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_DisableBrowser(const Handle(asiTcl_Interp)& interp,
                          int                          argc,
                          const char**                 argv)
{
  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  Handle(asiUI_IV)
    IV = Handle(asiUI_IV)::DownCast( interp->GetPlotter().Access() );
  //
  if ( !IV.IsNull() )
    IV->BROWSER_OFF();

  return TCL_OK;
}

//-----------------------------------------------------------------------------
#define DEBUG_GENERATE_PIXMAP
#include <BRepPreviewAPI_MakeBox.hxx>
#include <Image_AlienPixMap.hxx>

int ENGINE_ShowCommands(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
#ifdef DEBUG_GENERATE_PIXMAP
  gp_Pnt thePntMin (0, 0, 0);
  gp_Pnt thePntMax (100, 100, 100);
  int width = 200;
  int height = 300;

  BRepPreviewAPI_MakeBox aMakeBox;
  aMakeBox.Init (thePntMin, thePntMax);
  TopoDS_Shape shape = aMakeBox.Shape();
  Handle(Image_AlienPixMap) pixmap =
    asiAlgo_Utils::Graphics::GeneratePixmap(shape, width, height);
  pixmap->Save("d:/tmp/pixmap.png");

  return 0;
#endif


  if ( argc != 1 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  if ( !cmdEngine::cf )
    return TCL_OK;

  asiUI_DialogCommands*
    pDlg = new asiUI_DialogCommands(interp, interp->GetProgress(), nullptr);
  pDlg->setConsole(cmdEngine::cf->Console);
  //
  pDlg->show();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int ENGINE_Help(const Handle(asiTcl_Interp)& interp,
                int                          argc,
                const char**                 argv)
{
  if ( argc != 2 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  std::string originalCommand = argv[1];

  std::vector<asiTcl_CommandInfo> commands;
  interp->GetAvailableCommands(commands);

  std::string help = "";
  std::vector<asiTcl_CommandInfo>::const_iterator itCommands = commands.cbegin();
  for ( ; itCommands != commands.cend(); ++itCommands )
  {
    const asiTcl_CommandInfo& command = *itCommands;
    const std::string name            = command.Name;

    if ( originalCommand != name )
    {
      continue;
    }

    help = command.Help;
    break;
  }

  help = std::regex_replace(help, std::regex("<"), "&lt;");
  help = std::regex_replace(help, std::regex(">"), "&gt;");

  if ( help.empty() )
  {
    std::string message = std::string("Command '") + originalCommand + std::string("' was not found");
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << message.c_str());
  }
  else
  {
    interp->GetProgress().SendLogMessage(LogInfo(Normal) << help.c_str());
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdEngine::ClearViewers(const bool repaint)
{
  if ( cf.IsNull() )
    return;

  /* Update Part viewer */
  if ( cf->ViewerPart )
  {
    const vtkSmartPointer<asiVisu_PrsManager>& partPM = cf->ViewerPart->PrsMgr();
    partPM->DeleteAllPresentations();

    if ( repaint )
      cf->ViewerPart->Repaint();
  }

  /* Update Host viewer */
  if ( cf->ViewerHost )
  {
    const vtkSmartPointer<asiVisu_PrsManager>& hostPM = cf->ViewerHost->PrsMgr();
    hostPM->DeleteAllPresentations();

    if ( repaint )
      cf->ViewerHost->Repaint();
  }

  /* Update Domain viewer */
  if ( cf->ViewerDomain )
  {
    const vtkSmartPointer<asiVisu_PrsManager>& domainPM = cf->ViewerDomain->PrsMgr();
    domainPM->DeleteAllPresentations();

    if ( repaint )
      cf->ViewerDomain->Repaint();
  }
}

//-----------------------------------------------------------------------------

void cmdEngine::Factory(const Handle(asiTcl_Interp)&      interp,
                        const Handle(Standard_Transient)& data)
{
  static const char* group = "cmdEngine";

  /* ==========================
   *  Initialize UI facilities
   * ========================== */

  // Get common facilities
  Handle(asiUI_CommonFacilities)
    passedCF = Handle(asiUI_CommonFacilities)::DownCast(data);
  //
  if ( passedCF.IsNull() )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "[cmdEngine] UI facilities are not available. GUI may not be updated.");
  else
    cf = passedCF;

  /* ================================
   *  Initialize Data Model instance
   * ================================ */

  model = Handle(asiEngine_Model)::DownCast( interp->GetModel() );
  //
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "[cmdEngine] Data Model instance is null or not of asiEngine_Model kind.");
    return;
  }

  /* =====================
   *  Add custom commands
   * ===================== */

  interp->AddCommand("enable-notifier",
    //
    "enable-notifier\n"
    "\t Enables notification messages.",
    //
    __FILE__, group, ENGINE_EnableNotifier);

  interp->AddCommand("notifier",
    //
    "notifier {-init, -step, -finish}\n"
    "\t Manages notifier.",
    //
    __FILE__, group, ENGINE_Notifier);

  interp->AddCommand("disable-notifier",
    //
    "disable-notifier\n"
    "\t Disables notification messages.",
    //
    __FILE__, group, ENGINE_DisableNotifier);

  interp->AddCommand("enable-plotter",
    //
    "enable-plotter\n"
    "\t Enables imperative plotter.",
    //
    __FILE__, group, ENGINE_EnablePlotter);

  interp->AddCommand("disable-plotter",
    //
    "disable-plotter\n"
    "\t Disables imperative plotter.",
    //
    __FILE__, group, ENGINE_DisablePlotter);

  interp->AddCommand("enable-transactions",
    //
    "enable-transactions\n"
    "\t Enables data model transactions.",
    //
    __FILE__, group, ENGINE_EnableTransactions);

  interp->AddCommand("disable-transactions",
    //
    "disable-transactions\n"
    "\t Disables data model transactions.",
    //
    __FILE__, group, ENGINE_DisableTransactions);

  interp->AddCommand("enable-browser",
    //
    "enable-browser\n"
    "\t Enables object browser.",
    //
    __FILE__, group, ENGINE_EnableBrowser);

  interp->AddCommand("disable-browser",
    //
    "disable-browser\n"
    "\t Disables object browser.",
    //
    __FILE__, group, ENGINE_DisableBrowser);

  interp->AddCommand("show-commands",
    //
    "show-commands\n"
    "\t Opens UI dialog listing all available Tcl commands.",
    //
    __FILE__, group, ENGINE_ShowCommands);

  interp->AddCommand("help",
    //
    "help <command>\n"
    "\t Return the hints for command.",
    //
    __FILE__, group, ENGINE_Help);

  // Load sub-modules.
  Commands_Data        (interp, data);
  Commands_Editing     (interp, data);
  Commands_Inspection  (interp, data);
  Commands_Interaction (interp, data);
  Commands_Interop     (interp, data);
  Commands_Modeling    (interp, data);
  Commands_Naming      (interp, data);
  Commands_Viewer      (interp, data);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdEngine)
