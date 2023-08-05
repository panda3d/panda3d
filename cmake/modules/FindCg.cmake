# Filename: FindCg.cmake
# Author: kestred (8 Dec, 2013)
#
# Usage:
#   find_package(Cg [REQUIRED] [QUIET])
#
# Once done this will define:
#   CG_FOUND         - system has NvidiaCg
#   CG_INCLUDE_DIR   - the NvidiaCg include directory
#   CG_INCLUDE_DIRS  - directories for all NvidiaCg components
#   CG_LIBRARY_DIR   - the NvidiaCg library directory
#   CG_LIBRARY       - the path to the library binary
#   CG_LIBRARIES     - the paths to the Cg library and each library below.
#
#   CGGL_FOUND       - system has CgGL
#   CGGL_INCLUDE_DIR - the CgGL include directory
#   CGGL_LIBRARY_DIR - the CgGL library directory
#   CGGL_LIBRARY     - the path to the library binary
#


### Define macros to find each sublibrary ###

# Find Cg for OpenGL
macro(find_cggl)
  if(APPLE)
    # GL support is built-in on Apple
    set(CGGL_LIBRARY "${CG_LIBRARY}")
    set(CGGL_LIBRARY_DIR "${CG_LIBRARY_DIR}")
    set(CGGL_INCLUDE_DIR "${CG_INCLUDE_DIR}")
  endif()

  if(Cg_FIND_QUIETLY)
    set(CgGL_FIND_QUIETLY TRUE)
  endif()
  if(NOT CGGL_LIBRARY_DIR OR NOT CGGL_INCLUDE_DIR)
    # Find the include directory
    find_path(CGGL_INCLUDE_DIR
      NAMES "cgGL.h"
      PATHS "C:/Program Files/Cg"
            "C:/Program Files/NVIDIA Corporation/Cg/include"
            "/usr/include"
            "/usr/local/include"
            "/opt/Cg"
            "/opt/nvidia-cg-toolkit/include" # Gentoo
      PATH_SUFFIXES "" "Cg" "cg"
      DOC "The path to NvidiaCgGL's include directory."
    )

    # Find the library directory
    find_library(CGGL_LIBRARY
      NAMES "CgGL" "libCgGL"
      PATHS "C:/Program Files/Cg"
            "C:/Program Files/NVIDIA Corporation/Cg"
            "/usr"
            "/usr/lib/x86_64-linux-gnu"
            "/usr/local"
            "/opt/Cg"
            "/opt/nvidia-cg-toolkit" # Gentoo
      PATH_SUFFIXES "" "lib" "lib32" "lib64"
      DOC "The filepath to NvidiaCgGL's libary binary."
    )
    get_filename_component(CGGL_LIBRARY_DIR "${CGGL_LIBRARY}" PATH)
    set(CGGL_LIBRARY_DIR "${CGGL_LIBRARY_DIR}" CACHE PATH "The path to the CgGL library directory.") # Library path

    mark_as_advanced(CGGL_INCLUDE_DIR)
    mark_as_advanced(CGGL_LIBRARY_DIR)
    mark_as_advanced(CGGL_LIBRARY)
  endif()

  find_package_handle_standard_args(CgGL DEFAULT_MSG CGGL_LIBRARY CGGL_INCLUDE_DIR CGGL_LIBRARY_DIR)

endmacro()


