# Filename: FindFFMPEG.cmake
# Author: CFSworks (10 Apr, 2014)
#
# Usage:
#   find_package(FFMPEG [REQUIRED] [QUIET])
#
# Once done this will define:
#   FFMPEG_FOUND       - system has ffmpeg
#   FFMPEG_INCLUDE_DIR - the ffmpeg include directory
#   FFMPEG_LIBRARIES   - the path to the library binary
#
#   FFMPEG_LIBAVCODEC  - the path to the libavcodec library binary
#   FFMPEG_LIBAVFORMAT - the path to the libavformat library binary
#   FFMPEG_LIBAVUTIL   - the path to the libavutil library binary
#

# Find the libffmpeg include files
find_path(FFMPEG_INCLUDE_DIR
  NAMES "libavcodec/avcodec.h"
  PATHS "/usr/include"
        "/usr/local/include"
        "/sw/include"
        "/opt/include"
        "/opt/local/include"
        "/opt/csw/include"
  PATH_SUFFIXES "libav" "ffmpeg"
)

# Find the libavcodec library
find_library(FFMPEG_LIBAVCODEC
  NAMES "avcodec"
  PATHS "/usr"
        "/usr/local"
        "/usr/freeware"
        "/sw"
        "/opt"
        "/opt/csw"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

# Find the libavformat library
find_library(FFMPEG_LIBAVFORMAT
  NAMES "avformat"
  PATHS "/usr"
        "/usr/local"
        "/usr/freeware"
        "/sw"
        "/opt"
        "/opt/csw"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

# Find the libavutil library
find_library(FFMPEG_LIBAVUTIL
  NAMES "avutil"
  PATHS "/usr"
        "/usr/local"
        "/usr/freeware"
        "/sw"
        "/opt"
        "/opt/csw"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

mark_as_advanced(FFMPEG_INCLUDE_DIR)
mark_as_advanced(FFMPEG_LIBAVCODEC)
mark_as_advanced(FFMPEG_LIBAVFORMAT)
mark_as_advanced(FFMPEG_LIBAVUTIL)

# Translate library into library directory
if(FFMPEG_LIBAVCODEC)
  unset(FFMPEG_LIBRARY_DIR CACHE)
  get_filename_component(FFMPEG_LIBRARY_DIR "${FFMPEG_LIBAVCODEC}" PATH)
  set(FFMPEG_LIBRARY_DIR "${FFMPEG_LIBRARY_DIR}" CACHE PATH "The path to libffmpeg's library directory.") # Library path
endif()

set(FFMPEG_LIBRARIES)
if(FFMPEG_LIBAVCODEC)
  list(APPEND FFMPEG_LIBRARIES "${FFMPEG_LIBAVCODEC}")
endif()
if(FFMPEG_LIBAVFORMAT)
  list(APPEND FFMPEG_LIBRARIES "${FFMPEG_LIBAVFORMAT}")
endif()
if(FFMPEG_LIBAVUTIL)
  list(APPEND FFMPEG_LIBRARIES "${FFMPEG_LIBAVUTIL}")
endif()

if(APPLE)
  # When statically built for Apple, FFMPEG may have dependencies on these
  # additional frameworks and libraries.

  find_library(APPLE_COREVIDEO_LIBRARY CoreVideo)
  if(APPLE_COREVIDEO_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES "${APPLE_COREVIDEO_LIBRARY}")
  endif()

  find_library(APPLE_VDA_LIBRARY VideoDecodeAcceleration)
  if(APPLE_VDA_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES "${APPLE_VDA_LIBRARY}")
  endif()

  find_library(APPLE_ICONV_LIBRARY iconv)
  if(APPLE_ICONV_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES "${APPLE_ICONV_LIBRARY}")
  endif()

  find_library(APPLE_BZ2_LIBRARY bz2)
  if(APPLE_BZ2_LIBRARY)
    list(APPEND FFMPEG_LIBRARIES "${APPLE_BZ2_LIBRARY}")
  endif()
endif()

mark_as_advanced(FFMPEG_LIBRARY_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG DEFAULT_MSG FFMPEG_LIBRARIES FFMPEG_LIBAVCODEC FFMPEG_LIBAVFORMAT FFMPEG_LIBAVUTIL FFMPEG_INCLUDE_DIR FFMPEG_LIBRARY_DIR)
