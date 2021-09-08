project(asiActiveData)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  ActData.h
  ActData_Common.h
  ActData_Plugin.h
)

set (CPP_FILES
  ActData_Common.cpp
  ActData_Plugin.cpp
)

#------------------------------------------------------------------------------
# API
#------------------------------------------------------------------------------

set (api_H_FILES
  API/ActAPI_Common.h
  API/ActAPI_IAlgorithm.h
  API/ActAPI_IDataCursor.h
  API/ActAPI_ILogger.h
  API/ActAPI_IModel.h
  API/ActAPI_INode.h
  API/ActAPI_IParameter.h
  API/ActAPI_IPartition.h
  API/ActAPI_IPlotter.h
  API/ActAPI_IProgressNotifier.h
  API/ActAPI_ITreeFunction.h
  API/ActAPI_TxData.h
  API/ActAPI_TxRes.h
  API/ActAPI_Variables.h
  API/ActAPI_Version.h
)
set (api_CPP_FILES 
  API/ActAPI_IAlgorithm.cpp
  API/ActAPI_IDataCursor.cpp
  API/ActAPI_ILogger.cpp
  API/ActAPI_IModel.cpp
  API/ActAPI_INode.cpp
  API/ActAPI_IParameter.cpp
  API/ActAPI_IPartition.cpp
  API/ActAPI_IPlotter.cpp
  API/ActAPI_IProgressNotifier.cpp
  API/ActAPI_ITreeFunction.cpp
  API/ActAPI_Variables.cpp
)

#------------------------------------------------------------------------------
# OCAF drivers for binary file format
#------------------------------------------------------------------------------

set (drivers_H_FILES
  BinDrivers/ActData_BinDrivers.h
  BinDrivers/ActData_BinRetrievalDriver.h
  BinDrivers/ActData_BinStorageDriver.h
  BinDrivers/ActData_MeshDriver.h
)
set (drivers_CPP_FILES 
  BinDrivers/ActData_BinDrivers.cpp
  BinDrivers/ActData_BinRetrievalDriver.cpp
  BinDrivers/ActData_BinStorageDriver.cpp
  BinDrivers/ActData_MeshDriver.cpp
)

#------------------------------------------------------------------------------
# Kernel
#------------------------------------------------------------------------------

