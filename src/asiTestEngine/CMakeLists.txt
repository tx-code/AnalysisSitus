project (asiTestEngine)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  asiTestEngine.h
  asiTestEngine_Launcher.h
  asiTestEngine_FileComparator.h
  asiTestEngine_Macro.h
  asiTestEngine_ResultGroup.h
  asiTestEngine_Status.h
  asiTestEngine_TestCase.h
  asiTestEngine_Utils.h
  outcome.h
)

set (CPP_FILES
  asiTestEngine_FileComparator.cpp
  asiTestEngine_Launcher.cpp
  asiTestEngine_TestCase.cpp
  asiTestEngine_Utils.cpp
)

#------------------------------------------------------------------------------
# Report generator
#------------------------------------------------------------------------------

set (reportgen_H_FILES
  reportgen/asiTestEngine_DescriptionProc.h
  reportgen/asiTestEngine_ReportRenderer.h
  reportgen/asiTestEngine_ReportStyle.h
  reportgen/asiTestEngine_ReportStyleFactory.h
  reportgen/asiTestEngine_ReportTag.h
  reportgen/asiTestEngine_ReportTagFactory.h
)

set (reportgen_CPP_FILES
  reportgen/asiTestEngine_DescriptionProc.cpp
  reportgen/asiTestEngine_ReportRenderer.cpp
  reportgen/asiTestEngine_ReportStyle.cpp
  reportgen/asiTestEngine_ReportStyleFactory.cpp
  reportgen/asiTestEngine_ReportTag.cpp
  reportgen/asiTestEngine_ReportTagFactory.cpp
)

#------------------------------------------------------------------------------

# Create include variable
set (asiTestEngine_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/reportgen")
#
set (asiTestEngine_include_dir ${asiTestEngine_include_dir_loc} PARENT_SCOPE)

foreach (FILE ${H_FILES})
  source_group ("Header Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${CPP_FILES})
  source_group ("Source Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${reportgen_H_FILES})
  source_group ("Header Files\\Reporting" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${reportgen_CPP_FILES})
  source_group ("Source Files\\Reporting" FILES "${FILE}")
endforeach (FILE)

include_directories ( SYSTEM
                      ${asiTestEngine_include_dir_loc}
                      ${asiActiveData_include_dir}
                      ${asiData_include_dir_loc}
                      ${asiAlgo_include_dir}
                      ${asiTcl_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR} )

add_library (asiTestEngine SHARED
  ${H_FILES}           ${CPP_FILES}
  ${reportgen_H_FILES} ${reportgen_CPP_FILES})

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

target_link_libraries(asiTestEngine asiAlgo asiData asiTcl)

qt5_use_modules(asiTestEngine Core)

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  install (TARGETS asiTestEngine CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiTestEngine CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiTestEngine CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
endif()

install (TARGETS asiTestEngine
         CONFIGURATIONS Release
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bin COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library)

install (TARGETS asiTestEngine
         CONFIGURATIONS RelWithDebInfo
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bini COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library)

install (TARGETS asiTestEngine
         CONFIGURATIONS Debug
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bind COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library)

if (MSVC)
  install (FILES ${PROJECT_BINARY_DIR}/../../${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bind/asiTestEngine.pdb DESTINATION ${SDK_INSTALL_SUBDIR}bind CONFIGURATIONS Debug)
endif()

install (FILES ${H_FILES}           DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${reportgen_H_FILES} DESTINATION ${SDK_INSTALL_SUBDIR}include)
