# Filename: FindARToolKit.cmake
# Authors: CFSworks (3 Nov, 2018)
#
# Usage:
#   find_package(ARToolKit [REQUIRED] [QUIET])
#
# Once done this will define:
#   ARTOOLKIT_FOUND       - system has ARToolKit
#   ARTOOLKIT_INCLUDE_DIR - the include directory containing ARToolKit header files
#   ARTOOLKIT_LIBRARIES   - the paths to the ARToolKit client libraries
#

if(NOT ARTOOLKIT_INCLUDE_DIR)
  find_path(ARTOOLKIT_INCLUDE_DIR "AR/ar.h")

  mark_as_advanced(ARTOOLKIT_INCLUDE_DIR)
endif()

if(NOT ARTOOLKIT_AR_LIBRARY)
  find_library(ARTOOLKIT_AR_LIBRARY
    NAMES "AR" "libAR")

  mark_as_advanced(ARTOOLKIT_AR_LIBRARY)
endif()

if(NOT ARTOOLKIT_ARMulti_LIBRARY)
  find_library(ARTOOLKIT_ARMulti_LIBRARY
    NAMES "ARMulti" "libARMulti")

  mark_as_advanced(ARTOOLKIT_ARMulti_LIBRARY)
endif()

set(ARTOOLKIT_LIBRARIES)
if(ARTOOLKIT_AR_LIBRARY)
  list(APPEND ARTOOLKIT_LIBRARIES "${ARTOOLKIT_AR_LIBRARY}")
endif()
if(ARTOOLKIT_ARMulti_LIBRARY)
  list(APPEND ARTOOLKIT_LIBRARIES "${ARTOOLKIT_ARMulti_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ARToolKit DEFAULT_MSG
  ARTOOLKIT_INCLUDE_DIR ARTOOLKIT_LIBRARIES)
