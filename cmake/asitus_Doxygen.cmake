if (NOT DISTRIBUTION_GENERATE_DOC)
  ASITUS_UNSET(DOXYGEN_EXE)
  return()
endif()

message (STATUS "Processing Doxygen 3-rd party")

ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" "doxygen" DOXYGEN_DIR)
set (DOXYGEN_DIR "${3RDPARTY_DIR}/${DOXYGEN_DIR}")
message (STATUS "... Doxygen dir: ${DOXYGEN_DIR}")
set (DOXYGEN_EXE "${DOXYGEN_DIR}/doxygen${CMAKE_EXECUTABLE_SUFFIX}")
if (EXISTS "${DOXYGEN_EXE}")
  message (STATUS "... Doxygen executable found: ${DOXYGEN_EXE}")
  set (DOXYGEN_EXE "${DOXYGEN_EXE}" CACHE PATH "Doxygen - documentation generator" FORCE)
else()
  set (DOXYGEN_EXE "" CACHE PATH "Doxygen - documentation generator" FORCE)
endif()


