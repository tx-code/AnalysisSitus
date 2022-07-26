project(cmdTest)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  cmdTest.h
)
set (CPP_FILES
  cmdTest.cpp
  cmdTest_Mesh.cpp
  cmdTest_Shape.cpp
)

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
set (cmdTest_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};")
#
set (cmdTest_include_dir ${cmdTest_include_dir_loc} PARENT_SCOPE)

include_directories ( SYSTEM
                      ${cmdTest_include_dir_loc}
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

add_library (cmdTest SHARED
  ${H_FILES} ${CPP_FILES}
)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

target_link_libraries(cmdTest asiTcl asiAlgo asiAsm asiData asiVisu asiEngine asiUI)

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a software
#------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  install (TARGETS cmdTest CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS cmdTest CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS cmdTest CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a framework
#------------------------------------------------------------------------------

install (TARGETS cmdTest
         CONFIGURATIONS Release
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bin COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library)

install (TARGETS cmdTest
         CONFIGURATIONS RelWithDebInfo
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bini COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library)

install (TARGETS cmdTest
         CONFIGURATIONS Debug
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bind COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library)

if (MSVC)
  install (FILES ${PROJECT_BINARY_DIR}/../../${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bind/cmdTest.pdb DESTINATION ${SDK_INSTALL_SUBDIR}bind CONFIGURATIONS Debug)
endif()

install (FILES ${H_FILES} DESTINATION ${SDK_INSTALL_SUBDIR}include)
