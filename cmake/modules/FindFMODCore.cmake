# Filename: FindFMODCore.cmake
# Author: lachbr (4 Oct, 2020)
#
# Usage:
#   find_package(FMODCore [REQUIRED] [QUIET])
#
# Once done this will define:
#   FMODCORE_FOUND       - system has FMOD Core
#   FMODCORE_FOUND_INCLUDE_DIR - the FMOD Core include directory
#   FMODCORE_FOUND_LIBRARY_DIR - the FMOD Core library directory
#   FMODCORE_FOUND_LIBRARY     - the path to the library binary
#
#   FMODCORE_FOUND_32_LIBRARY - the filepath of the FMOD Core SDK 32-bit library
#   FMODCORE_FOUND_64_LIBRARY - the filepath of the FMOD Core SDK 64-bit library
#

# Find the include directory
find_path(FMODCORE_INCLUDE_DIR
  NAMES "fmod.h"
  PATHS "/usr/include"
        "/usr/local/include"
        "/sw/include"
        "/opt/include"
        "/opt/local/include"
        "/opt/csw/include"
        "/opt/fmod/include"
        "/opt/fmod/api/inc"
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Win32/api/inc"
  PATH_SUFFIXES "" "fmod/fmod" "fmod/fmod3" "fmod" "fmod3"
  DOC "The path to FMOD Core's include directory."
)

# Find the 32-bit library
find_library(FMODCORE_32_LIBRARY
  NAMES "fmod_vc" "fmod_bc" "fmod" "fmodL" "libfmod" "libfmodL" "fmod_vc" "fmodL_vc"
  PATHS "/usr"
        "/usr/local"
        "/usr/X11R6"
        "/usr/local/X11R6"
        "/sw"
        "/opt"
        "/opt/local"
        "/opt/csw"
        "/opt/fmod"
        "/opt/fmod/api"
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Win32/api/lib"
  PATH_SUFFIXES "" "lib" "lib32"
)

# Find the 64-bit library
find_library(FMODCORE_64_LIBRARY
  NAMES "fmod64" "libfmod64" "fmodL64" "libfmodL64" "fmod64_vc" "fmodL64_vc"
  PATHS "/usr"
        "/usr/local"
        "/usr/X11R6"
        "/usr/local/X11R6"
        "/sw"
        "/opt"
        "/opt/local"
        "/opt/csw"
        "/opt/fmod"
        "/opt/fmod/api"
        "/usr/freeware"
  PATH_SUFFIXES "" "lib" "lib64"
)

if(FMODCORE_32_LIBRARY)
  set(FMODCORE_LIBRARY ${FMODCORE_32_LIBRARY} CACHE FILEPATH "The filepath to FMOD Core's library binary.")
elseif(FMODCORE_64_LIBRARY)
  set(FMODCORE_LIBRARY ${FMODCORE_64_LIBRARY} CACHE FILEPATH "The filepath to FMOD Core's library binary.")
endif()

get_filename_component(FMODCORE_LIBRARY_DIR "${FMODCORE_LIBRARY}" PATH)
set(FMODCORE_LIBRARY_DIR "${FMODCORE_LIBRARY_DIR}" CACHE PATH "The path to FMOD Core's library directory.")

mark_as_advanced(FMODCORE_INCLUDE_DIR)
mark_as_advanced(FMODCORE_LIBRARY_DIR)
mark_as_advanced(FMODCORE_LIBRARY)
mark_as_advanced(FMODCORE_32_LIBRARY)
mark_as_advanced(FMODCORE_64_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMODCore DEFAULT_MSG FMODCORE_LIBRARY FMODCORE_INCLUDE_DIR FMODCORE_LIBRARY_DIR)
