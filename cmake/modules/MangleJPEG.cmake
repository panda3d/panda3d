# Filename: MangleJPEG.cmake
# Author: kestred (28 Nov, 2013)
#
# MangleJPEG replaces the output variables of the builtin FindJPEG
# with vars matching Panda3D's existing config-var names.
#

if(JPEG_FOUND)
	set(FOUND_JPEG TRUE)
	set(JPEG_LIBS jpeg)

	get_filename_component(JPEG_LIBRARY_DIR "${JPEG_LIBRARY}" PATH)

	set(JPEG_IPATH "${JPEG_INCLUDE_DIR}" CACHE PATH "The path to libjpeg's include directory.") # Include path
	set(JPEG_LPATH "${JPEG_LIBRARY_DIR}" CACHE PATH "The path to libjpeg's library directory.") # Library path
	mark_as_advanced(JPEG_IPATH)
	mark_as_advanced(JPEG_LPATH)

	unset(JPEG_FOUND)
	unset(JPEG_LIBRARY CACHE)
	unset(JPEG_LIBRARIES)
	unset(JPEG_INCLUDE_DIR CACHE)
	unset(JPEG_LIBRARY_DIR)
else()
	unset(JPEG_LIBRARY CACHE)
	unset(JPEG_INCLUDE_DIR CACHE)
endif()
