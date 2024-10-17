# Filename: FindRecast.cmake
# Authors: Maxwell175 (27 Feb 2022)
#
# Usage:
#   find_package(Recast [REQUIRED] [QUIET])
#
# Once done this will define:
#   RECAST_FOUND       - system has Recast
#   RECAST_INCLUDE_DIR - the include directory containing recastnavigation/
#   RECAST_LIBRARY     - the path to the recast library
#

find_path(RECAST_INCLUDE_DIR NAMES "recastnavigation/Recast.h")

find_library(RECAST_LIBRARY NAMES "Recast")

mark_as_advanced(RECAST_INCLUDE_DIR RECAST_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Recast DEFAULT_MSG RECAST_INCLUDE_DIR RECAST_LIBRARY)

if(RECAST_INCLUDE_DIR)
  add_library(Recast::Recast UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Recast::Recast PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${RECAST_INCLUDE_DIR}"
          IMPORTED_LOCATION "${RECAST_LIBRARY}")
endif()
