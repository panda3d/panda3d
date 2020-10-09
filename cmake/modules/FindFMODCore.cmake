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
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc"
  PATH_SUFFIXES "" "fmod/fmod" "fmod"
  DOC "The path to FMOD Core's include directory."
)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(fmod_win_lib_path "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64")
else()
  set(fmod_win_lib_path "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x86")
endif()

# Find the library directory
find_library(FMODCORE_LIBRARY
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
        "/usr/freeware"
        ${fmod_win_lib_path}
  PATH_SUFFIXES "" "lib" "lib32" "lib64"
  DOC "The filepath to FMOD Core's library binary."
)

get_filename_component(FMODCORE_LIBRARY_DIR "${FMODCORE_LIBRARY}" PATH)
set(FMODCORE_LIBRARY_DIR "${FMODCORE_LIBRARY_DIR}" CACHE PATH "The path to FMOD Core's library directory.")

mark_as_advanced(FMODCORE_INCLUDE_DIR)
mark_as_advanced(FMODCORE_LIBRARY_DIR)
mark_as_advanced(FMODCORE_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMODCore DEFAULT_MSG FMODCORE_LIBRARY FMODCORE_INCLUDE_DIR FMODCORE_LIBRARY_DIR)
