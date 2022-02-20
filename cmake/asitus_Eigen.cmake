message (STATUS "Processing Eigen 3-rd party")

ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" "eigen" EIGEN_DIR)

set (3RDPARTY_EIGEN_DIR "${3RDPARTY_DIR}/${EIGEN_DIR}" CACHE PATH "The directory containing Eigen" FORCE)

if (NOT EXISTS "${3RDPARTY_EIGEN_DIR}/Eigen")
  set (3RDPARTY_EIGEN_DIR "" CACHE PATH "" FORCE)
  message( SEND_ERROR "... Eigen is not found.")
endif()     

message (STATUS "... Eigen dir: ${3RDPARTY_EIGEN_DIR}")