set (kernel_H_FILES
  Kernel/ActData_Application.h
  Kernel/ActData_AsciiStringParameter.h
  Kernel/ActData_BaseModel.h
  Kernel/ActData_BaseNode.h
  Kernel/ActData_BasePartition.h
  Kernel/ActData_BaseTreeFunction.h
  Kernel/ActData_BoolArrayParameter.h
  Kernel/ActData_BoolParameter.h
  Kernel/ActData_ComplexArrayParameter.h
  Kernel/ActData_CopyPasteEngine.h
  Kernel/ActData_DependencyAnalyzer.h
  Kernel/ActData_DependencyGraph.h
  Kernel/ActData_DependencyGraphIterator.h
  Kernel/ActData_ExtTransactionEngine.h
  Kernel/ActData_FuncExecutionCtx.h
  Kernel/ActData_FuncExecutionTask.h
  Kernel/ActData_GraphFrozenException.h
  Kernel/ActData_GroupParameter.h
  Kernel/ActData_IntArrayParameter.h
  Kernel/ActData_IntParameter.h
  Kernel/ActData_LogBook.h
  Kernel/ActData_LogBookAttr.h
  Kernel/ActData_MeshParameter.h
  Kernel/ActData_MetaParameter.h
  Kernel/ActData_NameParameter.h
  Kernel/ActData_NodeFactory.h
  Kernel/ActData_ParameterDTO.h
  Kernel/ActData_ParameterFactory.h
  Kernel/ActData_RealArrayParameter.h
  Kernel/ActData_RealParameter.h
  Kernel/ActData_RefClassifier.h
  Kernel/ActData_ReferenceListParameter.h
  Kernel/ActData_ReferenceParameter.h
  Kernel/ActData_SamplerTreeNode.h
  Kernel/ActData_SelectionParameter.h
  Kernel/ActData_SequentialFuncIterator.h
  Kernel/ActData_ShapeParameter.h
  Kernel/ActData_StringArrayParameter.h
  Kernel/ActData_TimeStampParameter.h
  Kernel/ActData_TransactionEngine.h
  Kernel/ActData_TreeFunctionParameter.h
  Kernel/ActData_TreeFunctionPriority.h
  Kernel/ActData_TreeNodeParameter.h
  Kernel/ActData_TriangulationParameter.h
  Kernel/ActData_UserExtParameter.h
  Kernel/ActData_UserParameter.h
  Kernel/ActData_Utils.h
)
set (kernel_CPP_FILES 
  Kernel/ActData_Application.cpp
  Kernel/ActData_AsciiStringParameter.cpp
  Kernel/ActData_BaseModel.cpp
  Kernel/ActData_BaseNode.cpp
  Kernel/ActData_BasePartition.cpp
  Kernel/ActData_BaseTreeFunction.cpp
  Kernel/ActData_BoolArrayParameter.cpp
  Kernel/ActData_BoolParameter.cpp
  Kernel/ActData_ComplexArrayParameter.cpp
  Kernel/ActData_CopyPasteEngine.cpp
  Kernel/ActData_DependencyAnalyzer.cpp
  Kernel/ActData_DependencyGraph.cpp
  Kernel/ActData_DependencyGraphIterator.cpp
  Kernel/ActData_ExtTransactionEngine.cpp
  Kernel/ActData_FuncExecutionCtx.cpp
  Kernel/ActData_FuncExecutionTask.cpp
  Kernel/ActData_GroupParameter.cpp
  Kernel/ActData_IntArrayParameter.cpp
  Kernel/ActData_IntParameter.cpp
  Kernel/ActData_LogBook.cpp
  Kernel/ActData_LogBookAttr.cpp
  Kernel/ActData_MeshParameter.cpp
  Kernel/ActData_MetaParameter.cpp
  Kernel/ActData_NameParameter.cpp
  Kernel/ActData_NodeFactory.cpp
  Kernel/ActData_ParameterDTO.cpp
  Kernel/ActData_ParameterFactory.cpp
  Kernel/ActData_RealArrayParameter.cpp
  Kernel/ActData_RealParameter.cpp
  Kernel/ActData_RefClassifier.cpp
  Kernel/ActData_ReferenceListParameter.cpp
  Kernel/ActData_ReferenceParameter.cpp
  Kernel/ActData_SamplerTreeNode.cpp
  Kernel/ActData_SelectionParameter.cpp
  Kernel/ActData_SequentialFuncIterator.cpp
  Kernel/ActData_ShapeParameter.cpp
  Kernel/ActData_StringArrayParameter.cpp
  Kernel/ActData_TimeStampParameter.cpp
  Kernel/ActData_TransactionEngine.cpp
  Kernel/ActData_TreeFunctionParameter.cpp
  Kernel/ActData_TreeNodeParameter.cpp
  Kernel/ActData_TriangulationParameter.cpp
  Kernel/ActData_UserExtParameter.cpp
  Kernel/ActData_UserParameter.cpp
  Kernel/ActData_Utils.cpp
)

#------------------------------------------------------------------------------
# Mesh
#------------------------------------------------------------------------------

set (mesh_H_FILES
  Mesh/ActData_MeshAttr.h
  Mesh/ActData_MeshDeltaEntities.h
  Mesh/ActData_MeshMDelta.h
)

set (mesh_CPP_FILES 
  Mesh/ActData_MeshAttr.cpp
  Mesh/ActData_MeshDeltaEntities.cpp
  Mesh/ActData_MeshMDelta.cpp
)

#------------------------------------------------------------------------------
# Mesh data structures
#------------------------------------------------------------------------------

set (meshds_H_FILES
  Mesh/DS/ActData_Mesh.h
  Mesh/DS/ActData_Mesh_Direction.h
  Mesh/DS/ActData_Mesh_Edge.h
  Mesh/DS/ActData_Mesh_EdgesIterator.h
  Mesh/DS/ActData_Mesh_Element.h
  Mesh/DS/ActData_Mesh_ElementsIterator.h
  Mesh/DS/ActData_Mesh_ElementType.h
  Mesh/DS/ActData_Mesh_Face.h
  Mesh/DS/ActData_Mesh_FacesIterator.h
  Mesh/DS/ActData_Mesh_Group.h
  Mesh/DS/ActData_Mesh_IDFactory.h
  Mesh/DS/ActData_Mesh_MapOfMeshElement.h
  Mesh/DS/ActData_Mesh_MapOfMeshOrientedElement.h
  Mesh/DS/ActData_Mesh_Node.h
  Mesh/DS/ActData_Mesh_NodesIterator.h
  Mesh/DS/ActData_Mesh_Object.h
  Mesh/DS/ActData_Mesh_PntHasher.h
  Mesh/DS/ActData_Mesh_Position.h
  Mesh/DS/ActData_Mesh_Quadrangle.h
  Mesh/DS/ActData_Mesh_SpacePosition.h
  Mesh/DS/ActData_Mesh_Triangle.h
  Mesh/DS/ActData_Mesh_TypeOfPosition.h
)

