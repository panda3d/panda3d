# Filename: FindSPIRVCross.cmake
# Authors: rdb (9 Nov, 2024)
#
# Usage:
#   find_package(SPIRV-Cross [REQUIRED] [QUIET])
#
# Once done this will define:
#   SPIRV_CROSS_FOUND        - system has SPIRV-Cross
#   SPIRV_CROSS_INCLUDE_DIR  - the path to the location of the spirv_cross directory
#   SPIRV_CROSS_LIBRARIES    - the libraries to link against for SPIRV-Cross
#

find_path(SPIRV_CROSS_INCLUDE_DIR
  NAMES "spirv_cross/spirv_cross.hpp")

find_library(SPIRV_CROSS_CORE_LIBRARY
  NAMES "spirv-cross-core")

find_library(SPIRV_CROSS_GLSL_LIBRARY
  NAMES "spirv-cross-glsl")

find_library(SPIRV_CROSS_HLSL_LIBRARY
  NAMES "spirv-cross-hlsl")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRV-Cross DEFAULT_MSG SPIRV_CROSS_INCLUDE_DIR SPIRV_CROSS_LIBRARIES)

mark_as_advanced(SPIRV_CROSS_INCLUDE_DIR SPIRV_CROSS_LIBRARIES)

if(SPIRV_CROSS_CORE_LIBRARY)
  set(SPIRV_CROSS_FOUND ON)

  add_library(SPIRV-Cross::Core UNKNOWN IMPORTED)

  set_target_properties(SPIRV-Cross::Core PROPERTIES
    IMPORTED_LOCATION "${SPIRV_CROSS_CORE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE_DIR}")
endif()

if(SPIRV_CROSS_GLSL_LIBRARY)
  add_library(SPIRV-Cross::GLSL UNKNOWN IMPORTED)
  target_link_libraries(SPIRV-Cross::GLSL INTERFACE SPIRV-Cross::Core)

  set_target_properties(SPIRV-Cross::GLSL PROPERTIES
    IMPORTED_LOCATION "${SPIRV_CROSS_GLSL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE_DIR}")
endif()

if(SPIRV_CROSS_HLSL_LIBRARY)
  add_library(SPIRV-Cross::HLSL UNKNOWN IMPORTED)
  target_link_libraries(SPIRV-Cross::HLSL INTERFACE SPIRV-Cross::GLSL)

  set_target_properties(SPIRV-Cross::HLSL PROPERTIES
    IMPORTED_LOCATION "${SPIRV_CROSS_HLSL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE_DIR}")
endif()
