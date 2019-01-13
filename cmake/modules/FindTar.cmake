# Filename: FindTar.cmake
# Author: kestred (29 Nov, 2013)
#
# Usage:
#   find_package(Tar [REQUIRED] [QUIET])
#
# It sets the following variables:
#   TAR_FOUND   - system has libtar
#   TAR_INCLUDE_DIR   - the tar include directory
#   TAR_LIBRARY_DIR   - the tar library directory
#   TAR_LIBRARY - the path to the library binary
#

# Find the libtar include files
find_path(TAR_INCLUDE_DIR
  NAMES "libtar.h"
  PATHS "/usr/include"
        "/usr/local/include"
  PATH_SUFFIXES "" "tar" "libtar"
  DOC "The path to libtar's include directory."
)

# Find the libtar library (.a, .so)
find_library(TAR_LIBRARY
  NAMES "tar"
        "libtar"
  PATHS "/usr"
        "/usr/local"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)
get_filename_component(TAR_LIBRARY_DIR "${TAR_LIBRARY}" PATH)
set(TAR_LIBRARY_DIR "${TAR_LIBRARY_DIR}" CACHE PATH "The path to libtar's library directory.") # Library path

mark_as_advanced(TAR_INCLUDE_DIR)
mark_as_advanced(TAR_LIBRARY_DIR)
mark_as_advanced(TAR_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tar DEFAULT_MSG TAR_LIBRARY TAR_INCLUDE_DIR TAR_LIBRARY_DIR)
