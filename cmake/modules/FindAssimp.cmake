# Filename: FindAssimp.cmake
# Authors: CFSworks (9 Nov, 2018)
#
# Usage:
#   find_package(Assimp [REQUIRED] [QUIET])
#
# Once done this will define:
#   ASSIMP_FOUND        - system has Assimp
#   ASSIMP_INCLUDE_DIR  - the path to the location of the assimp/ directory
#   ASSIMP_LIBRARIES    - the libraries to link against for Assimp
#

find_path(ASSIMP_INCLUDE_DIR
  NAMES "assimp/Importer.hpp")

find_library(ASSIMP_ASSIMP_LIBRARY
  NAMES "assimp")

find_library(ASSIMP_IRRXML_LIBRARY
  NAMES "IrrXML")

if(ASSIMP_ASSIMP_LIBRARY)
  set(ASSIMP_LIBRARIES "${ASSIMP_ASSIMP_LIBRARY}")

  if(ASSIMP_IRRXML_LIBRARY)
    list(APPEND ASSIMP_LIBRARIES "${ASSIMP_IRRXML_LIBRARY}")
  endif()
endif()

mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Assimp DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARIES)
