# Filename: MangleZLIB.cmake
# Author: kestred (9 Dec, 2013)
#
# MangleZLIB replaces the output variables of the builtin FindZLIB
# with vars matching Panda3D's existing config-var names.
#

if(ZLIB_FOUND)
	set(FOUND_ZLIB TRUE)
	set(ZLIB_LIBS ZLIB)

	list(GET ZLIB_INCLUDE_DIRS 0 ZLIB_INCLUDE_DIR)
	list(GET ZLIB_LIBRARIES 0 ZLIB_LIBRARY)
	get_filename_component(ZLIB_LIBRARY_DIR "${ZLIB_LIBRARY}" PATH)

	set(ZLIB_IPATH "${ZLIB_INCLUDE_DIR}" CACHE PATH "The path to zlibs's include directory.") # Include path
	set(ZLIB_LPATH "${ZLIB_LIBRARY_DIR}" CACHE PATH "The path to zlibs's library directory.") # Library path
	mark_as_advanced(ZLIB_IPATH)
	mark_as_advanced(ZLIB_LPATH)

	set(ZLIB_VERSION ${ZLIB_VERSION_STRING})

	unset(ZLIB_FOUND)
	unset(ZLIB_VERSION_STRING)
	unset(ZLIB_VERSION_MAJOR)
	unset(ZLIB_VERSION_MINOR)
	unset(ZLIB_VERSION_PATCH)
	unset(ZLIB_VERSION_TWEAK)
	unset(ZLIB_MAJOR_VERSION)
	unset(ZLIB_MINOR_VERSION)
	unset(ZLIB_OATCH_VERSION)
endif()

unset(ZLIB_LIBRARY CACHE)
unset(ZLIB_LIBRARIES CACHE)
unset(ZLIB_INCLUDE_DIR CACHE)
unset(ZLIB_INCLUDE_DIRS CACHE)
