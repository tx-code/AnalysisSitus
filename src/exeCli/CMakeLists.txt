project(asiExeCli)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  exe_BaseCmd.h
  exe_CommandQueue.h
  exe_CommandWindow.h
  exe_Keywords.h
  exe_SharedQueue.h
)
set (CPP_FILES
  exe_BaseCmd.cpp
  exe_CommandWindow.cpp
  exe_Main.cpp
  exe_SharedQueue.cpp
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

foreach (FILE ${H_QT_FILES})
  set (src_files ${src_files} ${FILE})
  #
  unset (MOC_FILE)
  qt5_wrap_cpp(MOC_FILE ${FILE})
  message (STATUS "... Info: next MOC file ${MOC_FILE}")
  set (ui_moc_files ${ui_moc_files} ${MOC_FILE})
  #
  source_group ("Generated" FILES "${MOC_FILE}")
  source_group ("Header Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (exe_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};")
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
                      ${3RDPARTY_tcl_INCLUDE_DIR}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR}
                      ${3RDPARTY_tbb_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

#------------------------------------------------------------------------------
# Create executable
#------------------------------------------------------------------------------

add_executable(asiExeCli ${src_files})

# Set WinMain() as an entry point as we need a non-console app.
set_target_properties(asiExeCli PROPERTIES WIN32_EXECUTABLE TRUE)

#------------------------------------------------------------------------------
# Configure template
#------------------------------------------------------------------------------

set (X_COMPILER_BITNESS "x${COMPILER_BITNESS}")

configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/exePROTOTYPE.vcxproj.user.in
               ${asiExeCli_BINARY_DIR}/asiExeCli.vcxproj.user @ONLY)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

qt5_use_modules(asiExeCli Core)

target_link_libraries(asiExeCli asiTcl asiAlgo asiData asiVisu asiEngine asiUI)

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
    target_link_libraries (asiExeCli debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExeCli optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExeCli ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${VTK_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiExeCli debug ${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExeCli optimized ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExeCli ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a software
#------------------------------------------------------------------------------

if (WIN32)
  install (TARGETS asiExeCli CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiExeCli CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiExeCli CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
else()
  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin/asiExeCli DESTINATION bin
           CONFIGURATIONS Release
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bind/asiExeCli DESTINATION bin
           CONFIGURATIONS Debug
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bini/asiExeCli DESTINATION bin
           CONFIGURATIONS RelWithDebInfo
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_SOURCE_DIR}/asiExeCli.sh DESTINATION bin
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)
endif()
