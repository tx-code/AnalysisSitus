cmake_minimum_required (VERSION 3.0.0 FATAL_ERROR)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project name
project (AnalysisSitus CXX)

# Install dir of the built project
set (INSTALL_DIR "" CACHE PATH "Path where to install Analysis Situs.")
set (CMAKE_INSTALL_PREFIX "${INSTALL_DIR}" CACHE INTERNAL "" FORCE)

# Directories for embedded unit tests.
set (ASI_TEST_DUMPING "${CMAKE_BINARY_DIR}" CACHE PATH "Dumping directory for tests.")
set (ASI_TEST_DATA    "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Directory which contains test data.")
set (ASI_TEST_SCRIPTS "${CMAKE_SOURCE_DIR}/scripts" CACHE PATH "Directory which contains test scripts.")
set (ASI_TEST_DESCR   "${CMAKE_SOURCE_DIR}/src/asiTest/cases" CACHE PATH "Directory which contains descriptions of tests.")
set (ASI_PLUGINS_DIR  "${CMAKE_BINARY_DIR}/asi-plugins" CACHE PATH "The directory where to search for plugins.")

# Build configurations
set (CMAKE_CONFIGURATION_TYPES Release Debug RelWithDebInfo CACHE INTERNAL "" FORCE)

#-------------------------------------------------------------------------------
# Prepare finding 3rd-parties
#-------------------------------------------------------------------------------

include (${CMAKE_SOURCE_DIR}/cmake/asitus_3rdparty_macros.cmake)

