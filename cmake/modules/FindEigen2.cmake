# Filename: FindEigen2.cmake
# Authors: kestred (13 Dec, 2013)
#
# Usage:
#   find_package(Eigen2 [REQUIRED] [QUIET])
#
# Once done this will define:
#   EIGEN_FOUND        - system has any version of Eigen
#   EIGEN2_FOUND       - system has Eigen2
#   EIGEN2_INCLUDE_DIR - the Eigen2 include directory
#

if(NOT EIGEN2_INCLUDE_DIR)
  find_path(EIGEN2_INCLUDE_DIR
    NAMES Eigen/Core
    PATHS ${INCLUDE_INSTALL_DIR}
          ${KDE4_INCLUDE_DIR}
    PATH_SUFFIXES eigen2
  )

  mark_as_advanced(EIGEN2_INCLUDE_DIR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen2 DEFAULT_MSG EIGEN2_INCLUDE_DIR)

if(EIGEN2_FOUND)
	set(EIGEN_FOUND TRUE)
endif()