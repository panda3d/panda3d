# Filename: FindFFTW.cmake
# Author: Unknown (???), kestred (29 Nov, 2013)
#
# Usage:
#   find_package(FFTW [REQUIRED] [QUIET])
#
# Once done this will define:
#   FFTW_FOUND       - true if fftw is found on the system
#   FFTW_INCLUDE_DIR - the fftw include directory
#   FFTW_LIBRARY_DIR - the fftw library directory
#   FFTW_LIBRARY     - the path to the library binary
#
# The following variables will be checked by the function
#   FFTW_ROOT - if set, the libraries are exclusively searched under this path
#

if(NOT FFTW_INCLUDE_DIR OR NOT FFTW_LIBRARY_DIR)
  # Check if we can use PkgConfig
  find_package(PkgConfig QUIET)

  #Determine from PKG
  if(PKG_CONFIG_FOUND AND NOT FFTW_ROOT)
    pkg_check_modules(PKG_FFTW QUIET "fftw3")
  endif()

  if(FFTW_ROOT)
    # Try to find headers under root
    find_path(FFTW_INCLUDE_DIR
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
    find_path(FFTW_INCLUDE_DIR
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

  # Get the directory conaining FFTW_LIBRARY
  get_filename_component(FFTW_LIBRARY_DIR "${FFTW_LIBRARY}" PATH)
  set(FFTW_LIBRARY_DIR "${FFTW_LIBRARY_DIR}" CACHE PATH "The path to fftw's library directory.") # Library path

  mark_as_advanced(FFTW_INCLUDE_DIR)
  mark_as_advanced(FFTW_LIBRARY_DIR)
  mark_as_advanced(FFTW_LIBRARY)
  mark_as_advanced(FFTW_FFTWF_LIBRARY)
  mark_as_advanced(FFTW_FFTWL_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW DEFAULT_MSG FFTW_LIBRARY FFTW_INCLUDE_DIR FFTW_LIBRARY_DIR)