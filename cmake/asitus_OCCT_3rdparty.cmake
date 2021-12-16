ASITUS_THIRDPARTY_PRODUCT("tcl" "" "tcl.h" "tcl86")

# TBB
set (INSTALL_TBB OFF CACHE INTERNAL "" FORCE)
if (USE_THREADING OR OpenCASCADE_WITH_TBB)
  set (INSTALL_TBB ON CACHE INTERNAL "" FORCE)

  ASITUS_THIRDPARTY_PRODUCT("tbb" "tbb" "tbb.h" "tbb")
  if (USE_THREADING)
    add_definitions (-DUSE_THREADING)
  endif()
else()
  ASITUS_UNSET_3RDPARTY("tbb")
endif()

# Freetype
ASITUS_THIRDPARTY_PRODUCT("freetype" "" "ft2build.h" "freetype")

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  if (WIN32)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/tcl86.dll CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/tcl86.dll CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/tcl86.dll CONFIGURATIONS Debug          DESTINATION bind)

    install (FILES ${3RDPARTY_tcl_DIR}/bin/zlib1.dll CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/zlib1.dll CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/zlib1.dll CONFIGURATIONS Debug          DESTINATION bind)

    # Freetype
    install (FILES ${3RDPARTY_freetype_DIR}/bin/freetype.dll CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_freetype_DIR}/bin/freetype.dll CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_freetype_DIR}/bin/freetype.dll CONFIGURATIONS Debug          DESTINATION bind)

    if (USE_THREADING)
      install (FILES ${3RDPARTY_tbb_DLL_DIR}/tbb.dll CONFIGURATIONS Release DESTINATION bin)
    endif()
  else()
    install (FILES ${3RDPARTY_tcl_LIBRARY_DIR}/libtcl8.6.so CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_tcl_LIBRARY_DIR}/libtcl8.6.so CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_tcl_LIBRARY_DIR}/libtcl8.6.so CONFIGURATIONS Debug          DESTINATION bind)
  endif()
endif()
