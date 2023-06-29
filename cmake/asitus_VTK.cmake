ASITUS_THIRDPARTY_PRODUCT("vtk" "" "vtkAlgorithm.h" "vtkCommonCore")

add_definitions (-DUSE_VTK)

if (3RDPARTY_vtk_INCLUDE_DIR STREQUAL "")
  message (STATUS "... VTK Include dir is not conventional")
  list (REMOVE_ITEM 3RDPARTY_NOT_INCLUDED 3RDPARTY_vtk_INCLUDE_DIR)
  set (3RDPARTY_vtk_INCLUDE_DIR ${3RDPARTY_vtk_DIR}/include/vtk CACHE FILEPATH "Non-conventional inc dir" FORCE)
endif()

message (STATUS "... VTK Include dirs: ${3RDPARTY_vtk_INCLUDE_DIR}")
message (STATUS "... VTK Library dirs: ${3RDPARTY_vtk_LIBRARY_DIR}")
message (STATUS "... VTK Binary  dirs: ${3RDPARTY_vtk_DLL_DIR}")

string (REPLACE lib libd 3RDPARTY_vtk_LIBRARY_DIR_DEBUG ${3RDPARTY_vtk_LIBRARY_DIR})
if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}")
  if (WIN32)
    if (NOT EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/vtkCommonCore.lib")
      set (3RDPARTY_vtk_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  else()
    if (NOT EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/libvtkCommonCore.so")
      set (3RDPARTY_vtk_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

if (WIN32)
  string (REPLACE bin bind 3RDPARTY_vtk_DLL_DIR_DEBUG ${3RDPARTY_vtk_DLL_DIR})
  if (3RDPARTY_vtk_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}")
    if (NOT EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}/vtkCommonCore.dll")
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
  vtkChartsCore
  vtkCommonComputationalGeometry
  vtkCommonColor
  vtkCommonCore
  vtkCommonDataModel
  vtkCommonExecutionModel
  vtkCommonMath
  vtkCommonMisc
  vtkCommonSystem
  vtkCommonTransforms
  vtkFiltersCore
  vtkFiltersExtraction
  vtkFiltersGeneral
  vtkFiltersGeometry
  vtkFiltersHybrid
  vtkFiltersImaging
  vtkFiltersModeling
  vtkFiltersParallel
  vtkFiltersSources
  vtkFiltersStatistics
  vtkfreetype
  vtkGUISupportQt
  vtkglew
  vtkImagingColor
  vtkImagingCore
  vtkImagingFourier
  vtkImagingGeneral
  vtkImagingHybrid
  vtkImagingSources
  vtkInteractionWidgets
  vtkInteractionStyle
  vtkInfovisCore
  vtkInfovisLayout
  vtkIOCore
  vtkIOImage
  vtkIOLegacy
  vtkIOExport
  vtkIOXML
  vtkIOXMLParser
  vtkexpat
  vtkIOExportOpenGL2
  vtkParallelCore
  vtkRenderingAnnotation
  vtkRenderingContext2D
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingGL2PSOpenGL2
  vtkRenderingFreeType
  vtkRenderingLabel
  vtkRenderingOpenGL2
  vtkRenderingVolume
  vtksys
  vtkViewsContext2D
  vtkViewsCore
  vtkViewsInfovis
  vtkzlib
  vtklz4
  vtkgl2ps
  vtkpng
  vtklibharu
  vtkDICOMParser
  vtkmetaio
  vtktiff
  vtkjpeg
  vtklzma
  vtkdoubleconversion
)

ASITUS_INSTALL_3RDPARTY (LIBS "vtk" "" "1")
