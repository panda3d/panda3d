# Filename: FindTar.cmake
# Author: kestred (29 Nov, 2013)
#
# Once done this will define
#
#  HAVE_TAR - system has libtar
#  TAR_IPATH - the tar include directory
#  TAR_LPATH - the tar library directory
#  TAR_LIBS - the tar components found
#

if(TAR_IPATH AND TAR_LPATH)
	set(HAVE_TAR TRUE)
	set(TAR_LIBS tar)
else()
	# Find the libtar include files
	find_path(TAR_IPATH
		NAMES "libtar.h"
		PATHS "/usr/include"
		      "/usr/local/include"
		PATH_SUFFIXES "" "tar" "libtar"
		DOC "The path to libtar's include directory."
	)

	# Find the libtar library (.a, .so)
	find_library(TAR_LIBRARY
		NAMES "tar"
		      "libtar"
		PATHS "/usr"
		      "/usr/local"
		PATH_SUFFIXES "lib" "lib32" "lib64"
	)
	get_filename_component(TAR_LIBRARY_DIR "${TAR_LIBRARY}" PATH)
	set(TAR_LPATH "${TAR_LIBRARY_DIR}" CACHE PATH "The path to libtar's library directory.") # Library path

	if(TAR_IPATH AND TAR_LPATH)
		set(HAVE_TAR TRUE)
		set(TAR_LIBS tar)
		message("BAR")
	endif()

	#include(FindPackageHandleStandardArgs)
	#find_package_handle_standard_args(Tar DEFAULT_MSG TAR_IPATH)

	unset(TAR_LIBRARY_DIR)
	unset(TAR_LIBRARY CACHE)
	mark_as_advanced(TAR_IPATH)
	mark_as_advanced(TAR_LPATH)
endif()