# Find Cg for Direct3D 9
macro(find_cgd3d9)
  if(Cg_FIND_QUIETLY)
    set(CgD3D9_FIND_QUIETLY TRUE)
  endif()
  if(NOT CGD3D9_LIBRARY_DIR OR NOT CGD3D9_INCLUDE_DIR)
    # Find the include directory
    find_path(CGD3D9_INCLUDE_DIR
      NAMES "cgD3D9.h"
      PATHS "C:/Program Files/Cg"
            "C:/Program Files/NVIDIA Corporation/Cg/include"
            "/usr/include"
            "/usr/local/include"
            "/opt/Cg"
            "/opt/nvidia-cg-toolkit/include" # Gentoo
      PATH_SUFFIXES "" "Cg" "cg"
      DOC "The path to NvidiaCgD3D9's include directory."
    )

    # Find the library directory
    find_library(CGD3D9_LIBRARY
      NAMES "CgD3D9" "libCgD3D9"
      PATHS "C:/Program Files/Cg"
            "C:/Program Files/NVIDIA Corporation/Cg"
            "/usr"
            "/usr/lib/x86_64-linux-gnu"
            "/usr/local"
            "/opt/Cg"
            "/opt/nvidia-cg-toolkit" # Gentoo
      PATH_SUFFIXES "" "lib" "lib32" "lib64"
      DOC "The filepath to NvidiaCgD3D9's libary binary."
    )
    get_filename_component(CGD3D9_LIBRARY_DIR "${CGD3D9_LIBRARY}" PATH)
    set(CGD3D9_LIBRARY_DIR "${CGD3D9_LIBRARY_DIR}" CACHE PATH "The path to the CgD3D9 library directory.") # Library path

    mark_as_advanced(CGD3D9_INCLUDE_DIR)
    mark_as_advanced(CGD3D9_LIBRARY_DIR)
    mark_as_advanced(CGD3D9_LIBRARY)
  endif()

  find_package_handle_standard_args(CgD3D9 DEFAULT_MSG CGD3D9_LIBRARY CGD3D9_INCLUDE_DIR CGD3D9_LIBRARY_DIR)

endmacro()



# Find base Nvidia Cg
if(NOT CG_LIBRARY_DIR OR NOT CG_INCLUDE_DIRS)
  # Find the include directory
  find_path(CG_INCLUDE_DIR
    NAMES "Cg/cg.h"
    PATHS "C:/Program Files/Cg"
          "C:/Program Files/NVIDIA Corporation/Cg/include"
          "/usr/include"
          "/usr/local/include"
          "/opt/Cg"
          "/opt/nvidia-cg-toolkit/include" # Gentoo
    PATH_SUFFIXES "" "Cg" "cg"
    DOC "The path to NvidiaCg's include directory."
  )

  # Find the library directory
  find_library(CG_LIBRARY
    NAMES "Cg" "libCg"
    PATHS "C:/Program Files/Cg"
          "C:/Program Files/NVIDIA Corporation/Cg"
          "/usr"
          "/usr/lib/x86_64-linux-gnu"
          "/usr/local"
          "/opt/Cg"
          "/opt/nvidia-cg-toolkit" # Gentoo
    PATH_SUFFIXES "" "lib" "lib32" "lib64"
  )
  get_filename_component(CG_LIBRARY_DIR "${CG_LIBRARY}" PATH)
  set(CG_LIBRARY_DIR "${CG_LIBRARY_DIR}" CACHE PATH "The path to NvidiaCG's library directory.") # Library path

  string(REGEX REPLACE "/Cg$" "" CG_BASE_INCLUDE_DIR "${CG_INCLUDE_DIR}")
  set(CG_INCLUDE_DIRS ${CG_BASE_INCLUDE_DIR} ${CG_INCLUDE_DIR})

  mark_as_advanced(CG_INCLUDE_DIRS)
  mark_as_advanced(CG_INCLUDE_DIR)
  mark_as_advanced(CG_LIBRARY_DIR)
  mark_as_advanced(CG_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cg DEFAULT_MSG CG_LIBRARY CG_INCLUDE_DIRS CG_LIBRARY_DIR)

if(CG_INCLUDE_DIR AND CG_LIBRARY_DIR)
  find_cggl()
  find_cgd3d9()

  set(CG_LIBRARIES ${CG_LIBRARY})
  if(CGGL_LIBRARY)
    list(APPEND CG_LIBRARIES "${CGGL_LIBRARY}")
  endif()
  if(CGD3D9_LIBRARY)
    list(APPEND CG_LIBRARIES "${CGD3D9_LIBRARY}")
  endif()
endif()
