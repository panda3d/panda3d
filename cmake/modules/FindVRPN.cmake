# Filename: FindVRPN.cmake
# Author: kestred (29 Nov, 2013)
#
# Usage:
#   find_package(VRPN [REQUIRED] [QUIET])
#
# It sets the following variables:
#   VRPN_FOUND   - system has libvrpn
#   VRPN_INCLUDE_DIR   - the vrpn include directory
#   VRPN_LIBRARY_DIR   - the vrpn library directory
#   VRPN_LIBRARY - the path to the library binary
#

if(VRPN_INCLUDE_DIR AND VRPN_LIBRARY_DIR)
	set(FOUND_VRPN TRUE)
else()
	# Find the vrpn include files
	find_path(VRPN_INCLUDE_DIR
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
	set(VRPN_LIBRARY_DIR "${VRPN_LIBRARY_DIR}" CACHE PATH "The path to vrpn's library directory.") # Library path

	mark_as_advanced(VRPN_INCLUDE_DIR)
	mark_as_advanced(VRPN_LIBRARY_DIR)
	mark_as_advanced(VRPN_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRPN DEFAULT_MSG VRPN_LIBRARY VRPN_INCLUDE_DIR VRPN_LIBRARY_DIR)
