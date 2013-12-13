# Filename: FindMiles.cmake
# Author: kestred (9 Dec, 2013)
#
# Usage:
#   find_package(Miles [REQUIRED] [QUIET])
#
# Once done this will define:
#   MILES_FOUND       - system has Radgame's Miles SDK
#   RAD_MSS_FOUND     - system has Radgame's Miles SDK
#   MILES_INCLUDE_DIR - the Miles SDK include directory
#   MILES_LIBRARY_DIR - the Miles SDK library directory
#   MILES_LIBRARY     - the path to the library binary
#
#   MILES_RELEASE_LIBRARY - the filepath of the Miles SDK release library
#   MILES_RELDBG_LIBRARY  - the filepath of the Miles SDK optimize debug library
#   MILES_MINSIZE_LIBRARY - the filepath of the Miles SDK minimum size library
#   MILES_DEBUG_LIBRARY   - the filepath of the Miles SDK debug library
#

if(NOT MILES_INCLUDE_DIR OR NOT MILES_LIBRARY_DIR)
  # Find the Miles SDK include files
  find_path(MILES_INCLUDE_DIR
    NAMES "miles.h"
    PATHS "/usr/include"
          "/usr/local/include"
          "/opt/"
          "C:/Program Files"
          "C:/Program Files (x86)"
    PATH_SUFFIXES "" "miles" "Miles6" "miles/include" "Miles6/include"
    DOC "The path to the Miles SDK include directory."
  )

  # Find the Miles SDK libraries (.a, .so)
  find_library(MILES_RELEASE_LIBRARY
    NAMES "miles"
    PATHS "/usr"
          "/usr/local"
          "/opt/miles"
          "/opt/Miles6"
          "C:/Program Files/miles"
          "C:/Program Files (x86)/miles"
          "C:/Program Files/Miles6"
          "C:/Program Files (x86)/Miles6"
    PATH_SUFFIXES "lib" "lib32"
  )
  find_library(MILES_MINSIZE_LIBRARY
    NAMES "miles_s"
    PATHS "/usr"
          "/usr/local"
          "/opt/miles"
          "C:/Program Files/miles"
          "C:/Program Files (x86)/miles"
          "C:/Program Files/Miles6"
          "C:/Program Files (x86)/Miles6"
    PATH_SUFFIXES "lib" "lib32"
  )
  find_library(MILES_RELWITHDEBINFO_LIBRARY
    NAMES "miles_rd"
    PATHS "/usr"
          "/usr/local"
          "/opt/miles"
          "C:/Program Files/miles"
          "C:/Program Files (x86)/miles"
          "C:/Program Files/Miles6"
          "C:/Program Files (x86)/Miles6"
    PATH_SUFFIXES "lib" "lib32"
  )
  find_library(MILES_DEBUG_LIBRARY
    NAMES "miles_d"
    PATHS "/usr"
          "/usr/local"
          "/opt/miles"
          "C:/Program Files/miles"
          "C:/Program Files (x86)/miles"
          "C:/Program Files/Miles6"
          "C:/Program Files (x86)/Miles6"
    PATH_SUFFIXES "lib" "lib32"
  )

  # Choose library
  if(CMAKE_BUILD_TYPE MATCHES "Release" AND MILES_RELEASE_LIBRARY)
    set(MILES_LIBRARY ${MILES_RELEASE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" AND MILES_RELDBG_LIBRARY)
    set(MILES_LIBRARY ${MILES_RELWITHDEBINFO_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel" AND MILES_MINSIZE_LIBRARY)
    set(MILES_LIBRARY ${MILES_MINSIZE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "Debug" AND MILES_DEBUG_LIBRARY)
    set(MILES_LIBRARY ${MILES_DEBUG_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  endif()

  # Set library path
  get_filename_component(MILES_LIBRARY_DIR "${MILES_LIBRARY}" PATH)
  set(MILES_LIBRARY_DIR "${MILES_LIBRARY_DIR}" CACHE PATH "The path to the Miles SDK library directory.")

  # Check if we have everything we need
  if(MILES_INCLUDE_DIR AND MILES_LIBRARY_DIR)
    set(FOUND_MILES TRUE)
    set(MILES_LIBS Mss32)
  endif()

  mark_as_advanced(MILES_INCLUDE_DIR)
  mark_as_advanced(MILES_DEBUG_LIBRARY)
  mark_as_advanced(MILES_RELEASE_LIBRARY)
  mark_as_advanced(MILES_RELWITHDEBINFO_LIBRARY)
  mark_as_advanced(MILES_MINSIZE_LIBRARY)
endif()

# Choose library
if(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" AND MILES_RELDBG_LIBRARY)
  unset(MILES_LIBRARY CACHE)
  set(MILES_LIBRARY ${MILES_RELDBG_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel" AND MILES_MINSIZE_LIBRARY)
  unset(MILES_LIBRARY CACHE)
  set(MILES_LIBRARY ${MILES_MINSIZE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
elseif(CMAKE_BUILD_TYPE MATCHES "Debug" AND MILES_DEBUG_LIBRARY)
  unset(MILES_LIBRARY CACHE)
  set(MILES_LIBRARY ${MILES_DEBUG_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
elseif(MILES_RELEASE_LIBRARY)
  unset(MILES_LIBRARY CACHE)
  set(MILES_LIBRARY ${MILES_RELEASE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
endif()

# Set library path
if(DEFINED MILES_LIBRARY)
  unset(MILES_LIBRARY_DIR CACHE)
  get_filename_component(MILES_LIBRARY_DIR "${MILES_LIBRARY}" PATH)
  set(MILES_LIBRARY_DIR "${MILES_LIBRARY_DIR}" CACHE PATH "The path to the Miles SDK library directory.")
endif()

mark_as_advanced(MILES_LIBRARY)
mark_as_advanced(MILES_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Miles DEFAULT_MSG MILES_LIBRARY MILES_INCLUDE_DIR MILES_LIBRARY_DIR)