ASITUS_RETRIEVE_VERSION(${CMAKE_SOURCE_DIR}/src/asiAlgo/asiAlgo_Version.h VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

message (STATUS "Analysis Situs VERSION_MAJOR: ${VERSION_MAJOR}")
message (STATUS "Analysis Situs VERSION_MINOR: ${VERSION_MINOR}")
message (STATUS "Analysis Situs VERSION_PATCH: ${VERSION_PATCH}")

#------------------------------------------------------------------------------
# Distribution parameters
#------------------------------------------------------------------------------

# set type of application distribution
if (NOT DISTRIBUTION_TYPE)
  set (DISTRIBUTION_TYPE "Complete" CACHE STRING "Select distribution type." FORCE)
  set_property(CACHE DISTRIBUTION_TYPE PROPERTY STRINGS "Complete" "Algo")
endif()

if ( "${DISTRIBUTION_TYPE}" STREQUAL "Algo" )
  set (BUILD_ALGO_ONLY ON CACHE INTERNAL "" FORCE)
  add_definitions (-DBUILD_ALGO_ONLY)
  set (SDK_INSTALL_SUBDIR "" CACHE INTERNAL "" FORCE)
else()
  set (BUILD_ALGO_ONLY OFF CACHE INTERNAL "" FORCE)
  remove_definitions (-DBUILD_ALGO_ONLY)
  set (SDK_INSTALL_SUBDIR "framework/" CACHE INTERNAL "" FORCE)
endif()

#-------------------------------------------------------------------------------
# Find 3rd-parties
#-------------------------------------------------------------------------------

# Set a directory containing all 3-rd parties
set (3RDPARTY_DIR "" CACHE PATH "The root directory for all 3-rd parties.")

if (NOT DEFINED ${3RDPARTY_DIR} AND ${3RDPARTY_DIR} STREQUAL "")
  message (FATAL_ERROR "... root 3rdparty directory was not found.\nPlease, set 3RDPARTY_DIR variable.")
endif()

# Eigen
include (${CMAKE_SOURCE_DIR}/cmake/asitus_Eigen.cmake)

# Rapidjson
set (USE_RAPIDJSON OFF CACHE BOOL "Rapidjson is optional 3-rd party. It enables glTF export.")
#
include (${CMAKE_SOURCE_DIR}/cmake/asitus_Rapidjson.cmake)

# OCCT
include (${CMAKE_SOURCE_DIR}/cmake/asitus_OCCT.cmake)

# TBB is optional
set (USE_THREADING ON CACHE BOOL "TBB is optional 3-rd party. Building with TBB will allow to run algorithms in multi-threaded mode.")

# Mobius is optional
set (USE_MOBIUS OFF CACHE BOOL "Mobius is optional 3-rd party. It gives alternative tools for CAGD.")
#
include (${CMAKE_SOURCE_DIR}/cmake/asitus_Mobius.cmake)

# NetGen
set (USE_NETGEN OFF CACHE BOOL "NetGen mesh generator.")
#
include (${CMAKE_SOURCE_DIR}/cmake/asitus_NetGen.cmake)

# FBX SDK is optional
set (USE_FBX_SDK OFF CACHE BOOL "FBX SDK is optional 3-rd party. Building with FBX SDK will allow to export assemblies into FBX file format.")

# FBX SDK for processing *.fbx files
include (${CMAKE_SOURCE_DIR}/cmake/asitus_FBX_SDK.cmake)

# 3-rd parties for complete distribution of Analysis Situs.
if (NOT BUILD_ALGO_ONLY)
  # Qt
  include (${CMAKE_SOURCE_DIR}/cmake/asitus_Qt.cmake)

  # OCCT 3rdparty
  include (${CMAKE_SOURCE_DIR}/cmake/asitus_OCCT_3rdparty.cmake)

  # VTK
  include (${CMAKE_SOURCE_DIR}/cmake/asitus_VTK.cmake)

  # Inno Setup for installation
  include (${CMAKE_SOURCE_DIR}/cmake/asitus_InnoSetup.cmake)

  # Instant Meshes is optional
  set (USE_INSTANT_MESHES OFF CACHE BOOL "Instant Meshes are optionally employed for auto retopologizing of meshes to quads.")
  #
  if (USE_INSTANT_MESHES)
    set (INSTANT_MESHES_EXE "" CACHE FILEPATH "Full filename of Instant Meshes executable.")

    if (NOT INSTANT_MESHES_EXE)
      message (FATAL_ERROR "... Please, specify full filename for Instant Meshes executable.")
    else()
      if (NOT EXISTS "${INSTANT_MESHES_EXE}")
        message (FATAL_ERROR "... The specified Instant Meshes executable does not exist.")
      endif()
    endif()

    add_definitions (-DUSE_INSTANT_MESHES)
  else()
    unset (INSTANT_MESHES_EXE CACHE)
  endif()
endif()

# Set paths to dynamically linked libs
set (3RDPARTY_DLL_PATH "${3RDPARTY_tcl_DLL_DIR};\
${3RDPARTY_tbb_DLL_DIR};\
${3RDPARTY_freetype_DLL_DIR};\
${3RDPARTY_QT_DIR}/bin;")

set (3RDPARTY_DLL_DEB_PATH "${3RDPARTY_DLL_PATH}")

if (3RDPARTY_OCCT_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_DLL_DIR_DEBUG}")
  set (3RDPARTY_DLL_DEB_PATH "${3RDPARTY_DLL_DEB_PATH};${3RDPARTY_OCCT_DLL_DIR_DEBUG}")
endif()

if (3RDPARTY_vtk_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}")
  set (3RDPARTY_DLL_DEB_PATH "${3RDPARTY_DLL_DEB_PATH};${3RDPARTY_vtk_DLL_DIR_DEBUG}")
endif()

if (USE_MOBIUS)
  if (3RDPARTY_mobius_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_mobius_DLL_DIR_DEBUG}")
    set (3RDPARTY_DLL_DEB_PATH "${3RDPARTY_DLL_DEB_PATH};${3RDPARTY_mobius_DLL_DIR_DEBUG}")
  endif()

  set (3RDPARTY_DLL_PATH "${3RDPARTY_DLL_PATH};${3RDPARTY_mobius_DLL_DIR}")
endif()

if (USE_NETGEN)
  if (3RDPARTY_netgen_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_netgen_DLL_DIR_DEBUG}")
    set (3RDPARTY_DLL_DEB_PATH "${3RDPARTY_DLL_DEB_PATH};${3RDPARTY_netgen_DLL_DIR_DEBUG}")
  endif()

  set (3RDPARTY_DLL_PATH "${3RDPARTY_DLL_PATH};${3RDPARTY_netgen_DLL_DIR}")
endif()

if (USE_FBX_SDK)
  if (3RDPARTY_FBX_SDK_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_FBX_SDK_DLL_DIR_DEBUG}")
    set (3RDPARTY_DLL_DEB_PATH "${3RDPARTY_DLL_DEB_PATH};${3RDPARTY_FBX_SDK_DLL_DIR_DEBUG}")
  endif()

  set (3RDPARTY_DLL_PATH "${3RDPARTY_DLL_PATH};${3RDPARTY_FBX_SDK_DLL_DIR}")
endif()

set (3RDPARTY_DLL_PATH "${3RDPARTY_DLL_PATH};\
${3RDPARTY_OCCT_DLL_DIR};\
${3RDPARTY_vtk_DLL_DIR};")

#-------------------------------------------------------------------------------
string (REGEX REPLACE ";" " " 3RDPARTY_NOT_INCLUDED "${3RDPARTY_NOT_INCLUDED}")

# check all 3rdparty paths
if (3RDPARTY_NOT_INCLUDED)
  message (FATAL_ERROR "NOT FOUND: ${3RDPARTY_NOT_INCLUDED}")
endif()

#-------------------------------------------------------------------------------
# Configure compiler
#-------------------------------------------------------------------------------

# Configure warnings level
if (MSVC)
  add_definitions (/W4)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHa")
elseif (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions (-Wall -pedantic -Wno-unknown-pragmas)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")  
else()
  message ("Unknown compiler")
endif()

#-------------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------------

# Build directories
set (OS_WITH_BIT "${PLATFORM}${COMPILER_BITNESS}")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/lib")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/libi")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bini")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/libi")

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/libd")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bind")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/libd")

