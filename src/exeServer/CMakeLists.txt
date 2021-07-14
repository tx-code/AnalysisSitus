project(asiExeServer)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  exe_Keywords.h
)
set (H_QT_FILES
  exe_CommandServer.h
)
set (CPP_FILES
  exe_CommandServer.cpp
  exe_Main.cpp
)

#------------------------------------------------------------------------------
set (AD_LIB_FILES
  ActiveDataAPI
  ActiveData
  ActiveDataAux
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
  vtkChartsCore-8.2
  vtkCommonCore-8.2
  vtkCommonColor-8.2
  vtkCommonDataModel-8.2
  vtkCommonExecutionModel-8.2
  vtkCommonMath-8.2
  vtkCommonTransforms-8.2
  vtkCommonMisc-8.2
  vtkFiltersCore-8.2
  vtkFiltersGeneral-8.2
  vtkFiltersSources-8.2
  vtkFiltersGeometry-8.2
  vtkFiltersParallel-8.2
  vtkFiltersExtraction-8.2
  vtkFiltersModeling-8.2
  vtkGUISupportQt-8.2
  vtkInfovisLayout-8.2
  vtkIOCore-8.2
  vtkIOImage-8.2
  vtkIOExportOpenGL2-8.2
  vtkImagingCore-8.2
  vtkInteractionStyle-8.2
  vtkInteractionWidgets-8.2
  vtkRenderingAnnotation-8.2
  vtkRenderingContext2D-8.2
  vtkRenderingContextOpenGL2-8.2
  vtkRenderingCore-8.2
  vtkRenderingFreeType-8.2
  vtkRenderingOpenGL2-8.2
  vtkRenderingGL2PSOpenGL2-8.2
  vtkViewsContext2D-8.2
  vtkViewsInfovis-8.2
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
                      ${asiTcl_include_dir}
                      ${asiAlgo_include_dir}
                      ${asiData_include_dir}
                      ${asiVisu_include_dir}
                      ${asiEngine_include_dir}
                      ${asiUI_include_dir}
                      ${3RDPARTY_tcl_INCLUDE_DIR}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_active_data_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR}
                      ${3RDPARTY_tbb_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

#------------------------------------------------------------------------------
# Create executable
#------------------------------------------------------------------------------

add_executable(asiExeServer ${src_files} ${ui_moc_files})

# Set WinMain() as an entry point as we need a non-console app.
#set_target_properties(asiExeServer PROPERTIES WIN32_EXECUTABLE TRUE)

#------------------------------------------------------------------------------
# Configure template
#------------------------------------------------------------------------------

set (X_COMPILER_BITNESS "x${COMPILER_BITNESS}")

configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/exePROTOTYPE.vcxproj.user.in
               ${asiExeServer_BINARY_DIR}/asiExeServer.vcxproj.user @ONLY)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

qt5_use_modules(asiExeServer Core Network)

target_link_libraries(asiExeServer asiTcl asiAlgo asiData asiVisu asiEngine asiUI)

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
    target_link_libraries (asiExeServer debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExeServer optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExeServer ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${AD_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_active_data_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_active_data_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiExeServer debug ${3RDPARTY_active_data_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExeServer optimized ${3RDPARTY_active_data_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExeServer ${3RDPARTY_active_data_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${VTK_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiExeServer debug ${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiExeServer optimized ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiExeServer ${3RDPARTY_vtk_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a software
#------------------------------------------------------------------------------

if (WIN32)
  install (TARGETS asiExeServer CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiExeServer CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiExeServer CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
else()
  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin/asiExeServer DESTINATION bin
           CONFIGURATIONS Release
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bind/asiExeServer DESTINATION bin
           CONFIGURATIONS Debug
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bini/asiExeServer DESTINATION bin
           CONFIGURATIONS RelWithDebInfo
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)

  install (FILES ${CMAKE_SOURCE_DIR}/asiExeServer.sh DESTINATION bin
           PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                       GROUP_EXECUTE GROUP_READ
                       WORLD_EXECUTE WORLD_READ)
endif()
