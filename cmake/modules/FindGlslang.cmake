# Filename: FindGlslang.cmake
# Authors: RegDogg (Feb 15, 2023)
#
# Usage:
#   find_package(Glslang [REQUIRED] [QUIET])
#
# Once done this will define:
#   GLSLANG_FOUND       - system has Glslang
#   GLSLANG_INCLUDE_DIR - the path to the include directory
#   GLSLANG_LIBRARIES   - the paths to the Glslang libraries
#

find_path(GLSLANG_INCLUDE_DIR NAMES "glslang")

find_library(GLSLANG_LIBRARY NAMES "glslang")
find_library(GLSLANG_DefaultResourceLimits_LIBRARY NAMES "glslang-default-resource-limits")
find_library(GLSLANG_GenericCodeGen_LIBRARY NAMES "GenericCodeGen")
find_library(GLSLANG_HLSL_LIBRARY NAMES "HLSL")
find_library(GLSLANG_MachineIndependent_LIBRARY NAMES "MachineIndependent")
find_library(GLSLANG_OGLCompiler_LIBRARY NAMES "OGLCompiler")
find_library(GLSLANG_OSDependent_LIBRARY NAMES "OSDependent")
find_library(GLSLANG_SPIRV_LIBRARY NAMES "SPIRV")

set(GLSLANG_LIBRARIES)
if(GLSLANG_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_LIBRARY}")
endif()
if(GLSLANG_DefaultResourceLimits_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_DefaultResourceLimits_LIBRARY}")
endif()
if(GLSLANG_GenericCodeGen_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_GenericCodeGen_LIBRARY}")
endif()
if(GLSLANG_HLSL_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_HLSL_LIBRARY}")
endif()
if(GLSLANG_MachineIndependent_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_MachineIndependent_LIBRARY}")
endif()
if(GLSLANG_OGLCompiler_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_OGLCompiler_LIBRARY}")
endif()
if(GLSLANG_OSDependent_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_OSDependent_LIBRARY}")
endif()
if(GLSLANG_SPIRV_LIBRARY)
  list(APPEND GLSLANG_LIBRARIES "${GLSLANG_SPIRV_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glslang DEFAULT_MSG GLSLANG_INCLUDE_DIR GLSLANG_LIBRARIES)
