//-----------------------------------------------------------------------------
// Created on: 07 November 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2015-present, Sergey Slyadnev
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

// exe includes
#include <exe_CommandWindow.h>
#include <exe_Keywords.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

// OpenCascade includes
#include <OSD_Process.hxx>

// VTK init
#pragma warning(push, 0)
#include <vtkAutoInit.h>
#pragma warning(pop)

// Qt includes
#pragma warning(push, 0)
#include <QDir>
#pragma warning(pop)

// Activate object factories
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkIOExportOpenGL2)
VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2)

#if defined _WIN32
  #define RuntimePathVar "PATH"
#else
  #define RuntimePathVar "LD_LIBRARY_PATH"
#endif

//-----------------------------------------------------------------------------

#define EXE_LOAD_MODULE(name) \
{ \
  Handle(asiUI_BatchFacilities) __cf = asiUI_BatchFacilities::Instance();\
  \
  asiTcl_Plugin::Status status = asiTcl_Plugin::Load(__cf->Interp, __cf, name); \
  if ( status == asiTcl_Plugin::Status_Failed ) \
    __cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else if ( status == asiTcl_Plugin::Status_OK ) \
    __cf->Progress.SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

//-----------------------------------------------------------------------------

//! Application utilities.
namespace CliUtils
{
  Handle(exe_CommandQueue) Queue; //!< Command queue.
}

//-----------------------------------------------------------------------------
// Interpreter's thread
//-----------------------------------------------------------------------------

//! Working routine for Tcl interpreter thread.
DWORD WINAPI Thread_Interp(LPVOID)
{
  Handle(asiUI_BatchFacilities)
    cf = asiUI_BatchFacilities::Instance();

  // Give the console window time to appear.
  Sleep(100);

  cf->Interp = new asiTcl_Interp;
  cf->Interp->Init(true);
  cf->Interp->SetModel(cf->Model);
  cf->Interp->SetProgress(cf->Progress);
  cf->Interp->SetPlotter(cf->Plotter);

  // Load default commands.
  EXE_LOAD_MODULE("cmdMisc")
  EXE_LOAD_MODULE("cmdEngine")
  EXE_LOAD_MODULE("cmdRE")
  EXE_LOAD_MODULE("cmdDDF")
  EXE_LOAD_MODULE("cmdAsm")
  //
#ifdef USE_MOBIUS
  EXE_LOAD_MODULE("cmdMobius")
#endif

  // Lookup for custom plugins and try to load them.
  QDir pluginDir( QDir::currentPath() + "/asi-plugins" );
  TCollection_AsciiString pluginDirStr = pluginDir.absolutePath().toLatin1().data();
  //
  std::cout << "Looking for plugins at "
            << pluginDirStr.ToCString() << std::endl;
  //
  QStringList cmdLibs = pluginDir.entryList(QStringList() << "*.dll", QDir::Files);
  //
  foreach ( QString cmdLib, cmdLibs )
  {
    TCollection_AsciiString cmdLibName = cmdLib.section(".", 0, 0).toLatin1().data();
    //
    cf->Progress.SendLogMessage(LogNotice(Normal) << "Detected %1 as a custom plugin's library."
                                                  << cmdLibName);

    EXE_LOAD_MODULE(cmdLibName);
  }

  std::cout << AS_CMD_PROMPT;

  while ( 1 )
  {
    Handle(exe_BaseCmd) LastCommand = CliUtils::Queue->Last();

    // If there is something to proceed, let us do it
    if ( !LastCommand.IsNull() )
    {
      // Remove command from queue and execute it.
      CliUtils::Queue->Pop();
      //
      if ( !cf->Interp.IsNull() )
      {
        cf->Interp->Eval(LastCommand->Cmd);
        std::cout << AS_CMD_PROMPT;
      }
    }
  }

  return 0;
}

//! main().
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  //---------------------------------------------------------------------------
  // Set extra environment variables
  //---------------------------------------------------------------------------

  std::string workdir = OSD_Process::ExecutableFolder().ToCString();
  //
  asiAlgo_Utils::Str::ReplaceAll(workdir, "\\", "/");
  std::string
    resDir = asiAlgo_Utils::Str::Slashed(workdir) + "resources";

  qputenv( "CSF_PluginDefaults",    resDir.c_str() );
  qputenv( "CSF_ResourcesDefaults", resDir.c_str() );

  // Adjust PATH/LD_LIBRARY_PATH for loading the plugins.
  std::string
    pluginsDir = asiAlgo_Utils::Str::Slashed(workdir) + "asi-plugins";
  //
  qputenv(RuntimePathVar, qgetenv(RuntimePathVar) + ";" + pluginsDir.c_str());

  //---------------------------------------------------------------------------
  // Create CLI threads
  //---------------------------------------------------------------------------

  // Create command queue in the main thread
  if ( CliUtils::Queue.IsNull() )
    CliUtils::Queue = new exe_CommandQueue;

  // Create common facilities out of threads.
  Handle(asiUI_BatchFacilities)
    cf = asiUI_BatchFacilities::Instance(true, false, false);

  // Create thread for Interpreter
  HANDLE hInterpThread = CreateThread(NULL, 0, Thread_Interp, NULL, 0, NULL);
  if ( !hInterpThread)
    ExitProcess(NULL);

  // Create terminal.
  exe_CommandWindow ConsoleWindow(CliUtils::Queue);
  //
  if ( !ConsoleWindow.Create(true) )
    return 1;

  // Start message loop.
  ConsoleWindow.StartMessageLoop();

  // Close all thread handles upon completion
  CloseHandle(hInterpThread);

  return 0; // Success.
}
