# Filename: FindVRPN.cmake
# Authors: CFSworks (2 Nov, 2018)
#
# Usage:
#   find_package(VRPN [REQUIRED] [QUIET])
#
# Once done this will define:
#   VRPN_FOUND       - system has VRPN
#   VRPN_INCLUDE_DIR - the include directory containing VRPN header files
#   VRPN_LIBRARY     - the path to the VRPN client library
#

if(NOT VRPN_INCLUDE_DIR)
  find_path(VRPN_INCLUDE_DIR "vrpn_Connection.h")

  mark_as_advanced(VRPN_INCLUDE_DIR)
endif()

if(NOT VRPN_LIBRARY)
  find_library(VRPN_LIBRARY
    NAMES "vrpn")

  mark_as_advanced(VRPN_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRPN DEFAULT_MSG VRPN_INCLUDE_DIR VRPN_LIBRARY)
