# Filename: FindDetour.cmake
# Authors: Maxwell175 (27 Feb 2022)
#
# Usage:
#   find_package(Detour [REQUIRED] [QUIET])
#
# Once done this will define:
#   DETOUR_FOUND       - system has Detour
#   DETOUR_INCLUDE_DIR - the include directory containing recastnavigation/
#   DETOUR_LIBRARY     - the path to the detour library
#

find_path(DETOUR_INCLUDE_DIR NAMES "recastnavigation/DetourCommon.h")

find_library(DETOUR_LIBRARY NAMES "Detour")
find_library(DETOUR_TILE_LIBRARY NAMES "DetourTileCache")
find_library(DETOUR_CROWD_LIBRARY NAMES "DetourCrowd")

mark_as_advanced(DETOUR_INCLUDE_DIR DETOUR_LIBRARY DETOUR_TILE_LIBRARY DETOUR_CROWD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Detour DEFAULT_MSG DETOUR_INCLUDE_DIR DETOUR_LIBRARY)

if(DETOUR_INCLUDE_DIR)
  add_library(Detour::DetourMain UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Detour::DetourMain PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${DETOUR_INCLUDE_DIR}"
          IMPORTED_LOCATION "${DETOUR_LIBRARY}")

  add_library(Detour::DetourTile UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Detour::DetourTile PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${DETOUR_INCLUDE_DIR}"
          IMPORTED_LOCATION "${DETOUR_TILE_LIBRARY}")

  add_library(Detour::DetourCrowd UNKNOWN IMPORTED GLOBAL)

  set_target_properties(Detour::DetourCrowd PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${DETOUR_INCLUDE_DIR}"
          IMPORTED_LOCATION "${DETOUR_CROWD_LIBRARY}")

  add_library(Detour::Detour INTERFACE IMPORTED)
  set_property(TARGET Detour::Detour PROPERTY
      INTERFACE_LINK_LIBRARIES Detour::DetourMain Detour::DetourTile Detour::DetourCrowd)
endif()
