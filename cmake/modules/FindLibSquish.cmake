# Filename: FindLibSquish.cmake
# Author: kestred (7 Dec, 2013)
#
# Usage:
#   find_package(LibSquish [REQUIRED] [QUIET])
#
# Once done this will define:
#   LIBSQUISH_FOUND       - system has libsquish
#   LIBSQUISH_INCLUDE_DIR - the libsquish include directory
#   LIBSQUISH_LIBRARY_DIR - the libsquish library directory
#   LIBSQUISH_LIBRARY     - the path to the library binary
#
#   LIBSQUISH_RELEASE_LIBRARY - the filepath of the libsquish release library
#   LIBSQUISH_DEBUG_LIBRARY   - the filepath of the libsquish debug library
#

if(LibSquish_ROOT)
  # Search exclusively under the root
  find_path(LIBSQUISH_INCLUDE_DIR
    NAMES "squish.h"
    PATHS ${LibSquish_ROOT}
    PATH_SUFFIXES "include"
  )

  find_library(LIBSQUISH_RELEASE_LIBRARY
    NAMES "squish" "libsquish"
    PATHS ${LibSquish_ROOT}
    PATH_SUFFIXES "lib"
  )

  find_library(LIBSQUISH_DEBUG_LIBRARY
    NAMES "squishd" "libsquishd"
    PATHS ${LibSquish_ROOT}
    PATH_SUFFIXES "lib"
  )
else()
  # Find the libsquish include files
  find_path(LIBSQUISH_INCLUDE_DIR
    NAMES "squish.h"
    PATHS "/usr/include"
          "/usr/local/include"
          "/sw/include"
          "/opt/include"
          "/opt/local/include"
          "/opt/csw/include"
    PATH_SUFFIXES "" "cppunit"
  )

  # Find the libsquish library built for release
  find_library(LIBSQUISH_RELEASE_LIBRARY
    NAMES "squish" "libsquish"
    PATHS "/usr"
          "/usr/local"
          "/usr/freeware"
          "/sw"
          "/opt"
          "/opt/csw"
    PATH_SUFFIXES "lib" "lib32" "lib64"
  )

  # Find the libsquish library built for debug
  find_library(LIBSQUISH_DEBUG_LIBRARY
    NAMES "squishd" "libsquishd"
    PATHS "/usr"
          "/usr/local"
          "/usr/freeware"
          "/sw"
          "/opt"
          "/opt/csw"
    PATH_SUFFIXES "lib" "lib32" "lib64"
  )
endif()

mark_as_advanced(LIBSQUISH_INCLUDE_DIR)
mark_as_advanced(LIBSQUISH_RELEASE_LIBRARY)
mark_as_advanced(LIBSQUISH_DEBUG_LIBRARY)

# Choose library
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND LIBSQUISH_DEBUG_LIBRARY)
	set(LIBSQUISH_LIBRARY ${LIBSQUISH_DEBUG_LIBRARY} CACHE FILEPATH "The filepath to libsquish's library binary.")
elseif(LIBSQUISH_RELEASE_LIBRARY)
	set(LIBSQUISH_LIBRARY ${LIBSQUISH_RELEASE_LIBRARY} CACHE FILEPATH "The filepath to libsquish's library binary.")
elseif(LIBSQUISH_DEBUG_LIBRARY)
	set(LIBSQUISH_LIBRARY ${LIBSQUISH_DEBUG_LIBRARY} CACHE FILEPATH "The filepath to libsquish's library binary.")
endif()

# Translate library into library directory
if(LIBSQUISH_LIBRARY)
	unset(LIBSQUISH_LIBRARY_DIR CACHE)
	get_filename_component(LIBSQUISH_LIBRARY_DIR "${LIBSQUISH_LIBRARY}" PATH)
	set(LIBSQUISH_LIBRARY_DIR "${LIBSQUISH_LIBRARY_DIR}" CACHE PATH "The path to libsquish's library directory.") # Library path
endif()

mark_as_advanced(LIBSQUISH_LIBRARY)
mark_as_advanced(LIBSQUISH_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibSquish DEFAULT_MSG LIBSQUISH_LIBRARY LIBSQUISH_INCLUDE_DIR LIBSQUISH_LIBRARY_DIR)
