# Filename: FindOpenEXR.cmake
# Authors: CFSworks (5 Nov, 2018)
#
# Usage:
#   find_package(OpenEXR [REQUIRED] [QUIET])
#
# Once done this will define:
#   OPENEXR_FOUND       - system has OpenEXR
#   OPENEXR_INCLUDE_DIR - the include directory containing OpenEXR header files
#   OPENEXR_LIBRARIES   - the path to the OpenEXR libraries
#

find_path(OPENEXR_INCLUDE_DIR
  "ImfVersion.h"
  PATH_SUFFIXES "OpenEXR")

mark_as_advanced(OPENEXR_INCLUDE_DIR)

find_library(OPENEXR_imf_LIBRARY
  NAMES "IlmImf")

if(OPENEXR_imf_LIBRARY)
  get_filename_component(_imf_dir "${OPENEXR_imf_LIBRARY}" DIRECTORY)
  find_library(OPENEXR_imfutil_LIBRARY
    NAMES "IlmImfUtil"
    PATHS "${_imf_dir}"
    NO_DEFAULT_PATH)

  find_library(OPENEXR_ilmthread_LIBRARY
    NAMES "IlmThread"
    PATHS "${_imf_dir}"
    NO_DEFAULT_PATH)

  find_library(OPENEXR_iex_LIBRARY
    NAMES "Iex"
    PATHS "${_imf_dir}"
    NO_DEFAULT_PATH)

  find_library(OPENEXR_iexmath_LIBRARY
    NAMES "IexMath"
    PATHS "${_imf_dir}"
    NO_DEFAULT_PATH)

  find_library(OPENEXR_imath_LIBRARY
    NAMES "Imath"
    PATHS "${_imf_dir}"
    NO_DEFAULT_PATH)

  find_library(OPENEXR_half_LIBRARY
    NAMES "Half"
    PATHS "${_imf_dir}"
    NO_DEFAULT_PATH)

  unset(_imf_dir)
endif()

mark_as_advanced(
  OPENEXR_imf_LIBRARY
  OPENEXR_imfutil_LIBRARY
  OPENEXR_ilmthread_LIBRARY
  OPENEXR_iex_LIBRARY
  OPENEXR_iexmath_LIBRARY
  OPENEXR_imath_LIBRARY
  OPENEXR_half_LIBRARY
)

set(OPENEXR_LIBRARIES
  ${OPENEXR_imf_LIBRARY}
  ${OPENEXR_imfutil_LIBRARY}
  ${OPENEXR_ilmthread_LIBRARY}
  ${OPENEXR_iex_LIBRARY}
  ${OPENEXR_iexmath_LIBRARY}
  ${OPENEXR_imath_LIBRARY}
  ${OPENEXR_half_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR DEFAULT_MSG
  OPENEXR_INCLUDE_DIR OPENEXR_LIBRARIES

  OPENEXR_imf_LIBRARY
  OPENEXR_imfutil_LIBRARY
  OPENEXR_ilmthread_LIBRARY
  OPENEXR_iex_LIBRARY
  OPENEXR_iexmath_LIBRARY
  OPENEXR_imath_LIBRARY
  OPENEXR_half_LIBRARY
)
