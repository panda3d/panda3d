# Filename: FindSquish.cmake
# Author: kestred (7 Dec, 2013)
#
# Usage:
#   find_package(Squish [REQUIRED] [QUIET])
#
# Once done this will define:
#   SQUISH_FOUND       - system has squish
#   SQUISH_INCLUDE_DIR - the squish include directory
#   SQUISH_LIBRARY_DIR - the squish library directory
#   SQUISH_LIBRARY     - the path to the library binary
#
#   SQUISH_RELEASE_LIBRARY - the filepath of the squish release library
#   SQUISH_DEBUG_LIBRARY   - the filepath of the squish debug library
#

if(NOT SQUISH_INCLUDE_DIR OR NOT SQUISH_LIBRARY_DIR)
	# Find the libsquish include files
	find_path(SQUISH_INCLUDE_DIR
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
	find_library(SQUISH_RELEASE_LIBRARY
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
	find_library(SQUISH_DEBUG_LIBRARY
		NAMES "squishd" "libsquishd"
		PATHS "/usr"
		      "/usr/local"
		      "/usr/freeware"
		      "/sw"
		      "/opt"
		      "/opt/csw"
		PATH_SUFFIXES "lib" "lib32" "lib64"
	)


	mark_as_advanced(SQUISH_INCLUDE_DIR)
	mark_as_advanced(SQUISH_RELEASE_LIBRARY)
	mark_as_advanced(SQUISH_DEBUG_LIBRARY)
endif()

# Choose library
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND SQUISH_DEBUG_LIBRARY)	
	set(SQUISH_LIBRARY ${SQUISH_DEBUG_LIBRARY} CACHE FILEPATH "The filepath to libsquish's library binary.")
elseif(SQUISH_RELEASE_LIBRARY)	
	set(SQUISH_LIBRARY ${SQUISH_RELEASE_LIBRARY} CACHE FILEPATH "The filepath to libsquish's library binary.")
elseif(SQUISH_DEBUG_LIBRARY)	
	set(SQUISH_LIBRARY ${SQUISH_DEBUG_LIBRARY} CACHE FILEPATH "The filepath to libsquish's library binary.")
endif()

# Translate library into library directory
if(SQUISH_LIBRARY)
	unset(SQUISH_LIBRARY_DIR CACHE)
	get_filename_component(SQUISH_LIBRARY_DIR "${SQUISH_LIBRARY}" PATH)
	set(SQUISH_LIBRARY_DIR "${SQUISH_LIBRARY_DIR}" CACHE PATH "The path to libsquish's library directory.") # Library path
endif()

mark_as_advanced(SQUISH_LIBRARY)
mark_as_advanced(SQUISH_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Squish DEFAULT_MSG SQUISH_LIBRARY SQUISH_INCLUDE_DIR SQUISH_LIBRARY_DIR)
