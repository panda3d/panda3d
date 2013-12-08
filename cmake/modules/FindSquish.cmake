# Filename: FindSquish.cmake
# Author: kestred (7 Dec, 2013)
#
# Usage:
#   find_package(Squish [REQUIRED] [QUIET])
#
# It sets the following variables:
#   FOUND_SQUISH  - system has squish
#   SQUISH_IPATH - the squish include directory
#   SQUISH_LPATH - the squish library directory
#   SQUISH_RELEASE_LIBRARY - the filepath of the squish release library
#   SQUISH_DEBUG_LIBRARY - the filepath of the squish debug library
#   SQUISH_LIBS  - the squish components found
#

if(SQUISH_IPATH AND SQUISH_LPATH)
	set(FOUND_SQUISH TRUE)
	set(SQUISH_LIBS squish)

	# Use the library appropriate to the build type
	if(CMAKE_BUILD_TYPE MATCHES "Debug" AND SQUISH_DEBUG_LIBRARY)
		set(SQUISH_LIBRARY ${SQUISH_DEBUG_LIBRARY})
	elseif(SQUISH_RELEASE_LIBRARY)
		set(SQUISH_LIBRARY ${SQUISH_RELEASE_LIBRARY})
	endif()

	# Update Squish library path
	if(SQUISH_LIBRARY)
		get_filename_component(SQUISH_LIBRARY_DIR "${SQUISH_LIBRARY}" PATH)
		set(SQUISH_LPATH "${SQUISH_LIBRARY_DIR}" CACHE PATH "The path to libsquish's library directory.") # Library path
		unset(SQUISH_LIBRARY_DIR)
		unset(SQUISH_LIBRARY CACHE)
	endif()
else()
	# Find the libsquish include files
	find_path(SQUISH_IPATH
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

	# Use the library appropriate to the build type
	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		if(SQUISH_DEBUG_LIBRARY)
			set(SQUISH_LIBRARY ${SQUISH_DEBUG_LIBRARY})
		else()
			set(SQUISH_LIBRARY ${SQUISH_RELEASE_LIBRARY})
		endif()
	elseif(SQUISH_RELEASE_LIBRARY)
		set(SQUISH_LIBRARY ${SQUISH_RELEASE_LIBRARY})
	else()
		set(SQUISH_LIBRARY ${SQUISH_DEBUG_LIBRARY})
	endif()

	# Translate library into library path
	get_filename_component(SQUISH_LIBRARY_DIR "${SQUISH_LIBRARY}" PATH)
	set(SQUISH_LPATH "${SQUISH_LIBRARY_DIR}" CACHE PATH "The path to libsquish's library directory.") # Library path

	# Check if we have everything we need
	if(SQUISH_IPATH AND SQUISH_LPATH)
		set(FOUND_SQUISH TRUE)
		set(SQUISH_LIBS squish)
	endif()

	unset(SQUISH_LIBRARY_DIR)
	unset(SQUISH_LIBRARY CACHE)
	mark_as_advanced(SQUISH_IPATH)
	mark_as_advanced(SQUISH_LPATH)
	mark_as_advanced(SQUISH_RELEASE_LIBRARY)
	mark_as_advanced(SQUISH_DEBUG_LIBRARY)
endif()