# Filename: FindLZ4.cmake
# Authors: CFSworks (26 Oct, 2018)
#
# Usage:
#   find_package(LZ4 [REQUIRED] [QUIET])
#
# Once done this will define:
#   LZ4_FOUND       - system has lz4
#   LZ4_INCLUDE_DIR - the include directory containing lz4.h.
#   LZ4_LIBRARY     - the path to the lz4 library
#

find_path(LZ4_INCLUDE_DIRS "lz4.h" "lz4frame.h")

find_library(LZ4_LIBRARIES
  NAMES "liblz4" "lz4")

mark_as_advanced(LZ4_FOUND LZ4_INCLUDE_DIRS LZ4_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZ4 DEFAULT_MSG LZ4_INCLUDE_DIRS LZ4_LIBRARIES)
