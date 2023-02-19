# Filename: FindSpirvTools.cmake
# Authors: RegDogg (Feb 15, 2023)
#
# Usage:
#   find_package(SpirvTools [REQUIRED] [QUIET])
#
# Once done this will define:
#   SPIRVTOOLS_FOUND       - system has SpirvTools
#   SPIRVTOOLS_INCLUDE_DIR - the paths to the include directories
#   SPIRVTOOLS_LIBRARIES   - the paths to the SpirvTools libraries
#

find_path(SPIRVTOOLS_INCLUDE_DIR NAMES "spirv-tools")

find_library(SPIRVTOOLS_LIBRARY NAMES "SPIRV-Tools")
find_library(SPIRVTOOLS_OPT_LIBRARY NAMES "SPIRV-Tools-Opt")

set(SPIRVTOOLS_LIBRARIES)
if(SPIRVTOOLS_LIBRARY)
  list(APPEND SPIRVTOOLS_LIBRARIES "${SPIRVTOOLS_LIBRARY}")
endif()
if(SPIRVTOOLS_OPT_LIBRARY)
  list(APPEND SPIRVTOOLS_LIBRARIES "${SPIRVTOOLS_OPT_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SpirvTools DEFAULT_MSG SPIRVTOOLS_INCLUDE_DIR SPIRVTOOLS_LIBRARIES)