set (meshds_CPP_FILES 
  Mesh/DS/ActData_Mesh.cpp
  Mesh/DS/ActData_Mesh_Direction.cpp
  Mesh/DS/ActData_Mesh_Edge.cpp
  Mesh/DS/ActData_Mesh_Element.cpp
  Mesh/DS/ActData_Mesh_ElementsIterator.cpp
  Mesh/DS/ActData_Mesh_Face.cpp
  Mesh/DS/ActData_Mesh_Group.cpp
  Mesh/DS/ActData_Mesh_IDFactory.cpp
  Mesh/DS/ActData_Mesh_MapOfMeshElement.cpp
  Mesh/DS/ActData_Mesh_MapOfMeshOrientedElement.cpp
  Mesh/DS/ActData_Mesh_Node.cpp
  Mesh/DS/ActData_Mesh_Position.cpp
  Mesh/DS/ActData_Mesh_Quadrangle.cpp
  Mesh/DS/ActData_Mesh_Triangle.cpp
)

#------------------------------------------------------------------------------
# Programming patterns
#------------------------------------------------------------------------------

set (patterns_H_FILES
  Patterns/ActData_RealArrayOwnerAPI.h
  Patterns/ActData_RecordCollectionOwnerAPI.h
)

set (patterns_CPP_FILES 
  Patterns/ActData_RealArrayOwnerAPI.cpp
  Patterns/ActData_RecordCollectionOwnerAPI.cpp
)

#------------------------------------------------------------------------------
# Standard implementations
#------------------------------------------------------------------------------

set (std_H_FILES
  STD/ActData_BaseVarNode.h
  STD/ActData_BoolVarNode.h
  STD/ActData_BoolVarPartition.h
  STD/ActData_IntVarNode.h
  STD/ActData_IntVarPartition.h
  STD/ActData_RealEvaluatorFunc.h
  STD/ActData_RealVarNode.h
  STD/ActData_RealVarPartition.h
)

set (std_CPP_FILES 
  STD/ActData_BaseVarNode.cpp
  STD/ActData_BoolVarNode.cpp
  STD/ActData_BoolVarPartition.cpp
  STD/ActData_IntVarNode.cpp
  STD/ActData_IntVarPartition.cpp
  STD/ActData_RealEvaluatorFunc.cpp
  STD/ActData_RealVarNode.cpp
  STD/ActData_RealVarPartition.cpp
)

#------------------------------------------------------------------------------
# Tools
#------------------------------------------------------------------------------

set (tools_H_FILES
  Tools/ActData_CAFConversionAsset.h
  Tools/ActData_CAFConversionCtx.h
  Tools/ActData_CAFConversionModel.h
  Tools/ActData_CAFConversionNode.h
  Tools/ActData_CAFConversionParameter.h
  Tools/ActData_CAFConverter.h
  Tools/ActData_CAFConverterBase.h
  Tools/ActData_CAFConverterFw.h
  Tools/ActData_CAFDumper.h
  Tools/ActData_CAFLoader.h
  Tools/ActData_GraphToDot.h
  Tools/ActData_UniqueNodeName.h
  Tools/ActAux.h
  Tools/ActAux_ArrayUtils.h
  Tools/ActAux_Common.h
  Tools/ActAux_Env.h
  Tools/ActAux_ExprCalculator.h
  Tools/ActAux_FileDumper.h
  Tools/ActAux_ShapeFactory.h
  Tools/ActAux_SpyLog.h
  Tools/ActAux_TimeStamp.h
  Tools/ActAux_Timing.h
  Tools/ActAux_Utils.h
)

set (tools_CPP_FILES 
  Tools/ActData_CAFConversionAsset.cpp
  Tools/ActData_CAFConversionCtx.cpp
  Tools/ActData_CAFConversionModel.cpp
  Tools/ActData_CAFConversionNode.cpp
  Tools/ActData_CAFConversionParameter.cpp
  Tools/ActData_CAFConverter.cpp
  Tools/ActData_CAFConverterFw.cpp
  Tools/ActData_CAFDumper.cpp
  Tools/ActData_CAFLoader.cpp
  Tools/ActData_GraphToDot.cpp
  Tools/ActData_UniqueNodeName.cpp
  Tools/ActAux_ArrayUtils.cpp
  Tools/ActAux_ExprCalculator.cpp
  Tools/ActAux_FileDumper.cpp
  Tools/ActAux_ShapeFactory.cpp
  Tools/ActAux_SpyLog.cpp
  Tools/ActAux_TimeStamp.cpp
  Tools/ActAux_Utils.cpp
)

#------------------------------------------------------------------------------
# OpenCascade libraries
#------------------------------------------------------------------------------

set (OCCT_LIB_FILES
  TKernel
  TKMath
  TKBinL
  TKBin
  TKLCAF
  TKCAF
  TKCDF
  TKPrim
  TKBRep
  TKTopAlgo
)

