# Filename: MangleOpenAL.cmake
# Author: kestred (28 Nov, 2013)
#
# MangleOpenAL replaces the output variables of the builtin FindOpenAL
# with vars matching Panda3D's existing config-var names.
#

if(OPENAL_FOUND)
	set(FOUND_OPENAL TRUE)
	if(APPLE)
		set(OPENAL_FRAMEWORK OpenAL)
	else()
		set(OPENAL_LIBS openal)
	endif()

	get_filename_component(OPENAL_LIBRARY_DIR "${OPENAL_LIBRARY}" PATH)

	set(OPENAL_IPATH "${OPENAL_INCLUDE_DIR}" CACHE PATH "The path to OpenAL's include directory.") # Include path
	set(OPENAL_LPATH "${OPENAL_LIBRARY_DIR}" CACHE PATH "The path to OpenAL's library directory.") # Library path
	mark_as_advanced(OPENAL_IPATH)
	mark_as_advanced(OPENAL_LPATH)

	set(OPENAL_VERSION ${OPENAL_VERSION_STRING})

	unset(OPENAL_FOUND)
	unset(OPENAL_LIBRARY_DIR)
endif()

unset(OPENAL_INCLUDE_DIR CACHE)
