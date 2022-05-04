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

#if defined _WIN32
  #define RuntimePathVar "PATH"
#else
  #define RuntimePathVar "LD_LIBRARY_PATH"
#endif

#undef VTK_TEST
#ifndef VTK_TEST

// exe includes
#include <exe_CommonFacilities.h>
#include <exe_Keywords.h>
#include <exe_MainWindow.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

// asiVisu includes
#include <asiVisu_CalculusLawPrs.h>
#include <asiVisu_ClearancePrs.h>
#include <asiVisu_CurvatureCombsPrs.h>
#include <asiVisu_DeviationPrs.h>
#include <asiVisu_DiscrFacePrs.h>
#include <asiVisu_FaceDomainPrs.h>
#include <asiVisu_GeomBoundaryEdgesPrs.h>
#include <asiVisu_GeomCurvePrs.h>
#include <asiVisu_GeomEdgePrs.h>
#include <asiVisu_GeomFaceContourPrs.h>
#include <asiVisu_GeomFaceNormsPrs.h>
#include <asiVisu_GeomSurfPrs.h>
#include <asiVisu_Grid2dPrs.h>
#include <asiVisu_HatchingPrs.h>
#include <asiVisu_IVCurve2dPrs.h>
#include <asiVisu_IVCurvePrs.h>
#include <asiVisu_IVPointSet2dPrs.h>
#include <asiVisu_IVPointSetPrs.h>
#include <asiVisu_IVSurfacePrs.h>
#include <asiVisu_IVTessItemPrs.h>
#include <asiVisu_IVTextItemPrs.h>
#include <asiVisu_IVTopoItemPrs.h>
#include <asiVisu_IVVectorFieldPrs.h>
#include <asiVisu_OctreePrs.h>
#include <asiVisu_PartPrs.h>
#include <asiVisu_ReCoedgePrs.h>
#include <asiVisu_ReEdgePrs.h>
#include <asiVisu_RePatchPrs.h>
#include <asiVisu_ReVertexPrs.h>
#include <asiVisu_SurfDeviationPrs.h>
#include <asiVisu_TessellationPrs.h>
#include <asiVisu_TessellationNormsPrs.h>
#include <asiVisu_ThicknessPrs.h>
#include <asiVisu_TolerantRangePrs.h>
#include <asiVisu_TriangulationPrs.h>

// asiAlgo includes
#include <asiAlgo_Dictionary.h>
#include <asiAlgo_FileFormat.h>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QDesktopWidget>
#pragma warning(pop)

// VTK includes
#pragma warning(push, 0)
#include <vtkCamera.h>
#include <vtkOpenGLRenderWindow.h>
#pragma warning(pop)

// VTK init
#include <vtkAutoInit.h>

// OCCT includes
#include <OSD.hxx>
#include <OSD_Environment.hxx>
#include <OSD_Process.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QDir>
#include <QSplashScreen>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QTimer>
#pragma warning(pop)

// Activate object factories
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkIOExportOpenGL2)
VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2)

