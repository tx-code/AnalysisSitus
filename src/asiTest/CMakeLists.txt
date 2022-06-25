project(asiTest)

#------------------------------------------------------------------------------
# Common
#------------------------------------------------------------------------------

set (H_FILES
  asiTest.h
  asiTest_CaseIDs.h
  asiTest_CommonFacilities.h
  asiTest_TclTestCase.h
)
set (CPP_FILES
  asiTest_CommonFacilities.cpp
  asiTest_Main.cpp
  asiTest_TclTestCase.cpp
)

#------------------------------------------------------------------------------
# Test cases
#------------------------------------------------------------------------------

set (cases_activedata_H_FILES
  cases/activedata/ActTest_AsciiStringParameter.h
  cases/activedata/ActTest_BaseModel.h
  cases/activedata/ActTest_BoolArrayParameter.h
  cases/activedata/ActTest_BoolParameter.h
  cases/activedata/ActTest_CAFConversionCtx.h
  cases/activedata/ActTest_ComplexArrayParameter.h
  cases/activedata/ActTest_CopyPasteEngine.h
  cases/activedata/ActTest_DataFramework.h
  cases/activedata/ActTest_ExtTransactionEngine.h
  cases/activedata/ActTest_GroupParameter.h
  cases/activedata/ActTest_IntArrayParameter.h
  cases/activedata/ActTest_IntParameter.h
  cases/activedata/ActTest_MeshAttr.h
  cases/activedata/ActTest_MeshParameter.h
  cases/activedata/ActTest_NameParameter.h
  cases/activedata/ActTest_RealArrayParameter.h
  cases/activedata/ActTest_RealParameter.h
  cases/activedata/ActTest_ReferenceListParameter.h
  cases/activedata/ActTest_ReferenceParameter.h
  cases/activedata/ActTest_SelectionParameter.h
  cases/activedata/ActTest_ShapeParameter.h
  cases/activedata/ActTest_StringArrayParameter.h
  cases/activedata/ActTest_TimeStamp.h
  cases/activedata/ActTest_TimeStampParameter.h
  cases/activedata/ActTest_TreeFunctionParameter.h
  cases/activedata/ActTest_TreeNodeParameter.h
  cases/activedata/ActTest_TriangulationParameter.h
)
set (cases_activedata_CPP_FILES
  cases/activedata/ActTest_AsciiStringParameter.cpp
  cases/activedata/ActTest_BaseModel.cpp
  cases/activedata/ActTest_BoolArrayParameter.cpp
  cases/activedata/ActTest_BoolParameter.cpp
  cases/activedata/ActTest_CAFConversionCtx.cpp
  cases/activedata/ActTest_ComplexArrayParameter.cpp
  cases/activedata/ActTest_CopyPasteEngine.cpp
  cases/activedata/ActTest_DataFramework.cpp
  cases/activedata/ActTest_ExtTransactionEngine.cpp
  cases/activedata/ActTest_GroupParameter.cpp
  cases/activedata/ActTest_IntArrayParameter.cpp
  cases/activedata/ActTest_IntParameter.cpp
  cases/activedata/ActTest_MeshAttr.cpp
  cases/activedata/ActTest_MeshParameter.cpp
  cases/activedata/ActTest_NameParameter.cpp
  cases/activedata/ActTest_RealArrayParameter.cpp
  cases/activedata/ActTest_RealParameter.cpp
  cases/activedata/ActTest_ReferenceListParameter.cpp
  cases/activedata/ActTest_ReferenceParameter.cpp
  cases/activedata/ActTest_SelectionParameter.cpp
  cases/activedata/ActTest_ShapeParameter.cpp
  cases/activedata/ActTest_StringArrayParameter.cpp
  cases/activedata/ActTest_TimeStamp.cpp
  cases/activedata/ActTest_TimeStampParameter.cpp
  cases/activedata/ActTest_TreeFunctionParameter.cpp
  cases/activedata/ActTest_TreeNodeParameter.cpp
  cases/activedata/ActTest_TriangulationParameter.cpp
)

set (cases_activedata_model_H_FILES
  cases/activedata/model/ActTest_DummyModel.h
  cases/activedata/model/ActTest_DummyTreeFunction.h
  cases/activedata/model/ActTest_StubANode.h
  cases/activedata/model/ActTest_StubAPartition.h
  cases/activedata/model/ActTest_StubBNode.h
  cases/activedata/model/ActTest_StubBNodeConv.h
  cases/activedata/model/ActTest_StubBPartition.h
  cases/activedata/model/ActTest_StubCNode.h
  cases/activedata/model/ActTest_StubCNodeConv.h
  cases/activedata/model/ActTest_StubCPartition.h
  cases/activedata/model/ActTest_StubMeshNode.h
  cases/activedata/model/ActTest_StubMeshPartition.h
)
set (cases_activedata_model_CPP_FILES
  cases/activedata/model/ActTest_DummyModel.cpp
  cases/activedata/model/ActTest_DummyTreeFunction.cpp
  cases/activedata/model/ActTest_StubANode.cpp
  cases/activedata/model/ActTest_StubAPartition.cpp
  cases/activedata/model/ActTest_StubBNode.cpp
  cases/activedata/model/ActTest_StubBNodeConv.cpp
  cases/activedata/model/ActTest_StubBPartition.cpp
  cases/activedata/model/ActTest_StubCNode.cpp
  cases/activedata/model/ActTest_StubCNodeConv.cpp
  cases/activedata/model/ActTest_StubCPartition.cpp
  cases/activedata/model/ActTest_StubMeshNode.cpp
  cases/activedata/model/ActTest_StubMeshPartition.cpp
)

