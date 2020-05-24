# Filename: FindVorbisFile.cmake
# Authors: CFSworks (13 Jan, 2019)
#
# Usage:
#   find_package(VorbisFile [REQUIRED] [QUIET])
#
# Once done this will define:
#   VORBISFILE_FOUND        - system has Ogg and vorbisfile
#   VORBISFILE_INCLUDE_DIRS - the include directory/ies containing vorbis/ and ogg/
#   VORBISFILE_LIBRARIES    - the paths to the vorbis and vorbisfile libraries
#

# Find Ogg
find_package(Ogg QUIET)

# Find Vorbis
find_path(VORBIS_INCLUDE_DIR NAMES "vorbis/vorbisfile.h")

find_library(VORBIS_vorbis_LIBRARY NAMES "vorbis" "libvorbis_static")
find_library(VORBIS_vorbisfile_LIBRARY NAMES "vorbisfile" "libvorbisfile_static")

mark_as_advanced(VORBIS_INCLUDE_DIR VORBIS_vorbis_LIBRARY VORBIS_vorbisfile_LIBRARY)

# Define output variables
set(VORBISFILE_INCLUDE_DIRS ${VORBIS_INCLUDE_DIR})
if(NOT OGG_INCLUDE_DIR STREQUAL VORBIS_INCLUDE_DIR)
  list(APPEND VORBISFILE_INCLUDE_DIRS ${OGG_INCLUDE_DIR})
endif()
set(VORBISFILE_LIBRARIES ${VORBIS_vorbisfile_LIBRARY} ${VORBIS_vorbis_LIBRARY} ${OGG_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VorbisFile DEFAULT_MSG
  Ogg_FOUND
  VORBIS_INCLUDE_DIR VORBIS_vorbis_LIBRARY VORBIS_vorbisfile_LIBRARY)
