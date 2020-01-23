# Filename: FindAssimp.cmake
# Authors: CFSworks (9 Nov, 2018)
#
# Usage:
#   find_package(Assimp [REQUIRED] [QUIET])
#
# Once done this will define:
#   ASSIMP_FOUND        - system has Assimp
#   ASSIMP_INCLUDE_DIR  - the path to the location of the assimp/ directory
#   ASSIMP_LIBRARY      - the library to link against for Assimp
#

find_path(ASSIMP_INCLUDE_DIR
  NAMES "assimp/Importer.hpp")

find_library(ASSIMP_LIBRARY
  NAMES "assimp")

mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Assimp DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)
