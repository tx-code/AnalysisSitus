message (STATUS "Processing Eigen 3-rd party")

ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" "eigen" EIGEN_DIR)

find_package( Eigen3 REQUIRED )
include_directories( EIGEN3_INCLUDE_DIR )

set (3RDPARTY_EIGEN_DIR "${EIGEN3_INCLUDE_DIR}" CACHE PATH "The directory containing Eigen")

message (STATUS "... Eigen dir: ${3RDPARTY_EIGEN_DIR}")
message (STATUS "... Eigen dir: ${EIGEN3_INCLUDE_DIR}")
