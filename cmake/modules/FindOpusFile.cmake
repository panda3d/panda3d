# Filename: FindOpusFile.cmake
# Authors: CFSworks (13 Jan, 2019)
#
# Usage:
#   find_package(OpusFile [REQUIRED] [QUIET])
#
# Once done this will define:
#   OPUSFILE_FOUND        - system has Ogg and opusfile
#   OPUSFILE_INCLUDE_DIRS - the include directory/ies containing opus/ and ogg/
#   OPUSFILE_LIBRARIES    - the paths to the opus and opusfile libraries
#

# Find Ogg
find_package(Ogg QUIET)

# Find Opus
find_path(OPUS_INCLUDE_DIR NAMES "opus/opusfile.h")

find_library(OPUS_opus_LIBRARY NAMES "opus")
find_library(OPUS_opusfile_LIBRARY NAMES "opusfile")

mark_as_advanced(OPUS_INCLUDE_DIR OPUS_opus_LIBRARY OPUS_opusfile_LIBRARY)

# Define output variables
set(OPUSFILE_INCLUDE_DIRS ${OPUS_INCLUDE_DIR} "${OPUS_INCLUDE_DIR}/opus")
if(NOT OGG_INCLUDE_DIR STREQUAL OPUS_INCLUDE_DIR)
  list(APPEND OPUSFILE_INCLUDE_DIRS ${OGG_INCLUDE_DIR})
endif()
set(OPUSFILE_LIBRARIES ${OPUS_opusfile_LIBRARY} ${OPUS_opus_LIBRARY} ${OGG_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpusFile DEFAULT_MSG
  Ogg_FOUND
  OPUS_INCLUDE_DIR OPUS_opus_LIBRARY OPUS_opusfile_LIBRARY)
