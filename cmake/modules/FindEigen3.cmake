# Filename: FindEigen3.cmake
# Authors: kestred (13 Dec, 2013)
#
# Usage:
#   find_package(Eigen3 [REQUIRED] [QUIET])
#
# Once done this will define:
#   EIGEN_FOUND       - system has Eigen3
#   EIGEN_INCLUDE_DIR - the Eigen3 include directory
#

if(Eigen3_FIND_QUIETLY)
  set(Eigen_FIND_QUIETLY TRUE)
endif()

if(NOT EIGEN_INCLUDE_DIR)
  find_path(EIGEN_INCLUDE_DIR
    NAMES signature_of_eigen3_matrix_library
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          ${KDE4_INCLUDE_DIR}
    PATH_SUFFIXES eigen3 eigen
  )

  mark_as_advanced(EIGEN_INCLUDE_DIR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen DEFAULT_MSG EIGEN_INCLUDE_DIR)