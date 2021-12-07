ASITUS_THIRDPARTY_PRODUCT("OCCT" "" "Standard.hxx" "TKernel")

message (STATUS "... OCCT Include dirs: ${3RDPARTY_OCCT_INCLUDE_DIR}")
message (STATUS "... OCCT Library dirs: ${3RDPARTY_OCCT_LIBRARY_DIR}")
message (STATUS "... OCCT Binary  dirs: ${3RDPARTY_OCCT_DLL_DIR}")

string (REPLACE lib libd 3RDPARTY_OCCT_LIBRARY_DIR_DEBUG ${3RDPARTY_OCCT_LIBRARY_DIR})
if (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}")
  if (WIN32)
    if (NOT EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/TKernel.lib")
      set (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  else()
    if (NOT EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/libTKernel.so")
      set (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

if (WIN32)
  string (REPLACE bin bind 3RDPARTY_OCCT_DLL_DIR_DEBUG ${3RDPARTY_OCCT_DLL_DIR})
  if (3RDPARTY_OCCT_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_DLL_DIR_DEBUG}")
    if (NOT EXISTS "${3RDPARTY_OCCT_DLL_DIR_DEBUG}/TKernel.dll")
      set (3RDPARTY_OCCT_DLL_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

message (STATUS "... OCCT Debug Library dirs: ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}")
message (STATUS "... OCCT Debug Binary  dirs: ${3RDPARTY_OCCT_DLL_DIR_DEBUG}")

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

set (LIBS
  TKBin
  TKBinL
  TKBinXCAF
  TKBO
  TKBool
  TKBRep
  TKCAF
  TKCDF
  TKernel
  TKG2d
  TKG3d
  TKGeomAlgo
  TKGeomBase
  TKIGES
  TKLCAF
  TKMath
  TKMesh
  TKOffset
  TKPrim
  TKShHealing
  TKSTEP
  TKSTEP209
  TKSTEPAttr
  TKSTEPBase
  TKTopAlgo
  TKXSBase
  TKHLR
  TKFillet
  TKSTL
  TKXCAF
  TKXDESTEP
  TKXDEIGES
  TKVCAF
  TKV3d
  TKOpenGl
  TKService
)

if (NOT BUILD_ALGO_ONLY)
  ASITUS_INSTALL_3RDPARTY (LIBS "OCCT")
endif()
