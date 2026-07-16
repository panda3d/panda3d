# Filename: FindRmlUi.cmake
# Authors: tkfoss (03 Jul, 2026)
#
# Usage:
#   find_package(RmlUi [REQUIRED] [QUIET])
#
# RmlUi 6.0 or later is required; panda/src/rmlui is written against the
# layer/filter RenderInterface API introduced in 6.0.
#
# The upstream CMake package configuration (installed by RmlUi's
# `cmake --install`) is preferred, since it also carries the transitive
# dependencies of static RmlUi builds.  Without it, this falls back to
# searching for the headers and libraries directly; a static RmlUi found that
# way may need its dependencies (e.g. freetype) linked separately.
#
# Once done this will define:
#   RmlUi_FOUND            - system has RmlUi 6.0+
#   RmlUi::RmlUi           - imported target (core)
#   RmlUi::Debugger        - imported target (debugger, if found)
#
# In fallback (non-config) mode it will additionally define:
#   RmlUi_INCLUDE_DIR      - the RmlUi include directory
#   RmlUi_LIBRARY          - the RmlUi core library
#   RmlUi_DEBUGGER_LIBRARY - the RmlUi debugger library (optional)
#

# Prefer the upstream CMake package config.  The version requirement is not
# passed here: RmlUi installs an ExactVersion compatibility file, which would
# reject e.g. an installed 6.3 when 6.0 is requested.  It is checked below
# against RmlUi_VERSION instead.
find_package(RmlUi CONFIG QUIET)

if(RmlUi_FOUND AND TARGET RmlUi::RmlUi)
  if(RmlUi_VERSION VERSION_LESS "6.0")
    if(NOT RmlUi_FIND_QUIETLY)
      message(STATUS
        "Found RmlUi ${RmlUi_VERSION} at ${RmlUi_DIR}, but 6.0 or later is required.")
    endif()
    set(RmlUi_FOUND FALSE)

  else()
    # RmlUi's own CMake install exports the RmlUi::Debugger target when it was
    # built with the debugger enabled; look for a stray debugger library only
    # if it did not.
    if(NOT TARGET RmlUi::Debugger)
      find_library(RmlUi_DEBUGGER_LIBRARY
        NAMES "rmlui_debugger" "RmlUiDebugger" "rmlui_debuggerd" "RmlUiDebuggerd")
      mark_as_advanced(RmlUi_DEBUGGER_LIBRARY)

      if(RmlUi_DEBUGGER_LIBRARY)
        add_library(RmlUi::Debugger UNKNOWN IMPORTED GLOBAL)
        set_target_properties(RmlUi::Debugger PROPERTIES
          IMPORTED_LOCATION "${RmlUi_DEBUGGER_LIBRARY}"
          INTERFACE_LINK_LIBRARIES RmlUi::RmlUi)
      endif()
    endif()
    return()
  endif()
endif()

# Fall back to manual detection.
find_path(RmlUi_INCLUDE_DIR "RmlUi/Core.h")

# RmlUi's headers carry no version macro, so gate on a header that was
# introduced in 6.0 along with the render API this module requires.
if(NOT RmlUi_INCLUDE_DIR OR
   EXISTS "${RmlUi_INCLUDE_DIR}/RmlUi/Core/RenderManager.h")
  set(RmlUi_VERSION_6_OR_LATER TRUE)
else()
  set(RmlUi_VERSION_6_OR_LATER FALSE)
endif()

# "rmlui" is the library name since 5.0; the other spellings cover pre-6
# packaging variants, which the header check above will reject anyway, giving
# a version message rather than a silent not-found.
find_library(RmlUi_LIBRARY
  NAMES "rmlui" "RmlUi" "rmlui_core" "RmlUiCore")

find_library(RmlUi_DEBUG_LIBRARY
  NAMES "rmluid" "RmlUid" "rmlui_cored" "RmlUiCored")

