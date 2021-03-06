if (NOT USE_FBX_SDK)
  ASITUS_UNSET_3RDPARTY("FBX_SDK")
  return()
endif()

set (SubPathLib "")

if (NOT WIN32)
  set (SubPathLib lib/gcc4/x64/release)
endif()

ASITUS_THIRDPARTY_PRODUCT("FBX_SDK" "include" "fbxsdk.h" "libfbxsdk" "${SubPathLib}")

message (STATUS "... FBX_SDK Include dirs: ${3RDPARTY_FBX_SDK_INCLUDE_DIR}")
message (STATUS "... FBX_SDK Library dirs: ${3RDPARTY_FBX_SDK_LIBRARY_DIR}")
message (STATUS "... FBX_SDK Binary  dirs: ${3RDPARTY_FBX_SDK_DLL_DIR}")

add_definitions (-DFBXSDK_SHARED)

if (WIN32)
  GENERATE_3RDPARTY_DEBUG_VARIABLES("FBX_SDK" "libfbxsdk")
else()
  find_path (3RDPARTY_FBX_SDK_LIBRARY_DIR_DEBUG NAMES "libfbxsdk.so"
                                                PATHS ${3RDPARTY_FBX_SDK_DIR}
                                                PATH_SUFFIXES "lib/gcc4/x64/debug"
                                                CMAKE_FIND_ROOT_PATH_BOTH
                                                NO_DEFAULT_PATH)

  if (EXISTS "${3RDPARTY_FBX_SDK_LIBRARY_DIR_DEBUG}")
    if (NOT EXISTS "${3RDPARTY_FBX_SDK_LIBRARY_DIR_DEBUG}/libfbxsdk.so")
      set (3RDPARTY_FBX_SDK_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

message (STATUS "... FBX_SDK Debug Library dirs: ${3RDPARTY_FBX_SDK_LIBRARY_DIR_DEBUG}")
message (STATUS "... FBX_SDK Debug Binary  dirs: ${3RDPARTY_FBX_SDK_DLL_DIR_DEBUG}")

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

if (WIN32)
  set (LIBS
    libfbxsdk
  )
else()
  set (LIBS
    fbxsdk
  )
endif()

ASITUS_INSTALL_3RDPARTY (LIBS "FBX_SDK")
