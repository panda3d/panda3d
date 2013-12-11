# Filename: MangleFREETYPE.cmake
# Author: kestred (11 Dec, 2013)
#
# MangleFREETYPE replaces the output variables of the builtin FindFREETYPE
# with vars matching Panda3D's existing config-var names.
#

if(FREETYPE_FOUND)
	set(FOUND_FREETYPE TRUE)

	list(GET FREETYPE_INCLUDE_DIRS 0 FREETYPE_INCLUDE_DIR)

	get_filename_component(FREETYPE_LIBRARY_DIR "${FREETYPE_LIBRARY}" PATH)

	set(FREETYPE_IPATH "${FREETYPE_INCLUDE_DIR}" CACHE PATH "The path to Freetype's include directory.") # Include path
	set(FREETYPE_LPATH "${FREETYPE_LIBRARY_DIR}" CACHE PATH "The path to Freetype's library directory.") # Library path
	mark_as_advanced(FREETYPE_IPATH)
	mark_as_advanced(FREETYPE_LPATH)

	set(FREETYPE_VERSION ${FREETYPE_VERSION_STRING})

	unset(FREETYPE_FOUND)
	unset(FREETYPE_LIBRARY)
	unset(FREETYPE_INCLUDE_DIR)
	unset(FREETYPE_LIBRARY_DIR)
endif()

unset(FREETYPE_LIBRARY CACHE)
unset(FREETYPE_LIBRARIES CACHE)
unset(FREETYPE_INCLUDE_DIRS CACHE)
unset(FREETYPE_INCLUDE_DIR_ft2build CACHE)
unset(FREETYPE_INCLUDE_DIR_freetype2 CACHE)