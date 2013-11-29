# Filename: FindFFTW.cmake
# Author: Unknown (???), kestred (29 Nov, 2013)
#
# Usage:
#   find_package(FFTW [REQUIRED] [QUIET])
#
# It sets the following variables:
#  HAVE_FFTW  - true if fftw is found on the system
#  FFTW_IPATH - the fftw include directory
#  FFTW_LPATH - the fftw library directory
#
# The following variables will be checked by the function
#   FFTW_USE_STATIC_LIBS - if true, only static libraries are found
#   FFTW_ROOT            - if set, the libraries are exclusively searched under this path
#

if(FFTW_IPATH AND FFTW_LPATH)
	set(HAVE_FFTW TRUE)
else()
	# Check if we can use PkgConfig
	find_package(PkgConfig QUIET)

	#Determine from PKG
	if(PKG_CONFIG_FOUND AND NOT FFTW_ROOT)
		pkg_check_modules(PKG_FFTW QUIET "fftw3")
	endif()

	#Check whether to search static or dynamic libs
	set(CMAKE_FIND_LIBRARY_SUFFIXES_SAVED ${CMAKE_FIND_LIBRARY_SUFFIXES})

	if(${FFTW_USE_STATIC_LIBS})
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
	else()
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
	endif()


	if(FFTW_ROOT)
		# Try to find headers under root
		find_path(FFTW_IPATH
			NAMES "fftw3.h"
			PATHS ${FFTW_ROOT}
			PATH_SUFFIXES "include"
			NO_DEFAULT_PATH
		)

		# Try to find library under root
		find_library(FFTW_LIBRARY
			NAMES "fftw3"
			PATHS ${FFTW_ROOT}
			PATH_SUFFIXES "lib" "lib64"
			NO_DEFAULT_PATH
		)

		find_library(FFTW_FFTWF_LIBRARY
			NAMES "fftw3f"
			PATHS ${FFTW_ROOT}
			PATH_SUFFIXES "lib" "lib64"
			NO_DEFAULT_PATH
		)

		find_library(FFTW_FFTWL_LIBRARY
			NAMES "fftw3l"
			PATHS ${FFTW_ROOT}
			PATH_SUFFIXES "lib" "lib64"
			NO_DEFAULT_PATH
		)
	else()
		# Find headers the normal way
		find_path(FFTW_IPATH
			NAMES "fftw3.h"
			PATHS ${PKG_FFTW_INCLUDE_DIRS}
			      ${INCLUDE_INSTALL_DIR}
			      "/usr/include"
			      "/usr/local/include"
			PATH_SUFFIXES "" "fftw"
		)

		# Find library the normal way
		find_library(FFTW_LIBRARY
			NAMES "fftw3"
			PATHS ${PKG_FFTW_LIBRARY_DIRS}
			      ${LIB_INSTALL_DIR}
			      "/usr"
			      "/usr/local"
			PATH_SUFFIXES "" "lib" "lib32" "lib64"
		)

		find_library(FFTW_FFTWF_LIBRARY
			NAMES "fftw3f"
			PATHS ${PKG_FFTW_LIBRARY_DIRS}
			      ${LIB_INSTALL_DIR}
			      "/usr"
			      "/usr/local"
			PATH_SUFFIXES "" "lib" "lib32" "lib64"
		)


		find_library(FFTW_FFTWL_LIBRARY
			NAMES "fftw3l"
			PATHS ${PKG_FFTW_LIBRARY_DIRS}
			      ${LIB_INSTALL_DIR}
			      "/usr"
			      "/usr/local"
			PATH_SUFFIXES "" "lib" "lib32" "lib64"
		)
	endif()

	# Cleanup after the library suffixes
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAVED})
	unset(CMAKE_FIND_LIBRARY_SUFFIXES_SAVED)

	# Get the directory conaining FFTW_LIBRARY
	get_filename_component(FFTW_LIBRARY_DIR "${FFTW_LIBRARY}" PATH)
	set(FFTW_LPATH "${FFTW_LIBRARY_DIR}" CACHE PATH "The path to fftw's library directory.") # Library path


	# Check if we have everything we need
	if(FFTW_IPATH AND FFTW_LPATH)
		set(HAVE_FFTW TRUE)
	endif()

	unset(FFTW_LIBRARY_DIR)
	unset(FFTW_LIBRARY CACHE)
	mark_as_advanced(FFTW_IPATH)
	mark_as_advanced(FFTW_LPATH)
	mark_as_advanced(FFTW_FFTWF_LIBRARY)
	mark_as_advanced(FFTW_FFTWL_LIBRARY)
endif()
