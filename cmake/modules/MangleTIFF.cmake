# Filename: MangleTIFF.cmake
# Author: kestred (29 Nov, 2013)
#
# MangleTIFF replaces the output variables of the builtin FindTIFF
# with vars matching Panda3D's existing config-var names.
#

if(TIFF_FOUND)
	set(FOUND_TIFF TRUE)
	set(TIFF_LIBS tiff z)

	get_filename_component(TIFF_LIBRARY_DIR "${TIFF_LIBRARY}" PATH)

	set(TIFF_IPATH "${TIFF_INCLUDE_DIR}" CACHE PATH "The path to libTIFF's include directory.") # Include path
	set(TIFF_LPATH "${TIFF_LIBRARY_DIR}" CACHE PATH "The path to libTIFF's library directory.") # Library path
	mark_as_advanced(TIFF_IPATH)
	mark_as_advanced(TIFF_LPATH)

	set(TIFF_VERSION ${TIFF_VERSION_STRING})

	unset(TIFF_FOUND)
	unset(TIFF_LIBRARY_DIR)
endif()

unset(TIFF_INCLUDE_DIR CACHE)
