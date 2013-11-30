# Filename: ConfigurePackage.cmake
# Author: kestred (30 Nov, 2013)
#
# This modules defines a function which finds and configures libraries
# and packages for Panda3Ds configuration file headers (.h files).
#
# Additionally, it defines a macro for including Mangle modules.
#
# Assumes the file has already been found with find_package().
#

# Usage:
#   mangle_package(LIBRARY_NAME)
macro(mangle_package PKG_NAME)
	include(Mangle${PKG_NAME})
endmacro()

# Usage:
#   config_package(PACKAGE_NAME [REQUIRED] [COMMENT <display-name>])
function(config_package PKG_NAME)
	# Set function vars to defaults
	set(DISPLAY_NAME ${PKG_NAME})

	# Handle optional function arguments
	foreach(arg ${ARGN})
		if(ARG_IS_LIBRARY)
			unset(ARG_IS_LIBRARY)
			set(LIB_NAME ${arg})
			break()
		elseif(ARG_IS_COMMENT)
			unset(ARG_IS_COMMENT)
			set(DISPLAY_NAME ${arg})
			break()
		endif()

		if(arg MATCHES "REQUIRED")
			# TODO: Implement currently ignored
		elseif(arg MATCHES "COMMENT")
			set(ARG_IS_COMMENT TRUE)
		else()
			message(AUTHOR_WARNING "configure_package() unexpected argument: '${arg}'")
			return()
		endif()
	endforeach()
	if(ARG_IS_COMMENT)
		message(AUTHOR_WARNING "configure_package() expected display-name after 'COMMENT'")
	endif()

	if(FOUND_${PKG_NAME})
		# Output success after finding the package for the first time
		if(NOT DEFINED USE_${PKG_NAME})
		endif()

		# Add a USE_XYZZY config variable to the cache
		set(USE_${PKG_NAME} TRUE CACHE BOOL "If true, compile Panda3D with ${DISPLAY_NAME}")
		if(USE_${PKG_NAME})
			set(HAVE_${PKG_NAME} TRUE PARENT_SCOPE)
		endif()

	elseif(DEFINED USE_${PKG_NAME})
		# If we were compiling with a particular package, but we can't find it;
		#     then inform the user the package was lost.
		if(USE_${PKG_NAME})
			message(STATUS "- Can no longer find ${DISPLAY_NAME}")

			# Only unset if USE_XYZZY is true;
			# This allows us to set USE_XYZZY to false to silence the output
			unset(USE_${PKG_NAME})
		endif()

	else()
		# Output failure to find package
		message(STATUS "- Did not find ${DISPLAY_NAME}")
	endif()
endfunction()