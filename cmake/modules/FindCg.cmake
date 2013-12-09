# Filename: FindCg.cmake
#
# Usage:
#   find_package(Cg [REQUIRED] [QUIET])
#
# It sets the following variables:
#   FOUND_CG   - system has NvidiaCg
#   CG_IPATH   - the NvidiaCg include directory
#   CG_LPATH   - the NvidiaCg library directory
#   CG_LIBS    - the Cg components found
#
#   FOUND_CGGL - system has CgGL
#   CGGL_IPATH - the CgGL include directory
#   CGGL_LPATH - the CgGL library directory
#   CGGL_LIBS  - the CgGL components found
#


### Define macros to find each sublibrary ###

# Find Cg for OpenGL
macro(find_cggl)
	if(CGGL_LPATH AND CGGL_IPATH)
		# If its cached, we don't need to refind it
		set(FOUND_CGGL TRUE)
		if(WIN32)
			set(CGGL_LIBS cgGL.lib)
		else()
			set(CGGL_LIBS CgGL)
		endif()
	else()
		# Find the include directory
		find_path(CGGL_IPATH
			NAMES "cgGL.h"
			PATHS "C:/Program Files/Cg"
			      "C:/Program Files/NVIDIA Corporation/Cg/include"
			      "/usr/include"
			      "/usr/local/include"
			      "/opt/Cg"
			      "/opt/nvidia-cg-toolkit/include" # Gentoo
			PATH_SUFFIXES "" "Cg"
			DOC "The path to NvidiaCg's include directory."
		)

		# Find the library directory
		find_library(CGGL_LIBRARY
			NAMES "CgGL" "libCgGL"
			PATHS "C:/Program Files/Cg"
			      "C:/Program Files/NVIDIA Corporation/Cg"
			      "/usr"
			      "/usr/local"
			      "/opt/Cg"
			      "/opt/nvidia-cg-toolkit" # Gentoo
			PATH_SUFFIXES "" "lib" "lib32" "lib64"
		)
		get_filename_component(CGGL_LIBRARY_DIR "${CGGL_LIBRARY}" PATH)
		set(CGGL_LPATH "${CGGL_LIBRARY_DIR}" CACHE PATH "The path to the CgGL library directory.") # Library path

		unset(CGGL_LIBRARY_DIR)
		unset(CGGL_LIBRARY CACHE)


		# Check if we have everything we need
		if(CGGL_IPATH AND CGGL_LPATH)
			set(FOUND_CGGL TRUE)
			if(WIN32)
				set(CGGL_LIBS cgGL.lib)
			else()
				set(CGGL_LIBS CgGL)
			endif()
		endif()

		mark_as_advanced(CGGL_IPATH)
		mark_as_advanced(CGGL_LPATH)
	endif()
endmacro()


# Find Cg for DirectX 8
macro(find_cgdx8)
	# TODO: Implement
endmacro()


# Find Cg for DirectX 9
macro(find_cgdx9)
	# TODO: Implement
endmacro()


# Find Cg for DirectX 10
macro(find_cgdx10)
	# TODO: Implement
endmacro()



# Find base Nvidia Cg
if(CG_LPATH AND CG_IPATH)
	# If its cached, we don't need to refind it
	set(FOUND_CG TRUE)
	if(WIN32)
		set(CG_LIBS cg.lib)
	else()
		set(CG_LIBS Cg)
	endif()

	find_cggl()
	find_cgdx8()
	find_cgdx9()
	find_cgdx10()

else()
	# On OSX default to using the framework version of Cg.
	if(APPLE)
		include(${CMAKE_ROOT}/Modules/CMakeFindFrameworks.cmake)
		set(CG_INCLUDES)

		cmake_find_frameworks(Cg)
		if(Cg_FRAMEWORKS)
			foreach(dir ${Cg_FRAMEWORKS})
				set(CG_INCLUDES ${CG_INCLUDES} ${dir}/Headers ${dir}/PrivateHeaders)
			endforeach(dir)
			unset(Cg_FRAMEWORKS)

			# Find the include dir
			find_path(CG_IPATH
				NAMES "cg.h"
				PATHS ${CG_INCLUDES}
				DOC "The path to NvidiaCg's include directory."
			)
			unset(CG_INCLUDES)

			# Set the library dir (TODO: Check the correctness on Mac OS X)
			set(CG_LPATH "/Library/Frameworks/Cg.framework" CACHE PATH "The path to NvidiaCg's library directory.")
		endif()

	else()
		# Find the include directory
		find_path(CG_IPATH
			NAMES "cg.h"
			PATHS "C:/Program Files/Cg"
			      "C:/Program Files/NVIDIA Corporation/Cg/include"
			      "/usr/include"
			      "/usr/local/include"
			      "/opt/Cg"
			      "/opt/nvidia-cg-toolkit/include" # Gentoo
			PATH_SUFFIXES "" "Cg"
			DOC "The path to NvidiaCg's include directory."
		)

		# Find the library directory
		find_library(CG_LIBRARY
			NAMES "Cg" "libCg"
			PATHS "C:/Program Files/Cg"
			      "C:/Program Files/NVIDIA Corporation/Cg"
			      "/usr"
			      "/usr/local"
			      "/opt/Cg"
			      "/opt/nvidia-cg-toolkit" # Gentoo
			PATH_SUFFIXES "" "lib" "lib32" "lib64"
		)
		get_filename_component(CG_LIBRARY_DIR "${CG_LIBRARY}" PATH)
		set(CG_LPATH "${CG_LIBRARY_DIR}" CACHE PATH "The path to NvidiaCG's library directory.") # Library path

		unset(CG_LIBRARY_DIR)
		unset(CG_LIBRARY CACHE)
	endif()

	# Check if we have everything we need
	if(CG_IPATH AND CG_LPATH)
		set(FOUND_CG TRUE)
		if(WIN32)
			set(CG_LIBS cg.lib)
		else()
			set(CG_LIBS Cg)
		endif()

		find_cggl()
		find_cgdx8()
		find_cgdx9()
		find_cgdx10()

	endif()

	mark_as_advanced(CG_IPATH)
	mark_as_advanced(CG_LPATH)
endif()
