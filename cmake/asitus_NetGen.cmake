if (NOT USE_NETGEN)
  message (STATUS "Usetting NetGen 3-rd party")
  ASITUS_UNSET_3RDPARTY("netgen")
  return()
endif()

add_definitions (-DUSE_NETGEN)

ASITUS_THIRDPARTY_PRODUCT("netgen" "" "nglib.h" "ngcore")

if (3RDPARTY_netgen_INCLUDE_DIR STREQUAL "")
  list (REMOVE_ITEM 3RDPARTY_NOT_INCLUDED 3RDPARTY_netgen_INCLUDE_DIR)
  set (3RDPARTY_netgen_INCLUDE_DIR ${3RDPARTY_netgen_DIR}/include CACHE FILEPATH "Non-conventional inc dir" FORCE)
endif()

message (STATUS "... NetGen Include dirs: ${3RDPARTY_netgen_INCLUDE_DIR}")
message (STATUS "... NetGen Library dirs: ${3RDPARTY_netgen_LIBRARY_DIR}")
message (STATUS "... NetGen Binary  dirs: ${3RDPARTY_netgen_DLL_DIR}")

string (REPLACE lib libd 3RDPARTY_netgen_LIBRARY_DIR_DEBUG ${3RDPARTY_netgen_LIBRARY_DIR})
if (3RDPARTY_netgen_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_netgen_LIBRARY_DIR_DEBUG}")
  if (WIN32)
    if (NOT EXISTS "${3RDPARTY_netgen_LIBRARY_DIR_DEBUG}/ngcore.lib")
      set (3RDPARTY_netgen_LIBRARY_DIR_DEBUG "" CACHE INTERNAL FORCE)
    endif()
  else()
    if (NOT EXISTS "${3RDPARTY_netgen_LIBRARY_DIR_DEBUG}/libngcore.so")
      set (3RDPARTY_netgen_LIBRARY_DIR_DEBUG "" CACHE INTERNAL FORCE)
    endif()
  endif()
endif()

if (WIN32)
  string (REPLACE bin bind 3RDPARTY_netgen_DLL_DIR_DEBUG ${3RDPARTY_netgen_DLL_DIR})
  if (3RDPARTY_netgen_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_netgen_DLL_DIR_DEBUG}")
    if (NOT EXISTS "${3RDPARTY_netgen_DLL_DIR_DEBUG}/ngcore.dll")
      set (3RDPARTY_netgen_DLL_DIR_DEBUG "" CACHE INTERNAL FORCE)
    endif()
  endif()
endif()

message (STATUS "... netgen Debug Library dirs: ${3RDPARTY_netgen_LIBRARY_DIR_DEBUG}")
message (STATUS "... netgen Debug Binary  dirs: ${3RDPARTY_netgen_DLL_DIR_DEBUG}")

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

set (LIBS
  ngcore
  nglib
)

ASITUS_INSTALL_3RDPARTY (LIBS "netgen")
