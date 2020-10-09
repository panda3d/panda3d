# Filename: FindFMOD.cmake
# Author: lachbr (4 Oct, 2020)
#
# Usage:
#   find_package(FMOD [REQUIRED] [QUIET])
#
# Once done this will define:
#   FMOD_FOUND       - system has FMOD
#   FMOD_FOUND_INCLUDE_DIR - the FMOD include directory
#   FMOD_FOUND_LIBRARY_DIR - the FMOD library directory
#   FMOD_FOUND_LIBRARY     - the path to the library binary
#

# Find the include directory
find_path(FMOD_INCLUDE_DIR
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
  DOC "The path to FMOD's include directory."
)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(fmod_win_lib_path "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64")
else()
  set(fmod_win_lib_path "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x86")
endif()

# Find the library directory
find_library(FMOD_LIBRARY
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
  DOC "The filepath to FMOD's library binary."
)

get_filename_component(FMOD_LIBRARY_DIR "${FMOD_LIBRARY}" PATH)
set(FMOD_LIBRARY_DIR "${FMOD_LIBRARY_DIR}" CACHE PATH "The path to FMOD's library directory.")

mark_as_advanced(FMOD_INCLUDE_DIR)
mark_as_advanced(FMOD_LIBRARY_DIR)
mark_as_advanced(FMOD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMOD DEFAULT_MSG FMOD_LIBRARY FMOD_INCLUDE_DIR FMOD_LIBRARY_DIR)
