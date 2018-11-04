# Filename: FindVRPN.cmake
# Authors: CFSworks (2 Nov, 2018)
#
# Usage:
#   find_package(VRPN [REQUIRED] [QUIET])
#
# Once done this will define:
#   VRPN_FOUND       - system has VRPN
#   VRPN_INCLUDE_DIR - the include directory containing VRPN header files
#   VRPN_LIBRARIES   - the path to the VRPN client libraries
#

if(NOT VRPN_INCLUDE_DIR)
  find_path(VRPN_INCLUDE_DIR "vrpn_Connection.h")

  mark_as_advanced(VRPN_INCLUDE_DIR)
endif()

if(NOT VRPN_vrpn_LIBRARY)
  find_library(VRPN_vrpn_LIBRARY
    NAMES "vrpn")

  mark_as_advanced(VRPN_vrpn_LIBRARY)
endif()

if(VRPN_vrpn_LIBRARY AND NOT VRPN_quat_LIBRARY)
  get_filename_component(_vrpn_dir "${VRPN_vrpn_LIBRARY}" DIRECTORY)
  find_library(VRPN_quat_LIBRARY
    NAMES "quat"
    PATHS "${_vrpn_dir}"
    NO_DEFAULT_PATH)

  unset(_vrpn_dir)
  mark_as_advanced(VRPN_quat_LIBRARY)
endif()

set(VRPN_LIBRARIES)
if(VRPN_vrpn_LIBRARY)
  list(APPEND VRPN_LIBRARIES "${VRPN_vrpn_LIBRARY}")
endif()
if(VRPN_quat_LIBRARY)
  list(APPEND VRPN_LIBRARIES "${VRPN_quat_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRPN DEFAULT_MSG VRPN_INCLUDE_DIR VRPN_LIBRARIES)
