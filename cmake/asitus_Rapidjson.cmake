if (NOT USE_RAPIDJSON)
  message (STATUS "Usetting Rapidjson 3-rd party")
  ASITUS_UNSET_3RDPARTY("rapidjson")
  return()
endif()

message (STATUS "Processing Rapidjson 3-rd party")

add_definitions (-DUSE_RAPIDJSON)

ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" "rapidjson" RAPIDJSON_DIR)

set (3RDPARTY_rapidjson_DIR "${3RDPARTY_DIR}/${RAPIDJSON_DIR}/include" CACHE PATH "The directory containing Rapidjson." FORCE)

message (STATUS "... Rapidjson dir: ${3RDPARTY_rapidjson_DIR}")
