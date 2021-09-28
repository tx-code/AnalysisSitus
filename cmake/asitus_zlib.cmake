#--------------------------------------------------------------------------
# zlib
#--------------------------------------------------------------------------

if (WIN32)
  ASITUS_THIRDPARTY_PRODUCT("zlib" "" "zlib.h" "zlib" "")
else()
  ASITUS_THIRDPARTY_PRODUCT("zlib" "" "zlib.h" "libz" "")
endif()

message (STATUS "... zlib Include dirs: ${3RDPARTY_zlib_INCLUDE_DIR}")
message (STATUS "... zlib Library dirs: ${3RDPARTY_zlib_LIBRARY_DIR}")
message (STATUS "... zlib Binary  dirs: ${3RDPARTY_zlib_DLL_DIR}")

set (3RDPARTY_zlib_LIBRARY_DIR_DEBUG "${3RDPARTY_zlib_LIBRARY_DIR}" CACHE INTERNAL "" FORCE)
set (3RDPARTY_zlib_DLL_DIR_DEBUG "${3RDPARTY_zlib_DLL_DIR}" CACHE INTERNAL "" FORCE)

message (STATUS "... zlib Debug Library dirs: ${3RDPARTY_zlib_LIBRARY_DIR_DEBUG}")
message (STATUS "... zlib Debug Binary  dirs: ${3RDPARTY_zlib_DLL_DIR_DEBUG}")

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

if (WIN32)
  set (LIBS
    zlib
  )

  ASITUS_INSTALL_3RDPARTY (LIBS "zlib")
endif()
