# Filename: FindODE.cmake
# Author: CFSworks (7 Feb, 2014)
#
# Usage:
#   find_package(ODE [REQUIRED] [QUIET])
#
# Once done this will define:
#   ODE_FOUND       - system has ode
#   ODE_INCLUDE_DIR - the ode include directory
#   ODE_LIBRARY_DIR - the ode library directory
#   ODE_LIBRARY     - the path to the library binary
#
#   ODE_RELEASE_LIBRARY - the filepath of the ode release library
#   ODE_DEBUG_LIBRARY   - the filepath of the ode debug library
#

if(NOT ODE_INCLUDE_DIR OR NOT ODE_LIBRARY_DIR)
	# Find the libode include files
	find_path(ODE_INCLUDE_DIR
		NAMES "ode.h"
		PATHS "/usr/include"
		      "/usr/local/include"
		      "/sw/include"
		      "/opt/include"
		      "/opt/local/include"
		      "/opt/csw/include"
		PATH_SUFFIXES "" "ode"
	)

	# Find the libode library built for release
	find_library(ODE_RELEASE_LIBRARY
		NAMES "ode" "libode"
		PATHS "/usr"
		      "/usr/local"
		      "/usr/freeware"
		      "/sw"
		      "/opt"
		      "/opt/csw"
		PATH_SUFFIXES "lib" "lib32" "lib64"
	)

	# Find the libode library built for debug
	find_library(ODE_DEBUG_LIBRARY
		NAMES "oded" "liboded"
		PATHS "/usr"
		      "/usr/local"
		      "/usr/freeware"
		      "/sw"
		      "/opt"
		      "/opt/csw"
		PATH_SUFFIXES "lib" "lib32" "lib64"
	)


	mark_as_advanced(ODE_INCLUDE_DIR)
	mark_as_advanced(ODE_RELEASE_LIBRARY)
	mark_as_advanced(ODE_DEBUG_LIBRARY)
endif()

# Choose library
if(CMAKE_BUILD_TYPE MATCHES "Debug" AND ODE_DEBUG_LIBRARY)	
	set(ODE_LIBRARY ${ODE_DEBUG_LIBRARY} CACHE FILEPATH "The filepath to libode's library binary.")
elseif(ODE_RELEASE_LIBRARY)	
	set(ODE_LIBRARY ${ODE_RELEASE_LIBRARY} CACHE FILEPATH "The filepath to libode's library binary.")
elseif(ODE_DEBUG_LIBRARY)	
	set(ODE_LIBRARY ${ODE_DEBUG_LIBRARY} CACHE FILEPATH "The filepath to libode's library binary.")
endif()

# Translate library into library directory
if(ODE_LIBRARY)
	unset(ODE_LIBRARY_DIR CACHE)
	get_filename_component(ODE_LIBRARY_DIR "${ODE_LIBRARY}" PATH)
	set(ODE_LIBRARY_DIR "${ODE_LIBRARY_DIR}" CACHE PATH "The path to libode's library directory.") # Library path
endif()

mark_as_advanced(ODE_LIBRARY)
mark_as_advanced(ODE_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODE DEFAULT_MSG ODE_LIBRARY ODE_INCLUDE_DIR ODE_LIBRARY_DIR)
