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
#include <exe_CommandServer.h>
#include <exe_Keywords.h>

// OpenCascade includes
#include <OSD_Process.hxx>

// VTK init
#pragma warning(push, 0)
#include <vtkAutoInit.h>
#pragma warning(pop)

// Qt includes
#pragma warning(push, 0)
#include <QCoreApplication>
#include <QHostAddress>
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

//! Application utilities.
namespace CliUtils
{
  QHostAddress Host;    //!< CLI host.
  int          PortNum; //!< Port number.
}

//-----------------------------------------------------------------------------

//! main().
int main(int argc, char** argv)
{
  // Read host and port for the server.
  CliUtils::Host    = CLI_HostDefault;
  CliUtils::PortNum = CLI_PortDefault;
  std::string addrStr, portStr;
  //
  if ( asiExeCli::GetKeyValue(argc, argv, "host", addrStr) )
  {
    CliUtils::Host.setAddress( addrStr.c_str() );
  }
  //
  if ( asiExeCli::GetKeyValue(argc, argv, "port", portStr) )
  {
    CliUtils::PortNum = asiAlgo_Utils::Str::ToNumber<int>(portStr);
  }

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
  // Prepare common facilities and interpretor
  //---------------------------------------------------------------------------

  // Create common facilities out of threads.
  Handle(asiUI_BatchFacilities)
    cf = asiUI_BatchFacilities::Instance(true, true, false);

  //---------------------------------------------------------------------------
  // Create server thread
  //---------------------------------------------------------------------------

  QCoreApplication app(argc, argv);

  // Create Server
  exe_CommandServer*
    pServer = new exe_CommandServer(cf->Model,
                                    cf->Progress,
                                    cf->Plotter,
                                    CliUtils::Host,
                                    CliUtils::PortNum);
  //
  pServer->Run();

  return app.exec();
}
