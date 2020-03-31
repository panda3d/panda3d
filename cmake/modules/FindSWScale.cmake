# Filename: FindSWScale.cmake
# Author: CFSworks (10 Apr, 2014)
#
# Usage:
#   find_package(SWScale [REQUIRED] [QUIET])
#
# Once done this will define:
#   SWSCALE_FOUND       - system has ffmpeg's libswscale
#   SWSCALE_INCLUDE_DIR - the libswscale include directory
#   SWSCALE_LIBRARY     - the path to the library binary
#

# Find the libswscale include files
find_path(SWSCALE_INCLUDE_DIR
  NAMES "libswscale/swscale.h"
  PATHS "/usr/include"
        "/usr/local/include"
        "/sw/include"
        "/opt/include"
        "/opt/local/include"
        "/opt/csw/include"
  PATH_SUFFIXES "libav" "ffmpeg"
)

# Find the libswscale library
find_library(SWSCALE_LIBRARY
  NAMES "swscale"
  PATHS "/usr"
        "/usr/local"
        "/usr/freeware"
        "/sw"
        "/opt"
        "/opt/csw"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

mark_as_advanced(SWSCALE_INCLUDE_DIR)
mark_as_advanced(SWSCALE_LIBRARY)

# Translate library into library directory
if(SWSCALE_LIBRARY)
  unset(SWSCALE_LIBRARY_DIR CACHE)
  get_filename_component(SWSCALE_LIBRARY_DIR "${SWSCALE_LIBRARY}" PATH)
  set(SWSCALE_LIBRARY_DIR "${SWSCALE_LIBRARY_DIR}" CACHE PATH "The path to libffmpeg's library directory.") # Library path
endif()

mark_as_advanced(SWSCALE_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SWScale DEFAULT_MSG SWSCALE_LIBRARY SWSCALE_INCLUDE_DIR SWSCALE_LIBRARY_DIR)
