ASITUS_THIRDPARTY_PRODUCT("vtk" "" "vtkAlgorithm.h" "vtkCommonCore-8.2")

add_definitions (-DUSE_VTK)

if (3RDPARTY_vtk_INCLUDE_DIR STREQUAL "")
  message (STATUS "... VTK Include dir is not conventional")
  list (REMOVE_ITEM 3RDPARTY_NOT_INCLUDED 3RDPARTY_vtk_INCLUDE_DIR)
  set (3RDPARTY_vtk_INCLUDE_DIR ${3RDPARTY_vtk_DIR}/include/vtk-8.2 CACHE FILEPATH "Non-conventional inc dir" FORCE)
endif()

message (STATUS "... VTK Include dirs: ${3RDPARTY_vtk_INCLUDE_DIR}")
message (STATUS "... VTK Library dirs: ${3RDPARTY_vtk_LIBRARY_DIR}")
message (STATUS "... VTK Binary  dirs: ${3RDPARTY_vtk_DLL_DIR}")

string (REPLACE lib libd 3RDPARTY_vtk_LIBRARY_DIR_DEBUG ${3RDPARTY_vtk_LIBRARY_DIR})
if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}")
  if (WIN32)
    if (NOT EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/vtkCommonCore-8.2.lib")
      set (3RDPARTY_vtk_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  else()
    if (NOT EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/libvtkCommonCore-8.2.so")
      set (3RDPARTY_vtk_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

if (WIN32)
  string (REPLACE bin bind 3RDPARTY_vtk_DLL_DIR_DEBUG ${3RDPARTY_vtk_DLL_DIR})
  if (3RDPARTY_vtk_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}")
    if (NOT EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}/vtkCommonCore-8.2.dll")
      set (3RDPARTY_vtk_DLL_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

message (STATUS "... VTK Debug Library dirs: ${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}")
message (STATUS "... VTK Debug Binary  dirs: ${3RDPARTY_vtk_DLL_DIR_DEBUG}")

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

set (LIBS
  vtkChartsCore-8.2
  vtkCommonComputationalGeometry-8.2
  vtkCommonColor-8.2
  vtkCommonCore-8.2
  vtkCommonDataModel-8.2
  vtkCommonExecutionModel-8.2
  vtkCommonMath-8.2
  vtkCommonMisc-8.2
  vtkCommonSystem-8.2
  vtkCommonTransforms-8.2
  vtkFiltersCore-8.2
  vtkFiltersExtraction-8.2
  vtkFiltersGeneral-8.2
  vtkFiltersGeometry-8.2
  vtkFiltersHybrid-8.2
  vtkFiltersImaging-8.2
  vtkFiltersModeling-8.2
  vtkFiltersParallel-8.2
  vtkFiltersSources-8.2
  vtkFiltersStatistics-8.2
  vtkfreetype-8.2
  vtkGUISupportQt-8.2
  vtkglew-8.2
  vtkImagingColor-8.2
  vtkImagingCore-8.2
  vtkImagingFourier-8.2
  vtkImagingGeneral-8.2
  vtkImagingHybrid-8.2
  vtkImagingSources-8.2
  vtkInteractionWidgets-8.2
  vtkInteractionStyle-8.2
  vtkInfovisCore-8.2
  vtkInfovisLayout-8.2
  vtkIOCore-8.2
  vtkIOImage-8.2
  vtkIOLegacy-8.2
  vtkIOExport-8.2
  vtkIOXML-8.2
  vtkIOXMLParser-8.2
  vtkexpat-8.2
  vtkIOExportOpenGL2-8.2
  vtkParallelCore-8.2
  vtkRenderingAnnotation-8.2
  vtkRenderingContext2D-8.2
  vtkRenderingContextOpenGL2-8.2
  vtkRenderingCore-8.2
  vtkRenderingGL2PSOpenGL2-8.2
  vtkRenderingFreeType-8.2
  vtkRenderingLabel-8.2
  vtkRenderingOpenGL2-8.2
  vtkRenderingVolume-8.2
  vtksys-8.2
  vtkViewsContext2D-8.2
  vtkViewsCore-8.2
  vtkViewsInfovis-8.2
  vtkzlib-8.2
  vtklz4-8.2
  vtkgl2ps-8.2
  vtkpng-8.2
  vtklibharu-8.2
  vtkDICOMParser-8.2
  vtkmetaio-8.2
  vtktiff-8.2
  vtkjpeg-8.2
  vtklzma-8.2
  vtkdoubleconversion-8.2
)

ASITUS_INSTALL_3RDPARTY (LIBS "vtk" "" "1")
