# Filename: FindFMODEx.cmake
# Author: kestred (8 Dec, 2013)
#
# Usage:
#   find_package(FMODEx [REQUIRED] [QUIET])
#
# Once done this will define:
#   FMODEX_FOUND       - system has FMOD Ex
#   FMODEX_INCLUDE_DIR - the FMOD Ex include directory
#   FMODEX_LIBRARY_DIR - the FMOD Ex library directory
#   FMODEX_LIBRARY     - the path to the library binary
#
#   FMODEX_32_LIBRARY - the filepath of the FMOD Ex SDK 32-bit library
#   FMODEX_64_LIBRARY - the filepath of the FMOD Ex SDK 64-bit library
#

# Find the include directory
find_path(FMODEX_INCLUDE_DIR
  NAMES "fmod.h"
  PATHS "/usr/include"
        "/usr/local/include"
        "/sw/include"
        "/opt/include"
        "/opt/local/include"
        "/opt/csw/include"
        "/opt/fmodex/include"
        "/opt/fmodex/api/inc"
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Win32/api/inc"
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Win64/api/inc"
        "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Win64/api/inc"
  PATH_SUFFIXES "" "fmodex/fmod" "fmodex/fmod3" "fmod" "fmod3"
  DOC "The path to FMOD Ex's include directory."
)

# Find the 32-bit library
find_library(FMODEX_32_LIBRARY
  NAMES "fmodex_vc" "fmodex_bc" "fmodex" "fmodexL" "libfmodex" "libfmodexL" "fmodex_vc" "fmodexL_vc"
  PATHS "/usr"
        "/usr/local"
        "/usr/X11R6"
        "/usr/local/X11R6"
        "/sw"
        "/opt"
        "/opt/local"
        "/opt/csw"
        "/opt/fmodex"
        "/opt/fmodex/api"
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Win32/api/lib"
  PATH_SUFFIXES "" "lib" "lib32"
)

# Find the 64-bit library
find_library(FMODEX_64_LIBRARY
  NAMES "fmodex64" "libfmodex64" "fmodexL64" "libfmodexL64" "fmodex64_vc" "fmodexL64_vc"
  PATHS "/usr"
        "/usr/local"
        "/usr/X11R6"
        "/usr/local/X11R6"
        "/sw"
        "/opt"
        "/opt/local"
        "/opt/csw"
        "/opt/fmodex"
        "/opt/fmodex/api"
        "/usr/freeware"
        "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Win64/api/lib"
        "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Win64/api/lib"
  PATH_SUFFIXES "" "lib" "lib64"
)

# Prefer the library matching the architecture we are compiling for.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_fmodex_preferred_libs FMODEX_64_LIBRARY FMODEX_32_LIBRARY)
else()
  set(_fmodex_preferred_libs FMODEX_32_LIBRARY FMODEX_64_LIBRARY)
endif()

foreach(_fmodex_lib ${_fmodex_preferred_libs})
  if(${_fmodex_lib})
    set(FMODEX_LIBRARY ${${_fmodex_lib}} CACHE FILEPATH "The filepath to FMOD Ex's library binary.")
    break()
  endif()
endforeach(_fmodex_lib)
unset(_fmodex_preferred_libs)

get_filename_component(FMODEX_LIBRARY_DIR "${FMODEX_LIBRARY}" PATH)
set(FMODEX_LIBRARY_DIR "${FMODEX_LIBRARY_DIR}" CACHE PATH "The path to FMOD Ex's library directory.")

mark_as_advanced(FMODEX_INCLUDE_DIR)
mark_as_advanced(FMODEX_LIBRARY_DIR)
mark_as_advanced(FMODEX_LIBRARY)
mark_as_advanced(FMODEX_32_LIBRARY)
mark_as_advanced(FMODEX_64_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FMODEx DEFAULT_MSG FMODEX_LIBRARY FMODEX_INCLUDE_DIR FMODEX_LIBRARY_DIR)
