project(asiExe)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  exe_GenerateDocs.h
  exe_Keywords.h
)
set (CPP_FILES
  exe_GenerateDocs.cpp
  exe_Main.cpp
)

#------------------------------------------------------------------------------
# UI
#------------------------------------------------------------------------------

set (ui_H_FILES
  ui/exe_CommonFacilities.h
)
set (ui_H_UI_FILES
  ui/exe_MainWindow.h
)
set (ui_CPP_FILES
  ui/exe_CommonFacilities.cpp
  ui/exe_MainWindow.cpp
)

#------------------------------------------------------------------------------
set (OCCT_LIB_FILES
  TKernel
  TKMath
  TKBRep
  TKOffset
  TKTopAlgo
  TKG2d
  TKG3d
  TKGeomBase
  TKGeomAlgo
  TKMesh
  TKShHealing
  TKFeat
  TKBool
  TKBO
  TKPrim
  TKBin
  TKBinL
  TKBinXCAF
  TKLCAF
  TKCDF
  TKCAF
  TKXCAF
  TKService
  TKXSBase
  TKSTEP
  TKIGES
  TKXDESTEP
  TKXDEIGES
)
#------------------------------------------------------------------------------
set (VTK_LIB_FILES
  vtkChartsCore
  vtkCommonCore
  vtkCommonColor
  vtkCommonDataModel
  vtkCommonExecutionModel
  vtkCommonMath
  vtkCommonTransforms
  vtkCommonMisc
  vtkFiltersCore
  vtkFiltersGeneral
  vtkFiltersSources
  vtkFiltersGeometry
  vtkFiltersParallel
  vtkFiltersExtraction
  vtkFiltersModeling
  vtkGUISupportQt
  vtkInfovisLayout
  vtkIOCore
  vtkIOImage
  vtkIOExportOpenGL2
  vtkImagingCore
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkRenderingAnnotation
  vtkRenderingContext2D
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingOpenGL2
  vtkRenderingGL2PSOpenGL2
  vtkRenderingLabel
  vtkViewsContext2D
  vtkViewsInfovis
)

#------------------------------------------------------------------------------
# Add sources
#------------------------------------------------------------------------------

foreach (FILE ${H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${ui_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\UI" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${ui_H_UI_FILES})
  set (src_files ${src_files} ${FILE})
  #
  unset (MOC_FILE)
  qt5_wrap_cpp(MOC_FILE ${FILE})
  message (STATUS "... Info: next MOC file ${MOC_FILE}")
  set (ui_moc_files ${ui_moc_files} ${MOC_FILE})
  #
  source_group ("Generated" FILES "${MOC_FILE}")
  source_group ("Header Files\\UI" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${ui_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\UI" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Resources
#------------------------------------------------------------------------------

source_group ("Resources" FILES resources/asiExeDictionary.xml)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (exe_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};\
  ${CMAKE_CURRENT_SOURCE_DIR}/ui;")
#
set (exe_include_dir ${exe_include_dir_loc} PARENT_SCOPE)

include_directories ( SYSTEM
                      ${exe_include_dir_loc}
                      ${asiActiveData_include_dir}
                      ${asiTcl_include_dir}
                      ${asiAlgo_include_dir}
                      ${asiData_include_dir}
                      ${asiVisu_include_dir}
                      ${asiEngine_include_dir}
                      ${asiUI_include_dir}
                      ${asiTestEngine_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR}
                      ${3RDPARTY_tbb_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

COPY_RESOURCES_TO_BINARY_DIRS(resources/asiExeDictionary.xml)

#------------------------------------------------------------------------------
# Qt resources
#------------------------------------------------------------------------------

qt5_add_resources(RCC_FILE resources/style.qrc)
source_group ("Resources" FILES resources/style.qrc)
source_group ("Generated" FILES "${RCC_FILE}")

#------------------------------------------------------------------------------
# Create executable
#------------------------------------------------------------------------------

add_executable(asiExe ${ui_moc_files} ${src_files} ${RCC_FILE} resources/asiExeDictionary.xml)

set (USE_CONSOLE OFF CACHE BOOL "Using of debug console")
if (WIN32 AND MSVC)
  if (USE_CONSOLE)
    set_target_properties(asiExe PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup" OUTPUT_NAME "asiExe")
  else ()
    set_target_properties(asiExe PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup" OUTPUT_NAME "asiExe")
  endif ()
endif()

#------------------------------------------------------------------------------
# Configure template
#------------------------------------------------------------------------------

set (X_COMPILER_BITNESS "x${COMPILER_BITNESS}")

configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/exePROTOTYPE.vcxproj.user.in
               ${asiExe_BINARY_DIR}/asiExe.vcxproj.user @ONLY)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

qt5_use_modules(asiExe Core Widgets)

target_link_libraries(asiExe asiTcl asiAlgo asiData asiVisu asiEngine asiTestEngine asiUI)

if (3RDPARTY_tbb_LIBRARY_DIR_DEBUG)
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR_DEBUG})
else()
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR})
endif()

foreach (LIB_FILE ${OCCT_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiExe debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExe optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExe ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${VTK_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiExe debug ${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExe optimized ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExe ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a software
#------------------------------------------------------------------------------

if (WIN32)
  install (TARGETS asiExe CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiExe CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiExe CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
else()
  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin/asiExe DESTINATION bin
           CONFIGURATIONS Release
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bind/asiExe DESTINATION bind
           CONFIGURATIONS Debug
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bini/asiExe DESTINATION bini
           CONFIGURATIONS RelWithDebInfo
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_SOURCE_DIR}/asiExe.sh DESTINATION bin
           CONFIGURATIONS Release
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_SOURCE_DIR}/asiExe.sh DESTINATION bini
          CONFIGURATIONS RelWithDebInfo
          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
          GROUP_EXECUTE GROUP_READ
          WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_SOURCE_DIR}/asiExe.sh DESTINATION bind
          CONFIGURATIONS Debug
          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
          GROUP_EXECUTE GROUP_READ
          WORLD_EXECUTE WORLD_READ)
endif()

install (FILES resources/asitus/icon.ico CONFIGURATIONS Release DESTINATION bin)
install (FILES resources/asitus/icon64.ico CONFIGURATIONS Release DESTINATION bin)
install (FILES resources/asiExeDictionary.xml CONFIGURATIONS Release DESTINATION bin/resources)

install (FILES resources/asitus/icon.ico CONFIGURATIONS RelWithDebInfo DESTINATION bini)
install (FILES resources/asitus/icon64.ico CONFIGURATIONS RelWithDebInfo DESTINATION bini)
install (FILES resources/asiExeDictionary.xml CONFIGURATIONS RelWithDebInfo DESTINATION bini/resources)

install (FILES resources/asitus/icon.ico CONFIGURATIONS Debug DESTINATION bind)
install (FILES resources/asitus/icon64.ico CONFIGURATIONS Debug DESTINATION bind)
install (FILES resources/asiExeDictionary.xml CONFIGURATIONS Debug DESTINATION bind/resources)
