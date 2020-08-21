# Filename: FindEigen3.cmake
# Authors: kestred (13 Dec, 2013)
#
# Usage:
#   find_package(Eigen3 [REQUIRED] [QUIET])
#
# Once done this will define:
#   EIGEN_FOUND        - system has any version of Eigen
#   EIGEN3_FOUND       - system has Eigen3
#   EIGEN3_INCLUDE_DIR - the Eigen3 include directory
#

find_path(EIGEN3_INCLUDE_DIR
  NAMES signature_of_eigen3_matrix_library
  PATH_SUFFIXES eigen3 eigen)

mark_as_advanced(EIGEN3_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen3 DEFAULT_MSG EIGEN3_INCLUDE_DIR)

if(EIGEN3_FOUND)
  set(EIGEN_FOUND TRUE)
  set(EIGEN_INCLUDE_DIR ${EIGEN3_INCLUDE_DIR})
endif()
