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

find_path(OGG_INCLUDE_DIR NAMES "ogg/ogg.h")

find_library(OGG_LIBRARY NAMES "ogg" "libogg_static")

mark_as_advanced(OGG_INCLUDE_DIR OGG_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg DEFAULT_MSG OGG_INCLUDE_DIR OGG_LIBRARY)
