project(asiAsm)

#------------------------------------------------------------------------------

set (H_FILES
  asiAsm.h
  asiAsm_Dict.h
)
set (CPP_FILES
)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (common_H_FILES
  common/asiAsm_FileFormat.h
  common/asiAsm_Utils.h
)
set (common_CPP_FILES
  common/asiAsm_FileFormat.cpp
  common/asiAsm_Utils.cpp
)

#------------------------------------------------------------------------------
# DAO
#------------------------------------------------------------------------------

set (dao_H_FILES
  dao/asiAsm_DaoBase.h
  dao/asiAsm_DaoPrototype.h
  dao/asiAsm_DaoSceneTree.h
)
set (dao_CPP_FILES
  dao/asiAsm_DaoBase.cpp
  dao/asiAsm_DaoPrototype.cpp
  dao/asiAsm_DaoSceneTree.cpp
)

#------------------------------------------------------------------------------
# Entity
#------------------------------------------------------------------------------

set (entity_H_FILES
  entity/asiAsm_EntBase.h
  entity/asiAsm_EntPrototype.h
)
set (entity_CPP_FILES
)

#------------------------------------------------------------------------------
# Service
#------------------------------------------------------------------------------

set (service_H_FILES
  service/asiAsm_ServiceBase.h
  service/asiAsm_ServiceModel.h
  service/asiAsm_ServicePopulateDatabase.h
  service/asiAsm_ServicePrototype.h
  service/asiAsm_ServiceSceneTree.h
)
set (service_CPP_FILES
  service/asiAsm_ServiceModel.cpp
  service/asiAsm_ServicePopulateDatabase.cpp
  service/asiAsm_ServicePrototype.cpp
  service/asiAsm_ServiceSceneTree.cpp
)

#------------------------------------------------------------------------------
# XDE
#------------------------------------------------------------------------------

set (xde_H_FILES
  xde/asiAsm_XdeDoc.h
  xde/asiAsm_XdeDocIterator.h
  xde/asiAsm_XdeGraph.h
  xde/asiAsm_XdePersistentIds.h
)
set (xde_CPP_FILES
  xde/asiAsm_XdeDoc.cpp
  xde/asiAsm_XdeDocIterator.cpp
  xde/asiAsm_XdeGraph.cpp
)

#------------------------------------------------------------------------------
# Active Data libraries
#------------------------------------------------------------------------------

set (AD_LIB_FILES
  ActiveDataAPI
  ActiveData
  ActiveDataAux
)

#------------------------------------------------------------------------------
# OpenCascade libraries
#------------------------------------------------------------------------------

set (OCCT_LIB_FILES
  TKernel
  TKMath
  TKBRep
  TKOffset
  TKTopAlgo
  TKG2d
  TKG3d
  TKGeomBase
  TKGeomAlgo
  TKMesh
  TKShHealing
  TKFeat
  TKBool
  TKBO
  TKPrim
  TKBin
  TKBinL
  TKBinXCAF
  TKLCAF
  TKCDF
  TKCAF
  TKXCAF
  TKService
  TKXSBase
  TKSTEP
  TKIGES
  TKXDESTEP
  TKXDEIGES
)

#------------------------------------------------------------------------------
# TBB libraries
#------------------------------------------------------------------------------

set (TBB_LIB_FILES
  tbb
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

foreach (FILE ${common_H_FILES})
  source_group ("Header Files\\Common" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${common_CPP_FILES})
  source_group ("Source Files\\Common" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${dao_H_FILES})
  source_group ("Header Files\\DAO" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${dao_CPP_FILES})
  source_group ("Source Files\\DAO" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${entity_H_FILES})
  source_group ("Header Files\\Entity" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${entity_CPP_FILES})
  source_group ("Source Files\\Entity" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${service_H_FILES})
  source_group ("Header Files\\Service" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${service_CPP_FILES})
  source_group ("Source Files\\Service" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${xde_H_FILES})
  source_group ("Header Files\\XDE" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${xde_CPP_FILES})
  source_group ("Source Files\\XDE" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (asiAsm_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};\
  ${CMAKE_CURRENT_SOURCE_DIR}/common;\
  ${CMAKE_CURRENT_SOURCE_DIR}/dao;\
  ${CMAKE_CURRENT_SOURCE_DIR}/entity;\
  ${CMAKE_CURRENT_SOURCE_DIR}/service;\
  ${CMAKE_CURRENT_SOURCE_DIR}/xde;\
")
#
set (asiAsm_include_dir ${asiAsm_include_dir_loc} PARENT_SCOPE)

include_directories ( ${asiAsm_include_dir_loc}
                      ${asiAlgo_include_dir}
                      ${asiData_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_asitus_INCLUDE_DIR}
                      ${3RDPARTY_active_data_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_tbb_INCLUDE_DIR} )

#------------------------------------------------------------------------------
# Create library
#------------------------------------------------------------------------------

add_library (asiAsm SHARED
  ${H_FILES}         ${CPP_FILES}
  ${common_H_FILES}  ${common_CPP_FILES}
  ${dao_H_FILES}     ${dao_CPP_FILES}
  ${entity_H_FILES}  ${entity_CPP_FILES}
  ${service_H_FILES} ${service_CPP_FILES}
  ${xde_H_FILES}     ${xde_CPP_FILES}
)

target_link_libraries(asiAsm asiAlgo asiData)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

qt5_use_modules(asiAsm Core Sql)

if (3RDPARTY_tbb_LIBRARY_DIR_DEBUG)
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR_DEBUG})
else()
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR})
endif()

foreach (LIB_FILE ${AD_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_active_data_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_active_data_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiAsm debug ${3RDPARTY_active_data_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiAsm optimized ${3RDPARTY_active_data_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiAsm ${3RDPARTY_active_data_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${OCCT_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiAsm debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiAsm optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiAsm ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${TBB_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set (LIB_FILENAME_DEBUG "${LIB_FILE}_debug${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (LIB_FILENAME_DEBUG "lib${LIB_FILE}_debug${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  target_link_libraries (asiAsm debug ${3RDPARTY_tbb_LIBRARY_DIR}/${LIB_FILENAME_DEBUG})
  target_link_libraries (asiAsm optimized ${3RDPARTY_tbb_LIBRARY_DIR}/${LIB_FILENAME})
endforeach()

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

install (TARGETS asiAsm RUNTIME DESTINATION bin COMPONENT Runtime)
