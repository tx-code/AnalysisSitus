project(docapi)

### Set Doxygen variables ###
set (DOXYGEN_DIR        "${docapi_BINARY_DIR}/DOC")
set (DOXYGEN_OUTPUT_DIR "${DOXYGEN_DIR}/refdoc")
set (DOXYGEN_INDEX_FILE "${DOXYGEN_OUTPUT_DIR}/index.html")
set (DOXYGEN_TAG        "${DOXYGEN_OUTPUT_DIR}/RefDoc.tag")
set (DOXYGEN_INPUT_DIR  "${CMAKE_SOURCE_DIR}/src")
set (DOXYGEN_API_IMGS   "${docapi_SOURCE_DIR}/../imgs/algo")

### Generate Doxyfiles ###
# Find all the public headers and make them as input for Doxygen.
set (PUBLIC_HEADERS "")

# Get actual list of targets.
file (GLOB SRC_SUBDIRS RELATIVE "${DOXYGEN_INPUT_DIR}" "${DOXYGEN_INPUT_DIR}/*")

# Add public headers to the Doxygen.
foreach(T IN LISTS SRC_SUBDIRS)
  if (IS_DIRECTORY "${DOXYGEN_INPUT_DIR}/${T}")
    if (TARGET "${T}")
      file(GLOB_RECURSE SPH "${DOXYGEN_INPUT_DIR}/${T}/*.h*")
      list(APPEND PUBLIC_HEADERS "${SPH}")
    endif()
  endif()
endforeach()

set (DOXYGEN_INPUT_DIR "${PUBLIC_HEADERS}")

# Replace ; with space so string adheres to Doxygen syntax
string (REPLACE ";" " " DOXYGEN_INPUT_DIR "${DOXYGEN_INPUT_DIR}")

# Replace variables inside @@ with the actual values
set (DOXYFILE_IN  ${docapi_SOURCE_DIR}/Doxyfile.in)
set (DOXYFILE_OUT ${DOXYGEN_DIR}/Doxyfile)
configure_file (${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

set (DOXYGEN_INDEX_LOC "./refdoc/index.html") # keep it relative for proper work after install
configure_file (${DOXYGEN_STATIC}/index_ref.html
                ${DOXYGEN_DIR}/RefDoc.html @ONLY)

### Run  Doxygen ###
add_custom_command (OUTPUT  ${DOXYGEN_INDEX_FILE}
                    COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
                    COMMAND ${DOXYGEN_EXE} ${DOXYGEN_DIR}/Doxyfile
                    COMMAND ${CMAKE_COMMAND} -E echo "Done.")

add_custom_target (docapi ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set_property (TARGET docapi PROPERTY FOLDER Documentation)

install (DIRECTORY ${DOXYGEN_OUTPUT_DIR}       DESTINATION "${INSTALL_SUBDIR}doc")
install (FILES     ${DOXYGEN_DIR}/RefDoc.html  DESTINATION "${INSTALL_SUBDIR}doc")
