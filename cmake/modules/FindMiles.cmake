# Filename: FindMiles.cmake
# Author: kestred (9 Dec, 2013)
#
# Usage:
#   find_package(Miles [REQUIRED] [QUIET])
#
# It sets the following variables:
#   FOUND_RAD_MSS   - system has Radgame's Miles SDK
#   RAD_MSS_IPATH   - the Miles SDK include directory
#   RAD_MSS_LPATH   - the Miles SDK library directory
#   RAD_MSS_LIBS    - the Miles SDK components found
#   RAD_MSS_LIBRARY - the path to the library binary
#
#   RAD_MSS_RELEASE_LIBRARY        - the filepath of the Miles SDK release library
#   RAD_MSS_RELWITHDEBINFO_LIBRARY - the filepath of the Miles SDK optimize debug library
#   RAD_MSS_MINSIZE_LIBRARY        - the filepath of the Miles SDK minimum size library
#   RAD_MSS_DEBUG_LIBRARY          - the filepath of the Miles SDK debug library
#

if(RAD_MSS_IPATH AND RAD_MSS_LPATH)
  set(FOUND_RAD_MSS TRUE)
  set(RAD_MSS_LIBS Mss32)


  # Choose library
  if(CMAKE_BUILD_TYPE MATCHES "Release" AND RAD_MSS_RELEASE_LIBRARY)
    unset(RAD_MSS_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_RELEASE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" AND RAD_MSS_RELWITHDEBINFO_LIBRARY)
    unset(RAD_MSS_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_RELWITHDEBINFO_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel" AND RAD_MSS_MINSIZE_LIBRARY)
    unset(RAD_MSS_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_MINSIZE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "Debug" AND RAD_MSS_DEBUG_LIBRARY)
    unset(RAD_MSS_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_DEBUG_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  endif()

  # Set library path
  if(DEFINED RAD_MSS_LIBRARY)
    unset(RAD_MSS_LPATH)
    get_filename_component(RAD_MSS_LIBRARY_DIR "${RAD_MSS_LIBRARY}" PATH)
    set(RAD_MSS_LPATH "${RAD_MSS_LIBRARY_DIR}" CACHE PATH "The path to the Miles SDK library directory.")
    unset(RAD_MSS_LIBRARY_DIR)
  endif()
else()
  # Find the Miles SDK include files
  find_path(RAD_MSS_IPATH
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
  find_library(RAD_MSS_RELEASE_LIBRARY
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
  find_library(RAD_MSS_MINSIZE_LIBRARY
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
  find_library(RAD_MSS_RELWITHDEBINFO_LIBRARY
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
  find_library(RAD_MSS_DEBUG_LIBRARY
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
  if(CMAKE_BUILD_TYPE MATCHES "Release" AND RAD_MSS_RELEASE_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_RELEASE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" AND RAD_MSS_RELWITHDEBINFO_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_RELWITHDEBINFO_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel" AND RAD_MSS_MINSIZE_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_MINSIZE_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  elseif(CMAKE_BUILD_TYPE MATCHES "Debug" AND RAD_MSS_DEBUG_LIBRARY)
    set(RAD_MSS_LIBRARY ${RAD_MSS_DEBUG_LIBRARY} CACHE FILEPATH "The Miles SDK library file.")
  endif()

  # Set library path
  get_filename_component(RAD_MSS_LIBRARY_DIR "${RAD_MSS_LIBRARY}" PATH)
  set(RAD_MSS_LPATH "${RAD_MSS_LIBRARY_DIR}" CACHE PATH "The path to the Miles SDK library directory.")

  # Check if we have everything we need
  if(RAD_MSS_IPATH AND RAD_MSS_LPATH)
    set(FOUND_RAD_MSS TRUE)
    set(RAD_MSS_LIBS Mss32)
  endif()

  unset(RAD_MSS_LIBRARY_DIR)
  mark_as_advanced(RAD_MSS_IPATH)
  mark_as_advanced(RAD_MSS_LPATH)
  mark_as_advanced(RAD_MSS_LIBRARY)
  mark_as_advanced(RAD_MSS_DEBUG_LIBRARY)
  mark_as_advanced(RAD_MSS_RELEASE_LIBRARY)
  mark_as_advanced(RAD_MSS_RELWITHDEBINFO_LIBRARY)
  mark_as_advanced(RAD_MSS_MINSIZE_LIBRARY)
endif()
