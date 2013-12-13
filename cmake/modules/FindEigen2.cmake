# Filename: FindEigen2.cmake
# Authors: kestred (13 Dec, 2013)
#
# Usage:
#   find_package(Eigen2 [REQUIRED] [QUIET])
#
# Once done this will define:
#   EIGEN_FOUND       - system has Eigen2
#   EIGEN_INCLUDE_DIR - the Eigen2 include directory
#

if(Eigen2_FIND_QUIETLY)
  set(Eigen_FIND_QUIETLY TRUE)
endif()

if(NOT EIGEN_INCLUED_DIR)
  find_path(EIGEN_INCLUDE_DIR
    NAMES Eigen/Core
    PATHS ${INCLUDE_INSTALL_DIR}
          ${KDE4_INCLUDE_DIR}
    PATH_SUFFIXES eigen2
  )

  mark_as_advanced(EIGEN_INCLUDE_DIR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen DEFAULT_MSG EIGEN_INCLUDE_DIR)
