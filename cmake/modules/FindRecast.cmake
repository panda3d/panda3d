# Filename: FindOgg.cmake
# Authors: CFSworks (13 Jan, 2019)
#
# Usage:
#   find_package(Ogg [REQUIRED] [QUIET])
#
# Once done this will define:
#   OGG_FOUND       - system has Ogg
#   OGG_INCLUDE_DIR - the include directory containing ogg/
#   OGG_LIBRARY     - the path to the ogg library
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