if (WIN32)
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bini")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bind")
endif()

#-------------------------------------------------------------------------------
# Sub-projects
#-------------------------------------------------------------------------------

add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiActiveData)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiAlgo)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiAsm)

if (NOT BUILD_ALGO_ONLY)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiTcl)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiData)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiVisu)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiEngine)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiUI)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiTestEngine)

  if (WIN32)
    if (MSVC)
      add_subdirectory(${CMAKE_SOURCE_DIR}/src/exeCli)
      add_subdirectory(${CMAKE_SOURCE_DIR}/src/exeServer)
      add_subdirectory(${CMAKE_SOURCE_DIR}/src/asiTest)
    endif()
  endif()

  add_subdirectory(${CMAKE_SOURCE_DIR}/src/exe)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmdAsm)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmdDDF)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmdEngine)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmdMisc)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmdRE)
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmdMobius)

  # Setup compiler
  if (NOT "${INNOSETUP_EXE}" STREQUAL "" AND EXISTS "${INNOSETUP_EXE}")
    add_subdirectory(${CMAKE_SOURCE_DIR}/setup)
  endif()

endif()

#-------------------------------------------------------------------------------
# IDE folders
#-------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  set_property(TARGET asiAlgo       PROPERTY FOLDER Framework)
  set_property(TARGET asiAsm        PROPERTY FOLDER Framework)
  set_property(TARGET asiActiveData PROPERTY FOLDER Framework)
  set_property(TARGET asiData       PROPERTY FOLDER Framework)
  set_property(TARGET asiVisu       PROPERTY FOLDER Framework)
  set_property(TARGET asiEngine     PROPERTY FOLDER Framework)
  set_property(TARGET asiUI         PROPERTY FOLDER Framework)
  set_property(TARGET asiTestEngine PROPERTY FOLDER Framework)
  set_property(TARGET asiExe        PROPERTY FOLDER Executables)

  if (WIN32)
    if (MSVC)
      set_property(TARGET asiExeCli    PROPERTY FOLDER Executables)
      set_property(TARGET asiExeServer PROPERTY FOLDER Executables)
      set_property(TARGET asiTest      PROPERTY FOLDER Executables)
    endif()
  endif()

  if (NOT "${INNOSETUP_EXE}" STREQUAL "" AND EXISTS "${INNOSETUP_EXE}")
    set_property(TARGET setup PROPERTY FOLDER Utilities)
  endif()

  set_property(TARGET asiTcl    PROPERTY FOLDER Batch)
  set_property(TARGET cmdAsm    PROPERTY FOLDER Commands)
  set_property(TARGET cmdDDF    PROPERTY FOLDER Commands)
  set_property(TARGET cmdEngine PROPERTY FOLDER Commands)
  set_property(TARGET cmdMisc   PROPERTY FOLDER Commands)
  set_property(TARGET cmdRE     PROPERTY FOLDER Commands)
  set_property(TARGET cmdMobius PROPERTY FOLDER Commands)

  set_property (GLOBAL PROPERTY USE_FOLDERS ON)
endif()

#-------------------------------------------------------------------------------
# Installation
#-------------------------------------------------------------------------------

#install (DIRECTORY data DESTINATION bin)

if (NOT BUILD_ALGO_ONLY)
  if (EXISTS "${3RDPARTY_DIR}/Redistributable/msvcp120.dll")
    install (FILES "${3RDPARTY_DIR}/Redistributable/msvcp120.dll" DESTINATION bin)
  endif()

  if (EXISTS "${3RDPARTY_DIR}/Redistributable/msvcr120.dll")
    install (FILES "${3RDPARTY_DIR}/Redistributable/msvcr120.dll" DESTINATION bin)
  endif()
endif()
