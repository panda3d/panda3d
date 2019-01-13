# Filename: FindHarfBuzz.cmake
# Authors: CFSworks (2 Nov, 2018)
#
# Usage:
#   find_package(HarfBuzz [REQUIRED] [QUIET])
#
# Once done this will define:
#   HARFBUZZ_FOUND       - system has HarfBuzz
#   HARFBUZZ_INCLUDE_DIR - the include directory containing hb.h
#   HARFBUZZ_LIBRARY     - the path to the HarfBuzz library
#

find_path(HARFBUZZ_INCLUDE_DIR
  NAMES "hb.h"
  PATH_SUFFIXES "harfbuzz")

find_library(HARFBUZZ_LIBRARY
  NAMES "harfbuzz")

mark_as_advanced(HARFBUZZ_INCLUDE_DIR HARFBUZZ_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HarfBuzz DEFAULT_MSG HARFBUZZ_INCLUDE_DIR HARFBUZZ_LIBRARY)
