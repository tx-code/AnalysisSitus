project(cmdMobius)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  cmdMobius.h
  cmdMobius_Mesh.h
)
set (CPP_FILES
  cmdMobius.cpp
  cmdMobius_Mesh.cpp
)

#------------------------------------------------------------------------------
# Mobius libraries
#------------------------------------------------------------------------------

if (USE_MOBIUS)
  set (MOBIUS_LIB_FILES
    mobiusBSpl
    mobiusCascade
    mobiusCore
    mobiusGeom
    mobiusPoly
  )
endif()

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

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (cmdMobius_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};")
#
set (cmdMobius_include_dir ${cmdMobius_include_dir_loc} PARENT_SCOPE)

include_directories ( SYSTEM
                      ${cmdMobius_include_dir_loc}
                      ${asiActiveData_include_dir}
                      ${asiTcl_include_dir}
                      ${asiAlgo_include_dir}
                      ${asiAsm_include_dir}
                      ${asiData_include_dir}
                      ${asiVisu_include_dir}
                      ${asiEngine_include_dir}
                      ${asiUI_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

#------------------------------------------------------------------------------
# Create library
#------------------------------------------------------------------------------

add_library (cmdMobius SHARED
  ${H_FILES} ${CPP_FILES}
)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

target_link_libraries(cmdMobius asiTcl asiAlgo asiData asiVisu asiEngine asiUI)

if (USE_MOBIUS)
  foreach (LIB_FILE ${MOBIUS_LIB_FILES})
    if (WIN32)
      set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
      set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()

    if (3RDPARTY_mobius_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_mobius_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
      target_link_libraries (cmdMobius debug ${3RDPARTY_mobius_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
      target_link_libraries (cmdMobius optimized ${3RDPARTY_mobius_LIBRARY_DIR}/${LIB_FILENAME})
    else()
      target_link_libraries (cmdMobius ${3RDPARTY_mobius_LIBRARY_DIR}/${LIB_FILENAME})
    endif()
  endforeach()
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a software
#------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  install (TARGETS cmdMobius CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS cmdMobius CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS cmdMobius CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a framework
#------------------------------------------------------------------------------

install (TARGETS cmdMobius
         CONFIGURATIONS Release
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bin COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library)

install (TARGETS cmdMobius
         CONFIGURATIONS RelWithDebInfo
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bini COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library)

install (TARGETS cmdMobius
         CONFIGURATIONS Debug
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bind COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library)

if (MSVC)
  install (FILES ${PROJECT_BINARY_DIR}/../../${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bind/cmdMobius.pdb DESTINATION ${SDK_INSTALL_SUBDIR}bind CONFIGURATIONS Debug)
endif()
