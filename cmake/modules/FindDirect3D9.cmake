# Filename: FindDirect3D9.cmake
# Authors: CFSworks (26 Oct, 2018)
#
# Usage:
#   find_package(Direct3D9 [REQUIRED] [QUIET])
#
# This supports the following components:
#   d3dx9
#   dxerr
#   dxguid
#
# Once done this will define:
#   DIRECT3D9_FOUND       - system has Direct3D 9.x
#   DIRECT3D9_INCLUDE_DIR - the include directory containing d3d9.h - note that
#                           this will be empty if it's part of the Windows SDK.
#   DIRECT3D9_LIBRARY     - the path to d3d9.lib
#   DIRECT3D9_LIBRARIES   - the path to d3d9.lib and all extra component
#                           libraries
#

include(CheckIncludeFile)

if(Direct3D9_FIND_QUIETLY)
  if(DEFINED CMAKE_REQUIRED_QUIET)
    set(_OLD_CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET})
  endif()
  # Suppress check_include_file messages
  set(CMAKE_REQUIRED_QUIET ON)
endif()

check_include_file("d3d9.h" SYSTEM_INCLUDE_D3D9_H)
mark_as_advanced(SYSTEM_INCLUDE_D3D9_H)

if(Direct3D9_FIND_QUIETLY)
  if(DEFINED _OLD_CMAKE_REQUIRED_QUIET)
    set(CMAKE_REQUIRED_QUIET ${_OLD_CMAKE_REQUIRED_QUIET})
    unset(_OLD_CMAKE_REQUIRED_QUIET)
  else()
    unset(CMAKE_REQUIRED_QUIET)
  endif()
endif()

if(SYSTEM_INCLUDE_D3D9_H
    AND NOT Direct3D9_FIND_REQUIRED_d3dx9
    AND NOT Direct3D9_FIND_REQUIRED_dxerr)
  # It's available as #include <d3d9.h> - easy enough.  We'll use "." as a way
  # of saying "We found it, but please erase this variable later."
  set(DIRECT3D9_INCLUDE_DIR ".")

  # Since d3d9.h is on the search path, we can pretty much assume d3d9.lib is
  # as well.
  set(DIRECT3D9_LIBRARY "d3d9.lib")

  # And dxguid.lib, why not
  set(DIRECT3D9_dxguid_LIBRARY "dxguid.lib")

else()
  # We could not find it easily - maybe it's installed separately as part of
  # the DirectX SDK?

  find_path(DIRECT3D9_INCLUDE_DIR
    NAMES d3d9.h
    PATHS "$ENV{DXSDK_DIR}/Include")

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(dx_lib_path "$ENV{DXSDK_DIR}/Lib/x64/")
  else()
    set(dx_lib_path "$ENV{DXSDK_DIR}/Lib/x86/")
  endif()

  find_library(DIRECT3D9_LIBRARY d3d9 "${dx_lib_path}" NO_DEFAULT_PATH)

  find_library(DIRECT3D9_d3dx9_LIBRARY d3dx9 "${dx_lib_path}" NO_DEFAULT_PATH)
  find_library(DIRECT3D9_dxerr_LIBRARY dxerr "${dx_lib_path}" NO_DEFAULT_PATH)
  find_library(DIRECT3D9_dxguid_LIBRARY dxguid "${dx_lib_path}" NO_DEFAULT_PATH)

  unset(dx_lib_path)
endif()

mark_as_advanced(DIRECT3D9_INCLUDE_DIR DIRECT3D9_LIBRARY)
set(DIRECT3D9_LIBRARIES "${DIRECT3D9_LIBRARY}")

foreach(_component d3dx9 dxerr dxguid)
  if(DIRECT3D9_${_component}_LIBRARY)
    set(Direct3D9_${_component}_FOUND ON)
    list(FIND Direct3D9_FIND_COMPONENTS "${_component}" _index)
    if(${_index} GREATER -1)
      list(APPEND DIRECT3D9_LIBRARIES "${DIRECT3D9_${_component}_LIBRARY}")
    endif()
    unset(_index)
  endif()
endforeach(_component)
unset(_component)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Direct3D9 HANDLE_COMPONENTS
  REQUIRED_VARS DIRECT3D9_INCLUDE_DIR DIRECT3D9_LIBRARY DIRECT3D9_LIBRARIES)

# See above - if we found the include as part of the system path, we don't want
# to actually modify the include search path, but we need a non-empty string to
# satisfy find_package_handle_standard_args()
if(DIRECT3D9_INCLUDE_DIR STREQUAL ".")
  set(DIRECT3D9_INCLUDE_DIR "")
endif()