set (cases_asm_H_FILES
  cases/asm/asiTest_XdeDoc.h
)
set (cases_asm_CPP_FILES
  cases/asm/asiTest_XdeDoc.cpp
)

set (cases_editing_H_FILES
  cases/editing/asiTest_ConvertCanonical.h
  cases/editing/asiTest_InvertShells.h
  cases/editing/asiTest_KEV.h
  cases/editing/asiTest_RebuildEdge.h
  cases/editing/asiTest_SuppressBlends.h
)
set (cases_editing_CPP_FILES
  cases/editing/asiTest_ConvertCanonical.cpp
  cases/editing/asiTest_InvertShells.cpp
  cases/editing/asiTest_KEV.cpp
  cases/editing/asiTest_RebuildEdge.cpp
  cases/editing/asiTest_SuppressBlends.cpp
)

set (cases_exchange_H_FILES
  cases/exchange/asiTest_ExchangeMesh.h
)
set (cases_exchange_CPP_FILES
  cases/exchange/asiTest_ExchangeMesh.cpp
)

set (cases_framework_H_FILES
  cases/framework/asiTest_ChangeColor.h
  cases/framework/asiTest_DataDictionary.h
  cases/framework/asiTest_GenerateFacets.h
  cases/framework/asiTest_Utils.h
)
set (cases_framework_CPP_FILES
  cases/framework/asiTest_ChangeColor.cpp
  cases/framework/asiTest_DataDictionary.cpp
  cases/framework/asiTest_GenerateFacets.cpp
  cases/framework/asiTest_Utils.cpp
)

set (cases_inspection_H_FILES
  cases/inspection/asiTest_AAG.h
  cases/inspection/asiTest_EdgeVexity.h
  cases/inspection/asiTest_FaceGrid.h
  cases/inspection/asiTest_IsContourClosed.h
  cases/inspection/asiTest_RecognizeBlends.h
  cases/inspection/asiTest_RecognizeCavities.h
  cases/inspection/asiTest_RecognizeConvexHull.h
)
set (cases_inspection_CPP_FILES
  cases/inspection/asiTest_AAG.cpp
  cases/inspection/asiTest_EdgeVexity.cpp
  cases/inspection/asiTest_FaceGrid.cpp
  cases/inspection/asiTest_IsContourClosed.cpp
  cases/inspection/asiTest_RecognizeBlends.cpp
  cases/inspection/asiTest_RecognizeCavities.cpp
  cases/inspection/asiTest_RecognizeConvexHull.cpp
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

foreach (FILE ${cases_activedata_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\ActiveData" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_activedata_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\ActiveData" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_activedata_model_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\ActiveData\\Model" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_activedata_model_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\ActiveData\\Model" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_asm_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\Assemblies" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_asm_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\Assemblies" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_editing_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\Editing" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_editing_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\Editing" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_exchange_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\Exchange" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_exchange_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\Exchange" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_framework_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\Framework" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_framework_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\Framework" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_inspection_H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files\\Cases\\Inspection" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${cases_inspection_CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files\\Cases\\Inspection" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Resources
#------------------------------------------------------------------------------

source_group ("Resources" FILES resources/asiTestDictionary.xml)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (asiTest_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR};\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/activedata;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/activedata/model;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/asm;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/editing;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/exchange;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/framework;\
  ${CMAKE_CURRENT_SOURCE_DIR}/cases/inspection;")
#
set (asiTest_include_dir ${asiTest_include_dir_loc} PARENT_SCOPE)

include_directories ( SYSTEM
                      ${asiTest_include_dir_loc}
                      ${asiActiveData_include_dir}
                      ${asiAlgo_include_dir}
                      ${asiAsm_include_dir}
                      ${asiData_include_dir}
                      ${asiEngine_include_dir}
                      ${asiUI_include_dir}
                      ${asiVisu_include_dir}
                      ${asiTcl_include_dir}
                      ${asiTestEngine_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_vtk_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

#------------------------------------------------------------------------------
# Create executable
#------------------------------------------------------------------------------

add_executable(asiTest ${src_files} ${RCC_FILE} resources/asiTestDictionary.xml)

set (X_COMPILER_BITNESS "x${COMPILER_BITNESS}")

COPY_RESOURCES_TO_BINARY_DIRS(resources/asiTestDictionary.xml)

configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/exePROTOTYPE.vcxproj.user.in
               ${asiTest_BINARY_DIR}/asiTest.vcxproj.user @ONLY)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

target_link_libraries(asiTest asiAlgo asiAsm asiData asiEngine asiTestEngine asiTcl asiUI cmdEngine cmdMisc cmdTest)

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

if (WIN32)
  install (TARGETS asiTest CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiTest CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiTest CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)

  install (FILES resources/asiTestDictionary.xml CONFIGURATIONS Release        DESTINATION bin/resources)
  install (FILES resources/asiTestDictionary.xml CONFIGURATIONS RelWithDebInfo DESTINATION bini/resources)
  install (FILES resources/asiTestDictionary.xml CONFIGURATIONS Debug          DESTINATION bind/resources)
endif()
