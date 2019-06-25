# Filename: FindFCollada.cmake
# Author: CFSworks (17 Mar, 2019)
#
# Usage:
#   find_package(FCollada [REQUIRED] [QUIET])
#
# Once done this will define:
#   FCOLLADA_FOUND       - system has FCollada
#   FCOLLADA_INCLUDE_DIR - the FCollada include directory
#
#   FCOLLADA_RELEASE_LIBRARY - the filepath of the FCollada release library
#   FCOLLADA_DEBUG_LIBRARY   - the filepath of the FCollada debug library
#
#   FCollada::FCollada - The recommended FCollada library to link against
#

# Find the FCollada include files
find_path(FCOLLADA_INCLUDE_DIR "FCollada.h" PATH_SUFFIXES "FCollada")

# Find the library built for release
find_library(FCOLLADA_RELEASE_LIBRARY
  NAMES "FCollada" "libFCollada"
  "FColladaS" "libFColladaS"
  "FColladaU" "libFColladaU"
  "FColladaSU" "libFColladaSU"
)

# Find the library built for debug
find_library(FCOLLADA_DEBUG_LIBRARY
  NAMES "FColladaD" "libFColladaD"
  "FColladaSD" "libFColladaSD"
  "FColladaUD" "libFColladaUD"
  "FColladaSUD" "libFColladaSUD"
)

mark_as_advanced(FCOLLADA_INCLUDE_DIR)
mark_as_advanced(FCOLLADA_RELEASE_LIBRARY)
mark_as_advanced(FCOLLADA_DEBUG_LIBRARY)

set(_defines)
if(FCOLLADA_RELEASE_LIBRARY MATCHES "FCollada[^/]*U" OR
    FCOLLADA_DEBUG_LIBRARY MATCHES "FCollada[^/]*U")
  list(APPEND _defines "UNICODE")
endif()
if(NOT MSVC AND
    NOT FCOLLADA_RELEASE_LIBRARY MATCHES "FCollada[^/]*S" AND
    NOT FCOLLADA_DEBUG_LIBRARY MATCHES "FCollada[^/]*S")
  list(APPEND _defines "FCOLLADA_DLL")
endif()

# Identify the configs which we have available
set(_configs)
if(FCOLLADA_INCLUDE_DIR)
  if(FCOLLADA_RELEASE_LIBRARY)
    list(APPEND _configs RELEASE)
  endif()
  if(FCOLLADA_DEBUG_LIBRARY)
    list(APPEND _configs DEBUG)
  endif()

  if(_configs)
    set(_HAS_FCOLLADA_LIBRARY ON)
    add_library(FCollada::FCollada UNKNOWN IMPORTED GLOBAL)

    set_target_properties(FCollada::FCollada PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS "${_defines}"
      INTERFACE_INCLUDE_DIRECTORIES "${FCOLLADA_INCLUDE_DIR}")

  endif()

endif()

foreach(_config ${_configs})
  set_property(TARGET FCollada::FCollada
    APPEND PROPERTY IMPORTED_CONFIGURATIONS "${_config}")

  set_target_properties(FCollada::FCollada PROPERTIES
    IMPORTED_LOCATION_${_config} "${FCOLLADA_${_config}_LIBRARY}")

endforeach(_config)
unset(_config)
unset(_configs)
unset(_defines)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FCollada DEFAULT_MSG FCOLLADA_INCLUDE_DIR _HAS_FCOLLADA_LIBRARY)

unset(_HAS_FCOLLADA_LIBRARY)
