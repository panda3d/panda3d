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

find_path(LZ4_INCLUDE_DIR "lz4.h")

find_library(LZ4_LIBRARY
  NAMES "lz4", "lz4_static)")

if(${LZ4_LIBRARY})
    set(LZ4_LIBRARIES ${LZ4_LIBRARY}))

mark_as_advanced(LZ4_INCLUDE_DIR LZ4_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LZ4 DEFAULT_MSG LZ4_INCLUDE_DIR LZ4_LIBRARY)
