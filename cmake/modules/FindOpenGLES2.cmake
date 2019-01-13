# Filename: FindOpenGLES2.cmake
# Authors: CFSworks (21 Oct, 2018)
#
# Usage:
#   find_package(OpenGLES2 [REQUIRED] [QUIET])
#
# Once done this will define:
#   OPENGLES2_FOUND        - system has OpenGL ES 2.x
#   OPENGLES2_INCLUDE_DIR  - the include directory containing GLES2/gl2.h
#   OPENGLES2_LIBRARY      - the library to link against for OpenGL ES 2.x
#

find_path(OPENGLES2_INCLUDE_DIR "GLES2/gl2.h")

find_library(OPENGLES2_LIBRARY
  NAMES "GLESv2")

mark_as_advanced(OPENGLES2_INCLUDE_DIR OPENGLES2_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES2 DEFAULT_MSG OPENGLES2_INCLUDE_DIR OPENGLES2_LIBRARY)
