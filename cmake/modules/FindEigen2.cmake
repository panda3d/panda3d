# Filename: FindEigen2.cmake
# Authors: montel@kde.org, g.gael@free.fr
# Redistribution and use is allowed according to the terms of the BSD license.
#
# Module modified to mangle the names for Panda3D.
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(Eigen2 2.0.3)
# to require version 2.0.3 to newer of Eigen2.
#
# Once done this will define
#
#  HAVE_EIGEN - system has eigen lib with correct version
#  EIGEN_IPATH - the eigen include directory
#  EIGEN_VERSION - eigen version
#

if(NOT EIGEN_FIND_VERSION)
	if(NOT EIGEN_FIND_VERSION_MAJOR)
		set(EIGEN_FIND_VERSION_MAJOR 2)
	endif()
	if(NOT EIGEN_FIND_VERSION_MINOR)
		set(EIGEN_FIND_VERSION_MINOR 0)
	endif()
	if(NOT EIGEN_FIND_VERSION_PATCH)
		set(EIGEN_FIND_VERSION_PATCH 0)
	endif()

	set(EIGEN_FIND_VERSION "${EIGEN_FIND_VERSION_MAJOR}.${EIGEN_FIND_VERSION_MINOR}.${EIGEN_FIND_VERSION_PATCH}")
endif()


macro(_eigen_check_version)
	# Read version from Macros.h
	file(READ "${EIGEN_IPATH}/Eigen/src/Core/util/Macros.h" _eigen_version_header)

	# Parse version from read data
	string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen_world_version_match "${_eigen_version_header}")
	set(EIGEN_WORLD_VERSION "${CMAKE_MATCH_1}")
	string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen_major_version_match "${_eigen_version_header}")
	set(EIGEN_MAJOR_VERSION "${CMAKE_MATCH_1}")
	string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen_minor_version_match "${_eigen_version_header}")
	set(EIGEN_MINOR_VERSION "${CMAKE_MATCH_1}")

	# Set EIGEN_VERSION
	set(EIGEN_VERSION ${EIGEN_WORLD_VERSION}.${EIGEN_MAJOR_VERSION}.${EIGEN_MINOR_VERSION})

	# Make sure Eigen Version is at least the requested version
	if((${EIGEN_WORLD_VERSION} NOTEQUAL 2) OR (${EIGEN_MAJOR_VERSION} GREATER 10) OR (${EIGEN_VERSION} VERSION_LESS ${EIGEN_FIND_VERSION}))
		set(EIGEN_VERSION_OK FALSE)
	else()
		set(EIGEN_VERSION_OK TRUE)
	endif()
endmacro()


if(EIGEN_IPATH)
	# If already in cache, just check that the version is correct
	_eigen_check_version()
	set(HAVE_EIGEN ${EIGEN_VERSION_OK})
else()
	# Otherwise find it manually
	find_path(EIGEN_IPATH
		NAMES Eigen/Core
		PATHS ${INCLUDE_INSTALL_DIR}
		      ${KDE4_INCLUDE_DIR}
		PATH_SUFFIXES eigen2
	)

	# Checking to make sure it has the write version
	if(EIGEN_IPATH)
		_eigen_check_version()
		set(HAVE_EIGEN ${EIGEN_VERSION_OK})
	endif()

	mark_as_advanced(EIGEN_IPATH)
endif()
