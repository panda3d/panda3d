# Filename: FindEGL.cmake
# Authors: CFSworks (21 Oct, 2018)
#
# Usage:
#   find_package(EGL [REQUIRED] [QUIET])
#
# Once done this will define:
#   EGL_FOUND        - system has EGL
#   EGL_INCLUDE_DIR  - the include directory containing EGL/egl.h
#   EGL_LIBRARY      - the library to link against for EGL
#

find_path(EGL_INCLUDE_DIR "EGL/egl.h")

find_library(EGL_LIBRARY
  NAMES "EGL")

mark_as_advanced(EGL_INCLUDE_DIR EGL_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EGL DEFAULT_MSG EGL_INCLUDE_DIR EGL_LIBRARY)