#------------------------------------------------------------------------------
# TBB libraries
#------------------------------------------------------------------------------

if (ActiveData_USE_TBB)
  set (TBB_LIB_FILES
    tbb
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

foreach (FILE ${api_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\API" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${api_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\API" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${drivers_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Drivers" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${drivers_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Drivers" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${kernel_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Kernel" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${kernel_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Kernel" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${mesh_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Mesh" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${mesh_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Mesh" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${meshds_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Mesh\\DS" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${meshds_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Mesh\\DS" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${patterns_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Patterns" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${patterns_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Patterns" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${std_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\STD" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${std_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\STD" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${tools_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Tools" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${tools_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Tools" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (ActiveData_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};\
  ${CMAKE_CURRENT_SOURCE_DIR}/API;\
  ${CMAKE_CURRENT_SOURCE_DIR}/BinDrivers;\
  ${CMAKE_CURRENT_SOURCE_DIR}/Kernel;\
  ${CMAKE_CURRENT_SOURCE_DIR}/Mesh;\
  ${CMAKE_CURRENT_SOURCE_DIR}/Mesh/DS;\
  ${CMAKE_CURRENT_SOURCE_DIR}/Patterns;\
  ${CMAKE_CURRENT_SOURCE_DIR}/STD;\
  ${CMAKE_CURRENT_SOURCE_DIR}/Tools;")
#
set (asiActiveData_include_dir ${ActiveData_include_dir_loc} PARENT_SCOPE)

include_directories ( ${ActiveData_include_dir_loc}
                      ${3RDPARTY_OCCT_INCLUDE_DIR} )

if (ActiveData_USE_TBB)
  include_directories ( ${3RDPARTY_tbb_INCLUDE_DIR} )
endif()

#------------------------------------------------------------------------------
# Create library
#------------------------------------------------------------------------------

add_library (asiActiveData SHARED
  ${H_FILES}          ${CPP_FILES}
  ${api_H_FILES}      ${api_CPP_FILES}
  ${aux_H_FILES}      ${aux_CPP_FILES}
  ${drivers_H_FILES}  ${drivers_CPP_FILES}
  ${kernel_H_FILES}   ${kernel_CPP_FILES}
  ${mesh_H_FILES}     ${mesh_CPP_FILES}
  ${meshds_H_FILES}   ${meshds_CPP_FILES}
  ${patterns_H_FILES} ${patterns_CPP_FILES}
  ${std_H_FILES}      ${std_CPP_FILES}
  ${tools_H_FILES}    ${tools_CPP_FILES}
)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

if (ActiveData_USE_TBB)
  if (3RDPARTY_tbb_LIBRARY_DIR_DEBUG)
    link_directories(${3RDPARTY_tbb_LIBRARY_DIR_DEBUG})
  else()
    link_directories(${3RDPARTY_tbb_LIBRARY_DIR})
  endif()
endif()

foreach (LIB_FILE ${OCCT_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiActiveData debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiActiveData optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiActiveData ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

if (ActiveData_USE_TBB)
  foreach (LIB_FILE ${TBB_LIB_FILES})
    if (WIN32)
      set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
      set (LIB_FILENAME_DEBUG "${LIB_FILE}_debug${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
      set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set (LIB_FILENAME_DEBUG "lib${LIB_FILE}_debug${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()

    target_link_libraries (asiActiveData debug ${3RDPARTY_tbb_LIBRARY_DIR}/${LIB_FILENAME_DEBUG})
    target_link_libraries (asiActiveData optimized ${3RDPARTY_tbb_LIBRARY_DIR}/${LIB_FILENAME})
  endforeach()
endif()

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  install (TARGETS asiActiveData CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiActiveData CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiActiveData CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a framework
#------------------------------------------------------------------------------

install (TARGETS asiActiveData
         CONFIGURATIONS Release
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bin COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library)

install (TARGETS asiActiveData
         CONFIGURATIONS RelWithDebInfo
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bini COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library)

install (TARGETS asiActiveData
         CONFIGURATIONS Debug
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bind COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library)

if (MSVC)
  install (FILES ${PROJECT_BINARY_DIR}/../../${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bind/asiActiveData.pdb DESTINATION ${SDK_INSTALL_SUBDIR}bind CONFIGURATIONS Debug)
endif()

install (FILES ${H_FILES}          DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${api_H_FILES}      DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${drivers_H_FILES}  DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${kernel_H_FILES}   DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${mesh_H_FILES}     DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${meshds_H_FILES}   DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${patterns_H_FILES} DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${std_H_FILES}      DESTINATION ${SDK_INSTALL_SUBDIR}include)
install (FILES ${tools_H_FILES}    DESTINATION ${SDK_INSTALL_SUBDIR}include)