#define EXE_LOAD_MODULE(__cf, name) \
{ \
  asiTcl_Plugin::Status status = asiTcl_Plugin::Load(__cf->Interp, __cf, name); \
  if ( status == asiTcl_Plugin::Status_Failed ) \
    __cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else if ( status == asiTcl_Plugin::Status_OK ) \
    __cf->Progress.SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

//! main().
int main(int argc, char** argv)
{
  // Check whether batch mode is requested.
  std::string scriptArg;
  const bool
    isRunScript = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runscript, scriptArg);
  const bool
    isRunCommand = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runcommand, scriptArg);
  const bool
    isBatch = isRunScript || isRunCommand;

  std::cout << "Batch mode: " << (isBatch ? "true" : "false") << std::endl;

  // Get command line arguments to process in a batch mode.
  for ( int i = 0; i < argc; ++i )
    std::cout << "Passed arg[" << i << "]: " << argv[i] << std::endl;

  // Register Presentations.
  REGISTER_PRESENTATION(asiVisu_PartPrs)
  REGISTER_PRESENTATION(asiVisu_DeviationPrs)
  REGISTER_PRESENTATION(asiVisu_DiscrFacePrs)
  REGISTER_PRESENTATION(asiVisu_OctreePrs)
  REGISTER_PRESENTATION(asiVisu_GeomBoundaryEdgesPrs)
  REGISTER_PRESENTATION(asiVisu_GeomCurvePrs)
  REGISTER_PRESENTATION(asiVisu_GeomEdgePrs)
  REGISTER_PRESENTATION(asiVisu_FaceDomainPrs)
  REGISTER_PRESENTATION(asiVisu_GeomFaceNormsPrs)
  REGISTER_PRESENTATION(asiVisu_GeomFaceContourPrs)
  REGISTER_PRESENTATION(asiVisu_GeomSurfPrs)
  REGISTER_PRESENTATION(asiVisu_Grid2dPrs)
  REGISTER_PRESENTATION(asiVisu_HatchingPrs)
  REGISTER_PRESENTATION(asiVisu_CalculusLawPrs)
  REGISTER_PRESENTATION(asiVisu_CurvatureCombsPrs)
  REGISTER_PRESENTATION(asiVisu_SurfDeviationPrs)
  REGISTER_PRESENTATION(asiVisu_TessellationPrs)
  REGISTER_PRESENTATION(asiVisu_TessellationNormsPrs)
  REGISTER_PRESENTATION(asiVisu_TolerantRangePrs)
  REGISTER_PRESENTATION(asiVisu_TriangulationPrs)
  REGISTER_PRESENTATION(asiVisu_ReCoedgePrs)
  REGISTER_PRESENTATION(asiVisu_ReEdgePrs)
  REGISTER_PRESENTATION(asiVisu_RePatchPrs)
  REGISTER_PRESENTATION(asiVisu_ReVertexPrs)
  REGISTER_PRESENTATION(asiVisu_ThicknessPrs)
  REGISTER_PRESENTATION(asiVisu_ClearancePrs)

  // Imperative viewer.
  REGISTER_PRESENTATION(asiVisu_IVPointSet2dPrs)
  REGISTER_PRESENTATION(asiVisu_IVPointSetPrs)
  REGISTER_PRESENTATION(asiVisu_IVCurve2dPrs)
  REGISTER_PRESENTATION(asiVisu_IVCurvePrs)
  REGISTER_PRESENTATION(asiVisu_IVSurfacePrs)
  REGISTER_PRESENTATION(asiVisu_IVTessItemPrs)
  REGISTER_PRESENTATION(asiVisu_IVTextItemPrs)
  REGISTER_PRESENTATION(asiVisu_IVTopoItemPrs)
  REGISTER_PRESENTATION(asiVisu_IVVectorFieldPrs)

  //---------------------------------------------------------------------------
  // Environment
  //---------------------------------------------------------------------------

  std::string workdir = OSD_Process::ExecutableFolder().ToCString();
  //
  asiAlgo_Utils::Str::ReplaceAll(workdir, "\\", "/");

  // Adjust PATH/LD_LIBRARY_PATH for loading the plugins.
  std::string
    pluginsDir = asiAlgo_Utils::Str::Slashed(workdir) + "asi-plugins";
  //
  qputenv(RuntimePathVar, qgetenv(RuntimePathVar) + ";" + pluginsDir.c_str());
  //
  std::cout << RuntimePathVar
            << " = "
            << QStr2AsciiStr( QString::fromLatin1( qgetenv(RuntimePathVar).data() ) ).ToCString()
            << std::endl;

  // Set extra environment variables for resources.
  std::string
    resDir = asiAlgo_Utils::Str::Slashed(workdir) + "resources";
  //
  if ( QDir( resDir.c_str() ).exists() )
  {
    qputenv( "CSF_PluginDefaults",    resDir.c_str() );
    qputenv( "CSF_ResourcesDefaults", resDir.c_str() );

    TCollection_AsciiString resDirStr = QStr2AsciiStr( QString::fromLatin1( resDir.data() ) );
    //
    std::cout << "CSF_PluginDefaults: " << resDirStr.ToCString() << std::endl;
    std::cout << "CSF_ResourcesDefaults: " << resDirStr.ToCString() << std::endl;

    // Load data dictionary.
    std::string dictFilename    = resDir + "/asiExeDictionary.xml";
    QString     dictFilenameStr = QString::fromLatin1( dictFilename.data() );
    //
    if ( !asiAlgo_Dictionary::Load( QStr2AsciiStr(dictFilenameStr) ) )
    {
      std::cout << "Cannot load data dictionary from "
                << QStr2AsciiStr(dictFilenameStr).ToCString() << std::endl;
    }
  }

  //---------------------------------------------------------------------------
  // Batch vs UI initialization
  //---------------------------------------------------------------------------

  if ( !isBatch )
  {
    // Needed to ensure appropriate OpenGL context is created for VTK rendering.
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setVersion(3, 2);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setRedBufferSize(1);
    fmt.setGreenBufferSize(1);
    fmt.setBlueBufferSize(1);
    fmt.setDepthBufferSize(1);
    fmt.setStencilBufferSize(0);
    fmt.setStereo(false);
    fmt.setSamples( vtkOpenGLRenderWindow::GetGlobalMaximumNumberOfMultiSamples() );
    //
    QSurfaceFormat::setDefaultFormat(fmt);

    // Prepare application.
    QApplication app(argc, argv);
    //
#ifdef _DEBUG
    QApplication::setWindowIcon( QIcon(":icons/asitus/asitus-debug_icon_16x16.png") );
#else
    QApplication::setWindowIcon( QIcon(":icons/asitus/asitus_icon_16x16.png") );
#endif

    // Splash screen.
    QSplashScreen* pSplash = nullptr;
    //
    if ( !isBatch )
    {
      pSplash = new QSplashScreen( QPixmap(":img/asitus/splash.png"), Qt::WindowStaysOnTopHint );
      pSplash->show();
    }

    // Construct main window but do not show it to allow off-screen batch.
    exe_MainWindow* pMainWindow = new exe_MainWindow(isBatch);

    // Give splash screen some seconds, no matter how fast the main window appears.
    if ( pSplash )
    {
      QTimer::singleShot( 3000, pSplash, SLOT( close() ) );
      QTimer::singleShot( 3000, pMainWindow, SLOT( slInit() ) );
    }

    // Let Qt do whatever it wants to do before showing UI. This helps
    // to avoid some sort of blinking on launch.
    QApplication::processEvents(QEventLoop::AllEvents, 10000);

    // Move to a handy position.
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    const int center_x   = ( screenGeometry.width() - pMainWindow->width() ) / 2;
    const int center_y   = ( screenGeometry.height() - pMainWindow->height() ) / 2;
    //
    pMainWindow->move(center_x/8, center_y/4);

    // Show main window.
    pMainWindow->show();

    // Set focus on Tcl console.
    pMainWindow->Widgets.wConsole->setFocus();

    //---------------------------------------------------------------------------
    // Check the autoread log
    //---------------------------------------------------------------------------

    QFile qFile(asiTcl_AutoLogFilename);
    //
    if ( qFile.exists() )
    {
      if ( qFile.open(QIODevice::ReadOnly | QFile::Text) )
      {
        QTextStream in(&qFile);
        pMainWindow->Widgets.wConsole->setText( in.readAll() );
      }
    }

    //---------------------------------------------------------------------------
    // Process the second argument to open the passed file
    //---------------------------------------------------------------------------

    if ( argc == 2 )
    {
      QStringList qtArgs = QApplication::arguments();
      //
      TCollection_AsciiString
        arg1Str = QStr2AsciiStr( QDir::fromNativeSeparators( qtArgs.at(1) ) );

      // Prepare Tcl command.
      TCollection_AsciiString cmd;
      //
      cmd = "load-part"; cmd += " \""; cmd += arg1Str; cmd += "\"";

      // Execute command.
      if ( !cmd.IsEmpty() )
      {
        QApplication::processEvents(QEventLoop::AllEvents, 10000);

        // Get Tcl interpeter.
        const Handle(asiTcl_Interp)&
          interp = pMainWindow->Widgets.wConsole->GetInterp();

        if ( interp->Eval(cmd) != TCL_OK )
          std::cout << "Tcl finished with error." << std::endl;

        QApplication::processEvents(QEventLoop::AllEvents, 10000);

        if ( interp->Eval("fit") != TCL_OK )
          std::cout << "Tcl finished with error." << std::endl;
      }
    }

    // Run event loop.
    return app.exec();
  }

  else /* Batch mode */
  {
    std::cout << "Running Analysis Situs in batch mode..." << std::endl;

    // Prepare common facilities for batch mode.
    Handle(asiUI_BatchFacilities) cf = asiUI_BatchFacilities::Instance();

    // Load default commands.
    EXE_LOAD_MODULE(cf, "cmdMisc")
    EXE_LOAD_MODULE(cf, "cmdEngine")
    EXE_LOAD_MODULE(cf, "cmdRE")
    EXE_LOAD_MODULE(cf, "cmdDDF")
    EXE_LOAD_MODULE(cf, "cmdAsm")
    //
#ifdef USE_MOBIUS
    EXE_LOAD_MODULE(cf, "cmdMobius")
#endif

    // Lookup for custom plugins and try to load them.
    QDir pluginDir( QDir::currentPath() + "/asi-plugins" );
    TCollection_AsciiString pluginDirStr = pluginDir.absolutePath().toLatin1().data();
    //
    std::cout << "Looking for plugins at "
              << pluginDirStr.ToCString() << "..." << std::endl;
    //
    QStringList cmdLibs = pluginDir.entryList(QStringList() << "*.dll", QDir::Files);
    //
    foreach ( QString cmdLib, cmdLibs )
    {
      TCollection_AsciiString cmdLibName = cmdLib.section(".", 0, 0).toLatin1().data();
      //
      cf->Progress.SendLogMessage(LogNotice(Normal) << "Detected %1 as a custom plugin's library."
                                                    << cmdLibName);

      EXE_LOAD_MODULE(cf, cmdLibName);
    }

    // Execute script.
    const int
      ret = cf->Interp->Eval( isRunScript ? asiTcl_SourceCmd( scriptArg.c_str() )
                 /* run single command */ : scriptArg.c_str() );

    // Check result.
    if ( ret != TCL_OK )
      std::cout << "Batch mode finished with error code " << ret << "." << std::endl;
    else
      std::cout << "Batch mode finished successfully (error code " << ret << ")." << std::endl;

    return ret;
  }
}

#else

// VTK init
#include <vtkAutoInit.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);

int main(int, char *[])
{
  std::cout << "Hello, offscreen rendering!" << std::endl;

   // Create a sphere
   vtkSmartPointer<vtkSphereSource> sphereSource =
     vtkSmartPointer<vtkSphereSource>::New();

  // Create a mapper and actor
  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(sphereSource->GetOutputPort());

  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  // A renderer and render window
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetOffScreenRendering( 1 );
  renderWindow->AddRenderer(renderer);

  // Add the actors to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(1,1,1); // Background color white

  renderWindow->Render();

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->Update();

  vtkSmartPointer<vtkPNGWriter> writer =
    vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName("screenshot.png");
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();

  return 0;
}

#endif
