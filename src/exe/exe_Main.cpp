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
#include <exe_GenerateDocs.h>
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
#include <asiVisu_IVAxesPrs.h>
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

// asiUI includes
#include <dialogs/asiUI_DialogDump.h>

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

int runJsonView(int argc,
                char** argv,
                const std::string& scriptArg)
{
  if (scriptArg.empty())
    return 1;

  QApplication app(argc, argv);
  exe_MainWindow::setApplicationStyle(":qdarkstyle/style.qss");

  std::string fileName = scriptArg;
  QFile file(fileName.c_str());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return 0;

  QString fileBuff = file.readAll().constData();

  asiUI_DialogDump* dlg = new asiUI_DialogDump("Json text");
  dlg->Populate(fileBuff.toStdString());
  dlg->show();

  app.exec();
  return 0;
}

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

//! main().
int main(int argc, char** argv)
{
  QLocale::setDefault(QLocale::c());

  // Check whether batch mode is requested.
  std::string scriptArg;
  const bool
    isRunScript = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runscript, scriptArg);
  const bool
    isRunCommand = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runcommand, scriptArg);
  const bool
    isRunJsonView = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runjsonview, scriptArg);
  const bool
    isGenDoc = asiExe::HasKeyword(argc, argv, ASITUS_KW_gendoc);
  const bool
    isBatch = isRunScript || isRunCommand || isGenDoc;

  if ( isRunJsonView )
  {
    return runJsonView(argc, argv, scriptArg);
  }

  OSD::SetSignal(true);
  OSD::SetFloatingSignal(false);

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
  REGISTER_PRESENTATION(asiVisu_IVAxesPrs)
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

    // Absolute filename.
    QString logFilename = QDir::currentPath() + "/" + asiTcl_AutoLogFilename;

    std::cout << "Searching for autolog at " << QStr2StdStr(logFilename) << "...";

    QFile qFile(logFilename);
    //
    if ( qFile.exists() )
    {
      std::cout << " found." << std::endl;
      if ( qFile.open(QIODevice::ReadOnly | QFile::Text) )
      {
        QTextStream in(&qFile);
        pMainWindow->Widgets.wConsole->setText( in.readAll() );
      }
    }
    else
    {
      std::cout << " not found." << std::endl;
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

      // Check format.
      TCollection_AsciiString ext = asiAlgo_FileFormatTool::GetFileExtension(arg1Str);

      // Prepare Tcl command.
      TCollection_AsciiString cmd;
      //
      if ( ext == ACTBinExt )
      {
        cmd = "load"; cmd += " \""; cmd += arg1Str; cmd += "\"";
      }
      else
      {
        cmd = "load-part"; cmd += " \""; cmd += arg1Str; cmd += "\"";
      }

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
    EXE_LOAD_MODULE(cf, "cmdTest")
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
	QStringList cmdLibs;
#if defined WIN32
    cmdLibs = pluginDir.entryList(QStringList() << "*.dll", QDir::Files);
#else
    cmdLibs = pluginDir.entryList(QStringList() << "*.so", QDir::Files);
#endif
    //
    foreach ( QString cmdLib, cmdLibs )
    {
      TCollection_AsciiString cmdLibName = cmdLib.section(".", 0, 0).toLatin1().data();
      //
      cf->Progress.SendLogMessage(LogNotice(Normal) << "Detected %1 as a custom plugin's library."
                                                    << cmdLibName);

      EXE_LOAD_MODULE(cf, cmdLibName);
    }

    if ( isGenDoc )
    {
      std::string
        docsDir = asiAlgo_Utils::Str::Slashed( asiAlgo_Utils::Env::AsiDocs() );

      cf->Progress.SendLogMessage(LogNotice(Normal) << "Generating commands list in '%1'..."
                                                    << docsDir);

      std::string docFnIn  (docsDir + "commands_template.html");
      std::string docFnOut (docsDir + "commands.html");

      /* Generate documentation page with all Tcl commands listed */
      exe_GenerateDocs::Perform(cf->Interp, docFnIn, docFnOut);
    }
    else
    {
      /* Execute batch job */

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

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkGlyph3DMapper.h>
#include <vtkLabeledDataMapper.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTextProperty.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkGlyph3DMapper.h>
#include <vtkIntArray.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkPointSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkStringArray.h>
#include <vtkTextProperty.h>

namespace {
/**
 * Convert points to glyphs.
 *
 * @param points - The points to glyph
 * @param scale - The scale, used to determine the size of the glyph
 * representing the point, expressed as a fraction of the largest side of the
 * bounding box surrounding the points. e.g. 0.05
 *
 * @return The actor.
 */
vtkSmartPointer<vtkActor> PointToGlyph(vtkPoints* points, double const& scale);

} // namespace
int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Create a point set.
  vtkNew<vtkPointSource> pointSource;
  pointSource->SetNumberOfPoints(6);
  pointSource->Update();

  // Add label array.
  vtkNew<vtkStringArray> labels;
  labels->SetNumberOfValues(6);
  labels->SetName("labels");
  labels->SetValue(0, "Priority 10");
  labels->SetValue(1, "Priority 7");
  labels->SetValue(2, "Priority 6");
  labels->SetValue(3, "Priority 4");
  labels->SetValue(4, "Priority 4");
  labels->SetValue(5, "Priority 4");
  pointSource->GetOutput()->GetPointData()->AddArray(labels);

  // Add priority array.
  vtkNew<vtkIntArray> sizes;
  sizes->SetNumberOfValues(6);
  sizes->SetName("sizes");
  sizes->SetValue(0, 10);
  sizes->SetValue(1, 7);
  sizes->SetValue(2, 6);
  sizes->SetValue(3, 4);
  sizes->SetValue(4, 4);
  sizes->SetValue(5, 4);
  pointSource->GetOutput()->GetPointData()->AddArray(sizes);

  // Create a mapper and actor for the points.
  vtkNew<vtkPolyDataMapper> pointMapper;
  pointMapper->SetInputConnection(pointSource->GetOutputPort());

  vtkNew<vtkActor> pointActor;
  pointActor->SetMapper(pointMapper);

  // Map the points to spheres
  //auto sphereActor = PointToGlyph(pointSource->GetOutput()->GetPoints(), 0.05);
  //sphereActor->GetProperty()->SetColor(
  //    colors->GetColor3d("MistyRose").GetData());

  // Generate the label hierarchy.
  vtkNew<vtkPointSetToLabelHierarchy> pointSetToLabelHierarchyFilter;
  pointSetToLabelHierarchyFilter->SetInputConnection(
      pointSource->GetOutputPort());
  pointSetToLabelHierarchyFilter->SetLabelArrayName("labels");
  pointSetToLabelHierarchyFilter->SetPriorityArrayName("sizes");
  pointSetToLabelHierarchyFilter->Update();

  // Create a mapper and actor for the labels.
  vtkNew<vtkLabelPlacementMapper> labelMapper;
  labelMapper->SetInputConnection(
      pointSetToLabelHierarchyFilter->GetOutputPort());
  vtkNew<vtkActor2D> labelActor;
  labelActor->SetMapper(labelMapper);
  // labelActor->GetProperty()->SetColor(
  //    colors->GetColor3d("Yellow").GetData());

  // Create a renderer, render window, and interactor.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("LabelPlacementMapper");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Add the actors to the scene.
  renderer->AddActor(pointActor);
  //renderer->AddActor(sphereActor);
  renderer->AddActor(labelActor);
  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());

  // Render and interact.
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

namespace {

vtkSmartPointer<vtkActor> PointToGlyph(vtkPoints* points, double const& scale)
{
  auto bounds = points->GetBounds();
  double maxLen = 0;
  for (int i = 1; i < 3; ++i)
  {
    maxLen = std::max(bounds[i + 1] - bounds[i], maxLen);
  }

  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(scale * maxLen);

  vtkNew<vtkPolyData> pd;
  pd->SetPoints(points);

  vtkNew<vtkGlyph3DMapper> mapper;
  mapper->SetInputData(pd);
  mapper->SetSourceConnection(sphereSource->GetOutputPort());
  mapper->ScalarVisibilityOff();
  mapper->ScalingOff();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  return actor;
}

} // namespace

#endif
