# Filename: FindOpenGLES1.cmake
# Authors: CFSworks (21 Oct, 2018)
#
# Usage:
#   find_package(OpenGLES1 [REQUIRED] [QUIET])
#
# Once done this will define:
#   OPENGLES1_FOUND       - system has OpenGL ES 1.x
#   OPENGLES1_INCLUDE_DIR - the include directory containing GLES/gl.h
#   OPENGLES1_LIBRARY     - the library to link against for OpenGL ES 1.x
#

find_path(OPENGLES1_INCLUDE_DIR "GLES/gl.h")

find_library(OPENGLES1_LIBRARY
  NAMES "GLESv1" "GLESv1_CM" "GLES_CM")

mark_as_advanced(OPENGLES1_INCLUDE_DIR OPENGLES1_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES1 DEFAULT_MSG OPENGLES1_INCLUDE_DIR OPENGLES1_LIBRARY)
