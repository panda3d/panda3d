# Filename: FindEigen3.cmake
# Authors: montel@kde.org, g.gael@free.fr, jacob.benoit.1@gmail.com
# Redistribution and use is allowed according to the terms of the BSD license.
#
# Module modified to mangle the names for Panda3D.
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(Eigen3 3.1.2)
# to require version 3.1.2 or newer of Eigen3.
#
# Once done this will define
#
#  HAVE_EIGEN - system has eigen lib with correct version
#  EIGEN_IPATH - the eigen include directory
#  EIGEN_VERSION - eigen version
#

if(NOT Eigen3_FIND_VERSION)
	if(NOT Eigen3_FIND_VERSION_MAJOR)
		set(Eigen3_FIND_VERSION_MAJOR 2)
	endif(NOT Eigen3_FIND_VERSION_MAJOR)
	if(NOT Eigen3_FIND_VERSION_MINOR)
		set(Eigen3_FIND_VERSION_MINOR 91)
	endif(NOT Eigen3_FIND_VERSION_MINOR)
	if(NOT Eigen3_FIND_VERSION_PATCH)
		set(Eigen3_FIND_VERSION_PATCH 0)
	endif(NOT Eigen3_FIND_VERSION_PATCH)

	set(Eigen3_FIND_VERSION "${Eigen3_FIND_VERSION_MAJOR}.${Eigen3_FIND_VERSION_MINOR}.${Eigen3_FIND_VERSION_PATCH}")
endif(NOT Eigen3_FIND_VERSION)

macro(_eigen3_check_version)
	file(READ "${EIGEN_IPATH}/Eigen/src/Core/util/Macros.h" _eigen3_version_header)

	string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen3_world_version_match "${_eigen3_version_header}")
	set(EIGEN3_WORLD_VERSION "${CMAKE_MATCH_1}")
	string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen3_major_version_match "${_eigen3_version_header}")
	set(EIGEN3_MAJOR_VERSION "${CMAKE_MATCH_1}")
	string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen3_minor_version_match "${_eigen3_version_header}")
	set(EIGEN3_MINOR_VERSION "${CMAKE_MATCH_1}")

	set(EIGEN_VERSION ${EIGEN3_WORLD_VERSION}.${EIGEN3_MAJOR_VERSION}.${EIGEN3_MINOR_VERSION})
	if(${EIGEN_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})
		set(EIGEN_VERSION_OK FALSE)
	else(${EIGEN_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})
		set(EIGEN_VERSION_OK TRUE)
	endif(${EIGEN_VERSION} VERSION_LESS ${Eigen3_FIND_VERSION})

	if(NOT EIGEN_VERSION_OK)

		message(STATUS "Eigen3 version ${EIGEN_VERSION} found in ${EIGEN_IPATH}, "
									 "but at least version ${Eigen3_FIND_VERSION} is required")
	endif(NOT EIGEN_VERSION_OK)
endmacro(_eigen3_check_version)

if (EIGEN_IPATH)

	# in cache already
	_eigen3_check_version()
	set(HAVE_EIGEN ${EIGEN_VERSION_OK})

else (EIGEN_IPATH)

	find_path(EIGEN_IPATH NAMES signature_of_eigen3_matrix_library
			PATHS
				${CMAKE_INSTALL_PREFIX}/include
				${KDE4_INCLUDE_DIR}
			PATH_SUFFIXES eigen3 eigen
		)

	if(EIGEN_IPATH)
		_eigen3_check_version()
	endif(EIGEN_IPATH)

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Eigen3 DEFAULT_MSG EIGEN_IPATH EIGEN_VERSION_OK)

	mark_as_advanced(EIGEN_IPATH)

endif(EIGEN_IPATH)
