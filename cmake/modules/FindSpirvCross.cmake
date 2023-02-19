# Filename: FindSpirvCross.cmake
# Authors: RegDogg (Feb 15, 2023)
#
# Usage:
#   find_package(SpirvCross [REQUIRED] [QUIET])
#
# Once done this will define:
#   SPIRVCROSS_FOUND       - system has SpirvCross
#   SPIRVCROSS_INCLUDE_DIR - the paths to the include directories
#   SPIRVCROSS_LIBRARIES   - the paths to the SpirvCross libraries
#

find_path(SPIRVCROSS_INCLUDE_DIR NAMES "spirv_cross")

find_library(SPIRVCROSS_LIBRARY NAMES "spirv-cross-core")
find_library(SPIRVCROSS_GLSL_LIBRARY NAMES "spirv-cross-glsl")
find_library(SPIRVCROSS_HLSL_LIBRARY NAMES "spirv-cross-hlsl")

set(SPIRVCROSS_LIBRARIES)
if(SPIRVCROSS_LIBRARY)
  list(APPEND SPIRVCROSS_LIBRARIES "${SPIRVCROSS_LIBRARY}")
endif()
if(SPIRVCROSS_GLSL_LIBRARY)
  list(APPEND SPIRVCROSS_LIBRARIES "${SPIRVCROSS_GLSL_LIBRARY}")
endif()
if(SPIRVCROSS_HLSL_LIBRARY)
  list(APPEND SPIRVCROSS_LIBRARIES "${SPIRVCROSS_HLSL_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SpirvCross DEFAULT_MSG SPIRVCROSS_INCLUDE_DIR SPIRVCROSS_LIBRARIES)