find_library(RmlUi_DEBUGGER_LIBRARY
  NAMES "rmlui_debugger" "RmlUiDebugger")

find_library(RmlUi_DEBUGGER_DEBUG_LIBRARY
  NAMES "rmlui_debuggerd" "RmlUiDebuggerd")

mark_as_advanced(RmlUi_INCLUDE_DIR RmlUi_LIBRARY RmlUi_DEBUG_LIBRARY
                 RmlUi_DEBUGGER_LIBRARY RmlUi_DEBUGGER_DEBUG_LIBRARY)

if(RmlUi_VERSION_6_OR_LATER AND RmlUi_INCLUDE_DIR AND
   (RmlUi_LIBRARY OR RmlUi_DEBUG_LIBRARY))
  if(NOT TARGET RmlUi::RmlUi)
    add_library(RmlUi::RmlUi UNKNOWN IMPORTED GLOBAL)
    set_target_properties(RmlUi::RmlUi PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${RmlUi_INCLUDE_DIR}")

    # The plain IMPORTED_LOCATION serves configurations not matched below
    # (e.g. RelWithDebInfo), which would otherwise rely on CMake picking an
    # arbitrary available configuration.
    if(RmlUi_LIBRARY)
      set_property(TARGET RmlUi::RmlUi APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(RmlUi::RmlUi PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
        IMPORTED_LOCATION "${RmlUi_LIBRARY}"
        IMPORTED_LOCATION_RELEASE "${RmlUi_LIBRARY}")
    endif()

    if(RmlUi_DEBUG_LIBRARY)
      set_property(TARGET RmlUi::RmlUi APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(RmlUi::RmlUi PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
        IMPORTED_LOCATION_DEBUG "${RmlUi_DEBUG_LIBRARY}")
      if(NOT RmlUi_LIBRARY)
        set_target_properties(RmlUi::RmlUi PROPERTIES
          IMPORTED_LOCATION "${RmlUi_DEBUG_LIBRARY}")
      endif()
    endif()
  endif()

  if(NOT TARGET RmlUi::Debugger)
    if(RmlUi_DEBUGGER_LIBRARY OR RmlUi_DEBUGGER_DEBUG_LIBRARY)
      add_library(RmlUi::Debugger UNKNOWN IMPORTED GLOBAL)
      set_target_properties(RmlUi::Debugger PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RmlUi_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES RmlUi::RmlUi)
      if(RmlUi_DEBUGGER_LIBRARY)
        set_property(TARGET RmlUi::Debugger APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(RmlUi::Debugger PROPERTIES
          IMPORTED_LOCATION "${RmlUi_DEBUGGER_LIBRARY}"
          IMPORTED_LOCATION_RELEASE "${RmlUi_DEBUGGER_LIBRARY}")
      endif()
      if(RmlUi_DEBUGGER_DEBUG_LIBRARY)
        set_property(TARGET RmlUi::Debugger APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(RmlUi::Debugger PROPERTIES
          IMPORTED_LOCATION_DEBUG "${RmlUi_DEBUGGER_DEBUG_LIBRARY}")
        if(NOT RmlUi_DEBUGGER_LIBRARY)
          set_target_properties(RmlUi::Debugger PROPERTIES
            IMPORTED_LOCATION "${RmlUi_DEBUGGER_DEBUG_LIBRARY}")
        endif()
      endif()
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
if(RmlUi_LIBRARY)
  set(_RmlUi_ANY_LIBRARY "${RmlUi_LIBRARY}")
elseif(RmlUi_DEBUG_LIBRARY)
  set(_RmlUi_ANY_LIBRARY "${RmlUi_DEBUG_LIBRARY}")
endif()
find_package_handle_standard_args(RmlUi DEFAULT_MSG
  RmlUi_INCLUDE_DIR _RmlUi_ANY_LIBRARY RmlUi_VERSION_6_OR_LATER)
