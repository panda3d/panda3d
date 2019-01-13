# Filename: FindSWResample.cmake
# Author: CFSworks (10 Apr, 2014)
#
# Usage:
#   find_package(SWResample [REQUIRED] [QUIET])
#
# Once done this will define:
#   SWRESAMPLE_FOUND       - system has ffmpeg's libswresample
#   SWRESAMPLE_INCLUDE_DIR - the libswresample include directory
#   SWRESAMPLE_LIBRARY     - the path to the library binary
#

# Find the libswresample include files
find_path(SWRESAMPLE_INCLUDE_DIR
  NAMES "libswresample/swresample.h"
  PATHS "/usr/include"
        "/usr/local/include"
        "/sw/include"
        "/opt/include"
        "/opt/local/include"
        "/opt/csw/include"
  PATH_SUFFIXES "libav" "ffmpeg"
)

# Find the libswresample library
find_library(SWRESAMPLE_LIBRARY
  NAMES "swresample"
  PATHS "/usr"
        "/usr/local"
        "/usr/freeware"
        "/sw"
        "/opt"
        "/opt/csw"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

mark_as_advanced(SWRESAMPLE_INCLUDE_DIR)
mark_as_advanced(SWRESAMPLE_LIBRARY)

# Translate library into library directory
if(SWRESAMPLE_LIBRARY)
  unset(SWRESAMPLE_LIBRARY_DIR CACHE)
  get_filename_component(SWRESAMPLE_LIBRARY_DIR "${SWRESAMPLE_LIBRARY}" PATH)
  set(SWRESAMPLE_LIBRARY_DIR "${SWRESAMPLE_LIBRARY_DIR}" CACHE PATH "The path to libffmpeg's library directory.") # Library path
endif()

mark_as_advanced(SWRESAMPLE_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SWResample DEFAULT_MSG SWRESAMPLE_LIBRARY SWRESAMPLE_INCLUDE_DIR SWRESAMPLE_LIBRARY_DIR)
