# Filename: FindFMODEx.cmake
# Author: kestred (8 Dec, 2013)
#
# Usage:
#   find_package(FMODEx [REQUIRED] [QUIET])
#
# It sets the following variables:
#   FOUND_FMODEX   - system has FMOD Ex
#   FMODEX_IPATH   - the FMOD Ex include directory
#   FMODEX_LPATH   - the FMOD Ex library directory
#   FMODEX_LIBS    - the FMOD Ex components found
#

if(FMODEX_LPATH AND FMODEX_IPATH)
	# If its cached, we don't need to refind it
	set(FOUND_FMODEX TRUE)

	if(FMODEX_32_LIBRARY)
		set(FMODEX_LIBS fmodex)
	elseif(FMODEX_64_LIBRARY)
		set(FMODEX_LIBS fmodex64)
	endif()
else()
	# Find the include directory
	find_path(FMODEX_IPATH
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
		PATH_SUFFIXES "" "fmodex/fmod" "fmodex/fmod3" "fmod" "fmod3"
		DOC "The path to FMOD Ex's include directory."
	)

	# Find the 32-bit library
	find_library(FMODEX_32_LIBRARY
		NAMES "fmodex_vc" "fmodex_bc" "fmodex" "fmodexL" "libfmodex" "libfmodexL"
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
		NAMES "fmodex64" "libfmodex64" "fmodexL64" "libfmodexL64"
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
		PATH_SUFFIXES "" "lib" "lib64"
	)

	if(FMODEX_32_LIBRARY)
		set(FMODEX_LIBRARY ${FMODEX_32_LIBRARY})
		set(FMODEX_LIBS fmodex)
	elseif(FMODEX_64_LIBRARY)
		set(FMODEX_LIBRARY ${FMODEX_64_LIBRARY})
		set(FMODEX_LIBS fmodex64)
	endif()

	get_filename_component(FMODEX_LIBRARY_DIR "${FMODEX_LIBRARY}" PATH)
	set(FMODEX_LPATH "${FMODEX_LIBRARY_DIR}" CACHE PATH "The path to FMOD Ex's library directory.")

	# Check if we have everything we need
	if(FMODEX_IPATH AND FMODEX_LPATH)
		set(FOUND_FMODEX TRUE)
	endif()

	mark_as_advanced(FMODEX_IPATH)
	mark_as_advanced(FMODEX_LPATH)
	mark_as_advanced(FMODEX_32_LIBRARY)
	mark_as_advanced(FMODEX_64_LIBRARY)
	unset(FMODEX_LIBRARY_DIR)
	unset(FMODEX_LIBRARY CACHE)
endif()
