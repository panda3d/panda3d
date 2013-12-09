# Filename: FindTar.cmake
# Author: kestred (29 Nov, 2013)
#
# Usage:
#   find_package(Tar [REQUIRED] [QUIET])
#
# It sets the following variables:
#   FOUND_VRPN  - system has libvrpn
#   VRPN_IPATH - the vrpn include directory
#   VRPN_LPATH - the vrpn library directory
#

if(VRPN_IPATH AND VRPN_LPATH)
	set(FOUND_VRPN TRUE)
else()
	# Find the vrpn include files
	find_path(VRPN_IPATH
		NAMES "vrpn_Keyboard.h"
		PATHS "/usr/include"
		      "/usr/local/include"
		      "/opt/vrpn/include"
		PATH_SUFFIXES "" "vrpn"
		DOC "The path to vrpn's include directory."
	)

	# Find the libvrpn library (.a, .so)
	find_library(VRPN_LIBRARY
		NAMES "vrpn"
		      "libvrpn"
		PATHS "/usr"
		      "/usr/local"
		      "/opt/vrpn"
		PATH_SUFFIXES "lib" "lib32" "lib64"
	)
	get_filename_component(VRPN_LIBRARY_DIR "${VRPN_LIBRARY}" PATH)
	set(VRPN_LPATH "${VRPN_LIBRARY_DIR}" CACHE PATH "The path to vrpn's library directory.") # Library path

	# Check if we have everything we need
	if(VRPN_IPATH AND VRPN_LPATH)
		set(FOUND_VRPN TRUE)
	endif()

	unset(VRPN_LIBRARY_DIR)
	unset(VRPN_LIBRARY CACHE)
	mark_as_advanced(VRPN_IPATH)
	mark_as_advanced(VRPN_LPATH)
endif()
