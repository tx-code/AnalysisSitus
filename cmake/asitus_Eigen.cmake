message (STATUS "Processing Eigen 3-rd party")

if (WIN32)
  ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" "eigen" EIGEN_DIR)
else()
  find_package (Eigen3 REQUIRED)
  include_directories (EIGEN3_INCLUDE_DIR)

  set (3RDPARTY_EIGEN_DIR "${EIGEN3_INCLUDE_DIR}" CACHE INTERNAL "" FORCE)
endif()

set (3RDPARTY_EIGEN_DIR "${EIGEN3_INCLUDE_DIR}" CACHE PATH "The directory containing Eigen")

if (NOT EXISTS "${3RDPARTY_EIGEN_DIR}/Eigen/Eigen")
  set (3RDPARTY_EIGEN_DIR "" CACHE PATH "" FORCE)
  message( SEND_ERROR "... Eigen is not found.")
endif() 

message (STATUS "... Eigen dir: ${3RDPARTY_EIGEN_DIR}")
