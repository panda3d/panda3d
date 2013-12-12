# Filename: MangleGTK2.cmake
# Author: kestred (11 Dec, 2013)
#
# MangleGTK2 replaces the output variables of the builtin FindGTK2
# with vars matching Panda3D's existing config-var names.
#

if(GTK2_FOUND)
	set(FOUND_GTK TRUE)

	list(GET GTK2_LIBRARIES 0 GTK2_LIBRARY)
	list(GET GTK2_INCLUDE_DIRS 0 GTK2_INCLUDE_DIR)

	get_filename_component(GTK2_LIBRARY_DIR "${GTK2_LIBRARY}" PATH)

	set(GTK_IPATH "${GTK2_INCLUDE_DIR}" CACHE PATH "The path to gtk-2's include directory.") # Include path
	set(GTK_LPATH "${GTK2_LIBRARY_DIR}" CACHE PATH "The path to gtk-2's library directory.") # Library path
	mark_as_advanced(GTK_IPATH)
	mark_as_advanced(GTK_LPATH)

	set(GTK_LIBRARIES GTK2_LIBRARIES)

	unset(GTK2_FOUND)
	unset(GTK2_LIBRARY)
	unset(GTK2_INCLUDE_DIR)
	unset(GTK2_LIBRARY_DIR)
	unset(GTK2_MAJOR_VERSION)
	unset(GTK2_MINOR_VERSION)
	unset(GTK2_PATCH_VERSION)
endif()

unset(GTK2_LIBRARIES CACHE)
unset(GTK2_INCLUDE_DIRS CACHE)
unset(GTK2_DEFINITIONS CACHE)