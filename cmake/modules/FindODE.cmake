# Filename: FindODE.cmake
# Author: CFSworks (7 Feb, 2014)
#
# Usage:
#   find_package(ODE [REQUIRED] [QUIET])
#
# Once done this will define:
#   ODE_FOUND       - system has ode
#   ODE_INCLUDE_DIR - the ode include directory
#
#   ODE_RELEASE_LIBRARY - the filepath of the ode release library
#   ODE_DEBUG_LIBRARY   - the filepath of the ode debug library
#
#   ODE_SINGLE_DEBUG_LIBRARY   - the filepath of the single-precision ode debug library
#   ODE_SINGLE_RELEASE_LIBRARY   - the filepath of the single-precision ode release library
#   ODE_DOUBLE_DEBUG_LIBRARY   - the filepath of the double-precision ode debug library
#   ODE_DOUBLE_RELEASE_LIBRARY   - the filepath of the double-precision ode release library
#
#   ODE::ODE        - The recommended ODE library to link against
#   ODE::ODE_single - If available, this links against single-precision ODE
#   ODE::ODE_double - If available, this links against double-precision ODE
#

# Find the libode include files
find_path(ODE_INCLUDE_DIR "ode/ode.h")

# Find the libode library built for release
find_library(ODE_RELEASE_LIBRARY
  NAMES "ode" "libode")

# Find the libode library built for debug
find_library(ODE_DEBUG_LIBRARY
  NAMES "oded" "liboded")

# Find the single-precision library built for release
find_library(ODE_SINGLE_RELEASE_LIBRARY
  NAMES "ode_single" "libode_single")

# Find the single-precision library built for debug
find_library(ODE_SINGLE_DEBUG_LIBRARY
  NAMES "ode_singled" "libode_singled")

# Find the double-precision library built for release
find_library(ODE_DOUBLE_RELEASE_LIBRARY
  NAMES "ode_double" "libode_double")

# Find the double-precision library built for debug
find_library(ODE_DOUBLE_DEBUG_LIBRARY
  NAMES "ode_doubled" "libode_doubled")

# Find libccd, which ODE sometimes links against, so we want to let the linker
# know about it if it's present.
find_library(ODE_LIBCCD_LIBRARY
  NAMES "ccd" "libccd")

mark_as_advanced(ODE_INCLUDE_DIR)
mark_as_advanced(ODE_RELEASE_LIBRARY)
mark_as_advanced(ODE_DEBUG_LIBRARY)
mark_as_advanced(ODE_SINGLE_RELEASE_LIBRARY)
mark_as_advanced(ODE_SINGLE_DEBUG_LIBRARY)
mark_as_advanced(ODE_DOUBLE_RELEASE_LIBRARY)
mark_as_advanced(ODE_DOUBLE_DEBUG_LIBRARY)
mark_as_advanced(ODE_LIBCCD_LIBRARY)

# Define targets for both precisions (and unspecified)
foreach(_precision _single _double "")
  string(TOUPPER "${_precision}" _PRECISION)

  if(EXISTS "${ODE${_PRECISION}_RELEASE_LIBRARY}" OR
      EXISTS "${ODE${_PRECISION}_DEBUG_LIBRARY}")
    if(NOT TARGET ODE::ODE${_precision})
      add_library(ODE::ODE${_precision} UNKNOWN IMPORTED GLOBAL)

      set_target_properties(ODE::ODE${_precision} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ODE_INCLUDE_DIR}")

      if(ODE_LIBCCD_LIBRARY)
        set_target_properties(ODE::ODE${_precision} PROPERTIES
          INTERFACE_LINK_LIBRARIES "${ODE_LIBCCD_LIBRARY}")
      endif()

      if(EXISTS "${ODE${_PRECISION}_RELEASE_LIBRARY}")
        set_property(TARGET ODE::ODE${_precision} APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(ODE::ODE${_precision} PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
          IMPORTED_LOCATION_RELEASE "${ODE${_PRECISION}_RELEASE_LIBRARY}")
      endif()

      if(EXISTS "${ODE${_PRECISION}_DEBUG_LIBRARY}")
        set_property(TARGET ODE::ODE${_precision} APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(ODE::ODE${_precision} PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
          IMPORTED_LOCATION_DEBUG "${ODE${_PRECISION}_DEBUG_LIBRARY}")
      endif()

      # If this has a precision, we should be sure to define
      # dIDESINGLE/dIDEDOUBLE to keep it consistent
      if(_precision)
        string(REPLACE "_" "dIDE" _precision_symbol "${_PRECISION}")

        set_target_properties(ODE::ODE${_precision} PROPERTIES
          INTERFACE_COMPILE_DEFINITIONS "${_precision_symbol}")

        unset(_precision_symbol)
      endif()

    endif()
  endif()
endforeach(_precision)
unset(_precision)
unset(_PRECISION)

# OKAY.  If everything went well, we have ODE::ODE_single and/or
# ODE::ODE_double.  We might even have an ODE::ODE, but if not, we need to
# alias one of the other two to it.
if(NOT TARGET ODE::ODE)
  if(TARGET ODE::ODE_single)
    set(_copy_from "ODE::ODE_single")
  elseif(TARGET ODE::ODE_double)
    set(_copy_from "ODE::ODE_double")
  endif()

  if(DEFINED _copy_from)
    add_library(ODE::ODE UNKNOWN IMPORTED GLOBAL)

    foreach(_prop
        INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_COMPILE_DEFINITIONS
        INTERFACE_LINK_LIBRARIES
        IMPORTED_CONFIGURATIONS
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE
        IMPORTED_LOCATION_RELEASE
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG
        IMPORTED_LOCATION_DEBUG)

      get_target_property(_value "${_copy_from}" "${_prop}")
      if(_value)
        set_target_properties(ODE::ODE PROPERTIES "${_prop}" "${_value}")
      endif()
      unset(_value)
    endforeach(_prop)
    unset(_prop)
  endif()

  unset(_copy_from)
endif()

if(TARGET ODE::ODE)
  set(_HAS_ODE_LIBRARY ON)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODE DEFAULT_MSG ODE_INCLUDE_DIR _HAS_ODE_LIBRARY)

unset(_HAS_ODE_LIBRARY)
