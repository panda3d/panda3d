# Filename: ManglePNG.cmake
# Author: kestred (28 Nov, 2013)
#
# ManglePNG replaces the output variables of the builtin FindPNG
# with vars matching Panda3D's existing config-var names.
#

if(PNG_FOUND)
	set(FOUND_PNG TRUE)
	set(PNG_LIBS png)

	get_filename_component(PNG_LIBRARY_DIR "${PNG_LIBRARY}" PATH)

	set(PNG_IPATH "${PNG_INCLUDE_DIR}" CACHE PATH "The path to libPNG's include directory.") # Include path
	set(PNG_LPATH "${PNG_LIBRARY_DIR}" CACHE PATH "The path to libPNG's library directory.") # Library path
	mark_as_advanced(PNG_IPATH)
	mark_as_advanced(PNG_LPATH)

	set(PNG_VERSION ${PNG_VERSION_STRING})

	unset(PNG_FOUND)
	unset(PNG_DEFINITIONS)
	unset(PNG_LIBRARY_DIR)
	unset(PNG_VERSION_STRING)
endif()

unset(PNG_INCLUDE_DIR CACHE)
unset(PNG_PNG_INCLUDE_DIR CACHE)
